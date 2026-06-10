#include "messagehandler.h"
#include "sm2.h"
#include "sm3.h"
#include "sm4.h"
#include "utils.h"
#include <QByteArray>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

MessageHandler::MessageHandler(QObject *parent) : QObject(parent) {}

static QString hexPreview(const std::vector<unsigned char>& data, int bytes = 16)
{
    QString hex = Utils::bytesToHex(data);
    int maxChars = qMax(1, bytes) * 2;
    if (hex.size() <= maxChars)
        return hex;
    return hex.left(maxChars) + "...";
}

static QString textPreview(QString text, int chars = 32)
{
    text.replace("\r", "\\r").replace("\n", "\\n");
    if (text.size() <= chars)
        return text;
    return text.left(chars) + "...";
}

QByteArray MessageHandler::createEncryptedPayload(const QString& plainText,
                                                  const std::vector<unsigned char>& senderPrivateKey,
                                                  const std::vector<unsigned char>& receiverPublicKey,
                                                  const QString& fromNick,
                                                  Logger& logger)
{
    if (senderPrivateKey.empty() || receiverPublicKey.empty()) {
        logger.logCrypto("[错误] 发送方私钥或接收方公钥为空，无法生成国密密文包");
        return {};
    }

    QByteArray plainUtf8 = plainText.toUtf8();
    std::vector<unsigned char> plainBytes(plainUtf8.begin(), plainUtf8.end());

    std::vector<unsigned char> msgHash = SM3::hash(plainBytes);
    logger.logCrypto("[2/5] SM3 消息哈希: " + Utils::bytesToHex(msgHash));
    logger.logCrypto(QString::fromUtf8("[SM3] 明文摘要: %1 | 明文长度: %2 字节")
                     .arg(Utils::bytesToHex(msgHash))
                     .arg(plainBytes.size()));

    std::vector<unsigned char> signature = SM2::sign(msgHash, senderPrivateKey);
    if (signature.empty()) {
        logger.logCrypto("[错误] SM2 签名失败");
        return {};
    }
    logger.logCrypto("[3/5] SM2 签名: " + Utils::bytesToHex(signature));
    logger.logCrypto(QString::fromUtf8("[SM2] 摘要签名完成: %1 | 签名长度: %2 字节")
                     .arg(Utils::bytesToHex(signature))
                     .arg(signature.size()));

    std::vector<unsigned char> sm4Key(16);
    std::vector<unsigned char> iv(16);
    if (!Utils::randomBytes(sm4Key) || !Utils::randomBytes(iv)) {
        logger.logCrypto("[错误] 安全随机数生成失败");
        return {};
    }
    logger.logCrypto(QString::fromUtf8("[RNG] 生成一次性 SM4 会话密钥和 IV: key=%1 iv=%2")
                     .arg(Utils::bytesToHex(sm4Key), Utils::bytesToHex(iv)));

    std::vector<unsigned char> cipherText = SM4::cbcEncrypt(plainBytes, sm4Key, iv);
    std::vector<unsigned char> encryptedKey = SM2::encrypt(sm4Key, receiverPublicKey);
    if (cipherText.empty() || encryptedKey.empty()) {
        logger.logCrypto("[错误] SM4 加密或 SM2 密钥封装失败");
        return {};
    }
    logger.logCrypto("[4/5] SM4-CBC 加密完成, 密钥经 SM2 封装");
    logger.logCrypto(QString::fromUtf8("[SM4] CBC 加密完成: 密文长度 %1 字节 | cipher=%2")
                     .arg(cipherText.size())
                     .arg(Utils::bytesToHex(cipherText)));
    logger.logCrypto(QString::fromUtf8("[SM2] 会话密钥封装完成: encKey 长度 %1 字节 | encKey=%2")
                     .arg(encryptedKey.size())
                     .arg(Utils::bytesToHex(encryptedKey)));

    QJsonObject msg;
    msg["type"] = "message";
    msg["app"] = "gmsecurechat";
    msg["version"] = 2;
    msg["suite"] = "SM2+SM3+SM4-CBC";
    msg["fromNick"] = fromNick;
    msg["encKey"] = Utils::bytesToBase64(encryptedKey);
    msg["iv"] = Utils::bytesToBase64(iv);
    msg["cipher"] = Utils::bytesToBase64(cipherText);
    msg["msgHash"] = Utils::bytesToBase64(msgHash);
    msg["signature"] = Utils::bytesToBase64(signature);
    msg["timestamp"] = QDateTime::currentDateTime().toMSecsSinceEpoch();

    return QJsonDocument(msg).toJson();
}

