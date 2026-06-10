#ifndef SM4_H
#define SM4_H

#include <vector>

class SM4
{
public:
    static std::vector<unsigned char> cbcEncrypt(const std::vector<unsigned char>& plain,
                                                 const std::vector<unsigned char>& key,
                                                 const std::vector<unsigned char>& iv);
    static std::vector<unsigned char> cbcDecrypt(const std::vector<unsigned char>& cipher,
                                                 const std::vector<unsigned char>& key,
                                                 const std::vector<unsigned char>& iv);
};

#endif // SM4_H
