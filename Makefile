CC      = gcc
CFLAGS  = -Wall -Wextra -Iheaders 
LDFLAGS = -lncurses

TARGET  = pingoArchiver
BUILD_DIR = build

SRC = src/main.c \
      src/ui.c \
      src/commands.c \
      src/fsHelpers.c

OBJ = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC))

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean

