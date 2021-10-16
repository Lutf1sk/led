// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "allocators.h"
#include "align.h"

static inline INLINE
usz max(usz v1, usz v2) {
	return v1 > v2 ? v1 : v2;
}

void pframe_free_all(pframe_t* pool) {
	usz chunk_size = pool->chunk_size;
	u8* it_end = pool->mem + pool->size;
	pnode_t* last_node = NULL;
	for (u8* it = pool->mem; it + chunk_size < it_end; it += chunk_size) {
		pnode_t* node = (pnode_t*)it;
		node->next = last_node;
		last_node = node;
	}
	pool->head = last_node;

	if (pool->next)
		pframe_free_all(pool->next);
}

pframe_t* pframe_alloc(usz chunk_size, usz count) {
	if (!count)
		ferr("Invalid pool chunk count\n");

	usz header_size = pframe_header_size;

	usz size = chunk_size * count;
	size += pad(size + header_size, page_size);

	void* mem = alloc_pages(size + header_size);
	if (!mem)
		ferrf("Failed to allocate pool memory: %s\n", os_err_str());

	chunk_size = word_align_fwd(max(sizeof(pnode_t), chunk_size));

	pframe_t* header = mem;
	*header = pframe_make((u8*)mem + header_size, size, chunk_size);
	pframe_free_all(header);

	return header;
}

void pframe_free(pframe_t* pool) {
	if (pool->next)
		pframe_free(pool->next);
	free_pages(pool, pool->size + pframe_header_size);
}

void* pframe_reserve(pframe_t* pool) {
	if (!pool->head) {
		if (!pool->next)
			pool->next = pframe_alloc(pool->chunk_size, pool->size / pool->chunk_size);
		return pframe_reserve(pool->next);
	}

	pnode_t* chunk = pool->head;
	pool->head = chunk->next;
	return chunk;
}

void pframe_relinq(pframe_t* pool, void* chunk) {
	if (chunk >= pool->mem && chunk < (void*)((u8*)pool->mem + pool->size)) {
		pnode_t* node = chunk;
		node->next = pool->head;
		pool->head = node;
		return;
	}

	if (!pool->next)
		ferr("Attempt to relinquish invalid pool chunk\n");
	pframe_relinq(pool->next, chunk);
}
