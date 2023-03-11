
#ifndef SHELL_COMMANDS
#define SHELL_COMMANDS

#include "processtypes.h"
#include "updateprocess.h"

void
cd_command(char *currwd, vcharptr_t *argv);

int
execute_shell_command(vJob *jobs, char *currwd, Job *cmd);

#endif