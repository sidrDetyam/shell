
#ifndef SHELL_COMMANDS
#define SHELL_COMMANDS

#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "processtypes.h"
#include "updateprocess.h"

void cd_command(char* currwd, vcharptr_t* argv);

int execute_shell_command(vProcessPipeline* jobs, char* currwd, ProcessPipeline* cmd);

#endif