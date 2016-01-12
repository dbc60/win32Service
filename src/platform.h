/* ========================================================================
   Author: Douglas B. Cuthbertson
   (C) Copyright 2015 by Douglas B. Cuthbertson. All Rights Reserved.
   ======================================================================== */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif


//
// NOTE(doug): Standard Types. Keep this file compatible with C, so all code can be ported to
// platforms that don't have C++. It will make it possible to write a platform-specific library in C and have
// the platform-independent layer just work.
//
#include <stdint.h>
#include <stddef.h>


typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef size_t memory_index;

typedef float real32;
typedef double real64;

// NOTE(doug): Very brief type names.
typedef int8    s08;
typedef int16   s16;
typedef int32   s32;
typedef int64   s64;

typedef int8    i08;
typedef int16   i16;
typedef int32   i32;
typedef int64   i64;

typedef uint8   u08;
typedef uint16  u16;
typedef uint32  u32;
typedef uint64  u64;

typedef bool32  b32;
typedef wchar_t wch;
typedef real32  r32;
typedef real64  r64;

// Complement the definition of CHAR_BIT
#define U08_BIT CHAR_BIT
#define U16_BIT (sizeof(u16) * U08_BIT)
#define U32_BIT (sizeof(u32) * U08_BIT)
#define U64_BIT (sizeof(u64) * U08_BIT)
#define S16_BIT (sizeof(s16) * U08_BIT)
#define S32_BIT (sizeof(s32) * U08_BIT)
#define S64_BIT (sizeof(s64) * U08_BIT)

#define U08_MASK ((u08)(~0))
#define U16_MASK ((u16)(~0))
#define U16_MASK_HIGH   0xFF00
#define U16_MASK_LOW    0x00FF
#define U32_MASK ((u32)(~0))
#define U32_MASK_HIGH   0xFFFF0000
#define U32_MASK_LOW    0x0000FFFF
#define U64_MASK ((u64)(~0))
#define U64_MASK_HIGH   0xFFFFFFFF00000000
#define U64_MASK_LOW    0x00000000FFFFFFFF
#define S08_MASK ((s08)(~0))
#define S16_MASK ((s16)(~0))
#define S32_MASK ((s32)(~0))
#define S64_MASK ((s64)(~0))

#define BYTES_PER_U16 (sizeof(u16) / sizeof(u08))

#define Kilobytes(V) ((V) * 1024LL)
#define Megabytes(V) (Kilobytes(V) * 1024LL)
#define Gigabytes(V) (Megabytes(V) * 1024LL)
#define Terabytes(V) (Gigabytes(V) * 1024LL)


// IMPORTANT(doug): Originally, these three macros defined different usages of the keyword static as
// 'internal', 'local_persist' and 'global_variable'. I've changed them to all-caps and changed 'internal' to
// 'INTERNAL_FUNCTION, because 'internal' is used as a field name in some Windows structures. For example, see
// line 64 of the C++ header file "c:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/include/xiosbase".
//
// NOTE that the xiobase header file is included if your code includes the C++ <string> header. It may be
// included in other situations, too. It wasn't a problem in Casey's code for Handmade Hero, because he
// avoided std::string and std::stringstream. I'll be eliminating std::string and similar types from the
// platform layer shortly, but it was a bit annoying to run into this. I'll leave my macros as they are,
// below:
#define INTERNAL_FUNCTION static
#define LOCAL_VARIABLE static
#define GLOBAL_VARIABLE static


#ifdef __cplusplus
}
#endif
