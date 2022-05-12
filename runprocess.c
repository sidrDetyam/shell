
#include "runprocess.h"


static int intsfopen(const char* pathname, int flags, mode_t mode){
    int fd;
    while((fd = open(pathname, flags, mode))==-1){
        if(errno!=EINTR){
            return -1;
        }
    }
    return fd;
}


static int intsfdup2(int oldfd, int newfd){
    int fd;
    while((fd = dup2(oldfd, newfd))==-1){
        if(errno!=EINTR) {
            return -1;
        }
    }
    return fd;
}


static int intsfclose(int fd){
    while(close(fd) == -1){
        if(errno!=EINTR) {
            return -1;
        }
    }
    return 0;
}


static void input_redirection(int oldfd, int newfd){

    if(oldfd != newfd){
        if(intsfdup2(oldfd, newfd)==-1 || intsfclose(oldfd)==-1){
            perror("dup2 || close fail");
            exit(1);
        }
    }
}


void set_sigs(int is_block){

    sigset_t mask;
    sigemptyset(&mask);
    if(is_block) {
        sigaddset(&mask, SIGINT);
        sigaddset(&mask, SIGQUIT);
        sigaddset(&mask, SIGTSTP);
        sigaddset(&mask, SIGTTIN);
        sigaddset(&mask, SIGTTOU);
    }
    sigprocmask(SIG_SETMASK, &mask, NULL);
}


static void start_process (Process *p, pid_t pgid,
                    int infd, int outfd, int errfd,
                    int fg){

    pid_t pid = getpid();
    if (pgid == 0){
        pgid = pid;
    }

    if(setpgid(pid, pgid)==-1){
        perror("setpgid fail");
        exit(1);
    }
    if(fg){
        if(tcsetpgrp(0, pgid)==-1){
            perror("tcsetpgrp fail");
            exit(1);
        }
    }

    set_sigs(0);

    input_redirection(infd, STDIN_FILENO);
    input_redirection(outfd, STDOUT_FILENO);
    input_redirection(errfd, STDERR_FILENO);

    execvp(p->argv.ptr[0], p->argv.ptr);
    perror("execvp fail");
    exit(1);
}


int start_job(Job *job, int fg){

    pid_t pid, pgid;
    int procpipe[2], infile, outfile, outfd, infd;

    if(job->flags & IS_PPL_IN_FILE) {
        infile = intsfopen(job->infile, O_RDONLY, 0);
        if (infile == -1) {
            return -1;
        }
    }
    else{
        infile = STDIN_FILENO;
    }

    if(job->flags & IS_PPL_OUT_FILE){
        outfile = intsfopen(job->outfile,
                       O_WRONLY | O_CREAT | (job->flags & IS_PPL_OUT_APPEND ? O_APPEND : O_TRUNC),
                       S_IWUSR | S_IRUSR);

        if (outfile == -1) {
            if(job->flags & IS_PPL_IN_FILE){
                if(intsfclose(infile)==-1){
                    perror("close fail");
                    exit(1);
                }
            }
            return -2;
        }
    }
    else{
        outfile = STDOUT_FILENO;
    }


    for(size_t i=0; i < job->proc.cnt; ++i){

        infd = i==0? infile : procpipe[0];

        if(i+1 != job->proc.cnt){
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
            start_process(job->proc.ptr + i, i == 0 ? getpid() : job->pgid, infd,
                          outfd, STDERR_FILENO, fg);
        }
        else{
            if(i==0) {
                job->pgid = pid;
                /*
                 Call setpgid to avoid race condition when child 0
                 hasn't called setpgid yet and we're already running the next one.
                 There is no need to check for failure,
                 because it will only if child 0 has already called execvp
                 */
                setpgid(pid, job->pgid);
                if(fg) {
                    if(tcsetpgrp(0, job->pgid) == -1){
                        perror("tcsetpgrp fail");
                        exit(1);
                    }
                }
            }
            job->proc.ptr[i].pid = pid;
            job->proc.ptr[i].flags = 0;
            job->proc.ptr[i].status = 0;
        }

        if(infd!=STDIN_FILENO && intsfclose(infd)==-1 ||
            outfd!=STDOUT_FILENO && intsfclose(outfd)==-1){

            perror("close fail");
            exit(1);
        }
    }

    return 0;
}
