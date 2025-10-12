CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = net.o

SOURCES = main.c \
          net/helpers.c \
          net/setup.c \
          net/modules/menu.c \
          net/modules/receiver.c \
          net/modules/sender.c \
          net/modules/packet_handler.c \
          net/modules/routing.c

OBJECTS = $(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) -lpthread

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean
