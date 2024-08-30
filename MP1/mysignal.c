#include <stdio.h>
#include <stdlib.h>
#include <signal.h>     // For signal handling functions and definitions
#include <sys/types.h>  // For pid_t type
#include <sys/wait.h>   // For waitpid function
#include <errno.h>      // For errno

#include "mysignal.h"

void handle_sigchld(int sig) {
    int saved_errno = errno; // Save errno to restore it later, as some library functions might change it.
    pid_t pid;
    int status;

    // Loop to handle all terminated child processes
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("Process %d finished normally with exit code %d.\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Process %d terminated by signal %d.\n", pid, WTERMSIG(status));
        }
    }

    // Restore the original errno value
    errno = saved_errno;
}

void initialize_signal_handlers() {
    struct sigaction sa;

    // Zero out the sigaction struct
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sa.sa_handler = handle_sigchld;

    // Set the handler for SIGCHLD
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}
