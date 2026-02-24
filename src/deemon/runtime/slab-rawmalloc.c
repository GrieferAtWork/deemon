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
#include <deemon/alloc.h>            /* Dee_Free, Dee_Malloc, Dee_Memalign, Dee_ReleaseSystemMemory, Dee_TryMalloc, Dee_TryMemalign, Dee_TryReleaseSystemMemory, Dee_UntrackAlloc */
#include <deemon/system-features.h>  /* CONFIG_HAVE_*, MAP_ANONYMOUS, getpagesize, mmap64, munmap, sysconf */
#include <deemon/util/atomic.h>      /* atomic_cmpxch, atomic_read */
#include <deemon/util/lock.h>        /* Dee_atomic_lock_* */
#include <deemon/util/slab-config.h> /* Dee_SLAB_CHUNKSIZE_MAX */
#include <deemon/util/slab.h>        /* Dee_SLAB_PAGESIZE, Dee_slab_page */

#include <hybrid/bitset.h>          /* BITSET_SIZEOF, bitset_* */
#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */
#include <hybrid/host.h>            /* __ARCH_PAGESIZE */
#include <hybrid/sequence/list.h>   /* SLIST_*, TAILQ_* */
#include <hybrid/sequence/rbtree.h> /* RBTREE_NODE, RBTREE_ROOT */
#include <hybrid/typecore.h>        /* __BYTE_TYPE__ */

#include <stddef.h> /* offsetof, size_t */

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

#undef container_of
#define container_of COMPILER_CONTAINER_OF
#undef byte_t
#define byte_t __BYTE_TYPE__


#undef USE_psegment
#undef USE_fslab
#if !defined(__OPTIMIZE_SIZE__) && 1
#define USE_psegment /* Allocate slab pages in larger segments */
#define USE_fslab    /* Use s small cache of recently free'd slab pages */
#endif /* !__OPTIMIZE_SIZE__ */

DECL_BEGIN

/* Core SLAB MALLOC */
#undef cslab_malloc_USE_VirtualAlloc
#undef cslab_malloc_USE_mmap
#undef cslab_malloc_USE_Dee_Memalign
#undef cslab_malloc_USE_Dee_Memalign_MAYBE
#undef cslab_malloc_CAN_COALESCE /* randomly adjacent segments can be coalesced (else: must remember base address of every segment for `Dee_slab_page_rawfree_impl()') */
#if 0
#define cslab_malloc_USE_Dee_Memalign
#elif defined(__ARCH_PAGESIZE_MIN) && (Dee_SLAB_PAGESIZE > __ARCH_PAGESIZE_MIN)
#define cslab_malloc_USE_Dee_Memalign
#elif defined(CONFIG_HOST_WINDOWS)
#define cslab_malloc_USE_VirtualAlloc
#ifndef __ARCH_PAGESIZE_MIN
#define cslab_malloc_USE_Dee_Memalign_MAYBE
#endif /* !__ARCH_PAGESIZE_MIN */
#elif defined(CONFIG_HAVE_mmap) && defined(CONFIG_HAVE_MAP_ANONYMOUS)
#define cslab_malloc_USE_mmap
#define cslab_malloc_CAN_COALESCE
#ifndef __ARCH_PAGESIZE_MIN
#define cslab_malloc_USE_Dee_Memalign_MAYBE
#undef cslab_malloc_CAN_COALESCE
#endif /* !__ARCH_PAGESIZE_MIN */
#else /* ... */
#define cslab_malloc_USE_Dee_Memalign
#endif /* !... */


#ifdef cslab_malloc_USE_Dee_Memalign_MAYBE
PRIVATE size_t system_pagesize = 0;
#endif /* cslab_malloc_USE_Dee_Memalign_MAYBE */

