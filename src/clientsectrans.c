#include <stdio.h>
#include <openssl/evp.h>
#include <stdlib.h>
#include <string.h>
#include "../include/client.h"

void sha256(const char *input, char outputBuffer[65]) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;

    OpenSSL_add_all_digests();

    md = EVP_get_digestbyname("sha256");
    if (!md) {
        fprintf(stderr, "Error: SHA-256 not supported.\n");
        exit(EXIT_FAILURE);
    }

    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, input, strlen(input));
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);

    for (int i = 0; i < hash_len; i++) {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }

    outputBuffer[64] = '\0';
}

int main(){
    char outputBuffer[65];
    sha256("Hello World!", outputBuffer);
    printf("%d\n", sndmsg("Hello World!", 3000));
    printf("%d\n", sndmsg(outputBuffer, 3000));
    return 0;
}
