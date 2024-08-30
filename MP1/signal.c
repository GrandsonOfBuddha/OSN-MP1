#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "signal.h"

void sigchld_handler(int signum) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("Process %d exited normally with status %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Process %d was terminated by signal %d\n", pid, WTERMSIG(status));
        }
        fflush(stdout);  // Ensure the message is printed immediately
    }
}

void setup_signal_handler() {
    signal(SIGCHLD, sigchld_handler);  // Set up the signal handler for SIGCHLD
}
