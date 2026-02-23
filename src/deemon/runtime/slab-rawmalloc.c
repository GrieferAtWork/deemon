/* Copyright (c) 2018-2026 Griefer@Work                                       *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement (see the following) in the product     *
 *    documentation is required:                                              *
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_SLAB_RAWMALLOC_C
#define GUARD_DEEMON_RUNTIME_SLAB_RAWMALLOC_C 1

#include <deemon/api.h>

#if defined(CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR) || defined(__DEEMON__)
#include <deemon/alloc.h>            /* Dee_Free, Dee_Memalign, Dee_TryMemalign, Dee_UntrackAlloc */
#include <deemon/system-features.h>  /* Dee_SLAB_PAGESIZE, Dee_slab_page */
#include <deemon/util/slab-config.h> /* Dee_SLAB_CHUNKSIZE_MAX */
#include <deemon/util/slab.h>        /* Dee_SLAB_PAGESIZE, Dee_slab_page */

#include <hybrid/debug-alignment.h> /* Dee_SLAB_PAGESIZE, Dee_slab_page */
#include <hybrid/host.h>            /* Dee_SLAB_PAGESIZE, Dee_slab_page */
#include <hybrid/sequence/rbtree.h> /* Dee_SLAB_PAGESIZE, Dee_slab_page */

#include <stddef.h> /* size_t */

#if defined(Dee_SLAB_CHUNKSIZE_MAX) || defined(__DEEMON__)

#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#endif /* CONFIG_HOST_WINDOWS */

#if !defined(CONFIG_HAVE_mmap) && defined(CONFIG_HAVE_mmap64)
#define CONFIG_HAVE_mmap 1
#undef mmap
#define mmap mmap64
#endif /* mmap = mmap64 */

#ifndef CONFIG_HAVE_PROT_READ
#undef PROT_READ
#define PROT_READ 0
#endif /* !CONFIG_HAVE_PROT_READ */
#ifndef CONFIG_HAVE_PROT_WRITE
#undef PROT_WRITE
#define PROT_WRITE 0
#endif /* !CONFIG_HAVE_PROT_WRITE */
#ifndef CONFIG_HAVE_MAP_PRIVATE
#undef MAP_PRIVATE
#define MAP_PRIVATE 0
#endif /* !CONFIG_HAVE_MAP_PRIVATE */

#ifndef __ARCH_PAGESIZE_MIN
#ifdef __ARCH_PAGESIZE
#define __ARCH_PAGESIZE_MIN __ARCH_PAGESIZE /*!export-*/
#endif /* __ARCH_PAGESIZE */
#endif /* !__ARCH_PAGESIZE_MIN */

#ifdef __ARCH_PAGESIZE
#define malloc_getpagesize __ARCH_PAGESIZE
#define MALLOC_PAGESIZE    __ARCH_PAGESIZE
#elif defined(CONFIG_HAVE_getpagesize)
#define malloc_getpagesize getpagesize()
#else /* __ARCH_PAGESIZE */
#if !defined(_SC_PAGE_SIZE) && defined(_SC_PAGESIZE)
#define _SC_PAGE_SIZE _SC_PAGESIZE
#endif /* !_SC_PAGE_SIZE && _SC_PAGESIZE */
#if defined(_SC_PAGE_SIZE) && defined(CONFIG_HAVE_sysconf)
#define malloc_getpagesize sysconf(_SC_PAGE_SIZE)
#else /* _SC_PAGE_SIZE && CONFIG_HAVE_sysconf */
#ifdef CONFIG_HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif /* CONFIG_HAVE_SYS_PARAM_H */
#ifdef EXEC_PAGESIZE
#define malloc_getpagesize EXEC_PAGESIZE
#elif defined(NBPG)
#ifdef CLSIZE
#define malloc_getpagesize (NBPG * CLSIZE)
#else /* CLSIZE */
#define malloc_getpagesize NBPG
#endif /* !CLSIZE */
#elif defined(NBPC)
#define malloc_getpagesize NBPC
#elif defined(PAGESIZE)
#define malloc_getpagesize PAGESIZE
#else /* EXEC_PAGESIZE */
#define malloc_getpagesize ((size_t)4096U) /* just guess */
#endif /* !EXEC_PAGESIZE */
#endif /* !_SC_PAGE_SIZE || !CONFIG_HAVE_sysconf */
#endif /* !__ARCH_PAGESIZE */

