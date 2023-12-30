#include <stdio.h>
#include "../include/server.h"
#include "../include/dbmanagement.h"
#include <sqlite3.h>

/**
 * the main function of our application server
 */
int main(){
    User *user;
    TitleList *eltitro;
    sqlite3 *db = db_open(DBPATH);

    eltitro = get_file_list_from_table(db);
    for(int i = 1; i<=7; i++){
	    user = get_user_by_id(db,i);
	    printf("User: %s\n", user->username);
	    printf("title: %s\n", eltitro->fileTitles[i-1]);
    }

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
