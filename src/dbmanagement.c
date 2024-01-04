#include "../include/dbmanagement.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <time.h>

char *white_listed_user_names[] = {"mohamed", "ahmed",  "ali",
                                   "omar",    "khaled", "abdellah"};

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
    switch (rc) {
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
              "hash TEXT NOT NULL,"
              "owner INTEGER NOT NULL,"
              "FOREIGN KEY(owner) REFERENCES users(id));";

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

  /*
  char *sql = malloc(strlen(username) + strlen(password) + 100);
  sprintf(
      sql,
      "INSERT INTO users (username, password, role) VALUES ('%s', '%s', '%s');",
      username, password, role);
 */

  char sql[] = "INSERT INTO users (username, password, role) VALUES (?, ?, ?);";
  sqlite3_stmt *stmt;
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    sqlite3_free(zErrMsg);
    return -1;
  }

  sqlite3_bind_text(stmt, 1, username, MIN(strlen(username), MAX_STRING_LENGTH),
                    SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, password, MIN(strlen(password), MAX_STRING_LENGTH),
                    SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, role, MIN(strlen(role), MAX_STRING_LENGTH),
                    SQLITE_STATIC);

  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    return rc;
  }
  printf("Inserted User\n");
  sqlite3_finalize(stmt);

  return 0;
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

  /*
  char *sql = malloc(strlen(name) + strlen(path) + strlen(hash) + 100);
  sprintf(sql,
          "INSERT INTO files (name, path, hash, owner) VALUES ('%s', '%s', "
          "'%s', %d);",
          name, path, hash, owner);
  */
  char sql[] =
      "INSERT INTO files (name, path, hash, owner) VALUES (?, ?, ?, ?)";

  sqlite3_stmt *stmt;
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    sqlite3_free(zErrMsg);
    return -1;
  }

  sqlite3_bind_text(stmt, 1, name, MIN(strlen(name), MAX_STRING_LENGTH),
                    SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, path, MIN(strlen(path), MAX_STRING_LENGTH),
                    SQLITE_STATIC);
  sqlite3_bind_text(stmt, 3, hash, MIN(strlen(hash), MAX_STRING_LENGTH),
                    SQLITE_STATIC);
  sqlite3_bind_int(stmt, 4, owner);

  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    return rc;
  }

  sqlite3_finalize(stmt);
  printf("inserted file\n");

  return 0;
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
  sqlite3_finalize(res);

  if (rc == SQLITE_ROW) {
    user->id = sqlite3_column_int(res, 0);
    user->username = malloc(strlen(sqlite3_column_text(res, 1)) + 1);
    strncpy(user->username, sqlite3_column_text(res, 1),
            MIN(strlen(sqlite3_column_text(res, 1)), MAX_STRING_LENGTH));
    user->password = malloc(strlen(sqlite3_column_text(res, 2)) + 1);
    strncpy(user->password, sqlite3_column_text(res, 2),
            MIN(strlen(sqlite3_column_text(res, 2)), MAX_STRING_LENGTH));
    user->role = malloc(strlen(sqlite3_column_text(res, 3)) + 1);
    strncpy(user->role, sqlite3_column_text(res, 3),
            MIN(strlen(sqlite3_column_text(res, 3)), MAX_STRING_LENGTH));
  }

  free(sql);
  return user;
}

User *get_user_by_id(sqlite3 *db, int id) {
  char *zErrMsg = 0;
  int rc;
  sqlite3_stmt *res;

  char *sql = malloc(104);
  sprintf(sql, "SELECT * FROM users WHERE id = '%d';", id);

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
    strncpy(user->username, sqlite3_column_text(res, 1),
            MIN(strlen(sqlite3_column_text(res, 1)) + 1, MAX_STRING_LENGTH));
    user->password = malloc(strlen(sqlite3_column_text(res, 2)) + 1);
    strncpy(user->password, sqlite3_column_text(res, 2),
            MIN(strlen(sqlite3_column_text(res, 2)) + 1, MAX_STRING_LENGTH));
    user->role = malloc(strlen(sqlite3_column_text(res, 3)) + 1);
    strncpy(user->role, sqlite3_column_text(res, 3),
            MIN(strlen(sqlite3_column_text(res, 3)) + 1, MAX_STRING_LENGTH));
  }

  sqlite3_finalize(res);
  free(sql);
  return user;
}

/**
 * @brief get_file_from_table
 * @param db
 * @param filename
 * @return File instance or NULL if error
 */
