#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QByteArray>
#include <QObject>
#include <vector>
#include "infomanager.h"
#include "logger.h"
#include "udpmanager.h"

class MessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit MessageHandler(QObject *parent = nullptr);
    QByteArray createEncryptedPayload(const QString& plainText,
                                      const std::vector<unsigned char>& senderPrivateKey,
                                      const std::vector<unsigned char>& receiverPublicKey,
                                      const QString& fromNick,
                                      Logger& logger);
    bool sendEncryptedMessage(const QString& plainText, const QString& targetIp,
                              InfoManager& infoMgr, UDPManager& udp, Logger& logger);
    void onReceiveRawMessage(const QByteArray& data, const QString& fromIp,
                             InfoManager& infoMgr, Logger& logger, UDPManager& udp);
};

#endif // MESSAGEHANDLER_H
