CC = gcc
CFLAGS = -Wall -Wextra -std=c99 $(shell ncurses-config --cflags)
# CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200112L
TARGET = net.o

SOURCES = main.c \
          net/helpers.c \
          net/setup.c \
          net/ncurses.c \
          net/modules/menu.c \
          net/modules/receiver.c \
          net/modules/sender.c \
          net/modules/packet_handler.c \
          net/modules/routing.c

OBJECTS = $(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) -lpthread $(shell ncurses-config --libs)
# 	$(CC) $(OBJECTS) -o $(TARGET) -lpthread -lncurses

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean
