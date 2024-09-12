#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include "seek.h"

#define GREEN "\033[0;32m"
#define BLUE "\033[0;34m"
#define RESET "\033[0m"

// Function to print the relative path
void print_relative_path(char *base_path, char *path, int is_dir) {
    char relative_path[PATH_MAX];

    // Get the relative path by removing the base_path part
    if (strncmp(path, base_path, strlen(base_path)) == 0) {
        snprintf(relative_path, sizeof(relative_path), ".%s", path + strlen(base_path));
    } else {
        strncpy(relative_path, path, sizeof(relative_path));
    }

    // Print directories in blue, files in green
    if (is_dir) {
        printf(BLUE "%s\n" RESET, relative_path);
    } else {
        printf(GREEN "%s\n" RESET, relative_path);
    }
}

// Function to check permissions on a path
int has_permission(char *path, int is_dir) {
    if (is_dir) {
        return access(path, X_OK) == 0;
    } else {
        return access(path, R_OK) == 0;
    }
}

// Process individual entries and check if they match the search criteria
void process_entry(char *base_path, char *entry_path, char *entry_name, char *search_name, int search_files, int search_dirs, int *found, char **single_match_path, int *single_match_is_dir) {
    struct stat path_stat;
    if (stat(entry_path, &path_stat) == -1) {
        perror("Error getting file status");
        return;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        if (strncmp(entry_name, search_name, strlen(search_name)) == 0 && search_dirs) {
            print_relative_path(base_path, entry_path, 1);  // Print directory
            *found = 1;

            if (*single_match_path == NULL) {
                *single_match_path = strdup(entry_path);
                *single_match_is_dir = 1;
            } else {
                free(*single_match_path);  // Multiple matches found, clear the previous match
                *single_match_path = NULL;
            }
        }
    } else if (S_ISREG(path_stat.st_mode)) {
        if (strncmp(entry_name, search_name, strlen(search_name)) == 0 && search_files) {
            print_relative_path(base_path, entry_path, 0);  // Print file
            *found = 1;

            if (*single_match_path == NULL) {
                *single_match_path = strdup(entry_path);
                *single_match_is_dir = 0;
            } else {
                free(*single_match_path);  // Multiple matches found, clear the previous match
                *single_match_path = NULL;
            }
        }
    }
}

// Recursive directory search
void search_directory_recursive(char *base_path, char *current_path, char *search_name, int search_files, int search_dirs, int *found, char **single_match_path, int *single_match_is_dir) {
    DIR *dir = opendir(current_path);
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    char path[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", current_path, entry->d_name);

        process_entry(base_path, path, entry->d_name, search_name, search_files, search_dirs, found, single_match_path, single_match_is_dir);

        if (entry->d_type == DT_DIR) {
            search_directory_recursive(base_path, path, search_name, search_files, search_dirs, found, single_match_path, single_match_is_dir);
        }
    }

    closedir(dir);
}

// Main execution function for 'seek' command
void execute_seek(char *args[]) {
    int search_files = 1, search_dirs = 1, execute_flag = 0;
    char *search_name = NULL;
    char *target_directory = ".";
    int found = 0;
    char *single_match_path = NULL;
    int single_match_is_dir = 0;

    // Parse command arguments
    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] == '-') {
            if (strcmp(args[i], "-d") == 0) {
                search_files = 0;  // Only search for directories
            } else if (strcmp(args[i], "-f") == 0) {
                search_dirs = 0;  // Only search for files
            } else if (strcmp(args[i], "-e") == 0) {
                execute_flag = 1;  // Enable execute flag
            } else {
                printf("Invalid flags!\n");
                return;
            }
        } else if (search_name == NULL) {
            search_name = args[i];
        } else {
            target_directory = args[i];
        }
    }

    if (search_name == NULL) {
        printf("Usage: seek <flags> <search> <target_directory>\n");
        return;
    }

    if (!search_files && !search_dirs) {
        printf("Invalid flags!\n");
        return;
    }

    // Recursively search the target directory for matches
    search_directory_recursive(target_directory, target_directory, search_name, search_files, search_dirs, &found, &single_match_path, &single_match_is_dir);

    // Handle -e flag if enabled
    if (execute_flag && single_match_path != NULL) {
        if (has_permission(single_match_path, single_match_is_dir)) {
            if (single_match_is_dir) {
                if (chdir(single_match_path) == 0) {
                    printf("Changed directory to %s\n", single_match_path);
                } else {
                    perror("Error changing directory");
                }
            } else {
                FILE *file = fopen(single_match_path, "r");
                if (file != NULL) {
                    char line[1024];
                    while (fgets(line, sizeof(line), file)) {
                        printf("%s", line);
                    }
                    fclose(file);
                } else {
                    perror("Error reading file");
                }
            }
        } else {
            printf("Missing permissions for task!\n");
        }
    }

    if (!found) {
        printf("No match found!\n");
    }

    free(single_match_path);  // Free dynamically allocated memory
}
