#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "prompt.h"

// ANSI color codes
#define GREEN "\033[0;32m"
#define RESET "\033[0m"

void display_prompt() {
    char cwd[1024];
    char hostname[1024];
    char *username = getenv("USER");
    char *home = getenv("HOME");

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return;
    }

    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("gethostname() error");
        return;
    }

    // Replace home directory in the prompt with '~' and print the prompt in green
    if (home != NULL && strstr(cwd, home) == cwd) {
        printf(GREEN "%s@%s:~%s$ " RESET, username, hostname, cwd + strlen(home));
    } else {
        printf(GREEN "%s@%s:%s$ " RESET, username, hostname, cwd);
    }

    fflush(stdout); // Ensure the prompt is displayed
}