PRIVATE void *DCALL cslab_trymalloc(size_t size) {
#ifdef cslab_malloc_USE_Dee_Memalign_MAYBE
	if (system_pagesize == 0)
		system_pagesize = malloc_getpagesize;
	if (system_pagesize < Dee_SLAB_PAGESIZE)
		return Dee_UntrackAlloc((Dee_TryMemalign)(Dee_SLAB_PAGESIZE, size));
#endif /* cslab_malloc_USE_Dee_Memalign_MAYBE */

#ifdef cslab_malloc_USE_VirtualAlloc
	{
		void *ptr;
		DBG_ALIGNMENT_DISABLE();
		ptr = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		DBG_ALIGNMENT_ENABLE();
		if (Dee_TryReleaseSystemMemory()) {
			DBG_ALIGNMENT_DISABLE();
			ptr = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			DBG_ALIGNMENT_ENABLE();
		}
		return ptr;
	}
#endif /* cslab_malloc_USE_VirtualAlloc */

#ifdef cslab_malloc_USE_mmap
	{
		void *ptr;
		DBG_ALIGNMENT_DISABLE();
		ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		DBG_ALIGNMENT_ENABLE();
		if (ptr != (void *)MAP_FAILED)
			return ptr;
		if (Dee_TryReleaseSystemMemory()) {
			DBG_ALIGNMENT_DISABLE();
			ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			DBG_ALIGNMENT_ENABLE();
			if (ptr != (void *)MAP_FAILED)
				return ptr;
		}
		return NULL;
	}
#endif /* cslab_malloc_USE_mmap */

#ifdef cslab_malloc_USE_Dee_Memalign
	return Dee_UntrackAlloc((Dee_TryMemalign)(Dee_SLAB_PAGESIZE, size));
#endif /* cslab_malloc_USE_Dee_Memalign */
}

PRIVATE void *DCALL cslab_malloc(size_t size) {
#ifdef cslab_malloc_USE_Dee_Memalign_MAYBE
	if (system_pagesize == 0)
		system_pagesize = malloc_getpagesize;
	if (system_pagesize < Dee_SLAB_PAGESIZE)
		return Dee_UntrackAlloc((Dee_Memalign)(Dee_SLAB_PAGESIZE, size));
#endif /* cslab_malloc_USE_Dee_Memalign_MAYBE */
#ifdef cslab_malloc_USE_Dee_Memalign
	return Dee_UntrackAlloc((Dee_Memalign)(Dee_SLAB_PAGESIZE, size));
#else /* cslab_malloc_USE_Dee_Memalign */
	for (;;) {
		void *result = cslab_trymalloc(size);
		if (result != NULL)
			return result;
		if (!Dee_ReleaseSystemMemory())
			return NULL;
	}
#endif /* !cslab_malloc_USE_Dee_Memalign */
}

PRIVATE void DCALL cslab_free(void *pages, size_t size) {
	(void)pages;
	(void)size;
#ifdef cslab_malloc_USE_Dee_Memalign_MAYBE
	if (system_pagesize < Dee_SLAB_PAGESIZE) {
		(Dee_Free)(pages);
		return;
	}
#endif /* cslab_malloc_USE_Dee_Memalign_MAYBE */

#ifdef cslab_malloc_USE_VirtualAlloc
	DBG_ALIGNMENT_DISABLE();
	(void)VirtualFree(pages, 0, MEM_RELEASE);
	DBG_ALIGNMENT_ENABLE();
#endif /* cslab_malloc_USE_VirtualAlloc */

#ifdef cslab_malloc_USE_mmap
#ifdef CONFIG_HAVE_munmap
	(void)munmap(pages, size);
#endif /* CONFIG_HAVE_munmap */
#endif /* cslab_malloc_USE_mmap */

#ifdef cslab_malloc_USE_Dee_Memalign
	(Dee_Free)(pages);
#endif /* cslab_malloc_USE_Dee_Memalign */
}




#ifndef USE_psegment
#define base_trymalloc() ((struct Dee_slab_page *)cslab_trymalloc(Dee_SLAB_PAGESIZE))
#define base_malloc()    ((struct Dee_slab_page *)cslab_malloc(Dee_SLAB_PAGESIZE))
#define base_free(page)  cslab_free(page, Dee_SLAB_PAGESIZE)
#else /* !USE_psegment */
/* Descriptor for an allocated segment of slab pages */
struct psegment {
	byte_t                           *ps_minaddr; /* [1..1][const] Base address of segment */
	byte_t                           *ps_maxaddr; /* [1..1][const] Max address of segment */
	size_t                            ps_size;    /* [const] # of slab pages in this segment */
	size_t                            ps_free;    /* [lock(slab_segments_lock)] # of slab pages of this segment that are free */
	RBTREE_NODE(psegment)             ps_byaddr;  /* [lock(slab_segments_lock)] Node in tree of segments */
	TAILQ_ENTRY(psegment)             ps_byfree;  /* [lock(slab_segments_lock)][valid_if(ps_free != 0)] Link in list of segments sorted by # of free pages */
	size_t                            ps_temp;    /* [lock(slab_segments_lock)] Temporary used during optimizing */
	bool                              ps_red;     /* [lock(slab_segments_lock)] red/black bit for "ps_byaddr" */
	COMPILER_FLEXIBLE_ARRAY(bitset_t, ps_used);   /* [lock(slab_segments_lock)] Bitset of pages that are in-use */
};