bool MessageHandler::sendEncryptedMessage(const QString& plainText, const QString& targetIp,
                                          InfoManager& infoMgr, UDPManager& udp, Logger& logger)
{
    logger.logCrypto("========== 发送加密消息 ==========");
    logger.logCrypto(QString::fromUtf8("【发送流水线】明文进入安全管道: \"%1\"").arg(textPreview(plainText)));

    std::vector<unsigned char> remotePub = infoMgr.getPubKeyByIp(targetIp);
    if (remotePub.empty()) {
        logger.logCrypto("[错误] 未找到接收方公钥");
        return false;
    }
    logger.logCrypto("[1/5] 目标公钥: " + Utils::bytesToHex(remotePub));
    logger.logCrypto(QString::fromUtf8("[SM2] 接收方公钥指纹: %1").arg(Utils::shortFingerprint(remotePub)));

    QByteArray data = createEncryptedPayload(plainText,
                                             infoMgr.getSelfPrivateKey(),
                                             remotePub,
                                             infoMgr.getSelfNick(),
                                             logger);
    if (data.isEmpty())
        return false;

    udp.sendEncryptedMessage(targetIp, data);
    logger.logCrypto("[5/5] 密文已发送 -> " + targetIp);
    logger.logCrypto(QString::fromUtf8("[UDP] 已发送密文 JSON 包 -> %1 | 包大小 %2 字节 | 聊天窗口仅显示本地明文副本")
                     .arg(targetIp)
                     .arg(data.size()));
    return true;
}

