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
#ifndef GUARD_DEEMON_RUNTIME_SLAB_C
#define GUARD_DEEMON_RUNTIME_SLAB_C 1

#include <deemon/api.h>

#if defined(CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR) || defined(__DEEMON__)
#include <deemon/alloc.h>            /* DeeDbgSlab_*, DeeSlab_*, Dee_Free, Dee_Memalign, Dee_TryMemalign, Dee_UntrackAlloc */
#include <deemon/format.h>           /* PRFuSIZ, PRFxSIZ */
#include <deemon/gc.h>               /* DeeDbgGCSlab_*, DeeGCSlab_*, DeeGC_Head, DeeGC_Object, Dee_GC_OBJECT_OFFSET, Dee_gc_head */
#include <deemon/system-features.h>  /* memset */
#include <deemon/types.h>            /* DeeObject */
#include <deemon/util/slab-config.h> /* Dee_SLAB_* */
#include <deemon/util/slab.h>        /* Dee_OFFSET_SLAB_PAGE_META, Dee_SLAB_PAGESIZE, Dee_slab_page, Dee_slab_page_builder_offset_t */

#include <hybrid/align.h>    /* CEILDIV, IS_POWER_OF_TWO */
#include <hybrid/overflow.h> /* OVERFLOW_USUB */
#include <hybrid/typecore.h> /* __*_TYPE__, __CHAR_BIT__, __SIZEOF_INT_FASTn_T__, __SIZEOF_POINTER__, __UINTPTR_C, __UINTn_C */

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* UINT32_C, UINT64_C */

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#undef byte_t
#define byte_t __BYTE_TYPE__

#if defined(Dee_SLAB_CHUNKSIZE_MAX) || defined(__DEEMON__)
DECL_BEGIN

/* Do some sanity assertions on the slab configuration */
STATIC_ASSERT(IS_POWER_OF_TWO(Dee_SLAB_PAGESIZE));
STATIC_ASSERT(Dee_SLAB_PAGESIZE > Dee_SLAB_CHUNKSIZE_MAX);
STATIC_ASSERT(DeeSlab_EXISTS(Dee_SLAB_CHUNKSIZE_MIN));
STATIC_ASSERT(DeeSlab_EXISTS(Dee_SLAB_CHUNKSIZE_MAX));

/* Assert that all GC slabs exist when adjust for their actual size */
#define LOCAL_ASSERT_GC_SLAB_EXISTS(n, _) \
	STATIC_ASSERT(DeeSlab_EXISTS(n + Dee_GC_OBJECT_OFFSET));
Dee_SLAB_CHUNKSIZE_GC_FOREACH(LOCAL_ASSERT_GC_SLAB_EXISTS, ~)
#undef LOCAL_ASSERT_GC_SLAB_EXISTS


/* Implementation configuration */
#if !defined(NDEBUG) && 1
#define SLAB_DEBUG_INTERNAL 1
#else
#define SLAB_DEBUG_INTERNAL 0
#endif

#if !defined(NDEBUG) && 1
#define SLAB_DEBUG_EXTERNAL 1
#else
#define SLAB_DEBUG_EXTERNAL 0
#endif

#if __SIZEOF_POINTER__ > 4
#define WORD_4BYTE(x) UINT64_C(0x##x##x)
#else /* __SIZEOF_POINTER__ > 4 */
#define WORD_4BYTE(x) UINT32_C(0x##x)
#endif /*__SIZEOF_POINTER__ <= 4 */

#if SLAB_DEBUG_EXTERNAL
#define SLAB_DEBUG_MEMSET_ALLOC WORD_4BYTE(CACACACA) /* Set by DeeSlab_Malloc() */
#define SLAB_DEBUG_MEMSET_FREE  WORD_4BYTE(BAADF00D) /* Set by DeeSlab_Free() */
#endif /* SLAB_DEBUG_EXTERNAL */

#if SLAB_DEBUG_INTERNAL
#define slab_assert  Dee_ASSERT
#define slab_assertf Dee_ASSERTF
#elif !defined(__NO_builtin_assume)
#define slab_assert(expr)       __builtin_assume(expr)
#define slab_assertf(expr, ...) __builtin_assume(expr)
#else /* ... */
#define slab_assert(expr)       (void)0
#define slab_assertf(expr, ...) (void)0
#endif /* !... */


