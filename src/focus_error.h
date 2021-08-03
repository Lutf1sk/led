// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef FOCUS_ERROR_H
#define FOCUS_ERROR_H 1

#include "focus.h"

void notify_error(char* str);

void draw_notify_error(global_t* ed_global, void* win_, void* args);
void input_notify_error(global_t* ed_global, int c);

extern focus_t focus_notify_error;

#endif
