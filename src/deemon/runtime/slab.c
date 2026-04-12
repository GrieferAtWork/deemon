/*[[[vs
ClCompile.Optimization = MaxSpeed
ClCompile.InlineFunctionExpansion = AnySuitable
ClCompile.FavorSizeOrSpeed = Speed
ClCompile.OmitFramePointers = true
ClCompile.BasicRuntimeChecks = Default
]]]*/
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

/* Implementation configuration */
#if !defined(NDEBUG) && !defined(__OPTIMIZE_SIZE__) && 0
#define SLAB_DEBUG_INTERNAL 1
#else
#define SLAB_DEBUG_INTERNAL 0
#endif

#if !defined(NDEBUG) && !defined(__OPTIMIZE_SIZE__) && 1
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

/* Config option: enable support for detecting memory leaks caused by slabs */
#define SLAB_DEBUG_LEAKS (SLAB_DEBUG_EXTERNAL && 1)

/* Config option: keep track of slab pages that have been fully allocated
 *                (in a separate per-slab list). This is needed for leak
 *                detection to find those pages if at application exit some
 *                slab page ends up going entirely unused. */
#define SLAB_TRACK_FULL_PAGES SLAB_DEBUG_LEAKS



/* Disable NONNULL attributes -- needed so debug NULL-checks in DeeDbgSlab_Free() work */
#if SLAB_DEBUG_EXTERNAL
#undef NONNULL
#define NONNULL(x)
#endif /* SLAB_DEBUG_EXTERNAL */


#include <deemon/alloc.h>            /* DeeDbgSlab_*, DeeSlab_*, Dee_Free, Dee_Mallococ, Dee_TryMallococ, Dee_UntrackAlloc */
#include <deemon/format.h>           /* PRF* */
#include <deemon/gc.h>               /* DeeDbgGCSlab_*, DeeGCSlab_*, DeeGC_Head, DeeGC_Object, Dee_GC_OBJECT_OFFSET, Dee_gc_head */
#include <deemon/system-features.h>  /* memset */
#include <deemon/types.h>            /* DeeObject, Dee_ssize_t */
#include <deemon/util/atomic.h>      /* atomic_read */
#include <deemon/util/lock.h>        /* Dee_atomic_rwlock_* */
#include <deemon/util/slab-config.h> /* Dee_SLAB_*, _Dee_PRIVATE_SLAB_SELECT */
#include <deemon/util/slab.h>        /* Dee_SIZEOF_SLAB_PAGE_META, Dee_SLAB_PAGESIZE, Dee_slab_page, Dee_slab_page_* */

#include <hybrid/align.h>         /* CEILDIV, IS_ALIGNED, IS_POWER_OF_TWO */
#include <hybrid/overflow.h>      /* OVERFLOW_USUB */
#include <hybrid/sequence/list.h> /* LIST_* */
#include <hybrid/typecore.h>      /* __BYTE_TYPE__, __SHIFT_TYPE__, __SIZEOF_POINTER__ */

#include "slab.h"

#include <stddef.h> /* NULL, offsetof, size_t */
#include <stdint.h> /* UINT32_C, UINT64_C, uintptr_t */

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#undef container_of
#define container_of COMPILER_CONTAINER_OF

#undef byte_t
#define byte_t __BYTE_TYPE__
#undef shift_t
#define shift_t __SHIFT_TYPE__

/* Always enable optimizations for this file... */
#ifdef _MSC_VER
#pragma optimize("ts", on)
#endif /* _MSC_VER */

#if (defined(Dee_SLAB_CHUNKSIZE_MIN) != defined(Dee_SLAB_CHUNKSIZE_MAX))
#error "Either both 'Dee_SLAB_CHUNKSIZE_MIN' and 'Dee_SLAB_CHUNKSIZE_MAX' must be defined, or neither must be"
#endif /* ... */

#if defined(Dee_SLAB_CHUNKSIZE_MAX) || defined(__DEEMON__)
DECL_BEGIN

/* Do some sanity assertions on the slab configuration */
STATIC_ASSERT(IS_POWER_OF_TWO(Dee_SLAB_PAGESIZE));
STATIC_ASSERT_MSG(Dee_SLAB_PAGESIZE >= (SIZEOF_slab_bitword_t +
                                        Dee_SLAB_CHUNKSIZE_MAX +
                                        Dee_SIZEOF_SLAB_PAGE_META),
                  "Slab pages are too small to hold even a single "
                  "max-sized slab chunk when accounting for metadata");
STATIC_ASSERT(DeeSlab_EXISTS(Dee_SLAB_CHUNKSIZE_MIN));
STATIC_ASSERT(DeeSlab_EXISTS(Dee_SLAB_CHUNKSIZE_MAX));
STATIC_ASSERT(Dee_SIZEOF_SLAB_PAGE_META == sizeof(((struct Dee_slab_page *)0)->sp_meta));

/* Assert that all GC slabs exist when adjust for their actual size */
#define LOCAL_ASSERT_GC_SLAB_EXISTS(n, _) \
	STATIC_ASSERT(DeeSlab_EXISTS(n + Dee_GC_OBJECT_OFFSET));
