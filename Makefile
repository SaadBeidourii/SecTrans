CC = gcc
CFLAGS = -Wall -Wextra -I./include
LDFLAGS = -L./lib -lclient -lserver -Wl,-rpath=./lib

# Directories
SRC_DIR = src
BIN_DIR = bin
LIB_DIR = lib
INCLUDE_DIR = include
OBJ_DIR = obj

# Files
SERVER_SRC = $(SRC_DIR)/serversectrans.c
CLIENT_SRC = $(SRC_DIR)/clientsectrans.c

SERVER_OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRC))
CLIENT_OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLIENT_SRC))

# Targets
all: server client

server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/server $^ $(LDFLAGS)

client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/client $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/server $(BIN_DIR)/client

.PHONY: all server client clean

