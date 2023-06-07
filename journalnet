#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 2628
#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s user@host\n", argv[0]);
        exit(1);
    }

    char user[MAX_BUFFER_SIZE], host[MAX_BUFFER_SIZE];
    sscanf(argv[1], "%[^@]@%s", user, host);

    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[MAX_BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error on open socket");
        exit(1);
    }

    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr, "Error, host '%s' not found\n", host);
        exit(1);
    }

    //set server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error on estabilishing connection");
        exit(1);
    }

    //sending param to server
    snprintf(buffer, sizeof(buffer), "%s@%s", user, host);
    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        perror("Error on sending data to server");
        exit(1);
    }

    //reading server data
    memset(buffer, 0, sizeof(buffer));
    while ((n = read(sockfd, buffer, sizeof(buffer) - 1)) > 0) {
        printf("%s", buffer);
        memset(buffer, 0, sizeof(buffer));
    }
    if (n < 0) {
        perror("Error on reading server data");
        exit(1);
    }

    close(sockfd);

    return 0;
}