Dee_SLAB_CHUNKSIZE_GC_FOREACH(LOCAL_ASSERT_GC_SLAB_EXISTS, ~)
#undef LOCAL_ASSERT_GC_SLAB_EXISTS


#ifdef SLAB_DEBUG_MEMSET_ALLOC
LOCAL void DCALL slab_setalloc_data(void *p, size_t n) {
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
LOCAL void DCALL slab_setfree_data(void *p, size_t n) {
	size_t *iter = (size_t *)p;
	while (n >= sizeof(size_t)) {
		*iter++ = SLAB_DEBUG_MEMSET_FREE;
		n -= sizeof(size_t);
	}
}

PRIVATE void DCALL slab_chkfree_data(void const *p, size_t n) {
	size_t const *iter = (size_t const *)p;
	size_t rem = n;
	while (rem >= sizeof(size_t)) {
		size_t actual = *iter;
		ASSERTF(actual == SLAB_DEBUG_MEMSET_FREE,
		        "in %" PRFuSIZ "-sized slab chunk at %p: word at %p (offset: %" PRFuSIZ ") "
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


/* Debug debug versions of slab allocator functions */
#if !SLAB_DEBUG_LEAKS
#define dbg_slab_page_rawmalloc(cs, cc)    Dee_slab_page_rawmalloc()
#define dbg_slab_page_rawtrymalloc(cs, cc) Dee_slab_page_rawtrymalloc()
#define dbg_slab_page_rawfree(p, cs, cc)   Dee_slab_page_rawfree(p)
#define dbg_slab__attach(page, p, n, i, file, line) (p) /* Attach debug info and either free, */
#define dbg_slab__detach(page, p, n, i, file, line) (p)
#else /* !SLAB_DEBUG_LEAKS */

/* @param: cs: ChunkSize
 * @param: cc: ChunkCount */
#define dbg_slab_page_rawmalloc dbg_slab_page_rawmalloc
PRIVATE ATTR_NOINLINE ATTR_MALLOC WUNUSED ATTR_ASSUME_ALIGNED(Dee_SLAB_PAGESIZE)
struct Dee_slab_page *DCALL dbg_slab_page_rawmalloc(size_t cs, size_t cc) {
	struct pageleaks *li;
	struct Dee_slab_page *result;
	result = Dee_slab_page_rawmalloc();
	if unlikely(!result)
		goto err;
	li = (struct pageleaks *)Dee_UntrackAlloc(Dee_Mallococ(offsetof(struct pageleaks, pl_chunks),
	                                                       cc, sizeof(struct chunkleak)));
	if unlikely(!li)
		goto err_r;
	result->sp_meta.spm_leak = li;
	li->pl_chnksiz = cs;
	li->pl_chnkcnt = cc;
	return result;
err_r:
	Dee_slab_page_rawfree(result);
err:
	return NULL;
}

#define dbg_slab_page_rawtrymalloc dbg_slab_page_rawtrymalloc
PRIVATE ATTR_NOINLINE ATTR_MALLOC WUNUSED ATTR_ASSUME_ALIGNED(Dee_SLAB_PAGESIZE)
struct Dee_slab_page *DCALL dbg_slab_page_rawtrymalloc(size_t cs, size_t cc) {
	struct pageleaks *pl;
	struct Dee_slab_page *result;
	result = Dee_slab_page_rawtrymalloc();
	if unlikely(!result)
		goto err;
	pl = (struct pageleaks *)Dee_UntrackAlloc(Dee_TryMallococ(offsetof(struct pageleaks, pl_chunks),
	                                                          cc, sizeof(struct chunkleak)));
	if unlikely(!pl)
		goto err_r;
	result->sp_meta.spm_leak = pl;
	pl->pl_chnksiz = cs;
	pl->pl_chnkcnt = cc;
	return result;
err_r:
	Dee_slab_page_rawfree(result);
err:
	return NULL;
}

#define dbg_slab_page_rawfree dbg_slab_page_rawfree
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
dbg_slab_page_rawfree(struct Dee_slab_page *__restrict page, size_t cs, size_t cc) {
	struct pageleaks *pl = (struct pageleaks *)page->sp_meta.spm_leak;
	slab_assert(pl);
	slab_assert(pl->pl_chnksiz == cs);
	slab_assert(pl->pl_chnkcnt == cc);
	(void)cs;
	(void)cc;
	Dee_Free(pl);
	Dee_slab_page_rawfree(page);
}


/* Attach debug info (for leaks and such...)
 *
 * On error, can free "p" which is guarantied to
 * be an "n"-byte large slab, then return "NULL" */
PRIVATE WUNUSED NONNULL((1)) void *DCALL
dbg_slab__attach(struct Dee_slab_page *page, void *p,
                 size_t n, size_t i, char const *file, int line) {
	struct pageleaks *pl = (struct pageleaks *)page->sp_meta.spm_leak;
	if unlikely(line < 0)
		line = 0; /* Negative values have special meaning */
	slab_assert(pl);
	slab_assert(pl->pl_chnksiz == n);
	slab_assert(i < pl->pl_chnkcnt);
	(void)n;
	pl->pl_chunks[i].cl_file = file;
	pl->pl_chunks[i].cl_line = line;
	return p;
}

/* Free debug info if it exists for "p" (which is an "n"-byte large slab) */
PRIVATE ATTR_RETNONNULL NONNULL((1)) void *DCALL
dbg_slab__detach(struct Dee_slab_page *page, void *p,
                 size_t n, size_t i, char const *file, int line) {
	struct pageleaks *pl = (struct pageleaks *)page->sp_meta.spm_leak;
	slab_assert(pl);
	slab_assert(pl->pl_chnksiz == n);
	slab_assert(i < pl->pl_chnkcnt);
	(void)pl;
	(void)i;
	(void)n;
	(void)file;
	(void)line;
	pl->pl_chunks[i].cl_file = NULL;
	pl->pl_chunks[i].cl_line = -1;
	return p;
}
#endif /* SLAB_DEBUG_LEAKS */


/* Define slab GC allocators (use "Dee_SLAB_CHUNKSIZE_GC_FOREACH()" to
 * enumerate and call-forward to the relevant, matching slab function) */
LOCAL void *gc_initob(void *ptr) {
	if likely(ptr) {
		DBG_memset(ptr, 0xcc, Dee_GC_OBJECT_OFFSET);
		ptr = DeeGC_Object((struct Dee_gc_head *)ptr);
	}
	return ptr;
}

#define call_gc_slab(N, f, args) \
	_Dee_PRIVATE_SLAB_SELECT(N + Dee_GC_OBJECT_OFFSET, , f, args, (__builtin_unreachable(), NULL))
#define DEFINE_DeeGCSlab_API(n, _)                                                  \
	STATIC_ASSERT(DeeSlab_EXISTS(n + Dee_GC_OBJECT_OFFSET));                        \
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
		return call_gc_slab(n, DeeDbgSlab_UntrackAlloc, (p, file, line));           \
	}                                                                               \
	PUBLIC NONNULL((1)) void DCALL                                                  \
	DeeDbgGCSlab_Free##n(void *__restrict p, char const *file, int line) {          \
		call_gc_slab(n, DeeDbgSlab_Free, (DeeGC_Head((DeeObject *)p), file, line)); \
	}

Dee_SLAB_CHUNKSIZE_GC_FOREACH(DEFINE_DeeGCSlab_API, ~)
#undef DEFINE_DeeGCSlab_API

DECL_END

/* Slab implementations... */
#ifndef __INTELLISENSE__
/*[[[deemon
local minsize = 12;
local maxsize = 128;
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
#if Dee_SLAB_CHUNKSIZE_MAX > 128
#error "Configured value 'Dee_SLAB_CHUNKSIZE_MAX' is greater than supported maximum 128"
#endif /* Dee_SLAB_CHUNKSIZE_MAX > 128 */
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
#if DeeSlab_EXISTS(81)
#define DEFINE_CHUNK_SIZE 81
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(81) */
#if DeeSlab_EXISTS(82)
#define DEFINE_CHUNK_SIZE 82
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(82) */
#if DeeSlab_EXISTS(83)
#define DEFINE_CHUNK_SIZE 83
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(83) */
#if DeeSlab_EXISTS(84)
#define DEFINE_CHUNK_SIZE 84
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(84) */
#if DeeSlab_EXISTS(85)
#define DEFINE_CHUNK_SIZE 85
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(85) */
#if DeeSlab_EXISTS(86)
#define DEFINE_CHUNK_SIZE 86
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(86) */
#if DeeSlab_EXISTS(87)
#define DEFINE_CHUNK_SIZE 87
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(87) */
#if DeeSlab_EXISTS(88)
#define DEFINE_CHUNK_SIZE 88
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(88) */
#if DeeSlab_EXISTS(89)
#define DEFINE_CHUNK_SIZE 89
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(89) */
#if DeeSlab_EXISTS(90)
#define DEFINE_CHUNK_SIZE 90
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(90) */
#if DeeSlab_EXISTS(91)
#define DEFINE_CHUNK_SIZE 91
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(91) */
#if DeeSlab_EXISTS(92)
#define DEFINE_CHUNK_SIZE 92
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(92) */
#if DeeSlab_EXISTS(93)
#define DEFINE_CHUNK_SIZE 93
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(93) */
#if DeeSlab_EXISTS(94)
#define DEFINE_CHUNK_SIZE 94
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(94) */
#if DeeSlab_EXISTS(95)
#define DEFINE_CHUNK_SIZE 95
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(95) */
#if DeeSlab_EXISTS(96)
#define DEFINE_CHUNK_SIZE 96
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(96) */
#if DeeSlab_EXISTS(97)
#define DEFINE_CHUNK_SIZE 97
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(97) */
#if DeeSlab_EXISTS(98)
#define DEFINE_CHUNK_SIZE 98
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(98) */
#if DeeSlab_EXISTS(99)
#define DEFINE_CHUNK_SIZE 99
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(99) */
#if DeeSlab_EXISTS(100)
#define DEFINE_CHUNK_SIZE 100
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(100) */
#if DeeSlab_EXISTS(101)
#define DEFINE_CHUNK_SIZE 101
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(101) */
#if DeeSlab_EXISTS(102)
#define DEFINE_CHUNK_SIZE 102
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(102) */
#if DeeSlab_EXISTS(103)
#define DEFINE_CHUNK_SIZE 103
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(103) */
#if DeeSlab_EXISTS(104)
#define DEFINE_CHUNK_SIZE 104
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(104) */
#if DeeSlab_EXISTS(105)
#define DEFINE_CHUNK_SIZE 105
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(105) */
#if DeeSlab_EXISTS(106)
#define DEFINE_CHUNK_SIZE 106
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(106) */
#if DeeSlab_EXISTS(107)
#define DEFINE_CHUNK_SIZE 107
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(107) */
#if DeeSlab_EXISTS(108)
#define DEFINE_CHUNK_SIZE 108
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(108) */
#if DeeSlab_EXISTS(109)
#define DEFINE_CHUNK_SIZE 109
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(109) */
#if DeeSlab_EXISTS(110)
#define DEFINE_CHUNK_SIZE 110
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(110) */
#if DeeSlab_EXISTS(111)
#define DEFINE_CHUNK_SIZE 111
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(111) */
#if DeeSlab_EXISTS(112)
#define DEFINE_CHUNK_SIZE 112
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(112) */
#if DeeSlab_EXISTS(113)
#define DEFINE_CHUNK_SIZE 113
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(113) */
#if DeeSlab_EXISTS(114)
#define DEFINE_CHUNK_SIZE 114
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(114) */
#if DeeSlab_EXISTS(115)
#define DEFINE_CHUNK_SIZE 115
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(115) */
#if DeeSlab_EXISTS(116)
#define DEFINE_CHUNK_SIZE 116
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(116) */
#if DeeSlab_EXISTS(117)
#define DEFINE_CHUNK_SIZE 117
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(117) */
#if DeeSlab_EXISTS(118)
#define DEFINE_CHUNK_SIZE 118
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(118) */
#if DeeSlab_EXISTS(119)
#define DEFINE_CHUNK_SIZE 119
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(119) */
#if DeeSlab_EXISTS(120)
#define DEFINE_CHUNK_SIZE 120
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(120) */
#if DeeSlab_EXISTS(121)
#define DEFINE_CHUNK_SIZE 121
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(121) */
#if DeeSlab_EXISTS(122)
#define DEFINE_CHUNK_SIZE 122
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(122) */
#if DeeSlab_EXISTS(123)
#define DEFINE_CHUNK_SIZE 123
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(123) */
#if DeeSlab_EXISTS(124)
#define DEFINE_CHUNK_SIZE 124
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(124) */
#if DeeSlab_EXISTS(125)
#define DEFINE_CHUNK_SIZE 125
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(125) */
#if DeeSlab_EXISTS(126)
#define DEFINE_CHUNK_SIZE 126
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(126) */
#if DeeSlab_EXISTS(127)
#define DEFINE_CHUNK_SIZE 127
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(127) */
#if DeeSlab_EXISTS(128)
#define DEFINE_CHUNK_SIZE 128
#include "slab-sized.c.inl"
#endif /* DeeSlab_EXISTS(128) */
/*[[[end]]]*/
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

struct page_format {
	Dee_slab_page_builder_offset_t pf__max_chunk_count; /* max # of chunks that may exist in the page */
	Dee_slab_page_builder_offset_t pf__sizeof__sp_used; /* [== sizeof(sp_used) == SIZEOF_slab_bitword_t * CEILDIV(pf__max_chunk_count, BITSOF_slab_bitword_t)]
	                                                     * aka "offsetof__sp_data" */
};
#define PAGE_FORMAT_INIT(n)                                                                           \
	{                                                                                                 \
		/* .pf__max_chunk_count = */ MAX_CHUNK_COUNT(n),                                              \
		/* .pf__sizeof__sp_used = */ SIZEOF_slab_bitword_t * CEILDIV(MAX_CHUNK_COUNT(n), BITSOF_slab_bitword_t) \
	}


#define LOCAL_DEFINE_SLAB_FORMAT(n, _) \
	PRIVATE struct page_format const slab_format##n = PAGE_FORMAT_INIT(n);
Dee_SLAB_CHUNKSIZE_FOREACH(LOCAL_DEFINE_SLAB_FORMAT, ~)
#undef LOCAL_DEFINE_SLAB_FORMAT

PRIVATE ATTR_RETNONNULL ATTR_CONST WUNUSED
struct page_format const *DCALL get_page_format(size_t n) {
	ASSERTF(DeeSlab_EXISTS(n), "Invalid size slab: %" PRFuSIZ, n);
	switch (n) {
#define RETURN_FORMAT(n, _) case n: return &slab_format##n;
	Dee_SLAB_CHUNKSIZE_FOREACH(RETURN_FORMAT, ~)
#undef RETURN_FORMAT
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
	slab_bitword_t *self__sp_used = (slab_bitword_t *)self;
	Dee_slab_page_builder_offset_t lo_offset = self->sp_meta.spm_type.t_builder.spb_unused_lo;
	Dee_slab_page_builder_offset_t hi_offset = self->sp_meta.spm_type.t_builder.spb_unused_hi;
	if (lo_offset < fmt->pf__sizeof__sp_used)
		lo_offset = fmt->pf__sizeof__sp_used;

	/* Align "hi_offset" */
	if (OVERFLOW_USUB(hi_offset, fmt->pf__sizeof__sp_used, &hi_offset))
		goto fail;
	lo_offset -= fmt->pf__sizeof__sp_used;
	/* floor-align to nearest, valid slab-chunk */
	hi_offset = (hi_offset / (Dee_slab_page_builder_offset_t)n) * (Dee_slab_page_builder_offset_t)n;
	for (;;) {
		size_t bitno, bit_indx;
		slab_bitword_t bit_mask;
		if (hi_offset <= lo_offset)
			goto fail;
		hi_offset -= (Dee_slab_page_builder_offset_t)n; /* Allocate memory */
		bitno = hi_offset / (Dee_slab_page_builder_offset_t)n;
		slab_assert(bitno < fmt->pf__max_chunk_count);
		bit_indx = slab_bitword_indx(bitno);
		bit_mask = slab_bitword_mask(bitno);
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
	slab_bitword_t *self__sp_used = (slab_bitword_t *)self;
	struct page_format const *fmt = get_page_format(n);
	Dee_slab_page_builder_offset_t offsetof__p_from_self    = (Dee_slab_page_builder_offset_t)((byte_t *)p - (byte_t *)self);
	Dee_slab_page_builder_offset_t offsetof__p_from_sp_data = offsetof__p_from_self - fmt->pf__sizeof__sp_used;
	size_t bitno_of__p_in_sp_used = offsetof__p_from_sp_data / (Dee_slab_page_builder_offset_t)n;
	size_t indx_of__p_in_sp_used;
	slab_bitword_t word_of__p_in_sp_used;
	ASSERTF(offsetof__p_from_self >= fmt->pf__sizeof__sp_used,
	        "Given 'p' is too close to the start of the page for this chunk-size");
	ASSERTF(offsetof__p_from_self >= self->sp_meta.spm_type.t_builder.spb_unused_hi,
	        "Given 'p' is not in allocated area of page");
	ASSERTF((offsetof__p_from_sp_data % (Dee_slab_page_builder_offset_t)n) == 0,
	        "Given 'p' is incorrectly aligned for this chunk-size");
	ASSERTF(bitno_of__p_in_sp_used < fmt->pf__max_chunk_count,
	        "Given 'p' is too close to the end of the page for this chunk-size");

	indx_of__p_in_sp_used = slab_bitword_indx(bitno_of__p_in_sp_used);
	word_of__p_in_sp_used = slab_bitword_mask(bitno_of__p_in_sp_used);
	ASSERTF(self__sp_used[indx_of__p_in_sp_used] & word_of__p_in_sp_used,
	        "Given 'p' is not marked as allocated within this page");
	ASSERTF(self->sp_meta.spm_used, "Nothing marked as allocated");

	/* Mark the location as not-allocated within the slab's in-use bitset. */
	self__sp_used[indx_of__p_in_sp_used] &= ~word_of__p_in_sp_used;
	--self->sp_meta.spm_used;

	/* If the given 'p' was most-recently allocated from 'self',
	 * we can give back memory to the page's unused area. */
	if (self->sp_meta.spm_type.t_builder.spb_unused_hi == offsetof__p_from_self)
		self->sp_meta.spm_type.t_builder.spb_unused_hi += (Dee_slab_page_builder_offset_t)n;
}



/************************************************************************/
/* DEBUG VERIFICATION                                                   */
/************************************************************************/
#if SLAB_DEBUG_EXTERNAL
#define _SLAB_COUNT_CB(n, _) +1
#define SLAB_COUNT (0 Dee_SLAB_CHUNKSIZE_FOREACH(_SLAB_COUNT_CB, ~))

LIST_HEAD(Dee_slab_page_list, Dee_slab_page);
struct slab_spec {
	struct pagespecs           const ss_specs;     /* Slab page specs */
	struct Dee_slab_page_list *const ss_pages;     /* [1..1] LOCAL_slab_pages */
#if SLAB_TRACK_FULL_PAGES
	struct Dee_slab_page_list *const ss_fullpages; /* [1..1] LOCAL_slab_fullpages */
#endif /* SLAB_TRACK_FULL_PAGES */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t       *const ss_lock;      /* [1..1] LOCAL_slab_lock */
#endif /* !CONFIG_NO_THREADS */
};

PRIVATE struct slab_spec tpconst slab_specs[SLAB_COUNT] = {
#if SLAB_TRACK_FULL_PAGES
#define _SLAB_SPECS_INIT__FULLPAGES(n) , (struct Dee_slab_page_list *)&PP_CAT2(slab_fullpages, n)
#else /* SLAB_TRACK_FULL_PAGES */
#define _SLAB_SPECS_INIT__FULLPAGES(n) /* nothing */
#endif /* !SLAB_TRACK_FULL_PAGES */
#ifdef CONFIG_NO_THREADS
#define _SLAB_SPECS_INIT__LOCK(n) /* nothing */
#else /* CONFIG_NO_THREADS */
#define _SLAB_SPECS_INIT__LOCK(n) , &PP_CAT2(slab_lock, n)
#endif /* !CONFIG_NO_THREADS */
#define SLAB_SPECS_INIT_CB(n, _)                             \
	{                                                        \
		PAGESPECS_INIT(n),                                   \
		(struct Dee_slab_page_list *)&PP_CAT2(slab_pages, n) \
		_SLAB_SPECS_INIT__FULLPAGES(n)                       \
		_SLAB_SPECS_INIT__LOCK(n)                            \
	},
#ifndef __INTELLISENSE__
	Dee_SLAB_CHUNKSIZE_FOREACH(SLAB_SPECS_INIT_CB, ~)
#endif /* !__INTELLISENSE__ */
#undef SLAB_SPECS_INIT_CB
#undef _SLAB_SPECS_INIT__FULLPAGES
#undef _SLAB_SPECS_INIT__LOCK
};

PRIVATE NONNULL((1, 2)) void DCALL
check_slab_page(struct slab_spec const *__restrict spec,
                struct Dee_slab_page *__restrict self) {
	size_t bitno, orig_used, real_used;
	slab_bitword_t const *self__sp_used = (slab_bitword_t const *)self->sp_used_and_data;
	byte_t const *self__sp_data    = (byte_t const *)self + spec->ss_specs.ps_sizeof__sp_used;
again:
	orig_used = atomic_read(&self->sp_meta.spm_used);
	for (real_used = bitno = 0; bitno < spec->ss_specs.ps_chunkcount; ++bitno) {
		size_t bit_indx         = slab_bitword_indx(bitno);
		slab_bitword_t bit_mask = slab_bitword_mask(bitno);
		slab_bitword_t word     = atomic_read(&self__sp_used[bit_indx]);
		byte_t const *blob = self__sp_data + (bitno * spec->ss_specs.ps_chunksize);
		if ((word & bit_mask) == 0) {
			/* Verify that a chunk marked as unused is filled with the
			 * proper memory pattern (this detects use-after-free). */
#ifdef SLAB_DEBUG_MEMSET_FREE
			slab_chkfree_data(blob, spec->ss_specs.ps_chunksize);
#endif /* SLAB_DEBUG_MEMSET_FREE */
		} else {
			++real_used;
		}
	}

	/* Assert that no trailing bits are set in last word of "self__sp_used" */
	{
		shift_t lastbits = spec->ss_specs.ps_chunkcount % BITSOF_slab_bitword_t;
		size_t last_indx = (spec->ss_specs.ps_chunkcount - 1) / BITSOF_slab_bitword_t;
		slab_bitword_t lastmask = lastbits ? ((SLAB_BITWORD_C(1) << lastbits) - 1) : (slab_bitword_t)-1;
		slab_bitword_t lastword = atomic_read(&self__sp_used[last_indx]);
		ASSERTF((lastword & ~lastmask) == 0,
		        "Invalid in-use bits %#" PRFxN(SIZEOF_slab_bitword_t) " enabled "
		        /**/ "in last word of 'sp_used' (chunksize: %" PRFuSIZ ")\n"
		        "word: %#" PRFxN(SIZEOF_slab_bitword_t) "\n"
		        "mask: %#" PRFxN(SIZEOF_slab_bitword_t) "\n",
		        lastword & ~lastmask, spec->ss_specs.ps_chunksize, lastword, lastmask);
	}

	/* Other threads can DeeSlab_Free() chunks even while our caller has a write-lock
	 * the the slab's "ss_lock". Because of this, "real_used" may be greater than the
	 * originally read `orig_used', but when that is the case, a newly read 'orig_used'
	 * must also be greater. */
	ASSERTF(real_used >= orig_used,
	        "Too many chunks marked as in-use (chunksize: %" PRFuSIZ ")\n"
	        "sp_meta.spm_used: %" PRFuSIZ " (as specified in metadata prior to scan)\n"
	        "real_used:        %" PRFuSIZ " (# of 1-bits in 'sp_used')",
	        spec->ss_specs.ps_chunksize, orig_used, real_used);
	if (real_used > orig_used) {
		size_t new_orig_used = atomic_read(&self->sp_meta.spm_used);
		ASSERTF(new_orig_used > orig_used,
		        "Too few chunks marked as in-use (chunksize: %" PRFuSIZ ")\n"
		        "sp_meta.spm_used: %" PRFuSIZ " (as specified in metadata after scan)\n"
		        "orig_used:        %" PRFuSIZ " (as specified in metadata prior to scan)\n"
		        "real_used:        %" PRFuSIZ " (# of 1-bits in 'sp_used')",
		        spec->ss_specs.ps_chunksize, new_orig_used, orig_used, orig_used);
		orig_used = new_orig_used;
		goto again;
	}
}

PRIVATE NONNULL((1)) void DCALL
check_slab(struct slab_spec const *__restrict spec) {
	struct Dee_slab_page **p_iter, *iter;
	Dee_atomic_rwlock_write(spec->ss_lock);
	for (p_iter = LIST_PFIRST(spec->ss_pages); (iter = *p_iter) != NULL;
	     p_iter = LIST_PNEXT(iter, sp_meta.spm_type.t_link)) {
		struct Dee_slab_page **real_prev;
		ASSERTF(IS_ALIGNED((uintptr_t)iter, Dee_SLAB_PAGESIZE),
		        "Incorrectly aligned slab page base pointer %p (read from %p%s) "
		        /**/ "encountered in 'slab_pages%" PRFuSIZ "'%s",
		        iter, p_iter,
		        p_iter == LIST_PFIRST(spec->ss_pages)
		        ? " (first member of slab_pages)"
		        : "",
		        spec->ss_specs.ps_chunksize);
		real_prev = iter->sp_meta.spm_type.t_link.le_prev;
		ASSERTF(real_prev == p_iter,
		        "Broken self-pointer at %p in slab page %p (chunksize: %" PRFuSIZ "):\n"
		        "Expected: %p\n"
		        "Actual:   %p\n",
		        &iter->sp_meta.spm_type.t_link.le_prev,
		        iter, spec->ss_specs.ps_chunksize,
		        p_iter, real_prev);
		check_slab_page(spec, iter);
	}
	Dee_atomic_rwlock_endwrite(spec->ss_lock);
}
#endif /* SLAB_DEBUG_EXTERNAL */


#ifdef HAVE_Dee_slab_leaks_tryacquire
INTERN WUNUSED Dee_atomic_rwlock_t *DCALL
Dee_slab_leaks_tryacquire(void) {
#if SLAB_DEBUG_EXTERNAL
	size_t i;
	for (i = 0; i < SLAB_COUNT; ++i) {
		Dee_atomic_rwlock_t *lock = slab_specs[i].ss_lock;
		if (!Dee_atomic_rwlock_trywrite(lock)) {
			while (i--) {
				Dee_atomic_rwlock_t *prev_lock = slab_specs[i].ss_lock;
				Dee_atomic_rwlock_endwrite(prev_lock);
			}
			return lock;
		}
	}
#endif /* SLAB_DEBUG_EXTERNAL */
	return NULL;
}

INTERN void DCALL Dee_slab_leaks_release(void) {
#if SLAB_DEBUG_EXTERNAL
	size_t i = SLAB_COUNT;
	do {
		Dee_atomic_rwlock_t *lock;
		--i;
		lock = slab_specs[i].ss_lock;
		Dee_atomic_rwlock_endwrite(lock);
	} while (i);
#endif /* SLAB_DEBUG_EXTERNAL */
}
#endif /* HAVE_Dee_slab_leaks_tryacquire */


/* Enumerate all slab pages containing at least 1 allocated chunk.
 * Before calling this function, the caller must acquire locks by
 * use of `Dee_slab_leaks_tryacquire()'
 * @return: * : Dee_formatprinter_t-style aggregate of calls to `cb' */
#ifdef HAVE_Dee_slab_leaks_foreach_page
#if SLAB_DEBUG_EXTERNAL
PRIVATE NONNULL((1, 2, 3)) Dee_ssize_t DCALL
Dee_slab_leaks_foreach_page__list(struct slab_spec const *spec,
                                  struct Dee_slab_page_list *pages,
                                  Dee_slab_leaks_page_cb_t cb, void *arg) {
	Dee_ssize_t temp, result = 0;
	struct Dee_slab_page *page;
	LIST_FOREACH (page, pages, sp_meta.spm_type.t_link) {
		temp = (*cb)(arg, page, &spec->ss_specs);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}

PRIVATE NONNULL((1, 2)) Dee_ssize_t DCALL
Dee_slab_leaks_foreach_page__spec(struct slab_spec const *spec,
                                  Dee_slab_leaks_page_cb_t cb, void *arg) {
#if SLAB_TRACK_FULL_PAGES
	Dee_ssize_t result = Dee_slab_leaks_foreach_page__list(spec, spec->ss_pages, cb, arg);
	if likely(result >= 0) {
		Dee_ssize_t temp = Dee_slab_leaks_foreach_page__list(spec, spec->ss_fullpages, cb, arg);
		result = temp < 0 ? temp : (result + temp);
	}
	return result;
#else /* SLAB_TRACK_FULL_PAGES */
	return Dee_slab_leaks_foreach_page__list(spec, spec->ss_pages, cb, arg);
#endif /* !SLAB_TRACK_FULL_PAGES */
}
#endif /* SLAB_DEBUG_EXTERNAL */

INTERN NONNULL((1)) Dee_ssize_t DCALL
Dee_slab_leaks_foreach_page(Dee_slab_leaks_page_cb_t cb, void *arg) {
	Dee_ssize_t result = 0;
#if SLAB_DEBUG_EXTERNAL
	size_t i;
	for (i = 0; i < SLAB_COUNT; ++i) {
		Dee_ssize_t temp = Dee_slab_leaks_foreach_page__spec(&slab_specs[i], cb, arg);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
#endif /* SLAB_DEBUG_EXTERNAL */
	(void)cb;
	(void)arg;
	return result;
}
#endif /* HAVE_Dee_slab_leaks_foreach_page */


/* Called by `DeeHeap_CheckMemory()' */
#ifdef HAVE_DeeSlab_CheckMemory
INTERN void DCALL DeeSlab_CheckMemory(void) {
#if SLAB_DEBUG_EXTERNAL
	size_t i;
	for (i = 0; i < SLAB_COUNT; ++i)
		check_slab(&slab_specs[i]);
#endif /* SLAB_DEBUG_EXTERNAL */
}
#endif /* HAVE_DeeSlab_CheckMemory */



/* One-time, static test-code to demonstrate that a
 * 4-word metadata footer doesn't change anything */
#if defined(__INTELLISENSE__) && 0
#define EX_Dee_SIZEOF_SLAB_PAGE_META(ms)  (ms) /* `sizeof(struct Dee_slab_page::sp_meta)' */
#define EX_Dee_OFFSET_SLAB_PAGE_META(ms)  (Dee_SLAB_PAGESIZE - EX_Dee_SIZEOF_SLAB_PAGE_META(ms))
#define EX__UPPER__MAX_CHUNK_COUNT(ms, n) (EX_Dee_OFFSET_SLAB_PAGE_META(ms) / n)
#define EX__UPPER_ELEMOF__sp_used(ms, n)  CEILDIV(EX__UPPER__MAX_CHUNK_COUNT(ms, n), BITSOF_slab_bitword_t)
#define EX__UPPER_SIZEOF__sp_used(ms, n)  (EX__UPPER_ELEMOF__sp_used(ms, n) * SIZEOF_slab_bitword_t)
#define EX__LOWER_SIZEOF__sp_data(ms, n)  (EX_Dee_OFFSET_SLAB_PAGE_META(ms) - EX__UPPER_SIZEOF__sp_used(ms, n))
#define EX__LOWER__MAX_CHUNK_COUNT(ms, n) (EX__LOWER_SIZEOF__sp_data(ms, n) / n)
#define EX__LOWER_ELEMOF__sp_used(ms, n)  CEILDIV(EX__LOWER__MAX_CHUNK_COUNT(ms, n), BITSOF_slab_bitword_t)
#define EX__LOWER_SIZEOF__sp_used(ms, n)  (EX__LOWER_ELEMOF__sp_used(ms, n) * SIZEOF_slab_bitword_t)
#define EX__LOWER_BITSOF__sp_used(ms, n)  (EX__LOWER_ELEMOF__sp_used(ms, n) * BITSOF_slab_bitword_t)
#define EX__FINAL_SIZEOF__sp_data(ms, n)  (EX_Dee_OFFSET_SLAB_PAGE_META(ms) - EX__LOWER_SIZEOF__sp_used(ms, n))
#define EX__MAX_CHUNK_COUNT(ms, n)        (EX__FINAL_SIZEOF__sp_data(ms, n) / n)
#define EX_MAX_CHUNK_COUNT(ms, n)         (EX__MAX_CHUNK_COUNT(ms, n) > EX__LOWER_BITSOF__sp_used(ms, n) ? EX__LOWER_BITSOF__sp_used(ms, n) : EX__MAX_CHUNK_COUNT(ms, n))

#define C_MAX_CHUNK_COUNT_3WORD(n) EX_MAX_CHUNK_COUNT(3 * __SIZEOF_POINTER__, n)
#define C_MAX_CHUNK_COUNT_4WORD(n) EX_MAX_CHUNK_COUNT(4 * __SIZEOF_POINTER__, n)

#define ASSERT_SLAB_SIZES_UNCHANGED(n, _) \
	STATIC_ASSERT_MSG(C_MAX_CHUNK_COUNT_4WORD(n) == C_MAX_CHUNK_COUNT_3WORD(n), "changed for size " #n);
Dee_SLAB_CHUNKSIZE_FOREACH(ASSERT_SLAB_SIZES_UNCHANGED, ~)
#undef ASSERT_SLAB_SIZES_UNCHANGED
#endif /* __INTELLISENSE__ */



DECL_END
#endif /* Dee_SLAB_CHUNKSIZE_MAX */

#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

#endif /* !GUARD_DEEMON_RUNTIME_SLAB_C */
