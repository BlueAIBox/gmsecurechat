#include "filehandler.h"
#include "sm2.h"
#include "sm3.h"
#include "zuc.h"
#include "utils.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QtGlobal>

namespace {
constexpr int CHUNK_SIZE = 900;

std::vector<unsigned char> toBytes(const QByteArray& data)
{
    return std::vector<unsigned char>(data.begin(), data.end());
}

QByteArray toByteArray(const std::vector<unsigned char>& data)
{
    return QByteArray(reinterpret_cast<const char*>(data.data()), static_cast<int>(data.size()));
}

QString makeTransferKey(const QString& ip, const QString& transferId)
{
    return ip + "|" + transferId;
}

QString makeTransferId()
{
    std::vector<unsigned char> nonce(8);
    if (!Utils::randomBytes(nonce))
        return QString::number(QDateTime::currentMSecsSinceEpoch(), 16);
    return Utils::bytesToHex(nonce);
}

std::vector<unsigned char> deriveChunkIv(const std::vector<unsigned char>& baseIv, int index)
{
    std::vector<unsigned char> seed = baseIv;
    seed.push_back(static_cast<unsigned char>((index >> 24) & 0xFF));
    seed.push_back(static_cast<unsigned char>((index >> 16) & 0xFF));
    seed.push_back(static_cast<unsigned char>((index >> 8) & 0xFF));
    seed.push_back(static_cast<unsigned char>(index & 0xFF));

    std::vector<unsigned char> digest = SM3::hash(seed);
    return std::vector<unsigned char>(digest.begin(), digest.begin() + 16);
}
}

FileHandler::FileHandler(QObject *parent) : QObject(parent) {}

bool FileHandler::sendFile(const QString& filePath, const QString& targetIp,
                           InfoManager& infoMgr, UDPManager& udp, Logger& logger)
{
    logger.logCrypto("========== 发送加密文件 ==========");

    std::vector<unsigned char> remotePub = infoMgr.getPubKeyByIp(targetIp);
    if (remotePub.empty()) {
        logger.logCrypto("[错误] 无法获取对方公钥，发送文件失败");
        return false;
    }

    std::vector<unsigned char> shareKey;
    if (!SM2::keyExchange(infoMgr.getSelfPrivateKey(), remotePub, shareKey) || shareKey.size() < 32) {
        logger.logCrypto("[错误] SM2 密钥交换失败");
        return false;
    }

    std::vector<unsigned char> zucKey(shareKey.begin(), shareKey.begin() + 16);
    std::vector<unsigned char> baseIv(shareKey.begin() + 16, shareKey.begin() + 32);
    logger.logCrypto("[1/8] SM2 密钥协商完成，SM3 派生 ZUC 会话密钥: " + Utils::bytesToHex(zucKey, 8));

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        logger.logNetwork("[错误] 无法打开文件: " + filePath);
        return false;
    }
    QByteArray fileData = file.readAll();
    file.close();

    QFileInfo fi(filePath);
    QString fileName = fi.fileName();
    qint64 fileSize = fileData.size();

    std::vector<unsigned char> fileDataVec = toBytes(fileData);
    std::vector<unsigned char> fileHash = SM3::hash(fileDataVec);
    QString transferId = makeTransferId();
    logger.logCrypto("[2/8] SM3 文件哈希: " + Utils::bytesToHex(fileHash));

    int totalChunks = qMax(1, static_cast<int>((fileData.size() + CHUNK_SIZE - 1) / CHUNK_SIZE));
    logger.logCrypto("[3/8] 文件分块: " + QString::number(totalChunks) +
                     " 块, 每块独立 ZUC-IV, TransferID " + transferId);

    for (int i = 0; i < totalChunks; i++) {
        QByteArray chunk = fileData.mid(i * CHUNK_SIZE, CHUNK_SIZE);
        std::vector<unsigned char> chunkVec = toBytes(chunk);
        std::vector<unsigned char> signature = SM2::sign(chunkVec, infoMgr.getSelfPrivateKey());
        if (signature.empty()) {
            logger.logCrypto("[错误] 第 " + QString::number(i + 1) + " 块 SM2 签名失败");
            return false;
        }

        std::vector<unsigned char> chunkIv = deriveChunkIv(baseIv, i);
        std::vector<unsigned char> cipherVec = ZUC::encrypt(chunkVec, zucKey, chunkIv);
        QByteArray cipherBytes = toByteArray(cipherVec);

        QJsonObject block;
        block["type"] = "file_chunk";
        block["app"] = "gmsecurechat";
        block["version"] = 2;
        block["suite"] = "SM2+SM3+ZUC";
        block["transferId"] = transferId;
        block["fileName"] = fileName;
        block["fileSize"] = QString::number(fileSize);
        block["fileHash"] = Utils::bytesToBase64(fileHash);
        block["index"] = i;
        block["total"] = totalChunks;
        block["data"] = QString::fromLatin1(cipherBytes.toBase64());
        block["signature"] = Utils::bytesToBase64(signature);
        block["last"] = (i == totalChunks - 1);
        udp.sendFileChunk(targetIp, QJsonDocument(block).toJson(QJsonDocument::Compact));

        QThread::msleep(6);
    }

    logger.logCrypto("[4/8] ZUC 逐块加密完成，避免重复使用同一密钥流");
    logger.logCrypto("[5/8] 每个文件块已附带独立 SM2 签名");
    logger.logCrypto("[6/8] SM3 文件哈希随块发送，用于最终完整性校验");
    logger.logCrypto("[7/8] UDP 分块发送完成");
    logger.logCrypto("[8/8] 文件发送完成: " + fileName);
    return true;
}

