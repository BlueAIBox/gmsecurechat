#ifndef HERMESBRIDGE_H
#define HERMESBRIDGE_H

#include <QByteArray>
#include <QObject>
#include <QString>
#include <vector>

class InfoManager;
class Logger;
class MessageHandler;

class HermesBridge : public QObject
{
    Q_OBJECT
public:
    explicit HermesBridge(QObject* parent = nullptr);

    static QString peerId();
    static QString displayName();

    bool ensureIdentity();
    QString publicKeyHex() const;
    std::vector<unsigned char> publicKey() const;
    QString fingerprint() const;

    void setEndpoint(const QString& endpoint);
    QString endpoint() const;

    bool sendPrompt(const QString& plainText,
                    const InfoManager& infoMgr,
                    MessageHandler& msgHandler,
                    Logger& logger);
    void requestPlainReply(const QString& requestId,
                           const QString& prompt,
                           Logger& logger);

signals:
    void encryptedReplyReady(const QByteArray& packet);
    void plainReplyReady(const QString& requestId, const QString& replyText, const QString& errorText);
    void statusMessage(const QString& message, bool isError);

private:
    QString decryptForHermes(const QByteArray& packet,
                             const std::vector<unsigned char>& senderPublicKey,
                             Logger& logger,
                             bool* ok) const;
    void requestWslReply(const QString& prompt,
                         const std::vector<unsigned char>& selfPublicKey,
                         MessageHandler& msgHandler,
                         Logger& logger);
    void deliverReply(const QString& replyText,
                      const std::vector<unsigned char>& selfPublicKey,
                      MessageHandler& msgHandler,
                      Logger& logger);
    QString extractReplyText(const QByteArray& body, const QString& prompt = QString()) const;

    std::vector<unsigned char> priKey;
    std::vector<unsigned char> pubKey;
    QString currentEndpoint;
};

#endif // HERMESBRIDGE_H
