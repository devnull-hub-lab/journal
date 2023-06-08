A daemon that reads .journal user file

Dependencies to build journald: sqlite3

Instructions to build daemon: gcc journald.c -o journald -lsqlite3

Run Daemon: ./journald