#ifdef SLAB_DEBUG_MEMSET_ALLOC
PRIVATE void DCALL slab_setalloc_data(void *p, size_t n) {
	size_t *iter = (size_t *)p;
	while (n >= sizeof(size_t)) {
		*iter++ = SLAB_DEBUG_MEMSET_ALLOC;
		n -= sizeof(size_t);
	}
}
#else /* SLAB_DEBUG_MEMSET_ALLOC */
#define slab_setalloc_data(p, n) (void)0
#endif /* !SLAB_DEBUG_MEMSET_ALLOC */

#ifdef SLAB_DEBUG_MEMSET_FREE
PRIVATE void DCALL slab_setfree_data(void *p, size_t n) {
	size_t *iter = (size_t *)p;
	while (n >= sizeof(size_t)) {
		*iter++ = SLAB_DEBUG_MEMSET_FREE;
		n -= sizeof(size_t);
	}
}

PRIVATE void DCALL slab_chkfree_data(void *p, size_t n) {
	size_t *iter = (size_t *)p;
	size_t rem = n;
	while (rem >= sizeof(size_t)) {
		size_t actual = *iter;
		ASSERTF(actual == SLAB_DEBUG_MEMSET_FREE,
		        "in %" PRFuSIZ "-sized slap chunk at %p: word at %p (offset: %" PRFuSIZ ") "
		        /**/ "was modified after it was freed. Expected %#" PRFxSIZ ", but got %#" PRFxSIZ,
		        n, p, iter, (size_t)((byte_t *)iter - (byte_t *)p),
		        (size_t)SLAB_DEBUG_MEMSET_FREE, actual);
		rem -= sizeof(size_t);
		++iter;
	}
}
#else /* SLAB_DEBUG_MEMSET_FREE */
#define slab_setfree_data(p, n) (void)0
#define slab_chkfree_data(p, n) (void)0
#endif /* !SLAB_DEBUG_MEMSET_FREE */



/* Low-level slab allocator functions. -- These functions imply have
 * to allocate/free "Dee_SLAB_PAGESIZE"-sized, and aligned chunks of
 * memory. -- Where that memory comes from, or the address ranges it
 * resides in doesn't matter. */
PRIVATE ATTR_MALLOC WUNUSED void *DCALL slab_page_malloc(void) {
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
	return Dee_UntrackAlloc(Dee_Memalign(Dee_SLAB_PAGESIZE, Dee_SLAB_PAGESIZE));
}

PRIVATE ATTR_MALLOC WUNUSED void *DCALL slab_page_trymalloc(void) {
	return Dee_UntrackAlloc(Dee_TryMemalign(Dee_SLAB_PAGESIZE, Dee_SLAB_PAGESIZE));
}

PRIVATE NONNULL((1)) void DCALL slab_page_free(void *page) {
	Dee_Free(page);
}




/* Debug debug versions of slab allocator functions */
#ifdef NDEBUG
#define dbg_slab__attach(p, n, file, line) (p) /* Attach debug info and either free, */
#define dbg_slab__detach(p, n, file, line) (p)
#else /* NDEBUG */
PRIVATE WUNUSED void *DCALL
dbg_slab__attach(void *p, size_t n, char const *file, int line) {
	if (p) {
		/* TODO: Attach debug info (for leaks and such...)
		 * On error, can free "p" which is guarantied to
		 * be an "n"-byte large slab, then return "NULL" */
		(void)n;
		(void)file;
		(void)line;
	}
	return p;
}
PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) void *DCALL
dbg_slab__detach(void *p, size_t n, char const *file, int line) {
	/* TODO: Free debug info if it exists for "p" (which is an "n"-byte large slab) */
	(void)n;
	(void)file;
	(void)line;
	return p;
}
#endif /* !NDEBUG */

