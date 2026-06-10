#ifndef UDPMANAGER_H
#define UDPMANAGER_H

#include <QObject>
#include <QUdpSocket>

class UDPManager : public QObject
{
    Q_OBJECT
public:
    explicit UDPManager(QObject *parent = nullptr);
    bool start(quint16 port);
    void stop();
    void sendLoginBroadcast(const QString& nick, const QString& pubKeyHex, const QString& walletAddress = QString());
    void sendLogoutBroadcast(const QString& nick);
    void sendLoginResponse(const QString& targetIp, const QString& nick, const QString& pubKeyHex, const QString& walletAddress = QString());
    void sendEncryptedMessage(const QString& targetIp, const QByteArray& data);
    void sendFileChunk(const QString& targetIp, const QByteArray& chunk);
    void sendKeyExchangeData(const QString& targetIp, const QByteArray& data);
    QStringList getLocalIPs() const;

signals:
    void messageReceived(const QString& fromIp, const QString& fromNick, const QString& plainText);
    void fileReceived(const QString& fromNick, const QString& fileName, const QByteArray& fileData, bool verifyOk);
    void loginBroadcastReceived(const QString& ip, const QString& nick, const QString& pubKeyHex, const QString& walletAddress);
    void loginAckReceived(const QString& ip, const QString& nick, const QString& pubKeyHex, const QString& walletAddress);
    void logoutBroadcastReceived(const QString& ip, const QString& nick);
    void rawMessageReceived(const QString& fromIp, const QByteArray& data);
    void rawFileChunkReceived(const QString& fromIp, const QByteArray& chunk);

private slots:
    void processPendingDatagrams();

private:
    QUdpSocket* socket;
    quint16 localPort;
};

#endif // UDPMANAGER_H
