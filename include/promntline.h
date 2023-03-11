
#ifndef PROMNTLINE
#define PROMNTLINE

#include "processtypes.h"

#define MAX_CMD_LENGTH 4096

void
promptline(const char *prompt, char *line, vcharptr_t *history);

#endif
