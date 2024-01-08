#include "../include/client.h"
#include "../include/dbmanagement.h"
#include "../include/server.h"
#include <errno.h>
#include <fcntl.h>
#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FILE_SIZE 10485760

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
/*
// Function to decode Base64-encoded string to binary data
unsigned char* base64_decode(const char* encoded_data,size_t input_length,
size_t* output_length) { static const char base64_table[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    if (input_length % 4 != 0) {
        fprintf(stderr, "Invalid Base64-encoded string\n");
        exit(EXIT_FAILURE);
    }

    *output_length = input_length / 4 * 3;

    if (encoded_data[input_length - 1] == '=') {
        (*output_length)--;
    }
    if (encoded_data[input_length - 2] == '=') {
        (*output_length)--;
    }

    unsigned char* decoded_data = (unsigned char*)malloc(*output_length);
    if (!decoded_data) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    size_t i, j;
    for (i = 0, j = 0; i < input_length; ) {
        uint32_t sextet_a = encoded_data[i] == '=' ? 0 & i++ :
base64_table[(unsigned char)encoded_data[i++]]; uint32_t sextet_b =
encoded_data[i] == '=' ? 0 & i++ : base64_table[(unsigned
char)encoded_data[i++]]; uint32_t sextet_c = encoded_data[i] == '=' ? 0 & i++ :
base64_table[(unsigned char)encoded_data[i++]]; uint32_t sextet_d =
encoded_data[i] == '=' ? 0 & i++ : base64_table[(unsigned
char)encoded_data[i++]];

        uint32_t triple = (sextet_a << 18) | (sextet_b << 12) | (sextet_c << 6)
| sextet_d;

        if (j < *output_length) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = triple & 0xFF;
    }

    return decoded_data;
}
*/
void build_decoding_table() {

  decoding_table = malloc(256);

  for (int i = 0; i < 64; i++)
    decoding_table[(unsigned char)encoding_table[i]] = i;
}

