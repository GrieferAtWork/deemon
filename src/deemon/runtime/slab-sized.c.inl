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
#ifdef __INTELLISENSE__
#include "slab.c"
#define DEFINE_CHUNK_SIZE 32
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/alloc.h>            /* DeeSlab_* */
#include <deemon/system-features.h>  /* bzero */
#include <deemon/util/atomic.h>      /* atomic_* */
#include <deemon/util/lock.h>        /* Dee_atomic_rwlock_* */
#include <deemon/util/slab-config.h> /* Dee_SLAB_CHUNKSIZE_MAX, Dee_SLAB_CHUNKSIZE_MIN */
#include <deemon/util/slab.h>        /* Dee_OFFSET_SLAB_PAGE_META, Dee_SIZEOF_SLAB_PAGE_META, Dee_SLAB_PAGESIZE, Dee_SLAB_PAGE_META_FIELDS, Dee_slab_page, Dee_slab_page_* */

#include <hybrid/align.h>         /* CEILDIV */
#include <hybrid/bit.h>           /* CTZ */
#include <hybrid/sequence/list.h> /* LIST_* */
#include <hybrid/typecore.h>      /* __ALIGNOF_POINTER__, __BYTE_TYPE__, __CHAR_BIT__, __SHIFT_TYPE__ */

#include <stddef.h> /* offsetof, size_t */
#include <stdint.h> /* uintptr_t */

#undef byte_t
#define byte_t __BYTE_TYPE__
#undef shift_t
#define shift_t __SHIFT_TYPE__

#ifndef DEFINE_CHUNK_SIZE
#error "Must #define 'DEFINE_CHUNK_SIZE' before #including this file"
#endif /* !DEFINE_CHUNK_SIZE */

#define LOCAL_SYM(x) PP_CAT2(x, DEFINE_CHUNK_SIZE)

/* Local symbol names */
#define LOCAL_slab_page               LOCAL_SYM(slab_page)
#define LOCAL_slab_page_list          LOCAL_SYM(slab_page_list)
#define LOCAL_slab_pages              LOCAL_SYM(slab_pages)
#define LOCAL_slab_lock               LOCAL_SYM(slab_lock)
#define LOCAL_slab_malloc_in_page     LOCAL_SYM(slab_malloc_in_page)
#define LOCAL__DeeSlab_Malloc         LOCAL_SYM(DeeSlab_Malloc)
#define LOCAL__DeeSlab_Calloc         LOCAL_SYM(DeeSlab_Calloc)
#define LOCAL__DeeSlab_TryMalloc      LOCAL_SYM(DeeSlab_TryMalloc)
#define LOCAL__DeeSlab_TryCalloc      LOCAL_SYM(DeeSlab_TryCalloc)
#define LOCAL__DeeSlab_Free           LOCAL_SYM(DeeSlab_Free)
#define LOCAL__DeeDbgSlab_Malloc      LOCAL_SYM(DeeDbgSlab_Malloc)
#define LOCAL__DeeDbgSlab_Calloc      LOCAL_SYM(DeeDbgSlab_Calloc)
#define LOCAL__DeeDbgSlab_TryMalloc   LOCAL_SYM(DeeDbgSlab_TryMalloc)
#define LOCAL__DeeDbgSlab_TryCalloc   LOCAL_SYM(DeeDbgSlab_TryCalloc)
#define LOCAL__DeeDbgSlab_Free        LOCAL_SYM(DeeDbgSlab_Free)
#define LOCAL_DeeDbgSlab_UntrackAlloc LOCAL_SYM(DeeDbgSlab_UntrackAlloc)

#define LOCAL_DECL PUBLIC

/* Assert that the chunk size is valid as far as the slab configuration is concerned */
#if DEFINE_CHUNK_SIZE < Dee_SLAB_CHUNKSIZE_MIN
#error "Bad configuration: some chunk sizes are enabled that are less than 'Dee_SLAB_CHUNKSIZE_MIN'"
#endif /* DEFINE_CHUNK_SIZE < Dee_SLAB_CHUNKSIZE_MIN */
#if DEFINE_CHUNK_SIZE > Dee_SLAB_CHUNKSIZE_MAX
#error "Bad configuration: some chunk sizes are enabled that are greater than 'Dee_SLAB_CHUNKSIZE_MAX'"
#endif /* DEFINE_CHUNK_SIZE > Dee_SLAB_CHUNKSIZE_MAX */
#if !DeeSlab_EXISTS(DEFINE_CHUNK_SIZE)
#error "INTERNAL ERROR: Tried to define implementation for some non-existant 'DEFINE_CHUNK_SIZE'"
#endif /* !DeeSlab_EXISTS(DEFINE_CHUNK_SIZE) */

