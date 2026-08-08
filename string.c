#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "string.h"

char *str_convert_to_cstr(Arena *arena, String str)
{
    char *rv = arena_push_array(arena, char, str.size + 1);
    memcpy(rv, str.data, str.size);
    rv[str.size] = 0;
    return rv;
}

String str_create_from_cstr(Arena *arena, char *cstr) 
{
    String str = ZERO_STRUCT;
    str.size =strlen(cstr) ;
    str.capacity  = (str.size > STRING_DEFAULT_CAPACITY) ? (str.size * 2) : (STRING_DEFAULT_CAPACITY);
    str.data = arena_push_array(arena, char,str.capacity);
    memcpy(str.data, cstr, str.size);
    return str;
}

String str_create_from_sv(Arena *arena, String_View sv) 
{
    String str = ZERO_STRUCT;
    str.size =sv.count;
    str.capacity  = (str.size > STRING_DEFAULT_CAPACITY) ? (str.size * 2) : (STRING_DEFAULT_CAPACITY);
    str.data = arena_push_array(arena,char, str.capacity);
    memcpy(str.data, sv.data, sv.count);
    return str;
}

String str_clone(Arena *arena, String str) 
{
    String rv  = ZERO_STRUCT;
    rv.size = str.size;
    rv.capacity  = (rv.size > STRING_DEFAULT_CAPACITY) ? (rv.size * 2) : (STRING_DEFAULT_CAPACITY);
    rv.data = arena_push_array(arena, char, rv.capacity);
    memcpy(rv.data, str.data, rv.size);
    return rv;
}


String str_read_file_from_cstr(Arena *arena, char *path)
{
    String rv = ZERO_STRUCT;
    FILE *file = fopen(path, "rb");
    if (!file)
        return rv;

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    rv.size = size;
    rv.capacity  = (rv.size > STRING_DEFAULT_CAPACITY) ? (rv.size * 2) : (STRING_DEFAULT_CAPACITY);
    rv.data = arena_push_array(arena,char, rv.capacity);
    fread(rv.data, 1, size, file);
    fclose(file);
    return rv;
}

String str_read_file_from_str(Arena *arena, String str)
{
    Arena_Temp  scratch = arena_scratch_begin(&arena, 1);
    char * cstr = str_convert_to_cstr(scratch.arena, str);
    String rv = str_read_file_from_cstr(arena, cstr);
    arena_scratch_end(scratch);
    return rv;
}


int str_strcmp_cstr(String  str, char *cstr)
{
    int cstr_len = strlen(cstr);
    if (str.size != cstr_len)
        return 0;
    for (int i = 0; i < cstr_len; ++i) {
        if (str.data[i] != cstr[i])
            return 0;
    }
    return 1;
}

void str_print(String str)
{
    for (int i = 0; i < str.size; ++i)
        putchar(str.data[i]);
}

String str_join_str_str(Arena *arena, String s1, String s2)
{
    String rv = ZERO_STRUCT;
    rv.size = s1.size + s2.size;
    rv.capacity  = (rv.size > STRING_DEFAULT_CAPACITY) ? (rv.size * 2) : (STRING_DEFAULT_CAPACITY);
    rv.data = arena_push_array(arena, char, rv.capacity);
    memcpy(rv.data, s1.data  , s1.size);
    memcpy(&rv.data[s1.size], s2.data, s2.size);
    return rv;
}


String str_join_cstr_cstr (Arena *arena, char* cstr_1, char *cstr_2)
{
    String rv = ZERO_STRUCT;
    int clen_1 = strlen(cstr_1);
    int clen_2 = strlen(cstr_2);
    rv.size = clen_1 + clen_2;
    rv.capacity  = (rv.size > STRING_DEFAULT_CAPACITY) ? (rv.size * 2) : (STRING_DEFAULT_CAPACITY);
    rv.data = arena_push_array(arena, char, rv.capacity);
    memcpy(rv.data, cstr_1  , clen_1);
    memcpy(&rv.data[clen_1], cstr_2, clen_2);
    return rv;
}


String str_join_str_cstr (Arena *arena, String s1, char *cstr)
{
    String rv = ZERO_STRUCT;
    int cstr_count = strlen(cstr);
    rv.size = s1.size + cstr_count;
    rv.capacity  = (rv.size > STRING_DEFAULT_CAPACITY) ? (rv.size * 2) : (STRING_DEFAULT_CAPACITY);
    rv.data = arena_push_array(arena, char, rv.capacity);
    memcpy(rv.data, s1.data  , s1.size);
    memcpy(&rv.data[s1.size], cstr, cstr_count);
    return rv;
}


String str_join_cstr_str (Arena *arena, char *cstr, String s1 )
{
    String rv = ZERO_STRUCT;
    int cstr_count = strlen(cstr);
    rv.size = s1.size + cstr_count;
    rv.capacity  = (rv.size > STRING_DEFAULT_CAPACITY) ? (rv.size * 2) : (STRING_DEFAULT_CAPACITY);
    rv.data = arena_push_array(arena, char, rv.capacity);
    memcpy(rv.data, cstr  , cstr_count);
    memcpy(&rv.data[cstr_count], s1.data, s1.size);
    return rv;
}

