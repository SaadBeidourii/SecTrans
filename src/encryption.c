#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/encryption.h"
#include <openssl/dh.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define DH_BITS 2048

// Function to generate a random Initialization Vector (IV)
void generate_random_iv(unsigned char *iv, int size) {
    if (RAND_bytes(iv, size) != 1) {
        fprintf(stderr, "Error generating random IV\n");
        exit(EXIT_FAILURE);
    }
}

// Function to perform AES encryption
int aes_encrypt(const unsigned char *plaintext, int plaintext_len, const unsigned char *key,
                const unsigned char *iv, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len, ciphertext_len;

    // Create and initialize the context
    if (!(ctx = EVP_CIPHER_CTX_new()))
        return -1;

    // Initialize the encryption operation
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    // Perform the encryption
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len = len;

    // Finalize the encryption
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len += len;

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

// Function to perform AES decryption
int aes_decrypt(const unsigned char *ciphertext, int ciphertext_len, const unsigned char *key,
                const unsigned char *iv, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx;
    int len, plaintext_len;

    // Create and initialize the context
    if (!(ctx = EVP_CIPHER_CTX_new()))
        return -1;

    // Initialize the decryption operation
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    // Perform the decryption
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len = len;

    // Finalize the decryption
    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len += len;

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

void handleErrors(void) {
    ERR_print_errors_fp(stderr); // Print detailed OpenSSL errors
    perror("Error"); // Print system errors if any
    exit(EXIT_FAILURE);
}

void generateDHParameters(EVP_PKEY **params) {
    EVP_PKEY_CTX *param_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_DH, NULL);
    if (!param_ctx)
        handleErrors();

    if (EVP_PKEY_paramgen_init(param_ctx) <= 0)
        handleErrors();

    if (EVP_PKEY_CTX_set_dh_paramgen_prime_len(param_ctx, DH_BITS) <= 0)
        handleErrors();

    if (EVP_PKEY_paramgen(param_ctx, params) <= 0)
        handleErrors();

    EVP_PKEY_CTX_free(param_ctx);
}


void generateDHKeyPair(EVP_PKEY *params, EVP_PKEY **keypair) {
    // Generate key pair
    printf("Generating DH key pair...\n");
    EVP_PKEY_CTX *keygen_ctx = EVP_PKEY_CTX_new(params, NULL);
    if (!keygen_ctx)
        handleErrors();

    if (EVP_PKEY_keygen_init(keygen_ctx) <= 0)
        handleErrors();

    if (EVP_PKEY_keygen(keygen_ctx, keypair) <= 0)
        handleErrors();
    

    EVP_PKEY_CTX_free(keygen_ctx);
}

void computeDHSecret(EVP_PKEY *priv_key, EVP_PKEY *peer_pub_key,
                     unsigned char **secret, size_t *secret_len) {
    printf("Commencing computation of the DH secret...\n");

    // Create context for the DH secret computation
    EVP_PKEY_CTX *derive_ctx = EVP_PKEY_CTX_new(priv_key, NULL);
    if (!derive_ctx)
        handleErrors();

    // Initialize the context for deriving the shared secret
    if (EVP_PKEY_derive_init(derive_ctx) <= 0)
        handleErrors();

    // Setting the peer's public key for shared secret computation
    if (EVP_PKEY_derive_set_peer(derive_ctx, peer_pub_key) <= 0)
        handleErrors();

    // Determine buffer length for the shared secret
    if (EVP_PKEY_derive(derive_ctx, NULL, secret_len) <= 0)
        handleErrors();

    // Allocate memory for the shared secret
    *secret = (unsigned char *)malloc(*secret_len);
    if (*secret == NULL)
        handleErrors();

    // Derive the shared secret
    if (EVP_PKEY_derive(derive_ctx, *secret, secret_len) <= 0)
        handleErrors();

    // Free the context
    EVP_PKEY_CTX_free(derive_ctx);
}


void extractDHPublicKey(EVP_PKEY *keypair, unsigned char **public_key_data, size_t *public_key_len) {
	printf("we here\n");
    if (!EVP_PKEY_get_raw_public_key(keypair, NULL, public_key_len)) {
	    printf("please murder me i beg you\n");
        // Handle error
        handleErrors();
    }

	printf("we here\n");
    *public_key_data = (unsigned char *)malloc(*public_key_len);
	printf("we here\n");
    if (!*public_key_data) {
        // Handle memory allocation error
        handleErrors();
    }
	printf("we here\n");

    if (!EVP_PKEY_get_raw_public_key(keypair, *public_key_data, public_key_len)) {
        // Handle error
        free(*public_key_data);
        handleErrors();
    }
	printf("we here\n");
}

// Function to generate Diffie-Hellman key pair
EVP_PKEY* generate_dh_keypair(int bits) {
    EVP_PKEY_CTX *ctx;
    EVP_PKEY *params = NULL;
    
    // Create a new context for parameter generation
    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_DH, NULL);
    if (!ctx) {
        fprintf(stderr, "Error creating EVP_PKEY_CTX\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the parameter generation
    if (EVP_PKEY_paramgen_init(ctx) != 1) {
        fprintf(stderr, "Error initializing parameter generation\n");
        exit(EXIT_FAILURE);
    }

    // Set the Diffie-Hellman parameters
    if (EVP_PKEY_CTX_set_dh_paramgen_prime_len(ctx, bits) != 1) {
        fprintf(stderr, "Error setting DH parameters\n");
        exit(EXIT_FAILURE);
    }

    // Generate the parameters
    if (EVP_PKEY_paramgen(ctx, &params) != 1) {
        fprintf(stderr, "Error generating DH parameters\n");
        exit(EXIT_FAILURE);
    }

    // Create a new context for key generation
    EVP_PKEY_CTX *kctx;
    kctx = EVP_PKEY_CTX_new(params, NULL);
    if (!kctx) {
        fprintf(stderr, "Error creating EVP_PKEY_CTX for key generation\n");
        exit(EXIT_FAILURE);
    }

    EVP_PKEY *dh_key = NULL;

    // Generate the key
    if (EVP_PKEY_keygen_init(kctx) != 1 ||
        EVP_PKEY_keygen(kctx, &dh_key) != 1) {
        fprintf(stderr, "Error generating DH key\n");
        exit(EXIT_FAILURE);
    }

    EVP_PKEY_free(params);
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_CTX_free(kctx);

    return dh_key;
}

// Function to get the public key as a char array
char* get_public_key(EVP_PKEY *dh_key) {
    BIO *bio = BIO_new(BIO_s_mem());
    if (!bio) {
        fprintf(stderr, "Error creating BIO\n");
        exit(EXIT_FAILURE);
    }

    // Write public key to the BIO
    if (PEM_write_bio_PUBKEY(bio, dh_key) != 1) {
        fprintf(stderr, "Error writing public key to BIO\n");
        exit(EXIT_FAILURE);
    }

    char *public_key;
    long key_size = BIO_get_mem_data(bio, &public_key);
    char *result = malloc(key_size + 1);
    if (!result) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    // Copy public key from BIO to result
    memcpy(result, public_key, key_size);
    result[key_size] = '\0';

    BIO_free(bio);
    return result;
}
