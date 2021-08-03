// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef FOCUS_H
#define FOCUS_H 1

#include "common.h"

// Common

typedef struct global global_t;

typedef void (*fn_focus_draw_t)(global_t*, void*,void*);
typedef void (*fn_focus_input_t)(global_t*, int);

typedef
struct focus {
	fn_focus_draw_t draw;
	void* draw_args;
	
	fn_focus_input_t input;
} focus_t;

extern focus_t focus;

// File browser

void browse_files(void);

void draw_browse_files(global_t* ed_global, void* win, void* args);
void input_browse_files(global_t* ed_global, int c);

extern focus_t focus_browse_files;

// Filesystem browser

void browse_filesystem(void);

void draw_browse_filesystem(global_t* ed_global, void* win, void* args);
void input_browse_filesystem(global_t* ed_global, int c);

extern focus_t focus_browse_filesystem;

// Editor

void input_editor(global_t* ed, int c);

extern focus_t focus_editor;

// Local find

void find_local(isz start_x, isz start_y);

void draw_find_local(global_t* ed_global, void* win_, void* args);
void input_find_local(global_t* ed_global, int c);

extern focus_t focus_find_local;

// Notify error

void notify_error(char* str);

void draw_notify_error(global_t* ed_global, void* win_, void* args);
void input_notify_error(global_t* ed_global, int c);

extern focus_t focus_notify_error;

// Goto

void goto_line(void);

void draw_goto(global_t* ed_global, void* win, void* args);
void input_goto(global_t* ed_global, int c);

extern focus_t focus_goto;

#endif
