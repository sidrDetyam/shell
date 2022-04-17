
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "shell.h"
  
int promptline(char *prompt, char *line, int sizline){

    int n = 0;
 
    if(write(1, prompt, strlen(prompt))==-1){
        perror("write fail");
        exit(1);
    }
    
    while (1) {
        n += read(0, (line + n), sizline-n);
        *(line+n) = '\0';
 
        if (*(line+n-2) == '\\' && *(line+n-1) == '\n') {
            *(line+n) = ' ';
            *(line+n-1) = ' ';
            *(line+n-2) = ' ';
            continue;
        }
        return n;
    }
 }
 