#define DEFINE_DeeDbgSlab_API(n, _)                                      \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                               \
	DeeDbgSlab_Malloc##n(char const *file, int line) {                   \
		(void)file;                                                      \
		(void)line;                                                      \
		return dbg_slab__attach(DeeSlab_Malloc##n(), n, file, line);     \
	}                                                                    \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                               \
	DeeDbgSlab_Calloc##n(char const *file, int line) {                   \
		(void)file;                                                      \
		(void)line;                                                      \
		return dbg_slab__attach(DeeSlab_Calloc##n(), n, file, line);     \
	}                                                                    \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                               \
	DeeDbgSlab_TryMalloc##n(char const *file, int line) {                \
		(void)file;                                                      \
		(void)line;                                                      \
		return dbg_slab__attach(DeeSlab_TryMalloc##n(), n, file, line);  \
	}                                                                    \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                               \
	DeeDbgSlab_TryCalloc##n(char const *file, int line) {                \
		(void)file;                                                      \
		(void)line;                                                      \
		return dbg_slab__attach(DeeSlab_TryCalloc##n(), n, file, line);  \
	}                                                                    \
	PUBLIC void *DCALL                                                   \
	DeeDbgSlab_UntrackAlloc##n(void *p, char const *file, int line) {    \
		(void)file;                                                      \
		(void)line;                                                      \
		if (p)                                                           \
			p = dbg_slab__detach(p, n, file, line);                      \
		return p;                                                        \
	}                                                                    \
	PUBLIC NONNULL((1)) void DCALL                                       \
	DeeDbgSlab_Free##n(void *__restrict p, char const *file, int line) { \
		(void)file;                                                      \
		(void)line;                                                      \
		DeeSlab_Free##n(dbg_slab__detach(p, n, file, line));             \
	}
Dee_SLAB_CHUNKSIZE_FOREACH(DEFINE_DeeDbgSlab_API, ~)
#undef DEFINE_DeeDbgSlab_API



/* Define slab GC allocators (use "Dee_SLAB_CHUNKSIZE_GC_FOREACH()" to
 * enumerate and call-forward to the relevant, matching slab function) */
LOCAL void *gc_initob(void *ptr) {
	if likely(ptr) {
		DBG_memset(ptr, 0xcc, Dee_GC_OBJECT_OFFSET);
		ptr = DeeGC_Object((struct Dee_gc_head *)ptr);
	}
	return ptr;
}

#define call_gc_slab(N, f, args)                                                            \
	_Dee_PRIVATE_SLAB_SELECT(N + Dee_GC_OBJECT_OFFSET, , f, args,                           \
	                         ((int(*)[DeeSlab_EXISTS(N + Dee_GC_OBJECT_OFFSET) ? 1 : -1])0, \
	                          __builtin_unreachable(), NULL))
#define DEFINE_DeeGCSlab_API(n, _)                                                  \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                          \
	DeeGCSlab_Malloc##n(void) {                                                     \
		return gc_initob(call_gc_slab(n, DeeSlab_Malloc, ()));                      \
	}                                                                               \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                          \
	DeeGCSlab_Calloc##n(void) {                                                     \
		return gc_initob(call_gc_slab(n, DeeSlab_Calloc, ()));                      \
	}                                                                               \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                          \
	DeeGCSlab_TryMalloc##n(void) {                                                  \
		return gc_initob(call_gc_slab(n, DeeSlab_TryMalloc, ()));                   \
	}                                                                               \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                          \
	DeeGCSlab_TryCalloc##n(void) {                                                  \
		return gc_initob(call_gc_slab(n, DeeSlab_TryCalloc, ()));                   \
	}                                                                               \
	PUBLIC NONNULL((1)) void DCALL                                                  \
	DeeGCSlab_Free##n(void *__restrict p) {                                         \
		call_gc_slab(n, DeeSlab_Free, (DeeGC_Head((DeeObject *)p)));                \
	}                                                                               \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                          \
	DeeDbgGCSlab_Malloc##n(char const *file, int line) {                            \
		return gc_initob(call_gc_slab(n, DeeDbgSlab_Malloc, (file, line)));         \
	}                                                                               \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                          \
	DeeDbgGCSlab_Calloc##n(char const *file, int line) {                            \
		return gc_initob(call_gc_slab(n, DeeDbgSlab_Calloc, (file, line)));         \
	}                                                                               \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                          \
	DeeDbgGCSlab_TryMalloc##n(char const *file, int line) {                         \
		return gc_initob(call_gc_slab(n, DeeDbgSlab_TryMalloc, (file, line)));      \
	}                                                                               \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                          \
	DeeDbgGCSlab_TryCalloc##n(char const *file, int line) {                         \
		return gc_initob(call_gc_slab(n, DeeDbgSlab_TryCalloc, (file, line)));      \
	}                                                                               \
	PUBLIC void *DCALL                                                              \
	DeeDbgGCSlab_UntrackAlloc##n(void *p, char const *file, int line) {             \
		(void)file;                                                                 \
		(void)line;                                                                 \
		if (p) {                                                                    \
			(void)dbg_slab__detach(DeeGC_Head((DeeObject *)p),                      \
			                       n + Dee_GC_OBJECT_OFFSET,                        \
			                       file, line);                                     \
		}                                                                           \
		return p;                                                                   \
	}                                                                               \
	PUBLIC NONNULL((1)) void DCALL                                                  \
	DeeDbgGCSlab_Free##n(void *__restrict p, char const *file, int line) {          \
		call_gc_slab(n, DeeDbgSlab_Free, (DeeGC_Head((DeeObject *)p), file, line)); \
	}

