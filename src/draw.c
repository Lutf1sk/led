// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "draw.h"

#include <lt/io.h>

#include <string.h>

char* write_buf;
char* write_it;

void rec_str(char* str) {
	usz len = strlen(str);
	memcpy(write_it, str, len);
	write_it += len;
}

void rec_lstr(char* str, usz len) {
	memcpy(write_it, str, len);
	write_it += len;
}

void rec_c(char c) {
	*write_it++ = c;
}

void rec_nc(usz n, char c) {
	memset(write_it, c, n);
	write_it += n;
}


void rec_goto(u32 x, u32 y) {
	write_it += lt_sprintf(write_it, "\x1B[%ud;%udH", y, x);
}


void rec_clear(char* clr) {
	rec_str(clr);
	rec_str("\x1B[2J");
}

void rec_clearline(char* clr) {
	rec_str(clr);
	rec_str("\x1B[2K");
}


