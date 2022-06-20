// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef CONF_H
#define CONF_H 1

#include <stdio.h>

#include "common.h"

typedef
enum conf_stype {
	CONF_ARRAY,
	CONF_OBJECT,
	CONF_FLOAT,
	CONF_INT,
	CONF_STRING,
	CONF_CHAR,
	CONF_BOOL,
} conf_stype_t;

typedef
struct conf {
	conf_stype_t stype;
	usz child_count;
	struct conf* children;

	union {
		lstr_t* child_names;
		double float_val;
		i64 int_val;
		lstr_t str_val;
		char char_val;
		b8 bool_val;
	};
} conf_t;

static inline INLINE
conf_t conf_make_node(conf_stype_t stype) {
	conf_t node;
	node.child_count = 0;
	node.children = 0;
	node.child_names = 0;
	node.stype = stype;
	return node;
}

static inline INLINE
b8 conf_is_primitive(conf_stype_t stype) {
	return stype != CONF_ARRAY && stype != CONF_OBJECT;
}

conf_t conf_parse(lstr_t data);
void conf_fwrite(FILE* fp, conf_t node);

conf_t* conf_find(conf_t* obj, lstr_t key, conf_stype_t stype);

static inline INLINE
lstr_t conf_find_str_default(conf_t* obj, lstr_t key, lstr_t default_) {
	conf_t* node = conf_find(obj, key, CONF_STRING);
	return node ? node->str_val : default_;
}

static inline INLINE
b8 conf_find_bool_default(conf_t* obj, lstr_t key, b8 default_) {
	conf_t* node = conf_find(obj, key, CONF_BOOL);
	return node ? node->bool_val : default_;
}

static inline INLINE
i64 conf_find_int_default(conf_t* obj, lstr_t key, i64 default_) {
    conf_t* node = conf_find(obj, key, CONF_INT);
    return node ? node->int_val : default_;
}

void conf_free(conf_t* node);

#endif
