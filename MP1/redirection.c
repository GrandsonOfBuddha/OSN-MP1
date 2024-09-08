#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "redirection.h"

void handle_io_redirection(char *args[], int *in_redir, int *out_redir, int *append_redir) {
    *in_redir = -1;
    *out_redir = -1;
    *append_redir = -1;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            *out_redir = handle_file_redirection(args[i + 1], 1, 0);
            args[i] = NULL; 
        } else if (strcmp(args[i], ">>") == 0) {
            *append_redir = handle_file_redirection(args[i + 1], 1, 1);
            args[i] = NULL;
        } else if (strcmp(args[i], "<") == 0) {
            *in_redir = handle_file_redirection(args[i + 1], 0, 0);
            args[i] = NULL;
        }
    }
}

int handle_file_redirection(char *filename, int direction, int append) {
    if (direction == 1) {  // Output redirection
        int flags = O_WRONLY | O_CREAT;
        if (append) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }
        int fd = open(filename, flags, 0644);
        if (fd == -1) {
            perror("Error opening output file");
            return -1;
        }
        return fd;
    } else if (direction == 0) {  // Input redirection
        int fd = open(filename, O_RDONLY);
        if (fd == -1) {
            perror("No such input file found!");
            return -1;
        }
        return fd;
    }
    return -1;
}
