#include "sm4.h"
#include <openssl/evp.h>
#include <cstring>

std::vector<unsigned char> SM4::cbcEncrypt(const std::vector<unsigned char>& plain,
                                           const std::vector<unsigned char>& key,
                                           const std::vector<unsigned char>& iv)
{
    if (key.size() != 16 || iv.size() != 16)
        return {};

    // Manual PKCS#7 padding — we control it, not OpenSSL
    size_t padLen = 16 - (plain.size() % 16);
    std::vector<unsigned char> padded(plain.size() + padLen);
    std::memcpy(padded.data(), plain.data(), plain.size());
    for (size_t i = plain.size(); i < padded.size(); i++)
        padded[i] = static_cast<unsigned char>(padLen);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        return {};

    const EVP_CIPHER* cipher = EVP_sm4_cbc();
    if (EVP_EncryptInit_ex(ctx, cipher, NULL, key.data(), iv.data()) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    EVP_CIPHER_CTX_set_padding(ctx, 0);  // disable OpenSSL auto-padding

    std::vector<unsigned char> ciphertext(padded.size() + 16);
    int outlen = 0, tmplen = 0;
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &outlen, padded.data(), padded.size()) <= 0 ||
        EVP_EncryptFinal_ex(ctx, ciphertext.data() + outlen, &tmplen) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    outlen += tmplen;
    EVP_CIPHER_CTX_free(ctx);
    ciphertext.resize(outlen);
    return ciphertext;
}

std::vector<unsigned char> SM4::cbcDecrypt(const std::vector<unsigned char>& cipher,
                                           const std::vector<unsigned char>& key,
                                           const std::vector<unsigned char>& iv)
{
    if (key.size() != 16 || iv.size() != 16 || cipher.empty() || cipher.size() % 16 != 0)
        return {};

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        return {};

    const EVP_CIPHER* cipherType = EVP_sm4_cbc();
    if (EVP_DecryptInit_ex(ctx, cipherType, NULL, key.data(), iv.data()) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    EVP_CIPHER_CTX_set_padding(ctx, 0);  // disable OpenSSL auto-padding-removal

    std::vector<unsigned char> plain(cipher.size() + 16);
    int outlen = 0, tmplen = 0;
    if (EVP_DecryptUpdate(ctx, plain.data(), &outlen, cipher.data(), cipher.size()) <= 0 ||
        EVP_DecryptFinal_ex(ctx, plain.data() + outlen, &tmplen) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    outlen += tmplen;
    EVP_CIPHER_CTX_free(ctx);
    plain.resize(outlen);

    // Strip PKCS#7 padding — always, since we control it entirely
    if (plain.empty()) return plain;
    unsigned char padLen = plain.back();
    if (padLen == 0 || padLen > 16 || padLen > plain.size())
        return {};
    for (size_t i = plain.size() - padLen; i < plain.size(); i++) {
        if (plain[i] != padLen)
            return {};
    }
    plain.resize(plain.size() - padLen);
    return plain;
}
