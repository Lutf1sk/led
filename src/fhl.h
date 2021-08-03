// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef FHL_H
#define FHL_H 1

#include "common.h"

void* fhl_fopen_r(const char* path);
void* fhl_fopen_w(const char* path);

void fhl_fclose(void* stream);

usz fhl_fsize(void* stream);

usz fhl_fread(void* stream, void* data, usz len);
usz fhl_fwrite(void* stream, const void* data, usz len);

b8 fhl_permit_r(const char* path);
b8 fhl_permit_w(const char* path);
b8 fhl_permit_x(const char* path);

#endif
