#include "sm3.h"
#include <openssl/evp.h>

std::vector<unsigned char> SM3::hash(const std::vector<unsigned char>& data)
{
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sm3();
    unsigned char digest[32];
    unsigned int len = 32;
    EVP_DigestInit_ex(ctx, md, NULL);
    EVP_DigestUpdate(ctx, data.data(), data.size());
    EVP_DigestFinal_ex(ctx, digest, &len);
    EVP_MD_CTX_free(ctx);
    return std::vector<unsigned char>(digest, digest + 32);
}

std::string SM3::hashString(const std::string& data)
{
    auto h = hash(std::vector<unsigned char>(data.begin(), data.end()));
    return std::string(h.begin(), h.end());
}
