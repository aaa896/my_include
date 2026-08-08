#ifndef _ARENA_H_
#define _ARENA_H_


#include <stdlib.h>
#include <string.h>
#include "tdf.h"


#define ARENA_DEFAULT_COMMIT_SIZE KB(64)
#define ARENA_DEFAULT_RESERVE_SIZE MB(16)
#define ARENA_HEADER_SIZE 128

typedef struct Arena Arena;
struct Arena  {
    Arena *prev;
    Arena *current;
    Arena *free_last;

    u64 base_position;
    u64 position;

    u64 committed;
    u64 reserved;

    u64 commit_size;
    u64 reserve_size;
    u64 free_reserve_size;
};

STATIC_ASSERT(sizeof(Arena) <= ARENA_HEADER_SIZE, expected_arena_header_to_be_smaller_than_128_bytes);


typedef struct Arena_Temp Arena_Temp;
struct Arena_Temp {
    Arena *arena;
    u64 position;
};

typedef struct Arena_Config Arena_Config;
struct Arena_Config {
    size_t commit_size;
    size_t reserve_size;
};

void *memory_reserve(size_t size);
void memory_commit(void *mem,  size_t size);
void memory_release(void *mem, size_t size);
void memory_decommit(void *mem, size_t size);
int get_page_size();

void   arena_init(void);
void   arena_deinit(void);
Arena* arena_alloc_config(Arena_Config * arena);
Arena* arena_alloc(void);
void   arena_release(Arena*arena); 
void   arena_reset(Arena *arena);
u64    get_arena_position(Arena *arena);
void   arena_pop_to(Arena *arena, u64 position);
void   arena_pop(Arena *arena, u64 bytes);

void * arena_push(Arena *arena, size_t size, u64 align);
char *arena_push_cstr(Arena *arena, char *cstr);


Arena_Temp arena_temp_begin(Arena *arena);
void arena_temp_end(Arena_Temp temp);

void arena_scratch_alloc(void);
Arena_Temp arena_scratch_begin(Arena **conflicts, int conflict_count);
void arena_scratch_end(Arena_Temp scratch);


#define arena_push_array_align(arena, type, count, align)\
        (type*)memset(arena_push((arena), sizeof(type) * (count), (align)), 0, sizeof(type) * (count))

#define arena_push_array(arena, type, count)\
        arena_push_array_align((arena), type, (count), max(8, alignof(type)))

#define arena_push_struct(arena, type) \
        arena_push_array_align((arena), type, 1, max(8, alignof(type)))





#endif // _ARENA_H_

