CC = gcc
CFLAGS = -Wall -g -Wextra -I./include
LDFLAGS = -L./lib -lclient -lserver -lsqlite3 -lcrypto -lssl -Wl,-rpath=./lib
LDFLAGSCLIENT = -L./lib -lclient -lserver  -lcrypto -lssl -Wl,-rpath=./lib

# Directories
SRC_DIR = src
BIN_DIR = bin
LIB_DIR = lib
INCLUDE_DIR = include
OBJ_DIR = obj

# Files
SERVER_SRC = $(SRC_DIR)/serversectrans.c
CLIENT_SRC = $(SRC_DIR)/clientsectrans.c
INITDB_SRC = $(SRC_DIR)/initdb.c
DBMANAGEMENT_SRC = $(SRC_DIR)/dbmanagement.c
FAKE_SRC = $(SRC_DIR)/fake.c

SERVER_OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRC))
CLIENT_OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLIENT_SRC))
INITDB_OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(INITDB_SRC))
DBMANAGEMENT_OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(DBMANAGEMENT_SRC))
FAKE_OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(FAKE_SRC))

# Targets
all: server client initdb

server: $(SERVER_OBJ) $(DBMANAGEMENT_OBJ)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/server $^ $(LDFLAGS)

client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/client $^ $(LDFLAGSCLIENT)

initdb: $(INITDB_OBJ) $(DBMANAGEMENT_OBJ)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/initdb $^ $(LDFLAGS)

fake: $(FAKE_OBJ)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/fake $^ $(LDFLAGSCLIENT)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/server $(BIN_DIR)/client

.PHONY: all server client clean

