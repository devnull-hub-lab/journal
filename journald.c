#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sqlite3.h>
#include <pwd.h>

#define PORT 2628
#define MAX_BUFFER_SIZE 1024
#define TIME_LIMIT 5
#define DB_CLIENTS_PATH "/var/db/journald/client_data.db"
#define DB_JOURNAL_PATH "/home/%s/.journal.db"

typedef struct {
    time_t lastConnectionTime;
} ClientData;

int main() {

    struct passwd *pwd;
    const char *username = "journal";
    short uid_journal = -1;

    pwd = getpwnam(username);
    if (pwd == NULL) {
        fprintf(stderr, "Failed to get user information\n");
        return 1;
    }

    uid_journal = pwd->pw_uid;

    if (setuid(uid_journal) != 0) {
        fprintf(stderr, "Please run as root\n");
        return 1;
    }

    endpwent(); // Free pwd

    // Set the effective user ID to the desired user (journal), so init system can manage the daemon start/stop
    if (setuid(uid_journal) == -1) {
        perror("setuid");
        return 1;
    }

    int sockfd, newsockfd;
    socklen_t clientLength;
    char buffer[MAX_BUFFER_SIZE];
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(1);
    }

    // SO_REUSEADDR option to reuse the address if it's in use
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Error setting socket option");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    if (listen(sockfd, 5) < 0) {
        perror("Error listening");
        exit(1);
    }

    printf("Daemon started...\n");
    printf("Listening on port 2628\n");

    // SQLite3 clients database initialization
    sqlite3 *db_clients;
    if (sqlite3_open(DB_CLIENTS_PATH, &db_clients) != SQLITE_OK) {
        perror("Error opening database");
        exit(1);
    }

    // Create the clients table if it doesn't exist
    char *createTableQuery = "CREATE TABLE IF NOT EXISTS clients (ip TEXT PRIMARY KEY, last_connection INTEGER)";
    if (sqlite3_exec(db_clients, createTableQuery, NULL, 0, NULL) != SQLITE_OK) {
        perror("Error creating table");
        exit(1);
    }

    while (1) {
        clientLength = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clientLength);
        if (newsockfd < 0) {
            perror("Error accepting connection");
            exit(1);
        }

        // Get the client IP address as a string
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cli_addr.sin_addr), clientIP, INET_ADDRSTRLEN);

        // Get the current time
        time_t currentTime = time(NULL);

        // Check if the client is allowed to connect based on the last connection time
        int isAllowed = 1;

        // SQLite3 query
        sqlite3_stmt *selectStmt;
        char *selectQuery = "SELECT last_connection FROM clients WHERE ip = ?";
        if (sqlite3_prepare_v2(db_clients, selectQuery, -1, &selectStmt, NULL) == SQLITE_OK) {
            sqlite3_bind_text(selectStmt, 1, clientIP, -1, SQLITE_STATIC);
            if (sqlite3_step(selectStmt) == SQLITE_ROW) {
                time_t lastConnectionTime = sqlite3_column_int(selectStmt, 0);
                if (currentTime - lastConnectionTime < TIME_LIMIT) {
                    isAllowed = 0;
                }
            }
            sqlite3_finalize(selectStmt);
        }

        // Log in server terminal
        if (!isAllowed) {
            printf("Connection from %s refused (Time limit: %d seconds)\n", clientIP, TIME_LIMIT);
            close(newsockfd);
            continue;
        }

        // Update the last connection time
        sqlite3_stmt *updateStmt;
        char *updateQuery = "INSERT OR REPLACE INTO clients (ip, last_connection) VALUES (?, ?)";
        if (sqlite3_prepare_v2(db_clients, updateQuery, -1, &updateStmt, NULL) == SQLITE_OK) {
            sqlite3_bind_text(updateStmt, 1, clientIP, -1, SQLITE_STATIC);
            sqlite3_bind_int(updateStmt, 2, currentTime);
            sqlite3_step(updateStmt);
            sqlite3_finalize(updateStmt);
        }

        memset(buffer, 0, sizeof(buffer));
        if (read(newsockfd, buffer, sizeof(buffer) - 1) < 0) {
            perror("Error reading client buffer");
            exit(1);
        }

        char user[MAX_BUFFER_SIZE], host[MAX_BUFFER_SIZE];
        sscanf(buffer, "%[^@]@%s", user, host);

        char journal_path[MAX_BUFFER_SIZE];
        snprintf(journal_path, sizeof(journal_path), DB_JOURNAL_PATH, user);

        sqlite3 *db_journal;
        if (sqlite3_open(journal_path, &db_journal) != SQLITE_OK) {
            char errorMessage[MAX_BUFFER_SIZE];
            snprintf(errorMessage, sizeof(errorMessage), "Journal not found for %s@%s", user, host);
            write(newsockfd, errorMessage, strlen(errorMessage));
            close(newsockfd);
            exit(1);
        }

        selectQuery = "SELECT entry_date, entry FROM tbentries;";
        if (sqlite3_prepare_v2(db_journal, selectQuery, -1, &selectStmt, NULL) == SQLITE_OK) {
            while (sqlite3_step(selectStmt) == SQLITE_ROW) {
                const unsigned char *entry_date = sqlite3_column_text(selectStmt, 0);
                const unsigned char *entry = sqlite3_column_text(selectStmt, 1);

                // Format string with:
                // ###############
                // <date> <time>
                // Entry Data
                char entry_data[MAX_BUFFER_SIZE];
                snprintf(entry_data, sizeof(entry_data), "###############\n%s\n%s\n", entry_date, entry);

                if (write(newsockfd, entry_data, strlen(entry_data)) < 0) {
                    perror("Error writing to client socket");
                    exit(1);
                }
            }
        }

        sqlite3_finalize(selectStmt);
        sqlite3_close(db_journal);

        close(newsockfd);
    }

    // Close the main socket
    close(sockfd);

    // Close the SQLite3 database
    sqlite3_close(db_clients);
    return 0;
}
