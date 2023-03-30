all: bemu

.PHONY: all bemu bemu-release clean

CFLAGS = -Wall -g

CFLAGS_RELEASE = -Wall

BIN_DIR = ../bin/
LIB_DIR = ../lib/

SRC_PATH = src/
INCLUDE_DIRS = -Iinclude/

OUT_FILE = bemu

bemu:
	if [ ! -d $(BIN_DIR) ]; then mkdir -p $(BIN_DIR);fi
	gcc $(CFLAGS) $(INCLUDE_DIRS) $(SRC_PATH)MC14500.c $(SRC_PATH)instructions.c $(SRC_PATH)IODevice.c \
				$(SRC_PATH)main.c $(LIB_DIR)ulog.o -o $(BIN_DIR)$(OUT_FILE)


bemu-release:
	if [ ! -d $(BIN_DIR) ]; then mkdir -p $(BIN_DIR);fi
	gcc $(CFLAGS_RELEASE)  $(INCLUDE_DIRS) $(SRC_PATH)MC14500.c $(SRC_PATH)instructions.c $(SRC_PATH)IODevice.c \
				$(SRC_PATH)main.c $(LIB_DIR)ulog.o -o $(BIN_DIR)$(OUT_FILE)

clean:
	if [ -f $(BIN_DIR)$(OUT_FILE) ]; then rm $(BIN_DIR)$(OUT_FILE);fi
	
