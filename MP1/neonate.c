#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/select.h>
#include "neonate.h"

// Function to check for user key press ('x')
int check_key_press() {
    fd_set readfds;
    struct timeval timeout;

    // Set timeout to 0, as we don't want to block
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    int result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

    if (result > 0) {
        char ch;
        read(STDIN_FILENO, &ch, 1);
        if (ch == 'x' || ch == 'X') {
            return 1; // 'x' was pressed
        }
    }

    return 0; // No key press
}

// Function to find the most recent PID by scanning /proc
pid_t get_most_recent_pid() {
    DIR *dir;
    struct dirent *entry;
    pid_t recent_pid = 0;

    dir = opendir("/proc");
    if (!dir) {
        perror("Failed to open /proc");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Check if the directory name is a number (i.e., a PID)
        if (isdigit(entry->d_name[0])) {
            pid_t pid = atoi(entry->d_name);
            if (pid > recent_pid) {
                recent_pid = pid; // Update to the most recent PID
            }
        }
    }

    closedir(dir);
    return recent_pid;
}

// Main function for the neonate command
void execute_neonate(char *args[]) {
    if (args[1] == NULL || strcmp(args[1], "-n") != 0 || args[2] == NULL) {
        printf("Usage: neonate -n [time_arg]\n");
        return;
    }

    int time_interval = atoi(args[2]); // Get the time interval
    if (time_interval <= 0) {
        printf("Invalid time argument\n");
        return;
    }

    // Set terminal to non-blocking mode for key press detection
    int old_flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK);

    while (1) {
        pid_t recent_pid = get_most_recent_pid();
        if (recent_pid > 0) {
            printf("%d\n", recent_pid);
        } else {
            printf("Failed to get the most recent PID\n");
        }

        sleep(time_interval); // Wait for the specified time interval

        // Check if 'x' key was pressed to terminate
        if (check_key_press()) {
            printf("Key 'x' pressed, exiting.\n");
            break;
        }
    }

    // Restore terminal to blocking mode
    fcntl(STDIN_FILENO, F_SETFL, old_flags);
}