Dee_SLAB_CHUNKSIZE_GC_FOREACH(DEFINE_DeeGCSlab_API, ~)
#undef DEFINE_DeeGCSlab_API


/* Select "bitword_t" data type. */
#if __SIZEOF_POINTER__ <= 1
#define bitword_t        __UINT_FAST8_TYPE__
#define BITWORD_C(v)     __UINT8_C(v)
#define SIZEOF_bitword_t __SIZEOF_INT_FAST8_T__
#elif __SIZEOF_POINTER__ <= 2
#define bitword_t        __UINT_FAST16_TYPE__
#define BITWORD_C(v)     __UINT16_C(v)
#define SIZEOF_bitword_t __SIZEOF_INT_FAST16_T__
#elif __SIZEOF_POINTER__ <= 4
#define bitword_t        __UINT_FAST32_TYPE__
#define BITWORD_C(v)     __UINT32_C(v)
#define SIZEOF_bitword_t __SIZEOF_INT_FAST32_T__
#elif __SIZEOF_POINTER__ <= 8
#define bitword_t        __UINT_FAST64_TYPE__
#define BITWORD_C(v)     __UINT64_C(v)
#define SIZEOF_bitword_t __SIZEOF_INT_FAST64_T__
#else /* __SIZEOF_POINTER__ <= ... */
#define bitword_t        __UINTPTR_TYPE__
#define BITWORD_C(v)     __UINTPTR_C(v)
#define SIZEOF_bitword_t __SIZEOF_POINTER__
#endif /* __SIZEOF_POINTER__ > ... */
#define BITSOF_bitword_t (SIZEOF_bitword_t * __CHAR_BIT__)


/* Calculate the # of chunks that fit into a slab page, given the size of those chunks.
 * Since the formula for this is self-dependent, we use 3 consecutive approximations,
 * which seems to produce optimial results in all situations. */
#define _UPPER__MAX_CHUNK_COUNT(n) (Dee_OFFSET_SLAB_PAGE_META / n)
#define _UPPER_ELEMOF__sp_used(n)  CEILDIV(_UPPER__MAX_CHUNK_COUNT(n), BITSOF_bitword_t)
#define _UPPER_SIZEOF__sp_used(n)  (_UPPER_ELEMOF__sp_used(n) * SIZEOF_bitword_t)
#define _LOWER_SIZEOF__sp_data(n)  (Dee_OFFSET_SLAB_PAGE_META - _UPPER_SIZEOF__sp_used(n))
#define _LOWER__MAX_CHUNK_COUNT(n) (_LOWER_SIZEOF__sp_data(n) / n)
#define _LOWER_ELEMOF__sp_used(n)  CEILDIV(_LOWER__MAX_CHUNK_COUNT(n), BITSOF_bitword_t)
#define _LOWER_SIZEOF__sp_used(n)  (_LOWER_ELEMOF__sp_used(n) * SIZEOF_bitword_t)
#define _LOWER_BITSOF__sp_used(n)  (_LOWER_ELEMOF__sp_used(n) * BITSOF_bitword_t)
#define _FINAL_SIZEOF__sp_data(n)  (Dee_OFFSET_SLAB_PAGE_META - _LOWER_SIZEOF__sp_used(n))
#define _MAX_CHUNK_COUNT(n)        (_FINAL_SIZEOF__sp_data(n) / n)
#define MAX_CHUNK_COUNT(n)        (_MAX_CHUNK_COUNT(n) > _LOWER_BITSOF__sp_used(n) ? _LOWER_BITSOF__sp_used(n) : _MAX_CHUNK_COUNT(n))


