//
// Created by argem on 17.04.2022.
//

#ifndef SHELL_RUNPROCESS_H
#define SHELL_RUNPROCESS_H

#include "processtypes.h"

__attribute__((unused)) typedef void (*sighandler_t)(int);

void
set_sigs(int is_block);

int
start_job(Job *ppl, int fg);


#endif //SHELL_RUNPROCESS_H
