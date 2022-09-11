// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef COMMON_H
#define COMMON_H 1

#include <lt/lt.h>

// Fatal errors
void LT_NORETURN ferr(const char* str);
void LT_NORETURN ferrf(const char* fmt, ...);

// Warnings
void werr(const char* str);
void werrf(const char* fmt, ...);

// strerror wrapper
char* os_err_str(void);

#define ISIZE_MAX (SIZE_MAX >> 1)

#endif
