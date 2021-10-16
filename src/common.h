// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef COMMON_H
#define COMMON_H 1

// Platform
#if defined(__unix__)
#	define PLATFORM_UNIX
#endif

#if defined(linux)
#	define PLATFORM_LINUX
#elif defined(_WIN32) || defined(WIN32)
#	define PLATFORM_WINDOWS
#else
#	error Unsupported platform
#endif

// Compiler
#if defined(__clang__)
#	define CC_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#	define CC_GCC
#elif defined(_MSC_VER)
#	define CC_MSVC
#else
#	error Unsupported compiler
#endif

// Asserts
#define ENABLE_ASSERTS

#ifdef ENABLE_ASSERTS
#	include "assert.h"
#	define ASSERT(x) ((void)( (x) ? 0 : assert_failed(__FILE__, __LINE__, #x) ))
#	define ASSERT_NOT_REACHED() assert_unreachable_failed(__FILE__, __LINE__)
#else
#	define ASSERT(x) ((void)0)
#	define ASSERT_NOT_REACHED() ((void)0)
#endif

// Useful macros
#define KB(n) ((n) * 1024)
#define MB(n) ((n) * 1048576)
#define GB(n) ((n) * 1073741824)

// Less ridiculous primitive names
#include <stddef.h>
#include <stdint.h>

typedef size_t usz;
typedef intmax_t isz;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef u8 b8;

// Attributes
#if defined(CC_CLANG) || defined(CC_GCC)
#	define ATTRIB(a) __attribute__((a))

#	define INLINE ATTRIB(always_inline)
#	define FLATTEN ATTRIB(flatten)
#	define PACKED(name) struct ATTRIB(packed) name
#	define NORETURN ATTRIB(noreturn)
#	define NODISCARD ATTRIB(warn_unused_result)
#elif defined(CC_MSVC)
#	define INLINE __forceinline
#	define FLATTEN
#	define PACKED(name) __pragma(pack(push, 1)) struct name __pragma(pack(pop))
#	define NORETURN __declspec(noreturn)
#	define NODISCARD _Check_return_
#endif

// Better strings that double as string views
typedef
struct lstr {
	char* str;
	usz len;
} lstr_t;

#define LSTR(s, l) ((lstr_t){ (s), l })
#define CLSTR(s) ((lstr_t){ (s), strlen((s)) })
#define NLSTR() ((lstr_t){ NULL, 0 })

// Fatal errors
void NORETURN ferr(const char* str);
void NORETURN ferrf(const char* fmt, ...);

// Warnings
void werr(const char* str);
void werrf(const char* fmt, ...);

// strerror wrapper
char* os_err_str(void);

#define ISIZE_MAX (SIZE_MAX >> 1)

#endif