String str_join_cstr_sv(Arena *arena, char *cstr, String s1)
{
    String rv = ZERO_STRUCT;
    int cstr_count = strlen(cstr);
    rv.size = s1.size + cstr_count;
    rv.capacity  = (rv.size > STRING_DEFAULT_CAPACITY) ? (rv.size * 2) : (STRING_DEFAULT_CAPACITY);
    rv.data = arena_push_array(arena, char,rv.capacity);
    memcpy(rv.data, cstr  , cstr_count);
    memcpy(&rv.data[cstr_count], s1.data, s1.size);
    return rv;
}



void str_resize_capacity(Arena *arena, String *s1, size_t cap)
{
    char * new_data = arena_push_array(arena,char, cap);
    if (cap < (size_t)s1->size)
        s1->size = cap;
    memcpy(new_data, s1->data , s1->size);
    s1->data = new_data;
    s1->capacity = cap;
}

void str_append_str(Arena *arena, String *s1, String s2)
{
    int total_size = s1->size + s2.size;
    if (total_size > s1->capacity) {
        size_t cap = total_size * 2;
        str_resize_capacity(arena, s1, cap);
    }
    memcpy(s1->data + s1->size,  s2.data, s2.size);
    s1->size += s2.size;
}

void str_append_cstr(Arena *arena, String *s1, char *cstr)
{
    int cstr_size = strlen(cstr);
    int total_size = s1->size + cstr_size;
    if (total_size > s1->capacity) {
        size_t cap = total_size * 2;
        str_resize_capacity(arena, s1, cap);
    }
    memcpy(s1->data + s1->size,  cstr, cstr_size);
    s1->size += cstr_size;
}


void str_append_sv(Arena *arena, String *s1, String_View sv)
{
    int total_size = s1->size + sv.count;
    if (total_size > s1->capacity) {
        size_t cap = total_size * 2;
        str_resize_capacity(arena, s1, cap);
    }
    memcpy(s1->data + s1->size,  sv.data, sv.count);
    s1->size += sv.count;
}

void str_append_int(Arena *arena, String *s1, int x)
{

    int buff_size = 32;
    char buff[32] ;
    snprintf(buff, buff_size, "%d", x);
    str_append_cstr(arena, s1, buff);
}


String str_create_from_int(Arena *arena, int x)
{
    int buff_size = 32;
    char buff[32] ;
    snprintf(buff, buff_size, "%d", x);
    return str_create_from_cstr(arena, buff);
}
char* cstr_create_from_int(Arena *arena, int x)
{
    int buff_size = 32;
    char buff[32] ;
    snprintf(buff, buff_size, "%d", x);
    int slen = strlen(buff);
    char *rv = arena_push_array(arena, char, slen);
    return rv;
}



void sv_chop_left(String_View *sv, int n)
{
    if (n > sv->count) 
        n = sv->count;
    sv->count -= n;
    sv->data += n;
}

void sv_chop_right(String_View *sv, int n)
{

    if (n > sv->count) 
        n = sv->count;
    sv->count -= n;
}

String_View sv_chop_delim(String_View *sv, char c) 
{
    int rv_count = 0;
    for (int i = 0; i < sv->count; ++i) {
        if (sv->data[i] == c) {
            break;
        }
        ++rv_count;
    }

    if (rv_count < sv->count) {
        String_View rv = {.data= sv->data , 
            .count = rv_count};
        sv->data = sv->data + rv_count + 1;
        sv->count -= rv_count + 1;
        return rv;
    }
    String_View rv = *sv;
    sv_chop_left(sv, sv->count);
    return rv;
}


String_View sv_chop_type(String_View *sv, int (*istype)(int c)) 
{
    int rv_count = 0;
    for (int i = 0; i < sv->count; ++i) {
        if (istype(sv->data[i] )) {
            break;
        }
        ++rv_count;
    }

    if (rv_count < sv->count) {
        String_View rv = { .data= sv->data, .count = rv_count};
        sv->data = sv->data + rv_count + 1;
        sv->count -= rv_count + 1;
        return rv;
    }
    String_View rv = *sv;
    sv_chop_left(sv, sv->count);
    return rv;

}

void sv_trim_left(String_View *sv) 
{
    for (int i = 0; i < sv->count; ++i) {
        if (!isspace(sv->data[i] )) {
            sv->data +=i;
            sv->count -= i;
            return;
        }
    }
    sv_chop_left(sv, sv->count);
}

void sv_trim_right(String_View *sv) 
{

    for (int i = sv->count -1; i >= 0; --i) {
        if (!isspace(sv->data[i] )) {
            sv->count -= sv->count - i + 1;
            return;
        }
    }
    sv_chop_right(sv, sv->count);
}


void sv_trim(String_View *sv) 
{
    sv_trim_left(sv);
    sv_trim_right(sv);
}

String_View  sv_from_str(String str)
{
    String_View rv = {.data = str.data, .count = str.size};
    return rv;
}
