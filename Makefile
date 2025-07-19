CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC -Iinclude
LDFLAGS = -shared
EXAMPLE_LDFLAGS = -Wl,-rpath=$(BUILD_DIR) -L$(BUILD_DIR) -lplinux
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
EXAMPLE_DIR = example
LIB_NAME = libplinux.so

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

EXAMPLE_SRC = $(wildcard $(EXAMPLE_DIR)/*.c)
EXAMPLE_BINS = $(patsubst $(EXAMPLE_DIR)/%.c,$(EXAMPLE_DIR)/%,$(EXAMPLE_SRC))

all: $(BUILD_DIR)/$(LIB_NAME) $(EXAMPLE_BINS)

# Compilar objetos
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Criar a biblioteca compartilhada
$(BUILD_DIR)/$(LIB_NAME): $(OBJ_FILES)
	$(CC) $(LDFLAGS) -o $@ $^

# Compilar os exemplos linkando com a lib
$(EXAMPLE_DIR)/%: $(EXAMPLE_DIR)/%.c $(BUILD_DIR)/$(LIB_NAME)
	@mkdir -p $(EXAMPLE_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(EXAMPLE_LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
