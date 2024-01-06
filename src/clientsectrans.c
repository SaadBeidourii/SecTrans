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
#define BUFFER_SIZE 1024

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
 * Authenticates the user.
 */
void authenticate() {
  printf("Enter your username: ");
  char username[100];
  scanf("%s", username);
  printf("Enter your password: ");
  char password[100];
  scanf("%s", password);
  char message[1024];
  strcpy(message, username);
  strcat(message, " ");
  strcat(message, password);
  sndmsg(message, DOCKER_SERVER_PORT_NUMBER);
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

// Function to copy file content to a char array
char *copyFileContent(int fileDescriptor, int *size) {
  // Seek to the end of the file to determine its size
  off_t fileSize = lseek(fileDescriptor, 0, SEEK_END);

  *size = fileSize;
  // Check if seeking was successful
  if (fileSize == -1) {
    perror("Error seeking file");
    return NULL;
  }

  // Allocate memory for the char array
  char *fileContent = (char *)malloc(fileSize);

  // Check if memory allocation was successful
  if (fileContent == NULL) {
    perror("Error allocating memory");
    return NULL;
  }

  // Seek back to the beginning of the file
  if (lseek(fileDescriptor, 0, SEEK_SET) == -1) {
    perror("Error seeking file");
    free(fileContent);
    return NULL;
  }

  // Read the file content into the char array
  ssize_t bytesRead = read(fileDescriptor, fileContent, fileSize);
  printf("read this much : %ld\n", bytesRead);

  // Check if reading was successful
  if (bytesRead == -1) {
    perror("Error reading file");
    free(fileContent);
    return NULL;
  }

  return fileContent;
}

int readFile(int fd, char **buffer, size_t *size) {
  // Initialize buffer and size
  *buffer = NULL;
  *size = 0;

  // Temporary buffer to read file contents in chunks
  char tempBuffer[BUFFER_SIZE];
  ssize_t bytesRead;

  // Loop to read the file in chunks
  while ((bytesRead = read(fd, tempBuffer, sizeof(tempBuffer))) > 0 &&
         bytesRead <= MAX_FILE_SIZE) {
    // Resize the buffer to accommodate the new data
    *buffer = realloc(*buffer, *size + bytesRead);

    // Check if realloc was successful
    if (*buffer == NULL) {
      perror("Error reallocating buffer");
      return -1;
    }

    // Copy the new data to the end of the buffer
    memcpy(*buffer + *size, tempBuffer, bytesRead);

    // Update the size
    *size += bytesRead;
  }

  // Check for read errors
  if (bytesRead < 0) {
    perror("Error reading file");
    return -1;
  }

  if (bytesRead > MAX_FILE_SIZE) {
    printf("File size is too big\n");
    return -1;
  }

  return 0; // Success
}

/**
 * Sends a file to the server.
 *
 * @param filePath The path to the file you want to send.
 *
 * @return 0 on success, 1 on failure.
 */
int send_file(char *filePath) {
  char *fileContent;
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

  /*
  int counter = 0;
  do {
    counter++;
    size = read(fd, buffer, 1024);
    strncat(fileContent, buffer, size);
    // sndmsg(buffer, DOCKER_SERVER_PORT_NUMBER);
    // write(1, buffer, size);
    memset(buffer, 0, 1024);
  } while (size > 0 && counter <= 1024);
  */
  // readFile(fd, &fileContent, &size);
  fileContent = copyFileContent(fd, &size);

  // close the file
  close(fd);

  authenticate();

  strncat(firstMessage, getFileName(filePath), MIN(99, strlen(filePath)));
  strcat(firstMessage, " ");
  sha256(fileContent, hash);
  strcat(firstMessage, hash);
  sndmsg(firstMessage, DOCKER_SERVER_PORT_NUMBER);
  memset(buffer, 0, BUFFER_SIZE);
  sprintf(buffer, "%d", size);
  sndmsg(buffer, DOCKER_SERVER_PORT_NUMBER);
  for (int i = 0; i < 2; i++) {
    memset(buffer, 0, 1024);
    memcpy(buffer, fileContent + (i * 1024), 1024);
    for (int j = 0; j < 1024; j++) {
      printf("%x", buffer[j]);
    }
    sndmsg(buffer, DOCKER_SERVER_PORT_NUMBER);
    // sleep(1);
  }

  printf("file sent\n");
  /*
  for (int i = 0; i < size; i++) {
    printf("%c", fileContent[i]);
  }
  */
  free(fileContent);
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "very few arguments %s\n", argv[0]);
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
