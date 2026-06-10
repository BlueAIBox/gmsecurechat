#include "sm2.h"
#include "sm3.h"
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/ec.h>
#include <cctype>
#include <cstring>
#include <vector>

static EC_KEY* createPrivateEcKey(const std::vector<unsigned char>& priKey)
{
    if (priKey.empty())
        return nullptr;

    EC_KEY* ec = EC_KEY_new_by_curve_name(NID_sm2);
    if (!ec)
        return nullptr;

    BIGNUM* priv = BN_bin2bn(priKey.data(), priKey.size(), NULL);
    if (!priv) {
        EC_KEY_free(ec);
        return nullptr;
    }

    if (EC_KEY_set_private_key(ec, priv) != 1) {
        BN_free(priv);
        EC_KEY_free(ec);
        return nullptr;
    }

    const EC_GROUP* group = EC_KEY_get0_group(ec);
    if (!group) {
        BN_free(priv);
        EC_KEY_free(ec);
        return nullptr;
    }

    EC_POINT* pub = EC_POINT_new(group);
    if (!pub || EC_POINT_mul(group, pub, priv, NULL, NULL, NULL) != 1 ||
        EC_KEY_set_public_key(ec, pub) != 1) {
        EC_POINT_free(pub);
        BN_free(priv);
        EC_KEY_free(ec);
        return nullptr;
    }

    EC_POINT_free(pub);
    BN_free(priv);
    return ec;
}

bool SM2::generateKeyPair(std::vector<unsigned char>& priKey, std::vector<unsigned char>& pubKey)
{
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_SM2, NULL);
    if (!ctx) return false;
    if (EVP_PKEY_keygen_init(ctx) <= 0) { EVP_PKEY_CTX_free(ctx); return false; }
    EVP_PKEY* pkey = NULL;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) { EVP_PKEY_CTX_free(ctx); return false; }
    EVP_PKEY_CTX_free(ctx);

    EC_KEY* ec_key = EVP_PKEY_get1_EC_KEY(pkey);
    const BIGNUM* priv = EC_KEY_get0_private_key(ec_key);
    priKey.resize(32);
    BN_bn2binpad(priv, priKey.data(), 32);

    const EC_POINT* pub = EC_KEY_get0_public_key(ec_key);
    const EC_GROUP* group = EC_KEY_get0_group(ec_key);
    size_t len = EC_POINT_point2oct(group, pub, POINT_CONVERSION_UNCOMPRESSED, NULL, 0, NULL);
    pubKey.resize(len);
    EC_POINT_point2oct(group, pub, POINT_CONVERSION_UNCOMPRESSED, pubKey.data(), len, NULL);

    EC_KEY_free(ec_key);
    EVP_PKEY_free(pkey);
    return true;
}

std::vector<unsigned char> SM2::encrypt(const std::vector<unsigned char>& plain, const std::vector<unsigned char>& pubKey)
{
    if (pubKey.empty())
        return {};

    EVP_PKEY* pkey = EVP_PKEY_new();
    EC_KEY* ec = EC_KEY_new_by_curve_name(NID_sm2);
    if (!pkey || !ec) {
        EVP_PKEY_free(pkey);
        EC_KEY_free(ec);
        return {};
    }
    const EC_GROUP* group = EC_KEY_get0_group(ec);
    EC_POINT* point = EC_POINT_new(group);
    if (!point || EC_POINT_oct2point(group, point, pubKey.data(), pubKey.size(), NULL) != 1 ||
        EC_KEY_set_public_key(ec, point) != 1) {
        EC_POINT_free(point);
        EC_KEY_free(ec);
        EVP_PKEY_free(pkey);
        return {};
    }
    EVP_PKEY_assign_EC_KEY(pkey, ec);
    EC_POINT_free(point);

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) { EVP_PKEY_free(pkey); return {}; }
    if (EVP_PKEY_encrypt_init(ctx) <= 0) { EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(pkey); return {}; }
    size_t outlen = 0;
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, plain.data(), plain.size()) <= 0) { EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(pkey); return {}; }
    std::vector<unsigned char> cipher(outlen);
    if (EVP_PKEY_encrypt(ctx, cipher.data(), &outlen, plain.data(), plain.size()) <= 0) { EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(pkey); return {}; }
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    cipher.resize(outlen);
    return cipher;
}

std::vector<unsigned char> SM2::decrypt(const std::vector<unsigned char>& cipher, const std::vector<unsigned char>& priKey)
{
    EC_KEY* ec = createPrivateEcKey(priKey);
    if (!ec) return {};

    EVP_PKEY* pkey = EVP_PKEY_new();
    if (!pkey) {
        EC_KEY_free(ec);
        return {};
    }
    EVP_PKEY_assign_EC_KEY(pkey, ec);

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) { EVP_PKEY_free(pkey); return {}; }
    if (EVP_PKEY_decrypt_init(ctx) <= 0) { EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(pkey); return {}; }
    size_t outlen = 0;
    if (EVP_PKEY_decrypt(ctx, NULL, &outlen, cipher.data(), cipher.size()) <= 0) { EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(pkey); return {}; }
    std::vector<unsigned char> plain(outlen);
    if (EVP_PKEY_decrypt(ctx, plain.data(), &outlen, cipher.data(), cipher.size()) <= 0) { EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(pkey); return {}; }
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    plain.resize(outlen);
    return plain;
}

