
#ifndef UPDATE_PROCESS
#define UPDATE_PROCESS

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <sys/wait.h>
#include "processtypes.h"

int update_process(vProcessPipeline* ppls, siginfo_t *infop);

void update_status(vProcessPipeline* ppls);

int is_ppl_complete(ProcessPipeline* ppl);

int is_ppl_stopped(ProcessPipeline* ppl);

int is_ppl_background(ProcessPipeline* ppl);

void wait_job(vProcessPipeline* jobs, ProcessPipeline* job);

int find_complete_job(vProcessPipeline* ppls);

#endif
