CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
TARGET = main
SRC = main.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: clean