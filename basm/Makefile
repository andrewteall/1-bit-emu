all: basm

.PHONY: all  basm-release clean

CFLAGS = -Wall -g -lncurses

CFLAGS_RELEASE = -Wall -lncurses

BIN_DIR = ../bin/
LIB_DIR = ../lib/

SRC_PATH = src/
INCLUDE_DIRS = -Iinclude/ -I../ulog/include/
TEST_PATH = test/

OUT_FILE = basm

basm:
	if [ ! -d $(BIN_DIR) ]; then mkdir -p $(BIN_DIR);fi
	gcc $(CFLAGS)  $(INCLUDE_DIRS) $(SRC_PATH)main.c $(SRC_PATH)basm.c $(SRC_PATH)parser.c \
				$(SRC_PATH)lexer.c $(SRC_PATH)utils.c $(SRC_PATH)token.c $(LIB_DIR)ulog.o -o $(BIN_DIR)$(OUT_FILE)

basm-release:
	if [ ! -d $(BIN_DIR) ]; then mkdir -p $(BIN_DIR);fi
	gcc $(CFLAGS_RELEASE)  $(INCLUDE_DIRS) $(SRC_PATH)main.c $(SRC_PATH)basm.c $(SRC_PATH)parser.c \
				$(SRC_PATH)lexer.c $(LIB_DIR)ulog.o -o $(BIN_DIR)$(OUT_FILE)

lexer_test:
	gcc $(CFLAGS) $(INCLUDE_DIRS) $(TEST_PATH)$@.c $(SRC_PATH)lexer.c $(SRC_PATH)basm.c $(LIB_DIR)ulog.o -o $(TEST_PATH)$@
	chmod +x $(TEST_PATH)$@
	./$(TEST_PATH)$@
	rm $(TEST_PATH)$@

clean:
	if [ -f $(BIN_DIR)$(OUT_FILE) ]; then rm $(BIN_DIR)$(OUT_FILE);fi
	
