CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
AR = ar
ARFLAGS = rcs

SRC_DIR = src
EXAMPLE_DIR = example
BUILD_DIR = build
LIB_NAME = libplinux.a

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))
EXAMPLE_SRC = $(EXAMPLE_DIR)/main.c
EXAMPLE_BIN = $(EXAMPLE_DIR)/example

all: $(BUILD_DIR)/$(LIB_NAME) $(EXAMPLE_BIN)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(LIB_NAME): $(OBJ_FILES)
	$(AR) $(ARFLAGS) $@ $^

$(EXAMPLE_BIN): $(EXAMPLE_SRC) $(BUILD_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) $< -L$(BUILD_DIR) -lplinux -o $@

clean:
	rm -rf $(BUILD_DIR) $(EXAMPLE_BIN)

.PHONY: all clean