/* Assert that the chunk size makes sense from a logical point-of-view (note that this isn't
 * actually a requirement as far as semantics are concerned; the slab allocator could happily
 * define slabs for e.g. 3-byte chunks, and that would work just fine).
 *
 * -> But from a practical point-of-view, only chunks producing memory that is at least aligned
 *    by the alignment requirements of pointers would ever make sense (because anything less
 *    than that would mean that the memory is pretty much useless, as it couldn't even be used
 *    to store pointers). */
#if (DEFINE_CHUNK_SIZE % __ALIGNOF_POINTER__) != 0
#error "Bad configuration: chunks produced by 'DEFINE_CHUNK_SIZE' would not even be pointer-aligned"
#endif /* (DEFINE_CHUNK_SIZE % __ALIGNOF_POINTER__) != 0 */

DECL_BEGIN

/* Calculate "LOCAL_MAX_CHUNK_COUNT"  */
#define LOCAL_MAX_CHUNK_COUNT _MAX_CHUNK_COUNT(DEFINE_CHUNK_SIZE)
#if LOCAL_MAX_CHUNK_COUNT > _LOWER_BITSOF__sp_used(DEFINE_CHUNK_SIZE)
#undef LOCAL_MAX_CHUNK_COUNT
#define LOCAL_MAX_CHUNK_COUNT _LOWER_BITSOF__sp_used(DEFINE_CHUNK_SIZE)
#endif /* LOCAL_MAX_CHUNK_COUNT > _LOWER_BITSOF__sp_used(DEFINE_CHUNK_SIZE) */

/* Macros for some other constants */
#define LOCAL_ELEMOF__sp_used CEILDIV(LOCAL_MAX_CHUNK_COUNT, BITSOF_bitword_t)
#define LOCAL_SIZEOF__sp_data (LOCAL_MAX_CHUNK_COUNT * DEFINE_CHUNK_SIZE)
#define LOCAL_SIZEOF__sp_used (LOCAL_ELEMOF__sp_used * SIZEOF_bitword_t)
#define LOCAL_BITSOF__sp_used (LOCAL_ELEMOF__sp_used * BITSOF_bitword_t)
#define LOCAL_SIZEOF__sp_pad  (Dee_SLAB_PAGESIZE - (Dee_SIZEOF_SLAB_PAGE_META + LOCAL_SIZEOF__sp_used + LOCAL_SIZEOF__sp_data))

/* Sanity check the calculation above */
STATIC_ASSERT(LOCAL_BITSOF__sp_used == (LOCAL_SIZEOF__sp_used * __CHAR_BIT__));
STATIC_ASSERT(LOCAL_SIZEOF__sp_data == (LOCAL_MAX_CHUNK_COUNT * DEFINE_CHUNK_SIZE));
STATIC_ASSERT(LOCAL_MAX_CHUNK_COUNT <= LOCAL_BITSOF__sp_used);
STATIC_ASSERT((LOCAL_SIZEOF__sp_used + LOCAL_SIZEOF__sp_data +
               LOCAL_SIZEOF__sp_pad + Dee_SIZEOF_SLAB_PAGE_META) == Dee_SLAB_PAGESIZE);
STATIC_ASSERT_MSG(LOCAL_SIZEOF__sp_pad < (DEFINE_CHUNK_SIZE + SIZEOF_bitword_t),
                  "More padding than this wouldn't make sense, because "
                  "then 1 extra item in `sp_data' should have been used");

/* Define the "slab_page" structure */
struct LOCAL_slab_page {
	bitword_t sp_used[LOCAL_ELEMOF__sp_used]; /* Bitset of allocated chunk base addresses */
	byte_t    sp_data[LOCAL_SIZEOF__sp_data]; /* Slab payload data */
#if LOCAL_SIZEOF__sp_pad != 0
	byte_t   _sp_pad[LOCAL_SIZEOF__sp_pad];   /* Unused padding */
#endif /* LOCAL_SIZEOF__sp_pad != 0 */
	struct {
		Dee_SLAB_PAGE_META_FIELDS(LOCAL_slab_page)
	} sp_meta; /* Slab footer/metadata */
};

