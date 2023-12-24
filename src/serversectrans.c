#include <stdio.h>
#include "../include/server.h"
#include "../include/dbmanagement.h"
#include <sqlite3.h>

/**
 * the main function of our application server
 */
int main(){
    User *user;
    sqlite3 *db = db_open(DBPATH);
    user = get_user_by_id(db, 1);
    printf("User: %s\n", user->username);
    user = get_user_from_table(db, "ahmed");
    printf("User: %d\n", user->id);
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
