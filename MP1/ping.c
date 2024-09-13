#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "ping.h"
#include "activities.h"
#include "prompt.h"

pid_t foreground_pid = -1; // Track the current foreground process

// Function to handle the ping command
void execute_ping(char *args[]) {
    if (args[1] == NULL || args[2] == NULL) {
        printf("Usage: ping <pid> <signal_number>\n");
        return;
    }

    pid_t pid = atoi(args[1]);
    int signal_num = atoi(args[2]) % 32;  // Modulo 32 to stay within valid signal range

    if (kill(pid, 0) == -1) {
        if (errno == ESRCH) {
            printf("No such process found\n");
        } else {
            perror("Error checking process");
        }
        return;
    }

    if (kill(pid, signal_num) == 0) {
        printf("Sent signal %d to process with pid %d\n", signal_num, pid);
    } else {
        perror("Error sending signal");
    }
}

// Signal handler for SIGINT (Ctrl-C)
void handle_sigint(int sig) {
    if (foreground_pid > 0) {
        // Send SIGINT to the foreground process
        kill(foreground_pid, SIGINT);
    } else {
        printf("\nNo foreground process to interrupt\n");
    }
}

// Signal handler for SIGTSTP (Ctrl-Z)
void handle_sigtstp(int sig) {
    if (foreground_pid > 0) {
        // Send SIGTSTP to the foreground process
        if (kill(foreground_pid, SIGTSTP) == -1) {
            perror("Failed to send SIGTSTP to the process");
        } else {
            // Successfully stopped the process, now update its state
            update_process_state(foreground_pid, 0, 0); // Mark the process as stopped and in the background
            printf("\nForeground process [%d] stopped and moved to background\n", foreground_pid);
        }

        // After stopping, reset the foreground PID
        foreground_pid = -1;

        // Redisplay the prompt
        display_prompt();
    } else {
        printf("\nNo foreground process to stop\n");
        display_prompt();
    }
}



// Set up signal handlers for Ctrl-C, Ctrl-Z, and Ctrl-D
void setup_signal_handlers() {
    signal(SIGINT, handle_sigint);    // Handle Ctrl-C
    signal(SIGTSTP, handle_sigtstp);  // Handle Ctrl-Z
    // signal(SIGQUIT, handle_sigquit);  // Handle Ctrl-D
}

