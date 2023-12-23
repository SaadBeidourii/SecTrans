#ifndef ENCRYPTION_H
#define ENCRYPTION_H

typedef struct {
    char *blocks[1024];
    char signature[1024];
} data;

void encrypt(uint64_t input, uint64_t *output, uint64_t key[3]);
void decrypt(uint64_t input, uint64_t *output, uint64_t key[3]);
void encrypt_file(char *input, char *output, char *key);
void decrypt_file(char *input, char *output, char *key);
void sign(char *input, char *output, char *key);

#endif