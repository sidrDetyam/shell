
#include "shellcommands.h"


static void bg_command(vProcessPipeline* jobs, vcharptr_t* argv){

    if(argv->cnt != 3){
        printf("\nIncorrect count of argc for bg\n\n");
        return;
    }

    errno = 0;
    long ind = strtol(argv->ptr[1], NULL, 10);
    if(errno!=0){
        printf("\nIncorrect index for bg\n");
        return;
    }

    if(ind<0 || jobs->cnt<=ind || is_ppl_complete(jobs->ptr + ind)){
        printf("\nNo such job\n\n");
        return;
    }

    ProcessPipeline *job = jobs->ptr + ind;
    if(is_ppl_stopped(job)){
        if(kill(-job->pgid, SIGCONT)==-1){
            perror("kill cont fail");
            exit(1);
        }

        return;
    }

    printf("\nJob already running\n\n");
    return;
}


static void fg_command(vProcessPipeline* jobs, vcharptr_t* argv){

    if(argv->cnt != 3){
        printf("\nIncorrect count of argc for fg\n\n");
        return;
    }

    errno = 0;
    long ind = strtol(argv->ptr[1], NULL, 10);
    if(errno!=0){
        printf("\nIncorrect index for fg\n");
        return;
    }

    if(ind<0 || jobs->cnt<=ind || is_ppl_complete(jobs->ptr + ind)){
        printf("\nNo such job\n\n");
        return;
    }

    ProcessPipeline *job = jobs->ptr + ind;
    printf("%s\n", job->cmd);
    if(tcsetpgrp(0, job->pgid)==-1){
        perror("tcsetpgrp fail");
        exit(1);
    }

    job->flags &= ~IS_PPL_BG;

    if(kill(-job->pgid, SIGCONT) == -1) {
        perror("kill (SIGCONT) fail");
        exit(1);
    }

    wait_job(jobs, job);
    if(tcsetpgrp(0, getpgid(getpid()))==-1){
        perror("tcsetpgrp fail");
        exit(1);
    }
}


static void jobs_command(vProcessPipeline* jobs){

    printf("\n");
    for(size_t i=0; i<jobs->cnt; ++i){
        if(!is_ppl_complete(jobs->ptr+i)){
            printf("[%zu] %s    %s\n",
                   i,
                   is_ppl_stopped(jobs->ptr+i)? "stopped":"running",
                   jobs->ptr[i].cmd
            );
        }
    }
    printf("\n");
}


static void exit_command() {
    exit(0);
}


static void kill_command(vProcessPipeline* jobs, vcharptr_t* argv){

    if(argv->cnt != 3){
        printf("\nIncorrect count of argc for killjob\n\n");
        return;
    }

    errno = 0;
    long ind = strtol(argv->ptr[1], NULL, 10);
    if(errno!=0){
        printf("\nIncorrect index for killjob\n");
        return;
    }

    if(ind<0 || jobs->cnt<=ind || is_ppl_complete(jobs->ptr + ind)){
        printf("\nNo such job\n\n");
        return;
    }

    ProcessPipeline *job = jobs->ptr + ind;
    if(kill(-job->pgid, SIGKILL) == -1) {
        perror("kill (SIGCONT) fail");
        exit(1);
    }
}

//if argv == NULL, new wd = "."
void cd_command(char* currwd, vcharptr_t* argv){

    if(argv && argv->cnt!=3){
        printf("\nIncorrect count of args for cd\n\n");
        return;
    }

    if(chdir(argv? argv->ptr[1] : ".")==-1){
        perror("cd fail");
        return;
    }

    if(getcwd(currwd, PATH_MAX)==NULL){
        perror("getwd fail");
        exit(1);
    }

    size_t tmp = strlen(currwd);
    currwd[tmp] = '$';
    currwd[tmp+1] = ' ';
    currwd[tmp+2] = '\0';
}


int execute_shell_command(vProcessPipeline* jobs, char* currwd, ProcessPipeline* cmd){

    if(cmd->proc.cnt!=1 || cmd->flags & (IS_PPL_IN_FILE | IS_PPL_OUT_APPEND | IS_PPL_BG)){
        return 0;
    }

    Process* p = cmd->proc.ptr;
    if(strcmp(p->argv.ptr[0], "fg")==0) {
        fg_command(jobs, &p->argv);
        return 1;
    }
    if(strcmp(p->argv.ptr[0], "bg")==0) {
        bg_command(jobs, &p->argv);
        return 1;
    }
    if(strcmp(p->argv.ptr[0], "jobs")==0) {
        jobs_command(jobs);
        return 1;
    }
    if(strcmp(p->argv.ptr[0], "exit")==0) {
        exit_command();
        return 1; //unreachable
    }
    if(strcmp(p->argv.ptr[0], "killjob")==0) {
        kill_command(jobs, &p->argv);
        return 1;
    }
    if(strcmp(p->argv.ptr[0], "cd")==0) {
        cd_command(currwd, &p->argv);
        return 1;
    }

    return 0;
}
