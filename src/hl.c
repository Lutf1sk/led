#include "highlight.h"
#include "doc.h"

#include <lt/str.h>
#include <lt/mem.h>
#include <lt/ctype.h>

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

struct {
	lstr_t extension;
	hl_mode_t mode;
} modes[] = {
	{ CLSTR(".c"),		HL_C },
	{ CLSTR(".cpp"),	HL_C },
	{ CLSTR(".h"),		HL_C },
	{ CLSTR("COMMIT_EDITMSG"), HL_GIT_COMMIT },
};

hl_mode_t hl_find_mode(lstr_t path) {
	for (usz i = 0; i < sizeof(modes) / sizeof(*modes); ++i)
		if (lt_lstr_endswith(path, modes[i].extension))
			return modes[i].mode;
	return HL_UNKNOWN;
}

highl_t** hl_generate(doc_t* doc, hl_mode_t mode, lt_alloc_t* alloc) {
	switch (mode) {
	case HL_C: return hl_generate_c(doc, alloc);
	case HL_GIT_COMMIT: return hl_generate_git_commit(doc, alloc);
	case HL_UNKNOWN: return NULL;
	default: LT_ASSERT_NOT_REACHED(); return NULL;
	}
}