#define psegment_getminaddr(self) (self)->ps_minaddr
#define psegment_getmaxaddr(self) (self)->ps_maxaddr
#define psegment_getpage(self, index) \
	((struct Dee_slab_page *)(psegment_getminaddr(self) + (index) * Dee_SLAB_PAGESIZE))
#define psegment_indexof(self, page) \
	((size_t)(((byte_t *)(page) - psegment_getminaddr(self)) / Dee_SLAB_PAGESIZE))

/* Can't use slabs here for obvious reasons (hint: self-recursion) */
#define psegment_sizeof(n_pages)    (offsetof(struct psegment, ps_used) + BITSET_SIZEOF(n_pages))
#define psegment_malloc(n_pages)    ((struct psegment *)Dee_UntrackAlloc((Dee_Malloc)(psegment_sizeof(n_pages))))
#define psegment_trymalloc(n_pages) ((struct psegment *)Dee_UntrackAlloc((Dee_TryMalloc)(psegment_sizeof(n_pages))))
#define psegment_free(p)            Dee_Free(p)

#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_lock_t psegment_lock = Dee_ATOMIC_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define psegment_lock_available()  Dee_atomic_lock_available(&psegment_lock)
#define psegment_lock_acquired()   Dee_atomic_lock_acquired(&psegment_lock)
#define psegment_lock_tryacquire() Dee_atomic_lock_tryacquire(&psegment_lock)
#define psegment_lock_acquire()    Dee_atomic_lock_acquire(&psegment_lock)
#define psegment_lock_waitfor()    Dee_atomic_lock_waitfor(&psegment_lock)
#define psegment_lock_release()    Dee_atomic_lock_release(&psegment_lock)

TAILQ_HEAD(psegment_tailq, psegment);

/* [0..n][lock(psegment_lock)] List of page segments, sorted by "ps_free DESC"
 * (does not contain any segments with "ps_free == 0", or "ps_free >= ps_size") */
PRIVATE struct psegment_tailq psegment_byfree = TAILQ_HEAD_INITIALIZER(psegment_byfree);

/* [0..n][lock(psegment_lock)] List of page segments with "ps_free == 0" */
PRIVATE struct psegment_tailq psegment_empty = TAILQ_HEAD_INITIALIZER(psegment_empty);

/* [0..n][lock(psegment_lock)] Tree of page segments ordered by their address-range. */
PRIVATE RBTREE_ROOT(psegment) psegment_byaddr = NULL;

/* [lock(ATOMIC)] Page segment size (in slab pages) of new segments */
PRIVATE size_t psegment_size = 16;

/* min/max values for 'psegment_size' */
PRIVATE size_t psegment_size_min = 1;
PRIVATE size_t psegment_size_max = 1024;

PRIVATE void DCALL psegment_size__handle_alloc(size_t n_pages) {
	size_t more = n_pages << 1;
	if (more > psegment_size_max)
		more = psegment_size_max;
	if (more > n_pages)
		atomic_cmpxch(&psegment_size, n_pages, more);
}

PRIVATE void DCALL psegment_size__handle_free(size_t n_pages) {
	size_t less = n_pages >> 1;
	if (less < psegment_size_min)
		less = psegment_size_min;
	if (less < n_pages)
		atomic_cmpxch(&psegment_size, n_pages, less);
}


