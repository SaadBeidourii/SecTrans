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

#define MAX_FILE_SIZE 10485760
#define DOCKER_SERVER_PORT_NUMBER 3000

/**
 * Calculates the SHA-256 hash of a string.
 *
 * @param input The string to hash.
 * @param outputBuffer The buffer to store the hash in.
 */
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

  for (unsigned int i = 0; i < hash_len; i++) {
    sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
  }

  outputBuffer[64] = '\0';
}

/**
 * Returns the file name from a path.
 *
 * @param path The path to the file.
 *
 * @return The file name.
 */
const char *getFileName(const char *path) {
  // Find the last occurrence of the directory separator "/"
  const char *lastSlash = strrchr(path, '/');

  // If no slash is found, the whole path is the file name
  if (lastSlash == NULL) {
    return path;
  }

  // Re3turn the portion of the path after the last slash
  return lastSlash + 1;
}

/**
 * Sends a file to the server.
 *
 * @param filePath The path to the file you want to send.
 *
 * @return 0 on success, 1 on failure.
 */
int send_file(char *filePath) {
  char *fileContent = (char *)malloc(MAX_FILE_SIZE * sizeof(char));
  char firstMessage[1024] = "up ";
  char hash[65];
  char buffer[1024];
  int size = 0;
  //
  // open the file
  int fd;
  fd = open(filePath, O_RDONLY);
  if (fd <= 0) {
    fprintf(stderr, "Couldn't open %s\n", filePath);
    free(fileContent);
    exit(1);
  }
  struct stat stat_data;
  if (fstat(fd, &stat_data) < 0) {
    fprintf(stderr, "Failed to stat %s: %s\n", filePath, strerror(errno));
    free(fileContent);
    exit(1);
  }
  if (stat_data.st_uid == 0) {
    fprintf(stderr, "File %s is owned by root\n", filePath);
    free(fileContent);
    exit(1);
  }

  int counter = 0;
  do {
    counter++;
    size = read(fd, buffer, 1024);
    strncat(fileContent, buffer, size);
    // sndmsg(buffer, DOCKER_SERVER_PORT_NUMBER);
    // write(1, buffer, size);
    memset(buffer, 0, 1024);
  } while (size > 0 && counter <= 1024);

  if (size > 0) {
    printf("maximum size is 10Mib\n");
    free(fileContent);
    return -1;
  }

  strncat(firstMessage, getFileName(filePath), MIN(99, strlen(filePath)));
  strcat(firstMessage, " ");
  sha256(fileContent, hash);
  strcat(firstMessage, hash);
  sndmsg(firstMessage, DOCKER_SERVER_PORT_NUMBER);
  sprintf(buffer, "%d", (int)strlen(fileContent));
  sndmsg(buffer, DOCKER_SERVER_PORT_NUMBER);
  for (int i = 0; i < counter - 1; i++) {
    memset(buffer, 0, 1024);
    memcpy(buffer, fileContent + (i * 1024), 1024);
    printf("%s\n", buffer);
    sndmsg(buffer, DOCKER_SERVER_PORT_NUMBER);
    sleep(1);
  }

  free(fileContent);
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
