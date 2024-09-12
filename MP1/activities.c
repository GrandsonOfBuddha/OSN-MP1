#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "activities.h"

#define MAX_PROCESSES 100  // Maximum number of processes to track

// Array to store information about processes
ProcessInfo process_list[MAX_PROCESSES];
int process_count = 0;  // Number of processes being tracked

// Helper function to lexicographically sort processes by command name
int compare_processes(const void *a, const void *b) {
    ProcessInfo *processA = (ProcessInfo *)a;
    ProcessInfo *processB = (ProcessInfo *)b;
    return strcmp(processA->command, processB->command);
}

// Add a new process to the list
void add_process(pid_t pid, char *command, int is_foreground) {
    if (process_count < MAX_PROCESSES) {
        process_list[process_count].pid = pid;
        strncpy(process_list[process_count].command, command, sizeof(process_list[process_count].command) - 1);
        process_list[process_count].is_running = 1;  // Initially running
        process_list[process_count].is_foreground = is_foreground;  // Track if it's foreground or background
        process_count++;
    }
}

// Update the state of a process (running, stopped, foreground, or background)
void update_process_state(pid_t pid, int is_running, int is_foreground) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            process_list[i].is_running = is_running;
            process_list[i].is_foreground = is_foreground;  // Update foreground/background state
            break;
        }
    }
}

// Remove a process from the list
void remove_process(pid_t pid) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            // Shift the rest of the processes down
            for (int j = i; j < process_count - 1; j++) {
                process_list[j] = process_list[j + 1];
            }
            process_count--;
            break;
        }
    }
}

// Check if a process with the given pid exists
int is_process_tracked(pid_t pid) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            return 1;
        }
    }
    return 0;
}

// Get the foreground state of a process
int is_process_foreground(pid_t pid) {
    for (int i = 0; i < process_count; i++) {
        if (process_list[i].pid == pid) {
            return process_list[i].is_foreground;
        }
    }
    return 0;  // Process not found or it's in the background
}

// Print the list of processes in lexicographic order
void print_activities() {
    if (process_count == 0) {
        printf("No active processes.\n");
        return;
    }

    // Sort processes by command name
    qsort(process_list, process_count, sizeof(ProcessInfo), compare_processes);

    // Print the list
    for (int i = 0; i < process_count; i++) {
        printf("[%d] : %s - %s\n", process_list[i].pid, process_list[i].command,
               process_list[i].is_running ? "Running" : "Stopped");
    }
}
