#include "hermesbridge.h"
#include "infomanager.h"
#include "logger.h"
#include "messagehandler.h"
#include "sm2.h"
#include "sm3.h"
#include "sm4.h"
#include "utils.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QTimer>
#include <memory>

namespace {
struct HermesRequestState {
    QByteArray out;
    QByteArray err;
};

QString shellQuote(const QString& value)
{
    QString escaped = value;
    escaped.replace(QStringLiteral("'"), QStringLiteral("'\\''"));
    return QStringLiteral("'") + escaped + QStringLiteral("'");
}
}

HermesBridge::HermesBridge(QObject* parent)
    : QObject(parent)
    , currentEndpoint(QStringLiteral("hermes -z"))
{
    ensureIdentity();
}

QString HermesBridge::peerId()
{
    return QStringLiteral("hermes://local");
}

QString HermesBridge::displayName()
{
    return QString::fromUtf8("Hermes Agent");
}

bool HermesBridge::ensureIdentity()
{
    if (!priKey.empty() && !pubKey.empty())
        return true;
    return SM2::generateKeyPair(priKey, pubKey);
}

QString HermesBridge::publicKeyHex() const
{
    return QString::fromStdString(SM2::pubKeyToHex(pubKey));
}

std::vector<unsigned char> HermesBridge::publicKey() const
{
    return pubKey;
}

QString HermesBridge::fingerprint() const
{
    return Utils::shortFingerprint(pubKey);
}

void HermesBridge::setEndpoint(const QString& endpoint)
{
    QString trimmed = endpoint.trimmed();
    if (!trimmed.isEmpty())
        currentEndpoint = trimmed;
}

QString HermesBridge::endpoint() const
{
    return currentEndpoint;
}

bool HermesBridge::sendPrompt(const QString& plainText,
                              const InfoManager& infoMgr,
                              MessageHandler& msgHandler,
                              Logger& logger)
{
    if (!ensureIdentity()) {
        emit statusMessage(QString::fromUtf8("[HermesBridge] Hermes 本地 SM2 身份生成失败"), true);
        return false;
    }
    if (infoMgr.getSelfPrivateKey().empty() || infoMgr.getSelfPublicKey().empty()) {
        emit statusMessage(QString::fromUtf8("[HermesBridge] 请先登录，生成本机 SM2 身份后再连接 Hermes"), true);
        return false;
    }

    logger.logCrypto(QString::fromUtf8("========== Hermes Agent 本地加密桥接 =========="));
    logger.logCrypto(QString::fromUtf8("[1/6] 目标虚拟用户: %1 | SM2 指纹: %2")
                     .arg(displayName(), fingerprint()));

    QByteArray outgoing = msgHandler.createEncryptedPayload(plainText,
                                                            infoMgr.getSelfPrivateKey(),
                                                            pubKey,
                                                            infoMgr.getSelfNick(),
                                                            logger);
    if (outgoing.isEmpty())
        return false;

    bool unwrapOk = false;
    QString prompt = decryptForHermes(outgoing, infoMgr.getSelfPublicKey(), logger, &unwrapOk);
    if (!unwrapOk) {
        emit statusMessage(QString::fromUtf8("[HermesBridge] 本地桥接解密/验签失败，未把消息交给 Hermes"), true);
        return false;
    }

    logger.logCrypto(QString::fromUtf8("[5/6] HermesBridge 已完成 SM2 解封 + SM4 解密 + SM3/SM2 校验"));
    emit statusMessage(QString::fromUtf8("[HermesBridge] 已把加密消息安全解包，准备请求 WSL Hermes"), false);

    requestWslReply(prompt, infoMgr.getSelfPublicKey(), msgHandler, logger);
    return true;
}

