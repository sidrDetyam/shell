
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "shell.h"
#include "process_types.h"


 static char* blankskip(char *s){
    while (isspace(*s) && *s){
        ++s;
    }
    return s;
 }
  

struct RawCmdLine{
    char* line;
    int is_bg;
};
typedef struct RawCmdLine RawCmdLine;
#define ELEMENT_TYPE RawCmdLine
#include"CVector.h"


static void split_cmds(char* line, vRawCmdLine* cmds){

    char* s = line;
    static char cmd_delim[] = "\t&;\n";
    vRawCmdLine_init(cmds);

    while(*s){
        s = blankskip(s);
        if(!*s){
            break;
        }

        char* tmp = strpbrk(s, cmd_delim);
        RawCmdLine cmdline = {s, tmp!=NULL && *tmp == '&'};
        vRawCmdLine_push_back(cmds, &cmdline);
        s = tmp;

        if(s==NULL){
            break;
        }
        *s++ = '\0';
    }
}


static void extract_arg(char** s, char** arg){

    static char delim[] = ">< \t\n";

    char* arg_end = strpbrk(*s, delim);
    if(arg_end==NULL){
        arg_end = *s;
        while(*arg_end){
            ++arg_end;
        }
    }
    
    *arg = (char*) malloc(arg_end - *s + 1);
    if(*arg==NULL){
        perror("allocation fail");
        exit(1);
    }

    memcpy(*arg, *s, arg_end-*s);
    (*arg)[arg_end-*s] = '\0';
    *s = arg_end;
}


static int parse_cmd(char* cmd, vcharptr_t* argv, char** infile, char** outfile, int* is_append){

    char* s = cmd;
    *infile = NULL;
    *outfile = NULL;
    *is_append = 0;

    vcharptr_t_init(argv);

    while(*s){
        s = blankskip(s);
        if(!*s){
            break;
        }

        switch(*s) {            
            case '>':
                if (*(s+1) == '>') {
                    *is_append = 1;
                    ++s;
                }
                ++s;
                s = blankskip(s);
                if (!*s || *outfile) {
                    vcharptr_t_free_ptr(argv);
                    free(*infile);
                    free(*outfile);
                    *infile = *outfile = NULL;
                    return -1;
                }

                extract_arg(&s, outfile);
                break;
                  
            case '<':
                ++s;
                s = blankskip(s);
                if (!*s || *infile) {
                    vcharptr_t_free_ptr(argv);
                    free(*infile);
                    free(*outfile);
                    *infile = *outfile = NULL;
                    return -1;
                }

                extract_arg(&s, infile);
                break;

            default:
                if(*infile!=NULL || *outfile!=NULL){
                    vcharptr_t_free_ptr(argv);
                    free(*infile);
                    free(*outfile);
                    *infile = *outfile = NULL;
                    return -1;
                }

                char* arg;
                extract_arg(&s, &arg);
                vcharptr_t_push_back(argv, &arg);

                break;
          }
    }

    s = NULL;
    vcharptr_t_push_back(argv, &s);
    return 0;
}


static int parse_ppl(char* line, ProcessPipeline* ppl){

    vProcess_init(&ppl->proc);
    ppl->infile = NULL;
    ppl->outfile = NULL;
    ppl->flags = 0;

    char* s = line;
    while(*s){

        char* cmd_end = strchr(s, '|');
        if(cmd_end){
            *cmd_end = '\0';
        }

        char* infile;
        char* outfile;
        int is_append;
        vcharptr_t argv;
        int res = parse_cmd(s, &argv, &infile, &outfile, &is_append);

        if(res!=0 || cmd_end && outfile || ppl->proc.cnt!=0 && infile){

                for(size_t i=0; i<ppl->proc.cnt; ++i){
                    vcharptr_t_free_ptr(&ppl->proc.ptr[i].argv);
                }
                vProcess_free(&ppl->proc);
                vcharptr_t_free_ptr(&argv);
                free(infile);
                free(outfile);
                free(ppl->infile);
                free(ppl->outfile);

                return -1;
        }
        else{
            Process proc = {argv, 0};
            vProcess_push_back(&ppl->proc, &proc);

            if(infile){
                ppl->infile = infile;
                ppl->flags |= IS_IN_FILE;
            }
            if(outfile){
                ppl->outfile = outfile;
                ppl->flags |= IS_OUT_FILE;
                if(is_append){
                    ppl->flags |= IS_OUT_APPEND;
                }
            }
        }

        if(cmd_end){
            s = cmd_end + 1;
        }
        else{
            break;
        }
    }

    return 0;
}


int parseline(char* line, ProcessPipeline** ppls){

    vRawCmdLine cmds;
    split_cmds(line, &cmds);
    int cnt = cmds.cnt;
    *ppls = malloc(sizeof(ProcessPipeline) * cmds.cnt);
    if(*ppls==NULL){
        perror("allocation fail");
        exit(1);
    }

    for(int i=0; i<cmds.cnt; ++i){
        int res = parse_ppl(cmds.ptr[i].line, *ppls + i);
        if(cmds.ptr[i].is_bg) {
            (*ppls)[i].flags |= IS_BG;
        }
        if(res!=0){
            free(*ppls);
            vRawCmdLine_free(&cmds);
            return -1;
        }
    }

    vRawCmdLine_free(&cmds);
    return cnt;
}
