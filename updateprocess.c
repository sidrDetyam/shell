
#include "updateprocess.h"

int update_process(vProcessPipeline* ppls, siginfo_t *infop){

    for(int i=0; i<ppls->cnt; ++i) {

        for (int j = 0; j < ppls->ptr[i].proc.cnt; ++j) {
            Process *p = ppls->ptr[i].proc.ptr + j;

            if (p->pid == infop->si_pid) {
                p->status = infop->si_status;

                if (infop->si_code == CLD_STOPPED) {
                    p->flags |= IS_PROCESS_STOPPED;
                    
                    if(!(ppls->ptr[i].flags & IS_PPL_BG)){
                        ppls->ptr[i].flags |= IS_PPL_BG;
                        tcsetpgrp(0, getpgid(getpid()));
                    }
                }
                else if(infop->si_code == CLD_CONTINUED){
                    p->flags &= ~IS_PROCESS_STOPPED;
                } 
                else {
                    p->flags &= ~IS_PROCESS_STOPPED;
                    p->flags |= IS_PROCESS_COMPLETED;

                    if(infop->si_code==CLD_KILLED){
                        p->flags |= IS_END_BY_SIGNAL;
                    }

                    if(ppls->ptr[i].flags & IS_PPL_BG){
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


void update_status(vProcessPipeline* ppls){

    siginfo_t infop;
    int res;
    do{
        infop.si_pid = 0;
        res = waitid(P_ALL, 0, &infop, WEXITED | WSTOPPED | WCONTINUED | WNOHANG);

        if(res==0 && infop.si_pid!=0) {
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


int is_ppl_stopped(ProcessPipeline* ppl){

    for(int i=0; i<ppl->proc.cnt; ++i){
        if(ppl->proc.ptr[i].flags & IS_PROCESS_STOPPED){
            return 1;
        }
    }

    return 0;
}


int is_ppl_background(ProcessPipeline* ppl){

    return (ppl->flags & IS_PPL_BG) != 0;
}


void wait_job(vProcessPipeline* jobs, ProcessPipeline* job){

    do{
        update_status(jobs);
    }while(!is_ppl_complete(job) && !is_ppl_background(job) && !is_ppl_stopped(job));
}


int find_complete_job(vProcessPipeline* ppls){

    for(int i=0; i<ppls->cnt; ++i){
        if(is_ppl_complete(ppls->ptr + i)){
            return i;
        }
    }

    return -1;
}

