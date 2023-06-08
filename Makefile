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

	@id -u journal >/dev/null 2>&1 || useradd -m journal
	@grep -q "journal:/sbin/nologin" /etc/passwd || usermod -s /sbin/nologin journal
	@mkdir -p /var/db/journald/
	@chown journal:journal /var/db/journald/
	install -m 755 $(TARGET) $(INSTALL_DIR)

# Clean generated files
clean:
	rm -f $(TARGET)
  
