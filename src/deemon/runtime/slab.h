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
#ifndef GUARD_DEEMON_RUNTIME_SLAB_H
#define GUARD_DEEMON_RUNTIME_SLAB_H 1

#include <deemon/api.h>

#include <deemon/types.h>            /* Dee_ssize_t */
#include <deemon/util/lock.h>        /* Dee_atomic_rwlock_t */
#include <deemon/util/slab-config.h> /* Dee_SLAB_CHUNKSIZE_MAX */
#include <deemon/util/slab.h>        /* Dee_OFFSET_SLAB_PAGE_META, Dee_slab_page, Dee_slab_page_builder_offset_t */

#include <hybrid/align.h>    /* CEILDIV */
#include <hybrid/typecore.h> /* __CHAR_BIT__, __SIZEOF_INT_FASTn_T__, __SIZEOF_POINTER__, __UINTPTR_C, __UINTPTR_TYPE__, __UINT_FASTn_TYPE__, __UINTn_C */

#include <stddef.h> /* size_t */

#ifdef Dee_SLAB_CHUNKSIZE_MAX
DECL_BEGIN

/* Select "slab_bitword_t" data type. */
#if __SIZEOF_POINTER__ <= 1
#define slab_bitword_t        __UINT_FAST8_TYPE__
#define SLAB_BITWORD_C(v)     __UINT8_C(v)
#define SIZEOF_slab_bitword_t __SIZEOF_INT_FAST8_T__
#elif __SIZEOF_POINTER__ <= 2
#define slab_bitword_t        __UINT_FAST16_TYPE__
#define SLAB_BITWORD_C(v)     __UINT16_C(v)
#define SIZEOF_slab_bitword_t __SIZEOF_INT_FAST16_T__
#elif __SIZEOF_POINTER__ <= 4
#define slab_bitword_t        __UINT_FAST32_TYPE__
#define SLAB_BITWORD_C(v)     __UINT32_C(v)
#define SIZEOF_slab_bitword_t __SIZEOF_INT_FAST32_T__
#elif __SIZEOF_POINTER__ <= 8
#define slab_bitword_t        __UINT_FAST64_TYPE__
#define SLAB_BITWORD_C(v)     __UINT64_C(v)
#define SIZEOF_slab_bitword_t __SIZEOF_INT_FAST64_T__
#else /* __SIZEOF_POINTER__ <= ... */
#define slab_bitword_t        __UINTPTR_TYPE__
#define SLAB_BITWORD_C(v)     __UINTPTR_C(v)
#define SIZEOF_slab_bitword_t __SIZEOF_POINTER__
#endif /* __SIZEOF_POINTER__ > ... */
#define BITSOF_slab_bitword_t (SIZEOF_slab_bitword_t * __CHAR_BIT__)


/* Helpers for constructing bitset index/mask for a given
 * "bitno" (which is the index of the relevant chunk within
 * a slab page) */
#define slab_bitword_indx(bitno) ((bitno) / BITSOF_slab_bitword_t)
#define slab_bitword_mask(bitno) (SLAB_BITWORD_C(1) << ((bitno) % BITSOF_slab_bitword_t))


/* Calculate the # of chunks that fit into a slab page, given the size of those chunks.
 * Since the formula for this is self-dependent, we use 3 consecutive approximations,
 * which seems to produce optimial results in all situations. */
#define _UPPER__MAX_CHUNK_COUNT(n) (Dee_OFFSET_SLAB_PAGE_META / n)
#define _UPPER_ELEMOF__sp_used(n)  CEILDIV(_UPPER__MAX_CHUNK_COUNT(n), BITSOF_slab_bitword_t)
#define _UPPER_SIZEOF__sp_used(n)  (_UPPER_ELEMOF__sp_used(n) * SIZEOF_slab_bitword_t)
#define _LOWER_SIZEOF__sp_data(n)  (Dee_OFFSET_SLAB_PAGE_META - _UPPER_SIZEOF__sp_used(n))
#define _LOWER__MAX_CHUNK_COUNT(n) (_LOWER_SIZEOF__sp_data(n) / n)
#define _LOWER_ELEMOF__sp_used(n)  CEILDIV(_LOWER__MAX_CHUNK_COUNT(n), BITSOF_slab_bitword_t)
#define _LOWER_SIZEOF__sp_used(n)  (_LOWER_ELEMOF__sp_used(n) * SIZEOF_slab_bitword_t)
#define _LOWER_BITSOF__sp_used(n)  (_LOWER_ELEMOF__sp_used(n) * BITSOF_slab_bitword_t)
#define _FINAL_SIZEOF__sp_data(n)  (Dee_OFFSET_SLAB_PAGE_META - _LOWER_SIZEOF__sp_used(n))
#define _MAX_CHUNK_COUNT(n)        (_FINAL_SIZEOF__sp_data(n) / n)
#define MAX_CHUNK_COUNT(n)         (_MAX_CHUNK_COUNT(n) > _LOWER_BITSOF__sp_used(n) ? _LOWER_BITSOF__sp_used(n) : _MAX_CHUNK_COUNT(n))



