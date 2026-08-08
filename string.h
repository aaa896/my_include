#ifndef _STRING_H_
#define _STRING_H_
#include "arena.h"
#include "tdf.h"

#define SV_FMT "%.*s"
#define SV_ARG(x) x.count,x.data

typedef struct String_View String_View;
struct String_View {
    char *data;
    int count;
};

typedef struct String String;
struct String  {
    char *data;
    int size;
    int capacity;
};

#define STRING_DEFAULT_CAPACITY 16

String  str_create_from_cstr(Arena *arena, char *cstr) ;
String  str_create_from_sv(Arena *arena, String_View sv) ;
String  str_clone(Arena *arena, String str) ;
String  str_read_file_from_cstr(Arena *arena, char *path);
String  str_read_file_from_str(Arena *arena, String  str);
b32     str_strcmp_cstr(String sv , char *s);
void    str_print(String sv);
String  str_join_str_str(Arena *arena, String s1, String s2);
String  str_join_str_cstr(Arena *arena, String s1, char *s2);
String  str_join_cstr_str(Arena *arena, char *cstr, String s1);
void    str_resize_capacity(Arena *,String *s1, size_t cap);
void    str_append_str(Arena *arena, String *s1, String s2);
void    str_append_cstr(Arena *arena, String *s1, char *cstr);
void    str_append_sv(Arena *arena, String *s1, String_View sv);
void    str_append_int(Arena *arena, String *s1, int x);
char*   str_convert_to_cstr(Arena *arena,String str) ;
String  str_create_from_int(Arena *arena,int x);
char*   cstr_create_from_int(Arena *arena,int x);


void         sv_chop_left(String_View *sv, int n) ;
void         sv_chop_right(String_View *sv,int n) ;
void         sv_trim(String_View *sv) ;
String_View  sv_chop_type(String_View *sv, int (*istype)(int c)) ;
String_View  sv_chop_delim(String_View *sv, char c) ;
String_View  sv_from_str(String str);

#endif // _STRING_H_
