// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef FOCUS_H
#define FOCUS_H 1

#include "common.h"

#include <lt/fwd.h>

// Common

typedef void (*fn_focus_draw_t)(editor_t*, void*);
typedef void (*fn_focus_input_t)(editor_t*, u32);

typedef
struct focus {
	fn_focus_draw_t draw;
	void* draw_args;

	fn_focus_input_t input;
} focus_t;

extern focus_t focus;
extern lt_texted_t* line_input;

void focus_init(void);
b8 input_term_key(lt_texted_t* ed, u32 key);
b8 texted_input_term_key(lt_texted_t* ed, u32 key);
usz input_cursor_pos(lt_texted_t* ed);

// File browser

void browse_files(void);

void draw_browse_files(editor_t* editor, void* args);
void input_browse_files(editor_t* editor, u32 c);

// Filesystem browser

void browse_filesystem(void);

void draw_browse_filesystem(editor_t* editor, void* args);
void input_browse_filesystem(editor_t* editor, u32 c);

// Editor

void edit_file(editor_t* editor, doc_t* ed);

void input_editor(editor_t* ed, u32 c);

// Local find

void find_local_init(void);

void find_local(isz start_x, isz start_y);

void draw_find_local(editor_t* editor, void* args);
void input_find_local(editor_t* editor, u32 c);

// Notify error

void notify_error(char* str);

void draw_notify_error(editor_t* editor, void* args);
void input_notify_error(editor_t* editor, u32 c);

// Notify exit

void notify_exit(void);

void draw_exit(editor_t* editor, void* args);
void input_exit(editor_t* editor, u32 c);

// Terminal

void terminal(void);

void draw_terminal(editor_t* editor, void* args);
void input_terminal(editor_t* editor, u32 c);

// Command

void command(void);

void draw_command(editor_t* editor, void* args);
void input_command(editor_t* editor, u32 c);

#endif
