CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
TARGET = imgconv

SOURCES = main.c utilities/tools.c utilities/stb_impl.c utilities/converters.c utilities/writers.c
HEADERS = include/tools.h
OBJECTS = $(SOURCES:.c=.o)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lwebp -lm

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean
