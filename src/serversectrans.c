#include <stdio.h>
#include "../include/server.h"

/**
 * the main function of our application server
 */
int main(){
    if(startserver(3000) == 0){
        printf("Server started successfully!\n");
        while(1){
            char message[1024];
            printf("%d\n", getmsg(message));
            printf("Received message: %s\n", message);
        }
    }
    return 0;
}
