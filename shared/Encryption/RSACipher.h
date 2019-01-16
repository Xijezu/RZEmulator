#ifndef RSACIPHER_H
#define RSACIPHER_H

/*
* Taken from glandu2s librzu: https://github.com/glandu2/librzu
*/

#include "Common.h"
#include <memory>
#include <stdint.h>
#include <vector>

typedef struct rsa_st RSA;

class RsaCipher
{
  public:
    RsaCipher();
    ~RsaCipher();

    bool loadKey(const std::vector<uint8_t> &pemKey);
    bool getPemPublicKey(std::vector<uint8_t> &outKey);
    int generateKey();
    bool isInitialized();

    bool publicEncrypt(const uint8_t *input, size_t input_size, std::vector<uint8_t> &output);
    bool publicDecrypt(const uint8_t *input, size_t input_size, std::vector<uint8_t> &output);
    bool privateEncrypt(const uint8_t *input, size_t input_size, std::vector<uint8_t> &output);
    bool privateDecrypt(const uint8_t *input, size_t input_size, std::vector<uint8_t> &output);

  protected:
    void printError();

  private:
    std::unique_ptr<RSA, void (*)(RSA *)> rsaCipher;
};

#endif // RSACIPHER_H