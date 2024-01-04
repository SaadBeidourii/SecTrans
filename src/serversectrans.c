#include "../include/dbmanagement.h"
#include "../include/server.h"
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

/**
 * the main function of our application server
 */
int main() {
  if (startserver(3000) == 0) {
    printf("Server started successfully!\n");
    while (1) {
      char message[1024];
      getmsg(message);
      printf("%s\n", message);
      memset(message, 0, sizeof(message));
    }
  }
  return 0;
}