struct chunkleak {
	/* Value cases:
	 * - cl_file != NULL && cl_line >= 0:   Regular leak
	 * - cl_file != NULL && cl_line <  0:   Regular leak (reachable during GC leak detect)
	 * - cl_file == NULL && cl_line == -1:  Untracked leak */
	char const *cl_file; /* [0..1] Allocation file, or "NULL" if unknown */
	int         cl_line; /* Allocation line, or "0" if unknown. During GC leak
	                      * detection, this is also used to store reachability
	                      * metadata. */
};

/* Heap-allocated structure that is pointed-to by `struct Dee_slab_page::sp_meta::spm_leak'
 * When `spm_leak == NULL || Dee_slab_page_iscustom()', no leak debug info is attached to
 * the slab page. */
struct pageleaks {
	/* XXX: "pl_chnksiz" and "pl_chnkcnt" are only never used by assertions -- maybe get rid of them? */
	size_t pl_chnksiz; /* [const] Size of individual chunks within this leak page */
	size_t pl_chnkcnt; /* [const] # of chunks in this page */
	/* [lock(CALLER_IS_OWNER_OF_RELEVANT_CHUNK)]
	 * [valid_if(<chunk-is-allocated-in-linked-Dee_slab_page-sp_used>)]
	 * [pl_chnkcnt] Debug info attached to individual chunks. */
	COMPILER_FLEXIBLE_ARRAY(struct chunkleak, pl_chunks);
};

struct pagespecs {
	Dee_slab_page_builder_offset_t ps_chunksize;       /* Size of a single chunk */
	Dee_slab_page_builder_offset_t ps_chunkcount;      /* # of chunks in a page */
	Dee_slab_page_builder_offset_t ps_sizeof__sp_used; /* Size of "sp_used" / offset of "sp_data" */
};

#define PAGESPECS_INIT(n)                                                                \
	{                                                                                    \
		/* .ps_chunksize       = */ n,                                                   \
		/* .ps_chunkcount      = */ MAX_CHUNK_COUNT(n),                                  \
		/* .ps_sizeof__sp_used = */ CEILDIV(MAX_CHUNK_COUNT(n), BITSOF_slab_bitword_t) * \
		/*                       */ SIZEOF_slab_bitword_t,                               \
	}


/* Helper functions needed by "heap.c" to debug slab leaks */
#ifndef NDEBUG

/* Try acquire write-locks to the locks of all slab allocators.
 * If at least one of those locks can't be acquired, release all
 * locks already acquired and return the blocking lock. */
#ifndef CONFIG_NO_THREADS
INTDEF WUNUSED Dee_atomic_rwlock_t *DCALL Dee_slab_leaks_tryacquire(void);
INTDEF void DCALL Dee_slab_leaks_release(void);
#define HAVE_Dee_slab_leaks_tryacquire
#endif /* !CONFIG_NO_THREADS */

/* Callback prototype for `Dee_slab_leaks_foreach_page()' */
typedef WUNUSED_T NONNULL_T((2, 3)) Dee_ssize_t
(DCALL *Dee_slab_leaks_page_cb_t)(void *arg, struct Dee_slab_page *page,
                                  struct pagespecs const *specs);

/* Enumerate all slab pages containing at least 1 allocated chunk.
 * Before calling this function, the caller must acquire locks by
 * use of `Dee_slab_leaks_tryacquire()'
 * @return: * : Dee_formatprinter_t-style aggregate of calls to `cb' */
INTDEF NONNULL((1)) Dee_ssize_t DCALL
Dee_slab_leaks_foreach_page(Dee_slab_leaks_page_cb_t cb, void *arg);
#define HAVE_Dee_slab_leaks_foreach_page

/* Called by `DeeHeap_CheckMemory()' */
#define HAVE_DeeSlab_CheckMemory
INTDEF void DCALL DeeSlab_CheckMemory(void);
#endif /* !NDEBUG */

DECL_END

#endif /* Dee_SLAB_CHUNKSIZE_MAX */

#endif /* !GUARD_DEEMON_RUNTIME_SLAB_H */
