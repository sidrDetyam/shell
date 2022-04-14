
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include "shell.h"
#include <sys/wait.h>
#include "process_types.h"


#define ELEMENT_TYPE ProcessPipeline
#include "CVector.h"


int update_process(vProcessPipeline* ppls, siginfo_t *infop){

    for(int i=0; i<ppls->cnt; ++i) {

        for (int j = 0; j < ppls->ptr[i].proc.cnt; ++j) {
            Process *p = ppls->ptr[i].proc.ptr + j;

            //printf(" -- %d\n", (int) p->pid);
            if (p->pid == infop->si_pid) {
                p->status = infop->si_status;
                if (infop->si_code == CLD_STOPPED) {
                    p->flags |= IS_PROCESS_STOPPED;
                } else {
                    p->flags &= ~IS_PROCESS_STOPPED;
                    p->flags |= IS_PROCESS_COMPLETED;
                    if (CLD_KILLED == infop->si_code) {
                        printf("%d: Terminated by signal %d.\n",
                               (int) p->pid, p->status);
                    } else {
                        printf("%d: Exit with code %d.\n",
                               (int) p->pid, p->status);
                    }
                }
                return 0;
            }
        }
    }

    return -1;
}


void update_status(vProcessPipeline* ppls){

    siginfo_t infop;
    int res;
    do{
        infop.si_pid = 0;
        res = waitid(P_ALL, 0, &infop, WEXITED | WSTOPPED | WCONTINUED | WNOHANG);

        if(res==0 && infop.si_pid!=0) {
            //printf("here  %d\n", (int) infop.si_pid);
            update_process(ppls, &infop);
        }

    }while(res==0 && infop.si_pid!=0);
}


int is_ppl_complete(ProcessPipeline* ppl){

    for(int i=0; i<ppl->proc.cnt; ++i){
        if(!(ppl->proc.ptr[i].flags & IS_PROCESS_COMPLETED)){
            return 0;
        }
    }

    return 1;
}

void wait_job(vProcessPipeline* jobs, ProcessPipeline* job){

    while(!is_ppl_complete(job)){
        update_status(jobs);
    }
}



int main(int argc, char *argv[]){

    signal (SIGINT, SIG_DFL);
    signal (SIGQUIT, SIG_IGN);
    signal (SIGTSTP, SIG_IGN);
    signal (SIGTTIN, SIG_IGN);
    signal (SIGTTOU, SIG_IGN);
    //signal (SIGCHLD, SIG_IGN);

    char prompt[128] = "shell$ ";
    char line[1024];

    vProcessPipeline jobs;
    vProcessPipeline_init(&jobs);

    while (1){

        promptline(prompt, line, sizeof(line));
        ProcessPipeline* ppls;
        int nppls = parseline(line, &ppls);

        if(nppls<0){
            printf("\nsyntax error\n\n");
            continue;
        }

        for (int i = 0; i < nppls; i++) {

            vProcessPipeline_push_back(&jobs, ppls + i);

            if(ppls[i].flags & IS_BG) {
                launch_ppl(vProcessPipeline_back(&jobs), 0);
            }
            else {
                launch_ppl(vProcessPipeline_back(&jobs), 1);
                wait_job(&jobs, vProcessPipeline_back(&jobs));
            }

            void (*previous_handler)(int) = sigset(SIGTTOU, SIG_IGN);
            tcsetpgrp(0, getpid());
            sigset(SIGTTOU, previous_handler);
        }

        update_status(&jobs);
    }

    return 0;
}
