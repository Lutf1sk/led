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
	case 'a':
		if (lt_lseq(str, CLSTR("as"))) return 1;
		if (lt_lseq(str, CLSTR("async"))) return 1;
		if (lt_lseq(str, CLSTR("await"))) return 1;
		break;

	case 'b':
		if (lt_lseq(str, CLSTR("break"))) return 1;
		break;

	case 'c':
		if (lt_lseq(str, CLSTR("const"))) return 1;
		if (lt_lseq(str, CLSTR("continue"))) return 1;
		if (lt_lseq(str, CLSTR("crate"))) return 1;
		break;

	case 'd':
		if (lt_lseq(str, CLSTR("dyn"))) return 1;
		break;

	case 'e':
		if (lt_lseq(str, CLSTR("else"))) return 1;
		if (lt_lseq(str, CLSTR("enum"))) return 1;
		if (lt_lseq(str, CLSTR("extern"))) return 1;
		break;

	case 'f':
		if (lt_lseq(str, CLSTR("false"))) return 1;
		if (lt_lseq(str, CLSTR("fn"))) return 1;
		if (lt_lseq(str, CLSTR("for"))) return 1;
		break;

	case 'i':
		if (lt_lseq(str, CLSTR("if"))) return 1;
		if (lt_lseq(str, CLSTR("impl"))) return 1;
		if (lt_lseq(str, CLSTR("in"))) return 1;
		break;

	case 'l':
		if (lt_lseq(str, CLSTR("let"))) return 1;
		if (lt_lseq(str, CLSTR("loop"))) return 1;
		break;

	case 'm':
		if (lt_lseq(str, CLSTR("match"))) return 1;
		if (lt_lseq(str, CLSTR("mod"))) return 1;
		if (lt_lseq(str, CLSTR("move"))) return 1;
		if (lt_lseq(str, CLSTR("mut"))) return 1;
		break;

	case 'p':
		if (lt_lseq(str, CLSTR("pub"))) return 1;
		break;

	case 'r':
		if (lt_lseq(str, CLSTR("ref"))) return 1;
		if (lt_lseq(str, CLSTR("return"))) return 1;
		break;

	case 's':
		if (lt_lseq(str, CLSTR("self"))) return 1;
		if (lt_lseq(str, CLSTR("Self"))) return 1;
		if (lt_lseq(str, CLSTR("static"))) return 1;
		if (lt_lseq(str, CLSTR("struct"))) return 1;
		if (lt_lseq(str, CLSTR("super"))) return 1;
		break;

	case 't':
		if (lt_lseq(str, CLSTR("trait"))) return 1;
		if (lt_lseq(str, CLSTR("true"))) return 1;
		if (lt_lseq(str, CLSTR("type"))) return 1;
		break;

	case 'u':
		if (lt_lseq(str, CLSTR("unsafe"))) return 1;
		if (lt_lseq(str, CLSTR("use"))) return 1;
		break;

	case 'w':
		if (lt_lseq(str, CLSTR("where"))) return 1;
		if (lt_lseq(str, CLSTR("while"))) return 1;
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
		if (lt_lseq(str, CLSTR("b8"))) return 1;
		break;

	case 'c':
		if (lt_lseq(str, CLSTR("const"))) return 1;
		if (lt_lseq(str, CLSTR("char"))) return 1;
		break;

	case 'd':
		if (lt_lseq(str, CLSTR("double"))) return 1;
		break;

	case 'e':
		if (lt_lseq(str, CLSTR("enum"))) return 1;
		break;

	case 'f':
		if (lt_lseq(str, CLSTR("float"))) return 1;
		if (lt_lseq(str, CLSTR("f32"))) return 1;
		if (lt_lseq(str, CLSTR("f64"))) return 1;
		break;

	case 'i':
		if (lt_lseq(str, CLSTR("int"))) return 1;
		if (lt_lseq(str, CLSTR("isz"))) return 1;
		if (lt_lseq(str, CLSTR("i8"))) return 1;
		if (lt_lseq(str, CLSTR("i16"))) return 1;
		if (lt_lseq(str, CLSTR("i32"))) return 1;
		if (lt_lseq(str, CLSTR("i64"))) return 1;
		break;

	case 'l':
		if (lt_lseq(str, CLSTR("long"))) return 1;
		break;

	case 's':
		if (lt_lseq(str, CLSTR("signed"))) return 1;
		if (lt_lseq(str, CLSTR("short"))) return 1;
		if (lt_lseq(str, CLSTR("struct"))) return 1;
		break;

	case 'u':
		if (lt_lseq(str, CLSTR("union"))) return 1;
		if (lt_lseq(str, CLSTR("unsigned"))) return 1;
		if (lt_lseq(str, CLSTR("usz"))) return 1;
		if (lt_lseq(str, CLSTR("u8"))) return 1;
		if (lt_lseq(str, CLSTR("u16"))) return 1;
		if (lt_lseq(str, CLSTR("u32"))) return 1;
		if (lt_lseq(str, CLSTR("u64"))) return 1;
		break;

	case 'v':
		if (lt_lseq(str, CLSTR("volatile"))) return 1;
		if (lt_lseq(str, CLSTR("void"))) return 1;
		break;

	default:
		break;
	}
	return 0;
}

static
highl_t* gen_line(lstr_t line, multiline_mode_t* ml_mode, lt_arena_t* alloc) {
	highl_t* head = NULL;
	highl_t** node = &head;
	int c = 0;
	usz i = 0, start = 0;

	if (*ml_mode == MLMODE_COMMENT)
		goto parse_multiline_comment;

	while (i < line.len) {
		c = line.str[i];
		start = i++;

		modeid_t mode = HLM_UNKNOWN;

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

		highl_t* new = lt_amalloc_lean(alloc, sizeof(highl_t));
		new->len = i - start;
		new->mode = mode;

		*node = new;
		node = &new->next;
	}
	*node = NULL;

	return head;
}

highl_t** hl_generate_rust(doc_t* doc, lt_arena_t* alloc) {
	usz line_count = lt_texted_line_count(&doc->ed);

	highl_t** lines = lt_amalloc_lean(alloc, line_count * sizeof(highl_t*));

	multiline_mode_t mode = MLMODE_NONE;
	for (usz i = 0; i < line_count; ++i)
		lines[i] = gen_line(lt_texted_line_str(&doc->ed, i), &mode, alloc);

	return lines;
}

