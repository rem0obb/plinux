CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude -Ilibraries/plthook/include
AR = ar
ARFLAGS = rcs

SRC_DIR = src
EXAMPLE_DIR = example
BUILD_DIR = build
PLTHOOK_DIR = libraries/plthook
PLTHOOK_SRC_DIR = $(PLTHOOK_DIR)/src
PLTHOOK_BUILD_DIR = $(BUILD_DIR)/plthook
LIB_NAME = libplinux.a
PLTHOOK_LIB = libplthook.a

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

PLTHOOK_SRC_FILES = $(wildcard $(PLTHOOK_SRC_DIR)/*.c)
PLTHOOK_OBJ_FILES = $(patsubst $(PLTHOOK_SRC_DIR)/%.c,$(PLTHOOK_BUILD_DIR)/%.o,$(PLTHOOK_SRC_FILES))

EXAMPLE_SRC = $(EXAMPLE_DIR)/main.c
EXAMPLE_BIN = $(EXAMPLE_DIR)/example

all: $(BUILD_DIR)/$(LIB_NAME) $(PLTHOOK_BUILD_DIR)/$(PLTHOOK_LIB) $(EXAMPLE_BIN)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(PLTHOOK_BUILD_DIR)/%.o: $(PLTHOOK_SRC_DIR)/%.c
	@mkdir -p $(PLTHOOK_BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(LIB_NAME): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $@ $^

$(PLTHOOK_BUILD_DIR)/$(PLTHOOK_LIB): $(PLTHOOK_OBJ_FILES)
	$(AR) $(ARFLAGS) $@ $^

$(EXAMPLE_BIN): $(EXAMPLE_SRC) $(BUILD_DIR)/$(LIB_NAME) $(PLTHOOK_BUILD_DIR)/$(PLTHOOK_LIB)
	$(CC) $(CFLAGS) $< -L$(BUILD_DIR) -L$(PLTHOOK_BUILD_DIR) -lplinux -lplthook -o $@

clean:
	rm -rf $(BUILD_DIR) $(EXAMPLE_BIN)

.PHONY: all clean
