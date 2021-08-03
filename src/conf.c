// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "conf.h"

#include "chartypes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline INLINE
b8 is_numeric_head(char c) {
	return is_digit(c);
}

static inline INLINE
b8 is_numeric_body(char c) {
	return is_numeric_head(c) || c == '.' || is_alpha(c);
}

static inline INLINE
b8 is_ident_head(char c) {
	return is_alpha(c) || c == '_';
}

static inline INLINE
b8 is_ident_body(char c) {
	return is_ident_head(c) || is_digit(c);
}

typedef
struct parse_ctx {
	lstr_t data;
	usz it;
} parse_ctx_t;

static inline INLINE
char consume(parse_ctx_t* cx) {
	if (cx->it == cx->data.len)
		return 0;

	return cx->data.str[cx->it++];
}

static inline INLINE
char consume_char(parse_ctx_t* cx, char c) {
	char consumed = consume(cx);
	if (consumed != c)
		ferrf("Config: Expected '%c', got '%c'\n", c, consumed); // Error
	return consumed;
}

static inline INLINE
char peek(parse_ctx_t* cx, isz offs) {
	if (cx->it + offs >= cx->data.len)
		return 0;
	return cx->data.str[cx->it + offs];
}

static inline INLINE
char* get_ptr(parse_ctx_t* cx) {
	return &cx->data.str[cx->it];
}

static inline INLINE
void skip_whitespace(parse_ctx_t* cx) {
	while (is_whitespace(peek(cx, 0)))
		consume(cx);
}

#define ALLOC_CHUNK_SIZE 32

static
void conf_add_child(conf_t* node, conf_t* child) {
	if (!node->children || node->child_count % ALLOC_CHUNK_SIZE == 0) {
		usz new_count = (node->child_count + ALLOC_CHUNK_SIZE);
		usz sz = new_count * sizeof(conf_t);
		node->children = realloc(node->children, sz);
		if (!node->children)
			goto out_of_memory;

		if (node->stype == CONF_OBJECT) {
			usz sz = new_count * sizeof(lstr_t);
			node->child_names = realloc(node->child_names, sz);
			if (!node->child_names)
				goto out_of_memory;
		}
	}

    node->children[node->child_count++] = *child;
	return;

out_of_memory:
	ferrf("Config: Memory allocation failed: %s\n", os_err_str());
	return;
}

static conf_t conf_parse_node(parse_ctx_t* cx);

static
conf_t conf_parse_object(parse_ctx_t* cx) {
	conf_t obj = conf_make_node(CONF_OBJECT);

	consume_char(cx, '{');
	while (peek(cx, 0) != '}') {
		skip_whitespace(cx);
		lstr_t name = LSTR(get_ptr(cx), 0);
		if (!is_ident_head(peek(cx, 0)))
			ferrf("Config: '%c' is not a valid identifier\n", peek(cx, 0)); // Error
		while (is_ident_body(peek(cx, 0))) {
			consume(cx);
			++name.len;
		}
		consume_char(cx, ':');
		conf_t new_node = conf_parse_node(cx);

		conf_add_child(&obj, &new_node);
		obj.child_names[obj.child_count - 1] = name;

		skip_whitespace(cx);
		if (conf_is_primitive(new_node.stype)) {
			consume_char(cx, ';');
			skip_whitespace(cx);
		}
	}
	consume(cx);

	return obj;
}

static
conf_t conf_parse_numeric(parse_ctx_t* cx) {
	i64 val = 0;
	while (is_digit(peek(cx, 0))) {
		char c = consume(cx);
		val *= 10;
		val += c - '0';
		if (peek(cx, 0) == '.') {
			consume(cx);

			i64 val_low = 0;
			i64 decimal_mult = 1;

			while (is_digit(peek(cx, 0))) {
				decimal_mult *= 10;

				char c = consume(cx);
				val_low *= 10;
				val_low += c - '0';
			}

			consume_char(cx, 'f');
			conf_t node = conf_make_node(CONF_FLOAT);
			node.float_val = (f64)val + ((f64)val_low / (f64)decimal_mult);
			return node;
		}
	}

	if (peek(cx, 0) == 'f') {
		consume(cx);
		conf_t node = conf_make_node(CONF_FLOAT);
		node.float_val = (f64)val;
		return node;
	}

	conf_t node = conf_make_node(CONF_INT);
	node.int_val = val;
	return node;
}

