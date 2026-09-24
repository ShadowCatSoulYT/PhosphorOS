#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdbool.h>
#include <flanterm.h>

extern struct flanterm_context *terminal;

void write(char *text, int color, bool light);
void nwrite(char *text, int color, bool light);

#endif