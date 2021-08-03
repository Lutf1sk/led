// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef FOCUS_BROWSE_FILES_H
#define FOCUS_BROWSE_FILES_H 1

#include "focus.h"

void browse_files(void);

void draw_browse_files(global_t* ed_global, void* win, void* args);
void input_browse_files(global_t* ed_global, int c);

extern focus_t focus_browse_files;

#endif