static
conf_t conf_parse_node(parse_ctx_t* cx) {
	skip_whitespace(cx);

	switch (peek(cx, 0)) {
	case '"': { // String
		consume(cx);
		lstr_t str = LSTR(get_ptr(cx), 0);
		while (peek(cx, 0) != '"') {
			consume(cx);
			++str.len;
		}
		consume(cx);

		conf_t node = conf_make_node(CONF_STRING);
		node.str_val = str;
		return node;
	}	break;

	case '\'': { // Char
		consume(cx);
		lstr_t str = LSTR(get_ptr(cx), 0);
		while (peek(cx, 0) != '\'') {
			consume(cx);
			++str.len;
		}
		consume(cx);

		conf_t node = conf_make_node(CONF_CHAR);
		node.char_val = 'F';
		return node;
	}	break;

	case '{': { // Object
		return conf_parse_object(cx);
	}	break;

	case '[': { // Array
		consume(cx);
		conf_t arr = conf_make_node(CONF_ARRAY);
		while (peek(cx, 0) != ']') {
			conf_t new_node = conf_parse_node(cx);
			skip_whitespace(cx);
			if (peek(cx, 0) != ']')
				consume_char(cx, ',');
			conf_add_child(&arr, &new_node);
			skip_whitespace(cx);
		}
		consume(cx);
		return arr;
	}	break;

	case 't': { // Bool true
		consume(cx);
		if (consume(cx) != 'r') goto bool_err;
		if (consume(cx) != 'u') goto bool_err;
		if (consume(cx) != 'e') goto bool_err;
		conf_t node = conf_make_node(CONF_BOOL);
		node.bool_val = 1;
		return node;
	}	break;

	case 'f': // Bool false
		consume(cx);
		if (consume(cx) != 'a') goto bool_err;
		if (consume(cx) != 'l') goto bool_err;
		if (consume(cx) != 's') goto bool_err;
		if (consume(cx) != 'e') goto bool_err;
		conf_t node = conf_make_node(CONF_BOOL);
		node.bool_val = 0;
		return node;
		break;

	bool_err:
		ferr("Config: Unknown identifier\n");
		return conf_make_node(CONF_INT);
		break;

	default: {
		if (is_digit(peek(cx, 0)))// Number
			return conf_parse_numeric(cx);
		ferrf("Config: Unexpected character '%c'\n", peek(cx, 0));
		return conf_make_node(CONF_INT);
	}	break;
	}
	ASSERT_NOT_REACHED();
}

conf_t conf_parse(lstr_t data) {
	parse_ctx_t ctx;
	ctx.data = data;
	ctx.it = 0;

	parse_ctx_t* cx = &ctx;

	conf_t obj = conf_make_node(CONF_OBJECT);

	while (peek(cx, 0)) {
		skip_whitespace(cx);
		lstr_t name = LSTR(get_ptr(cx), 0);
		if (!is_ident_head(peek(cx, 0)))
			ferrf("Config: '%c' is not a valid identifier\n", peek(cx, 0)); // Error
		while (is_ident_body(peek(cx, 0))) {
			consume(cx);
			++name.len;
		}
		consume_char(cx, ':');
		conf_t new_node = conf_parse_node(cx);

		conf_add_child(&obj, &new_node);
		obj.child_names[obj.child_count - 1] = name;

		skip_whitespace(cx);
		if (conf_is_primitive(new_node.stype)) {
			consume_char(cx, ';');
			skip_whitespace(cx);
		}
	}

	return obj;
}

static
void conf_fwrite_with_indent(FILE* fp, conf_t node, usz indent);

static inline INLINE
void conf_write_indent(FILE* fp, usz indent) {
	for (usz i = 0; i < indent; ++i)
		fputc('\t', fp);
}

static
void conf_fwrite_obj_children(FILE* fp, conf_t node, usz indent) {
	for (usz i = 0; i < node.child_count; ++i) {
		lstr_t name = node.child_names[i];

		conf_write_indent(fp, indent);
		fprintf(fp, "%.*s: ", (int)name.len, name.str);

		conf_fwrite_with_indent(fp, node.children[i], indent);
		if (conf_is_primitive(node.children[i].stype))
			fputc(';', fp);
		fputc('\n', fp);
	}
}

static
void conf_fwrite_with_indent(FILE* fp, conf_t node, usz indent) {
	switch (node.stype) {
	case CONF_BOOL:
		fputs(node.bool_val ? "true" : "false", fp);
		break;

	case CONF_OBJECT:
		fputs("{\n", fp);
		conf_fwrite_obj_children(fp, node, indent + 1);
		conf_write_indent(fp, indent);
		fputs("}", fp);
		break;

	case CONF_ARRAY:
		fputs("[\n", fp);
		for (usz i = 0; i < node.child_count; ++i) {
			conf_write_indent(fp, indent + 1);
			conf_fwrite_with_indent(fp, node.children[i], indent + 1);
			fputs(",\n", fp);
		}
		conf_write_indent(fp, indent);
		fputs("]", fp);
		break;

	case CONF_CHAR:
		fprintf(fp, "'%c'", node.char_val);
		break;

	case CONF_STRING:
		fprintf(fp, "\"%.*s\"", (int)node.str_val.len, node.str_val.str);
		break;

	case CONF_INT:
		fprintf(fp, "%lu", node.int_val);
		break;

	case CONF_FLOAT:
		fprintf(fp, "%ff", node.float_val);
		break;
	}
}

void conf_fwrite(FILE* fp, conf_t node) {
	conf_fwrite_obj_children(fp, node, 0);
	fputc('\n', fp);
}

conf_t* conf_find(conf_t* obj, lstr_t name, conf_stype_t stype) {
	for (usz i = 0; i < obj->child_count; ++i) {
		if (obj->child_names[i].len != name.len)
			continue;
		if (obj->children[i].stype != stype)
			continue;
		if (memcmp(obj->child_names[i].str, name.str, name.len) != 0)
			continue;

		return &obj->children[i];
	}

	return NULL;
}

void conf_free(conf_t* node) {
	switch (node->stype) {
		case CONF_OBJECT:
			free(node->child_names);
		case CONF_ARRAY:
			for (usz i = 0; i < node->child_count; ++i)
				conf_free(&node->children[i]);
			free(node->children);
			break;

		default:
			break;
	}
}