QString HermesBridge::decryptForHermes(const QByteArray& packet,
                                       const std::vector<unsigned char>& senderPublicKey,
                                       Logger& logger,
                                       bool* ok) const
{
    if (ok)
        *ok = false;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(packet, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        logger.logCrypto(QString::fromUtf8("[HermesBridge] 密文包 JSON 解析失败"));
        return {};
    }

    QJsonObject obj = doc.object();
    std::vector<unsigned char> encKey = Utils::base64ToBytes(obj["encKey"].toString());
    std::vector<unsigned char> sm4Key = SM2::decrypt(encKey, priKey);
    if (sm4Key.size() != 16) {
        logger.logCrypto(QString::fromUtf8("[HermesBridge] SM2 解封会话密钥失败"));
        return {};
    }

    std::vector<unsigned char> iv = Utils::base64ToBytes(obj["iv"].toString());
    std::vector<unsigned char> cipher = Utils::base64ToBytes(obj["cipher"].toString());
    std::vector<unsigned char> plain = SM4::cbcDecrypt(cipher, sm4Key, iv);
    QString plainText = QString::fromUtf8(reinterpret_cast<const char*>(plain.data()), plain.size());

    std::vector<unsigned char> computedHash = SM3::hash(plain);
    std::vector<unsigned char> receivedHash = Utils::base64ToBytes(obj["msgHash"].toString());
    std::vector<unsigned char> sig = Utils::base64ToBytes(obj["signature"].toString());
    bool hashOk = (computedHash == receivedHash);
    bool sigOk = SM2::verify(computedHash, sig, senderPublicKey);

    logger.logCrypto(QString::fromUtf8("[HermesBridge] 本机 Agent 解包: SM3=%1 | SM2验签=%2 | 明文长度=%3 字节")
                     .arg(hashOk ? QString::fromUtf8("通过") : QString::fromUtf8("失败"))
                     .arg(sigOk ? QString::fromUtf8("通过") : QString::fromUtf8("失败"))
                     .arg(plain.size()));
    if (!hashOk || !sigOk)
        return {};

    if (ok)
        *ok = true;
    return plainText;
}

void HermesBridge::requestWslReply(const QString& prompt,
                                   const std::vector<unsigned char>& selfPublicKey,
                                   MessageHandler& msgHandler,
                                   Logger& logger)
{
    QString command = currentEndpoint.trimmed();
    if (command.startsWith(QStringLiteral("wsl:"), Qt::CaseInsensitive))
        command = command.mid(4).trimmed();
    if (command.isEmpty())
        command = QStringLiteral("hermes -z");
    if (command == QStringLiteral("hermes"))
        command = QStringLiteral("hermes -z");

    QString shellCommand = command;
    if (shellCommand.contains(QStringLiteral("%1"))) {
        shellCommand = shellCommand.arg(shellQuote(prompt));
    } else {
        shellCommand += QStringLiteral(" ") + shellQuote(prompt);
    }

    QProcess* process = new QProcess(this);
    process->setProgram(QStringLiteral("wsl.exe"));
    process->setArguments(QStringList() << QStringLiteral("bash") << QStringLiteral("-lc") << shellCommand);
    process->setProcessChannelMode(QProcess::SeparateChannels);
    auto state = std::make_shared<HermesRequestState>();

    QTimer* timeout = new QTimer(process);
    timeout->setSingleShot(true);
    connect(timeout, &QTimer::timeout, process, [this, process]() {
        emit statusMessage(QString::fromUtf8("[HermesBridge] Hermes -z 超时，已终止本次请求"), true);
        process->kill();
    });

    connect(process, &QProcess::readyReadStandardOutput, process, [process, state]() {
        state->out.append(process->readAllStandardOutput());
    });
    connect(process, &QProcess::readyReadStandardError, process, [process, state]() {
        state->err.append(process->readAllStandardError());
    });

    connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this, process, timeout, state,
             selfPublicKey, &msgHandler, &logger, prompt](int exitCode, QProcess::ExitStatus status) mutable {
                timeout->stop();
                state->out.append(process->readAllStandardOutput());
                state->err.append(process->readAllStandardError());
                process->deleteLater();

                if (status != QProcess::NormalExit || exitCode != 0) {
                    emit statusMessage(QString::fromUtf8("[HermesBridge] WSL Hermes 返回失败: ") +
                                       QString::fromUtf8(state->err.left(240)), true);
                    return;
                }

                QString replyText = extractReplyText(state->out, prompt);
                if (replyText.isEmpty() && !state->err.isEmpty())
                    replyText = extractReplyText(state->err, prompt);
                if (replyText.isEmpty()) {
                    emit statusMessage(QString::fromUtf8("[HermesBridge] Hermes -z 没有产生可解析回复"), true);
                } else {
                    deliverReply(replyText, selfPublicKey, msgHandler, logger);
                }
            });

    process->start();
    timeout->start(120000);
    emit statusMessage(QString::fromUtf8("[HermesBridge] 已通过 WSL one-shot 命令请求 Hermes: ") + command, false);
}

