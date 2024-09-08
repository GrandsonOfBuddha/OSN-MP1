#ifndef REDIRECTION_H
#define REDIRECTION_H

void handle_io_redirection(char *args[], int *in_redir, int *out_redir, int *append_redir);
int handle_file_redirection(char *filename, int direction, int append);

#endif
