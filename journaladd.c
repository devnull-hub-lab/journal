#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_BUFFER_SIZE 1024

int main() {
    char user[MAX_BUFFER_SIZE];
    snprintf(user, sizeof(user), "%s", getenv("USER"));

    char journalPath[MAX_BUFFER_SIZE];
    snprintf(journalPath, sizeof(journalPath), "/home/%s/.journal", user);

    FILE *file = fopen(journalPath, "a");
    if (file == NULL) {
        perror("Error opening journal file");
        exit(1);
    }

    fprintf(file, "###############\n");

    // Get current date/time
    time_t currentTime = time(NULL);
    struct tm *localTime = localtime(&currentTime);

    // Format Date
    char date[MAX_BUFFER_SIZE];
    strftime(date, sizeof(date), "%m/%d/%Y", localTime);
    fprintf(file, "%s\n", date);

    // Formata Time
    char time[MAX_BUFFER_SIZE];
    strftime(time, sizeof(time), "%H:%M", localTime);
    fprintf(file, "%s\n", time);

    printf("Enter your journal entry (Press Ctrl+D to save and exit):\n");

    char buffer[MAX_BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        fprintf(file, "%s", buffer);
    }

    fclose(file);

    printf("Text added to the journal.\n");

    return 0;
}
