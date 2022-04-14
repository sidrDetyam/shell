
#include<stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include "process_types.h"

void launch_process (Process *p,
                     pid_t pgid,
                     int infile, int outfile, int errfile,
                     int foreground){

    pid_t pid = getpid();
    if (pgid == 0){
        pgid = pid;
    }

    setpgid (pid, pgid);
    if(foreground) {
        tcsetpgrp(0, pgid);
    }

    signal (SIGINT, SIG_DFL);
    signal (SIGQUIT, SIG_DFL);
    signal (SIGTSTP, SIG_DFL);
    signal (SIGTTIN, SIG_DFL);
    signal (SIGTTOU, SIG_DFL);
    signal (SIGCHLD, SIG_DFL);


    if (infile != STDIN_FILENO){
        dup2 (infile, STDIN_FILENO);
        close (infile);
    }

    if (outfile != STDOUT_FILENO){
        dup2 (outfile, STDOUT_FILENO);
        close (outfile);
    }

    if (errfile != STDERR_FILENO){
        dup2 (errfile, STDERR_FILENO);
        close (errfile);
    }

    execvp (p->argv.ptr[0], p->argv.ptr);
    perror ("execvp fail");
    exit (1);
}


int launch_ppl (ProcessPipeline *ppl, int foreground){

    pid_t pid, pgid;
    int mypipe[2], infile, outfile, outfd, infd;

    if(ppl->flags & IS_IN_FILE) {
        infile = open(ppl->infile, O_RDONLY);
        if (infile == -1) {
            return -1;
        }
    }
    else{
        infile = STDIN_FILENO;
    }

    if(ppl->flags & IS_OUT_FILE){
        outfile = open(ppl->outfile,
                       O_WRONLY | O_CREAT | (ppl->flags & IS_OUT_APPEND? O_APPEND : 0),
                       0777);

        printf("\n%d\n", ppl->flags & IS_OUT_APPEND);

        if(ppl->flags & IS_IN_FILE){
            close(infile);
        }

        if (outfile == -1) {
            return -2;
        }
    }
    else{
        outfile = STDOUT_FILENO;
    }


    for(int i=0; i < ppl->proc.cnt; ++i){

        infd = i==0? infile : mypipe[0];

        if(i+1 != ppl->proc.cnt){
            if (pipe (mypipe) < 0){
                perror ("pipe fail");
                exit (1);
            }
            outfd = mypipe[1];
        }
        else{
            outfd = outfile;
        }

        pid = fork ();
        if (pid == 0) {
            launch_process(ppl->proc.ptr+i, i==0? getpid() : ppl->pgid, infd,
                           outfd, STDERR_FILENO, foreground);
        }
        else if (pid < 0){
            perror ("fork fail");
            exit (1);
        }
        else{
            if(i==0) {
                ppl->pgid = pid;
            }
            setpgid(pid, ppl->pgid);
        }

        if (infd != STDIN_FILENO) {
            close(infd);
        }
        if (outfd != STDOUT_FILENO) {
            close(outfd);
        }
    }
}

