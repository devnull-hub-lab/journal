#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>

#define MAX_BUFFER_SIZE 1024

int main() {
    char user[MAX_BUFFER_SIZE];
    snprintf(user, sizeof(user), "%s", getenv("USER"));

    char dbPath[MAX_BUFFER_SIZE];
    snprintf(dbPath, sizeof(dbPath), "/home/%s/.journal.db", user);

    sqlite3 *db;
    int rc = sqlite3_open(dbPath, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char *createTableSQL = "CREATE TABLE IF NOT EXISTS tbentries (seq INTEGER, subseq INTEGER, entry TEXT, PRIMARY KEY (seq, subseq));";
    rc = sqlite3_exec(db, createTableSQL, NULL, 0, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char *selectSeqSQL = "SELECT MAX(seq) FROM tbentries;";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, selectSeqSQL, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    int seq = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        seq = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    seq++;

    printf("Enter your journal entry (Press <return> and Ctrl+D to save and exit):\n");

    char buffer[MAX_BUFFER_SIZE];
    time_t currentTime = time(NULL);
    struct tm *timeInfo = localtime(&currentTime);
    char dateTime[MAX_BUFFER_SIZE];
    strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M:%S", timeInfo);

    int subseq = 1; // each line in a subseq from a inclusion

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        char *insertSQL = sqlite3_mprintf("INSERT INTO tbentries (seq, subseq, entry) VALUES (%d, %d, '%q');", seq, subseq, buffer);
        rc = sqlite3_exec(db, insertSQL, NULL, 0, NULL);
        sqlite3_free(insertSQL);

        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            exit(1);
        }
        subseq++;
    }

    sqlite3_close(db);

    printf("Text added to the journal.\n");

    return 0;
}