void FileHandler::onReceiveFileChunk(const QByteArray& chunkData, const QString& fromIp,
                                     InfoManager& infoMgr, Logger& logger, UDPManager& udp)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(chunkData, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return;

    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();
    if (type == "file_signatures") {
        logger.logCrypto("[提示] 收到旧版签名汇总包；当前协议使用“签名随块发送”");
        return;
    }
    if (type != "file_chunk")
        return;

    int idx = obj["index"].toInt(-1);
    int total = obj["total"].toInt(0);
    if (total <= 0 || idx < 0 || idx >= total) {
        logger.logCrypto("[错误] 文件块序号异常，已丢弃");
        return;
    }

    QString transferId = obj["transferId"].toString();
    if (transferId.isEmpty())
        transferId = obj["fileHash"].toString().left(16);

    QByteArray cipher = QByteArray::fromBase64(obj["data"].toString().toLatin1());
    QByteArray signature = QByteArray::fromBase64(obj["signature"].toString().toLatin1());
    if (signature.isEmpty()) {
        logger.logCrypto("[错误] 文件块缺少 SM2 签名，已丢弃");
        return;
    }

    QString fromNick = infoMgr.getNickByIp(fromIp);
    if (fromNick.isEmpty())
        fromNick = fromIp;

    QString key = makeTransferKey(fromIp, transferId);
    if (!pendingReceives.contains(key)) {
        ReceivingFile rf;
        rf.fromNick = fromNick;
        rf.fileName = obj["fileName"].toString();
        rf.fileSize = obj["fileSize"].toString().toLongLong();
        rf.totalChunks = total;
        rf.transferId = transferId;
        rf.encryptedChunks.resize(total);
        rf.signatures.resize(total);
        rf.receivedFlags.fill(false, total);
        rf.expectedFileHash = Utils::base64ToBytes(obj["fileHash"].toString());
        pendingReceives[key] = rf;

        logger.logCrypto("========== 接收加密文件 ==========");
        logger.logCrypto("[1/8] 接收文件: " + rf.fileName + ", 大小 " +
                         QString::number(rf.fileSize) + " 字节, " +
                         QString::number(total) + " 块, TransferID " + transferId);
    }

    ReceivingFile& rf = pendingReceives[key];
    if (rf.totalChunks != total) {
        logger.logCrypto("[错误] 文件块总数不一致，已丢弃");
        return;
    }

    if (!rf.receivedFlags[idx]) {
        rf.encryptedChunks[idx] = cipher;
        rf.signatures[idx] = signature;
        rf.receivedFlags[idx] = true;
    }

    int receivedCount = 0;
    for (bool flag : rf.receivedFlags) {
        if (flag)
            receivedCount++;
    }
    logger.logCrypto("[2/8] 接收块 " + QString::number(idx + 1) + "/" +
                     QString::number(total) + " (" + QString::number(receivedCount) +
                     "/" + QString::number(total) + " 已收到)");

    tryAssembleFile(key, fromIp, infoMgr, logger, udp);
}

