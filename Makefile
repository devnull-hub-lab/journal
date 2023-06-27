# Compiler
CC := gcc

# Compiler flags
CFLAGS := -Wall -Wextra -I/usr/include/sqlite3
LDFLAGS := -lsqlite3

# Source code files
SOURCE_JOURNALD := journal-daemon.c
SOURCE_JOURNALCLI := journal-cli.c

# Executable files
TARGET_JOURNALD := journal-daemon
TARGET_JOURNALCLI := journal

INSTALL_DIR_JOURNALD := /usr/local/sbin
INSTALL_DIR_JOURNALCLI := /usr/local/bin

# Default target
all: $(TARGET_JOURNALD) $(TARGET_JOURNALCLI)

# Build the targets
$(TARGET_JOURNALD): $(SOURCE_JOURNALD)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

$(TARGET_JOURNALCLI): $(SOURCE_JOURNALCLI)
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
	install -m 755 $(TARGET_JOURNALD) $(INSTALL_DIR_JOURNALD)
	install -m 755 $(TARGET_JOURNALCLI) $(INSTALL_DIR_JOURNALCLI)

# Clean generated files
clean:
	rm -f $(TARGET_JOURNALD) $(TARGET_JOURNALCLI)
	
