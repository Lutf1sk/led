#include "highlight.h"
#include "doc.h"
#include "token_chars.h"
#include "chartypes.h"
#include "allocators.h"

#include <string.h>

typedef
enum multiline_mode {
	MLMODE_NONE,
	MLMODE_COMMENT,
} multiline_mode_t;

static
b8 lstreq(lstr_t s1, lstr_t s2) {
	if (s1.len != s2.len)
		return 0;
	return memcmp(s1.str, s2.str, s1.len) == 0;
}

static
b8 is_keyword(lstr_t str) {
	if (!str.len)
		return 0;

	switch (str.str[0]) {
	case 'b':
		if (lstreq(str, CLSTR("break"))) return 1;
		break;

	case 'c':
		if (lstreq(str, CLSTR("continue"))) return 1;
		if (lstreq(str, CLSTR("case"))) return 1;
		break;

	case 'd':
		if (lstreq(str, CLSTR("default"))) return 1;
		if (lstreq(str, CLSTR("do"))) return 1;
		if (lstreq(str, CLSTR("define"))) return 1;
		if (lstreq(str, CLSTR("def"))) return 1;
		break;

	case 'e':
		if (lstreq(str, CLSTR("else"))) return 1;
		if (lstreq(str, CLSTR("endif"))) return 1;
		if (lstreq(str, CLSTR("extern"))) return 1;
		if (lstreq(str, CLSTR("elif"))) return 1;
		break;

	case 'f':
		if (lstreq(str, CLSTR("for"))) return 1;
		break;

	case 'g':
		if (lstreq(str, CLSTR("goto"))) return 1;
		break;

	case 'i':
		if (lstreq(str, CLSTR("if"))) return 1;
		if (lstreq(str, CLSTR("inline"))) return 1;
		if (lstreq(str, CLSTR("include"))) return 1;
		if (lstreq(str, CLSTR("ifdef"))) return 1;
		if (lstreq(str, CLSTR("ifndef"))) return 1;
		break;

	case 'l':
		if (lstreq(str, CLSTR("let"))) return 1;
		break;

	case 'n':
		if (lstreq(str, CLSTR("null"))) return 1;
		break;

	case 'r':
		if (lstreq(str, CLSTR("return"))) return 1;
		break;

	case 's':
		if (lstreq(str, CLSTR("switch"))) return 1;
		if (lstreq(str, CLSTR("static"))) return 1;
		break;

	case 't':
		if (lstreq(str, CLSTR("typedef"))) return 1;
		break;

	case 'u':
		if (lstreq(str, CLSTR("undef"))) return 1;
		break;

	case 'w':
		if (lstreq(str, CLSTR("while"))) return 1;
		break;

	default:
		break;
	}
	return 0;
}

static
b8 is_datatype(lstr_t str) {
	if (!str.len)
		return 0;

	switch (str.str[0]) {
	case 'b':
		if (lstreq(str, CLSTR("b8"))) return 1;
		break;

	case 'c':
		if (lstreq(str, CLSTR("const"))) return 1;
		if (lstreq(str, CLSTR("char"))) return 1;
		break;

	case 'd':
		if (lstreq(str, CLSTR("double"))) return 1;
		break;

	case 'e':
		if (lstreq(str, CLSTR("enum"))) return 1;
		break;

	case 'f':
		if (lstreq(str, CLSTR("float"))) return 1;
		if (lstreq(str, CLSTR("f32"))) return 1;
		if (lstreq(str, CLSTR("f64"))) return 1;
		break;

	case 'i':
		if (lstreq(str, CLSTR("int"))) return 1;
		if (lstreq(str, CLSTR("isz"))) return 1;
		if (lstreq(str, CLSTR("i8"))) return 1;
		if (lstreq(str, CLSTR("i16"))) return 1;
		if (lstreq(str, CLSTR("i32"))) return 1;
		if (lstreq(str, CLSTR("i64"))) return 1;
		break;

	case 'l':
		if (lstreq(str, CLSTR("long"))) return 1;
		break;

	case 's':
		if (lstreq(str, CLSTR("signed"))) return 1;
		if (lstreq(str, CLSTR("short"))) return 1;
		if (lstreq(str, CLSTR("struct"))) return 1;
		break;

	case 'u':
		if (lstreq(str, CLSTR("union"))) return 1;
		if (lstreq(str, CLSTR("unsigned"))) return 1;
		if (lstreq(str, CLSTR("usz"))) return 1;
		if (lstreq(str, CLSTR("u8"))) return 1;
		if (lstreq(str, CLSTR("u16"))) return 1;
		if (lstreq(str, CLSTR("u32"))) return 1;
		if (lstreq(str, CLSTR("u64"))) return 1;
		break;

	case 'v':
		if (lstreq(str, CLSTR("volatile"))) return 1;
		if (lstreq(str, CLSTR("void"))) return 1;
		break;

	default:
		break;
	}
	return 0;
}

