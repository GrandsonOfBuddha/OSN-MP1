#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // For chdir and getcwd
#include <sys/stat.h>
#include "myshrc.h"
#include "command.h"  // Include to execute the original command functions
#include "seek.h"

#define ALIAS_COUNT 100
#define ALIAS_MAX_LENGTH 100

typedef struct {
    char alias[ALIAS_MAX_LENGTH];
    char command[ALIAS_MAX_LENGTH];
} Alias;

Alias aliases[ALIAS_COUNT];
int alias_count = 0;

// Load .myshrc file to setup aliases and functions
void load_myshrc() {
    FILE *file = fopen(".myshrc", "r");
    if (file == NULL) {
        // If file does not exist, simply return
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;  // Remove newline

        // Parse alias lines
        if (strncmp(line, "alias", 5) == 0) {
            char *alias_name = strtok(line + 6, "=");
            char *alias_command = strtok(NULL, "=");
            if (alias_name && alias_command) {
                strcpy(aliases[alias_count].alias, alias_name);
                strcpy(aliases[alias_count].command, alias_command);
                alias_count++;
            }
        }
    }

    fclose(file);
}

// Check if the command matches any alias and replace it with the real command
char *check_alias(char *command) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(command, aliases[i].alias) == 0) {
            return aliases[i].command;
        }
    }
    return command;  // Return original command if no alias matches
}

// Execute custom functions like mk_hop and hop_seek
void execute_function(char *func_name, char *arg) {
    if (strcmp(func_name, "mk_hop") == 0) {
        // Create the directory
        if (mkdir(arg, 0755) == 0) {
            // Hop into the created directory
            if (chdir(arg) == 0) {
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("Changed directory to %s\n", cwd);
                } else {
                    perror("getcwd() error");
                }
            } else {
                perror("chdir() error");
            }
        } else {
            perror("mkdir() error");
        }
    } else if (strcmp(func_name, "hop_seek") == 0) {
        // Change into the directory
        if (chdir(arg) == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("Changed directory to %s\n", cwd);
            } else {
                perror("getcwd() error");
            }
            // Call seek to find files in the directory
            char *seek_args[] = {"seek", arg, NULL};
            execute_seek(seek_args);  // Call the seek command to search within the directory
        } else {
            perror("chdir() error");
        }
    } else {
        printf("Invalid function: %s\n", func_name);
    }
}