void MessageHandler::onReceiveRawMessage(const QByteArray& data, const QString& fromIp,
                                         InfoManager& infoMgr, Logger& logger, UDPManager& udp)
{
    logger.logCrypto("========== 接收加密消息 ==========");
    logger.logCrypto(QString::fromUtf8("【接收流水线】收到 UDP 数据包: %1 字节，开始解析安全字段").arg(data.size()));

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) return;
    QJsonObject obj = doc.object();
    if (obj["type"].toString() != "message") return;
    QString fromNick = obj["fromNick"].toString();

    std::vector<unsigned char> priKey = infoMgr.getSelfPrivateKey();
    if (priKey.empty()) {
        logger.logCrypto("[错误] 本地私钥为空");
        return;
    }

    logger.logCrypto("[1/6] 收到密文，来自: " + fromNick + " (" + fromIp + ")");
    logger.logCrypto(QString::fromUtf8("[UDP] 密文包来源: %1 (%2)").arg(fromNick, fromIp));

    std::vector<unsigned char> encKey = Utils::base64ToBytes(obj["encKey"].toString());
    std::vector<unsigned char> sm4Key = SM2::decrypt(encKey, priKey);
    if (sm4Key.size() != 16) {
        logger.logCrypto("[错误] SM2 解密会话密钥失败");
        logger.logCrypto(QString::fromUtf8("[SM2] 解封失败输入 encKey=%1").arg(Utils::bytesToHex(encKey)));
        return;
    }
    logger.logCrypto("[2/6] SM2 解密会话密钥: " + Utils::bytesToHex(sm4Key));
    logger.logCrypto(QString::fromUtf8("[SM2] 解封会话密钥成功: encKey=%1 | sm4Key=%2")
                     .arg(Utils::bytesToHex(encKey), Utils::bytesToHex(sm4Key)));

    std::vector<unsigned char> iv = Utils::base64ToBytes(obj["iv"].toString());
    std::vector<unsigned char> cipher = Utils::base64ToBytes(obj["cipher"].toString());
    if (iv.size() != 16 || cipher.empty()) {
        logger.logCrypto("[错误] 消息字段不完整，已拒绝显示");
        logger.logCrypto(QString::fromUtf8("[字段] ivLen=%1 cipherLen=%2 encKeyLen=%3")
                         .arg(iv.size())
                         .arg(cipher.size())
                         .arg(encKey.size()));
        return;
    }
    logger.logCrypto(QString::fromUtf8("[SM4] 准备解密: cipher=%1 | 密文长度 %2 字节 | iv=%3")
                     .arg(Utils::bytesToHex(cipher))
                     .arg(cipher.size())
                     .arg(Utils::bytesToHex(iv)));

    std::vector<unsigned char> plain = SM4::cbcDecrypt(cipher, sm4Key, iv);
    if (plain.empty() && !cipher.empty()) {
        logger.logCrypto("[错误] SM4-CBC 解密失败，已拒绝显示");
        return;
    }
    logger.logCrypto("[3/6] SM4-CBC 解密完成, 明文长度: " + QString::number(plain.size()) + " 字节");
    QString plainText = QString::fromUtf8(reinterpret_cast<const char*>(plain.data()), plain.size());
    logger.logCrypto(QString::fromUtf8("[SM4] 解密得到明文: \"%1\" | 明文长度 %2 字节")
                     .arg(textPreview(plainText, 120))
                     .arg(plain.size()));

    std::vector<unsigned char> computedHash = SM3::hash(plain);
    logger.logCrypto("[4/6] SM3 计算消息哈希: " + Utils::bytesToHex(computedHash));
    logger.logCrypto(QString::fromUtf8("[SM3] 本地重新计算摘要: %1").arg(Utils::bytesToHex(computedHash)));

    std::vector<unsigned char> receivedHash = Utils::base64ToBytes(obj["msgHash"].toString());
    bool hashMatch = (computedHash == receivedHash);
    logger.logCrypto("[5/6] 哈希校验: " + QString(hashMatch ? "一致" : "不匹配!"));
    logger.logCrypto("      本地: " + Utils::bytesToHex(computedHash));
    logger.logCrypto("      收到: " + Utils::bytesToHex(receivedHash));
    logger.logCrypto(QString::fromUtf8("[SM3] 完整性校验: %1")
                     .arg(hashMatch ? QString::fromUtf8("通过，消息未被篡改") : QString::fromUtf8("失败，摘要不一致")));

    std::vector<unsigned char> sig = Utils::base64ToBytes(obj["signature"].toString());
    logger.logCrypto(QString::fromUtf8("[SM2] 收到签名: %1 | 签名长度: %2 字节")
                     .arg(Utils::bytesToHex(sig))
                     .arg(sig.size()));
    std::vector<unsigned char> remotePub = infoMgr.getPubKeyByIp(fromIp);
    if (remotePub.empty()) {
        logger.logCrypto("[错误] 未找到发送方公钥，无法验签，已拒绝显示");
        return;
    }

    bool sigOk = SM2::verify(computedHash, sig, remotePub);
    logger.logCrypto("[6/6] SM2 验签: " + QString(sigOk ? "通过" : "失败!"));
    logger.logCrypto(QString::fromUtf8("[SM2] 发送者验签: %1 | 发送方公钥指纹: %2")
                     .arg(sigOk ? QString::fromUtf8("通过，身份可信") : QString::fromUtf8("失败，身份不可信"))
                     .arg(Utils::shortFingerprint(remotePub)));
    if (!hashMatch || !sigOk) {
        logger.logCrypto("[拦截] 消息完整性或身份校验失败，未进入聊天窗口");
        logger.logCrypto(QString::fromUtf8("[拦截] 安全校验未通过，消息不会显示到聊天窗口"));
        return;
    }

    logger.logCrypto(QString::fromUtf8("[放行] SM4解密 + SM3完整性 + SM2验签全部通过，消息进入聊天窗口"));
    emit udp.messageReceived(fromIp, fromNick, plainText);
}
