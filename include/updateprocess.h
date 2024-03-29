
#ifndef UPDATE_PROCESS
#define UPDATE_PROCESS

#include <signal.h>
#include "processtypes.h"

int
update_process(vJob *jobs, siginfo_t *infop);

void
update_status(vJob *jobs);

int
is_job_complete(Job *ppl);

int
is_job_stopped(Job *job);

int
is_job_background(Job *job);

void
wait_job(vJob *jobs, Job *job);

int
find_complete_job(vJob *jobs);

#endif
