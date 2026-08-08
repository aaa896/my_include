#ifndef _TDF_H_
#define _TDF_H_
#include <stdint.h>
#include <stdio.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int32_t  b32;
typedef int64_t  i64;

typedef float    f32;
typedef double   f64;

#ifdef __cplusplus
#define CPP 1
#else
#define CPP 0
#endif

#if CPP
# define C_LINKAGE_BEGIN extern "C"{
# define C_LINKAGE_END }
# define C_LINKAGE extern "C"
#else
# define C_LINKAGE_BEGIN
# define C_LINKAGE_END
# define C_LINKAGE
#endif


//#if CPP
//#define struct(x) struct x
//#else 
//#define struct(x) typedef struct x x;struct x
//#endif
//
//#if CPP
//#define enum(x) enum x
//#else
//#define enum(x) typedef enum x x;enum x
//#endif

#if CPP
#define ZERO_STRUCT {}
#else
#define ZERO_STRUCT {0}
#endif

#define KB(n) (((uint64_t)(n)) << 10)
#define MB(n) (((uint64_t)(n)) << 20)
#define GB(n) (((uint64_t)(n)) << 30)
#define TB(n) (((uint64_t)(n)) << 40)


#define min(x,y) ((x > y) ? (y) : (x))
#define max(x,y) ((x < y) ? (y) : (x))
#define clamp_top(a, max) min(a, max)
#define clamp_bottom(a, min) max(a, min)

#define is_pow2(x) (!((x) & ((x) -1)))
#define align_pow2(x, b) (((x) + (b) - 1) & (~((b) - 1)))

#define STRINGIFY_(S) #S
#define STRINGIFY(S) STRINGIFY_(S)

#define STRCAT_(A,B) A##B
#define STRCAT(A,B) STRCAT_(A,B)


#define ARRAY_COUNT(array) (int)((sizeof(array) / sizeof(array[0])))

#if !defined(PLATFORM_WINDOWS)
#if defined(_WIN32)
#define PLATFORM_WINDOWS 1
#else
#define PLATFORM_WINDOWS 0
#endif
#endif

#if !defined(_WIN32) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
#define PLATFORM_UNIX 1
#else
#define PLATFORM_UNIX 0
#endif

#if !defined(PLATFORM_LINUX)
#if defined(__linux__)
#define PLATFORM_LINUX 1
#else
#define PLATFORM_LINUX 0
#endif
#endif



#if !defined(COMPILER_MSVC)
#if defined(_MSC_VER) && !defined(__clang__)
#define COMPILER_MSVC 1
#else
#define COMPILER_MSVC 0
#endif
#endif

#if !defined(COMPILER_CLANG)
#if defined(__clang__)
#define COMPILER_CLANG 1
#else
#define COMPILER_CLANG 0
#endif
#endif

#if !defined(COMPILER_GCC)
#if defined(__GNUC__) && !defined(__clang__)
#define COMPILER_GCC 1
#else
#define COMPILER_GCC 0
#endif
#endif


#if COMPILER_GCC
#define alignof(x) __alignof__(x)
#elif defined(COMPILER_CLANG)
#define alignof(x) __alignof(x)
#elif defined(COMPILER_MSVC)
#define alignof(x) __alignof(x)
#else
#error unsupported compiler
#endif

#ifdef COMPILER_GCC 
#define THREAD_LOCAL __thread
#elif COMPILER_MSVC
#define THREAD_LOCAL __declspec(thread)
#else
#error unsupported_compiler
#endif

#if COMPILER_MSVC
#define THREAD_LOCAL __declspec(thread)
#elif COMPILER_CLANG
#define THREAD_LOCAL __thread
#elif COMPILER_GCC
#define THREAD_LOCAL __thread
#else
#error "Unsupported compiler"
#endif



#if COMPILER_GCC
#if defined(__SANITIZE_ADDRESS__)
#define ASAN_USE 1
#else
#define ASAN_USE 0
#endif
#elif COMPILER_MSVC
#if defined(__SANITIZE_ADDRESS__)
#define ASAN_USE 1
#else
#define ASAN_USE 0
#endif
#elif COMPILER_CLANG
#if defined(__has_feature)
#if  __has_feature(address_sanitizer)
#define ASAN_USE 1
#else
#define ASAN_USE 0
#endif
#endif
#endif

#if CPP
# define C_LINKAGE_BEGIN extern "C"{
# define C_LINKAGE_END }
# define C_LINKAGE extern "C"
#else
# define C_LINKAGE_BEGIN
# define C_LINKAGE_END
# define C_LINKAGE
#endif


#if ASAN_USE
C_LINKAGE void __asan_poison_memory_region(const volatile void *addr, size_t size);
C_LINKAGE void __asan_unpoison_memory_region(const volatile void *addr, size_t size);
#define asan_poison_memory_region(addr, size) __asan_poison_memory_region((addr), (size))
#define asan_unpoison_memory_region(addr, size) __asan_unpoison_memory_region((addr), (size))
#else
#define asan_poison_memory_region(addr, size) ((void)(addr), (void)(size))
#define asan_unpoison_memory_region(addr, size) ((void)(addr), (void)(size))
#endif



#ifndef STATIC_ASSERT
#define STATIC_ASSERT(COND,MSG) static int static_assertion_##MSG[(COND)?1:-1]
#endif



#ifndef FAIL_MSG
#define FAIL_MSG(...)\
    do {\
        fprintf(stderr,__VA_ARGS__);\
        fprintf(stderr,"\n %s:%d:%s \n\n",__FILE__, __LINE__, __func__);\
        exit(-1);\
    }while(0)
#endif


#ifndef ASSERT
#define ASSERT(x)\
    do {\
        if (!(x)) FAIL_MSG("Assertion failed %s", #x);\
    }while(0)
#endif


#endif // _TDF_H_
