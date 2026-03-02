#ifndef DYNAMIC_ARRAY_H_
#define DYNAMIC_ARRAY_H_

#include <stdlib.h>
#include <string.h>

#define ARRAY_CAPACITY_DEFAULT 1

typedef struct Array_Header Array_Header;
struct Array_Header {
    size_t capacity;
    size_t size;
};

#define array_size(array) (((Array_Header*)array - 1)->size)

#define array_free(array) do{\
    free(((Array_Header*)array - 1));\
    array = 0;\
    }while(0)

#define array_append(array,value)\
    do{\
    if (!array) {\
        size_t size = (sizeof(Array_Header) + (ARRAY_CAPACITY_DEFAULT * sizeof(*array)));\
        array = malloc(size);\
        memset(array,0, size);\
        ((Array_Header*)array)->capacity = (ARRAY_CAPACITY_DEFAULT );\
        ((Array_Header*)array)->size = 0;\
        array = (void*)( (Array_Header*)array + 1);\
    }\
    size_t size =  ((Array_Header*)array - 1)->size;\
    size_t capacity =  ((Array_Header*)array - 1)->capacity;\
    if (size == capacity) {\
        capacity *= 2;\
        array = realloc( ((Array_Header*)array -1), capacity * sizeof(*array) + sizeof(Array_Header));\
        ((Array_Header*)array)->capacity = capacity;\
        array = (void*)( (Array_Header*)array + 1);\
    }\
    array[size] = value;\
    ((Array_Header*)array - 1)->size += 1;\
    }while(0)




#endif // DYNAMIC_ARRAY_H_
