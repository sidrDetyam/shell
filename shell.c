
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>

#include "processtypes.h"
#include "shell.h"
#include "updateprocess.h"
#include "runprocess.h"
#include "shellcommands.h"


int main(int argc, char *argv[]){

    signal (SIGINT, SIG_IGN);
    signal (SIGQUIT, SIG_IGN);
    signal (SIGTSTP, SIG_IGN);
    signal (SIGTTIN, SIG_IGN);
    signal (SIGTTOU, SIG_IGN);

    char currwd[PATH_MAX];
    char line[MAX_CMD_LENGTH];
    cd_command(currwd, NULL);

    vProcessPipeline jobs;
    vProcessPipeline_init(&jobs);

    while (1){

        promptline(currwd, line, sizeof(line));
        ProcessPipeline* ppls;
        int nppls = parse_line(line, &ppls);

        if(nppls<0){
            printf("\nsyntax error\n\n");
            continue;
        }

        for (int i=0; i < nppls; i++) {

            if(execute_shell_command(&jobs, currwd, ppls+i)){
                continue;
            }

            int ind = find_complete_job(&jobs);
            if(ind==-1){
                vProcessPipeline_push_back(&jobs, ppls + i);
                ind = jobs.cnt-1;
            }
            else{
                for(int j=0; j<jobs.ptr[ind].proc.cnt; ++j){
                    vcharptr_t_free_ptr(&jobs.ptr[ind].proc.ptr[j].argv);
                }
                free(jobs.ptr[ind].cmd);
                vProcessPipeline_assign(&jobs, ppls+i, ind);
            }

            int res;
            if(ppls[i].flags & IS_PPL_BG) {
                res = start_ppl(jobs.ptr+ind, 0);
                printf("[%d] %d\n", ind, (int)jobs.ptr[ind].pgid);
            }
            else {
                res = start_ppl(jobs.ptr+ind, 1);
                wait_job(&jobs, jobs.ptr+ind);
            }
            
            if(res<0){
                printf("\nfail with in/out files\n\n");
            }

            tcsetpgrp(0, getpid());
        }
        free(ppls);

        update_status(&jobs);
    }

    return 0;
}