/* Sanity check the page-struct definition */
STATIC_ASSERT(sizeof(struct LOCAL_slab_page) == Dee_SLAB_PAGESIZE);
STATIC_ASSERT(offsetof(struct LOCAL_slab_page, sp_used) == 0);
STATIC_ASSERT(offsetof(struct LOCAL_slab_page, sp_data) == LOCAL_SIZEOF__sp_used);
STATIC_ASSERT(offsetof(struct LOCAL_slab_page, sp_meta) == Dee_OFFSET_SLAB_PAGE_META);
STATIC_ASSERT(sizeof(((struct LOCAL_slab_page *)0)->sp_meta) == Dee_SIZEOF_SLAB_PAGE_META);

LIST_HEAD(LOCAL_slab_page_list, LOCAL_slab_page);

/* Lock for this specific slab-size */
#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t LOCAL_slab_lock = Dee_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */

#define LOCAL_slab_lock_reading()    Dee_atomic_rwlock_reading(&LOCAL_slab_lock)
#define LOCAL_slab_lock_writing()    Dee_atomic_rwlock_writing(&LOCAL_slab_lock)
#define LOCAL_slab_lock_tryread()    Dee_atomic_rwlock_tryread(&LOCAL_slab_lock)
#define LOCAL_slab_lock_trywrite()   Dee_atomic_rwlock_trywrite(&LOCAL_slab_lock)
#define LOCAL_slab_lock_canread()    Dee_atomic_rwlock_canread(&LOCAL_slab_lock)
#define LOCAL_slab_lock_canwrite()   Dee_atomic_rwlock_canwrite(&LOCAL_slab_lock)
#define LOCAL_slab_lock_waitread()   Dee_atomic_rwlock_waitread(&LOCAL_slab_lock)
#define LOCAL_slab_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&LOCAL_slab_lock)
#define LOCAL_slab_lock_read()       Dee_atomic_rwlock_read(&LOCAL_slab_lock)
#define LOCAL_slab_lock_write()      Dee_atomic_rwlock_write(&LOCAL_slab_lock)
#define LOCAL_slab_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&LOCAL_slab_lock)
#define LOCAL_slab_lock_upgrade()    Dee_atomic_rwlock_upgrade(&LOCAL_slab_lock)
#define LOCAL_slab_lock_downgrade()  Dee_atomic_rwlock_downgrade(&LOCAL_slab_lock)
#define LOCAL_slab_lock_endwrite()   Dee_atomic_rwlock_endwrite(&LOCAL_slab_lock)
#define LOCAL_slab_lock_endread()    Dee_atomic_rwlock_endread(&LOCAL_slab_lock)
#define LOCAL_slab_lock_end()        Dee_atomic_rwlock_end(&LOCAL_slab_lock)


/* [0..n][lock(LOCAL_slab_lock)] Pages containing at least 1 free, and at least 1 allocated chunk.
 * - fully allocated pages aren't tracked anywhere
 * - fully free pages are stored in a global free-list (so they can be used by all slab allocators) */
PRIVATE struct LOCAL_slab_page_list LOCAL_slab_pages = LIST_HEAD_INITIALIZER(LOCAL_slab_pages);



/* Calculate the last-word mask for "sp_used" */
#define LOCAL_sp_used__UNUSED_TRAILING_BITS (LOCAL_BITSOF__sp_used - LOCAL_MAX_CHUNK_COUNT)
#define LOCAL_sp_used__USED_TRAILING_BITS   (BITSOF_bitword_t - LOCAL_sp_used__UNUSED_TRAILING_BITS)
#define LOCAL_sp_used__LAST_USED_MASK       ((BITWORD_C(1) << LOCAL_sp_used__USED_TRAILING_BITS) - 1)



