#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include "iman.h"

#define BUFFER_SIZE 8192

void fetch_man_page(const char *command_name) {
    int sockfd;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
    char request[1024];

    // Create the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return;
    }
    
    // Get the server's DNS entry
    server = gethostbyname("man.he.net");
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        close(sockfd);
        return;
    }

    // Build the server's Internet address
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(80);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        perror("ERROR connecting");
        close(sockfd);
        return;
    }
    
    // Construct the HTTP request
    snprintf(request, sizeof(request), "GET /cgi-bin/man.cgi?topic=%s HTTP/1.1\r\nHost: man.he.net\r\nConnection: close\r\n\r\n", command_name);

    // Send the HTTP request to the server
    if (write(sockfd, request, strlen(request)) < 0) {
        perror("ERROR writing to socket");
        close(sockfd);
        return;
    }
    
    // Read and print the server's reply
    bzero(buffer, BUFFER_SIZE);
    int read_bytes;
    while ((read_bytes = read(sockfd, buffer, BUFFER_SIZE - 1)) > 0) {
        printf("%s", buffer);
        bzero(buffer, BUFFER_SIZE);
    }
    
    if (read_bytes < 0) {
        perror("ERROR reading from socket");
    }

    // Close the socket
    close(sockfd);
}

