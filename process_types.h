
#ifndef PROCESS_TYPES
#define PROCESS_TYPES

#include <sys/types.h>


typedef char* charptr_t;
#define ELEMENT_TYPE charptr_t
#include"CVector.h"


struct Process{
    vcharptr_t argv;
    pid_t pid;
};
typedef struct Process Process;


#define IS_BG 1
#define IS_IN_FILE 2
#define IS_OUT_FILE 4
#define IS_OUT_APPEND 8


#define ELEMENT_TYPE Process
#include"CVector.h"

struct ProcessPipeline{

    vProcess proc;
    pid_t pgid;
    int flags;

    char* outfile;
    char* infile;
};
typedef struct ProcessPipeline ProcessPipeline;


#endif