#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1l)
#endif /* !MAP_FAILED */


DECL_BEGIN


#if 0 /* TODO */
#undef slab_pages_malloc_USE_VirtualAlloc
#undef slab_pages_malloc_USE_mmap
#undef slab_pages_malloc_USE_Dee_Memalign
#undef slab_pages_malloc_USE_Dee_Memalign_MAYBE
#undef slab_pages_malloc_CAN_COALESCE /* randomly adjacent segments can be coalesced (else: must remember base address of every segment for `Dee_slab_page_rawfree_impl()') */
#if defined(__ARCH_PAGESIZE_MIN) && (Dee_SLAB_PAGESIZE > __ARCH_PAGESIZE_MIN)
#define slab_pages_malloc_USE_Dee_Memalign
#elif defined(CONFIG_HOST_WINDOWS)
#define slab_pages_malloc_USE_VirtualAlloc
#ifndef __ARCH_PAGESIZE_MIN
#define slab_pages_malloc_USE_Dee_Memalign_MAYBE
#endif /* !__ARCH_PAGESIZE_MIN */
#elif defined(CONFIG_HAVE_mmap) && defined(CONFIG_HAVE_MAP_ANONYMOUS)
#define slab_pages_malloc_USE_mmap
#define slab_pages_malloc_CAN_COALESCE
#ifndef __ARCH_PAGESIZE_MIN
#define slab_pages_malloc_USE_Dee_Memalign_MAYBE
#undef slab_pages_malloc_CAN_COALESCE
#endif /* !__ARCH_PAGESIZE_MIN */
#else /* ... */
#define slab_pages_malloc_USE_Dee_Memalign
#endif /* !... */


#ifdef slab_pages_malloc_USE_Dee_Memalign_MAYBE
PRIVATE size_t system_pagesize = 0;
#endif /* slab_pages_malloc_USE_Dee_Memalign_MAYBE */

PRIVATE void *DCALL slab_pages_malloc(size_t size) {
#ifdef slab_pages_malloc_USE_Dee_Memalign_MAYBE
	if (system_pagesize == 0)
		system_pagesize = slab_pages_malloc_USE_Dee_Memalign_MAYBE;
	if (system_pagesize < Dee_SLAB_PAGESIZE)
		return Dee_UntrackAlloc((Dee_Memalign)(Dee_SLAB_PAGESIZE, size));
#endif /* slab_pages_malloc_USE_Dee_Memalign_MAYBE */

#ifdef slab_pages_malloc_USE_VirtualAlloc
	{
		void *ptr;
		DBG_ALIGNMENT_DISABLE();
		ptr = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		DBG_ALIGNMENT_ENABLE();
		return ptr;
	}
#endif /* slab_pages_malloc_USE_VirtualAlloc */

#ifdef slab_pages_malloc_USE_mmap
	{
		void *ptr;
		DBG_ALIGNMENT_DISABLE();
		ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		DBG_ALIGNMENT_ENABLE();
		return ptr != (void *)MAP_FAILED ? ptr : NULL;
	}
#endif /* slab_pages_malloc_USE_mmap */

#ifdef slab_pages_malloc_USE_Dee_Memalign
	return Dee_UntrackAlloc((Dee_Memalign)(Dee_SLAB_PAGESIZE, size));
#endif /* slab_pages_malloc_USE_Dee_Memalign */
}

PRIVATE void DCALL slab_pages_free(void *pages, size_t size) {
	(void)pages;
	(void)size;
#ifdef slab_pages_malloc_USE_Dee_Memalign_MAYBE
	if (system_pagesize < Dee_SLAB_PAGESIZE) {
		(Dee_Free)(pages);
		return;
	}
#endif /* slab_pages_malloc_USE_Dee_Memalign_MAYBE */

#ifdef slab_pages_malloc_USE_VirtualAlloc
	DBG_ALIGNMENT_DISABLE();
	(void)VirtualFree(pages, 0, MEM_RELEASE);
	DBG_ALIGNMENT_ENABLE();
#endif /* slab_pages_malloc_USE_VirtualAlloc */

#ifdef slab_pages_malloc_USE_mmap
#ifdef CONFIG_HAVE_munmap
	(void)munmap(pages, size);
#endif /* CONFIG_HAVE_munmap */
#endif /* slab_pages_malloc_USE_mmap */

#ifdef slab_pages_malloc_USE_Dee_Memalign
	(Dee_Free)(pages);
#endif /* slab_pages_malloc_USE_Dee_Memalign */
}



