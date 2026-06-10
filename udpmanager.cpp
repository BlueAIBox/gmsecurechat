#include "udpmanager.h"
#include <QNetworkInterface>
#include <QJsonDocument>
#include <QJsonObject>

UDPManager::UDPManager(QObject *parent) : QObject(parent), socket(new QUdpSocket(this)), localPort(0) {}

bool UDPManager::start(quint16 port)
{
    localPort = port;
    if (socket->state() != QAbstractSocket::UnconnectedState)
        socket->close();

    if (!socket->bind(QHostAddress::AnyIPv4, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
        return false;

    connect(socket, &QUdpSocket::readyRead, this, &UDPManager::processPendingDatagrams, Qt::UniqueConnection);
    return true;
}

void UDPManager::stop()
{
    socket->close();
}

void UDPManager::sendLoginBroadcast(const QString& nick, const QString& pubKeyHex, const QString& walletAddress)
{
    QJsonObject obj;
    obj["type"] = "login";
    obj["app"] = "gmsecurechat";
    obj["version"] = 2;
    obj["suite"] = "SM2+SM3+SM4/ZUC";
    obj["nick"] = nick;
    obj["pubKey"] = pubKeyHex;
    obj["wallet"] = walletAddress.trimmed();
    QByteArray data = QJsonDocument(obj).toJson();

    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    for (const auto& iface : ifaces) {
        if (!(iface.flags() & QNetworkInterface::IsUp) || (iface.flags() & QNetworkInterface::IsLoopBack))
            continue;
        for (const auto& addr : iface.addressEntries()) {
            if (addr.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                QHostAddress broadcast = addr.broadcast();
                if (!broadcast.isNull()) {
                    socket->writeDatagram(data, broadcast, localPort);
                }
            }
        }
    }
}

void UDPManager::sendLoginResponse(const QString& targetIp, const QString& nick, const QString& pubKeyHex, const QString& walletAddress)
{
    QJsonObject obj;
    obj["type"] = "login_ack";
    obj["app"] = "gmsecurechat";
    obj["version"] = 2;
    obj["suite"] = "SM2+SM3+SM4/ZUC";
    obj["nick"] = nick;
    obj["pubKey"] = pubKeyHex;
    obj["wallet"] = walletAddress.trimmed();
    QByteArray data = QJsonDocument(obj).toJson();
    socket->writeDatagram(data, QHostAddress(targetIp), localPort);
}

void UDPManager::sendLogoutBroadcast(const QString& nick)
{
    QJsonObject obj;
    obj["type"] = "logout";
    obj["app"] = "gmsecurechat";
    obj["version"] = 2;
    obj["nick"] = nick;
    QByteArray data = QJsonDocument(obj).toJson();
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    for (const auto& iface : ifaces) {
        if (!(iface.flags() & QNetworkInterface::IsUp) || (iface.flags() & QNetworkInterface::IsLoopBack))
            continue;
        for (const auto& addr : iface.addressEntries()) {
            if (addr.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                QHostAddress broadcast = addr.broadcast();
                if (!broadcast.isNull())
                    socket->writeDatagram(data, broadcast, localPort);
            }
        }
    }
}

void UDPManager::sendEncryptedMessage(const QString& targetIp, const QByteArray& data)
{
    socket->writeDatagram(data, QHostAddress(targetIp), localPort);
}

void UDPManager::sendFileChunk(const QString& targetIp, const QByteArray& chunk)
{
    socket->writeDatagram(chunk, QHostAddress(targetIp), localPort);
}

void UDPManager::sendKeyExchangeData(const QString& targetIp, const QByteArray& data)
{
    socket->writeDatagram(data, QHostAddress(targetIp), localPort);
}

QStringList UDPManager::getLocalIPs() const
{
    QStringList ips;
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    for (const auto& iface : ifaces) {
        if (!(iface.flags() & QNetworkInterface::IsUp) || (iface.flags() & QNetworkInterface::IsLoopBack))
            continue;
        for (const auto& addr : iface.addressEntries()) {
            if (addr.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                ips.append(addr.ip().toString());
            }
        }
    }
    return ips;
}

static QString normalizeIp(const QHostAddress& addr)
{
    QString s = addr.toString();
    if (s.startsWith("::ffff:"))
        return s.mid(7);
    return s;
}

void UDPManager::processPendingDatagrams()
{
    while (socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QString senderIp = normalizeIp(sender);

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(datagram, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            QString type = obj["type"].toString();
            if (type == "login") {
                emit loginBroadcastReceived(senderIp, obj["nick"].toString(), obj["pubKey"].toString(), obj["wallet"].toString());
            } else if (type == "login_ack") {
                emit loginAckReceived(senderIp, obj["nick"].toString(), obj["pubKey"].toString(), obj["wallet"].toString());
            } else if (type == "logout") {
                emit logoutBroadcastReceived(senderIp, obj["nick"].toString());
            } else if (type == "message") {
                emit rawMessageReceived(senderIp, datagram);
            } else if (type == "file_chunk" || type == "file_signatures") {
                emit rawFileChunkReceived(senderIp, datagram);
            }
        }
    }
}
