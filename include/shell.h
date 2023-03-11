
#ifndef SHELL
#define SHELL

#include <stdio.h>

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include "processtypes.h"

int parse_line(char* line, Job** jobs);

#define MAX_CMD_LENGTH 4096
void promptline(const char *prompt, char *line, vcharptr_t* history);

#endif
