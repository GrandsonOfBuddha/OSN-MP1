#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "pipe.h"
#include "command.h"
#include "input.h"

int contains_pipe(const char *input) {
    return strchr(input, '|') != NULL;
}

void execute_pipe(char *input) {
    char *commands[1024];
    char *command = strtok(input, "|");
    int num_commands = 0;

    while (command != NULL) {
        commands[num_commands++] = command;
        command = strtok(NULL, "|");
    }
    commands[num_commands] = NULL;

    if (num_commands == 1) {
        execute_command(commands[0]);
        return;
    }

    int pipefd[2], in_fd = 0, status;
    pid_t pid;

    for (int i = 0; i < num_commands; i++) {
        commands[i] = trim_whitespace(commands[i]);

        if (strlen(commands[i]) == 0) {
            fprintf(stderr, "Invalid use of pipe\n");
            return;
        }

        if (i < num_commands - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe error");
                exit(EXIT_FAILURE);
            }
        }

        pid = fork();
        if (pid == 0) {
            // Child process

            if (in_fd != 0) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            if (i < num_commands - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            if (i < num_commands - 1) {
                close(pipefd[0]);
            }

            // Execute the command
            execute_command(commands[i]);
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("fork error");
            exit(EXIT_FAILURE);
        }

        waitpid(pid, &status, 0); 

        if (i < num_commands - 1) {
            close(pipefd[1]);
        }

        in_fd = pipefd[0];
    }
}

