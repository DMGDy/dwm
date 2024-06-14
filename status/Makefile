CC = gcc
CFLAGS = -Wall -Wextra -g -Og
LDFLAGS = -liw -lX11 -lXrender -lXext -lpulse

TARGET = status
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

PREFIX = /usr/local
INSTALL_DIR = $(PREFIX)/bin

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

install: $(TARGET)
	mkdir -p $(INSTALL_DIR)
	cp $(TARGET) $(INSTALL_DIR)

uninstall:
	rm -f $(INSTALL_DIR)/$(TARGET)
