// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#ifndef ALLOCATORS_H
#define ALLOCATORS_H

#include "common.h"

// Common
void allocators_initialize(void);
void allocators_terminate(void);

void* alloc_pages(usz size);
void free_pages(void* addr, usz size);

extern usz aframe_header_size;
extern usz pframe_header_size;
extern usz page_size;

// Arena allocator
typedef
struct aframe {
	void* mem_start;
	void* mem_pos;
	usz free_bytes;
	usz size;
} aframe_t;

static inline INLINE
aframe_t aframe_make(void* ptr, usz size) {
	aframe_t frame;
	frame.mem_start = ptr;
	frame.mem_pos = ptr;
	frame.free_bytes = size;
	frame.size = size;
	return frame;
}

aframe_t* aframe_alloc(usz size);
void aframe_free(aframe_t* arena);
void* aframe_reserve(aframe_t* arena, usz size);

typedef
struct arestore {
	void* mem_pos;
	usz free_bytes;
} arestore_t;

static inline INLINE
arestore_t arestore_make(aframe_t* arena) {
	arestore_t restore_point;
	restore_point.mem_pos = arena->mem_pos;
	restore_point.free_bytes = arena->free_bytes;
	return restore_point;
}
void aframe_restore(aframe_t* arena, arestore_t* restore_point);

// Pool allocator
typedef struct pnode pnode_t;

typedef
struct pframe {
	void* mem;
	usz size;
	usz chunk_size;
	pnode_t* head;
	struct pframe* next;
} pframe_t;

static inline INLINE
pframe_t pframe_make(void* mem, usz aligned_size, usz chunk_size) {
	pframe_t frame;
	frame.mem = mem;
	frame.size = aligned_size;
	frame.chunk_size = chunk_size;
	frame.head = NULL;
	frame.next = NULL;
	return frame;
}

typedef
struct pnode {
	struct pnode* next;
} pnode_t;

static inline INLINE
pnode_t pnode_make(pnode_t* next) {
	return (pnode_t){next};
}

void pframe_free_all(pframe_t* pool);

pframe_t* pframe_alloc(usz chunk_size, usz count);
void pframe_free(pframe_t* pool);

void* pframe_reserve(pframe_t* pool);
void pframe_relinq(pframe_t* pool, void* chunk);

#endif
