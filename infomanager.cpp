#include "infomanager.h"
#include "sm2.h"

InfoManager::InfoManager(QObject *parent) : QObject(parent) {}

void InfoManager::setSelfNick(const QString& nick) { selfNick = nick; }
QString InfoManager::getSelfNick() const { return selfNick; }
void InfoManager::setSelfWalletAddress(const QString& walletAddress) { selfWalletAddress = walletAddress; }
QString InfoManager::getSelfWalletAddress() const { return selfWalletAddress; }

bool InfoManager::generateSelfKeys()
{
    return SM2::generateKeyPair(selfPriKey, selfPubKey);
}

std::vector<unsigned char> InfoManager::getSelfPrivateKey() const { return selfPriKey; }
std::vector<unsigned char> InfoManager::getSelfPublicKey() const { return selfPubKey; }
QString InfoManager::getSelfPublicKeyHex() const { return QString::fromStdString(SM2::pubKeyToHex(selfPubKey)); }

bool InfoManager::addOrUpdateUser(const QString& ip, const QString& nick, const QString& pubKeyHex, const QString& walletAddress)
{
    std::vector<unsigned char> pubKey = SM2::hexToPubKey(pubKeyHex.toStdString());
    if (ip.trimmed().isEmpty() || nick.trimmed().isEmpty() ||
        pubKey.size() != 65 || pubKey[0] != 0x04) {
        return false;
    }

    OnlineUser user;
    user.ip = ip;
    user.nick = nick;
    user.walletAddress = walletAddress.trimmed();
    user.pubKey = pubKey;
    users[ip] = user;
    emit userListChanged();
    return true;
}

void InfoManager::removeUser(const QString& ip)
{
    users.remove(ip);
    emit userListChanged();
}

std::vector<unsigned char> InfoManager::getPubKeyByIp(const QString& ip) const
{
    if (users.contains(ip))
        return users[ip].pubKey;
    return {};
}

QString InfoManager::getNickByIp(const QString& ip) const
{
    if (users.contains(ip))
        return users[ip].nick;
    return "";
}

QString InfoManager::getWalletByIp(const QString& ip) const
{
    if (users.contains(ip))
        return users[ip].walletAddress;
    return "";
}

QString InfoManager::getIpByNick(const QString& nick) const
{
    for (auto it = users.begin(); it != users.end(); ++it) {
        if (it->nick == nick)
            return it.key();
    }
    return "";
}

QList<OnlineUser> InfoManager::getOnlineUsers() const
{
    return users.values();
}
