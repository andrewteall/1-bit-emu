################################################################################
# Global Variables
################################################################################
# Compiler
CC := gcc
# CC := clang

# Output Targets
TARGET_EXEC := bemu

# Libraries needed for Linker
LIBRARIES := ncurses m

# Folder Paths
BIN_DIR := ../bin/
LIB_DIR := ../lib/
SRC_DIR := src/
INC_DIR := include/ ../ulog/include/
BUILD_DIR := build/

# These libs are needed to run(the former) and to build (the latter)
# Not used in current Makefile
DEPS := libgpiod2 libgpiod-dev

################################################################################
# Environment Variables
################################################################################
# PREFIX is environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

################################################################################
# Flags
################################################################################
# Define Include Flags
INC_FLAGS := $(addprefix -I,$(INC_DIR))

# Define Linker Flags
LDFLAGS := $(addprefix -l,$(LIBRARIES))

# Define Compiler Flags
CFLAGS := $(INC_FLAGS) -Wall

################################################################################
# Source and Object Variables
################################################################################
# Define all the C files we want to compile
SRCS := $(shell find $(SRC_DIR) -maxdepth 1 -name '*.c' -printf '%f\n')

# Define Output objects from Src Groups. 
# Convert *.c names to *.o and prepend the build path
OBJS := $(subst .c.o,.o,$(SRCS:%=$(BUILD_DIR)$(SRC_DIR)%.o))

################################################################################
# Object Build
################################################################################
# Object step for C source bin
$(BUILD_DIR)$(SRC_DIR)%.o: $(SRC_DIR)%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $^ -o $@

################################################################################
# Target Build
################################################################################
.PHONY: all
all: bemu

.PHONY: bemu
bemu: $(BIN_DIR)$(TARGET_EXEC)

# Bin Build
$(BIN_DIR)$(TARGET_EXEC): $(OBJS)
		mkdir -p $(BIN_DIR)
		$(CC) -o $(BIN_DIR)$(TARGET_EXEC) $(OBJS) $(LIB_DIR)ulog.o $(LDFLAGS)

# bemu:
# 	if [ ! -d $(BIN_DIR) ]; then mkdir -p $(BIN_DIR);fi
# 	gcc $(CFLAGS) $(INCLUDE_DIRS) $(SRC_PATH)main.c $(SRC_PATH)bemu.c $(SRC_PATH)MC14500.c  $(SRC_PATH)IODevice.c \
# 				$(SRC_PATH)MCSystem.c $(SRC_PATH)MCUtils.c $(LIB_DIR)ulog.o \
# 				-o $(BIN_DIR)$(OUT_FILE) -lncurses

clean:
	if [ -d $(BUILD_DIR) ]; then rm -r $(BUILD_DIR);fi
	if [ -f $(BIN_DIR)$(OUT_FILE) ]; then rm $(BIN_DIR)$(OUT_FILE);fi
	
