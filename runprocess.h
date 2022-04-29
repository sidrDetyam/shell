//
// Created by argem on 17.04.2022.
//

#ifndef SHELL_RUNPROCESS_H
#define SHELL_RUNPROCESS_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include "processtypes.h"

typedef void (*sighandler_t)(int);

void set_sigs(sighandler_t handler);

int start_job(Job *ppl, int fg);


#endif //SHELL_RUNPROCESS_H