bool FileHandler::tryAssembleFile(const QString& key, const QString& fromIp,
                                  InfoManager& infoMgr, Logger& logger, UDPManager& udp)
{
    if (!pendingReceives.contains(key))
        return false;

    ReceivingFile& rf = pendingReceives[key];
    for (bool received : rf.receivedFlags) {
        if (!received)
            return false;
    }

    logger.logCrypto("[3/8] 所有数据块已收齐，按 index 重组密文");

    std::vector<unsigned char> remotePub = infoMgr.getPubKeyByIp(fromIp);
    std::vector<unsigned char> shareKey;
    if (remotePub.empty() ||
        !SM2::keyExchange(infoMgr.getSelfPrivateKey(), remotePub, shareKey) ||
        shareKey.size() < 32) {
        logger.logCrypto("[错误] SM2 密钥交换失败");
        pendingReceives.remove(key);
        return false;
    }

    std::vector<unsigned char> zucKey(shareKey.begin(), shareKey.begin() + 16);
    std::vector<unsigned char> baseIv(shareKey.begin() + 16, shareKey.begin() + 32);
    logger.logCrypto("[4/8] SM2+SM3 派生 ZUC 会话密钥完成");

    QByteArray plainData;
    bool verifyOk = true;
    for (int i = 0; i < rf.totalChunks; i++) {
        std::vector<unsigned char> chunkCipherVec = toBytes(rf.encryptedChunks[i]);
        std::vector<unsigned char> chunkIv = deriveChunkIv(baseIv, i);
        std::vector<unsigned char> plainVec = ZUC::decrypt(chunkCipherVec, zucKey, chunkIv);

        std::vector<unsigned char> sig = toBytes(rf.signatures[i]);
        if (!SM2::verify(plainVec, sig, remotePub)) {
            verifyOk = false;
            logger.logCrypto("[警告] 第 " + QString::number(i + 1) + " 块 SM2 验签失败");
        }

        plainData.append(toByteArray(plainVec));
    }

    if (plainData.size() > rf.fileSize)
        plainData.truncate(static_cast<int>(rf.fileSize));
    if (plainData.size() != rf.fileSize) {
        verifyOk = false;
        logger.logCrypto("[警告] 文件长度不一致，期望 " + QString::number(rf.fileSize) +
                         " 字节，实际 " + QString::number(plainData.size()) + " 字节");
    }

    logger.logCrypto("[5/8] ZUC 解密 + 逐块 SM2 验签: " + QString(verifyOk ? "全部通过" : "存在失败"));

    std::vector<unsigned char> fileHash = SM3::hash(toBytes(plainData));
    logger.logCrypto("[6/8] SM3 文件哈希: " + Utils::bytesToHex(fileHash));
    bool hashMatch = (fileHash == rf.expectedFileHash);
    logger.logCrypto("[7/8] 文件哈希校验: " + QString(hashMatch ? "一致" : "不匹配!"));
    logger.logCrypto("      本地:  " + Utils::bytesToHex(fileHash));
    logger.logCrypto("      期望:  " + Utils::bytesToHex(rf.expectedFileHash));

    bool finalOk = verifyOk && hashMatch;
    logger.logCrypto("[8/8] 文件接收完成: " + rf.fileName +
                     " | 完整性: " + QString(finalOk ? "通过" : "失败"));

    emit udp.fileReceived(rf.fromNick, rf.fileName, plainData, finalOk);
    pendingReceives.remove(key);
    return finalOk;
}
