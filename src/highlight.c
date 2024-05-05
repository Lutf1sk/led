#include "highlight.h"
#include "doc.h"

#include <lt/str.h>
#include <lt/mem.h>
#include <lt/ctype.h>
#include <lt/darr.h>
#include <lt/conf.h>

static
highl_t** hl_generate_git_commit(doc_t* doc, lt_arena_t* alloc) {
	usz line_count = lt_texted_line_count(&doc->ed);
	highl_t** lines = lt_amalloc_lean(alloc, line_count * sizeof(highl_t*));

	for (usz i = 0; i < line_count; ++i) {
		highl_t** node = &lines[i];
		lstr_t line = lt_texted_line_str(&doc->ed, i);

		char* it = line.str, *end = line.str + line.len;
		char* wrap_point = it + 72;

		while (it < end) {
			char* start = it;
			u8 mode = (i ? HLM_UNKNOWN : HLM_KEYWORD);

			if (it >= wrap_point) {
				mode = HLM_PUNCTUATION;
				while (it < end) {
					++it;
				}
			}
			else if (*it == '#') {
				mode = HLM_COMMENT;
				it = end;
			}
			else if (lt_is_space(*it)) {
				mode = HLM_INDENT;
				while (it < end && lt_is_space(*it)) {
					++it;
				}
			}
			else {
				while (it < end && it < wrap_point && *it != '#' && !lt_is_space(*it)) {
					++it;
				}
			}

			highl_t* new = lt_amalloc_lean(alloc, sizeof(highl_t));
			new->len = it - start;
			new->mode = mode;

			*node = new;
			node = &new->next;
		}
		*node = NULL;
	}
	return lines;
}

static
highl_t** hl_generate_unknown(doc_t* doc, lt_arena_t* alloc) {
	return NULL;
}

typedef
struct modeext {
	lstr_t extension;
	modeid_t mode;
} modeext_t;

static lt_darr(modeext_t) extension_modes = NULL;

hlmode_t modes[HL_COUNT] = {
#define HLMODE_OP(ename, sname, comment, func) { CLSTR(sname), CLSTR(comment), func },
	FOR_EACH_HLMODE()
#undef HLMODE_OP
};

modeid_t hl_find_mode_by_name(lstr_t name) {
	for (usz i = 0; i < HL_COUNT; ++i) {
		if (lt_lseq_nocase(name, modes[i].name)) {
			return i;
		}
	}
	return HL_UNKNOWN;
}

modeid_t hl_find_mode_by_extension(lstr_t path) {
	if (!extension_modes) {
		return HL_UNKNOWN;
	}

	for (usz i = 0; i < lt_darr_count(extension_modes); ++i) {
		if (lt_lssuffix(path, extension_modes[i].extension)) {
			return extension_modes[i].mode;
		}
	}
	return HL_UNKNOWN;
}

void hl_register_extension(lstr_t extension, lstr_t mode_str) {
	modeid_t mode = hl_find_mode_by_name(mode_str);
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

void hl_load(lt_conf_t* hls) {
	if (!hls)
		return;

	for (usz i = 0; i < hls->child_count; ++i) {
		lt_conf_t* hl = &hls->children[i];
		if (hl->stype != LT_CONF_OBJECT) {
			continue;
		}

		lstr_t ext = NLSTR();
		if (!lt_conf_find_str(hl, CLSTR("extension"), &ext)) {
			lt_werrf("highlight object missing 'extension'\n");
			continue;
		}

		lstr_t modestr = NLSTR();
		if (!lt_conf_find_str(hl, CLSTR("mode"), &modestr)) {
			lt_werrf("highlight object missing 'extension'\n");
			continue;
		}

		hl_register_extension(ext, modestr);
	}
}

