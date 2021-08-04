// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "doc.h"
#include "fhl.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

usz doc_find_str(doc_t* doc, lstr_t str, doc_pos_t* out_pos) {
	if (!str.len)
		return 0;

	doc_pos_t* out_it = out_pos;

	usz found_count = 0;
	for (usz i = 0; i < doc->line_count; ++i) {
		lstr_t* line = &doc->lines[i];

		for (usz j = 0; j + str.len <= line->len; ++j) {
			if (memcmp(&line->str[j], str.str, str.len) == 0) {
				++found_count;
				if (out_it)
					*(out_it++) = (doc_pos_t){ i, j };
			}
		}
	}

	return found_count;
}

void doc_insert_char(doc_t* doc, usz line_index, usz index, char ch) {
	doc->unsaved = 1;

	lstr_t* line = &doc->lines[line_index];
	line->str = realloc(line->str, line->len + 1);
	if (!line->str)
		ferrf("Failed to allocate memory: %s\n", os_err_str());

	if (index != line->len)
		memmove(&line->str[index + 1], &line->str[index], line->len - index);
	line->str[index] = ch;
	++line->len;
}

void doc_erase_char(doc_t* doc, usz line_index, usz index) {
	doc->unsaved = 1;

	lstr_t* line = &doc->lines[line_index];
	memmove(&line->str[index], &line->str[index + 1], --line->len - index);
}


void doc_insert_str(doc_t* doc, usz line_index, usz index, lstr_t str) {
	doc->unsaved = 1;

	lstr_t* line = &doc->lines[line_index];
	usz new_len = line->len + str.len;
	
	line->str = realloc(line->str, new_len);
	if (new_len && !line->str)
		ferrf("Failed to allocate memory: %s\n", os_err_str());

	if (index != line->len)
		memmove(&line->str[index + str.len], &line->str[index], line->len - index);
	memcpy(&line->str[index], str.str, str.len);
	line->len = new_len;
}

void doc_erase_str(doc_t* doc, usz line_index, usz index, usz len) {
	doc->unsaved = 1;
	
	
	lstr_t* line = &doc->lines[line_index];
	line->len -= len;
	//ferrf("li: %zu, i: %zu, len: %zu\n", line_index, index, len);
	memmove(&line->str[index], &line->str[index + len], line->len - index);
}

void doc_split_line(doc_t* doc, usz line_index, usz index) {
	doc->unsaved = 1;
	// Reallocate to fit new line
	doc->lines = realloc(doc->lines, (doc->line_count + 1) * sizeof(lstr_t));
	if (!doc->lines)
		ferrf("Failed to allocate memory: %s\n", os_err_str());

	if (line_index != doc->line_count)
		memmove(&doc->lines[line_index + 1], &doc->lines[line_index], (doc->line_count - line_index) * sizeof(lstr_t));
	++doc->line_count;

	// Replace newly allocated line
	lstr_t* old_line = &doc->lines[line_index];
	lstr_t new_line = LSTR(NULL, old_line->len - index);
	old_line->len -= new_line.len;

	if (new_line.len) {
		new_line.str = malloc(new_line.len);
		if (!new_line.str)
			ferrf("Failed to allocate memory: %s\n", os_err_str());
		memcpy(new_line.str, &old_line->str[index], new_line.len);
	}

	doc->lines[line_index + 1] = new_line;
}

void doc_merge_line(doc_t* doc, usz line_index) {
	doc->unsaved = 1;

	ASSERT(line_index);

	lstr_t old_line = doc->lines[line_index];
	lstr_t* new_line = &doc->lines[line_index - 1];
	memmove(&doc->lines[line_index], &doc->lines[line_index + 1], (doc->line_count - line_index - 1) * sizeof(lstr_t));

	usz new_len = new_line->len + old_line.len;
	if (new_len) {
		new_line->str = realloc(new_line->str, new_len);
		if (!new_line->str)
			ferrf("Failed to allocate memory: %s\n", os_err_str());
		memcpy(new_line->str + new_line->len, old_line.str, old_line.len);
		new_line->len = new_len;
	}

	if (old_line.str)
		free(old_line.str);

	--doc->line_count;
}

void doc_load(doc_t* doc) {
	void* fp = fhl_fopen_r(doc->path);
	usz size = 0;
	char* data = NULL;
	if (fp) {
		size = fhl_fsize(fp);
		data = malloc(size);
		if (fhl_fread(fp, data, size) != size)
			ferrf("Failed to read from '%s'\n", doc->path);
		fhl_fclose(fp);
		
		if (!fhl_permit_w(doc->path))
			doc->read_only = 1;
	}
	else
		doc->new = 1;

	doc->line_count = 0;
	for (usz i = 0; i < size;) {
		while (i < size && data[i++] != '\n')
			;
		++doc->line_count;
	}

	if (doc->line_count) {
		doc->lines = malloc(sizeof(lstr_t) * doc->line_count);
		if (!doc->lines)
			ferrf("Failed to allocate memory: %s\n", os_err_str());
	}
	else {
		// The only time the line count is 0 is if the size of the file is 0
		// To handle this, just create a single empty line and return
		doc->line_count = 1;
		doc->lines = malloc(sizeof(lstr_t));
		if (!doc->lines)
			ferrf("Failed to allocate memory: %s\n", os_err_str());
		doc->lines[0] = NLSTR();

		free(data);
		return;
	}

	int line_i = 0;
	for (usz i = 0; i < size;) {
		usz start = i;
		while (i < size && data[i] != '\n')
			++i;
		usz len = i++ - start;

		if (!len) {
			doc->lines[line_i++] = NLSTR();
			continue;
		}

		char* str = malloc(len);
		if (!str)
			ferrf("Failed to allocate memory: %s\n", os_err_str());
		memcpy(str, &data[start], len);
		doc->lines[line_i++] = LSTR(str, len);
	}

	free(data);
}

b8 doc_save(doc_t* doc) {
	void* fp = fhl_fopen_w(doc->path);
	if (!fp)
		return 0;

	for (usz i = 0; i < doc->line_count; ++i)
		fprintf(fp, "%.*s\n", (int)doc->lines[i].len, doc->lines[i].str);

	fhl_fclose(fp);

	doc->unsaved = 0;
	doc->new = 0;
	doc->read_only = 0;

	return 1;
}

void doc_free(doc_t* doc) {
	for (usz i = 0; i < doc->line_count; ++i) {
		char* str = doc->lines[i].str;
		if (str)
			free(str);
	}
	if (doc->lines)
		free(doc->lines);
}

