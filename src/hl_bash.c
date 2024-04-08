#include <lt/ctype.h>
#include <lt/str.h>
#include <lt/mem.h>

#include "highlight.h"
#include "doc.h"

#define EMIT(x) do { \
	highl_t* new = lt_amalloc(alloc, sizeof(highl_t)); \
	new->len = it - start; \
	new->mode = (x); \
	*node = new; \
	node = &new->next; \
} while (0)

#define SKIP_INDENT() do { \
	char* start = it; \
	while (it < end && lt_is_space(*it)) \
		++it; \
	if (it > start) \
		EMIT(HLM_INDENT); \
} while (0)

static
hl_mode_t keyword(lstr_t str) {
	if (lt_lseq(str, CLSTR("alias"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("if"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("else"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("fi"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("for"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("do"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("done"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("set"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("then"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("export"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("return"))) return HLM_KEYWORD;
	return HLM_FUNCTION;
}

static LT_INLINE
b8 is_cmd_char(char c) {
	return lt_is_ident_body(c) || c == '-' || c == '/';
}

static
highl_t* gen_line(lstr_t line, lt_arena_t* alloc) {
	highl_t* head = NULL;
	highl_t** node = &head;
	char* it = line.str, *end = it + line.len;

cmd_start:
	SKIP_INDENT();

	// conditionals
	char* start = it;
	while (it < end && is_cmd_char(*it))
		++it;
	if (it > start) {
		lstr_t cmd = lt_lsfrom_range(start, it);

		if (it < end && *it == '=') {
			EMIT(HLM_IDENTIFIER);
		}
		else if (lt_lseq(cmd, CLSTR("sudo"))) {
			EMIT(HLM_KEYWORD);

			for (;;) {
				SKIP_INDENT();

				if (it >= end || *it != '-') {
					break;
				}

				start = it;
				while (it < end && is_cmd_char(*it)) {
					++it;
				}
				EMIT(HLM_OPERATOR);
			}

			goto cmd_start;
		}
		else {
			EMIT(keyword(cmd));
		}
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

		case ';': case '|': case '&': case '>': case '<':
			mode = HLM_PUNCTUATION;
			EMIT(mode);
			goto cmd_start;

		case '"':
			while (it < end && (c = *it++) != '"') {
				if (it < end && c == '\\')
					++it;
			}
			mode = HLM_STRING;
			break;

		case '~':
			mode = HLM_OPERATOR;
			break;

		case '\'':
			while (it < end && (c = *it++) != '\'') {
				if (it < end && c == '\\')
					++it;
			}
			mode = HLM_CHAR;
			break;

		case '\\': mode = HLM_PUNCTUATION; break;

		case '%': case '*': case '(': case ')': case '{': case '}': case '=':
		 	mode = HLM_OPERATOR;
		 	break;

		case '+': case ':': case '!':
			if (it < end && *it == '=') {
				++it;
				mode = HLM_OPERATOR;
			}
			else
				mode = HLM_IDENTIFIER;
			break;

		case '-':
			while (it < end && is_cmd_char(*it))
				++it;
			mode = HLM_OPERATOR;
			break;

		case '$':
			if (*it == '@' || *it == '<' || *it == '-') {
				++it;
			}
			else if (*it == '(' || *it == '{') {
				++it;
				EMIT(HLM_OPERATOR);
				start = it;

				while (it < end && is_cmd_char(*it))
					++it;
			}
			else {
				while (it < end && is_cmd_char(*it))
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

			while (it < end && is_cmd_char(*it))
				++it;

			mode = HLM_IDENTIFIER;
			break;
		}

		EMIT(mode);
	}
	*node = NULL;

	return head;
}

highl_t** hl_generate_bash(doc_t* doc, lt_arena_t* alloc) {
	usz line_count = lt_texted_line_count(&doc->ed);

	highl_t** lines = lt_amalloc_lean(alloc, line_count * sizeof(highl_t*));

	for (usz i = 0; i < line_count; ++i)
		lines[i] = gen_line(lt_texted_line_str(&doc->ed, i), alloc);

	return lines;
}

