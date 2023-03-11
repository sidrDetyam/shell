
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../include/updateprocess.h"

int
update_process(vJob *jobs, siginfo_t *infop) {

    for (size_t i = 0; i < jobs->cnt; ++i) {

        for (size_t j = 0; j < jobs->ptr[i].proc.cnt; ++j) {
            Process *p = jobs->ptr[i].proc.ptr + j;

            if (p->pid == infop->si_pid) {
                p->status = infop->si_status;

                if (infop->si_code == CLD_STOPPED) {
                    p->flags |= IS_PROCESS_STOPPED;

                    if (!(jobs->ptr[i].flags & IS_PPL_BG)) {
                        jobs->ptr[i].flags |= IS_PPL_BG;
                        if (tcsetpgrp(0, getpid()) == -1) {
                            perror("tcsetpgrp fail");
                            exit(1);
                        }
                    }
                } else if (infop->si_code == CLD_CONTINUED) {
                    p->flags &= ~IS_PROCESS_STOPPED;
                } else {
                    p->flags &= ~IS_PROCESS_STOPPED;
                    p->flags |= IS_PROCESS_COMPLETED;

                    if (infop->si_code == CLD_KILLED) {
                        p->flags |= IS_END_BY_SIGNAL;
                    }

                    if (jobs->ptr[i].flags & IS_PPL_BG) {
                        if (CLD_KILLED == infop->si_code) {
                            printf("\n[%d]: Signal %d\n", (int) p->pid, p->status);
                        } else {
                            printf("\n[%d]: Exit with code %d\n", (int) p->pid, p->status);
                        }
                    }
                }

                return 0;
            }
        }
    }

    return -1;
}


void
update_status(vJob *jobs) {

    siginfo_t infop;
    int res;
    do {
        infop.si_pid = 0;
        res = waitid(P_ALL, 0, &infop, WEXITED | WSTOPPED | WCONTINUED | WNOHANG);

        if (res == 0 && infop.si_pid != 0) {
            update_process(jobs, &infop);
        }

    } while (res == 0 && infop.si_pid != 0);
}


int
is_job_complete(Job *job) {

    for (size_t i = 0; i < job->proc.cnt; ++i) {
        if (!(job->proc.ptr[i].flags & IS_PROCESS_COMPLETED)) {
            return 0;
        }
    }

    return 1;
}


int
is_job_stopped(Job *job) {

    for (size_t i = 0; i < job->proc.cnt; ++i) {
        if (job->proc.ptr[i].flags & IS_PROCESS_STOPPED) {
            return 1;
        }
    }

    return 0;
}


int
is_job_background(Job *job) {

    return (job->flags & IS_PPL_BG) != 0;
}


void
wait_job(vJob *jobs, Job *job) {

    do {
        update_status(jobs);
    } while (!is_job_complete(job) && !is_job_background(job) && !is_job_stopped(job));
}


int
find_complete_job(vJob *jobs) {

    for (size_t i = 0; i < jobs->cnt; ++i) {
        if (is_job_complete(jobs->ptr + i)) {
            return (int) i;
        }
    }

    return -1;
}

