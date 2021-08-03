// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef FOCUS_GOTO_H
#define FOCUS_GOTO_H 1

#include "focus.h"

void goto_line(void);

void draw_goto(global_t* ed_global, void* win, void* args);
void input_goto(global_t* ed_global, int c);

extern focus_t focus_goto;

#endif
