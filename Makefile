# Compiler
CC := gcc

# Compiler flags
CFLAGS := -Wall -Wextra -I/usr/include/sqlite3
LDFLAGS := -lsqlite3

# Source code files
SOURCE_JOURNALD := journald.c
SOURCE_JOURNALADD := journaladd.c

# Executable files
TARGET_JOURNALD := journald
TARGET_JOURNALADD := journaladd

INSTALL_DIR_JOURNALD := /usr/local/sbin
INSTALL_DIR_JOURNALADD := /usr/local/bin

# Default target
all: $(TARGET_JOURNALD) $(TARGET_JOURNALADD)

# Build the targets
$(TARGET_JOURNALD): $(SOURCE_JOURNALD)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

$(TARGET_JOURNALADD): $(SOURCE_JOURNALADD)
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
	install -m 755 $(TARGET_JOURNALADD) $(INSTALL_DIR_JOURNALADD)

# Clean generated files
clean:
	rm -f $(TARGET_JOURNALD) $(TARGET_JOURNALADD)
	