#ifndef slab_pages_malloc_CAN_COALESCE
struct slab_page_segment {
	void                          *sps_base; /* [1..1][lock(slab_segments_lock)] Base address of segment */
	size_t                         sps_size; /* [lock(slab_segments_lock)] # of bytes in this segment of slab pages */
	RBTREE_NODE(slab_page_segment) sps_node; /* [lock(slab_segments_lock)] Node in tree of segments */
};
#endif /* !slab_pages_malloc_CAN_COALESCE */

/* TODO */
#endif


/* Raw alloc/free functions for slab pages. These form the back-bone of
 * the actual slab page allocator and are used whenever a special-purpose
 * slab allocator...
 * - ... requires a new slab page because there are no more pages with free chunk
 * - ... determines that the last allocated chunk of some slab page has been free'd
 *       (and the page doesn't have a custom free function), so it wants to give
 *       the page back to the underlying page allocator.
 *
 * WARNING: The memory of the pages returned these malloc functions is entirely
 *          uninitialized (similarly, `Dee_slab_page_rawfree()' does not care
 *          about the contents of memory within the pages it's given). As such,
 *          the caller is responsible for doing all initialization/finalization
 * WARNING: Do not pass custom slab pages to `Dee_slab_page_rawfree()'! These
 *          are just the dumb, low-level page allocation functions. If you want
 *          to support custom free function, that's up to you! */
PUBLIC ATTR_MALLOC WUNUSED ATTR_ASSUME_ALIGNED(Dee_SLAB_PAGESIZE)
struct Dee_slab_page *DCALL Dee_slab_page_rawmalloc(void) {
	/* TODO: OS-specific implementations for (so-long as "Dee_SLAB_PAGESIZE <= __ARCH_PAGESIZE"):
	 * - VirtualAlloc()
	 * - mmap()
	 *
	 * These impls should then also allocate more than a single slab
	 * page at-a-time, and keep a global cache of free'd / unused slab
	 * pages so-as to be able to more quickly hand out pages that were
	 * already allocated in previous passes.
	 *
	 * This cache system should also be cleared by `DeeHeap_Trim()' */
	Dee_DPRINT("Dee_slab_page_rawmalloc()\n");
	return (struct Dee_slab_page *)Dee_UntrackAlloc(Dee_Memalign(Dee_SLAB_PAGESIZE, Dee_SLAB_PAGESIZE));
}

PUBLIC ATTR_MALLOC WUNUSED ATTR_ASSUME_ALIGNED(Dee_SLAB_PAGESIZE)
struct Dee_slab_page *DCALL Dee_slab_page_tryrawmalloc(void) {
	Dee_DPRINT("Dee_slab_page_tryrawmalloc()\n");
	return (struct Dee_slab_page *)Dee_UntrackAlloc(Dee_TryMemalign(Dee_SLAB_PAGESIZE, Dee_SLAB_PAGESIZE));
}

PUBLIC NONNULL((1)) void DCALL
Dee_slab_page_rawfree(struct Dee_slab_page *__restrict page) {
	Dee_DPRINT("Dee_slab_page_rawfree()\n");
	Dee_Free(page);
}



/* Clear caches kept by the raw slab page allocator.
 * This function is automatically called by `DeeHeap_Trim()'
 * @param: pad: Try to keep at least this many bytes within the cache
 * @return: * : The # of bytes free'd from the cache. */
INTERN size_t DCALL Dee_slab_page_rawtrim(size_t pad) {
	/* TODO */
	(void)pad;
	return 0;
}


DECL_END
#endif /* Dee_SLAB_CHUNKSIZE_MAX */

#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

#endif /* !GUARD_DEEMON_RUNTIME_SLAB_RAWMALLOC_C */
