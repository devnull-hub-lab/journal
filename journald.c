#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 2628
#define MAX_BUFFER_SIZE 1024

int main() {
    int sockfd, newsockfd;
    socklen_t clientLength;
    char buffer[MAX_BUFFER_SIZE];
    struct sockaddr_in serv_addr, cli_addr;
    FILE *file;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error on open socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error to bind socket");
        exit(1);
    }

    if (listen(sockfd, 5) < 0) {
        perror("Error on listennig");
        exit(1);
    }

    printf("Daemon started...\n");

    //Every con
    while (1) {
        clientLength = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clientLength);
        if (newsockfd < 0) {
            perror("Error on receiving connection");
            exit(1);
        }

        memset(buffer, 0, sizeof(buffer));
        if (read(newsockfd, buffer, sizeof(buffer) - 1) < 0) {
            perror("Error on reading buffer client");
            exit(1);
        }

        char user[MAX_BUFFER_SIZE], host[MAX_BUFFER_SIZE];
        sscanf(buffer, "%[^@]@%s", user, host);

        char path[MAX_BUFFER_SIZE];
        snprintf(path, sizeof(path), "/home/%s/.journal", user);

        file = fopen(path, "r");
        if (file == NULL) {
            perror("Error on reading .journal user file");
            exit(1);
        }

        //sending buffer to client
        memset(buffer, 0, sizeof(buffer));
        char fileContent[MAX_BUFFER_SIZE];
        while (fgets(fileContent, sizeof(fileContent), file) != NULL) {
            strcat(buffer, fileContent);
        }
        write(newsockfd, buffer, strlen(buffer));

        fclose(file);
        close(newsockfd);
    }
    //close main socket
    close(sockfd);

    return 0;
}
