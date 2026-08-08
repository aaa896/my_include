#include "arena.h"


#ifdef PLATFORM_LINUX
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#elif PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#undef min
#undef max
#include <windows.h>
#include <stdarg.h>
#include <string.h>
#else
#error "OS not supported"
#endif


void *memory_reserve(size_t size)
{
#ifdef PLATFORM_LINUX
    void *ptr = mmap(0, size, PROT_NONE, MAP_ANONYMOUS|MAP_PRIVATE, -1 , 0);
    ASSERT(MAP_FAILED != ptr);
    return ptr;
#elifdef PLATFORM_WINDOWS
    LPVOID  ptr = VirtualAlloc( 0, size, MEM_RESERVE, PAGE_NOACCESS);
    ASSERT(ptr);
    return ptr;
#else
#error memory_reserve_unsupported_platform
#endif
}

void memory_commit(void *mem,  size_t size)
{
#ifdef PLATFORM_LINUX
    ASSERT(!mprotect(mem, size, PROT_READ|PROT_WRITE));
    memset(mem, 0, size);
#elifdef PLATFORM_WINDOWS
    LPVOID  res = VirtualAlloc( mem, size, MEM_COMMIT, PAGE_READWRITE);
    ASSERT(res);
#else
#error unsupported_platform_memory_commit
#endif
}

void memory_release(void *mem, size_t size)
{
#ifdef PLATFORM_LINUX
    ASSERT(!munmap(mem , size));
#elifdef PLATFORM_WINDOWS
    VirtualFree(mem,0, MEM_RELEASE);
#else
#error memory_release_unsupported_platform
#endif
}

void memory_decommit(void *mem, size_t size)
{
#ifdef PLATFORM_LINUX
    madvise(mem, size, MADV_DONTNEED);
    ASSERT(!mprotect(mem, size, PROT_NONE));
#elifdef PLATFORM_WINDOWS
    VirtualFree(mem,size, MEM_DECOMMIT);
#else
#error memory_decommit_unsupported_platform
#endif
}


int get_page_size()
{
#ifdef PLATFORM_LINUX
    int page_size =  getpagesize();
    return page_size;
#elifdef PLATFORM_WINDOWS
    SYSTEM_INFO sys_info = { 0 };
    GetSystemInfo(&sys_info);
    info.page_size = sys_info.dwPageSize;
    return info.page_size;
#else
#error unsupported_platform_page_size
#endif

}

static THREAD_LOCAL Arena *scratch_arenas[2];

Arena *arena_alloc_config(Arena_Config *config)
{
    u64 reserve_size = config->reserve_size + ARENA_HEADER_SIZE;
    u64 commit_size = config->commit_size;
    int page_size = get_page_size();

    reserve_size = align_pow2(reserve_size, page_size);
    commit_size  = align_pow2(commit_size, page_size);

    void *mem = memory_reserve(reserve_size);
    memory_commit(mem, commit_size);

    Arena *arena = (Arena*)mem;
    arena->current = arena;
    arena->free_last = 0;
    arena->base_position = 0;
    arena->position = ARENA_HEADER_SIZE;
    arena->reserved= reserve_size;
    arena->committed= commit_size;
    arena->reserve_size = config->reserve_size;
    arena->commit_size = config->commit_size;
    arena->free_reserve_size = 0;

    asan_poison_memory_region(mem, commit_size);
    asan_unpoison_memory_region(mem, ARENA_HEADER_SIZE);

    return arena;
}

Arena* arena_alloc()
{
    Arena_Config config = {
        .commit_size = ARENA_DEFAULT_COMMIT_SIZE,
        .reserve_size = ARENA_DEFAULT_RESERVE_SIZE,
    };
    return arena_alloc_config(&config);
}



void arena_init() 
{
    arena_scratch_alloc();
}

void arena_release(Arena *arena)
{
    for (Arena *current = arena->current, *prev = 0; current; current = prev) {
        prev = current->prev;
        memory_release(current, current->reserved);
    }
}

void arena_reset(Arena *arena)
{
    arena_pop_to(arena, 0);
}

void arena_deinit(void)
{
    for (int i = 0; i  < ARRAY_COUNT(scratch_arenas); ++i) {
        arena_release((scratch_arenas[i]));
        scratch_arenas[i] = 0;
    }
}

u64 get_arena_position(Arena *arena)
{
    ASSERT(arena);
    Arena *current = arena->current;
    return current->base_position + current->position;
}

void arena_pop(Arena *arena, u64 size)
{
    u64 position_old = get_arena_position(arena);
    u64 position_new = position_old;
    if (size < position_old) {
        position_new = position_old - size;
    }
    arena_pop_to(arena, position_new);
}



void arena_pop_to(Arena *arena, u64 position)
{
    u64 big_position = clamp_bottom(ARENA_HEADER_SIZE, position);
    Arena *current = arena->current;

    for (Arena *prev = 0; current->base_position >= big_position;
            current = prev) {
        prev = current->prev;
        current->position = ARENA_HEADER_SIZE;
        arena->free_reserve_size += current->reserve_size;
        current->prev = arena->free_last;
        arena->free_last = current;
        asan_poison_memory_region(
                (uint8_t *)current + ARENA_HEADER_SIZE,
                current->reserve_size - ARENA_HEADER_SIZE);
    }

    arena->current = current;
    u64 new_position = big_position - current->base_position;

    ASSERT(new_position <= current->position);
    asan_poison_memory_region((uint8_t *)current + new_position, (current->position - new_position));
    current->position = new_position;
}


