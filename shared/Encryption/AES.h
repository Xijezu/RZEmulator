#ifndef AESPASSWORDCIPHER_H
#define AESPASSWORDCIPHER_H

/*
 * Taken from glandu2s librzu: https://github.com/glandu2/librzu
 */

#include <memory>
#include <stdint.h>
#include <vector>

typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;

class AesPasswordCipher {
public:
    AesPasswordCipher();
    ~AesPasswordCipher();

    void init(const uint8_t *key);
    void init();
    void getKey(std::vector<uint8_t> &key);

    bool encrypt(const uint8_t *input, size_t input_size, std::vector<uint8_t> &output);
    bool decrypt(const uint8_t *input, size_t input_size, std::vector<uint8_t> &output);

private:
    uint8_t key[32];
    std::unique_ptr<EVP_CIPHER_CTX, void (*)(EVP_CIPHER_CTX *)> evpCipher;
};

#endif // AESPASSWORDCIPHER_H