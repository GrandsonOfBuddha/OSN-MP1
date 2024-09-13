#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "prompt.h"
#include "input.h"
#include "command.h"
#include "mysignal.h"
#include "hop.h"
#include "myshrc.h"
#include "ping.h"

char shell_start_dir[PATH_MAX];

int main() {
     if (getcwd(shell_start_dir, sizeof(shell_start_dir)) == NULL) {
        perror("Error getting shell start directory");
        exit(EXIT_FAILURE);
    }
    setup_signal_handlers();
    load_myshrc();
    initialize_signal_handlers();
    initialize_home_directory();
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
