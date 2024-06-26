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

// Notify exit

void notify_exit(void);

void draw_exit(editor_t* editor, void* args);
void input_exit(editor_t* editor, u32 c);

// Command

void command(void);

void draw_command(editor_t* editor, void* args);
void input_command(editor_t* editor, u32 c);

// Shell

void shell_load(lt_conf_t* cf);
void run_shell(void);

// Relative jump

void reljump(b8 bwd);

void draw_reljump(editor_t* editor, void* args);
void input_reljump(editor_t* editor, u32 c);

// Extract lines

void extract(b8 delete_src, b8 insert_new);

void draw_extract(editor_t* editor, void* args);
void input_extract(editor_t* editor, u32 c);

// Find character

void findch(b8 bwd);

void draw_findch(editor_t* editor, void* args);
void input_findch(editor_t* editor, u32 c);

#endif
