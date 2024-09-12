#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include <sys/types.h>

typedef struct {
    pid_t pid;                  // Process ID
    char command[256];          // Command name
    int is_running;             // 1 if running, 0 if stopped
    int is_foreground;          // 1 if foreground, 0 if background
} ProcessInfo;

// Add a process to the tracking list
void add_process(pid_t pid, char *command, int is_foreground);

// Update the state of a process
void update_process_state(pid_t pid, int is_running, int is_foreground);

// Remove a process from the list
void remove_process(pid_t pid);

// Check if a process is being tracked
int is_process_tracked(pid_t pid);

// Check if a process is in the foreground
int is_process_foreground(pid_t pid);

// Print the list of running/stopped processes
void print_activities(void);

#endif
