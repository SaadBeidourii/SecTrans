CC := gcc
CFLAGS := -Wall -Wextra -I./include

SRC_DIR := src
LIB_DIR := lib

SERVER_SRC := $(SRC_DIR)/server_impl.c
CLIENT_SRC := $(SRC_DIR)/client.c

SERVER_OBJ := $(patsubst $(SRC_DIR)/%.c, $(SRC_DIR)/%.o, $(SERVER_SRC))
CLIENT_OBJ := $(patsubst $(SRC_DIR)/%.c, $(SRC_DIR)/%.o, $(CLIENT_SRC))

SERVER_BIN := server
CLIENT_BIN := client

LIBRARY_FILES := $(LIB_DIR)/libserver.so $(LIB_DIR)/libclient.so

.PHONY: all clean

all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJ) -L$(LIB_DIR) -lserver

$(CLIENT_BIN): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJ) -L$(LIB_DIR) -lclient

$(SERVER_OBJ): $(SERVER_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(CLIENT_OBJ): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o
	rm -f $(SERVER_BIN) $(CLIENT_BIN)

