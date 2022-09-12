#include "highlight.h"

#include <lt/str.h>

struct {
	lstr_t extension;
	highl_mode_t mode;
} modes[] = {
	{ CLSTR(".c"),		HL_C },
	{ CLSTR(".cpp"),	HL_C },
	{ CLSTR(".h"),		HL_C },
	{ CLSTR("COMMIT_EDITMSG"), HL_GIT_COMMIT },
};

highl_mode_t hl_find_mode(lstr_t path) {
	for (usz i = 0; i < sizeof(modes) / sizeof(*modes); ++i)
		if (lt_lstr_endswith(path, modes[i].extension))
			return modes[i].mode;
	return HL_UNKNOWN;
}

highl_t** highl_generate(doc_t* doc, highl_mode_t mode, lt_alloc_t* alloc) {
	switch (mode) {
	case HL_C: return highl_generate_c(doc, alloc);
	case HL_GIT_COMMIT: return NULL;
	case HL_UNKNOWN: return NULL;
	default: LT_ASSERT_NOT_REACHED(); return NULL;
	}
}