void HermesBridge::requestPlainReply(const QString& requestId,
                                     const QString& prompt,
                                     Logger& logger)
{
    QString command = currentEndpoint.trimmed();
    if (command.startsWith(QStringLiteral("wsl:"), Qt::CaseInsensitive))
        command = command.mid(4).trimmed();
    if (command.isEmpty())
        command = QStringLiteral("hermes -z");
    if (command == QStringLiteral("hermes"))
        command = QStringLiteral("hermes -z");

    QString shellCommand = command;
    if (shellCommand.contains(QStringLiteral("%1"))) {
        shellCommand = shellCommand.arg(shellQuote(prompt));
    } else {
        shellCommand += QStringLiteral(" ") + shellQuote(prompt);
    }

    QProcess* process = new QProcess(this);
    process->setProgram(QStringLiteral("wsl.exe"));
    process->setArguments(QStringList() << QStringLiteral("bash") << QStringLiteral("-lc") << shellCommand);
    process->setProcessChannelMode(QProcess::SeparateChannels);
    auto state = std::make_shared<HermesRequestState>();

    QTimer* timeout = new QTimer(process);
    timeout->setSingleShot(true);
    connect(timeout, &QTimer::timeout, process, [this, process, state]() {
        state->err = QByteArray("Hermes proxy request timeout");
        emit statusMessage(QString::fromUtf8("[HermesBridge] Hermes 代理请求超时，已终止本次请求"), true);
        process->kill();
    });

    connect(process, &QProcess::readyReadStandardOutput, process, [process, state]() {
        state->out.append(process->readAllStandardOutput());
    });
    connect(process, &QProcess::readyReadStandardError, process, [process, state]() {
        state->err.append(process->readAllStandardError());
    });

    connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this, process, timeout, state, requestId, prompt](int exitCode, QProcess::ExitStatus status) mutable {
                timeout->stop();
                state->out.append(process->readAllStandardOutput());
                state->err.append(process->readAllStandardError());
                process->deleteLater();

                if (status != QProcess::NormalExit || exitCode != 0) {
                    QString error = QString::fromUtf8(state->err.left(360)).trimmed();
                    if (error.isEmpty())
                        error = QString::fromUtf8("Hermes one-shot command failed");
                    emit plainReplyReady(requestId, QString(), error);
                    return;
                }

                QString replyText = extractReplyText(state->out, prompt);
                if (replyText.isEmpty() && !state->err.isEmpty())
                    replyText = extractReplyText(state->err, prompt);
                if (replyText.isEmpty()) {
                    emit plainReplyReady(requestId, QString(), QString::fromUtf8("Hermes 没有产生可解析回复"));
                } else {
                    emit plainReplyReady(requestId, replyText, QString());
                }
            });

    logger.logCrypto(QString::fromUtf8("[HermesProxy] 已通过 WSL one-shot 命令请求 Hermes: %1 | request=%2")
                     .arg(command, requestId.left(12)));
    process->start();
    timeout->start(120000);
}

void HermesBridge::deliverReply(const QString& replyText,
                                const std::vector<unsigned char>& selfPublicKey,
                                MessageHandler& msgHandler,
                                Logger& logger)
{
    QString text = replyText.trimmed();
    if (text.isEmpty()) {
        emit statusMessage(QString::fromUtf8("[HermesBridge] Hermes 返回内容为空"), true);
        return;
    }

    logger.logCrypto(QString::fromUtf8("[6/6] Hermes 回复进入国密回包: SM2签名 + SM4加密 -> 本机接收链路"));
    QByteArray packet = msgHandler.createEncryptedPayload(text,
                                                          priKey,
                                                          selfPublicKey,
                                                          displayName(),
                                                          logger);
    if (packet.isEmpty()) {
        emit statusMessage(QString::fromUtf8("[HermesBridge] Hermes 回复加密封包失败"), true);
        return;
    }

    emit encryptedReplyReady(packet);
}

