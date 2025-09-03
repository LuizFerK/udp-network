CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = net.o
SOURCES = main.c net/helpers.c net/setup.c net/modules/menu.c
OBJECTS = $(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean
