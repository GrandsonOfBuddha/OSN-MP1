#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "reveal.h"

#define BLUE   "\x1b[34m"
#define GREEN  "\x1b[32m"
#define WHITE  "\x1b[37m"
#define RESET  "\x1b[0m"

void print_file_info(struct stat *file_stat, char *file_name) {
    // Print file type and permissions
    printf((S_ISDIR(file_stat->st_mode)) ? "d" : "-");
    printf((file_stat->st_mode & S_IRUSR) ? "r" : "-");
    printf((file_stat->st_mode & S_IWUSR) ? "w" : "-");
    printf((file_stat->st_mode & S_IXUSR) ? "x" : "-");
    printf((file_stat->st_mode & S_IRGRP) ? "r" : "-");
    printf((file_stat->st_mode & S_IWGRP) ? "w" : "-");
    printf((file_stat->st_mode & S_IXGRP) ? "x" : "-");
    printf((file_stat->st_mode & S_IROTH) ? "r" : "-");
    printf((file_stat->st_mode & S_IWOTH) ? "w" : "-");
    printf((file_stat->st_mode & S_IXOTH) ? "x" : "-");
    
    // Print number of links
    printf(" %ld", (long) file_stat->st_nlink);
    
    // Print owner and group
    printf(" %s %s", getpwuid(file_stat->st_uid)->pw_name, getgrgid(file_stat->st_gid)->gr_name);
    
    // Print file size
    printf(" %5lld", (long long) file_stat->st_size);
    
    // Print time of last modification
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&file_stat->st_mtime));
    printf(" %s", time_str);
    
    // Print the file name
    if (S_ISDIR(file_stat->st_mode)) {
        printf(" " BLUE "%s" RESET "\n", file_name);
    } else if (file_stat->st_mode & S_IXUSR) {
        printf(" " GREEN "%s" RESET "\n", file_name);
    } else {
        printf(" " WHITE "%s" RESET "\n", file_name);
    }
}

void execute_reveal(char *args[]) {
    int show_hidden = 0;
    int show_long = 0;
    char *path = ".";  // Default path is the current directory

    // Parse flags and path
    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] == '-') {
            for (int j = 1; args[i][j] != '\0'; j++) {
                if (args[i][j] == 'a') {
                    show_hidden = 1;
                } else if (args[i][j] == 'l') {
                    show_long = 1;
                }
            }
        } else {
            path = args[i];
        }
    }

    // Handle special symbols
    if (strcmp(path, "~") == 0) {
        path = getenv("HOME");
    } else if (strcmp(path, "-") == 0) {
        path = getenv("OLDPWD");
    } else if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0) {
        // Do nothing, use as is
    } else if (path[0] == '~') {
        static char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s%s", getenv("HOME"), path + 1);
        path = full_path;
    }

    // Open directory
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("ERROR");
        return;
    }

    struct dirent *entry;
    struct stat file_stat;
    char full_path[1024];

    // Read directory entries into an array to sort them
    char *entries[1024];
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (!show_hidden && entry->d_name[0] == '.') {
            continue;
        }
        entries[count++] = strdup(entry->d_name);
    }
    closedir(dir);

    // Sort the entries in lexicographic order
    qsort(entries, count, sizeof(char *), (int (*)(const void *, const void *)) strcmp);

    // Print the entries
    for (int i = 0; i < count; i++) {
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entries[i]);
        if (stat(full_path, &file_stat) == -1) {
            perror("stat error");
            continue;
        }

        if (show_long) {
            print_file_info(&file_stat, entries[i]);
        } else {
            if (S_ISDIR(file_stat.st_mode)) {
                printf(BLUE "%s" RESET "\n", entries[i]);
            } else if (file_stat.st_mode & S_IXUSR) {
                printf(GREEN "%s" RESET "\n", entries[i]);
            } else {
                printf(WHITE "%s" RESET "\n", entries[i]);
            }
        }

        free(entries[i]);
    }
}
