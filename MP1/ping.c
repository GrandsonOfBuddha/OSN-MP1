#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "ping.h"
#include "activities.h"

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
        // If there is a foreground process, send it the SIGINT signal
        kill(foreground_pid, SIGINT);
    } else {
        // If no foreground process is running, do nothing (prevents shell from exiting)
        printf("\nNo foreground process to interrupt\n");
        fflush(stdout);  // Ensure the prompt is re-displayed correctly
    }
}

// Signal handler for SIGTSTP (Ctrl-Z)
void handle_sigtstp(int sig) {
    if (foreground_pid > 0) {
        // Send SIGTSTP to the foreground process to suspend it
        kill(foreground_pid, SIGTSTP);
        update_process_state(foreground_pid, 0, 0);  // Mark process as stopped
        printf("\nForeground process stopped\n");
    } else {
        printf("\nNo foreground process to stop\n");
    }
}

// Signal handler for SIGQUIT (Ctrl-D)
void handle_sigquit(int sig) {
    printf("\nExiting shell\n");
    // Add logic to kill any background processes here if necessary
    exit(0);
}

// Set up signal handlers for Ctrl-C, Ctrl-Z, and Ctrl-D
void setup_signal_handlers() {
    struct sigaction sa;

    // SIGINT (Ctrl-C) handler
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);

    // SIGTSTP (Ctrl-Z) handler
    sa.sa_handler = handle_sigtstp;
    sigaction(SIGTSTP, &sa, NULL);

    // SIGQUIT (Ctrl-D) handler
    sa.sa_handler = handle_sigquit;
    sigaction(SIGQUIT, &sa, NULL);
}