void * arena_push(Arena *arena, size_t size, u64 align)
{
    if (!size)
        return 0;
    ASSERT(arena);

    Arena *current = arena->current;
    u64 pos_start = align_pow2(current->position, align);
    u64 pos_end = pos_start + size;
    if (current->reserved < pos_end) {
        Arena *new_block = 0;
        Arena *prev = 0;
        for (new_block = arena->free_last; new_block; prev = new_block, new_block = new_block->prev) {
            if (new_block->reserved >= align_pow2(size, align)) {
                if (prev) {
                    prev->prev = new_block->prev;
                }else {
                    arena->free_last = new_block->prev;
                }
                arena->free_reserve_size -= new_block->reserve_size;
                asan_unpoison_memory_region( (uint8_t *)new_block + ARENA_HEADER_SIZE, new_block->reserve_size - ARENA_HEADER_SIZE);

                break;
            }

        }
        if (!new_block) {
            u64 reserve_size = current->reserve_size;
            u64 commit_size = current->commit_size;

            if (size + ARENA_HEADER_SIZE > reserve_size) {
                reserve_size = align_pow2(
                        size + ARENA_HEADER_SIZE, align);
                commit_size = align_pow2(
                        size + ARENA_HEADER_SIZE, align);
            }
            Arena_Config config = {
                .commit_size = commit_size,
                .reserve_size = reserve_size,
            };
            new_block = arena_alloc_config(&config);
        }

        new_block->base_position = current->base_position + current->reserved;
        new_block->prev = arena->current;
        arena->current = new_block;
        current = new_block;
        pos_start = align_pow2(current->position, align);
        pos_end = pos_start + size;
        ASSERT(pos_end <= current->reserved);
    }

    if (current->committed < pos_end) {
        uint64_t commit_post_aligned = pos_end + current->commit_size - 1;

        commit_post_aligned -= commit_post_aligned % current->commit_size;
        uint64_t commit_post_clamped = clamp_top(commit_post_aligned, current->reserved);
        uint64_t commit_size = commit_post_clamped - current->committed;
        uint8_t *commit_ptr = (uint8_t *)current + current->committed;

        memory_commit(commit_ptr, commit_size);
        current->committed = commit_post_clamped;
    }

    void *result = 0;

    if (current->committed >= pos_end) {
        result = ((uint8_t *)current) + pos_start;
        current->position = pos_end;
        asan_unpoison_memory_region(result, size);
    }

    if (!result) 
        FAIL_MSG("Arena allocation failed");


    return result;
}


Arena_Temp arena_temp_begin(Arena *arena)
{
    Arena_Temp temp= {
        .arena = arena,
        .position = arena->position,
    };
    return temp;
}


void arena_temp_end(Arena_Temp temp)
{
    arena_pop_to(temp.arena, temp.position);
}


Arena_Temp arena_scratch_begin(Arena **conflicts, int conflict_count)
{
    Arena_Temp  temp = ZERO_STRUCT;
    int total_scratch_arenas = ARRAY_COUNT(scratch_arenas);
    for (int scratch_arena_index = 0; scratch_arena_index < total_scratch_arenas; ++scratch_arena_index) {
        b32 found = 1;
        for (int arena_index = 0; arena_index < conflict_count; ++arena_index) {
            if (conflicts[arena_index] == scratch_arenas[scratch_arena_index]) {
                found = 0;
                break;
            }
        }
        if (found) {
            temp = arena_temp_begin(scratch_arenas[scratch_arena_index]);
            break;
        }
    }
    return temp;
}

void arena_scratch_end(Arena_Temp scratch)
{
        arena_temp_end(scratch);
}


void arena_scratch_alloc(void)
{
    for (int i = 0; i  < ARRAY_COUNT(scratch_arenas); ++i) {
        scratch_arenas[i] = arena_alloc();
    }
}

char *arena_push_cstr(Arena *arena, char *cstr)
{
        ASSERT(arena);
        size_t length = strlen(cstr);
        char *dest = arena_push_array(arena, char, length + 1);
        strncpy(dest, cstr, length + 1);
        return dest;
}

char *arena_push_cstr_fmt_va(Arena *arena, const char *fmt, va_list args)
{
        ASSERT(arena);
        char temp[1024];
        int length = snprintf(temp, sizeof(temp), fmt, args);
        if (length < 0) {
                FAIL_MSG("Failed to format string");
        }
        char *dest = arena_push_array(arena, char, (uint64_t)length + 1);
        strncpy(dest, temp, (uint64_t)length + 1);
        return dest;
}


char *arena_push_cstr_fmt(Arena *arena, char *fmt, ...)
{
        ASSERT(arena);
        va_list args;
        va_start(args, fmt);
        char *result = arena_push_cstr_fmt_va(arena, fmt, args);
        va_end(args);
        return result;
}

