
#ifndef SHELL
#define SHELL

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "processtypes.h"

int parse_line(char* line, Job** jobs);

#define MAX_CMD_LENGTH 4096
int promptline(char *prompt, char *line, int sizline);

#endif
