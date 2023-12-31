#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/encryption.h"
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

// Function to generate RSA key pair
RSA* generateRSAKeyPair() {
    RSA* rsa = RSA_new();
    BIGNUM* e = BN_new();
    unsigned long exponent = RSA_F4;

    BN_set_word(e, exponent);
    RSA_generate_key_ex(rsa, 2048, e, NULL);

    BN_free(e);
    return rsa;
}

RSA* extractRSAPublicKey(RSA* rsa) {
    RSA* publicKey = RSAPublicKey_dup(rsa);
    return publicKey;
}

// Function to extract the private key from an RSA structure
RSA* extractRSAPrivateKey(RSA* rsa) {
    RSA* privateKey = RSAPrivateKey_dup(rsa);
    return privateKey;
}

// Function to perform RSA encryption
unsigned char* rsaEncrypt(const char* plaintext, RSA* rsa) {
    int rsaSize = RSA_size(rsa);
    printf("size : %d\n",rsaSize);
    unsigned char* ciphertext = (unsigned char*)malloc(rsaSize);
    int result = RSA_public_encrypt(strlen(plaintext) + 1, (const unsigned char*)plaintext, ciphertext, rsa, RSA_PKCS1_PADDING);

    if (result != -1) {
        printf("Encrypted Text: ");
        for (int i = 0; i < result; ++i) {
            printf("%02x", ciphertext[i]);
        }
        printf("\n");
    } else {
        fprintf(stderr, "Encryption failed\n");
    }

    return ciphertext;
}

// Function to perform RSA decryption
unsigned char* rsaDecrypt(const unsigned char* ciphertext, RSA* rsa) {
    int ciphertextSize = RSA_size(rsa);
    unsigned char* decryptedText = (unsigned char*)malloc(ciphertextSize);
    int result = RSA_private_decrypt(ciphertextSize, ciphertext, decryptedText, rsa, RSA_PKCS1_PADDING);

    if (result != -1) {
        printf("Decrypted Text: %s\n", decryptedText);
    } else {
        fprintf(stderr, "Decryption failed\n");
    }

    return decryptedText;
}

int main() {
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();

    // Generate RSA key pair
    RSA* rsa = generateRSAKeyPair();

    RSA *privateKey = extractRSAPrivateKey(rsa);
    RSA *publicKey = extractRSAPublicKey(rsa);

    // Example: Encrypt and decrypt a message
    const char* plaintext = "Hello, RSA!";
    const unsigned char* ciphertext = rsaEncrypt(plaintext,publicKey);
    
    rsaDecrypt(ciphertext, privateKey);

    // Clean up
    RSA_free(rsa);
    EVP_cleanup();
    ERR_free_strings();

    return 0;
}





