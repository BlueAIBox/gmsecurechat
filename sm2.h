#ifndef SM2_H
#define SM2_H

#include <vector>
#include <string>

class SM2
{
public:
    static bool generateKeyPair(std::vector<unsigned char>& priKey,
                                std::vector<unsigned char>& pubKey);
    static std::vector<unsigned char> encrypt(const std::vector<unsigned char>& plain,
                                              const std::vector<unsigned char>& pubKey);
    static std::vector<unsigned char> decrypt(const std::vector<unsigned char>& cipher,
                                              const std::vector<unsigned char>& priKey);
    static std::vector<unsigned char> sign(const std::vector<unsigned char>& data,
                                           const std::vector<unsigned char>& priKey);
    static bool verify(const std::vector<unsigned char>& data,
                       const std::vector<unsigned char>& signature,
                       const std::vector<unsigned char>& pubKey);
    static bool keyExchange(const std::vector<unsigned char>& localPri,
                            const std::vector<unsigned char>& remotePub,
                            std::vector<unsigned char>& shareKey);
    static std::string pubKeyToHex(const std::vector<unsigned char>& pub);
    static std::vector<unsigned char> hexToPubKey(const std::string& hex);
};

#endif // SM2_H