/* Helper function to find+allocate a previously reserved chunk within a given slab page. */
PRIVATE WUNUSED NONNULL((1)) void *DCALL
LOCAL_slab_malloc_in_page(struct LOCAL_slab_page *__restrict page) {
	/* Caller guaranties that something is free (since they reserved a spot for us)
	 * -- we just need to find that spot! */
	bitword_t word;

#define LOCAL_maskfor(i)                \
	((i) >= (LOCAL_ELEMOF__sp_used - 1) \
	 ? LOCAL_sp_used__LAST_USED_MASK    \
	 : (bitword_t) - 1)
#if 1 /* Both impls here work the same */
#define LOCAL_alloc_in_word(p, word, mask) atomic_cmpxch_weak(p, word, word | mask)
#else
#define LOCAL_alloc_in_word(p, word, mask) (!(atomic_fetchor(p, mask) & mask))
#endif
#define LOCAL_alloc_from(i, full_mask)                                        \
	if ((word = atomic_read(&page->sp_used[i])) != (full_mask)) {             \
		/* There seems to be something free here! */                          \
		shift_t free_bit     = CTZ(~word);                                    \
		bitword_t alloc_mask = (bitword_t)1 << free_bit;                      \
		slab_assert(!(word & alloc_mask));                                    \
		if (LOCAL_alloc_in_word(&page->sp_used[i], word, alloc_mask)) {       \
			/* Got it! -- now just calculate the pointer we need to return */ \
			size_t index  = i * BITSOF_bitword_t + free_bit;                  \
			size_t offset = index * DEFINE_CHUNK_SIZE;                        \
			return page->sp_data + offset;                                    \
		}                                                                     \
	}

	/* Specialized code for forceably unwound loops */
/*[[[deemon
local maxInline = 16;
print("#ifndef __OPTIMIZE_SIZE__");
print("#if LOCAL_ELEMOF__sp_used <= ", maxInline);
print("	for (;;) {");
print("#define LOCAL_HAVE_slab_malloc_in_page");
print("		LOCAL_alloc_from(0, LOCAL_maskfor(0));");
for (local i: [1:maxInline+1]) {
	print("#if LOCAL_ELEMOF__sp_used >= ", i+1);
	print("		LOCAL_alloc_from(", i, ", LOCAL_maskfor(", i, "));");
}
for (local i: [maxInline:0,-1])
	print("#endif /" "* LOCAL_ELEMOF__sp_used >= ", i+1, " *" "/");
print("	}");
print("#endif /" "* LOCAL_ELEMOF__sp_used <= ", maxInline, " *" "/");
print("#endif /" "* !__OPTIMIZE_SIZE__ *" "/");
]]]*/
#ifndef __OPTIMIZE_SIZE__
#if LOCAL_ELEMOF__sp_used <= 16
	for (;;) {
#define LOCAL_HAVE_slab_malloc_in_page
		LOCAL_alloc_from(0, LOCAL_maskfor(0));
#if LOCAL_ELEMOF__sp_used >= 2
		LOCAL_alloc_from(1, LOCAL_maskfor(1));
#if LOCAL_ELEMOF__sp_used >= 3
		LOCAL_alloc_from(2, LOCAL_maskfor(2));
#if LOCAL_ELEMOF__sp_used >= 4
		LOCAL_alloc_from(3, LOCAL_maskfor(3));
#if LOCAL_ELEMOF__sp_used >= 5
		LOCAL_alloc_from(4, LOCAL_maskfor(4));
#if LOCAL_ELEMOF__sp_used >= 6
		LOCAL_alloc_from(5, LOCAL_maskfor(5));
#if LOCAL_ELEMOF__sp_used >= 7
		LOCAL_alloc_from(6, LOCAL_maskfor(6));
#if LOCAL_ELEMOF__sp_used >= 8
		LOCAL_alloc_from(7, LOCAL_maskfor(7));
#if LOCAL_ELEMOF__sp_used >= 9
		LOCAL_alloc_from(8, LOCAL_maskfor(8));
#if LOCAL_ELEMOF__sp_used >= 10
		LOCAL_alloc_from(9, LOCAL_maskfor(9));
#if LOCAL_ELEMOF__sp_used >= 11
		LOCAL_alloc_from(10, LOCAL_maskfor(10));
#if LOCAL_ELEMOF__sp_used >= 12
		LOCAL_alloc_from(11, LOCAL_maskfor(11));
#if LOCAL_ELEMOF__sp_used >= 13
		LOCAL_alloc_from(12, LOCAL_maskfor(12));
#if LOCAL_ELEMOF__sp_used >= 14
		LOCAL_alloc_from(13, LOCAL_maskfor(13));
#if LOCAL_ELEMOF__sp_used >= 15
		LOCAL_alloc_from(14, LOCAL_maskfor(14));
#if LOCAL_ELEMOF__sp_used >= 16
		LOCAL_alloc_from(15, LOCAL_maskfor(15));
#if LOCAL_ELEMOF__sp_used >= 17
		LOCAL_alloc_from(16, LOCAL_maskfor(16));
#endif /* LOCAL_ELEMOF__sp_used >= 17 */
#endif /* LOCAL_ELEMOF__sp_used >= 16 */
#endif /* LOCAL_ELEMOF__sp_used >= 15 */
#endif /* LOCAL_ELEMOF__sp_used >= 14 */
#endif /* LOCAL_ELEMOF__sp_used >= 13 */
#endif /* LOCAL_ELEMOF__sp_used >= 12 */
#endif /* LOCAL_ELEMOF__sp_used >= 11 */
#endif /* LOCAL_ELEMOF__sp_used >= 10 */
#endif /* LOCAL_ELEMOF__sp_used >= 9 */
#endif /* LOCAL_ELEMOF__sp_used >= 8 */
#endif /* LOCAL_ELEMOF__sp_used >= 7 */
#endif /* LOCAL_ELEMOF__sp_used >= 6 */
#endif /* LOCAL_ELEMOF__sp_used >= 5 */
#endif /* LOCAL_ELEMOF__sp_used >= 4 */
#endif /* LOCAL_ELEMOF__sp_used >= 3 */
#endif /* LOCAL_ELEMOF__sp_used >= 2 */
	}
#endif /* LOCAL_ELEMOF__sp_used <= 16 */
#endif /* !__OPTIMIZE_SIZE__ */
/*[[[end]]]*/

	/* Fallback code using a proper loop */
#ifndef LOCAL_HAVE_slab_malloc_in_page
	for (;;) {
		size_t i;
		for (i = 0; i < LOCAL_ELEMOF__sp_used; ++i) {
			bitword_t full_mask = LOCAL_maskfor(i);
			LOCAL_alloc_from(i, full_mask);
		}
	}
#endif /* !LOCAL_HAVE_slab_malloc_in_page */
#undef LOCAL_HAVE_slab_malloc_in_page
#undef LOCAL_alloc_from
#undef LOCAL_alloc_in_word
#undef LOCAL_maskfor
}


