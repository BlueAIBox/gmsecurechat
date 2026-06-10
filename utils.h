#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <vector>

class Utils
{
public:
    static bool randomBytes(std::vector<unsigned char>& buf);
    static QString bytesToBase64(const std::vector<unsigned char>& bytes);
    static std::vector<unsigned char> base64ToBytes(const QString& base64);
    static QString bytesToHex(const std::vector<unsigned char>& bytes, int maxLen = -1);
    static QString shortFingerprint(const std::vector<unsigned char>& bytes, int bytesShown = 8);
};

#endif // UTILS_H
