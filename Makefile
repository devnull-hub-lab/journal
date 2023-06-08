# Compiler
CC := gcc

#Compiler flags
CFLAGS := -Wall -Wextra -I/usr/include/sqlite3
LDFLAGS := -lsqlite3

# Source code file
SOURCE := journald.c

# Executable file
TARGET := journald

INSTALL_DIR = /usr/local/sbin

# Default target
all: $(TARGET)

# Build the target
$(TARGET): $(SOURCE)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

install:
	@if [ $$(id -u) -ne 0 ]; then \
		echo "You must run 'make install' as root (UID 0) to install the program."; \
		exit 1; \
	fi

	@useradd -m journal
	@usermod -s /sbin/nologin journal
	install -m 755 $(TARGET) $(INSTALL_DIR)

# Clean generated files
clean:
	rm -f $(TARGET)
  