#define RBTREE(name)           psegment_byaddr_##name
#define RBTREE_T               struct psegment
#define RBTREE_Tkey            byte_t const *
#define RBTREE_GETMINKEY(self) psegment_getminaddr(self)
#define RBTREE_GETMAXKEY(self) psegment_getmaxaddr(self)
#define RBTREE_NODEFIELD       ps_byaddr
#define RBTREE_REDFIELD        ps_red
#define RBTREE_CC              DFCALL
#define RBTREE_NOTHROW         NOTHROW
#define RBTREE_DECL            PRIVATE
#define RBTREE_IMPL            PRIVATE
#define RBTREE_OMIT_REMOVE
DECL_END
#include <hybrid/sequence/rbtree-abi.h>
DECL_BEGIN

PRIVATE WUNUSED struct Dee_slab_page *DCALL
base_malloc_from_cache(void) {
	struct psegment *seg;
	struct Dee_slab_page *result;
	psegment_lock_acquire();
	/* Always try to allocate from the segment with the least # of remaining
	 * free pages. That segment is always the last element of `psegment_byfree',
	 * since that list is sorted by "ps_free DESC" */
	seg = TAILQ_LAST(&psegment_byfree, psegment_tailq);
	if (seg) {
		size_t free_index;
		ASSERT(seg->ps_free > 0);
		ASSERT(seg->ps_free < seg->ps_size);
		free_index = bitset_ffc(seg->ps_used, seg->ps_size);
		ASSERT(free_index < seg->ps_size);
		bitset_set(seg->ps_used, free_index);
		--seg->ps_free;
		if (seg->ps_free == 0) {
			TAILQ_REMOVE(&psegment_byfree, seg, ps_byfree);
			TAILQ_INSERT_TAIL(&psegment_empty, seg, ps_byfree);
		}
		result = psegment_getpage(seg, free_index);
		psegment_lock_release();
		return result;
	}
	psegment_lock_release();
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
psegment_byfree_insert(struct psegment *__restrict seg) {
	struct psegment *succ = TAILQ_FIRST(&psegment_byfree);
	while (succ) {
		if (succ->ps_free <= seg->ps_free) {
			TAILQ_INSERT_BEFORE(succ, seg, ps_byfree);
			return;
		}
		succ = TAILQ_NEXT(succ, ps_byfree);
	}
	TAILQ_INSERT_TAIL(&psegment_byfree, seg, ps_byfree);
}

LOCAL WUNUSED struct Dee_slab_page *DCALL
base_malloc_from_new_segment(bool do_try) {
	void *cdata;
	size_t n_pages;
	size_t n_bytes;
	struct psegment *seg;
	n_pages = atomic_read(&psegment_size);
again_alloc_n_pages:
	ASSERT(n_pages >= 1);
	n_bytes = n_pages * Dee_SLAB_PAGESIZE;
	seg = psegment_trymalloc(n_pages);
	if unlikely(!seg) {
		if (n_pages >= 2) {
			n_pages >>= 1;
			goto again_alloc_n_pages;
		}
		if (do_try)
			goto err;
		seg = psegment_malloc(n_pages);
		if unlikely(!seg)
			goto err;
	}

	/* Allocate raw (underlying) data */
	cdata = cslab_trymalloc(n_bytes);
	if unlikely(!cdata) {
		if (n_pages >= 2) {
			/* Try to go smaller... */
			n_pages >>= 1;
			psegment_free(seg);
			goto again_alloc_n_pages;
		}
		if (do_try)
			goto err_seg;
		cdata = cslab_malloc(n_bytes);
		if (!cdata)
			goto err_seg;
	}

	/* Potentially update "psegment_size" */
	psegment_size__handle_alloc(n_pages);

	/* Initialize segment */
	bitset_clearall(seg->ps_used, n_pages);
	bitset_set(seg->ps_used, 0);
	seg->ps_minaddr = (byte_t *)cdata;
	seg->ps_maxaddr = (byte_t *)cdata + n_bytes - 1;
	seg->ps_size    = n_pages;
	seg->ps_free    = n_pages - 1;

	psegment_lock_acquire();
	psegment_byfree_insert(seg);
	psegment_byaddr_insert(&psegment_byaddr, seg);
	psegment_lock_release();

	return psegment_getpage(seg, 0);
err_seg:
	psegment_free(seg);
err:
	return NULL;
}

PRIVATE WUNUSED struct Dee_slab_page *DCALL
base_malloc(void) {
	struct Dee_slab_page *result = base_malloc_from_cache();
	if (result)
		return result;
	return base_malloc_from_new_segment(false);
}

PRIVATE struct Dee_slab_page *DCALL
base_trymalloc(void) {
	struct Dee_slab_page *result = base_malloc_from_cache();
	if (result)
		return result;
	return base_malloc_from_new_segment(true);
}

PRIVATE NONNULL((1)) void DCALL
base_free(struct Dee_slab_page *__restrict page) {
	struct psegment *seg;
	struct psegment *prev;
	size_t index;
	psegment_lock_acquire();
	seg = psegment_byaddr_locate(psegment_byaddr, (byte_t *)page);
	ASSERTF(seg, "No slab segment exists at %p", page);
	ASSERT(seg->ps_free < seg->ps_size);
	index = psegment_indexof(seg, page);
	ASSERT(index < seg->ps_size);
	ASSERTF(bitset_test(seg->ps_used, index), "Page does not appear to be allocated");
	bitset_clear(seg->ps_used, index);
	++seg->ps_free;
	ASSERT(seg->ps_free <= seg->ps_size);
	if (seg->ps_free >= seg->ps_size) {
		/* Free this segment */
		psegment_byaddr_removenode(&psegment_byaddr, seg);
		if unlikely(seg->ps_free == 1) {
			TAILQ_REMOVE(&psegment_empty, seg, ps_byfree);
		} else {
			TAILQ_REMOVE(&psegment_byfree, seg, ps_byfree);
		}
		psegment_lock_release();
		ASSERT((seg->ps_size * Dee_SLAB_PAGESIZE) ==
		       (size_t)(psegment_getmaxaddr(seg) - psegment_getminaddr(seg)) + 1);
		/* Potentially update "psegment_size" */
		psegment_size__handle_free(seg->ps_size);
		cslab_free(psegment_getminaddr(seg), seg->ps_size * Dee_SLAB_PAGESIZE);
		psegment_free(seg);
		return;
	}

	if (seg->ps_free == 1) {
		/* First page of segment free'd */
		TAILQ_REMOVE(&psegment_empty, seg, ps_byfree);
		TAILQ_INSERT_TAIL(&psegment_byfree, seg, ps_byfree);
	} else {
		/* Update position of segment within "psegment_byfree" */
		while ((prev = TAILQ_PREV(seg, psegment_tailq, ps_byfree)) != NULL) {
			if (prev->ps_free >= seg->ps_free)
				break; /* Ordering has been fixed! */
			TAILQ_REMOVE(&psegment_byfree, seg, ps_byfree);
			TAILQ_INSERT_BEFORE(prev, seg, ps_byfree);
		}
	}
	psegment_lock_release();
}
#endif /* USE_psegment */



/* Fast free-page cache */
#ifdef USE_fslab
struct fslab_page {
	union {
		SLIST_ENTRY(fslab_page) d_free; /* Link in list of free pages */
		struct Dee_slab_page    d_page; /* Underlying page */
	} fsp_data;
};

SLIST_HEAD(fslab_list, fslab_page);

#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_lock_t fslab_lock = Dee_ATOMIC_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define fslab_lock_available()  Dee_atomic_lock_available(&fslab_lock)
#define fslab_lock_acquired()   Dee_atomic_lock_acquired(&fslab_lock)
#define fslab_lock_tryacquire() Dee_atomic_lock_tryacquire(&fslab_lock)
#define fslab_lock_acquire()    Dee_atomic_lock_acquire(&fslab_lock)
#define fslab_lock_waitfor()    Dee_atomic_lock_waitfor(&fslab_lock)
#define fslab_lock_release()    Dee_atomic_lock_release(&fslab_lock)

/* [0..n][lock(fslab_lock)] Fast cache of free pages */
PRIVATE struct fslab_list fslab_pages = SLIST_HEAD_INITIALIZER(fslab_pages);

/* [lock(fslab_lock)] # of elements in `fslab_pages' */
PRIVATE size_t fslab_size = 0;

/* [lock(fslab_lock)] # of calls to `Dee_slab_page_rawmalloc_uncached()'
 * since the last clear of the fast-free cache */
PRIVATE size_t fslab_allocs_since_last_clear = 0;

#ifndef FSLAB_TRESHOLD_MIN
#define FSLAB_TRESHOLD_MIN 16
#endif /* !FSLAB_TRESHOLD_MIN */

/* [lock(fslab_lock)] Threshold before half of "fslab_pages"
 * are passed to "Dee_slab_page_rawfree_uncached()".
 * - Increased before 'Dee_slab_page_rawmalloc_uncached()' when:
 *   fslab_allocs_since_last_clear == 0
 * - Decreased before 'Dee_slab_page_rawfree_uncached()' when:
 *   fslab_allocs_since_last_clear == 0 */
PRIVATE size_t fslab_treshold = FSLAB_TRESHOLD_MIN - 1;


/* Free pages from "fslab_pages", but try to keep at least
 * "keep" pages around. Afterwards, release "fslab_lock" and
 * return the # of pages that were actually free'd.
 *
 * @return: 0 : Locks were lost without anything being free'd.
 *              Caller should must try again */
PRIVATE size_t DCALL
fslab_free_some_pages_and_release(size_t keep) {
	size_t result = 0;
	struct fslab_list free_list;
	ASSERT(fslab_size > keep);
	ASSERT(!SLIST_EMPTY(&fslab_pages));
#ifndef __OPTIMIZE_SIZE__
	if (keep == 0) {
		/* Special case: just free everything */
		free_list = fslab_pages;
		SLIST_INIT(&fslab_pages);
		fslab_size = 0;
		goto release_locks_2;
	}
#endif /* !__OPTIMIZE_SIZE__ */
	if (!psegment_lock_tryacquire())
		return 0;

	SLIST_INIT(&free_list);
	for (;;) {
		struct fslab_page **p_iter, *iter;
		struct psegment *largest_segment;

		/* Figure out what segments will look like if we don't keep anything */
		SLIST_FOREACH (iter, &fslab_pages, fsp_data.d_free) {
			struct psegment *seg;
			seg = psegment_byaddr_locate(psegment_byaddr, (byte_t *)iter);
			ASSERTF(seg, "Missing slab segment at %p", iter);
			seg->ps_temp = seg->ps_free;
		}
		largest_segment = NULL;
		SLIST_FOREACH (iter, &fslab_pages, fsp_data.d_free) {
			struct psegment *seg;
			seg = psegment_byaddr_locate(psegment_byaddr, (byte_t *)iter);
			ASSERTF(seg, "Missing slab segment at %p", iter);
			++seg->ps_temp;
			ASSERT(seg->ps_temp <= seg->ps_size);
			if (largest_segment == NULL ||
			    largest_segment->ps_temp < seg->ps_temp)
				largest_segment = seg;
		}
		ASSERT(largest_segment);

		/* Free pages belonging to the largest segment */
		/* TODO: This tends to free pages that were recently used (and may thus still
		 *       be in the host CPU's cache banks) -- at this point, it'd be better
		 *       to enumerate "fslab_pages" in reverse, freeing pages that were least
		 *       recently used! */
		for (p_iter = SLIST_PFIRST(&fslab_pages); (iter = *p_iter) != NULL;) {
			struct psegment *seg;
			seg = psegment_byaddr_locate(psegment_byaddr, (byte_t *)iter);
			ASSERTF(seg, "Missing slab segment at %p", iter);
			if (seg == largest_segment) {
				/* Add to free list */
				SLIST_P_REMOVE(p_iter, fsp_data.d_free);
				SLIST_INSERT(&free_list, iter, fsp_data.d_free);
				ASSERT(fslab_size != 0);
				--fslab_size;
				if (fslab_size <= keep)
					goto release_locks;
			} else {
				p_iter = SLIST_PNEXT(iter, fsp_data.d_free);
			}
		}
		ASSERT(fslab_size > keep);
	}

release_locks:
	ASSERT(!SLIST_EMPTY(&free_list));
	psegment_lock_release();
#ifndef __OPTIMIZE_SIZE__
release_locks_2:
#endif /* !__OPTIMIZE_SIZE__ */
	fslab_lock_release();

	/* Actually free pages */
	result = 0;
	do {
		struct fslab_page *page;
		page = SLIST_FIRST(&free_list);
		SLIST_REMOVE_HEAD(&free_list, fsp_data.d_free);
		base_free(&page->fsp_data.d_page);
	} while (!SLIST_EMPTY(&free_list));

	return result;
}

PRIVATE WUNUSED struct fslab_page *DCALL fslab_trymalloc(void) {
	struct fslab_page *result;
	fslab_lock_acquire();
	ASSERT(!!SLIST_EMPTY(&fslab_pages) == !!(fslab_size == 0));
	result = SLIST_FIRST(&fslab_pages);
	if (result) {
		SLIST_REMOVE_HEAD(&fslab_pages, fsp_data.d_free);
		--fslab_size;
	} else {
		if (fslab_allocs_since_last_clear == 0)
			++fslab_treshold;
		++fslab_allocs_since_last_clear;
	}
	fslab_lock_release();
	return result;
}

PRIVATE NONNULL((1)) void DCALL
fslab_free(struct fslab_page *page) {
	size_t new_target_slab_size;
	fslab_lock_acquire();
	SLIST_INSERT_HEAD(&fslab_pages, page, fsp_data.d_free);
	++fslab_size;
	if (fslab_size < fslab_treshold) {
		fslab_lock_release();
		return;
	}
	if (fslab_allocs_since_last_clear == 0 && fslab_treshold > FSLAB_TRESHOLD_MIN)
		--fslab_treshold;
	fslab_allocs_since_last_clear = 0;
	new_target_slab_size = fslab_size / 2;
	/* Exchange a given page for another that should be used in favor of "page" */
	while (fslab_free_some_pages_and_release(new_target_slab_size) == 0) {
		fslab_lock_acquire();
		if (new_target_slab_size >= fslab_size) {
			fslab_lock_release();
			break;
		}
	}
}

PRIVATE size_t DCALL
fslab_clearcache(size_t pad) {
	size_t result;
	size_t n_keep = pad / Dee_SLAB_PAGESIZE;
again:
	fslab_lock_acquire();
	if (n_keep >= fslab_size) {
		fslab_lock_release();
		return 0;
	}
	result = fslab_free_some_pages_and_release(n_keep);
	if unlikely(!result)
		goto again;
	return result;
}
#endif /* USE_fslab */

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
#ifdef USE_fslab
	struct fslab_page *fast = fslab_trymalloc();
	if (fast)
		return &fast->fsp_data.d_page;
#endif /* USE_fslab */
	return base_malloc();
}

