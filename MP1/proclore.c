#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "proclore.h"

void execute_proclore(char *pid) {
    char path[256];
    char buffer[1024];
    char *target_pid = pid ? pid : "self";  // Default to "self" if no PID is provided
    snprintf(path, sizeof(path), "/proc/%s/stat", target_pid);

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening process stat file");
        return;
    }

    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("Error reading process stat file");
        close(fd);
        return;
    }
    buffer[bytes_read] = '\0';  // Null-terminate the buffer

    close(fd);

    // Parse the relevant fields from the stat file
    int pid_value, pgrp;
    char comm[256], state;
    unsigned long vsize;
    sscanf(buffer, "%d %s %c %*d %d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %lu", 
           &pid_value, comm, &state, &pgrp, &vsize);

    // Determine the process status and foreground/background status
    char *status;
    if (state == 'R') {
        status = "+R";  // Running
    } else if (state == 'S') {
        status = "+S";  // Sleeping
    } else if (state == 'Z') {
        status = "Z";  // Zombie
    } else {
        status = "Unknown";
    }

    snprintf(path, sizeof(path), "/proc/%s/exe", target_pid);
    ssize_t len = readlink(path, buffer, sizeof(buffer) - 1);
    if (len == -1) {
        perror("Error reading executable path");
        return;
    }
    buffer[len] = '\0';

    printf("pid : %d\n", pid_value);
    printf("process status : %s\n", status);
    printf("process group : %d\n", pgrp);
    printf("virtual memory : %lu\n", vsize);
    printf("executable path : %s\n", buffer);
}
