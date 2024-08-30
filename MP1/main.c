#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "prompt.h"
#include "input.h"
#include "command.h"
#include "signal.h"
#include "hop.h"

char shell_start_dir[PATH_MAX];

int main() {
     if (getcwd(shell_start_dir, sizeof(shell_start_dir)) == NULL) {
        perror("Error getting shell start directory");
        exit(EXIT_FAILURE);
    }
    setup_signal_handler();  // Set up the signal handler at the start of the program
    initialize_home_directory();  // Initialize the shell's home directory
    char input[1024];

    while (1) {
        display_prompt();   // Display the custom shell prompt
        if (get_input(input, sizeof(input)) == 0) {
            continue;
        }

        // Process the input (handles ';' and '&' separated commands)
        process_input(input);
    }

    return 0;
}
