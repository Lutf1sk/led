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

#define KW(x) if (lt_lseq(str, CLSTR(x))) return 1
	switch (str.str[0]) {
	case 'a':
		KW("abstract");
		KW("arguments");
		break;

	case 'b':
		KW("break");
		break;

	case 'c':
		KW("case");
		KW("catch");
		KW("const");
		KW("continue");
		KW("class");
		break;

	case 'd':
		KW("debugger");
		KW("default");
		KW("delete");
		KW("do");
		break;

	case 'e':
		KW("else");
		KW("eval");
		KW("enum");
		KW("export");
		KW("extends");
		break;

	case 'f':
		KW("false");
		KW("final");
		KW("finally");
		KW("for");
		KW("function");
		break;

	case 'g':
		KW("goto");
		break;

	case 'i':
		KW("if");
		KW("implements");
		KW("in");
		KW("instanceof");
		KW("interface");
		KW("import");
		break;

	case 'l':
		KW("let");
		break;

	case 'n':
		KW("native");
		KW("new");
		KW("null");
		break;

	case 'p':
		KW("package");
		KW("private");
		KW("protected");
		KW("public");
		break;

	case 'r':
		KW("return");
		break;

	case 's':
		KW("static");
		KW("switch");
		KW("synchronized");
		KW("super");
		break;

	case 't':
		KW("this");
		KW("throw");
		KW("throws");
		KW("transient");
		KW("true");
		KW("try");
		KW("typeof");
		break;

	case 'v':
		KW("var");
		KW("volatile");
		break;

	case 'w':
		KW("while");
		KW("with");
		break;

	case 'y':
		KW("yield");
		break;

	default:
		break;
	}
#undef KW
	return 0;
}

static
b8 is_datatype(lstr_t str) {
	if (!str.len)
		return 0;

#define DT(x) if (lt_lseq(str, CLSTR(x))) return 1
	switch (str.str[0]) {
	case 'b':
		DT("boolean");
		DT("byte");
		break;

	case 'c':
		DT("char");
		break;

	case 'd':
		DT("double");
		break;

	case 'f':
		DT("float");
		break;

	case 'i':
		DT("int");
		break;

	case 'l':
		DT("long");
		break;

	case 's':
		DT("short");
		break;

	case 'v':
		DT("void");
		break;

	default:
		break;
	}
#undef DT
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

highl_t** hl_generate_js(doc_t* doc, lt_arena_t* alloc) {
	usz line_count = lt_texted_line_count(&doc->ed);

	highl_t** lines = lt_amalloc_lean(alloc, line_count * sizeof(highl_t*));

	multiline_mode_t mode = MLMODE_NONE;
	for (usz i = 0; i < line_count; ++i)
		lines[i] = gen_line(lt_texted_line_str(&doc->ed, i), &mode, alloc);

	return lines;
}

