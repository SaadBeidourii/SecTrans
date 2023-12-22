#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/dbmanagement.h"

char white_listed_first_named[10][5] = {"john",  "jane",  "joe",   "jill",
                                        "jack",  "james", "jenny", "josh",
                                        "julie", "jim"};
char white_listed_last_named[10][8] = {
    "smith",    "doe",   "jones", "jackson", "johnson",
    "williams", "brown", "davis", "miller",  "wilson"};

char roles[3][10] = {"manager", "employee", "admin"};

/**
 * @brief db_open
 * @param db_name
 * @return sqlite3 instance or NULL if error
 */
sqlite3 *db_open(char *db_name) {
  sqlite3 *db;
  int rc;
  printf("db_name: %s\n", db_name);

  rc = sqlite3_open(db_name, &db);
  if (rc) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        switch(rc) {
            case SQLITE_ERROR:
                fprintf(stderr, "SQLITE_ERROR: Generic error\n");
                break;
            case SQLITE_INTERNAL:
                fprintf(stderr, "SQLITE_INTERNAL: Internal logic error in SQLite\n");
                break;
            // Add more cases for other potential error codes
            default:
                fprintf(stderr, "Unknown error code: %d\n", rc);
        }
    return NULL;
  } else {
    fprintf(stderr, "Opened database successfully\n");
    return db;
  }
}

/**
 * @brief db_close
 * @param db
 */
void db_close(sqlite3 *db) { sqlite3_close(db); }

/**
 * @brief create_user_table
 * @param db
 * @return 0 if success, -1 if error
 */
int create_user_table(sqlite3 *db) {
  char *zErrMsg = 0;
  int rc;

  char *sql =
      "CREATE TABLE users("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "username TEXT NOT NULL,"
      "password TEXT NOT NULL,"
      "role TEXT CHECK (role IN ('admin', 'manager', 'employee')) NOT NULL);";

  rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  } else {
    fprintf(stdout, "Table created successfully\n");
    return 0;
  }
}

/**
 * @brief create_file_table
 * @param db
 * @return 0 if success, -1 if error
 */
int create_file_table(sqlite3 *db) {
  char *zErrMsg = 0;
  int rc;

  char *sql = "CREATE TABLE files("
              "id INTEGER PRIMARY KEY AUTOINCREMENT,"
              "name TEXT NOT NULL,"
              "path TEXT NOT NULL,"
              "hash TEXT NOT NULL);";

  rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  } else {
    fprintf(stdout, "Table created successfully\n");
    return 0;
  }
}

/**
 * @brief insert_user_into_table
 * @param db
 * @param username
 * @param password
 * @param role
 * @return 0 if success, -1 if error
 */
int insert_user_into_table(sqlite3 *db, char *username, char *password,
                           char *role) {
  char *zErrMsg = 0;
  int rc;

  char *sql = malloc(strlen(username) + strlen(password) + 100);
  sprintf(
      sql,
      "INSERT INTO users (username, password, role) VALUES ('%s', '%s', '%s');",
      username, password, role);

  rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    free(sql);
    return -1;
  } else {
    fprintf(stdout, "User inserted successfully\n");
    free(sql);
    return 0;
  }
}

/**
 * @brief insert_file_into_table
 * @param db
 * @param name
 * @param path
 * @param hash
 * @param owner
 * @return 0 if success, -1 if error
 */
int insert_file_into_table(sqlite3 *db, char *name, char *path, char *hash,
                           int owner) {
  char *zErrMsg = 0;
  int rc;

  char *sql = malloc(strlen(name) + strlen(path) + strlen(hash) + 100);
  sprintf(sql,
          "INSERT INTO files (name, path, hash, owner) VALUES ('%s', '%s', "
          "'%s', %d);",
          name, path, hash, owner);

  rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    free(sql);
    return -1;
  } else {
    fprintf(stdout, "File inserted successfully\n");
    free(sql);
    return 0;
  }
}

/**
 * @brief get_user_from_table
 * @param db
 * @param username
 * @return User instance or NULL if error
 */
User *get_user_from_table(sqlite3 *db, char *username) {
  char *zErrMsg = 0;
  int rc;
  sqlite3_stmt *res;

  char *sql = malloc(strlen(username) + 100);
  sprintf(sql, "SELECT * FROM users WHERE username = '%s';", username);

  rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    sqlite3_free(zErrMsg);
    free(sql);
    return NULL;
  }

  User *user = malloc(sizeof(User));
  user->id = -1;
  user->username = NULL;
  user->password = NULL;
  user->role = NULL;

  rc = sqlite3_step(res);
  if (rc == SQLITE_ROW) {
    user->id = sqlite3_column_int(res, 0);
    user->username = malloc(strlen(sqlite3_column_text(res, 1)) + 1);
    strcpy(user->username, sqlite3_column_text(res, 1));
    user->password = malloc(strlen(sqlite3_column_text(res, 2)) + 1);
    strcpy(user->password, sqlite3_column_text(res, 2));
    user->role = malloc(strlen(sqlite3_column_text(res, 3)) + 1);
    strcpy(user->role, sqlite3_column_text(res, 3));
  }

  sqlite3_finalize(res);
  free(sql);
  return user;
}

/**
 * @brief get_file_form_table
 * @param db
 * @param filename
 * @return File instance or NULL if error
 */
File *get_file_form_table(sqlite3 *db, char filename[]) {
  char *zErrMsg = 0;
  int rc;
  sqlite3_stmt *res;

  char *sql = malloc(strlen(filename) + 100);
  sprintf(sql, "SELECT * FROM files WHERE name = '%s';", filename);

  rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    sqlite3_free(zErrMsg);
    free(sql);
    return NULL;
  }

  File *file = malloc(sizeof(File));
  file->id = -1;
  file->filename = NULL;
  file->path = NULL;
  file->hash = NULL;

  rc = sqlite3_step(res);
  if (rc == SQLITE_ROW) {
    file->id = sqlite3_column_int(res, 0);
    file->filename = malloc(strlen(sqlite3_column_text(res, 1)) + 1);
    strcpy(file->filename, sqlite3_column_text(res, 1));
    file->path = malloc(strlen(sqlite3_column_text(res, 2)) + 1);
    strcpy(file->path, sqlite3_column_text(res, 2));
    file->hash = malloc(strlen(sqlite3_column_text(res, 3)) + 1);
    strcpy(file->hash, sqlite3_column_text(res, 3));
  }

  sqlite3_finalize(res);
  free(sql);
  return file;
}

/**
 * @brief get_file_list_from_table
 * @param db
 * @return FileList instance or NULL if error
 */
int fill_test_data() {
  sqlite3 *db = db_open(DBPATH);
  srand(time(NULL));

  if (db == NULL) {
      printf("l7ma9\n");
    return -1;
  }

  sqlite3_exec(db, "DROP TABLE IF EXISTS users;", NULL, 0, NULL);
  sqlite3_exec(db, "DROP TABLE IF EXISTS files;", NULL, 0, NULL);
  create_user_table(db);
  create_file_table(db);

  int i;
  for (i = 0; i < 5; i++) {
    int random = rand() % 2;
    char *username = malloc(20);
    char *password = malloc(20);
    char *role = malloc(20);
    sprintf(username, "%s%s", white_listed_first_named[i],
            white_listed_last_named[i]);
    sprintf(password, "%s%s", white_listed_first_named[i],
            white_listed_last_named[i]);
    sprintf(role, "%s", roles[random]);
    insert_user_into_table(db, username, password, role);
    free(username);
    free(password);
    free(role);
  }
  db_close(db);
  return 0;
}

