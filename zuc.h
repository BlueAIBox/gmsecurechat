#ifndef ZUC_H
#define ZUC_H

#include <vector>

class ZUC
{
public:
    static void init(const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv);
    static void generateKeystream(unsigned char* output, int len);
    static std::vector<unsigned char> encrypt(const std::vector<unsigned char>& plain,
                                              const std::vector<unsigned char>& key,
                                              const std::vector<unsigned char>& iv);
    static std::vector<unsigned char> decrypt(const std::vector<unsigned char>& cipher,
                                              const std::vector<unsigned char>& key,
                                              const std::vector<unsigned char>& iv);
};

#endif // ZUC_H