DECL_END

/* Slab implementations... */
#ifndef __INTELLISENSE__
/*[[[deemon
local minsize = 12;
local maxsize = 80;
print("#if Dee_SLAB_CHUNKSIZE_MIN < ", minsize);
print("#error \"Configured value 'Dee_SLAB_CHUNKSIZE_MIN' is less than supported minimum ", minsize, "\"");
print("#endif /" "* Dee_SLAB_CHUNKSIZE_MIN < ", minsize, " *" "/");
print("#if Dee_SLAB_CHUNKSIZE_MAX > ", maxsize);
print("#error \"Configured value 'Dee_SLAB_CHUNKSIZE_MAX' is greater than supported maximum ", maxsize, "\"");
print("#endif /" "* Dee_SLAB_CHUNKSIZE_MAX > ", maxsize, " *" "/");
for (local n: [minsize:maxsize+1]) {
	print("#if DeeSlab_EXISTS(", n, ")");
	print("#define DEFINE_CHUNK_SIZE ", n);
	print("#include \"slab-sized.c.inl\"");
	print("#endif /" "* DeeSlab_EXISTS(", n, ") *"  "/");
}
]]]*/
#if Dee_SLAB_CHUNKSIZE_MIN < 12
#error "Configured value 'Dee_SLAB_CHUNKSIZE_MIN' is less than supported minimum 12"
#endif /* Dee_SLAB_CHUNKSIZE_MIN < 12 */
#if Dee_SLAB_CHUNKSIZE_MAX > 80
#error "Configured value 'Dee_SLAB_CHUNKSIZE_MAX' is greater than supported maximum 80"
#endif /* Dee_SLAB_CHUNKSIZE_MAX > 80 */
#if DeeSlab_EXISTS(12)
#define DEFINE_CHUNK_SIZE 12
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(12) */
#if DeeSlab_EXISTS(13)
#define DEFINE_CHUNK_SIZE 13
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(13) */
#if DeeSlab_EXISTS(14)
#define DEFINE_CHUNK_SIZE 14
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(14) */
#if DeeSlab_EXISTS(15)
#define DEFINE_CHUNK_SIZE 15
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(15) */
#if DeeSlab_EXISTS(16)
#define DEFINE_CHUNK_SIZE 16
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(16) */
#if DeeSlab_EXISTS(17)
#define DEFINE_CHUNK_SIZE 17
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(17) */
#if DeeSlab_EXISTS(18)
#define DEFINE_CHUNK_SIZE 18
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(18) */
#if DeeSlab_EXISTS(19)
#define DEFINE_CHUNK_SIZE 19
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(19) */
#if DeeSlab_EXISTS(20)
#define DEFINE_CHUNK_SIZE 20
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(20) */
#if DeeSlab_EXISTS(21)
#define DEFINE_CHUNK_SIZE 21
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(21) */
#if DeeSlab_EXISTS(22)
#define DEFINE_CHUNK_SIZE 22
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(22) */
#if DeeSlab_EXISTS(23)
#define DEFINE_CHUNK_SIZE 23
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(23) */
#if DeeSlab_EXISTS(24)
#define DEFINE_CHUNK_SIZE 24
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(24) */
#if DeeSlab_EXISTS(25)
#define DEFINE_CHUNK_SIZE 25
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(25) */
#if DeeSlab_EXISTS(26)
#define DEFINE_CHUNK_SIZE 26
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(26) */
#if DeeSlab_EXISTS(27)
#define DEFINE_CHUNK_SIZE 27
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(27) */
#if DeeSlab_EXISTS(28)
#define DEFINE_CHUNK_SIZE 28
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(28) */
#if DeeSlab_EXISTS(29)
#define DEFINE_CHUNK_SIZE 29
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(29) */
#if DeeSlab_EXISTS(30)
#define DEFINE_CHUNK_SIZE 30
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(30) */
#if DeeSlab_EXISTS(31)
#define DEFINE_CHUNK_SIZE 31
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(31) */
#if DeeSlab_EXISTS(32)
#define DEFINE_CHUNK_SIZE 32
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(32) */
#if DeeSlab_EXISTS(33)
#define DEFINE_CHUNK_SIZE 33
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(33) */
#if DeeSlab_EXISTS(34)
#define DEFINE_CHUNK_SIZE 34
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(34) */
#if DeeSlab_EXISTS(35)
#define DEFINE_CHUNK_SIZE 35
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(35) */
#if DeeSlab_EXISTS(36)
#define DEFINE_CHUNK_SIZE 36
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(36) */
#if DeeSlab_EXISTS(37)
#define DEFINE_CHUNK_SIZE 37
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(37) */
#if DeeSlab_EXISTS(38)
#define DEFINE_CHUNK_SIZE 38
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(38) */
#if DeeSlab_EXISTS(39)
#define DEFINE_CHUNK_SIZE 39
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(39) */
#if DeeSlab_EXISTS(40)
#define DEFINE_CHUNK_SIZE 40
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(40) */
#if DeeSlab_EXISTS(41)
#define DEFINE_CHUNK_SIZE 41
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(41) */
#if DeeSlab_EXISTS(42)
#define DEFINE_CHUNK_SIZE 42
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(42) */
#if DeeSlab_EXISTS(43)
#define DEFINE_CHUNK_SIZE 43
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(43) */
#if DeeSlab_EXISTS(44)
#define DEFINE_CHUNK_SIZE 44
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(44) */
#if DeeSlab_EXISTS(45)
#define DEFINE_CHUNK_SIZE 45
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(45) */
#if DeeSlab_EXISTS(46)
#define DEFINE_CHUNK_SIZE 46
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(46) */
#if DeeSlab_EXISTS(47)
#define DEFINE_CHUNK_SIZE 47
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(47) */
#if DeeSlab_EXISTS(48)
#define DEFINE_CHUNK_SIZE 48
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(48) */
#if DeeSlab_EXISTS(49)
#define DEFINE_CHUNK_SIZE 49
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(49) */
#if DeeSlab_EXISTS(50)
#define DEFINE_CHUNK_SIZE 50
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(50) */
#if DeeSlab_EXISTS(51)
#define DEFINE_CHUNK_SIZE 51
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(51) */
#if DeeSlab_EXISTS(52)
#define DEFINE_CHUNK_SIZE 52
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(52) */
#if DeeSlab_EXISTS(53)
#define DEFINE_CHUNK_SIZE 53
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(53) */
#if DeeSlab_EXISTS(54)
#define DEFINE_CHUNK_SIZE 54
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(54) */
#if DeeSlab_EXISTS(55)
#define DEFINE_CHUNK_SIZE 55
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(55) */
#if DeeSlab_EXISTS(56)
#define DEFINE_CHUNK_SIZE 56
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(56) */
#if DeeSlab_EXISTS(57)
#define DEFINE_CHUNK_SIZE 57
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(57) */
#if DeeSlab_EXISTS(58)
#define DEFINE_CHUNK_SIZE 58
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(58) */
#if DeeSlab_EXISTS(59)
#define DEFINE_CHUNK_SIZE 59
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(59) */
#if DeeSlab_EXISTS(60)
#define DEFINE_CHUNK_SIZE 60
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(60) */
#if DeeSlab_EXISTS(61)
#define DEFINE_CHUNK_SIZE 61
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(61) */
#if DeeSlab_EXISTS(62)
#define DEFINE_CHUNK_SIZE 62
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(62) */
#if DeeSlab_EXISTS(63)
#define DEFINE_CHUNK_SIZE 63
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(63) */
#if DeeSlab_EXISTS(64)
#define DEFINE_CHUNK_SIZE 64
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(64) */
#if DeeSlab_EXISTS(65)
#define DEFINE_CHUNK_SIZE 65
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(65) */
#if DeeSlab_EXISTS(66)
#define DEFINE_CHUNK_SIZE 66
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(66) */
#if DeeSlab_EXISTS(67)
#define DEFINE_CHUNK_SIZE 67
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(67) */
#if DeeSlab_EXISTS(68)
#define DEFINE_CHUNK_SIZE 68
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(68) */
#if DeeSlab_EXISTS(69)
#define DEFINE_CHUNK_SIZE 69
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(69) */
#if DeeSlab_EXISTS(70)
#define DEFINE_CHUNK_SIZE 70
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(70) */
#if DeeSlab_EXISTS(71)
#define DEFINE_CHUNK_SIZE 71
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(71) */
#if DeeSlab_EXISTS(72)
#define DEFINE_CHUNK_SIZE 72
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(72) */
#if DeeSlab_EXISTS(73)
#define DEFINE_CHUNK_SIZE 73
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(73) */
#if DeeSlab_EXISTS(74)
#define DEFINE_CHUNK_SIZE 74
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(74) */
#if DeeSlab_EXISTS(75)
#define DEFINE_CHUNK_SIZE 75
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(75) */
#if DeeSlab_EXISTS(76)
#define DEFINE_CHUNK_SIZE 76
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(76) */
#if DeeSlab_EXISTS(77)
#define DEFINE_CHUNK_SIZE 77
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(77) */
#if DeeSlab_EXISTS(78)
#define DEFINE_CHUNK_SIZE 78
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(78) */
#if DeeSlab_EXISTS(79)
#define DEFINE_CHUNK_SIZE 79
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(79) */
#if DeeSlab_EXISTS(80)
#define DEFINE_CHUNK_SIZE 80
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(80) */
/*[[[end]]]*/
#endif /* !__INTELLISENSE__ */
#endif /* Dee_SLAB_CHUNKSIZE_MAX */