/* Select primary malloc implementation */
#if SLAB_DEBUG_LEAKS
/* Because of how much non-debug slab allocators are used, also directly define them here */
#if !defined(__OPTIMIZE_SIZE__) && !defined(__INTELLISENSE__) && 1
#define LOCAL_DeeSlab_Malloc            LOCAL__DeeSlab_Malloc
#define LOCAL_DeeSlab_Calloc            LOCAL__DeeSlab_Calloc
#define LOCAL_DeeSlab_TryMalloc         LOCAL__DeeSlab_TryMalloc
#define LOCAL_DeeSlab_TryCalloc         LOCAL__DeeSlab_TryCalloc
#define LOCAL_DeeSlab_Malloc_DBG_PARAMS void
#define LOCAL_DeeSlab_Malloc_DBG_ARGS   /* nothing */
#undef LOCAL_DeeSlab_Malloc_DBG_ARGS_PRESENT
DECL_END
#define DEFINE_LOCAL_DeeSlab_Malloc
#include "slab-sized-alloc.c.inl"
#define DEFINE_LOCAL_DeeSlab_TryMalloc
#include "slab-sized-alloc.c.inl"
DECL_BEGIN
#undef LOCAL_DeeSlab_Malloc
#undef LOCAL_DeeSlab_Calloc
#undef LOCAL_DeeSlab_TryMalloc
#undef LOCAL_DeeSlab_TryCalloc
#undef LOCAL_DeeSlab_Malloc_DBG_PARAMS
#undef LOCAL_DeeSlab_Malloc_DBG_ARGS
#else /* !__OPTIMIZE_SIZE__ */
LOCAL_DECL ATTR_MALLOC WUNUSED void *DCALL
LOCAL__DeeSlab_Malloc(void) {
	return LOCAL__DeeDbgSlab_Malloc(NULL, 0);
}

LOCAL_DECL ATTR_MALLOC WUNUSED void *DCALL
LOCAL__DeeSlab_Calloc(void) {
	return LOCAL__DeeDbgSlab_Calloc(NULL, 0);
}

LOCAL_DECL ATTR_MALLOC WUNUSED void *DCALL
LOCAL__DeeSlab_TryMalloc(void) {
	return LOCAL__DeeDbgSlab_TryMalloc(NULL, 0);
}

