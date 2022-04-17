
#include "runprocess.h"

static void start_process (Process *p, pid_t pgid,
                    int infd, int outfd, int errfd,
                    int fg){

    pid_t pid = getpid();
    if (pgid == 0){
        pgid = pid;
    }

    setpgid(pid, pgid);
    if(fg){
        tcsetpgrp(0, pgid);
    }

    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);


    if (infd != STDIN_FILENO){
        dup2(infd, STDIN_FILENO);
        close (infd);
    }

    if (outfd != STDOUT_FILENO){
        dup2(outfd, STDOUT_FILENO);
        close(outfd);
    }

    if (errfd != STDERR_FILENO){
        dup2(errfd, STDERR_FILENO);
        close(errfd);
    }

    execvp(p->argv.ptr[0], p->argv.ptr);
    perror("execvp fail");
    exit(1);
}


int start_ppl(ProcessPipeline *ppl, int fg){

    pid_t pid, pgid;
    int procpipe[2], infile, outfile, outfd, infd;

    if(ppl->flags & IS_PPL_IN_FILE) {
        infile = open(ppl->infile, O_RDONLY);
        if (infile == -1) {
            return -1;
        }
    }
    else{
        infile = STDIN_FILENO;
    }

    if(ppl->flags & IS_PPL_OUT_FILE){
        outfile = open(ppl->outfile,
                       O_WRONLY | O_CREAT | (ppl->flags & IS_PPL_OUT_APPEND? O_APPEND : O_TRUNC),
                       0777);

        if (outfile == -1) {
            if(ppl->flags & IS_PPL_IN_FILE){
                close(infile);
            }
            return -2;
        }
    }
    else{
        outfile = STDOUT_FILENO;
    }


    for(int i=0; i < ppl->proc.cnt; ++i){

        infd = i==0? infile : procpipe[0];

        if(i+1 != ppl->proc.cnt){
            if (pipe (procpipe) < 0){
                perror ("pipe fail");
                exit (1);
            }
            outfd = procpipe[1];
        }
        else{
            outfd = outfile;
        }

        pid = fork ();
        if (pid < 0){
            perror ("fork fail");
            exit (1);
        }

        if (pid == 0) {
            start_process(ppl->proc.ptr+i, i==0? getpid() : ppl->pgid, infd,
                            outfd, STDERR_FILENO, fg);
        }
        else{
            if(i==0) {
                ppl->pgid = pid;
                setpgid(pid, ppl->pgid);
                if(fg) {
                    tcsetpgrp(0, ppl->pgid);
                }
            }
            ppl->proc.ptr[i].pid = pid;
            ppl->proc.ptr[i].flags = 0;
            ppl->proc.ptr[i].status = 0;
        }

        if (infd != STDIN_FILENO) {
            close(infd);
        }
        if (outfd != STDOUT_FILENO) {
            close(outfd);
        }
    }

    return 0;
}
