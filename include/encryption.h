#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <openssl/rsa.h>

void extractDHPublicKey(EVP_PKEY *keypair, unsigned char **public_key_data,
                        size_t *public_key_len);

void computeDHSecret(EVP_PKEY *priv_key, EVP_PKEY *peer_pub_key,
                     unsigned char **secret, size_t *secret_len);

void generateDHKeyPair(EVP_PKEY *params, EVP_PKEY **keypair);

void generateDHParameters(EVP_PKEY **params);

void handleErrors(void);

int aes_decrypt(const unsigned char *ciphertext, int ciphertext_len,
                const unsigned char *key, const unsigned char *iv,
                unsigned char *plaintext);

int aes_encrypt(const unsigned char *plaintext, int plaintext_len,
                const unsigned char *key, const unsigned char *iv,
                unsigned char *ciphertext);

void generate_random_iv(unsigned char *iv, int size);

#endif
