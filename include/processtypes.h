
#ifndef PROCESS_TYPES
#define PROCESS_TYPES

#include <sys/types.h>


typedef char *charptr_t;
#define ELEMENT_TYPE charptr_t

#include "../utils/CVector_def.h"


#define IS_PROCESS_COMPLETED 1
#define IS_PROCESS_STOPPED 2
//#define IS_PROCESS_BG 4
#define IS_END_BY_SIGNAL 8

struct Process {
    vcharptr_t argv;
    pid_t pid;
    int flags;
    int status;
};
typedef struct Process Process;


#define IS_PPL_BG 1
#define IS_PPL_IN_FILE 2
#define IS_PPL_OUT_FILE 4
#define IS_PPL_OUT_APPEND 8


#define ELEMENT_TYPE Process

#include "../utils/CVector_def.h"


struct Job {
    vProcess proc;
    pid_t pgid;
    int flags;
    char *outfile;
    char *infile;
    char *cmd;
};
typedef struct Job Job;


#define ELEMENT_TYPE Job

#include "../utils/CVector_def.h"


#endif
