// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "common.h"

#if defined(PLATFORM_LINUX)
#	include <errno.h>
#	include <string.h>

#	define LAST_ERROR_STR() strerror(errno)
#elif defined(PLATFORM_WINDOWS)
#	include <windows.h>

#define LAST_ERROR_STR() strerror(GetLastError())
#endif

char* os_err_str(void) {
	return LAST_ERROR_STR();
}

// Fatal errors
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void ferr(const char* str) {
	fprintf(stderr, "FERR: %s", str);
	exit(1);
}

void ferrf(const char* fmt, ...) {
	fputs("FERR: ", stderr);

	va_list list;
	va_start(list, fmt);
	vfprintf(stderr, fmt, list);
	va_end(list);

	exit(1);
}

// Warnings
void werr(const char* str) {
	fprintf(stderr, "WARN: %s", str);
}

void werrf(const char* fmt, ...) {
	fputs("WARN: ", stderr);

	va_list list;
	va_start(list, fmt);
	vfprintf(stderr, fmt, list);
	va_end(list);
}
