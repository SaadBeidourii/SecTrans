#include "../include/dbmanagement.h"
#include "../include/server.h"
#include <fcntl.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_FILE_SIZE 10485760

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
  for (int i = 0; (i + 1) * 1024 < fileSize; i++) {
    //printf("i: %d\n", i * 1024);
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
	    printf("%x", message[j]);
    }
    printf("\n");

    memset(message, 0, sizeof(message));
  }
  printf("file recieved\n");
  /*
  for (int i = 0; i < fileSize; i++) {
    printf("%c", fileContent[i]);
  }
  */
  printf("THE FILE SIZE IS: %ld\n", sizeof(fileContent));

  createAndWriteToFile(fileName, fileContent, fileSize);
}

/**
 * the main function of our application server
 */
int main() {
  char firstMessage[2][1024];
  sqlite3 *db = db_open(DBPATH);
  if (startserver(3000) == 0) {
    printf("Server started successfully!\n");
    while (1) {
      char message[1024];
      getmsg(message);
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
