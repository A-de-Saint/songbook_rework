CC = gcc
CFLAGS = -Wall -Wextra -Werror -Iinclude

SRC_DIR = src
OBJ_DIR = obj
BIN = songbook
WIN_BIN = songbook.exe
LCURL_DLL = $(wildcard /mingw64/bin/libcurl-*.dll)

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all clean windows

all: $(BIN)

windows: $(WIN_BIN) cpy-deps

cpy-deps:
	ldd $(WIN_BIN) | grep mingw64 | awk '{print $$3}' | xargs -I{} cp {} .
	cp /mingw64/etc/ssl/certs/ca-bundle.crt .

$(WIN_BIN): $(OBJS)
	$(CC) $(OBJS) -static-libgcc -lcurl -o $@

$(BIN): $(OBJS)
	$(CC) $(OBJS) -lcurl -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN) *.dll *.crt