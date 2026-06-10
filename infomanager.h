#ifndef INFOMANAGER_H
#define INFOMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <vector>

struct OnlineUser {
    QString ip;
    QString nick;
    QString walletAddress;
    std::vector<unsigned char> pubKey;
};

class InfoManager : public QObject
{
    Q_OBJECT
public:
    explicit InfoManager(QObject *parent = nullptr);

    void setSelfNick(const QString& nick);
    QString getSelfNick() const;
    void setSelfWalletAddress(const QString& walletAddress);
    QString getSelfWalletAddress() const;
    bool generateSelfKeys();
    std::vector<unsigned char> getSelfPrivateKey() const;
    std::vector<unsigned char> getSelfPublicKey() const;
    QString getSelfPublicKeyHex() const;

    bool addOrUpdateUser(const QString& ip, const QString& nick, const QString& pubKeyHex, const QString& walletAddress = QString());
    void removeUser(const QString& ip);
    std::vector<unsigned char> getPubKeyByIp(const QString& ip) const;
    QString getNickByIp(const QString& ip) const;
    QString getWalletByIp(const QString& ip) const;
    QString getIpByNick(const QString& nick) const;
    QList<OnlineUser> getOnlineUsers() const;

signals:
    void userListChanged();

private:
    QString selfNick;
    QString selfWalletAddress;
    std::vector<unsigned char> selfPriKey;
    std::vector<unsigned char> selfPubKey;
    QMap<QString, OnlineUser> users;
};

#endif // INFOMANAGER_H
