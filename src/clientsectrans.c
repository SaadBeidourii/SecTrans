#include "../include/client.h"
#include <errno.h>
#include <fcntl.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

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

const char *getFileName(const char *path) {
  // Find the last occurrence of the directory separator "/"
  const char *lastSlash = strrchr(path, '/');

  // If no slash is found, the whole path is the file name
  if (lastSlash == NULL) {
    return path;
  }

  // Return the portion of the path after the last slash
  return lastSlash + 1;
}

int send_file(char *filePath) {
  char firstMessage[1024] = "up ";
  strncat(firstMessage, getFileName(filePath), MIN(99, strlen(filePath)));
  sndmsg(firstMessage, 4000);
  char buffer[1024];
  int size = 0;
  // open the file
  int fd;
  fd = open(filePath, O_RDONLY);
  if (fd <= 0) {
    fprintf(stderr, "Couldn't open %s\n", filePath);
    exit(1);
  }
  struct stat stat_data;
  if (fstat(fd, &stat_data) < 0) {
    fprintf(stderr, "Failed to stat %s: %s\n", filePath, strerror(errno));
    exit(1);
  }
  if (stat_data.st_uid == 0) {
    fprintf(stderr, "File %s is owned by root\n", filePath);
    exit(1);
  }

  do {
    size = read(fd, buffer, 1024);
    sndmsg(buffer, 4000);
    write(1, buffer, size);
    sleep(2);
    memset(buffer, 0, 1024);
  } while (size > 0);

  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "very few arguments\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Process command-line options
  if (strcmp(argv[1], "-up") == 0 && argc == 3) {
    send_file(argv[2]);
  } /*else if (strcmp(argv[1], "-list") == 0 && argc == 2) {
      listFunction();
  } else if (strcmp(argv[1], "-down") == 0 && argc == 3) {
      downloadFunction(argv[2]);
  }*/
  else {
    fprintf(stderr, "Invalid command-line options\n");
  }

  return EXIT_SUCCESS;
}
