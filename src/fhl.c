// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "fhl.h"

#include <stdio.h>

#if defined(PLATFORM_LINUX)
#	include <unistd.h>

#	define FILE_R_PERM(path) (access((path), R_OK) == 0)
#	define FILE_W_PERM(path) (access((path), W_OK) == 0)
#	define FILE_X_PERM(path) (access((path), X_OK) == 0)
#elif defined(PLATFORM_WINDOWS)

#endif

void* fhl_fopen_r(const char* path) {
    return fopen(path, "r");
}

void* fhl_fopen_w(const char* path) {
    return fopen(path, "w");
}

void fhl_fclose(void* stream) {
	fclose(stream);
}

usz fhl_fsize(void* stream) {
    usz pos = ftell(stream);
    fseek(stream, 0, SEEK_END);
    usz size = ftell(stream);
    if (pos != size)
        fseek(stream, pos, SEEK_SET);
    return size;
}

usz fhl_fread(void* stream, void* data, usz len) {
    return fread(data, 1, len, stream);
}

usz fhl_fwrite(void* stream, const void* data, usz len) {
    return fwrite(data, 1, len, stream);
}

b8 fhl_permit_r(const char* path) {
	return FILE_R_PERM(path);
}

b8 fhl_permit_w(const char* path) {
	return FILE_W_PERM(path);
}

b8 fhl_permit_x(const char* path) {
	return FILE_X_PERM(path);
}

