#include "../include/client.h"
#include "../include/server.h"
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

static char encoding_table[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

char *base64_encode(const unsigned char *data, size_t input_length,
                    size_t *output_length) {

  *output_length = 4 * ((input_length + 2) / 3);

  char *encoded_data = malloc(*output_length);
  if (encoded_data == NULL)
    return NULL;

  for (int i = 0, j = 0; i < input_length;) {

    uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
    uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
    uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

    uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
  }

  for (int i = 0; i < mod_table[input_length % 3]; i++)
    encoded_data[*output_length - 1 - i] = '=';

  return encoded_data;
}
void base64_cleanup() { free(decoding_table); }

void build_decoding_table() {

  decoding_table = malloc(64 * sizeof(int));
  printf("error here ?\n");

  for (int i = 0; i < 64; i++)
    decoding_table[(unsigned char)encoding_table[i]] = i;
}

unsigned char *base64_decode(const char *data, size_t input_length,
                             size_t *output_length) {

  if (decoding_table == NULL) {
	  printf("here\n");
	  build_decoding_table();
	  printf("here\n");
  }

  if (input_length % 4 != 0)
    return NULL;

  *output_length = input_length / 4 * 3;

  if (data[input_length - 1] == '=')
    (*output_length)--;
  if (data[input_length - 2] == '=')
    (*output_length)--;

  printf("the output length is %ld\n", *output_length);
  printf("el data\n");
  unsigned char *decoded_data = malloc(*output_length);
  printf("el data\n");
  if (decoded_data == NULL)
    return NULL;

  for (int i = 0, j = 0; i < input_length;) {

    uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
    uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
    uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
    uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

    uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) +
                      (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

    if (j < *output_length)
      decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
    if (j < *output_length)
      decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
    if (j < *output_length)
      decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
  }

  return decoded_data;
}

void createAndWriteToFile(const char *fileName, const char *fileContent,
                          int fileSize) {
  // Define the directory where the file will be saved
  const char *directory = "./";

  // Concatenate the directory and file name to get the full path
  char fullPath[256]; // Adjust the size as needed
  snprintf(fullPath, sizeof(fullPath), "%s%s", directory, fileName);
  printf("Full Path: %s\n", fullPath);

  // Open the file in write mode
  //  FILE *file = fopen(fullPath, "w");
  int fd = open(fullPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    perror("Error opening PNG file for writing");
    exit(1);
  }
  // Write the content to the file
  // fprintf(file, "%s", fileContent);
  // memcpy(file, fileContent, fileSize);
  write(fd, fileContent, fileSize);

  // Close the file
  close(fd);

  printf("File %s created and written successfully.\n", fullPath);
}

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
  fgets(username, 99, stdin);
  printf("Enter your password: ");
  char password[100];
  fgets(password, 99, stdin);
  char message[1024];
  printf("username: %s\n", username);
  printf("password: %s\n", password);
  username[strlen(username) - 1] = '\0';
  password[strlen(password) - 1] = '\0';
  strcpy(message, username);
  strcat(message, " ");
  strcat(message, password);
  printf("message: %s\n", message);
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

void splitString(const char *input, char token1[1024], char token2[1024],
                 char token3[1024]) {
  memset(token1, 0, 1024);
  memset(token2, 0, 1024);
  memset(token3, 0, 1024);
  // Use strtok to get the first token
  char *firstToken = strtok((char *)input, " ");
  if (firstToken != NULL) {
    strcpy(token1, firstToken);

    // Use strtok to get the second token
    char *secondToken = strtok(NULL, " ");
    if (secondToken != NULL) {
      strcpy(token2, secondToken);

      // Use strtok to get the third token
      char *thirdToken = strtok(NULL, " ");
      if (thirdToken != NULL) {
        strcpy(token3, thirdToken);
      } else {
        // If there's no third token, set the third token to an empty string
        token3[0] = '\0';
      }
    } else {
      // If there's no second token, set both the second and third tokens to an
      // empty string
      token2[0] = '\0';
      token3[0] = '\0';
    }
  } else {
    // If there's no first token, set all tokens to an empty string
    token1[0] = '\0';
    token2[0] = '\0';
    token3[0] = '\0';
  }
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
  size_t realSizelhrba = 0;
  char *ELMISAJ =
      base64_encode((unsigned char *)fileContent, size, &realSizelhrba);
  memset(buffer, 0, BUFFER_SIZE);
  sprintf(buffer, "%04d", realSizelhrba);
  sndmsg(buffer, DOCKER_SERVER_PORT_NUMBER);
  for (int i = 0; i < (realSizelhrba / BUFFER_SIZE) + 1; i++) {
    memset(buffer, 0, 1024);
    memcpy(buffer, ELMISAJ + (i * 1024), 1024);
    for (int j = 0; j < 1024; j++) {
      printf("%c", buffer[j]);
    }
    printf("\n");
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

int getFileList() {
  char buffer[1024] = "list ";
  char portBuffer[15];
  authenticate();
  int i = 0;
  do {
    i++;
  } while (startserver(DOCKER_SERVER_PORT_NUMBER + i) != 0);
  sprintf(portBuffer, "%d", DOCKER_SERVER_PORT_NUMBER + i);
  strcat(buffer, portBuffer);
  sndmsg(buffer, DOCKER_SERVER_PORT_NUMBER);
  memset(buffer, 0, 1024);
  getmsg(buffer);
  printf("%s\n", buffer);
  stopserver();
  return 0;
}

int downloadFile(char *fileName) {
  char buffer[1024] = "down ";
  char firstMessage[3][1024];
  char portBuffer[5];
  authenticate();
  int i = 0;
  do {
    i++;
  } while (startserver(DOCKER_SERVER_PORT_NUMBER + i) != 0);
  sprintf(portBuffer, "%d", DOCKER_SERVER_PORT_NUMBER + i);
  strcat(buffer, portBuffer);
  strcat(buffer, " ");
  strcat(buffer, fileName);
  sndmsg(buffer, DOCKER_SERVER_PORT_NUMBER);
  memset(buffer, 0, 1024);
  getmsg(buffer);
  splitString(buffer, firstMessage[0],firstMessage[1] , firstMessage[2]);
  int encodedSize = atoi(firstMessage[0]);
  char hash[65];
  strncpy(hash, firstMessage[1], 64);
  hash[64] = '\0';
  printf("encodedSize %d\n", encodedSize);
  char *encodedContent = calloc(encodedSize, sizeof(char));

  for (int i = 0; i * 1024 < encodedSize ; i++) {
    memset(buffer, 0, 1023);
    getmsg(buffer);
    printf("recieved : %s\n", buffer);
    strncat(encodedContent, buffer, 1024);
  }
  // printf("encodedContent %x\n", encodedContent);
  size_t decodedSize = 0;
  unsigned char *decodedContent =
      base64_decode(encodedContent, encodedSize, &decodedSize);
  free(encodedContent);
  char calculatedHash[65];
  sha256(decodedContent, calculatedHash);
  if(strcmp(calculatedHash, hash) != 0){
	  printf("the hashes of the files are different, the recieved file is corrupted!\n");
	  return -1;
  }
  createAndWriteToFile(fileName, (char *)decodedContent, decodedSize);
  free(decodedContent);
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
  } else if (strcmp(argv[1], "-list") == 0 && argc == 2) {
    getFileList();
  } else if (strcmp(argv[1], "-down") == 0 && argc == 3) {
    if (strlen(argv[2]) > 100) {
      printf("the file name can't be over 100 characters long\n");
      return -1;
    }
    downloadFile(argv[2]);
  } else {
    fprintf(stderr, "Invalid command-line options\n");
  }

  return EXIT_SUCCESS;
}
