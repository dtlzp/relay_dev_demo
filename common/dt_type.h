
/**
 * Copyright (c) 2022 Shenzhen Dingtian Technologies Co.,Ltd All rights reserved.
 * website: www.dingtian-tech.com
 * author: lzp<lzp@dingtian-tech.com>
 * sales: stephen liu<stephen@dingtian-tech.com>
 * date: 2022/6/20
 * file: dt_type.h
 * function: dingtian type define
 */
 
#ifndef _DT_TYPE_H_
#define _DT_TYPE_H_

#ifndef FALSE
    #define FALSE   (0)
#endif
#ifndef TRUE
    #define TRUE    (1)
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

#include <stdint.h>

typedef unsigned char       u8_t;
typedef int8_t              s8_t;
typedef char                b8_t;

typedef unsigned short      u16_t;
typedef short               s16_t;

typedef int                 b32_t;

typedef unsigned int        u32_t;
typedef int                 s32_t;

typedef unsigned long long  u64_t;
typedef long long           s64_t;

typedef float               f32_t;
typedef double              f64_t;

typedef unsigned int*       p32_t;

#ifdef _WIN32

#ifdef _WIN64
typedef u64_t*  up_t;
typedef u64_t   pv_t;
#define PADDR_STR   "%016llx"
#else /* win32 */
typedef u32_t*  up_t;
typedef u32_t   pv_t;
#define PADDR_STR   "%08x"
#endif

#else/* gcc/linux/unix */

#if (64 == __INT_WIDTH__)
typedef u64_t*  up_t;
typedef u64_t   pv_t;
#define PADDR_STR   "%016llx"
#else /* x86 */
typedef u32_t*  up_t;
typedef u32_t   pv_t;
#define PADDR_STR   "%08x"
#endif

#endif

////////////////////////////////////////////////
typedef unsigned char       u8;
typedef char                s8;
typedef char                b8;

typedef unsigned short      u16;
typedef short               s16;

typedef int                 b32;

typedef unsigned int        u32;
typedef int                 s32;

typedef unsigned long long  u64;
typedef long long           s64;

typedef unsigned int*       p32;

typedef u8  byte;

typedef s64_t sec_t;

#define S8SIZE  (1)
#define U8SIZE  (1)
#define S16SIZE (2)
#define U16SIZE (2)
#define S32SIZE (4)
#define U32SIZE (4)
#define S64SIZE (8)
#define U64SIZE (8)
#define P32SIZE (4)

#define S8MIN   (0x80)
#define S8MAX   (0x7f)
#define U8MIN   (0x00U)
#define U8MAX   (0xffU)
#define S16MIN  (0x8000)
#define S16MAX  (0x7fff)
#define U16MIN  (0x0000U)
#define U16MAX  (0xffffU)
#define U24MAX  (0xffffffU)
#define S32MIN  (0x80000000)
#define S32MAX  (0x7fffffff)
#define U32MIN  (0x00000000U)
#define U32MAX  (0xffffffffU)
#define S64MIN  (0x8000000000000000LL)
#define S64MAX  (0x7fffffffffffffffLL)
#define U64MIN  (0x0000000000000000ULL)
#define U64MAX  (0xffffffffffffffffULL)

#define CRLF    "\r\n"
#define CR      '\r'
#define LF      '\n'

#endif /* end _DT_TYPE_H_ */



