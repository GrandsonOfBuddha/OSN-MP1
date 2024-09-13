#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "input.h"
#include "log.h"

int get_input(char *input, size_t size) {
    if (fgets(input, size, stdin) == NULL) {
        exit(0);
    }

    // Remove trailing newline character
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }

    // Log the command immediately after taking input, if it's valid
    if (strcmp(input, "") != 0 && strncmp(input, "log", 3) != 0) {
        log_command(input);
    }

    return 1;
}

// Helper function to trim leading and trailing spaces and tabs
char *trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char) *str)) str++;

    if (*str == 0) // All spaces
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char) *end)) end--;

    end[1] = '\0';

    return str;
}
