/*
 * Taken from glandu2s librzu: https://github.com/glandu2/librzu
 */

#include "RSACipher.h"

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <string.h>

#include "Log.h"

RsaCipher::RsaCipher()
    : rsaCipher(nullptr, &RSA_free)
{
}

RsaCipher::~RsaCipher() {}

bool RsaCipher::loadKey(const std::vector<uint8_t> &pemKey)
{
    std::unique_ptr<BIO, int (*)(BIO *)> bio(nullptr, &BIO_free);

    bio.reset(BIO_new_mem_buf(const_cast<uint8_t *>(pemKey.data()), (int)pemKey.size()));
    rsaCipher.reset(PEM_read_bio_RSA_PUBKEY(bio.get(), NULL, NULL, NULL));

    if (!rsaCipher) {
        printError();
        return false;
    }

    return true;
}

bool RsaCipher::getPemPublicKey(std::vector<uint8_t> &outKey)
{
    if (rsaCipher) {
        std::unique_ptr<BIO, decltype(&::BIO_free)> b(BIO_new(BIO_s_mem()), ::BIO_free);
        if (!PEM_write_bio_RSA_PUBKEY(b.get(), rsaCipher.get())) {
            printError();
            return false;
        }

        outKey.resize(BIO_get_mem_data(b.get(), nullptr));
        BIO_read(b.get(), &outKey[0], (int)outKey.size());

        return true;
    }

    return false;
}

int RsaCipher::generateKey()
{
    std::unique_ptr<BIGNUM, decltype(&::BN_free)> e(BN_new(), ::BN_free);

    rsaCipher.reset(RSA_new());
    BN_set_word(e.get(), RSA_F4);
    int result = RSA_generate_key_ex(rsaCipher.get(), 1024, e.get(), NULL);
    if (!result)
        printError();

    return result;
}

bool RsaCipher::isInitialized()
{
    return rsaCipher.get() != nullptr;
}

bool RsaCipher::publicEncrypt(const uint8_t *input, size_t input_size, std::vector<uint8_t> &output)
{
    output.resize(RSA_size(rsaCipher.get()));
    int result = RSA_public_encrypt(input_size, input, output.data(), rsaCipher.get(), RSA_PKCS1_PADDING);
    if (result < 0) {
        output.clear();
        printError();
        return false;
    }

    output.resize(result);
    return true;
}

bool RsaCipher::publicDecrypt(const uint8_t *input, size_t input_size, std::vector<uint8_t> &output)
{
    output.resize(RSA_size(rsaCipher.get()));
    int result = RSA_public_decrypt(input_size, input, output.data(), rsaCipher.get(), RSA_PKCS1_PADDING);
    if (result < 0) {
        output.clear();
        printError();
        return false;
    }

    output.resize(result);
    return true;
}

bool RsaCipher::privateEncrypt(const uint8_t *input, size_t input_size, std::vector<uint8_t> &output)
{
    output.resize(RSA_size(rsaCipher.get()));
    int result = RSA_private_encrypt(input_size, input, output.data(), rsaCipher.get(), RSA_PKCS1_PADDING);
    if (result < 0) {
        output.clear();
        printError();
        return false;
    }

    output.resize(result);
    return true;
}

bool RsaCipher::privateDecrypt(const uint8_t *input, size_t input_size, std::vector<uint8_t> &output)
{
    output.resize(RSA_size(rsaCipher.get()));
    int result = RSA_private_decrypt(input_size, input, output.data(), rsaCipher.get(), RSA_PKCS1_PADDING);
    if (result < 0) {
        output.clear();
        printError();
        return false;
    }

    output.resize(result);
    return true;
}

void RsaCipher::printError()
{
    unsigned long errorCode = ERR_get_error();
    if (errorCode)
        NG_LOG_WARN("network", "AES: error: %s\n", ERR_error_string(errorCode, nullptr));
}