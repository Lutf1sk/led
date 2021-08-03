// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef FILE_BROWSER_H
#define FILE_BROWSER_H 1

#include "common.h"

#define PATH_MAX_LEN (512)

typedef struct editor editor_t;
typedef struct global global_t;

editor_t* fb_first_file(void);

usz fb_find_files(editor_t** out, usz out_count, lstr_t str);
editor_t* fb_find_file(lstr_t str);

editor_t* fb_open(global_t* ed_global, lstr_t path);
void fb_close(editor_t* ed);

#endif
