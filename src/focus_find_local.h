// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef FOCUS_FIND_LOCAL_H
#define FOCUS_FIND_LOCAL_H 1

#include "focus.h"
#include "common.h"

void find_local(isz start_y, isz start_x);

void draw_find_local(global_t* ed_global, void* win, void* args);
void input_find_local(global_t* ed_global, int c);

extern focus_t focus_find_local;

#endif
