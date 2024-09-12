#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h> // for fcntl and non-blocking
#include "iman.h"

#define BUFFER_SIZE 4096

// Helper function to skip the HTTP headers
char *skip_http_headers(char *response) {
    char *header_end = strstr(response, "\r\n\r\n");
    if (header_end) {
        return header_end + 4;  // Skip over the header
    }
    return NULL;
}

void execute_iman(char *args[]) {
    if (args[1] == NULL) {
        printf("Usage: iMan <command>\n");
        return;
    }

    char *command = args[1];

    // Establish socket connection
    int sockfd;
    struct sockaddr_in server_addr;
    char send_buffer[BUFFER_SIZE], recv_buffer[BUFFER_SIZE];

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return;
    }

    // Setup server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80);  // HTTP port
    if (inet_pton(AF_INET, "72.52.122.98", &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sockfd);
        return;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return;
    }

    // Prepare the HTTP GET request for the man page
    snprintf(send_buffer, sizeof(send_buffer),
             "GET /cgi-bin/man.cgi?query=%s HTTP/1.1\r\n"
             "Host: man.he.net\r\n"
             "Connection: close\r\n"
             "\r\n",
             command);

    // Send the request
    if (send(sockfd, send_buffer, strlen(send_buffer), 0) < 0) {
        perror("Error sending request");
        close(sockfd);
        return;
    }

    // Read the response
    int received;
    int header_done = 0;
    char *content_start = NULL;

    while ((received = recv(sockfd, recv_buffer, sizeof(recv_buffer) - 1, 0)) > 0) {
        recv_buffer[received] = '\0';  // Null-terminate the response

        if (!header_done) {
            // Check for the end of the HTTP header and start of content
            content_start = skip_http_headers(recv_buffer);
            if (content_start) {
                header_done = 1;
                printf("%s", content_start);  // Print the content after the header
            }
        } else {
            printf("%s", recv_buffer);  // Print remaining content
        }
    }

    if (received == 0) {
        // Connection closed by the server
        printf("\n[End of man page]\n");
    } else if (received < 0) {
        perror("Error receiving data");
    }

    // Close the socket
    close(sockfd);
}
