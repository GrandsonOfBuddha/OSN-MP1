#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "seek.h"

#define GREEN "\033[0;32m"
#define BLUE "\033[0;34m"
#define RESET "\033[0m"

void print_relative_path(char *base_path, char *path, int is_dir) {
    if (is_dir) {
        printf(BLUE "%s\n" RESET, path);
    } else {
        printf(GREEN "%s\n" RESET, path);
    }
}

int has_permission(char *path, int is_dir) {
    if (is_dir) {
        return access(path, X_OK) == 0;
    } else {
        return access(path, R_OK) == 0;
    }
}

int search_directory(char *base_path, char *search_name, int search_files, int search_dirs, int execute_flag, int *found) {
    DIR *dir = opendir(base_path);
    if (dir == NULL) {
        perror("Error opening directory");
        return 0;
    }

    struct dirent *entry;
    char path[1024];
    int single_match = 0;
    char single_match_path[1024];
    int single_match_is_dir = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);

        struct stat path_stat;
        if (stat(path, &path_stat) == -1) {
            perror("Error getting file status");
            continue;
        }

        if (S_ISDIR(path_stat.st_mode)) {
            if (strcmp(entry->d_name, search_name) == 0 && search_dirs) {
                print_relative_path(base_path, path, 1);
                *found = 1;

                if (single_match == 0) {
                    single_match = 1;
                    strcpy(single_match_path, path);
                    single_match_is_dir = 1;
                } else {
                    single_match = 2;
                }
            }

            search_directory(path, search_name, search_files, search_dirs, execute_flag, found);
        } else if (S_ISREG(path_stat.st_mode)) {
            if (strcmp(entry->d_name, search_name) == 0 && search_files) {
                print_relative_path(base_path, path, 0);
                *found = 1;

                if (single_match == 0) {
                    single_match = 1;
                    strcpy(single_match_path, path);
                    single_match_is_dir = 0;
                } else {
                    single_match = 2;
                }
            }
        }
    }
    closedir(dir);

    // Handle -e flag execution if only a single match was found
    if (single_match == 1 && execute_flag) {
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

    return *found;
}

void execute_seek(char *args[]) {
    int search_files = 1, search_dirs = 1, execute_flag = 0;
    char *search_name = NULL;
    char *target_directory = ".";
    int found = 0;

    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] == '-') {
            if (strcmp(args[i], "-d") == 0) {
                search_files = 0;
            } else if (strcmp(args[i], "-f") == 0) {
                search_dirs = 0;
            } else if (strcmp(args[i], "-e") == 0) {
                execute_flag = 1;
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

    search_directory(target_directory, search_name, search_files, search_dirs, execute_flag, &found);

    if (!found) {
        printf("No match found!\n");
    }
}
