#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hop.h"

static char prev_dir[1024] = "";
static char shell_home_dir[1024] = "";

void initialize_home_directory() {
    // Get the current working directory at the shell startup and store it as the "home" directory.
    if (getcwd(shell_home_dir, sizeof(shell_home_dir)) == NULL) {
        perror("Error getting the initial directory");
        exit(EXIT_FAILURE);
    }
}

void execute_hop(char *args[]) {
    char cwd[1024];

    if (args[1] == NULL) {
        // Hop to the shell's "home" directory
        if (chdir(shell_home_dir) != 0) {
            perror("ERROR");
        }
    } else {
        for (int i = 1; args[i] != NULL; i++) {
            char *path = args[i];

            if (strcmp(path, "~") == 0) {
                path = shell_home_dir;
            } else if (strcmp(path, "-") == 0) {
                if (strlen(prev_dir) == 0) {
                    fprintf(stderr, "ERROR: No previous directory\n");
                    return;
                }
                path = prev_dir;
            } else if (strcmp(path, ".") == 0) {
                continue;
            } else if (strcmp(path, "..") == 0) {
                path = "..";
            } else if (path[0] == '~') {
                char full_path[1024];
                snprintf(full_path, sizeof(full_path), "%s%s", shell_home_dir, path + 1);
                path = full_path;
            }

            if (chdir(path) != 0) {
                perror("ERROR");
                return;
            }

            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                strcpy(prev_dir, cwd);
                printf("%s\n", cwd);
            } else {
                perror("getcwd() error");
                return;
            }
        }
    }
}
