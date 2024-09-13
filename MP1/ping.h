#ifndef PING_H
#define PING_H

extern pid_t foreground_pid;

void execute_ping(char *args[]);

void setup_signal_handlers();
void handle_sigint(int sig);
void handle_sigtstp(int sig);

#endif
