#include <lt/ctype.h>
#include <lt/str.h>
#include <lt/mem.h>

#include "highlight.h"
#include "doc.h"

#define EMIT(x) { \
	highl_t* new = lt_malloc(alloc, sizeof(highl_t)); \
	new->len = it - start; \
	new->mode = (x); \
	*node = new; \
	node = &new->next; \
}

static
hl_mode_t keyword(lstr_t str) {
	if (lt_lstr_eq(str, CLSTR("ifdef"))) return HLM_KEYWORD;
	if (lt_lstr_eq(str, CLSTR("ifndef"))) return HLM_KEYWORD;
	if (lt_lstr_eq(str, CLSTR("ifeq"))) return HLM_KEYWORD;
	if (lt_lstr_eq(str, CLSTR("ifneq"))) return HLM_KEYWORD;
	if (lt_lstr_eq(str, CLSTR("endif"))) return HLM_KEYWORD;
	if (lt_lstr_eq(str, CLSTR("else"))) return HLM_KEYWORD;
	return HLM_IDENTIFIER;
}

static
highl_t* gen_line(lstr_t line, lt_arena_t* alloc) {
	highl_t* head = NULL;
	highl_t** node = &head;
	char* it = line.str, *end = it + line.len;

	b8 is_command = 0;

	// skip leading indent
	if (it < end && lt_is_space(*it)) {
		is_command = 1;

		char* start = it;
		while (it < end && lt_is_space(*it))
			++it;
		if (it > start)
			EMIT(HLM_INDENT);
	}

	// modifiers
	char* start = it;
	while (it < end && (*it == '-' || *it == '@'))
		++it;
	if (it > start)
		EMIT(HLM_PUNCTUATION);

	// conditionals
	if (!is_command) {
		start = it;
		while (it < end && lt_is_ident_body(*it))
			++it;
		if (it > start)
			EMIT(keyword(lt_lstr_from_range(start, it)));
	}

	while (it < end) {
		char c = *it;
		char* start = it++;

		hl_mode_t mode = HLM_UNKNOWN;

		switch (c) {
		case '#':
			it = end;
			mode = HLM_COMMENT;
			break;

		case '"':
			while (it < end && (c = *it++) != '"') {
				if (it < end && c == '\\')
					++it;
			}
			mode = HLM_STRING;
			break;

		case '\'':
			while (it < end && (c = *it++) != '\'') {
				if (it < end && c == '\\')
					++it;
			}
			mode = HLM_CHAR;
			break;

		case '\\': mode = HLM_PUNCTUATION; break;

		case '(': case ')': case '{': case '}': case '%': case '*':
			mode = HLM_OPERATOR;
			break;

		case '+': case ':':
			if (it < end && *it == '=') {
				++it;
				mode = HLM_OPERATOR;
			}
			else
				mode = HLM_IDENTIFIER;
			break;

		case '=':
			mode = HLM_OPERATOR;
			break;

		case '$':
			if (*it == '@' || *it == '<') {
				++it;
			}
			else if (*it == '(' || *it == '{') {
				++it;
				EMIT(HLM_OPERATOR);
				start = it;

				while (it < end && lt_is_ident_body(*it))
					++it;
			}
			else {
				while (it < end && lt_is_ident_body(*it))
					++it;
			}
			mode = HLM_CHAR;
			break;

		default:
			if (lt_is_space(c)) {
				while (it < end && lt_is_space(*it))
					++it;
				mode = HLM_INDENT;
				break;
			}

			while (it < end && lt_is_ident_body(*it))
				++it;

			mode = HLM_IDENTIFIER;
			break;
		}

		EMIT(mode);
	}
	*node = NULL;

	return head;
}

highl_t** hl_generate_makefile(doc_t* doc, lt_arena_t* alloc) {
	usz line_count = lt_texted_line_count(&doc->ed);

	highl_t** lines = lt_amalloc_lean(alloc, line_count * sizeof(highl_t*));

	for (usz i = 0; i < line_count; ++i)
		lines[i] = gen_line(lt_texted_line_str(&doc->ed, i), alloc);

	return lines;
}

