#ifndef COMMAND_H
#define COMMAND_H

void execute_command(char *input);
void process_input(char *input);
void execute_background_command(char *command);
void execute_cd(char *args[]);
void execute_pwd();
void execute_export(char *args[]);
void execute_unset(char *args[]);
void execute_exit();
void execute_alias(char *args[]);
void execute_unalias(char *args[]);
void execute_umask(char *args[]);
void execute_read(char *args[]);

#endif
