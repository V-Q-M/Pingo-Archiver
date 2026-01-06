CC      = gcc
CFLAGS  = -Wall -Wextra -Iheaders
LDFLAGS = -lncurses

TARGET  = pingoArchiver

SRC = src/main.c \
      src/ui.c \
      src/commands.c \
      src/fsHelpers.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean

