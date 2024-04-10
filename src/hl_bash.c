#include <lt/ctype.h>
#include <lt/str.h>
#include <lt/mem.h>

#include "highlight.h"
#include "doc.h"

static
hl_mode_t keyword(lstr_t str) {
	if (lt_lseq(str, CLSTR("if"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("then"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("else"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("elif"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("fi"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("case"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("esac"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("for"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("select"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("while"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("until"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("do"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("done"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("in"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("function"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("time"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("coproc"))) return HLM_KEYWORD;

	if (lt_lseq(str, CLSTR("alias"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("set"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("source"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("export"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("return"))) return HLM_KEYWORD;
	if (lt_lseq(str, CLSTR("exec"))) return HLM_KEYWORD;

	if (lt_lseq(str, CLSTR("sudo"))) return HLM_KEYWORD;
	return HLM_FUNCTION;
}

static LT_INLINE
b8 is_cmd_char(char c) {
	return lt_is_ident_body(c) || c == '-' || c == '/' || c == '.';
}

static LT_INLINE
b8 char_pending(char** it, char* end, char c) {
	return *it < end && **it == c;
}

static
void emit(highl_t*** node, hl_tk_t toktype, usz len, lt_arena_t* alloc) {
	if (!len) {
		return;
	}

	highl_t* new = lt_amalloc_lean(alloc, sizeof(highl_t));
	new->len = len;
	new->mode = toktype;
	**node = new;
	*node = &new->next;
}

static
lstr_t consume_cmd_str(char** it, char* end) {
	char* start = *it;
	while (*it < end && is_cmd_char(**it)) {
		++*it;
	}
	return lt_lsfrom_range(start, *it);
}

static
lstr_t consume_indent(char** it, char* end) {
	char* start = *it;
	while (*it < end && lt_is_space(**it)) {
		++*it;
	}
	return lt_lsfrom_range(start, *it);
}

static void gen_cmd(highl_t*** node, char** it, char* end, char terminator, lt_arena_t* alloc);

static
void gen_indent(highl_t*** node, char** it, char* end, lt_arena_t* alloc) {
	emit(node, HLM_INDENT, consume_indent(it, end).len, alloc);
}

static
void gen_insertion(highl_t*** node, char** it, char* end, lt_arena_t* alloc) {
	if (*it < end && (**it == '@' || **it == '<' || **it == '-')) {
		++*it;
		emit(node, HLM_CHAR, 1, alloc);
	}
	else if (char_pending(it, end, '(')) {
		++*it;
		emit(node, HLM_OPERATOR, 1, alloc);
		gen_cmd(node, it, end, ')', alloc);
	}
	else if (char_pending(it, end, '{')) {
		++*it;
		emit(node, HLM_OPERATOR, 1, alloc);
		emit(node, HLM_CHAR, consume_cmd_str(it, end).len, alloc);
	}
	else {
		emit(node, HLM_CHAR, consume_cmd_str(it, end).len, alloc);
	}
}

static
void gen_cmd(highl_t*** node, char** it, char* end, char terminator, lt_arena_t* alloc) {
	gen_indent(node, it, end, alloc);

	lstr_t cmd = consume_cmd_str(it, end);
	if (cmd.len) {
		if (char_pending(it, end, '=')) {
			emit(node, HLM_IDENTIFIER, cmd.len, alloc);
		}
		else for (;;) {
			emit(node, keyword(cmd), cmd.len, alloc);

			if (!lt_lseq(cmd, CLSTR("exec")) && !lt_lseq(cmd, CLSTR("sudo"))) {
				break;
			}

			for (;;) {
				gen_indent(node, it, end, alloc);
				if (*it >= end || **it != '-') {
					break;
				}
				emit(node, HLM_OPERATOR, consume_cmd_str(it, end).len, alloc);
			}

			cmd = consume_cmd_str(it, end);
		}
	}

	char c = 0xFF;
	while (*it < end && c != terminator) {
		c = **it;
		char* start = (*it)++;

		switch (c) {
		case '#': emit(node, HLM_COMMENT, (*it = end) - start, alloc); break;
		case '\\': emit(node, HLM_PUNCTUATION, *it - start, alloc); break;

		case ';': case '|': case '&':
			emit(node, HLM_PUNCTUATION, *it - start, alloc);
			gen_cmd(node, it, end, terminator, alloc);
			return;

		case '*': case '(': case ')': case '{': case '}': case '=': case '~': case '>': case '<':
		 	emit(node, HLM_OPERATOR, 1, alloc);
		 	break;

		case '"':
			while (*it < end) {
				c = *(*it)++;
				if (c == '\"') {
					break;
				}
				else if (*it < end && c == '\\') {
					++*it;
				}
				else if (c == '$') {
					emit(node, HLM_STRING, *it - start - 1, alloc);
					emit(node, HLM_CHAR, 1, alloc);
					gen_insertion(node, it, end, alloc);
					start = *it;
				}
			}
			emit(node, HLM_STRING, *it - start, alloc);
			break;

		case '\'':
			while (*it < end) {
				c = *(*it)++;
				if (c == '\'') {
					break;
				}
				else if (*it < end && c == '\\') {
					++*it;
				}
			}
			emit(node, HLM_CHAR, *it - start, alloc);
			break;

		case '!':
			if (char_pending(it, end, '=')) {
				++*it;
			}
			emit(node, HLM_OPERATOR, *it - start, alloc);
			break;

		case '-':
			emit(node, HLM_NUMBER, consume_cmd_str(it, end).len + 1, alloc);
			break;

		case '$':
			emit(node, HLM_CHAR, 1, alloc);
			gen_insertion(node, it, end, alloc);
			break;

		default:
			if (lt_is_space(c)) {
				emit(node, HLM_INDENT, consume_indent(it, end).len + 1, alloc);
			}
			else if (is_cmd_char(c)) {
				emit(node, HLM_IDENTIFIER, consume_cmd_str(it, end).len + 1, alloc);
			}
			else {
				emit(node, HLM_UNKNOWN, 1, alloc);
			}
			break;
		}
	}
}

static
highl_t* gen_line(lstr_t line, lt_arena_t* alloc) {
	highl_t* head;
	highl_t** node = &head;
	char* it = line.str, *end = it + line.len;

	gen_cmd(&node, &it, end, 0, alloc);

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

