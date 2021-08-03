// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "allocators.h"
#include "align.h"

aframe_t* aframe_alloc(usz size) {
	if (!size)
		ferrf("Invalid arena size '%zu'\n", size);

	usz header_size = aframe_header_size;

	size += pad(size + header_size, page_size);

	void* mem = alloc_pages(size + header_size);
	if (!mem)
		ferrf("Failed to allocate arena memory: %s\n", os_err_str());

	*(aframe_t*)mem = aframe_make((char*)mem + header_size, size);
	return mem;
}

void aframe_free(aframe_t* frame) {
	free_pages(frame, frame->size + aframe_header_size);
}

void* aframe_reserve(aframe_t* frame, usz size) {
	usz aligned_size = align_fwd(size, sizeof(usz));

	if (aligned_size > frame->free_bytes)
		return NULL;

	void* ptr = frame->mem_pos;
	frame->free_bytes -= aligned_size;
	frame->mem_pos = (char*)frame->mem_pos + aligned_size;
	return ptr;
}

void aframe_restore(aframe_t* arena, arestore_t* restore_point) {
	arena->mem_pos = restore_point->mem_pos;
	arena->free_bytes = restore_point->free_bytes;
}
