
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/shellcommands.h"


static long
get_index(vJob *jobs, vcharptr_t *argv) {

    if (argv->cnt != 3) {
        printf("\nIncorrect count of argc\n\n");
        return -1;
    }

    errno = 0;
    char *end;
    long ind = strtol(argv->ptr[1], &end, 10);
    if (errno != 0 || end - argv->ptr[1] != strlen(argv->ptr[1])) {
        printf("\nIncorrect index\n\n");
        return -1;
    }

    if (ind < 0 || jobs->cnt <= ind || is_job_complete(jobs->ptr + ind)) {
        printf("\nNo such job\n\n");
        return -1;
    }

    return ind;
}


static void
bg_command(vJob *jobs, vcharptr_t *argv) {

    long ind = get_index(jobs, argv);
    if (ind == -1) {
        return;
    }

    Job *job = jobs->ptr + ind;
    if (is_job_stopped(job)) {
        if (kill(-job->pgid, SIGCONT) == -1) {
            perror("kill cont fail");
            exit(1);
        }

        return;
    }

    printf("\nJob already running\n\n");
}


static void
fg_command(vJob *jobs, vcharptr_t *argv) {

    long ind = get_index(jobs, argv);
    if (ind == -1) {
        return;
    }

    Job *job = jobs->ptr + ind;
    printf("%s\n", job->cmd);
    if (tcsetpgrp(0, job->pgid) == -1) {
        perror("tcsetpgrp fail");
        exit(1);
    }

    job->flags &= ~IS_PPL_BG;

    if (kill(-job->pgid, SIGCONT) == -1) {
        perror("kill (SIGCONT) fail");
        exit(1);
    }

    wait_job(jobs, job);
    if (tcsetpgrp(0, getpgid(getpid())) == -1) {
        perror("tcsetpgrp fail");
        exit(1);
    }
}


static void
jobs_command(vJob *jobs) {

    printf("\n");
    for (size_t i = 0; i < jobs->cnt; ++i) {
        if (!is_job_complete(jobs->ptr + i)) {
            printf("[%zu] %s    %s\n",
                   i,
                   is_job_stopped(jobs->ptr + i) ? "stopped" : "running",
                   jobs->ptr[i].cmd
            );
        }
    }
    printf("\n");
}


static void
exit_command() {
    exit(0);
}


static void
kill_command(vJob *jobs, vcharptr_t *argv) {

    long ind = get_index(jobs, argv);
    if (ind == -1) {
        return;
    }

    Job *job = jobs->ptr + ind;
    if (kill(-job->pgid, SIGKILL) == -1) {
        perror("kill (SIGCONT) fail");
        exit(1);
    }
}

//if argv == NULL, new wd = "."
void
cd_command(char *currwd, vcharptr_t *argv) {

    if (argv && argv->cnt != 3) {
        printf("\nIncorrect count of args for cd\n\n");
        return;
    }

    if (chdir(argv ? argv->ptr[1] : ".") == -1) {
        perror("cd fail");
        return;
    }

    if (getcwd(currwd, PATH_MAX) == NULL) {
        perror("getwd fail");
        exit(1);
    }
}


int
execute_shell_command(vJob *jobs, char *currwd, Job *cmd) {

    if (cmd->proc.cnt != 1 || cmd->flags & (IS_PPL_IN_FILE | IS_PPL_OUT_APPEND | IS_PPL_BG)) {
        return 0;
    }

    Process *p = cmd->proc.ptr;
    if (strcmp(p->argv.ptr[0], "fg") == 0) {
        fg_command(jobs, &p->argv);
        return 1;
    }
    if (strcmp(p->argv.ptr[0], "bg") == 0) {
        bg_command(jobs, &p->argv);
        return 1;
    }
    if (strcmp(p->argv.ptr[0], "jobs") == 0) {
        jobs_command(jobs);
        return 1;
    }
    if (strcmp(p->argv.ptr[0], "exit") == 0) {
        exit_command();
        return 1; //unreachable
    }
    if (strcmp(p->argv.ptr[0], "killjob") == 0) {
        kill_command(jobs, &p->argv);
        return 1;
    }
    if (strcmp(p->argv.ptr[0], "cd") == 0) {
        cd_command(currwd, &p->argv);
        return 1;
    }

    return 0;
}
