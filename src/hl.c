#include "highlight.h"
#include "doc.h"

#include <lt/str.h>
#include <lt/mem.h>
#include <lt/ctype.h>
#include <lt/darr.h>

static
highl_t** hl_generate_git_commit(doc_t* doc, lt_alloc_t* alloc) {
	highl_t** lines = lt_malloc(alloc, doc->line_count * sizeof(highl_t*));
	for (usz i = 0; i < doc->line_count; ++i) {
		highl_t** node = &lines[i];
		lstr_t line = doc->lines[i];

		char* it = line.str, *end = line.str + line.len;
		char* wrap_point = it + 72;

		while (it < end) {
			char* start = it;
			u8 mode = HLM_UNKNOWN;
			if (!i)
				mode = HLM_KEYWORD;

			if (it >= wrap_point) {
				mode = HLM_PUNCTUATION;
				while (it < end)
					++it;
			}
			else if (*it == '#') {
				mode = HLM_COMMENT;
				it = end;
			}
			else if (lt_is_space(*it)) {
				mode = HLM_INDENT;
				while (it < end && lt_is_space(*it))
					++it;
			}
			else {
				while (it < end && it < wrap_point && *it != '#' && !lt_is_space(*it))
					++it;
			}

			highl_t* new = lt_malloc(alloc, sizeof(highl_t));
			new->len = it - start;
			new->mode = mode;

			*node = new;
			node = &new->next;
		}
		*node = NULL;
	}
	return lines;
}

typedef
struct modeext {
	lstr_t extension;
	hl_mode_t mode;
} modeext_t;

static lt_darr(modeext_t) extension_modes = NULL;

struct {
	lstr_t str;
	hl_mode_t mode;
} mode_strs[] = {
	{ CLSTR("c"),			HL_C },
	{ CLSTR("c#"),			HL_CS },
	{ CLSTR("onyx"),		HL_ONYX },
	{ CLSTR("rust"),		HL_RUST },
	{ CLSTR("javascript"),	HL_JS },
	{ CLSTR("git_commit"),	HL_GIT_COMMIT },
};

hl_mode_t hl_find_mode_by_name(lstr_t name) {
	for (usz i = 0; i < sizeof(mode_strs) / sizeof(*mode_strs); ++i) {
		if (lt_lstr_eq(name, mode_strs[i].str)) {
			return mode_strs[i].mode;
		}
	}
	return HL_UNKNOWN;
}

hl_mode_t hl_find_mode_by_extension(lstr_t path) {
	if (!extension_modes)
		return HL_UNKNOWN;

	for (usz i = 0; i < lt_darr_count(extension_modes); ++i) {
		if (lt_lstr_endswith(path, extension_modes[i].extension)) {
			return extension_modes[i].mode;
		}
	}
	return HL_UNKNOWN;
}

void hl_register_extension(lstr_t extension, lstr_t mode_str) {
	hl_mode_t mode = hl_find_mode_by_name(mode_str);
	if (mode == HL_UNKNOWN) {
		lt_werrf("unknown highlighting mode '%S'\n", mode_str);
		return;
	}

	if (!extension_modes) {
		extension_modes = lt_darr_create(modeext_t, 128, lt_libc_heap);
		LT_ASSERT(extension_modes);
	}

	lt_darr_push(extension_modes, (modeext_t){ lt_strdup(lt_libc_heap, extension), mode });
}

highl_t** hl_generate(doc_t* doc, hl_mode_t mode, lt_alloc_t* alloc) {
	switch (mode) {
	case HL_C: return hl_generate_c(doc, alloc);
	case HL_CS: return hl_generate_cs(doc, alloc);
	case HL_ONYX: return hl_generate_onyx(doc, alloc);
	case HL_RUST: return hl_generate_rust(doc, alloc);
	case HL_JS: return hl_generate_js(doc, alloc);
	case HL_GIT_COMMIT: return hl_generate_git_commit(doc, alloc);
	case HL_UNKNOWN: return NULL;
	default: LT_ASSERT_NOT_REACHED(); return NULL;
	}
}

