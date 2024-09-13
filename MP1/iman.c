#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define BUFFER_SIZE 8192
#define REQUEST_SIZE 2048  // Increased size to handle longer strings

void fetch_man_page(const char *command_name) {
    int sockfd;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
    char request[REQUEST_SIZE];  // Increase the size of the request buffer

    // Step 1: Hostname and path formatting
    const char *hostname = "man.he.net";
    char path_format[1024];

    // Build the path with the command_name
    snprintf(path_format, sizeof(path_format), "/?topic=%s&section=all", command_name);

    // Step 2: Create the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return;
    }

    // Step 3: Get the server's DNS entry
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        close(sockfd);
        return;
    }

    // Step 4: Build the server's Internet address
    memset(&serveraddr, 0, sizeof(serveraddr));  // Zero out the server address
    serveraddr.sin_family = AF_INET;
    memcpy(&serveraddr.sin_addr.s_addr, server->h_addr, server->h_length);
    serveraddr.sin_port = htons(80);  // Connect to port 80 for HTTP

    // Step 5: Connect to the server
    if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        perror("ERROR connecting");
        close(sockfd);
        return;
    }

    // Step 6: Construct the HTTP GET request using the formatted path
    memset(request, 0, sizeof(request));  // Zero out the request buffer
    snprintf(request, sizeof(request), 
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n", path_format, hostname);

    // Step 7: Send the HTTP request to the server
    if (write(sockfd, request, strlen(request)) < 0) {
        perror("ERROR writing to socket");
        close(sockfd);
        return;
    }

    // Step 8: Read and print the server's reply
    memset(buffer, 0, sizeof(buffer));  // Clear the buffer
    int read_bytes;
    while ((read_bytes = read(sockfd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[read_bytes] = '\0';  // Null-terminate the response
        printf("%s", buffer);       // Print the content
        memset(buffer, 0, sizeof(buffer));  // Clear buffer for the next read
    }

    if (read_bytes < 0) {
        perror("ERROR reading from socket");
    }

    // Step 9: Close the socket
    close(sockfd);
}
