#include <stdio.h>
#include <stdlib.h>
#include "../include/server.h"

int main() {
    char* messages[200];
    int i = 0;

    if (startserver(3000) == 0) {
        printf("Server started successfully!\n");
        while (1) {
            char* message = malloc(1024);  // Allocate memory for the message
            if (getmsg(message) == 0) {
                free(message);
                break;  // Handle error or exit loop when no more messages
            }
            messages[i] = message;
            i++;
            printf("Received message: %s\n", message);
        }
    }

    // Free dynamically allocated memory
    for (int j = 0; j < i; j++) {
        free(messages[j]);
    }

    return 0;
}