DECL_BEGIN

struct page_format {
	unsigned int pf__max_chunk_count; /* max # of chunks that may exist in the page */
	unsigned int pf__sizeof__sp_used; /* [== sizeof(sp_used) == SIZEOF_bitword_t * CEILDIV(pf__max_chunk_count, BITSOF_bitword_t)]
	                                   * aka "offsetof__sp_data" */
};
#define PAGE_FORMAT_INIT(n)                                                                           \
	{                                                                                                 \
		/* .pf__max_chunk_count = */ MAX_CHUNK_COUNT(n),                                              \
		/* .pf__sizeof__sp_used = */ SIZEOF_bitword_t * CEILDIV(MAX_CHUNK_COUNT(n), BITSOF_bitword_t) \
	}


PRIVATE ATTR_RETNONNULL ATTR_CONST WUNUSED
struct page_format const *DCALL get_page_format(size_t n) {
	ASSERTF(DeeSlab_EXISTS(n), "Invalid size slab: %" PRFuSIZ, n);
	switch (n) {
#define DEFINE_FORMAT(n, _)                                       \
	case n: {                                                     \
		static struct page_format const pf = PAGE_FORMAT_INIT(n); \
		return &pf;                                               \
	}	break;
	Dee_SLAB_CHUNKSIZE_FOREACH(DEFINE_FORMAT, ~)
#undef PAGE_FORMAT_INIT
	default: __builtin_unreachable();
	}
	__builtin_unreachable();
}