File *get_file_from_table(sqlite3 *db, char filename[]) {
  char *zErrMsg = 0;
  int rc;
  sqlite3_stmt *res;

  /*
  char *sql = malloc(strlen(filename) + 100);
  sprintf(sql, "SELECT * FROM files WHERE name = '%s';", filename);
  */
  char sql[] = "SELECT * FROM files WHERE name = ?";

  rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    sqlite3_free(zErrMsg);
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
    strncpy(file->filename, sqlite3_column_text(res, 1),
            MIN(strlen(sqlite3_column_text(res, 1)) + 1, MAX_STRING_LENGTH));
    file->path = malloc(strlen(sqlite3_column_text(res, 2)) + 1);
    strncpy(file->path, sqlite3_column_text(res, 2),
            MIN(strlen(sqlite3_column_text(res, 2)) + 1, MAX_STRING_LENGTH));
    file->hash = malloc(strlen(sqlite3_column_text(res, 3)) + 1);
    strncpy(file->hash, sqlite3_column_text(res, 3),
            MIN(strlen(sqlite3_column_text(res, 3)) + 1, MAX_STRING_LENGTH));
  }

  sqlite3_finalize(res);
  return file;
}

/**
 * @brief get_file_list_from_table
 * @param db
 * @return TitleList instance or NULL if error
 */
TitleList *get_file_list_from_table(sqlite3 *db) {
  // get the files from the database
  // allocate memory for the list
  // fill the list with the files
  TitleList *titleList = NULL;
  sqlite3_stmt *stmt;
  const char *countSql =
      "SELECT COUNT(*) FROM files"; // Replace with your actual table name
  const char *selectSql =
      "SELECT name FROM files"; // Replace with your actual table name
  int rc;

  // Prepare the statement to get the count of titles
  rc = sqlite3_prepare_v2(db, countSql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Cannot prepare count statement: %s\n", sqlite3_errmsg(db));
    return NULL;
  }

  // Execute the count statement
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_ROW) {
    fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return NULL;
  }

  // Get the count of titles
  int titleCount = sqlite3_column_int(stmt, 0);

  // Finalize the count statement
  sqlite3_finalize(stmt);

  // Allocate memory for titleList
  titleList = (TitleList *)malloc(sizeof(TitleList));
  if (titleList == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    return NULL;
  }

  // Initialize titleList
  titleList->fileTitles = (char **)malloc((titleCount) * sizeof(char *));
  if (titleList->fileTitles == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    free(titleList);
    return NULL;
  }
  titleList->size = titleCount;

  // Prepare the statement to get the titles
  rc = sqlite3_prepare_v2(db, selectSql, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Cannot prepare select statement: %s\n",
            sqlite3_errmsg(db));
    free(titleList->fileTitles);
    free(titleList);
    return NULL;
  }

  printf("titleCount: %d\n", titleCount);
  // Fetch data and fill the titleList
  for (int i = 0;
       (i < (titleCount) + 1) && (rc = sqlite3_step(stmt)) == SQLITE_ROW; ++i) {
    const char *title = (const char *)sqlite3_column_text(stmt, 0);

    printf("the index is %d and i am inserting %s\n", i, title);
    // Allocate memory for the title
    titleList->fileTitles[i] = strdup(title);
    if (titleList->fileTitles[i] == NULL) {
      fprintf(stderr, "Memory allocation error\n");
      free(titleList->fileTitles);
      free(titleList);
      sqlite3_finalize(stmt);
      return NULL;
    }
  }

  // Check if there was an error or if the loop ended successfully
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    free(titleList->fileTitles);
    free(titleList);
    sqlite3_finalize(stmt);
    return NULL;
  }

  // Finalize the statement
  sqlite3_finalize(stmt);

  return titleList; // return the list
}

/**
 * @brief fill in test data
 */
int fill_test_data() {
  printf("FILLING TEST DATA!\n");
  sqlite3 *db = db_open(DBPATH);
  srand(time(NULL));

  if (db == NULL) {
    return -1;
  }

  sqlite3_exec(db, "DROP TABLE IF EXISTS users;", NULL, 0, NULL);
  sqlite3_exec(db, "DROP TABLE IF EXISTS files;", NULL, 0, NULL);
  create_user_table(db);
  create_file_table(db);

  insert_user_into_table(db, "admin", "admin", "admin");
  insert_file_into_table(db, "el filo", "el ziko", "el pico", 1);
  int i;
  for (i = 0; i < 6; i++) {
    int random = rand() % 2;
    char *username = malloc(20);
    char *password = malloc(20);
    char *role = malloc(20);
    sprintf(username, "%s", white_listed_user_names[i]);
    sprintf(password, "%s", white_listed_user_names[i]);
    sprintf(role, "%s", roles[random]);
    insert_user_into_table(db, username, password, role);
    insert_file_into_table(db, "el filo", "el ziko", "el pico", i + 1);
    free(username);
    free(password);
    free(role);
  }
  db_close(db);
  return 0;
}
