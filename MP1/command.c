#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "command.h"
#include "input.h"
#include "hop.h"
#include "reveal.h"
#include "log.h"
#include "mysignal.h"
#include "proclore.h"
#include "seek.h"
#include "redirection.h"
#include "pipe.h"
#include "myshrc.h"
#include "activities.h"
#include "ping.h"
#include "fg_bg.h"
#include "neonate.h"
#include "iman.h"

static char prev_dir[1024] = "";

#define RED "\033[0;31m"
#define RESET "\033[0m"

void execute_command(char *input)
{
    // Don't reset foreground_pid until we know it's a foreground command
    input = check_alias(input);

    if (contains_pipe(input))
    {
        execute_pipe(input);
        return;
    }

    char *args[1024];
    char *token;
    int i = 0;
    int is_builtin = 0;                                   // Flag to check if a command is built-in
    int in_redir = -1, out_redir = -1, append_redir = -1; // Redirection flags

    // Split the input into arguments
    token = strtok(input, " ");
    while (token != NULL)
    {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (args[0] == NULL)
    {
        return; // No command entered
    }

    // Check if the command is a built-in command
    if (strcmp(args[0], "cd") == 0)
    {
        execute_cd(args);
        return;
    }
    else if (strcmp(args[0], "pwd") == 0)
    {
        execute_pwd();
        return;
    }
    else if (strcmp(args[0], "export") == 0)
    {
        execute_export(args);
        return;
    }
    else if (strcmp(args[0], "unset") == 0)
    {
        execute_unset(args);
        return;
    }
    else if (strcmp(args[0], "exit") == 0)
    {
        execute_exit();
        return;
    }
    else if (strcmp(args[0], "alias") == 0)
    {
        execute_alias(args);
        return;
    }
    else if (strcmp(args[0], "unalias") == 0)
    {
        execute_unalias(args);
        return;
    }
    else if (strcmp(args[0], "umask") == 0)
    {
        execute_umask(args);
        return;
    }
    else if (strcmp(args[0], "read") == 0)
    {
        execute_read(args);
        return;
    }
    else if (strcmp(args[0], "hop") == 0)
    {
        execute_hop(args);
        return;
    }
    else if (strcmp(args[0], "reveal") == 0)
    {
        execute_reveal(args);
        return;
    }
    else if (strcmp(args[0], "tr") == 0)
    {
        execute_tr(args);
        return;
    }
    else if (strcmp(args[0], "proclore") == 0)
    {
        execute_proclore(args[1]);
        return;
    }
    else if (strcmp(args[0], "mk_hop") == 0 || strcmp(args[0], "hop_reveal") == 0 || strcmp(args[0], "hop_seek") == 0)
    {
        if (args[1] != NULL)
        {
            execute_function(args[0], args[1]);
        }
        else
        {
            printf("Function requires an argument.\n");
        }
        return;
    }
    else if (strcmp(args[0], "log") == 0)
    {
        if (args[1] == NULL)
        {
            display_log();
        }
        else if (strcmp(args[1], "purge") == 0)
        {
            purge_log();
        }
        else if (strcmp(args[1], "execute") == 0 && args[2] != NULL)
        {
            int index = atoi(args[2]);
            execute_log_command(index);
        }
        else
        {
            printf(RED "Invalid log command\n" RESET);
        }
        return;
    }
    else if (strcmp(args[0], "seek") == 0)
    {
        execute_seek(args);
        return;
    }
    else if (strcmp(args[0], "ping") == 0)
    {
        execute_ping(args); // Call the function to send a signal to a process
        return;
    }
    else if (strcmp(args[0], "activities") == 0)
    {
        print_activities(); // Call the function to print the list of activities
        return;
    }
    else if (strcmp(args[0], "fg") == 0)
    {
        if (args[1] == NULL)
        {
            printf("Usage: fg <pid>\n");
        }
        else
        {
            int pid = atoi(args[1]); // Convert the argument to an integer (process ID)
            fg_command(pid);         // Call the fg command
        }
        return; // Early return
    }
    else if (strcmp(args[0], "bg") == 0)
    {
        if (args[1] == NULL)
        {
            printf("Usage: bg <pid>\n");
        }
        else
        {
            int pid = atoi(args[1]); // Convert the argument to an integer (process ID)
            bg_command(pid);         // Call the bg command
        }
        return; // Early return
    }
    else if (strcmp(args[0], "neonate") == 0)
    {
        execute_neonate(args); // Call the neonate function
        return;                // Exit after executing neonate
    }
    else if (strcmp(args[0], "iMan") == 0)
    {
        fetch_man_page(args); // Call iMan command
        return;             // Exit after executing iMan
    }

    // Handle external commands
    if (!is_builtin)
    {
        int is_background = 0;
        if (strcmp(args[i - 1], "&") == 0)
        {
            is_background = 1;
            args[i - 1] = NULL; // Remove '&' from arguments
        }

        // Handle I/O redirection
        handle_io_redirection(args, &in_redir, &out_redir, &append_redir);

        pid_t pid = fork();
        if (pid == 0)
        { // Child process
            if (in_redir != -1)
            {
                dup2(in_redir, STDIN_FILENO);
                close(in_redir);
            }
            if (out_redir != -1)
            {
                dup2(out_redir, STDOUT_FILENO);
                close(out_redir);
            }
            if (append_redir != -1)
            {
                dup2(append_redir, STDOUT_FILENO);
                close(append_redir);
            }

            if (execvp(args[0], args) == -1)
            {
                fprintf(stderr, RED "ERROR: '%s' is not a valid command\n" RESET, args[0]);
            }
            exit(EXIT_FAILURE);
        }
        else if (pid < 0)
        { // Fork failed
            perror("fork() error");
        }
        else
        { // Parent process
            if (is_background)
            {
                // Background process: add to the process list as running in the background
                add_process(pid, args[0], 0);    // Mark as background process
                printf("%d\n", pid);             // Print the PID of the background process
                update_process_state(pid, 1, 0); // Mark process as running in the background
            }
            else
            {
                // Foreground process: add to process list and wait for completion
                add_process(pid, args[0], 1); // Mark as foreground process
                foreground_pid = pid;
                struct timeval start, end;
                gettimeofday(&start, NULL);

                waitpid(pid, NULL, 0); // Wait for the foreground process to complete

                foreground_pid = -1;             // Reset after process completion
                update_process_state(pid, 0, 0); // Mark process as stopped after completion
                gettimeofday(&end, NULL);

                long seconds = end.tv_sec - start.tv_sec;
                if (seconds > 2)
                {
                    printf("Process '%s' took %lds\n", args[0], seconds);
                }
            }
        }
    }
}

void process_input(char *input)
{
    // Check if fgets or the input reading function returned NULL (Ctrl+D is pressed)
    if (input == NULL) {
        
    }

    char *command;
    char *rest = input;

    while ((command = strtok_r(rest, ";", &rest)))
    {
        command = trim_whitespace(command);

        char *background_part = strchr(command, '&');
        if (background_part)
        {
            *background_part = '\0'; // Split the command at '&'
            background_part++;       // Move to the next command
            background_part = trim_whitespace(background_part);

            if (strlen(command) > 0)
            {
                execute_background_command(command);
            }

            if (strlen(background_part) > 0)
            {
                execute_command(background_part);
            }
        }
        else
        {
            execute_command(command);
        }
    }
}


void execute_background_command(char *command)
{
    pid_t pid = fork();
    if (pid == 0)
    { // Child process
        // Execute the command
        char *args[1024];
        char *token;
        int i = 0;

        token = strtok(command, " ");
        while (token != NULL)
        {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        if (execvp(args[0], args) == -1)
        {
            perror("Error executing command");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {                                 // Parent process
        add_process(pid, command, 0); // Track the process as background
        printf("[%d] %d\n", 1, pid);  // Print the PID of the background process
    }
}

void execute_cd(char *args[])
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "ERROR: expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("ERROR");
        }
    }
}

void execute_pwd()
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("%s\n", cwd);
    }
    else
    {
        perror("getcwd() error");
    }
}