static
highl_t* gen_line(aframe_t* arena, lstr_t line, multiline_mode_t* ml_mode) {
	highl_t* head = NULL;
	highl_t** node = &head;
	int c = 0;
	usz i = 0, start = 0;

	if (*ml_mode == MLMODE_COMMENT)
		goto parse_multiline_comment;

	while (i < line.len) {
		c = line.str[i];
		start = i++;

		highl_mode_t mode = HLM_UNKNOWN;

		switch (c) {
		case '/':
			if (i >= line.len) {
				mode = HLM_OPERATOR;
				break;
			}

			c = line.str[i];
			if (c == '/') {
				i = line.len;
				mode = HLM_COMMENT;
				break;
			}
			else if (c == '*') {
				*ml_mode = MLMODE_COMMENT;

			parse_multiline_comment:
				while (i < line.len) {
					c = line.str[i++];
					if (c == '*' && i < line.len && line.str[i] == '/') {
						++i;
						*ml_mode = MLMODE_NONE;
						break;
					}
				}
				mode = HLM_COMMENT;
				break;
			}

			mode = HLM_OPERATOR;
			break;

		case '#':
			mode = HLM_HASH;
			break;

		case '"':
			while (i < line.len) {
				c = line.str[i++];
				if (c == '"')
					break;
				if (c == '\\' && i < line.len)
					++i;
			}
			mode = HLM_STRING;
			break;

		case '\'':
			while (i < line.len) {
				c = line.str[i++];
				if (c == '\'')
					break;
				if (c == '\\' && i < line.len)
					++i;
			}
			mode = HLM_CHAR;
			break;

		case '_':
			mode = HLM_IDENTIFIER;
			break;

		default:
			if (is_space(c)) {
				while (i < line.len) {
					c = line.str[i];
					if (!is_space(c))
						break;
					++i;
				}
				mode = HLM_INDENT;
			}
			else if (is_ident_head(c)) {
				mode = HLM_IDENTIFIER;

				while (i < line.len) {
					c = line.str[i];
					if (!is_ident_body(c)) {
						if (c == '(')
							mode = HLM_FUNCTION;
						break;
					}
					++i;
				}

				lstr_t tk = LSTR(&line.str[start], i - start);

				if (is_keyword(tk))
					mode = HLM_KEYWORD;
				else if (is_datatype(tk))
					mode = HLM_DATATYPE;
			}
			else if (is_numeric_head(c)) {
				while (i < line.len) {
					if (!is_numeric_body(line.str[i]))
						break;
					++i;
				}
				mode = HLM_NUMBER;
			}
			else if (is_operator(c))
				mode = HLM_OPERATOR;
			else if (is_punc(c))
				mode = HLM_PUNCTUATION;
			break;
		}

		highl_t* new = aframe_reserve(arena, sizeof(highl_t));
		new->len = i - start;
		new->mode = mode;

		*node = new;
		node = &new->next;
	}
	*node = NULL;

	return head;
}

highl_t** highl_generate(aframe_t* arena, doc_t* doc) {
	highl_t** lines = aframe_reserve(arena, doc->line_count * sizeof(highl_t*));

	multiline_mode_t mode = MLMODE_NONE;
	usz line_count = doc->line_count;

	for (usz i = 0; i < line_count; ++i)
		lines[i] = gen_line(arena, doc->lines[i], &mode);

	return lines;
}

