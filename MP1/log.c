#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>   // For PATH_MAX
#include "log.h"
#include "command.h"

#ifndef PATH_MAX
#define PATH_MAX 4096  // Define PATH_MAX if it's not defined
#endif

extern char shell_start_dir[PATH_MAX];  // Global variable storing the shell's start directory

#define MAX_LOG_SIZE 15

// Helper function to get the full path of the log file
void get_log_file_path(char *log_file_path) {
    int result = snprintf(log_file_path, PATH_MAX, "%s/command_log.txt", shell_start_dir);
    if (result >= PATH_MAX || result < 0) {
        fprintf(stderr, "Error: Path length exceeds maximum allowed length\n");
        exit(EXIT_FAILURE);
    }
}


void log_command(const char *command) {
    char log_file_path[PATH_MAX];
    get_log_file_path(log_file_path);

    FILE *file = fopen(log_file_path, "r+");
    if (file == NULL) {
        file = fopen(log_file_path, "w+");
        if (file == NULL) {
            perror("Error opening log file");
            return;
        }
    }

    char current_log[MAX_LOG_SIZE][1024];
    int count = 0;

    // Read existing log entries
    while (fgets(current_log[count], sizeof(current_log[count]), file) != NULL) {
        current_log[count][strcspn(current_log[count], "\n")] = '\0';  // Remove newline
        if (strcmp(current_log[count], command) == 0) {
            fclose(file);
            return;  // Do not log duplicate commands
        }
        count++;
    }

    // Add the new command if it's not the same as the last one
    if (count == 0 || strcmp(current_log[count - 1], command) != 0) {
        if (count < MAX_LOG_SIZE) {
            strcpy(current_log[count], command);
            count++;
        } else {
            // Shift log entries if the log is full
            for (int i = 1; i < MAX_LOG_SIZE; i++) {
                strcpy(current_log[i - 1], current_log[i]);
            }
            strcpy(current_log[MAX_LOG_SIZE - 1], command);
        }
    }

    // Rewrite the log file
    freopen(log_file_path, "w", file);
    for (int i = 0; i < count; i++) {
        fprintf(file, "%s\n", current_log[i]);
    }
    fclose(file);
}

void display_log() {
    char log_file_path[PATH_MAX];
    get_log_file_path(log_file_path);

    FILE *file = fopen(log_file_path, "r");
    if (file == NULL) {
        printf("No log available.\n");
        return;
    }

    char line[1024];
    int index = 1;
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = '\0';  // Remove newline
        printf("%d: %s\n", index++, line);
    }
    fclose(file);
}

void purge_log() {
    char log_file_path[PATH_MAX];
    get_log_file_path(log_file_path);

    FILE *file = fopen(log_file_path, "w");
    if (file != NULL) {
        fclose(file);
    }
}

void execute_log_command(int index) {
    char log_file_path[PATH_MAX];
    get_log_file_path(log_file_path);

    FILE *file = fopen(log_file_path, "r");
    if (file == NULL) {
        printf("No log available.\n");
        return;
    }

    char line[1024];
    int current_index = 1;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (current_index == index) {
            line[strcspn(line, "\n")] = '\0';  // Remove newline
            fclose(file);

            printf("Executing: %s\n", line);

            // Directly execute the command using your shell's command execution function
            execute_command(line);  // This will use your shell's execute_command function

            // If it's not the most recent command, log it
            if (current_index != 1) {
                log_command(line);
            }
            return;
        }
        current_index++;
    }
    printf("Invalid log index.\n");
    fclose(file);
}
