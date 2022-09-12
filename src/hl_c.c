#include <lt/ctype.h>
#include <lt/str.h>
#include <lt/mem.h>

#include "highlight.h"
#include "doc.h"

typedef
enum multiline_mode {
	MLMODE_NONE,
	MLMODE_COMMENT,
} multiline_mode_t;

static
b8 is_keyword(lstr_t str) {
	if (!str.len)
		return 0;

	switch (str.str[0]) {
	case 'b':
		if (lt_lstr_eq(str, CLSTR("break"))) return 1;
		break;

	case 'c':
		if (lt_lstr_eq(str, CLSTR("continue"))) return 1;
		if (lt_lstr_eq(str, CLSTR("case"))) return 1;
		break;

	case 'd':
		if (lt_lstr_eq(str, CLSTR("default"))) return 1;
		if (lt_lstr_eq(str, CLSTR("do"))) return 1;
		if (lt_lstr_eq(str, CLSTR("define"))) return 1;
		break;

	case 'e':
		if (lt_lstr_eq(str, CLSTR("else"))) return 1;
		if (lt_lstr_eq(str, CLSTR("endif"))) return 1;
		if (lt_lstr_eq(str, CLSTR("extern"))) return 1;
		if (lt_lstr_eq(str, CLSTR("elif"))) return 1;
		break;

	case 'f':
		if (lt_lstr_eq(str, CLSTR("for"))) return 1;
		break;

	case 'g':
		if (lt_lstr_eq(str, CLSTR("goto"))) return 1;
		break;

	case 'i':
		if (lt_lstr_eq(str, CLSTR("if"))) return 1;
		if (lt_lstr_eq(str, CLSTR("inline"))) return 1;
		if (lt_lstr_eq(str, CLSTR("include"))) return 1;
		if (lt_lstr_eq(str, CLSTR("import"))) return 1;
		if (lt_lstr_eq(str, CLSTR("ifdef"))) return 1;
		if (lt_lstr_eq(str, CLSTR("ifndef"))) return 1;
		break;

	case 'n':
		if (lt_lstr_eq(str, CLSTR("null"))) return 1;
		break;

	case 'r':
		if (lt_lstr_eq(str, CLSTR("return"))) return 1;
		break;

	case 's':
		if (lt_lstr_eq(str, CLSTR("switch"))) return 1;
		if (lt_lstr_eq(str, CLSTR("static"))) return 1;
		break;

	case 't':
		if (lt_lstr_eq(str, CLSTR("typedef"))) return 1;
		break;

	case 'u':
		if (lt_lstr_eq(str, CLSTR("undef"))) return 1;
		break;

	case 'w':
		if (lt_lstr_eq(str, CLSTR("while"))) return 1;
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
		if (lt_lstr_eq(str, CLSTR("b8"))) return 1;
		break;

	case 'c':
		if (lt_lstr_eq(str, CLSTR("const"))) return 1;
		if (lt_lstr_eq(str, CLSTR("char"))) return 1;
		break;

	case 'd':
		if (lt_lstr_eq(str, CLSTR("double"))) return 1;
		break;

	case 'e':
		if (lt_lstr_eq(str, CLSTR("enum"))) return 1;
		break;

	case 'f':
		if (lt_lstr_eq(str, CLSTR("float"))) return 1;
		if (lt_lstr_eq(str, CLSTR("f32"))) return 1;
		if (lt_lstr_eq(str, CLSTR("f64"))) return 1;
		break;

	case 'i':
		if (lt_lstr_eq(str, CLSTR("int"))) return 1;
		if (lt_lstr_eq(str, CLSTR("isz"))) return 1;
		if (lt_lstr_eq(str, CLSTR("i8"))) return 1;
		if (lt_lstr_eq(str, CLSTR("i16"))) return 1;
		if (lt_lstr_eq(str, CLSTR("i32"))) return 1;
		if (lt_lstr_eq(str, CLSTR("i64"))) return 1;
		break;

	case 'l':
		if (lt_lstr_eq(str, CLSTR("long"))) return 1;
		break;

	case 's':
		if (lt_lstr_eq(str, CLSTR("signed"))) return 1;
		if (lt_lstr_eq(str, CLSTR("short"))) return 1;
		if (lt_lstr_eq(str, CLSTR("struct"))) return 1;
		break;

	case 'u':
		if (lt_lstr_eq(str, CLSTR("union"))) return 1;
		if (lt_lstr_eq(str, CLSTR("unsigned"))) return 1;
		if (lt_lstr_eq(str, CLSTR("usz"))) return 1;
		if (lt_lstr_eq(str, CLSTR("u8"))) return 1;
		if (lt_lstr_eq(str, CLSTR("u16"))) return 1;
		if (lt_lstr_eq(str, CLSTR("u32"))) return 1;
		if (lt_lstr_eq(str, CLSTR("u64"))) return 1;
		break;

	case 'v':
		if (lt_lstr_eq(str, CLSTR("volatile"))) return 1;
		if (lt_lstr_eq(str, CLSTR("void"))) return 1;
		break;

	default:
		break;
	}
	return 0;
}

static
highl_t* gen_line(lstr_t line, multiline_mode_t* ml_mode, lt_alloc_t* alloc) {
	highl_t* head = NULL;
	highl_t** node = &head;
	int c = 0;
	usz i = 0, start = 0;

	if (*ml_mode == MLMODE_COMMENT)
		goto parse_multiline_comment;

	while (i < line.len) {
		c = line.str[i];
		start = i++;

		hl_mode_t mode = HLM_UNKNOWN;

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
			if (lt_is_space(c)) {
				while (i < line.len) {
					c = line.str[i];
					if (!lt_is_space(c))
						break;
					++i;
				}
				mode = HLM_INDENT;
			}
			else if (lt_is_ident_head(c)) {
				mode = HLM_IDENTIFIER;

				while (i < line.len) {
					c = line.str[i];
					if (!lt_is_ident_body(c)) {
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
			else if (lt_is_numeric_head(c)) {
				while (i < line.len) {
					if (!lt_is_numeric_body(line.str[i]))
						break;
					++i;
				}
				mode = HLM_NUMBER;
			}
			else if (lt_is_operator(c))
				mode = HLM_OPERATOR;
			else if (lt_is_punc(c))
				mode = HLM_PUNCTUATION;
			break;
		}

		highl_t* new = lt_malloc(alloc, sizeof(highl_t));
		new->len = i - start;
		new->mode = mode;

		*node = new;
		node = &new->next;
	}
	*node = NULL;

	return head;
}

highl_t** hl_generate_c(doc_t* doc, lt_alloc_t* alloc) {
	highl_t** lines = lt_malloc(alloc, doc->line_count * sizeof(highl_t*));

	multiline_mode_t mode = MLMODE_NONE;
	usz line_count = doc->line_count;

	for (usz i = 0; i < line_count; ++i)
		lines[i] = gen_line(doc->lines[i], &mode, alloc);

	return lines;
}

