#include <stdio.h>
#include <stdlib.h>
#include <signal.h>    
#include <sys/types.h>  
#include <sys/wait.h>   
#include <errno.h>      

#include "mysignal.h"
#include "activities.h"  // Include for updating process state

void handle_sigchld(int sig) {
    int saved_errno = errno;  // Save errno to restore later
    pid_t pid;
    int status;

    // Loop over all child processes that have terminated
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("Process %d finished normally with exit code %d.\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Process %d terminated by signal %d.\n", pid, WTERMSIG(status));
        }

        // Update the process state in the activities list to "stopped"
        update_process_state(pid, 0, 0);  // Mark as stopped (0), background (0)
    }

    errno = saved_errno;  // Restore errno
}

void initialize_signal_handlers() {
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sa.sa_handler = handle_sigchld;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}