PUBLIC ATTR_MALLOC WUNUSED ATTR_ASSUME_ALIGNED(Dee_SLAB_PAGESIZE)
struct Dee_slab_page *DCALL Dee_slab_page_tryrawmalloc(void) {
#ifdef USE_fslab
	struct fslab_page *fast = fslab_trymalloc();
	if (fast)
		return &fast->fsp_data.d_page;
#endif /* USE_fslab */
	return base_trymalloc();
}

PUBLIC NONNULL((1)) void DCALL
Dee_slab_page_rawfree(struct Dee_slab_page *__restrict page) {
	/* TODO: Debug checks for use-after-free (fill "page" with a
	 *       known debug pattern, probably the same pattern as
	 *       also used by the actual slab allocator, and define
	 *       a function called by `DeeHeap_CheckMemory()' to
	 *       verify that this pattern is still in-place in all
	 *       places it should be) */
#ifdef USE_fslab
	struct fslab_page *fpage;
	fpage = container_of(page, struct fslab_page, fsp_data.d_page);
	fslab_free(fpage);
#else /* USE_fslab */
	base_free(page);
#endif /* !USE_fslab */
}



/* Clear caches kept by the raw slab page allocator.
 * This function is automatically called by `DeeHeap_Trim()'
 * @param: pad: Try to keep at least this many bytes within the cache
 * @return: * : The # of bytes free'd from the cache. */
INTERN size_t DCALL Dee_slab_page_rawtrim(size_t pad) {
	size_t result = 0;
	(void)pad;
#ifdef USE_fslab
	result += fslab_clearcache(pad);
#endif /* USE_fslab */
#ifdef USE_psegment
#ifdef cslab_malloc_CAN_COALESCE
	/* TODO: Split/truncate "psegment"s to get cut out unused pages */
#endif /* cslab_malloc_CAN_COALESCE */
#endif /* USE_psegment */
	return result;
}


DECL_END
#endif /* Dee_SLAB_CHUNKSIZE_MAX */

#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

#endif /* !GUARD_DEEMON_RUNTIME_SLAB_RAWMALLOC_C */