QString HermesBridge::extractReplyText(const QByteArray& body, const QString& prompt) const
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(body, &err);
    if (err.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();
        const QStringList keys = {
            QStringLiteral("reply"),
            QStringLiteral("message"),
            QStringLiteral("response"),
            QStringLiteral("content"),
            QStringLiteral("text"),
            QStringLiteral("answer")
        };
        for (const QString& key : keys) {
            if (obj.contains(key) && obj.value(key).isString())
                return obj.value(key).toString().trimmed();
        }

        QJsonArray choices = obj.value(QStringLiteral("choices")).toArray();
        if (!choices.isEmpty() && choices.first().isObject()) {
            QJsonObject first = choices.first().toObject();
            QJsonObject message = first.value(QStringLiteral("message")).toObject();
            if (message.value(QStringLiteral("content")).isString())
                return message.value(QStringLiteral("content")).toString().trimmed();
            if (first.value(QStringLiteral("text")).isString())
                return first.value(QStringLiteral("text")).toString().trimmed();
        }

        return QString::fromUtf8(doc.toJson(QJsonDocument::Compact)).trimmed();
    }

    QString text = QString::fromUtf8(body);
    text.remove(QRegularExpression(QStringLiteral("\\x1B\\[[0-?]*[ -/]*[@-~]")));
    text.remove(QRegularExpression(QStringLiteral("\\x1B\\][^\\x07]*(\\x07|\\x1B\\\\)")));
    text.replace('\r', '\n');

    QString normalizedPrompt = prompt.trimmed();
    if (!normalizedPrompt.isEmpty()) {
        int promptPos = text.lastIndexOf(normalizedPrompt, Qt::CaseInsensitive);
        if (promptPos >= 0)
            text = text.mid(promptPos + normalizedPrompt.size());
    }

    QStringList kept;
    for (QString line : text.split('\n')) {
        line = line.trimmed();
        if (line.isEmpty())
            continue;
        line.remove(QRegularExpression(QStringLiteral("[\\x00-\\x08\\x0B\\x0C\\x0E-\\x1F]")));
        line = line.simplified();
        if (line.isEmpty())
            continue;

        bool noise =
            line.contains(QStringLiteral("Hermes Agent v")) ||
            line.contains(QStringLiteral("Available Tools")) ||
            line.contains(QStringLiteral("Available Skills")) ||
            line.contains(QStringLiteral("Welcome to Hermes Agent")) ||
            line.contains(QStringLiteral("Script started")) ||
            line.contains(QStringLiteral("Script done")) ||
            line.contains(QStringLiteral("Session:")) ||
            line.contains(QStringLiteral("Tip:")) ||
            line.contains(QStringLiteral("current one stays free")) ||
            line.contains(QStringLiteral("mimo-v2.5-pro")) ||
            line.contains(QStringLiteral("ctx --")) ||
            line.contains(QStringLiteral("Goodbye!")) ||
            line.contains(QStringLiteral("browser:")) ||
            line.contains(QStringLiteral("browser-cdp")) ||
            line.contains(QStringLiteral("cronjob:")) ||
            line.contains(QStringLiteral("delegation:")) ||
            line.contains(QStringLiteral("data-science:")) ||
            line.contains(QStringLiteral("kanban-")) ||
            line.contains(QStringLiteral("hermes-agent")) ||
            line.contains(QStringLiteral("code_execution:")) ||
            line.contains(QStringLiteral("autonomous-ai-agents:")) ||
            line.contains(QStringLiteral("polymarket,")) ||
            line.contains(QStringLiteral("github:")) ||
            line.contains(QStringLiteral("commands")) ||
            line.contains(QStringLiteral("tools")) ||
            line.contains(QStringLiteral("skills")) ||
            line.count('|') >= 4 ||
            QRegularExpression(QStringLiteral("^[\\s_\\-—─=|>]+$")).match(line).hasMatch() ||
            line.startsWith(QStringLiteral("|")) ||
            line.startsWith(QStringLiteral("—")) ||
            line.startsWith(QStringLiteral("─")) ||
            line.startsWith(QStringLiteral("-"));
        if (noise)
            continue;

        QString cleaned = line;
        int arrow = cleaned.lastIndexOf(QStringLiteral("->"));
        if (arrow >= 0)
            cleaned = cleaned.mid(arrow + 2).trimmed();
        cleaned.remove(QRegularExpression(QStringLiteral("^[$>]+\\s*")));
        cleaned = cleaned.trimmed();

        if (!normalizedPrompt.isEmpty() &&
            cleaned.compare(normalizedPrompt, Qt::CaseInsensitive) == 0) {
            continue;
        }
        if (!cleaned.isEmpty())
            kept << cleaned;
    }

    if (kept.isEmpty())
        return QString();
    return kept.join('\n').left(2400).trimmed();
}
