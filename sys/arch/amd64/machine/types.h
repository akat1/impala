#ifndef __MACHINE_TYPES_H
#define __MACHINE_TYPES_H

typedef unsigned char u_char;
typedef unsigned int u_int;
typedef unsigned long u_long;

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned long long int uint64_t;

typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;


typedef signed char  int8_t;
typedef signed short int16_t;
typedef signed int   int32_t;
typedef signed long long int int64_t;

#define __ARCH_amd64__

#ifdef __ARCH_x86__
typedef uint32_t uintptr_t;
typedef int32_t  intptr_t;
#elif defined(__ARCH_amd64__)
typedef uint64_t uintptr_t;
typedef int64_t  intptr_t;
#else
#error "unknown architecture"
#endif

typedef void* addr_t;
typedef const void* caddr_t;


#ifndef __SIZE_T
#define __SIZE_T
    #ifdef __ARCH_x86__
    typedef uint32_t size_t;
    #elif defined(__ARCH_amd64__)
    typedef uint64_t size_t;
    #else
    #error "unknown architecture"
    #endif
#endif

#ifndef __PTRDIFF_T
#define __PTRDIFF_T
    #ifdef __ARCH_x86__
    typedef uint32_t ptrdiff_t;
    #elif defined(__ARCH_amd64__)
    typedef uint64_t ptrdiff_t;
    #else
    #error "unknown architecture"
    #endif
#endif


#ifdef __ARCH_x86__
typedef int32_t  ssize_t;
#elif defined(__ARCH_amd64__)
typedef int64_t  ssize_t;
#else
#error "unknown architecture"
#endif

#endif
