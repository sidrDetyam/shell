
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include "shell.h"
#include "process_types.h"


int main(int argc, char *argv[]){

    signal (SIGINT, SIG_DFL);
    signal (SIGQUIT, SIG_IGN);
    signal (SIGTSTP, SIG_IGN);
    signal (SIGTTIN, SIG_IGN);
    signal (SIGTTOU, SIG_IGN);
    signal (SIGCHLD, SIG_IGN);

    char prompt[128] = "shell$ ";
    char line[1024];

    while (1){

        promptline(prompt, line, sizeof(line));
        ProcessPipeline* ppls;
        int nppls = parseline(line, &ppls);

        if(nppls<0){
            printf("\nsyntax error\n\n");
            continue;
        }

        for (int i = 0; i < nppls; i++) {


            launch_ppl(ppls+i, 1);
            waitpid(-1, NULL, 0);


            void (*previous_handler)(int) = sigset(SIGTTOU, SIG_IGN);
            tcsetpgrp(0, getpid());
            sigset(SIGTTOU, previous_handler);
        }
    }

    return 0;
}
