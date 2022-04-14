
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include "shell.h"
#include "process_types.h"

char *infile, *outfile, *appfile;
struct command cmds[MAXCMDS];
char bkgrnd;

//#define DEBUG


// #define IS_BG 1
// #define IS_IN_FILE 2
// #define IS_OUT_FILE 4
// #define IS_OUT_APPEND 8


// #define ELEMENT_TYPE Process
// #include"CVector.h"

// struct ProcessPipeline{

//     vProcess proc;
//     pid_t pgid;
//     int flags;

//     char* outfile;
//     char* infile;
// };


int main(int argc, char *argv[]){


    char s[1024] = "  cat foo.c < input.txt | sort | ls > out.txt; ls -lai & ";
    printf("Command: %s\n\n", s);

    ProcessPipeline* ppls;
    int res = parse_line(s, &ppls);

    if(res==-1){
        printf("syntax error\n");
    }
    else{

        printf("Cnt of ppls: %d\n\n", res);
        for(int i=0; i<res; ++i){
            
            printf("Cnt of cmds: %d\n", ppls[i].proc.cnt);
            printf("infile: %s %d\n", ppls[i].infile, ppls[i].flags & IS_IN_FILE);
            printf("outfile: %s %d\n", ppls[i].outfile, ppls[i].flags & IS_OUT_FILE);

            for(int j=0; j<ppls[i].proc.cnt; ++j){
                printf("Command %d:\n", j);
                for(int k = 0; ppls[i].proc.ptr[j].argv.ptr[k]; ++k){
                    printf(" -- %s\n", ppls[i].proc.ptr[j].argv.ptr[k]);
                }
            }

            printf("\n\n");
        }
    }



//    register int i;
//     char line[1024];      /*  allow large command lines  */
//     int ncmds;
//     char prompt[50];      /* shell prompt */

//     /* PLACE SIGNAL CODE HERE */

//     sprintf(prompt,"[%s] ", argv[0]);

//     pid_t zombie = fork();
//     if(zombie==0){
//         exit(0);
//     }


//     while (promptline(prompt, line, sizeof(line)) > 0) {    /*
// until eof  */

//     if ((ncmds = parseline(line)) <= 0)
//         continue;   /* read next line */

// #ifdef DEBUG
// {
//     int i, j;
//     for (i = 0; i < ncmds; i++) {
//         for (j = 0; cmds[i].cmdargs[j] != (char *) NULL; j++)
//               fprintf(stderr, "cmd[%d].cmdargs[%d] = %s\n",
//                i, j, cmds[i].cmdargs[j]);
//           fprintf(stderr, "cmds[%d].cmdflag = %o\n", i,
// cmds[i].cmdflag);
//       }
//   }
//  #endif
 
//         for (i = 0; i < ncmds; i++) {
 
//             pid_t pid = fork();

//             if(pid==0){

//                 setpgid(0, 0);
//                 void (*previous_handler)(int) = sigset(SIGTTOU, SIG_IGN);
//                 tcsetpgrp(0, getpid());
//                 sigset(SIGTTOU, previous_handler);

//                 execvp(cmds[i].cmdargs[0], cmds[i].cmdargs);
//                 perror("Cmd error: ");
//                 exit(1);
//             }

//             waitpid(pid, NULL, 0);

//             void (*previous_handler)(int) = sigset(SIGTTOU, SIG_IGN);
//             tcsetpgrp(0, getpid());
//             sigset(SIGTTOU, previous_handler);

        
 
//       }  /* close while */
//     }

    return 0;
}
 /* PLACE SIGNAL CODE HERE */