/* Allocate/free custom chunks within a custom slab-page.
 * Also note that using these builder functions, you can
 * even mix-and-match chunks of different sizes, and it
 * will just work (though you have to remember which chunk
 * has which size, since the chunk's size is always needed
 * in order to safely free that chunk)
 *
 * NOTE: These function intentionally do *NOT* assert alignment of "self",
 *       meaning you can use these functions to build a custom slab page
 *       in unaligned memory, before simply memcpy-ing the page into a new
 *       memory location (that is probably) aligned, and keep working with
 *       it without any other relocation-work needed (assuming that payload
 *       area being moved don't require relocations).
 *
 * WARNING: The caller of these functions is responsible to ensure that
 *          `DeeSlab_EXISTS(n)' (or `DeeGCSlab_EXISTS(n - Dee_GC_OBJECT_OFFSET)')
 *          This requirement is asserted internally, so you'll get an assert
 *          failure if you don't comply with this requirement!
 *
 * @return: * :   Pointer into `self->sp_data' to an n-byte payload area
 * @return: NULL: Insufficient memory -- given slab page "self" does not
 *                have space for another "n"-byte large slab. (you should
 *                probably allocate another ) */
PUBLIC WUNUSED NONNULL((1)) void *DCALL
Dee_slab_page_buildmalloc(struct Dee_slab_page *__restrict self, size_t n) {
	struct page_format const *fmt = get_page_format(n);
	bitword_t *self__sp_used = (bitword_t *)self;
	Dee_slab_page_builder_offset_t lo_offset = self->sp_meta.spm_type.t_builder.spb_unused_lo;
	Dee_slab_page_builder_offset_t hi_offset = self->sp_meta.spm_type.t_builder.spb_unused_hi;
	if (lo_offset < fmt->pf__sizeof__sp_used)
		lo_offset = fmt->pf__sizeof__sp_used;

	/* Align "hi_offset" */
	if (OVERFLOW_USUB(hi_offset, fmt->pf__sizeof__sp_used, &hi_offset))
		goto fail;
	lo_offset -= fmt->pf__sizeof__sp_used;
	hi_offset = (hi_offset / n) * n; /* floor-align to nearest, valid slab-chunk */
	for (;;) {
		size_t bitno, bit_indx;
		bitword_t bit_mask;
		if (hi_offset <= lo_offset)
			goto fail;
		hi_offset -= n; /* Allocate memory */
		bitno = hi_offset / n;
		slab_assert(bitno < fmt->pf__max_chunk_count);
		bit_indx = bitno / BITSOF_bitword_t;
		bit_mask = BITWORD_C(1) << (bitno % BITSOF_bitword_t);
		if (self__sp_used[bit_indx] & bit_mask)
			continue; /* Conflict with some other allocation -> cannot use this position (try the next one) */

		/* Mark location as allocated */
		self__sp_used[bit_indx] |= bit_mask;
		break;
	}

	/* Adjust offsets to be independent of "sizeof(sp_used)" */
	lo_offset += fmt->pf__sizeof__sp_used;
	hi_offset += fmt->pf__sizeof__sp_used;

	/* Remember offsets of new unused-area (the unused-area can only ever shrink btw) */
	slab_assert(self->sp_meta.spm_type.t_builder.spb_unused_lo <= lo_offset);
	slab_assert(self->sp_meta.spm_type.t_builder.spb_unused_hi >= hi_offset);
	self->sp_meta.spm_type.t_builder.spb_unused_lo = lo_offset;
	self->sp_meta.spm_type.t_builder.spb_unused_hi = hi_offset;
	++self->sp_meta.spm_used;

	/* Freshly allocated location is at the far end of the (now smaller) free-area. */
	return (byte_t *)self + hi_offset;
fail:
	return NULL;
}

