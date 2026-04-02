CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
TARGET = imgconv
DEB_TARGETS = debian/.debhelper debian/imgconv debian/debhelper-build-stamp debian/imgconv.substvars debian/files
SOURCES = main.c utilities/tools.c utilities/stb_impl.c utilities/converters.c utilities/writers.c
HEADERS = include/tools.h

# Build directory
BUILDDIR = build

# Convert source paths into build/*.o paths
OBJECTS = $(patsubst %.c,$(BUILDDIR)/%.o,$(SOURCES))

# Default target
$(TARGET): $(BUILDDIR) $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lwebp -lm

# Rule to create build directory if missing
$(BUILDDIR):
	mkdir -p $(BUILDDIR)/utilities

# Pattern rule for compiling .c → build/.o
$(BUILDDIR)/%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
windows:
	x86_64-w64-mingw32-gcc $(CFLAGS) -DNO_WEBP -o imgconv.exe $(SOURCES) -lm
clean:
	rm -rf $(BUILDDIR) $(TARGET) $(DEB_TARGETS) imgconv.exe
install:
	mkdir -p $(PREFIX)/bin
	cp imgconv $(PREFIX)/bin/

.PHONY: clean