// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef FILE_BROWSER_H
#define FILE_BROWSER_H 1

#include "common.h"

#define PATH_MAX_LEN (512)

doc_t* fb_find_unsaved(void);
doc_t* fb_first_file(void);

usz fb_find_files(doc_t** out, usz out_count, lstr_t str);
doc_t* fb_find_file(lstr_t str);

doc_t* fb_open(editor_t* editor, lstr_t path);
void fb_close(doc_t* doc);

#endif
