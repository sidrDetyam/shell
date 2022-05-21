
#include "shell.h"


static void init_tty(struct termios* savtty){

    struct termios tty;
    if(tcgetattr(0, &tty)==-1){
        perror("tcgetaattr fail");
        exit(1);
    }
    *savtty = tty;
    tty.c_lflag &= ~(ECHO | ICANON);
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSAFLUSH, &tty)==-1){
        perror("tcgetaattr fail");
        exit(1);
    }
    setbuf(stdout, (char *) NULL);
}


static void backspace(char* input_buff, size_t* curr, size_t* rpos){

    --*curr;
    --*rpos;
    for(size_t i=*curr; i < *rpos; ++i){
        input_buff[i] = input_buff[i + 1];
    }

    putchar('\010');
    for(size_t i=*curr; i<*rpos; ++i){
        putchar(input_buff[i]);
    }
    putchar(' ');
    for(size_t i=*curr; i<*rpos+1; ++i){
        putchar('\010');
    }
}


static void insert_character(char* input_buff, size_t* curr, size_t* rpos, char ch){

    for(size_t i = *rpos; i > *curr; --i){
        input_buff[i] = input_buff[i - 1];
    }
    input_buff[*curr] = ch;
    ++*curr;
    ++*rpos;

    for(size_t i=*curr-1; i<*rpos; ++i){
        putchar(input_buff[i]);
    }
    for(size_t i=*curr; i<*rpos; ++i){
        putchar('\010');
    }
}



#define MAX_SEQ_LEN 5
#define MAX_INPUT_LEN 4096
static int input_cmd(char* res, const char* original_cmd, int *is_up){

    static char input_buff[MAX_INPUT_LEN];
    static char read_buff[MAX_SEQ_LEN];

    strcpy(input_buff, original_cmd);
    size_t curr, rpos;
    curr = rpos = strlen(original_cmd);
    printf("%s", input_buff);
    fflush(stdout);

    while(1){
        int cnt = read(0, read_buff, MAX_SEQ_LEN);

        if(cnt==1){
            switch (read_buff[0]) {
                case 10:
                    input_buff[rpos] = '\0';
                    putchar('\n');
                    strcpy(res, input_buff);
                    return 1;

                case 127:
                    if(curr>0){
                        backspace(input_buff, &curr, &rpos);
                    }
                    continue;

                default:
                    insert_character(input_buff, &curr, &rpos, read_buff[0]);
                    continue;
            }
        }


        if(cnt==3 && read_buff[0]=='\033' && read_buff[1]=='['){

            switch(read_buff[2]) {
                case 'D':
                    if(curr>0){
                        --curr;
                        putchar('\010');
                    }
                    continue;

                case 'C':
                    if(curr < rpos){
                        ++curr;
                        printf("\033[1C");
                        fflush(stdout);
                    }
                    continue;

                case 'A':
                case 'B':
                    for(int i=0; i<rpos-curr; ++i){
                        putchar(' ');
                    }
                    for(int i=0; i<rpos; ++i){
                        putchar('\010');
                        putchar(' ');
                        putchar('\010');
                    }
                    *is_up = read_buff[2] == 'A';
                    return 0;
            }
        }
    }
}


int promptline(const char *prompt, char *line, vcharptr_t* history){

    printf("\x1B[32m");
    fflush(stdout);
    if(write(1, prompt, strlen(prompt))==-1){
        perror("write fail");
        exit(1);
    }
    printf("\x1B[0m$ ");
    fflush(stdout);

    struct termios savetty;
    init_tty(&savetty);
    int is_up;

    static char empty[0] = "";
    char* tmp_ptr = empty;
    vcharptr_t_push_back(history, &tmp_ptr);

    int curr = (int)history->cnt - 1;
    while(!input_cmd(line, history->ptr[curr], &is_up)){
        if(is_up){
            if(curr>0) {
                --curr;
            }
            continue;
        }
        if(curr < history->cnt - 1){
            ++curr;
        }
    }
    vcharptr_t_pop_back(history);
    if(tcsetattr(0, TCSAFLUSH, &savetty)==-1){
        perror("tcsetattr fail");
        exit(1);
    }
 }
 