void execute_export(char *args[])
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "ERROR: expected argument to \"export\"\n");
        return;
    }
    char *env_var = strtok(args[1], "=");
    char *env_value = strtok(NULL, "=");

    if (env_var && env_value)
    {
        if (setenv(env_var, env_value, 1) != 0)
        {
            perror("ERROR");
        }
    }
    else
    {
        fprintf(stderr, "ERROR: invalid format for \"export\", expected VAR=VALUE\n");
    }
}

void execute_unset(char *args[])
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "ERROR: expected argument to \"unset\"\n");
        return;
    }
    if (unsetenv(args[1]) != 0)
    {
        perror("ERROR");
    }
}

void execute_exit()
{
    exit(0);
}

void execute_alias(char *args[])
{
    fprintf(stderr, "Alias command not implemented\n");
}

void execute_unalias(char *args[])
{
    fprintf(stderr, "Unalias command not implemented\n");
}

void execute_umask(char *args[])
{
    if (args[1] == NULL)
    {
        mode_t current_mask = umask(0);
        umask(current_mask);
        printf("%04o\n", current_mask);
    }
    else
    {
        mode_t new_mask;
        sscanf(args[1], "%o", &new_mask); // Correct format specifier for mode_t
        umask(new_mask);
    }
}

void execute_read(char *args[])
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "ERROR: expected argument to \"read\"\n");
        return;
    }

    printf("%s: ", args[1]);
    char input[1024];
    if (fgets(input, sizeof(input), stdin) != NULL)
    {
        input[strcspn(input, "\n")] = '\0'; // Remove the newline character
        setenv(args[1], input, 1);
    }
    else
    {
        perror("ERROR");
    }
}

void execute_tr(char *args[])
{
    if (args[1] == NULL || args[2] == NULL)
    {
        fprintf(stderr, "ERROR: tr requires two arguments\n");
        return;
    }

    char *set1 = args[1];
    char *set2 = args[2];

    // Check if both sets have the same length
    if (strlen(set1) != strlen(set2))
    {
        fprintf(stderr, "ERROR: tr sets must be the same length\n");
        return;
    }

    int c;
    while ((c = getchar()) != EOF)
    {
        char *pos = strchr(set1, c);
        if (pos)
        {
            // Replace character with corresponding character from set2
            putchar(set2[pos - set1]);
        }
        else
        {
            putchar(c); // Print character as is if it's not in set1
        }
    }
}
