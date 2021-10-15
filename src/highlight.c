#include "highlight.h"
#include "doc.h"
#include "token_chars.h"
#include "allocators.h"
#include "chartypes.h"

#include <ctype.h>

#include <string.h>
#include <stdlib.h>

typedef
struct ctx {
	lstr_t line;
	usz index;
} ctx_t;

static
char consume(ctx_t* cx) {
	if (cx->index >= cx->line.len)
		return 0;
	return cx->line.str[cx->index++];
}

static
char peek(ctx_t* cx) {
	return cx->index >= cx->line.len ? 0 : cx->line.str[cx->index];
}

static
b8 is_punctuation(char c) {
	switch (c) {
	case '.': case ',': case ':': case ';':
		return 1;
	default:
		return 0;
	}
}

static
b8 is_operator(char c) {
	switch (c) {
	case '+': case '-': case '*': case '/': case '%':
	case '&': case '|': case '^': case '~':
	case '=': case '!': case '?': case '<': case '>':
		return 1;
	default:
		return 0;
	}
}

static
b8 is_bracket(char c) {
	switch (c) {
	case '[': case ']': case '{': case '}': case '(': case ')':
		return 1;
	default:
		return 0;
	}
}

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
highl_t* gen_line(pframe_t* pool, lstr_t* line) {
	if (!line->len)
		return NULL;

	ctx_t context;
	context.line = *line;
	context.index = 0;
	ctx_t* cx = &context;

	highl_t* head;
	highl_t** node = &head;
	char c;
	while ((c = peek(cx))) {
		highl_t* new = pframe_reserve(pool);
		if (!new)
			return NULL;

		char* tk_start = &cx->line.str[cx->index];
		new->len = 1;
		consume(cx);

		switch (c) {
		case '/':
			if (peek(cx) == '/') {
				while (consume(cx))
					++new->len;
				new->mode = HLM_COMMENT;
			}
			else
				new->mode = HLM_OPERATOR;
			break;

		case '#':
			new->mode = HLM_HASH;
			break;

		case '"':
			c = peek(cx);
			while (c && c != '"') {
				c = consume(cx);
				if (c == '\\' && peek(cx) == '"') {
					consume(cx);
					++new->len;
				}
				c = peek(cx);
				++new->len;
			}
			if (c) {
				consume(cx);
				++new->len;
			}
			new->mode = HLM_STRING;
			break;

		case '\'':
			c = peek(cx);
			while (c && c != '\'') {
				c = consume(cx);
				if (c == '\\' && peek(cx) == '\'') {
					consume(cx);
					++new->len;
				}
				c = peek(cx);
				++new->len;
			}
			if (c) {
				consume(cx);
				++new->len;
			}
			new->mode = HLM_CHAR;
			break;

		default:
			if (isspace(c)) {
				c = peek(cx);
				while (c && isspace(c)) {
					consume(cx);
					c = peek(cx);
					++new->len;
				}
				new->mode = HLM_INDENT;
			}
			else if (is_numeric_head(c)) {
				c = peek(cx);
				while (c && is_numeric_body(c)) {
					consume(cx);
					c = peek(cx);
					++new->len;
				}
				new->mode = HLM_NUMBER;
			}
			else if (is_punctuation(c))
				new->mode = HLM_PUNCTUATION;
			else if (is_bracket(c))
				new->mode = HLM_BRACKET;
			else if (is_operator(c)) {
				c = peek(cx);
				while (c && is_operator(c)) {
					consume(cx);
					c = peek(cx);
					++new->len;
				}
				new->mode = HLM_OPERATOR;
			}
			else if (is_ident_head(c)) {
				c = peek(cx);
				while (c && is_ident_body(c)) {
					consume(cx);
					c = peek(cx);
					++new->len;
				}

				lstr_t tk = LSTR(tk_start, &cx->line.str[cx->index] - tk_start);

				if (is_keyword(tk))
					new->mode = HLM_KEYWORD;
				else if (is_datatype(tk))
					new->mode = HLM_DATATYPE;
				else if (peek(cx) == '(')
					new->mode = HLM_FUNCTION;
				else
					new->mode = HLM_IDENTIFIER;
			}
			else
				new->mode = HLM_UNKNOWN;
			break;
		}

		*node = new;
		node = &new->next;
	}
	*node = NULL;

	return head;
}

highl_t** highl_generate(pframe_t* pool, doc_t* doc) {
	highl_t** lines = malloc(doc->line_count * sizeof(highl_t*));
	if (!lines)
		ferrf("Failed to allocate memory: %s\n", os_err_str());

	pframe_free_all(pool);

	for (usz i = 0; i < doc->line_count; ++i)
		lines[i] = gen_line(pool, &doc->lines[i]);

	return lines;
}

