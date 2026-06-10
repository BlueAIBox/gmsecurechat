#include "utils.h"
#include "sm3.h"
#include <QByteArray>
#include <QStringList>
#include <openssl/rand.h>

bool Utils::randomBytes(std::vector<unsigned char>& buf)
{
    if (buf.empty())
        return true;
    return RAND_bytes(buf.data(), static_cast<int>(buf.size())) == 1;
}

QString Utils::bytesToBase64(const std::vector<unsigned char>& bytes)
{
    QByteArray raw(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    return QString::fromLatin1(raw.toBase64());
}

std::vector<unsigned char> Utils::base64ToBytes(const QString& base64)
{
    QByteArray ba = QByteArray::fromBase64(base64.toLatin1());
    if (ba.isEmpty() && !base64.isEmpty())
        return {};
    return std::vector<unsigned char>(ba.begin(), ba.end());
}

QString Utils::bytesToHex(const std::vector<unsigned char>& bytes, int maxLen)
{
    QString result;
    int len = (maxLen > 0 && maxLen < (int)bytes.size()) ? maxLen : (int)bytes.size();
    for (int i = 0; i < len; i++) {
        result += QString::asprintf("%02x", bytes[i]);
    }
    if (maxLen > 0 && (int)bytes.size() > maxLen)
        result += "...";
    return result;
}

QString Utils::shortFingerprint(const std::vector<unsigned char>& bytes, int bytesShown)
{
    if (bytes.empty() || bytesShown <= 0)
        return "";

    std::vector<unsigned char> digest = SM3::hash(bytes);
    QString hex = bytesToHex(digest, bytesShown);
    QStringList groups;
    for (int i = 0; i < hex.size(); i += 4)
        groups << hex.mid(i, 4);
    return groups.join(":").toUpper();
}
