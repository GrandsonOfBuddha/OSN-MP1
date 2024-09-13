#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define BUFFER_SIZE 8192
#define REQUEST_SIZE 2048  // Increased size to handle longer strings

// Function to skip HTTP headers and find the start of the content
char *skip_http_headers(char *response) {
    char *header_end = strstr(response, "\r\n\r\n");
    if (header_end) {
        return header_end + 4;  // Skip over the headers
    }
    return response;  // If no headers found, return the response as-is
}

// Function to remove HTML tags from the content
void remove_html_tags(char *src) {
    char *dst = src;
    int in_tag = 0;  // Flag to track if we're inside an HTML tag

    while (*src) {
        if (*src == '<') {
            in_tag = 1;  // Entering an HTML tag
        } else if (*src == '>') {
            in_tag = 0;  // Exiting an HTML tag
            src++;
            continue;
        }

        if (!in_tag) {
            *dst++ = *src;  // Copy only the characters outside of tags
        }

        src++;
    }
    *dst = '\0';  // Null-terminate the string
}

void fetch_man_page(const char *command_name) {
    int sockfd;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
    char request[REQUEST_SIZE];  // Increase the size of the request buffer
    
    // Step 1: Hostname and path formatting
    const char *hostname = "man.he.net";
    
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
    snprintf(request, sizeof(request), "GET /?topic=%s&section=all HTTP/1.1\r\nHost: man.he.net\r\nUser-Agent: iMan/1.0\r\nConnection: close\r\n\r\n", command_name);

    // Step 7: Send the HTTP request to the server
    if (send(sockfd, request, strlen(request), 0) == -1) {
        perror("ERROR writing to socket");
        close(sockfd);
        return;
    }

    // Step 8: Read and process the server's reply
    int read_bytes;
    int headers_skipped = 0;
    while ((read_bytes = recv(sockfd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[read_bytes] = '\0';  // Null-terminate the response

        if (!headers_skipped) {
            // Skip the HTTP headers before processing the content
            char *content_start = skip_http_headers(buffer);
            remove_html_tags(content_start);  // Remove HTML tags
            printf("%s", content_start);      // Print the content
            headers_skipped = 1;
        } else {
            remove_html_tags(buffer);  // Remove HTML tags from remaining content
            printf("%s", buffer);
        }
    }

    if (read_bytes < 0) {
        perror("ERROR reading from socket");
    }

    // Step 9: Close the socket
    close(sockfd);
}
