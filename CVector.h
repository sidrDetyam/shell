//
// Created by argem on 25.03.2022.
//

#include<stdlib.h>
#include<errno.h>
#include<string.h>

///TODO - include guard if type has committed(possible?)

#define CONCAT(a,b) CONCAT_V(a,b)
#define CONCAT_V(a,b) a##b

#define VECTOR CONCAT(v, ELEMENT_TYPE)


struct VECTOR{
    ELEMENT_TYPE* ptr;
    size_t cnt;
    size_t capacity;
};
typedef struct VECTOR VECTOR;

#ifndef REALLOC_RATIO
#define REALLOC_RATIO 2
#endif

static void CONCAT(VECTOR, _assign) (struct VECTOR* vector, ELEMENT_TYPE* el, size_t ind){

    memcpy(vector->ptr + ind, el, sizeof(ELEMENT_TYPE));
}


static ELEMENT_TYPE* CONCAT(VECTOR, _back) (struct VECTOR* vector){
    return vector->ptr + vector->cnt-1;
}


static void CONCAT(VECTOR, _alloc) (struct VECTOR* vector){

    if(vector->cnt == vector->capacity){
        void* tmp = realloc(vector->ptr,
                            (vector->capacity * REALLOC_RATIO + 1) * sizeof(ELEMENT_TYPE));
        if(tmp==NULL){
            perror("Out of memory");
            exit(1);
        }

        vector->capacity = vector->capacity * REALLOC_RATIO + 1;
        vector->ptr = tmp;
    }
}


static void CONCAT(VECTOR, _push_back) (struct VECTOR* vector, ELEMENT_TYPE* el){

    CONCAT(VECTOR, _alloc)(vector);
    memcpy(&vector->ptr[vector->cnt], el, sizeof(ELEMENT_TYPE));
    ++vector->cnt;
}


static void CONCAT(VECTOR, _init) (struct VECTOR* vector){

    vector->cnt = 0;
    vector->ptr = NULL;
    vector->capacity = 0;
}


static void CONCAT(VECTOR, _free) (struct VECTOR* vector){

    free(vector->ptr);
    CONCAT(VECTOR, _init)(vector);
}


//if ELEMENT_TYPE is pointer to something
static void CONCAT(VECTOR, _free_ptr) (struct VECTOR* vector){

    int* foo;
    int** ptr =  &foo;
    for(size_t i=0; i<vector->cnt; ++i){
        memcpy(ptr, &vector->ptr[i], sizeof(int*));
        free(*ptr);
    }
    CONCAT(VECTOR, _free)(vector);
}

#undef ELEMENT_TYPE
#undef REALLOC_RATIO
