#ifndef FG_BG_H
#define FG_BG_H

// Bring a process to the foreground and give it terminal control
void fg_command(int pid);

// Resume a stopped background process
void bg_command(int pid);

#endif
