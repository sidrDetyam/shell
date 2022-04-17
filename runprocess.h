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


int start_ppl(ProcessPipeline *ppl, int fg);


#endif //SHELL_RUNPROCESS_H
