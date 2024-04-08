// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "doc.h"
#include "editor.h"

#include <lt/io.h>
#include <lt/mem.h>
#include <lt/str.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BYTE_ORDER_MARK CLSTR("\xEF\xBB\xBF")

#include <unistd.h>

void doc_load(doc_t* doc, editor_t* ed) {
	LT_ASSERT(lt_texted_create(&doc->ed, lt_libc_heap) == LT_SUCCESS);
	doc->ed.usr = ed;
	doc->ed.find_visual_x = (lt_texted_find_visual_x_fn_t)cursor_x_to_screen_x;
	doc->ed.find_cursor_x = (lt_texted_find_cursor_x_fn_t)screen_x_to_cursor_x;

	lstr_t file;
	if (lt_freadallp(doc->path, &file, lt_libc_heap) != LT_SUCCESS) {
		doc->new = 1;
		return;
	}
	else {
		char cstr_path[LT_PATH_MAX];
		LT_ASSERT(doc->path.len + 1 < LT_PATH_MAX);
		memcpy(cstr_path, doc->path.str, doc->path.len);
		cstr_path[doc->path.len] = 0;

		doc->read_only = access(cstr_path, W_OK) != 0;
	}

	// Find line ending type
	usz first_line_len = lt_lssplit(file, '\n').len;
	doc->crlf = (first_line_len >= 1 && file.str[first_line_len - 1] == '\r');

	// Skip byte order mark if present
	char* text_start = file.str;
	char* text_end = file.str + file.len;
	if (lt_lsprefix(file, BYTE_ORDER_MARK)) {
		doc->leading_bom = 1;
		text_start += BYTE_ORDER_MARK.len;
	}

	lt_texted_input_str(&doc->ed, lt_lsfrom_range(text_start, text_end));
	lt_texted_gotoxy(&doc->ed, 0, 0, 1);

	lt_mfree(lt_libc_heap, file.str);
}

b8 doc_save(doc_t* doc) {
	lt_file_t* f = lt_fopenp(doc->path, LT_FILE_W, 0, lt_libc_heap);
	if (!f) {
		return 0;
	}
	if (doc->leading_bom && lt_fwrite(f, BYTE_ORDER_MARK.str, BYTE_ORDER_MARK.len) != BYTE_ORDER_MARK.len) {
		goto err0;
	}
	if (lt_texted_write_contents(&doc->ed, (lt_write_fn_t)lt_fwrite, f) < 0) {
		goto err0;
	}

	lt_fclose(f, lt_libc_heap);
	doc->unsaved = 0;
	doc->new = 0;
	doc->read_only = 0;
	return 1;

err0:
	lt_fclose(f, lt_libc_heap);
	return 0;
}

void doc_free(doc_t* doc) {
	lt_texted_destroy(&doc->ed);
}

