#include "../include/dbmanagement.h"
#include "../include/server.h"
#include <fcntl.h>
#include <sqlite3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void recieveFile(char *fileName) {
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
}

/**
 * the main function of our application server
 */
int main() {
  char firstMessage[2][1024];
  sqlite3 *db = db_open(DBPATH);
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
      splitString(message, firstMessage[0], firstMessage[1]);
      printf("%s\n", firstMessage[0]);
      User *user = malloc(sizeof(User));
      user = get_user_from_table(db, firstMessage[0]);
      if (user != NULL && strcmp(user->password, firstMessage[1]) == 0) {
        printf("we in the if muthafucka\n");
        getmsg(message);
        splitString(message, firstMessage[0], firstMessage[1]);
        if (memcmp(firstMessage[0], "up", 3) == 0) {
          recieveFile(firstMessage[1]);
        }
        memset(message, 0, sizeof(message));
      }
    }
  }
  return 0;
}