std::vector<unsigned char> SM2::sign(const std::vector<unsigned char>& data, const std::vector<unsigned char>& priKey)
{
    EC_KEY* ec = createPrivateEcKey(priKey);
    if (!ec) return {};

    EVP_PKEY* pkey = EVP_PKEY_new();
    if (!pkey) {
        EC_KEY_free(ec);
        return {};
    }
    EVP_PKEY_assign_EC_KEY(pkey, ec);

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        EVP_PKEY_free(pkey);
        return {};
    }
    const EVP_MD* md = EVP_sm3();
    EVP_PKEY_CTX* pctx = NULL;
    if (EVP_DigestSignInit(mdctx, &pctx, md, NULL, pkey) <= 0) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return {};
    }
    size_t siglen = 0;
    if (EVP_DigestSign(mdctx, NULL, &siglen, data.data(), data.size()) <= 0) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return {};
    }
    std::vector<unsigned char> sig(siglen);
    if (EVP_DigestSign(mdctx, sig.data(), &siglen, data.data(), data.size()) <= 0) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return {};
    }
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);
    sig.resize(siglen);
    return sig;
}

bool SM2::verify(const std::vector<unsigned char>& data, const std::vector<unsigned char>& signature, const std::vector<unsigned char>& pubKey)
{
    if (signature.empty() || pubKey.empty())
        return false;

    EVP_PKEY* pkey = EVP_PKEY_new();
    EC_KEY* ec = EC_KEY_new_by_curve_name(NID_sm2);
    if (!pkey || !ec) {
        EVP_PKEY_free(pkey);
        EC_KEY_free(ec);
        return false;
    }
    const EC_GROUP* group = EC_KEY_get0_group(ec);
    EC_POINT* point = EC_POINT_new(group);
    if (!point || EC_POINT_oct2point(group, point, pubKey.data(), pubKey.size(), NULL) != 1 ||
        EC_KEY_set_public_key(ec, point) != 1) {
        EC_POINT_free(point);
        EC_KEY_free(ec);
        EVP_PKEY_free(pkey);
        return false;
    }
    EVP_PKEY_assign_EC_KEY(pkey, ec);
    EC_POINT_free(point);

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        EVP_PKEY_free(pkey);
        return false;
    }
    const EVP_MD* md = EVP_sm3();
    EVP_PKEY_CTX* pctx = NULL;
    if (EVP_DigestVerifyInit(mdctx, &pctx, md, NULL, pkey) <= 0) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return false;
    }
    int ret = EVP_DigestVerify(mdctx, signature.data(), signature.size(), data.data(), data.size());
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);
    return ret == 1;
}

bool SM2::keyExchange(const std::vector<unsigned char>& localPri, const std::vector<unsigned char>& remotePub, std::vector<unsigned char>& shareKey)
{
    shareKey.clear();

    EC_KEY* ec_local = createPrivateEcKey(localPri);
    if (!ec_local || remotePub.empty()) {
        EC_KEY_free(ec_local);
        return false;
    }

    BIGNUM* priv = BN_bin2bn(localPri.data(), localPri.size(), NULL);
    if (!priv) {
        EC_KEY_free(ec_local);
        return false;
    }

    const EC_GROUP* group = EC_KEY_get0_group(ec_local);
    EC_POINT* remote_point = EC_POINT_new(group);
    if (!remote_point || EC_POINT_oct2point(group, remote_point, remotePub.data(), remotePub.size(), NULL) != 1) {
        EC_POINT_free(remote_point);
        EC_KEY_free(ec_local);
        BN_free(priv);
        return false;
    }

    EC_POINT* shared = EC_POINT_new(group);
    if (!shared || EC_POINT_mul(group, shared, NULL, remote_point, priv, NULL) != 1) {
        EC_POINT_free(remote_point);
        EC_POINT_free(shared);
        EC_KEY_free(ec_local);
        BN_free(priv);
        return false;
    }

    unsigned char buf[65];
    size_t len = EC_POINT_point2oct(group, shared, POINT_CONVERSION_UNCOMPRESSED, buf, 65, NULL);
    if (len == 0) {
        EC_POINT_free(remote_point);
        EC_POINT_free(shared);
        EC_KEY_free(ec_local);
        BN_free(priv);
        return false;
    }

    std::vector<unsigned char> sharedPoint(buf, buf + len);
    shareKey = SM3::hash(sharedPoint);

    EC_POINT_free(remote_point);
    EC_POINT_free(shared);
    EC_KEY_free(ec_local);
    BN_free(priv);
    return shareKey.size() >= 32;
}

std::string SM2::pubKeyToHex(const std::vector<unsigned char>& pub)
{
    static const char hex[] = "0123456789abcdef";
    std::string out;
    out.reserve(pub.size() * 2);
    for (unsigned char c : pub) {
        out.push_back(hex[c >> 4]);
        out.push_back(hex[c & 0x0F]);
    }
    return out;
}

std::vector<unsigned char> SM2::hexToPubKey(const std::string& hex)
{
    std::vector<unsigned char> out;
    if (hex.size() % 2 != 0) return out;
    out.reserve(hex.size() / 2);

    auto nibble = [](char ch) -> int {
        unsigned char c = static_cast<unsigned char>(ch);
        if (c >= '0' && c <= '9') return c - '0';
        c = static_cast<unsigned char>(std::tolower(c));
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return -1;
    };

    for (size_t i = 0; i < hex.size(); i += 2) {
        int hi = nibble(hex[i]);
        int lo = nibble(hex[i + 1]);
        if (hi < 0 || lo < 0)
            return {};
        out.push_back(static_cast<unsigned char>((hi << 4) | lo));
    }
    return out;
}
