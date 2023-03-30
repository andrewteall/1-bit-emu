all: ulog.c

.PHONY: all ulog-release clean

CC = gcc

LIB_FILE = ulog.o
CFLAGS = -Wall -c -g

CFLAGS_RELEASE = -Wall -c

LIB_DIR = ../lib/

SRC_PATH = src/
INCLUDE_DIRS = -Iinclude/

OUT_FILE = ulog.o

ulog.c:
	if [ ! -d $(LIB_DIR) ]; then mkdir -p $(LIB_DIR);fi
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(SRC_PATH)$@ -o $(LIB_DIR)$(OUT_FILE)

ulog-release:
	if [ ! -d $(LIB_DIR) ]; then mkdir -p $(LIB_DIR);fi
	$(CC) $(CFLAGS_RELEASE) $(INCLUDE_DIRS) $(SRC_PATH)ulog.c -o $(LIB_DIR)$(OUT_FILE)

clean:
	if [ -f $(LIB_DIR)$(LIB_FILE) ]; then rm -r $(LIB_DIR)$(LIB_FILE);fi
	
