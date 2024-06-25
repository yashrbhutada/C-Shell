#ifndef __REDIRECT_H
#define __REDIRECT_H

typedef int bool;

#define true 1
#define false 0

char inpFile[256];
char outFile[256];
char arg[256];

bool iR;
bool oR;
bool to_append;

void check_redirection(char *prompt);
bool do_redirection();

#endif