unsigned char *base64_decode(const char *data, size_t input_length,
                             size_t *output_length) {

  if (decoding_table == NULL)
    build_decoding_table();

  if (input_length % 4 != 0)
    return NULL;

  *output_length = input_length / 4 * 3;
  if (data[input_length - 1] == '=')
    (*output_length)--;
  if (data[input_length - 2] == '=')
    (*output_length)--;

  unsigned char *decoded_data = malloc(*output_length);
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

void base64_cleanup() { free(decoding_table); }
void createAndWriteToFile(const char *fileName, const char *fileContent,
                          int fileSize) {
  // Define the directory where the file will be saved
  const char *directory = "./files/";

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

/*
// Function to split a string into two tokens based on spaces
void splitString(const char *input, char token1[1024], char token2[1024]) {
  // Use strtok to get the first token
  char *firstToken = strtok((char *)input, " ");
  if (firstToken != NULL) {
    strcpy(token1, firstToken);

    // Use strtok to get the second token
    char *secondToken = strtok(NULL, " ");
    if (secondToken != NULL) {
      strcpy(token2, secondToken);
    } else {
      // If there's no second token, set the second token to an empty string
      token2[0] = '\0';
    }
  } else {
    // If there's no first token, set both tokens to an empty string
    token1[0] = '\0';
    token2[0] = '\0';
  }
}
*/

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
/*
⠀⢸⠂⠀⠀⠀⠘⣧⠀⠀⣟⠛⠲⢤⡀⠀⠀⣰⠏⠀⠀⠀⠀⠀⢹⡀
⠀⡿⠀⠀⠀⠀⠀⠈⢷⡀⢻⡀⠀⠀⠙⢦⣰⠏⠀⠀⠀⠀⠀⠀⢸⠀
⠀⡇⠀⠀⠀⠀⠀⠀⢀⣻⠞⠛⠀⠀⠀⠀⠻⠀⠀⠀⠀⠀⠀⠀⢸⠀
⠀⡇⠀⠀⠀⠀⠀⠀⠛⠓⠒⠓⠓⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⠀
⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⠀
⠀⢿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣀⣀⣀⠀⠀⢀⡟⠀
⠀⠘⣇⠀⠘⣿⠋⢹⠛⣿⡇⠀⠀⠀⠀⣿⣿⡇⠀⢳⠉⠀⣠⡾⠁⠀
⣦⣤⣽⣆⢀⡇⠀⢸⡇⣾⡇⠀⠀⠀⠀⣿⣿⡷⠀⢸⡇⠐⠛⠛⣿⠀
⠹⣦⠀⠀⠸⡇⠀⠸⣿⡿⠁⢀⡀⠀⠀⠿⠿⠃⠀⢸⠇⠀⢀⡾⠁⠀
⠀⠈⡿⢠⢶⣡⡄⠀⠀⠀⠀⠉⠁⠀⠀⠀⠀⠀⣴⣧⠆⠀⢻⡄⠀⠀
⠀⢸⠃⠀⠘⠉⠀⠀⠀⠠⣄⡴⠲⠶⠴⠃⠀⠀⠀⠉⡀⠀⠀⢻⡄⠀
⠀⠘⠒⠒⠻⢦⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣤⠞⠛⠒⠛⠋⠁⠀
⠀⠀⠀⠀⠀⠀⠸⣟⠓⠒⠂⠀⠀⠀⠀⠀⠈⢷⡀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠙⣦⠀⠀⠀⠀⠀⠀⠀⠀⠈⢷⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⣼⣃⡀⠀⠀⠀⠀⠀⠀⠀⠀⠘⣆⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠉⣹⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⡿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡆⠀⠀⠀⠀⠀
*/

void recieveFile(char *fileName, char *hash, User user) {
  int counter = 0;
  printf("currently recieving file %s\n", fileName);
  char *fileContent = (char *)malloc(MAX_FILE_SIZE * sizeof(char));
  /*
  for(int i = 0; i < 1024; i++) {
    char message[1024];
    getmsg(message);
    strncat(fileContent, message, 1024);
  }
  */

  char message[1024];
  getmsg(message);
  int fileSize = atoi(message);
  printf("file size: %d\n", fileSize);
  memset(message, 0, sizeof(message));
  for (int i = 0; (i) * 1024 < fileSize; i++) {
    printf("i: %d\n", i * 1024);
    size_t currentLength = strlen(fileContent);
    getmsg(message);
    /*
    for (int j = 0; j < 1024; j++) {
      fileContent[counter] = message[j];
      counter++;
    }
    */
    // strcpy(fileContent + currentLength, message);
    strncat(fileContent, message, 1024);
    // printf("%x\n", *message);
    for (int j = 0; j < 1024; j++) {
      printf("%c", message[j]);
    }
    printf("\n");

    memset(message, 0, 1024);
  }
  printf("file recieved\n");
  /*
  for (int i = 0; i < fileSize; i++) {
    printf("%c", fileContent[i]);
  }
  */
  // printf("THE FILE SIZE IS: %ld\n", sizeof(fileContent));
  size_t decodedSize = 0;
  unsigned char *decodedFile =
      base64_decode(fileContent, fileSize, &decodedSize);

  createAndWriteToFile(fileName, decodedFile, decodedSize);
  sqlite3 *db = db_open(DBPATH);
  char path[100] = "./files/";
  strcat(path, fileName);
  insert_file_into_table(db, fileName, path, hash, user.id);
  db_close(db);
}

void getFileList(char *port, User *user) {
  char buffer[1024];
  memset(buffer, 0, 1024);
  sqlite3 *db = db_open(DBPATH);
  TitleList *titles = query_files_for_user(db, *user);
  for (int i = 0; i < titles->size; i++) {
    printf("%s\n", titles->fileTitles[i]);
    strcat(buffer, titles->fileTitles[i]);
    strcat(buffer, "\n");
  }
  db_close(db);
  free_title_list(titles);
  int portNumber = atoi(port);
  sndmsg(buffer, portNumber);
}

// Function to copy file content to a char array
char *copyFileContent(char *fileName, int *size) {
  char buffer[1024] = "./files/";
  strncat(buffer, fileName, 100);
  int fileDescriptor = open(buffer, O_RDONLY);

  if (fileDescriptor <= 0) {
    fprintf(stderr, "Couldn't open %s\n", fileName);
    exit(1);
  }
  struct stat stat_data;
  if (fstat(fileDescriptor, &stat_data) < 0) {
    fprintf(stderr, "Failed to stat %s: %s\n", fileName, strerror(errno));
    exit(1);
  }
  if (stat_data.st_uid == 0) {
    fprintf(stderr, "File %s is owned by root\n", fileName);
    exit(1);
  }
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

void sendFile(char *portNumber, char *fileName, User user) {
  sqlite3 *db = db_open(DBPATH);
  printf("recieved file name is :%s\n", fileName);
  File *file = get_file_from_table(db, fileName);
  printf("%s\n", file->filename);
  User *fileOwner = get_user_by_id(db, file->owner);
  printf("owner id is %d\n", fileOwner->id);
  db_close(db);


  if (strcmp(fileOwner->role, "admin") == 0 &&
      (strcmp(user.role, "manager") == 0 ||
       strcmp(user.role, "employee") == 0)) {
    printf("NOT ALLOWED");
    return;
  }

  printf("after first if\n");
  if (strcmp(fileOwner->role, "manager") == 0 &&
      strcmp(user.role, "employee") == 0) {
    printf("NOT ALLOWED");
    return;
  }

  int size = 0;
  char *fileContent = copyFileContent(fileName, &size);
  size_t encodedSize = 0;
  char *encodedContent =
      base64_encode((unsigned char *)fileContent, size, &encodedSize);
  free(fileContent);
  char message[1024];
  sprintf(message, "%ld", encodedSize);
  strcat(message, " ");
  strcat(message, file->hash);
  sndmsg(message, atoi(portNumber));
  memset(message, 0, 1024);
  for (int i = 0; (i) * 1024 < encodedSize; i++) {
    printf("i: %d\n", i * 1024);
    size_t currentLength = strlen(encodedContent);
    memcpy(message, encodedContent + i * 1024, 1024);
    sndmsg(message, atoi(portNumber));
    memset(message, 0, 1024);
  }
  printf("file sent\n");
  free(fileOwner);
  free(file);
  free(encodedContent);
}

/**
 * the main function of our application server
 */
int main() {
  char firstMessage[3][1024];
  if (startserver(3000) == 0) {
    printf("Server started successfully!\n");
    char *message = (char *)malloc(1024 * sizeof(char));
    memset(message, 0, 1024);
    while (1) {
      memset(message, 0, 1024);
      getmsg(message);
      /*
      size_t sizeafter = 0;
      unsigned char *elmisoj =
          base64_decode(message, strlen(message), &sizeafter);
          */
      splitString(message, firstMessage[0], firstMessage[1], firstMessage[2]);
      printf("%s\n", firstMessage[0]);
      User *user = malloc(sizeof(User));
      sqlite3 *db = db_open(DBPATH);
      user = get_user_from_table(db, firstMessage[0]);
      db_close(db);
      if (user != NULL && strcmp(user->password, firstMessage[1]) == 0) {
        getmsg(message);
        printf("the first message my sir is %s\n", message);
        splitString(message, firstMessage[0], firstMessage[1], firstMessage[2]);
        if (memcmp(firstMessage[0], "up", 3) == 0) {
          recieveFile(firstMessage[1], firstMessage[2], *user);
        } else if (memcmp(firstMessage[0], "list", 5) == 0) {
          getFileList(firstMessage[1], user);
        } else if (memcmp(firstMessage[0], "down", 5) == 0) {
          sendFile(firstMessage[1], firstMessage[2], *user);
        }
        memset(message, 0, 1024);
      }
    }
  }
  return 0;
}
