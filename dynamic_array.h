#ifndef _DYNAMIC_ARRAY_H_
#define _DYNAMIC_ARRAY_H_

#define ARRAY_CAPACITY_DEFAULT 8

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "tdf.h"


typedef struct {
    size_t count;
    size_t capacity;
}Array_Header;

#ifndef ARRAY_ALIGN
#define ARRAY_ALIGN alignof(max_align_t)
#endif
//_Static_assert(!(ARRAY_ALIGN  & (ARRAY_ALIGN -1)), "Align must be power 2");


#define get_array_header(array)  ((Array_Header*)(((uint8_t*)array) - (sizeof(Array_Header) + get_array_header_padding)))
#define get_array_header_padding ( ((sizeof(Array_Header) & (ARRAY_ALIGN -1)) == 0) ? (0) : (ARRAY_ALIGN - (sizeof(Array_Header) & (ARRAY_ALIGN -1))))
#define get_array_count(array)   (get_array_header(array)->count)
#define get_array_tail(array)    ( array[get_array_count(array) -1])

#define create_array(array, cap)\
    do {\
        Array_Header header = ZERO_STRUCT;\
        header.capacity = cap ;\
        int item_size =  sizeof((*array)[0]);\
        int header_space = sizeof(Array_Header) + get_array_header_padding;\
        int array_size = header.capacity * item_size;\
        Array_Header *header_data = (Array_Header*)malloc(array_size + header_space);\
        if (!header_data) assert(0 && "create array malloc fail");\
        memset(header_data, 0, array_size + header_space);\
        memcpy(header_data, &header, sizeof(Array_Header));\
        (*array) = (typeof(*array))((uint8_t*)header_data + header_space);\
    } while (0);


#define array_append(array, item)\
    do {\
        if ((*array) == 0) {\
            create_array(array, ARRAY_CAPACITY_DEFAULT);\
        }\
        int item_size = sizeof((*array)[0]);\
        int header_space = sizeof(Array_Header) + get_array_header_padding;\
        Array_Header *header = (Array_Header*)((uint8_t*)((*array))  - header_space);\
        if (header->count == header->capacity) {\
            header->capacity *= 2;\
            int new_size =  header->capacity * sizeof((*array)[0]) + header_space;\
            header = (Array_Header*)realloc(header, new_size);\
            if (!header) assert(0 && "array realloc fail\n");\
            (*array) = (typeof(*array))((uint8_t*)header + header_space);\
        }\
        (*array)[header->count] = item;\
        header->count += 1;\
    }while(0);


#endif // _DYNAMIC_ARRAY_H_
