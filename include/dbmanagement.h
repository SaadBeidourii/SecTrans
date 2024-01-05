#ifndef DBMANAGEMENT_H
#define DBMANAGEMENT_H

#include <sqlite3.h>


#define DBNAME "db.sqlite3"
#define DBPATH "./db.sqlite3"
#define USER_TABLE "users"
#define FILE_TABLE "files"
#define MAX_STRING_LENGTH 100

typedef struct {
  int id;
  char *username;
  char *password; char *role;
} User;

typedef struct {
  int id;
  char *filename;
  char *path;
  char *hash;
  int owner;
} File;

typedef struct {
  File *files;
  int size;
} FileList;

typedef struct {
  char **fileTitles;
  int size;
} TitleList;

sqlite3 *
db_open(char *db_name);
void db_close(sqlite3 *db);
int create_user_table(sqlite3 *db);
int create_file_table(sqlite3 *db);
int insert_user_into_table(sqlite3 *db, char *username, char *password,
                           char *role);
int insert_file_into_table(sqlite3 *db, char *name, char *path, char *hash,
                           int owner);
User *get_user_from_table(sqlite3 *db, char *username);
User *get_user_by_id(sqlite3 *db, int id);
File *get_file_form_table(sqlite3 *db, char filename[]);

int fill_test_data();
TitleList *get_file_list_from_table(sqlite3 *db);

#endif