LOCAL_DECL ATTR_MALLOC WUNUSED void *DCALL
LOCAL__DeeSlab_TryCalloc(void) {
	return LOCAL__DeeDbgSlab_TryCalloc(NULL, 0);
}
#endif /* __OPTIMIZE_SIZE__ */

#define LOCAL_DeeSlab_Malloc            LOCAL__DeeDbgSlab_Malloc
#define LOCAL_DeeSlab_Calloc            LOCAL__DeeDbgSlab_Calloc
#define LOCAL_DeeSlab_TryMalloc         LOCAL__DeeDbgSlab_TryMalloc
#define LOCAL_DeeSlab_TryCalloc         LOCAL__DeeDbgSlab_TryCalloc
#define LOCAL_DeeSlab_Malloc_DBG_PARAMS char const *file, int line
#define LOCAL_DeeSlab_Malloc_DBG_ARGS   file, line
#define LOCAL_DeeSlab_Malloc_DBG_ARGS_PRESENT
#else /* SLAB_DEBUG_LEAKS */
#define LOCAL_DeeSlab_Malloc            LOCAL__DeeSlab_Malloc
#define LOCAL_DeeSlab_Calloc            LOCAL__DeeSlab_Calloc
#define LOCAL_DeeSlab_TryMalloc         LOCAL__DeeSlab_TryMalloc
#define LOCAL_DeeSlab_TryCalloc         LOCAL__DeeSlab_TryCalloc
#define LOCAL_DeeSlab_Malloc_DBG_PARAMS void
#define LOCAL_DeeSlab_Malloc_DBG_ARGS   /* nothing */
#undef LOCAL_DeeSlab_Malloc_DBG_ARGS_PRESENT

LOCAL_DECL ATTR_MALLOC WUNUSED void *DCALL
LOCAL__DeeDbgSlab_Malloc(char const *file, int line) {
	(void)file;
	(void)line;
	return LOCAL__DeeSlab_Malloc();
}

LOCAL_DECL ATTR_MALLOC WUNUSED void *DCALL
LOCAL__DeeDbgSlab_Calloc(char const *file, int line) {
	(void)file;
	(void)line;
	return LOCAL__DeeSlab_Calloc();
}

LOCAL_DECL ATTR_MALLOC WUNUSED void *DCALL
LOCAL__DeeDbgSlab_TryMalloc(char const *file, int line) {
	(void)file;
	(void)line;
	return LOCAL__DeeSlab_TryMalloc();
}

LOCAL_DECL ATTR_MALLOC WUNUSED void *DCALL
LOCAL__DeeDbgSlab_TryCalloc(char const *file, int line) {
	(void)file;
	(void)line;
	return LOCAL__DeeSlab_TryCalloc();
}
#endif /* !SLAB_DEBUG_LEAKS */

/* Implementation of slab malloc functions... */
#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_LOCAL_DeeSlab_Malloc
#include "slab-sized-alloc.c.inl"
#define DEFINE_LOCAL_DeeSlab_TryMalloc
#include "slab-sized-alloc.c.inl"
DECL_BEGIN

#undef LOCAL_DeeSlab_Malloc
#undef LOCAL_DeeSlab_Calloc
#undef LOCAL_DeeSlab_TryMalloc
#undef LOCAL_DeeSlab_TryCalloc
#undef LOCAL_DeeSlab_Malloc_DBG_PARAMS
#undef LOCAL_DeeSlab_Malloc_DBG_ARGS
#undef LOCAL_DeeSlab_Malloc_DBG_ARGS_PRESENT
#endif /* !__INTELLISENSE__ */




/* Select primary free implementation */
#if SLAB_DEBUG_LEAKS || SLAB_DEBUG_EXTERNAL
/* Because of how much non-debug slab allocators are used, also directly define them here */
#if !defined(__OPTIMIZE_SIZE__) && !defined(__INTELLISENSE__) && 1
#define LOCAL_DeeSlab_Free           LOCAL__DeeSlab_Free
#define LOCAL_DeeSlab_Free__DBG_PARAMS /* nothing */
#undef LOCAL_DeeSlab_Free__DBG_PARAMS_PRESENT
DECL_END
#include "slab-sized-free.c.inl"
DECL_BEGIN
#undef LOCAL_DeeSlab_Free
#undef LOCAL_DeeSlab_Free__DBG_PARAMS
#else /* !__OPTIMIZE_SIZE__ */
LOCAL_DECL NONNULL((1)) void DCALL
LOCAL__DeeSlab_Free(void *__restrict p) {
	LOCAL__DeeDbgSlab_Free(p, NULL, 0);
}
#endif /* __OPTIMIZE_SIZE__ */