PUBLIC NONNULL((1, 2)) void DCALL
Dee_slab_page_buildfree(struct Dee_slab_page *self, void *p, size_t n) {
	bitword_t *self__sp_used = (bitword_t *)self;
	struct page_format const *fmt = get_page_format(n);
	Dee_slab_page_builder_offset_t offsetof__p_from_self    = (size_t)((byte_t *)p - (byte_t *)self);
	Dee_slab_page_builder_offset_t offsetof__p_from_sp_data = offsetof__p_from_self - fmt->pf__sizeof__sp_used;
	size_t bitno_of__p_in_sp_used = offsetof__p_from_sp_data / n;
	size_t indx_of__p_in_sp_used;
	bitword_t word_of__p_in_sp_used;
	ASSERTF(offsetof__p_from_self >= fmt->pf__sizeof__sp_used,
	        "Given 'p' is too close to the start of the page for this chunk-size");
	ASSERTF(offsetof__p_from_self >= self->sp_meta.spm_type.t_builder.spb_unused_hi,
	        "Given 'p' is not in allocated area of page");
	ASSERTF((offsetof__p_from_sp_data % n) == 0,
	        "Given 'p' is incorrectly aligned for this chunk-size");
	ASSERTF(bitno_of__p_in_sp_used < fmt->pf__max_chunk_count,
	        "Given 'p' is too close to the end of the page for this chunk-size");

	indx_of__p_in_sp_used = bitno_of__p_in_sp_used / BITSOF_bitword_t;
	word_of__p_in_sp_used = BITWORD_C(1) << (bitno_of__p_in_sp_used % BITSOF_bitword_t);
	ASSERTF(self__sp_used[indx_of__p_in_sp_used] & word_of__p_in_sp_used,
	        "Given 'p' is not marked as allocated within this page");

	/* Mark the location as not-allocated within the slab's in-use bitset. */
	self__sp_used[indx_of__p_in_sp_used] &= ~word_of__p_in_sp_used;
	--self->sp_meta.spm_used;

	/* If the given 'p' was most-recently allocated from 'self',
	 * we can give back memory to the page's unused area. */
	if (self->sp_meta.spm_type.t_builder.spb_unused_hi == offsetof__p_from_self)
		self->sp_meta.spm_type.t_builder.spb_unused_hi += n;
}

DECL_END

#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

#endif /* !GUARD_DEEMON_RUNTIME_SLAB_C */
