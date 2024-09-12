#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "fg_bg.h"
#include "activities.h"  // Assuming this manages the process tracking

extern pid_t foreground_pid;  // To keep track of the current foreground process

// Function to bring a background process to the foreground
void fg_command(int pid) {
    // Check if the process exists in the activities (process tracking)
    if (!is_process_tracked(pid)) {
        printf("No such process found\n");
        return;
    }

    // Wait for the process to continue running in the foreground
    foreground_pid = pid;  // Mark the process as the current foreground process
    update_process_state(pid, 1, 1);  // Mark as running and foreground
    if (kill(pid, SIGCONT) == -1) {
        perror("Error sending SIGCONT to process");
        return;
    }

    // Bring the process to the foreground and wait for it to complete
    int status;
    if (waitpid(pid, &status, WUNTRACED) == -1) {
        perror("Error waiting for process in foreground");
    }

    // After the process terminates or is stopped, clear the foreground PID
    foreground_pid = -1;
}

// Function to resume a stopped background process
void bg_command(int pid) {
    // Check if the process exists in the activities (process tracking)
    if (!is_process_tracked(pid)) {
        printf("No such process found\n");
        return;
    }

    // Resume the background process
    update_process_state(pid, 1, 0);  // Mark as running and background
    if (kill(pid, SIGCONT) == -1) {
        perror("Error sending SIGCONT to process");
        return;
    }

    printf("Process [%d] is now running in the background\n", pid);
}