#define LOCAL_DeeSlab_Free           LOCAL__DeeDbgSlab_Free
#define LOCAL_DeeSlab_Free__DBG_PARAMS , char const *file, int line
#define LOCAL_DeeSlab_Free__DBG_PARAMS_PRESENT
#else /* SLAB_DEBUG_LEAKS || SLAB_DEBUG_EXTERNAL */
#define LOCAL_DeeSlab_Free           LOCAL__DeeSlab_Free
#define LOCAL_DeeSlab_Free__DBG_PARAMS /* nothing */
#undef LOCAL_DeeSlab_Free__DBG_PARAMS_PRESENT

LOCAL_DECL NONNULL((1)) void DCALL
LOCAL__DeeDbgSlab_Free(void *__restrict p, char const *file, int line) {
	(void)file;
	(void)line;
	LOCAL__DeeSlab_Free(p);
}
#endif /* !SLAB_DEBUG_LEAKS && !SLAB_DEBUG_EXTERNAL */

#ifndef __INTELLISENSE__
DECL_END
#include "slab-sized-free.c.inl"
DECL_BEGIN
#undef LOCAL_DeeSlab_Free
#undef LOCAL_DeeSlab_Free__DBG_PARAMS
#undef LOCAL_DeeSlab_Free__DBG_PARAMS_PRESENT
#endif /* !__INTELLISENSE__ */



LOCAL_DECL void *DCALL
LOCAL_DeeDbgSlab_UntrackAlloc(void *p, char const *file, int line) {
	(void)file;
	(void)line;
#if SLAB_DEBUG_LEAKS
	if likely(p != NULL)
		p = dbg_slab__detach(p, DEFINE_CHUNK_SIZE, file, line);
#endif /* SLAB_DEBUG_LEAKS */
	return p;
}

#ifndef __INTELLISENSE__
#undef LOCAL_MAX_CHUNK_COUNT
#undef LOCAL_ELEMOF__sp_used
#undef LOCAL_SIZEOF__sp_data
#undef LOCAL_SIZEOF__sp_used
#undef LOCAL_BITSOF__sp_used
#undef LOCAL_SIZEOF__sp_pad

#undef LOCAL_slab_lock_reading
#undef LOCAL_slab_lock_writing
#undef LOCAL_slab_lock_tryread
#undef LOCAL_slab_lock_trywrite
#undef LOCAL_slab_lock_canread
#undef LOCAL_slab_lock_canwrite
#undef LOCAL_slab_lock_waitread
#undef LOCAL_slab_lock_waitwrite
#undef LOCAL_slab_lock_read
#undef LOCAL_slab_lock_write
#undef LOCAL_slab_lock_tryupgrade
#undef LOCAL_slab_lock_upgrade
#undef LOCAL_slab_lock_downgrade
#undef LOCAL_slab_lock_endwrite
#undef LOCAL_slab_lock_endread
#undef LOCAL_slab_lock_end
#endif /* !__INTELLISENSE__ */

DECL_END

#ifndef __INTELLISENSE__
#undef LOCAL_slab_page
#undef LOCAL_slab_page_list
#undef LOCAL_slab_pages
#undef LOCAL_slab_lock
#undef LOCAL_slab_malloc_in_page
#undef LOCAL__DeeSlab_Malloc
#undef LOCAL__DeeSlab_Calloc
#undef LOCAL__DeeSlab_TryMalloc
#undef LOCAL__DeeSlab_TryCalloc
#undef LOCAL__DeeSlab_Free
#undef LOCAL__DeeDbgSlab_Malloc
#undef LOCAL__DeeDbgSlab_Calloc
#undef LOCAL__DeeDbgSlab_TryMalloc
#undef LOCAL__DeeDbgSlab_TryCalloc
#undef LOCAL__DeeDbgSlab_Free
#undef LOCAL_DeeDbgSlab_UntrackAlloc
#undef LOCAL_DECL
#undef LOCAL_SYM

#undef DEFINE_CHUNK_SIZE
#endif /* !__INTELLISENSE__ */
