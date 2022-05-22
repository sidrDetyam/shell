
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

#include "processtypes.h"
#include "shell.h"
#include "updateprocess.h"
#include "runprocess.h"
#include "shellcommands.h"


int main(int argc, char *argv[]){

    if(!isatty(0) || !isatty(1)){
        perror("TTY");
        exit(1);
    }
    set_sigs(1);

    pid_t shell_pid = getpid();
    if(setpgid(shell_pid, shell_pid)==-1){
        perror("setpgid fail");
        exit(1);
    }

    char currwd[PATH_MAX];
    char line[MAX_CMD_LENGTH];
    cd_command(currwd, NULL);

    vJob jobs;
    vJob_init(&jobs);

    vcharptr_t cmd_history;
    vcharptr_t_init(&cmd_history);

    while (1){

        promptline(currwd, line, &cmd_history);
        char* cmd = malloc(strlen(line)+1);
        if(cmd==NULL){
            perror("allocation fail");
            exit(1);
        }

        if(strlen(line)==0){
            continue;
        }
        strcpy(cmd, line);
        if(cmd_history.cnt==0 || strcmp(*vcharptr_t_back(&cmd_history), line)!=0){
            vcharptr_t_push_back(&cmd_history, &cmd);
        }

        Job* parsed_jobs;
        int jobs_count = parse_line(line, &parsed_jobs);

        if(jobs_count < 0){
            printf("\nsyntax error\n\n");
            continue;
        }

        for (size_t i=0; i < jobs_count; i++) {

            if(execute_shell_command(&jobs, currwd, parsed_jobs + i)){
                continue;
            }

            int ind = find_complete_job(&jobs);
            if(ind==-1){
                vJob_push_back(&jobs, parsed_jobs + i);
                ind = (int) jobs.cnt-1;
            }
            else{
                for(int j=0; j<jobs.ptr[ind].proc.cnt; ++j){
                    vcharptr_t_free_ptr(&jobs.ptr[ind].proc.ptr[j].argv);
                }
                free(jobs.ptr[ind].cmd);
                vJob_assign(&jobs, parsed_jobs + i, ind);
            }

            int res;
            if(parsed_jobs[i].flags & IS_PPL_BG) {
                res = start_job(jobs.ptr+ind, 0);
                printf("[%d] %d\n", ind, (int)jobs.ptr[ind].pgid);
            }
            else {
                res = start_job(jobs.ptr+ind, 1);
                wait_job(&jobs, jobs.ptr+ind);
            }
            
            if(res<0){
                printf("\nfail with in/out files\n\n");
            }

            if(tcsetpgrp(0, shell_pid)==-1){
                perror("tcsetpgrp fail");
                exit(1);
            }
        }
        free(parsed_jobs);

        update_status(&jobs);
    }

    return 0;
}
