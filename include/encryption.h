#ifndef ENCRYPTION_H
#define ENCRYPTION_H

typedef struct {
    char *blocks[1024];
    char signature[1024];
} data;

void encrypt(char *input, char *output, char *key);
void decrypt(char *input, char *output, char *key);
void encrypt_file(char *input, char *output, char *key);
void decrypt_file(char *input, char *output, char *key);
void sign(char *input, char *output, char *key);

#endif