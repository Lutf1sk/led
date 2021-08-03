// Copyright (C) 2021, Alex Edin <lutfisk@lutfisk.net>
// SPDX-License-Identifier: GPL-2.0+

#include "allocators.h"
#include "align.h"

#ifdef PLATFORM_LINUX
#	include <unistd.h>
#	include <sys/mman.h>

#	define MAP_PAGES(size) mmap(NULL, (size), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
#	define UNMAP_PAGES(addr, size) munmap((addr), (size))
#	define MAP_PAGES_ERROR MAP_FAILED
#elif defined(PLATFORM_WINDOWS)
#	include <windows.h>
#	include <memoryapi.h>

#	define MAP_PAGES(size) VirtualAlloc(NULL, (size), MEM_RESERVE, PAGE_READWRITE)
#	define UNMAP_PAGES(addr, size) VirtualFree((addr), 0, MEM_RELEASE)
#	define MAP_PAGES_ERROR NULL
#endif

usz page_size = 0;
usz aframe_header_size = (usz)-1;
usz pframe_header_size = (usz)-1;

void allocators_initialize(void) {
	// Get page size
#ifdef PLATFORM_LINUX
	page_size = sysconf(_SC_PAGE_SIZE);
#elif defined(PLATFORM_WINDOWS)
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	page_size = sys_info.dwPageSize;
#endif

	// Store padded arena header size
	aframe_header_size = align_fwd(sizeof(aframe_t), 2 * sizeof(usz));
	pframe_header_size = align_fwd(sizeof(pframe_t), 2 * sizeof(usz));
}

void allocators_terminate(void) {

}

void* alloc_pages(usz size) {
	void* pages = MAP_PAGES(size);
	if (pages == MAP_PAGES_ERROR)
		return NULL;
	return pages;
}

void free_pages(void* addr, usz size) {
	UNMAP_PAGES(addr, size);
}
