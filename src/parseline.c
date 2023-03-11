

#include "../include/shell.h"


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
#include "../utils/CVector_def.h"
#define ELEMENT_TYPE RawCmdLine
#include "../utils/CVector_impl.h"


static char* strpbrk_escaped(const char* s, const char* cmd_delim, int* is_escaped){

    static char t[2] = " ";
    for(const char* i = s; *i!='\0'; ++i){
        t[0] = *i;
        if(strpbrk(cmd_delim, t)!=NULL && *is_escaped % 2 == 0){
            return (char*)i;
        }

        if(*i == '"'){
            *is_escaped = (*is_escaped+1)%2;
        }
    }

    return NULL;
}


static int split_cmds(char* line, vRawCmdLine* cmds){

    char* s = line;
    static const char cmd_delim[] = "\t&;\n";
    vRawCmdLine_init(cmds);
    int is_escaped = 0;

    while(*s){
        s = blankskip(s);
        if(!*s){
            break;
        }

        char* tmp = strpbrk_escaped(s, cmd_delim, &is_escaped);
        if(is_escaped){
            vRawCmdLine_free(cmds);
            return 0;
        }

        RawCmdLine cmdline = {s, tmp!=NULL && *tmp == '&'};
        vRawCmdLine_push_back(cmds, &cmdline);
        s = tmp;

        if(s==NULL){
            break;
        }
        *s++ = '\0';
    }

    return 1;
}


static void extract_arg(char** s, char** arg){

    static const char delim[] = "\">< \t\n";

    char* arg_end;
    if(**s == '"'){
        ++*s;
        arg_end = strchr(*s, '"');
    }
    else{
        arg_end = strpbrk(*s, delim);
        if(arg_end==NULL){
            arg_end = *s;
            while(*arg_end){
                ++arg_end;
            }
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
    if(**s == '"'){
        ++*s;
    }
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


static int parse_job(char* line, Job* job){

    vProcess_init(&job->proc);
    job->infile = NULL;
    job->outfile = NULL;
    if((job->cmd = (char*) malloc(strlen(line))) == NULL){
        perror("allocation fail");
        exit(1);
    }
    strcpy(job->cmd, line);
    job->flags = 0;

    static const char delim[2] = "|";
    int is_escaped = 0;

    char* s = line;
    while(*s){

        char* cmd_end = strpbrk_escaped(s, delim, &is_escaped);
        if(cmd_end){
            *cmd_end = '\0';
        }

        char* infile;
        char* outfile;
        int is_append;
        vcharptr_t argv;
        int res = parse_cmd(s, &argv, &infile, &outfile, &is_append);

        if(res!=0 || cmd_end && outfile || job->proc.cnt!=0 && infile){

                for(size_t i=0; i<job->proc.cnt; ++i){
                    vcharptr_t_free_ptr(&job->proc.ptr[i].argv);
                }
                vProcess_free(&job->proc);
                vcharptr_t_free_ptr(&argv);
                free(infile);
                free(outfile);
                free(job->infile);
                free(job->outfile);

                return -1;
        }
        else{
            Process proc = {argv, 0};
            vProcess_push_back(&job->proc, &proc);

            if(infile){
                job->infile = infile;
                job->flags |= IS_PPL_IN_FILE;
            }
            if(outfile){
                job->outfile = outfile;
                job->flags |= IS_PPL_OUT_FILE;
                if(is_append){
                    job->flags |= IS_PPL_OUT_APPEND;
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


int parse_line(char* line, Job** jobs){

    vRawCmdLine cmds;
    if(!split_cmds(line, &cmds)){
        return -1;
    }


    int cnt = (int) cmds.cnt;
    *jobs = malloc(sizeof(Job) * cmds.cnt);
    if(*jobs == NULL){
        perror("allocation fail");
        exit(1);
    }

    for(int i=0; i<cmds.cnt; ++i){
        int res = parse_job(cmds.ptr[i].line, *jobs + i);
        if(cmds.ptr[i].is_bg) {
            (*jobs)[i].flags |= IS_PPL_BG;
        }
        if(res!=0){
            free(*jobs);
            vRawCmdLine_free(&cmds);
            return -1;
        }
    }

    vRawCmdLine_free(&cmds);
    return cnt;
}
