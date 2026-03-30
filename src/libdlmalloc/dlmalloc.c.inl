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
#ifndef GUARD_LIBDLMALLOC_DLMALLOC_C_INL
#define GUARD_LIBDLMALLOC_DLMALLOC_C_INL 1

/* Disable these attributes in here -- we must routinely write past the end
 * of our own allocations, because how else are we supposed to manage adjacent
 * heap blocks and/or initialize footers.
 *
 * Without this, we get crashes: "*** buffer overflow detected ***: terminated"
 *
 * I think this is due to GCC intentionally crashing the program in places
 * where we intentionally write past the end of a heap block in to overwrite
 * the block's footer with extra debug info... */
#undef ATTR_ALLOC_SIZE
#define ATTR_ALLOC_SIZE(ppars) /* Nothing */
#undef ATTR_ALLOC_ALIGN
#define ATTR_ALLOC_ALIGN(pari) /* Nothing */

#include <hybrid/atomic.h>          /* ATOMIC_* */
#include <hybrid/bit.h>             /* CLZ, CTZ */
#include <hybrid/compiler.h>
#include <hybrid/debug-alignment.h> /* DBG_ALIGNMENT_DISABLE, DBG_ALIGNMENT_ENABLE */
#include <hybrid/host.h>            /* __ARCH_PAGESIZE */
#include <hybrid/limitcore.h>       /* __SIZE_MAX__ */
#include <hybrid/overflow.h>        /* OVERFLOW_UMUL */
#include <hybrid/sched/yield.h>     /* SCHED_YIELD */
#include <hybrid/sequence/list.h>   /* SLIST_* */
#include <hybrid/typecore.h>        /* __ALIGNOF_MAX_ALIGN_T__, __BYTE_TYPE__, __CHAR_BIT__, __SIZEOF_POINTER__, __SIZEOF_SIZE_T__, __SIZE_C */

#include <stddef.h> /* offsetof, size_t */
#include <stdint.h> /* UINT32_C, UINT64_C, uintptr_t */

#ifndef SIZE_MAX
#define SIZE_MAX __SIZE_MAX__
#endif /* !SIZE_MAX */
#ifndef SIZE_C
#ifdef __SIZE_C
#define SIZE_C __SIZE_C
#else /* __SIZE_C */
#define SIZE_C(x) ((size_t)x)
#endif /* !__SIZE_C */
#endif /* !SIZE_C */

#undef byte_t
#define byte_t __BYTE_TYPE__
#undef container_of
#define container_of COMPILER_CONTAINER_OF

DECL_BEGIN

/* Heavily modified version of dlmalloc (Doug Lea's malloc).
 *
 * dlmalloc is public domain [*], but I feel like it's just
 * a matter of respect to state this inspiration, and give
 * credit where credit is due ;)
 *
 *
 * The basis for this implementation is the already-modified version found in the KOS source tree here:
 * >> https://github.com/GrieferAtWork/KOSmk4/blob/2ec470169e5df4a9f0b28fd4ff0c105a4f731852/kos/src/libdlmalloc/dlmalloc.c
 *
 * That file in turn is based on the original dlmalloc:
 * >> v2.8.6 Wed Aug 29 06:57:58 2012  Doug Lea
 *
 * [*] dlmalloc can normally be found here: https://gee.cs.oswego.edu/pub/misc/malloc.c
 *     But I just noticed that more recent versions of dlmalloc (as of 2023) are no longer
 *     considered "public domain". For this purpose, I'd like to clarify that the basis
 *     of the dlmalloc is "v2.8.6", as released mid-2012. Nothing added to the official
 *     release of dlmalloc after that point has ever been added here (as a matter of
 *     fact: other than the license change, nothing has been added to it since then,
 *     either). A copy of the original "v2.8.6" release (with its original "public domain"
 *     licensing statement) can be found here:
 *     https://web.archive.org/web/20130508225521/http://gee.cs.oswego.edu/pub/misc/malloc.c
 *
 * Heavy modifications have been made:
 * - DL_DEBUG_MEMSET_ALLOC / DL_DEBUG_MEMSET_FREE:
 *   Debug option to fill freshly malloc'd / free'd memory with patterns
 * - GM_ONLY: **only** provide a single, global mspace
 * - FLAG4_BIT_HEAP_REGION: define an ABI for serializable heap chunks
 * - Determine system implementation (VirtualAlloc / mmap / sbrk) based
 *   on user-provided DL_CONFIG_HAVE_* macros
 * - Add support for a 4th system implementation: malloc(3)
 *   (iow: implement dlmalloc() using the system's native malloc(3))
 * - Split dlmalloc debug checks into INTERNAL and EXTERNAL
 * - Change dlcalloc to take only a single argument
 * - Assert in-use in dlmalloc_usable_size
 * - Use-after-free detection when re-used memory is allocated again
 * - USE_PENDING_FREE_LIST: Support for guarantied non-blocking dlfree()
 * - USE_PER_THREAD_MSTATE: Support for thread-local heaps
 */


/* Threading configuration:
 *
 * - USE_LOCKS -------------------------------------------------
 *   Same as already defined by the original dlmalloc: adds
 *   locking to guard against multi-threaded access to globals
 * -------------------------------------------------------------
 *
 *
 * - USE_PENDING_FREE_LIST -------------------------------------
 *   Makes use of SLIST_ATOMIC_INSERT to form a queue of pending
 *   heap pointers when dlfree() is unable to acquire the lock
 *   added by USE_LOCKS. When this extension is enabled, freeing
 *   memory is a guarantied non-blocking operation.
 * -------------------------------------------------------------
 *
 *
 * - USE_PER_THREAD_MSTATE -------------------------------------
 *   Provide a per-thread "mstate" that is used when the global
 *   state is locked.
 *
 *   Makes use of thread-local memory, MSPACES and FOOTERS to:
 *   - Allow for an arbitrary number of lazily allocated, extra
 *     mspaces that can be used to allocate memory from a
 *     thread-local mspace when the global mspace ("gm") cannot
 *     be locked.
 *   - When a thread that had previously allocated a private
 *     mspace exits, that mspace is added to a global queue of
 *     available mspaces, such that another thread can re-use
 *     it (actually freeing these mspaces isn't possible, since
 *     a thread exiting doesn't mean that memory it allocated is
 *     no longer in-use: lazily allocated caches, and any memory
 *     used by the thread's return value are examples that would
 *     come to mind)
 *   - When dlmalloc() fails to acquire the USE_LOCKS lock, then
 *     the calling thread's tls_mspace() is used (if available)
 *   - Since FOOTERS=1 + MSPACES=1 stores the originating mspace
 *     within the returned heap pointer, dlfree() is able to
 *     free memory from a thread-local mspace, even when free'd
 *     from another thread!
 *   This extension provides an **EXTREMELY** significant boost
 *   in performance when API consumers make heavy use of threads,
 *   as it solves the problem of atomic contention and the fact
 *   that only a single thread can allocate from the same mspace
 *   at the same time.
 *
 *   When enabled, the user must define some additional macros
 *   that are needed for managing the live-time / lazy-
 *   initialization of the thread-local mspace/heap:
 *   >> PRIVATE _Thread_local void *tls_heap = NULL
 *   >>     __attribute__((cleanup(tls_mspace_destroy)));
 *   >> #define DL_TLS_GETHEAP(p) (void)(*(p) = tls_heap)
 *   >> #define DL_TLS_SETHEAP(v) (void)(tls_heap = (v))
 * -------------------------------------------------------------
 *
 *
 * - USE_MSPACE_MALLOC_LOCKLESS --------------------------------
 *   Define a new method "mspace_malloc_lockless" (same as
 *   "mspace_malloc", but caller must do "PREACTION") This is a
 *   minor optimization on-top of "USE_PER_THREAD_MSTATE", since
 *   it allows `dlmalloc()' to select either the "tls" or "gm"
 *   allocator, based on whichever becomes available first,
 *   rather than having to unconditionally rely on "tls" as soon
 *   as "gm" cannot be locked even once.
 * -------------------------------------------------------------
 */
#ifndef USE_LOCKS
#define USE_LOCKS 0
#endif /* !USE_LOCKS */
#ifndef USE_PENDING_FREE_LIST
#define USE_PENDING_FREE_LIST 0
#endif /* !USE_PENDING_FREE_LIST */
#ifndef USE_PER_THREAD_MSTATE
#define USE_PER_THREAD_MSTATE 0
#endif /* !USE_PER_THREAD_MSTATE */
#ifndef USE_MSPACE_MALLOC_LOCKLESS
#define USE_MSPACE_MALLOC_LOCKLESS USE_PER_THREAD_MSTATE
#endif /* !USE_MSPACE_MALLOC_LOCKLESS */



/* Debug configuration:
 *
 * - DL_DEBUG_EXTERNAL -----------------------------------------
 *   Main switch for enabling debug features meant to protect
 *   against, and check for errors that may be made by code
 *   outside of this (modified) dlmalloc implementation. Also
 *   defines the default state of a number of individually
 *   configurable debug features:
 *   - footers
 *   - usage checks
 *   - debug-memset patterns
 *   - leak detector
 *   - ...
 * -------------------------------------------------------------
 *
 *
 * - DL_DEBUG_INTERNAL -----------------------------------------
 *   internal debugging (self-consistency assertions).
 *
 *   Enable debug checks/assertions that are internal to this
 *   (modified) dlmalloc implementation. This mainly includes
 *   checks to ensure consistency during/after the internal
 *   state transitions made during dlmalloc() or dlfree().
 *   Generally, this switch can be kept off, but it may be of
 *   use when debugging bugs introduced by one of the many
 *   additions made to dlmalloc here.
 * -------------------------------------------------------------
 *
 *
 * - DL_DEBUG_MEMSET_ALLOC / DL_DEBUG_MEMSET_FREE --------------
 *   Special word-sized patterns with which to fill memory after
 *   it was allocated by dlmalloc(). This not only makes a lot
 *   of bad-api-usage-related bugs fully consistent and easily
 *   reproducible, but it also makes it very easy to spot memory
 *   that:
 *   - was allocated but not initialized  (CCCCCCCC)
 *   - was free'd but is still referenced (DEADBEEF)
 *   Note that the pattern for newly allocated memory was chosen
 *   to repeat itself across all bytes of some word. This way,
 *   it becomes trivial to spot uninitialized memory when only
 *   parts of some word were initialized.
 * -------------------------------------------------------------
 *
 *
 * - DETECT_USE_AFTER_FREE -------------------------------------
 *   When memory is allocated that was previously free'd, check
 *   that its contents still reflect `DL_DEBUG_MEMSET_FREE_PATTERN'.
 *   This feature requires `DL_DEBUG_MEMSET_FREE_PATTERN' to work.
 * -------------------------------------------------------------
 */
#ifndef DL_DEBUG_INTERNAL
#if !defined(NDEBUG) && !defined(__OPTIMIZE_SIZE__) && 0
#define DL_DEBUG_INTERNAL 1
#else
#define DL_DEBUG_INTERNAL 0
#endif
#endif /* !DL_DEBUG_INTERNAL */
#ifndef DL_DEBUG_EXTERNAL
#if !defined(NDEBUG) && !defined(__OPTIMIZE_SIZE__) && 1
#define DL_DEBUG_EXTERNAL 1
#else
#define DL_DEBUG_EXTERNAL 0
#endif
#endif /* !DL_DEBUG_EXTERNAL */

#ifndef DL_DEBUG_MEMSET_ALLOC
#define DL_DEBUG_MEMSET_ALLOC DL_DEBUG_EXTERNAL
#endif /* !DL_DEBUG_MEMSET_ALLOC */
#ifndef DL_DEBUG_MEMSET_FREE
#define DL_DEBUG_MEMSET_FREE DL_DEBUG_EXTERNAL
#endif /* !DL_DEBUG_MEMSET_FREE */
#ifndef DETECT_USE_AFTER_FREE
#define DETECT_USE_AFTER_FREE DL_DEBUG_MEMSET_FREE
#endif /* !DETECT_USE_AFTER_FREE */

#if __SIZEOF_POINTER__ > 4
#define WORD_4BYTE(x) UINT64_C(0x##x##x)
#else /* __SIZEOF_POINTER__ > 4 */
#define WORD_4BYTE(x) UINT32_C(0x##x)
#endif /*__SIZEOF_POINTER__ <= 4 */

#if !DL_DEBUG_MEMSET_ALLOC
#undef DL_DEBUG_MEMSET_ALLOC_PATTERN
#elif !defined(DL_DEBUG_MEMSET_ALLOC_PATTERN)
#define DL_DEBUG_MEMSET_ALLOC_PATTERN WORD_4BYTE(CCCCCCCC) /* Set by dlmalloc() */
#endif

#if !DL_DEBUG_MEMSET_FREE
#undef DL_DEBUG_MEMSET_FREE_PATTERN
#elif !defined(DL_DEBUG_MEMSET_FREE_PATTERN)
#define DL_DEBUG_MEMSET_FREE_PATTERN WORD_4BYTE(DEADBEEF) /* Set by dlfree() */
#endif





/* Configure some of dlmalloc's remaining builtin config options. */
#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT __ALIGNOF_MAX_ALIGN_T__
#endif /* !MALLOC_ALIGNMENT */

/* The whole idea is to implement `dlmalloc()', so we'll need
 * more than MSPACES (but see `USE_PER_THREAD_MSTATE') */
#ifndef ONLY_MSPACES
#define ONLY_MSPACES 0
#endif /* ONLY_MSPACES */

/* Wouldn't even work anymore since RTCHECK was replaced with "ext_assert()" */
#ifndef PROCEED_ON_ERROR
#define PROCEED_ON_ERROR 0
#endif /* PROCEED_ON_ERROR */

/* Enable support for `dlmalloc_inspect_all()' */
#ifndef MALLOC_INSPECT_ALL
#define MALLOC_INSPECT_ALL 0
#endif /* MALLOC_INSPECT_ALL */

/* Need to enable MSPACES support for `tls_mspace()' */
#ifndef MSPACES
#define MSPACES (ONLY_MSPACES || USE_PER_THREAD_MSTATE)
#endif /* MSPACES */

/* Without MSPACES, we only need the "gm" mspace */
#ifndef GM_ONLY
#define GM_ONLY (!MSPACES)
#endif /* GM_ONLY */

/* Set to `1' to disable some asserts/assumptions that were originally used
 * to implement `RTCHECK'. These were turned into assertions/__builtin_assume,
 * so this option kind-of does the opposite now... */
#ifndef INSECURE
#define INSECURE 0
#endif /* INSECURE */

/* xor-mask footers of heap chunks with "mparams.magic" when using them as mspace indicator */
#ifndef XOR_MASK_MCHUNK_FOOT
#define XOR_MASK_MCHUNK_FOOT DL_DEBUG_EXTERNAL
#endif /* XOR_MASK_MCHUNK_FOOT */

#ifndef FOOTERS
#define FOOTERS USE_PER_THREAD_MSTATE /* Needed for `dlfree()' to detect source mspace! */
#endif /* !FOOTERS */

/* Custom dlmalloc extension:
 * FLAG4_BIT on an allocation means it's part of "STRUCT_dlheapregion" */
#ifndef FLAG4_BIT_HEAP_REGION
#define FLAG4_BIT_HEAP_REGION 0
#endif /* !FLAG4_BIT_HEAP_REGION */

/* Extension to "FLAG4_BIT_HEAP_REGION" and "DL_DEBUG_MEMSET_FREE":
 * - When inside of `dlfree()' with a chunk that is part of a heap
 *   region, only apply "DL_DEBUG_MEMSET_FREE_PATTERN" to selective words:
 * >> size_t i, n_words = CHUNK_SIZE / sizeof(HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_WORD_T);
 * >> HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_WORD_T *iter = (HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_WORD_T *)CHUNK_BASE;
 * >> for (i = 0; i < n_words; ++i, ++iter) {
 * >>     if (HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_CANSET(i * sizeof(HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_WORD_T), iter)) {
 * >>         *iter = DL_DEBUG_MEMSET_FREE_PATTERN;
 * >>     } else {
 * >>         // Skip this word
 * >>     }
 * >> }
 *
 * When this feature flag is "1", you must define the additional macros:
 * >> #define HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_CANSET(offset, p_word) <boolean-expression>   // MAND
 * >> #define HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_WORD_T                 <word-type>            // OPT */
#ifndef FLAG4_BIT_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE
#define FLAG4_BIT_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE 0
#endif /* !FLAG4_BIT_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE */

/* Extension to 'FLAG4_BIT_HEAP_REGION':
 * - Allow `STRUCT_dlheaptail::ht_zero' to be non-zero and point at some
 *   extra, custom debug information / metadata that should be associated
 *   with the heap region.
 * - When this field is non-zero, an additional callback is invoked before
 *   the region's actual `hr_destroy' function pointer is called:
 *
 *   >> static ATTR_RETNONNULL NONNULL((1)) STRUCT_dlheapregion *DCALL
 *   >> dl_dbg_heapregion_dispose(STRUCT_dlheapregion *__restrict region);
 *
 *   This callback should re-return "region", after disposing any debug
 *   info that may have been linked to the region (which was probably done
 *   via its "ht_zero"-field, because again: this only happens when "ht_zero"
 *   ends up being non-zero) */
#ifndef FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
#define FLAG4_BIT_HEAP_REGION_DBG_DISPOSE 0
#endif /* !FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */


/* Auto-complete API configuration */
#ifndef MALLINFO_FIELD_TYPE
#define MALLINFO_FIELD_TYPE int
#endif /* !MALLINFO_FIELD_TYPE */
#ifndef NO_MALLINFO
#define NO_MALLINFO 0
#endif /* !NO_MALLINFO */
#ifndef NO_MALLOC_STATS
#define NO_MALLOC_STATS 0
#endif /* !NO_MALLOC_STATS */
#ifndef NO_SEGMENT_TRAVERSAL
#define NO_SEGMENT_TRAVERSAL 0
#endif /* !NO_SEGMENT_TRAVERSAL */
#ifndef NO_POSIX_MEMALIGN
#define NO_POSIX_MEMALIGN 0
#endif /* !NO_POSIX_MEMALIGN */
#ifndef NO_VALLOC
#define NO_VALLOC 0
#endif /* !NO_VALLOC */
#ifndef NO_PVALLOC
#define NO_PVALLOC 0
#endif /* !NO_PVALLOC */
#ifndef NO_MALLOPT
#define NO_MALLOPT 0
#endif /* !NO_MALLOPT */
#ifndef NO_MALLOC_FOOTPRINT
#define NO_MALLOC_FOOTPRINT 0
#endif /* !NO_MALLOC_FOOTPRINT */
#ifndef NO_INDEPENDENT_ALLOC
#define NO_INDEPENDENT_ALLOC 0
#endif /* !NO_INDEPENDENT_ALLOC */
#ifndef NO_BULK_FREE
#define NO_BULK_FREE 0
#endif /* !NO_BULK_FREE */
#ifndef NO_MALLOC_TRIM
#define NO_MALLOC_TRIM 0
#endif /* !NO_MALLOC_TRIM */

#ifndef NO_MSPACE_FREE
#define NO_MSPACE_FREE 0
#endif /* !NO_MSPACE_FREE */
#ifndef NO_MSPACE_REALLOC
#define NO_MSPACE_REALLOC 0
#endif /* !NO_MSPACE_REALLOC */
#ifndef NO_MSPACE_REALLOC_IN_PLACE
#define NO_MSPACE_REALLOC_IN_PLACE 0
#endif /* !NO_MSPACE_REALLOC_IN_PLACE */
#ifndef NO_MSPACE_USABLE_SIZE
#define NO_MSPACE_USABLE_SIZE 0
#endif /* !NO_MSPACE_USABLE_SIZE */
#ifndef NO_MSPACE_TRACK_LARGE_CHUNKS
#define NO_MSPACE_TRACK_LARGE_CHUNKS 0
#endif /* !NO_MSPACE_TRACK_LARGE_CHUNKS */
#ifndef NO_CREATE_MSPACE_WITH_BASE
#define NO_CREATE_MSPACE_WITH_BASE 0
#endif /* !NO_CREATE_MSPACE_WITH_BASE */
#ifndef NO_MSPACE_CALLOC
#define NO_MSPACE_CALLOC 0
#endif /* !NO_MSPACE_CALLOC */
#ifndef NO_MSPACE_MEMALIGN
#define NO_MSPACE_MEMALIGN 0
#endif /* !NO_MSPACE_MEMALIGN */
#ifndef NO_MSPACE_FOOTPRINT
#define NO_MSPACE_FOOTPRINT 0
#endif /* !NO_MSPACE_FOOTPRINT */
#ifndef NO_MSPACE_MAX_FOOTPRINT
#define NO_MSPACE_MAX_FOOTPRINT 0
#endif /* !NO_MSPACE_MAX_FOOTPRINT */
#ifndef NO_MSPACE_FOOTPRINT_LIMIT
#define NO_MSPACE_FOOTPRINT_LIMIT 0
#endif /* !NO_MSPACE_FOOTPRINT_LIMIT */
#ifndef NO_MSPACE_SET_FOOTPRINT_LIMIT
#define NO_MSPACE_SET_FOOTPRINT_LIMIT 0
#endif /* !NO_MSPACE_SET_FOOTPRINT_LIMIT */
#ifndef NO_MSPACE_TRIM
#define NO_MSPACE_TRIM 0
#endif /* !NO_MSPACE_TRIM */
#ifndef NO_MSPACE_MALLINFO
#define NO_MSPACE_MALLINFO 0
#endif /* !NO_MSPACE_MALLINFO */
#ifndef NO_DESTROY_MSPACE
#define NO_DESTROY_MSPACE 0
#endif /* !NO_DESTROY_MSPACE */

#ifndef M_TRIM_THRESHOLD
#define M_TRIM_THRESHOLD (-1)
#endif /* !M_TRIM_THRESHOLD */
#ifndef M_GRANULARITY
#define M_GRANULARITY    (-2)
#endif /* !M_GRANULARITY */
#ifndef M_MMAP_THRESHOLD
#define M_MMAP_THRESHOLD (-3)
#endif /* !M_MMAP_THRESHOLD */


/* Defaults for host system features (stuff that isn't supported) */
#ifndef DL_CONFIG_HAVE_VirtualAlloc
#define DL_CONFIG_HAVE_VirtualAlloc 0
#endif /* !DL_CONFIG_HAVE_VirtualAlloc */
#ifndef DL_CONFIG_HAVE_mmap
#define DL_CONFIG_HAVE_mmap 0
#endif /* !DL_CONFIG_HAVE_mmap */
#ifndef DL_CONFIG_HAVE_munmap
#define DL_CONFIG_HAVE_munmap 0
#endif /* !DL_CONFIG_HAVE_munmap */
#ifndef DL_CONFIG_HAVE_PROT_READ
#define DL_CONFIG_HAVE_PROT_READ 0
#endif /* !DL_CONFIG_HAVE_PROT_READ */
#ifndef DL_CONFIG_HAVE_PROT_WRITE
#define DL_CONFIG_HAVE_PROT_WRITE 0
#endif /* !DL_CONFIG_HAVE_PROT_WRITE */
#ifndef DL_CONFIG_HAVE_MAP_PRIVATE
#define DL_CONFIG_HAVE_MAP_PRIVATE 0
#endif /* !DL_CONFIG_HAVE_MAP_PRIVATE */
#ifndef DL_CONFIG_HAVE_MAP_ANONYMOUS
#define DL_CONFIG_HAVE_MAP_ANONYMOUS 0
#endif /* !DL_CONFIG_HAVE_MAP_ANONYMOUS */
#ifndef DL_CONFIG_HAVE_sbrk
#define DL_CONFIG_HAVE_sbrk 0
#endif /* !DL_CONFIG_HAVE_sbrk */
#ifndef DL_CONFIG_HAVE_mremap
#define DL_CONFIG_HAVE_mremap 0
#endif /* !DL_CONFIG_HAVE_mremap */
#ifndef DL_CONFIG_HAVE_malloc
#define DL_CONFIG_HAVE_malloc 0
#endif /* !DL_CONFIG_HAVE_malloc */
#ifndef DL_CONFIG_HAVE_calloc
#define DL_CONFIG_HAVE_calloc 0
#endif /* !DL_CONFIG_HAVE_calloc */
#ifndef DL_CONFIG_HAVE_free
#define DL_CONFIG_HAVE_free 0
#endif /* !DL_CONFIG_HAVE_free */
#ifndef DL_CONFIG_HAVE_time
#define DL_CONFIG_HAVE_time 0
#endif /* !DL_CONFIG_HAVE_time */
#ifndef DL_CONFIG_HAVE_getpagesize
#define DL_CONFIG_HAVE_getpagesize 0
#endif /* !DL_CONFIG_HAVE_getpagesize */
#ifndef DL_CONFIG_HAVE_sysconf
#define DL_CONFIG_HAVE_sysconf 0
#endif /* !DL_CONFIG_HAVE_sysconf */
#ifndef DL_CONFIG_HAVE_SYS_PARAM_H
#define DL_CONFIG_HAVE_SYS_PARAM_H 0
#endif /* !DL_CONFIG_HAVE_SYS_PARAM_H */
#ifndef DL_CONFIG_HAVE_MREMAP_MAYMOVE
#define DL_CONFIG_HAVE_MREMAP_MAYMOVE 0
#endif /* !DL_CONFIG_HAVE_MREMAP_MAYMOVE */


/* Load fallback assert macro */
#ifndef DL_ASSERT
DECL_END
#include <hybrid/__assert.h> /* __hybrid_assert, __hybrid_assertf */
DECL_BEGIN
#define DL_ASSERT  __hybrid_assert
#define DL_ASSERTF __hybrid_assertf
#endif /* !DL_ASSERTF */
#ifndef DL_ASSERTF
#define DL_ASSERTF(expr, ...) DL_ASSERT(expr)
#endif /* !DL_ASSERTF */


/* EXTernal_ASSERT  (for external assertions; i.e. API usage, use-after-free, etc.) */
#undef ext_assert
#undef ext_assertf
#if DL_DEBUG_EXTERNAL
#define ext_assert  DL_ASSERT
#define ext_assertf DL_ASSERTF
#elif !defined(__NO_builtin_assume)
#define ext_assert             __builtin_assume
#define ext_assertf(expr, ...) __builtin_assume(expr)
#else /* ... */
#define ext_assert(expr) (void)0
#define ext_assertf(...) (void)0
#endif

/* DLmalloc_ASSERT  (for internal assertions; i.e. self-consistency checks) */
#undef dl_assert
#undef dl_assertf
#if DL_DEBUG_INTERNAL
#define dl_assert  DL_ASSERT
#define dl_assertf DL_ASSERTF
#elif !defined(__NO_builtin_assume)
#define dl_assert             __builtin_assume
#define dl_assertf(expr, ...) __builtin_assume(expr)
#else /* ... */
#define dl_assert(expr) (void)0
#define dl_assertf(...) (void)0
#endif


#if !NO_MALLINFO
#ifndef STRUCT_mallinfo
#define STRUCT_mallinfo struct mallinfo
struct mallinfo {
	MALLINFO_FIELD_TYPE arena;    /* non-mmapped space allocated from system */
	MALLINFO_FIELD_TYPE ordblks;  /* number of free chunks */
	MALLINFO_FIELD_TYPE smblks;   /* always 0 */
	MALLINFO_FIELD_TYPE hblks;    /* always 0 */
	MALLINFO_FIELD_TYPE hblkhd;   /* space in mmapped regions */
	MALLINFO_FIELD_TYPE usmblks;  /* maximum total allocated space */
	MALLINFO_FIELD_TYPE fsmblks;  /* always 0 */
	MALLINFO_FIELD_TYPE uordblks; /* total allocated space */
	MALLINFO_FIELD_TYPE fordblks; /* total free space */
	MALLINFO_FIELD_TYPE keepcost; /* releasable (via malloc_trim) space */
};
#define STRUCT_mallinfo_get_arena(x)       (x)->arena
#define STRUCT_mallinfo_set_arena(x, v)    (void)((x)->arena = (v))
#define STRUCT_mallinfo_get_ordblks(x)     (x)->ordblks
#define STRUCT_mallinfo_set_ordblks(x, v)  (void)((x)->ordblks = (v))
#define STRUCT_mallinfo_get_hblkhd(x)      (x)->hblkhd
#define STRUCT_mallinfo_set_hblkhd(x, v)   (void)((x)->hblkhd = (v))
#define STRUCT_mallinfo_get_usmblks(x)     (x)->usmblks
#define STRUCT_mallinfo_set_usmblks(x, v)  (void)((x)->usmblks = (v))
#define STRUCT_mallinfo_get_uordblks(x)    (x)->uordblks
#define STRUCT_mallinfo_set_uordblks(x, v) (void)((x)->uordblks = (v))
#define STRUCT_mallinfo_get_fordblks(x)    (x)->fordblks
#define STRUCT_mallinfo_set_fordblks(x, v) (void)((x)->fordblks = (v))
#define STRUCT_mallinfo_get_keepcost(x)    (x)->keepcost
#define STRUCT_mallinfo_set_keepcost(x, v) (void)((x)->keepcost = (v))
#endif /* !STRUCT_mallinfo */
#endif /* !NO_MALLINFO */


/* Check for some illegal configurations */
#if GM_ONLY && ONLY_MSPACES
#error "Make up your mind: do you only want <gm>, or do you only want <mspace>es?"
#endif /* GM_ONLY && ONLY_MSPACES */
#if GM_ONLY && MSPACES
#error "You can't have both 'only <gm>' and 'allow multiple <mspace>es'"
#endif /* GM_ONLY && MSPACES */
#if ONLY_MSPACES && !MSPACES
#error "If you want 'ONLY_MSPACES=1', you can't configure 'MSPACES=0'"
#endif /* ONLY_MSPACES && !MSPACES */

#if USE_PENDING_FREE_LIST && !USE_LOCKS
#error "'struct freelist' is only used when the relevant mstate can't be locked, but 'USE_LOCKS=0' means that is never the case"
#endif /* USE_PENDING_FREE_LIST && !USE_LOCKS */
#if USE_PER_THREAD_MSTATE && !USE_LOCKS
#error "'tls_mspace()' is used when 'gm' can't be locked, but USE_LOCKS=0 means 'gm' has no locks (and could thus always be locked)"
#endif /* USE_PER_THREAD_MSTATE && !USE_LOCKS */
#if USE_MSPACE_MALLOC_LOCKLESS && !USE_LOCKS
#error "'mspace_malloc_lockless()' with 'USE_LOCKS=0' doesn't make sense: mspace_malloc() already is lock-less in this case!"
#endif /* USE_MSPACE_MALLOC_LOCKLESS && !USE_LOCKS */

#if !MSPACES && USE_PER_THREAD_MSTATE
#error "'tls_mspace()' (as enabled by 'USE_PER_THREAD_MSTATE=1') requires that 'MSPACES=1' be enabled"
#endif /* !MSPACES && USE_PER_THREAD_MSTATE */

#if FLAG4_BIT_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE
#if !FLAG4_BIT_HEAP_REGION
#error "'FLAG4_BIT_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE=1' requires 'FLAG4_BIT_HEAP_REGION=1'"
#endif /* !FLAG4_BIT_HEAP_REGION */
#ifndef HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_CANSET
#error "Must '#define HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_CANSET(offset, p_word)' because 'FLAG4_BIT_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE=1'"
#endif /* !HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_CANSET */
#endif /* FLAG4_BIT_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE */
#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE && !FLAG4_BIT_HEAP_REGION
#error "'FLAG4_BIT_HEAP_REGION_DBG_DISPOSE=1' requires 'FLAG4_BIT_HEAP_REGION=1'"
#endif /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE && !FLAG4_BIT_HEAP_REGION */




/* ------------------- Declarations of public routines ------------------- */

/* Calling conventions/declaration-level for different APIs:
 *
 * Every dlmalloc "api" can have its visibility/calling-convention/name modified.
 * e.g.:
 *    >> #define DL_CC_dlmalloc     ATTR_STDCALL
 *    >> #define DL_DECL_dlmalloc   INTDEF
 *    >> #define DL_IMPL_dlmalloc   INTERN
 *    >> #define dlmalloc           my_malloc
 * This would result in "dlmalloc" being defined as:
 *    >> INTDEF void *ATTR_STDCALL my_malloc(size_t num_bytes);
 *    >> ...
 *    >> INTERN void *ATTR_STDCALL my_malloc(size_t num_bytes) {
 *    >>     ...
 *    >> }
 */

/*[[[deemon
local APIS = {
	"dlmalloc",
	"dlcalloc",
	"dlfree",
	"dlrealloc",
	"dlmemalign",
	"dlrealloc_in_place",
	"dlposix_memalign",
	"dlvalloc",
	"dlmallopt",
	"dlmalloc_footprint",
	"dlmalloc_max_footprint",
	"dlmalloc_footprint_limit",
	"dlmalloc_set_footprint_limit",
	"handler",
	"dlmallinfo",
	"dlindependent_calloc",
	"dlindependent_comalloc",
	"dlbulk_free",
	"dlpvalloc",
	"dlmalloc_trim",
	"dlmalloc_stats",
	"create_mspace",
	"destroy_mspace",
	"create_mspace_with_base",
	"mspace_track_large_chunks",
	"mspace_malloc",
	"mspace_malloc_lockless",
	"mspace_free",
	"mspace_realloc",
	"mspace_realloc_in_place",
	"mspace_calloc",
	"mspace_memalign",
	"mspace_independent_calloc",
	"mspace_independent_comalloc",
	"mspace_bulk_free",
	"mspace_footprint",
	"mspace_max_footprint",
	"mspace_footprint_limit",
	"mspace_set_footprint_limit",
	"mspace_mallinfo",
	"mspace_usable_size",
	"mspace_malloc_stats",
	"mspace_trim",
	"tls_mspace",
	"tls_mspace_foreach",
	"tls_mspace_destroy",
};
for (local what: { "DECL", "IMPL", "CC" }) {
	print("#ifndef DL_", what, "_DEFAULT");
	print("#define DL_", what, "_DEFAULT /" "* nothing *" "/");
	print("#endif /" "* !DL_", what, "_DEFAULT *" "/");
	for (local api: APIS) {
		print("#ifndef DL_", what, "_", api);
		print("#define DL_", what, "_", api, " DL_", what, "_DEFAULT");
		print("#endif /" "* !DL_", what, "_", api, " *" "/");
	}
}
]]]*/
#ifndef DL_DECL_DEFAULT
#define DL_DECL_DEFAULT /* nothing */
#endif /* !DL_DECL_DEFAULT */
#ifndef DL_DECL_dlmalloc
#define DL_DECL_dlmalloc DL_DECL_DEFAULT
#endif /* !DL_DECL_dlmalloc */
#ifndef DL_DECL_dlcalloc
#define DL_DECL_dlcalloc DL_DECL_DEFAULT
#endif /* !DL_DECL_dlcalloc */
#ifndef DL_DECL_dlfree
#define DL_DECL_dlfree DL_DECL_DEFAULT
#endif /* !DL_DECL_dlfree */
#ifndef DL_DECL_dlrealloc
#define DL_DECL_dlrealloc DL_DECL_DEFAULT
#endif /* !DL_DECL_dlrealloc */
#ifndef DL_DECL_dlmemalign
#define DL_DECL_dlmemalign DL_DECL_DEFAULT
#endif /* !DL_DECL_dlmemalign */
#ifndef DL_DECL_dlrealloc_in_place
#define DL_DECL_dlrealloc_in_place DL_DECL_DEFAULT
#endif /* !DL_DECL_dlrealloc_in_place */
#ifndef DL_DECL_dlposix_memalign
#define DL_DECL_dlposix_memalign DL_DECL_DEFAULT
#endif /* !DL_DECL_dlposix_memalign */
#ifndef DL_DECL_dlvalloc
#define DL_DECL_dlvalloc DL_DECL_DEFAULT
#endif /* !DL_DECL_dlvalloc */
#ifndef DL_DECL_dlmallopt
#define DL_DECL_dlmallopt DL_DECL_DEFAULT
#endif /* !DL_DECL_dlmallopt */
#ifndef DL_DECL_dlmalloc_footprint
#define DL_DECL_dlmalloc_footprint DL_DECL_DEFAULT
#endif /* !DL_DECL_dlmalloc_footprint */
#ifndef DL_DECL_dlmalloc_max_footprint
#define DL_DECL_dlmalloc_max_footprint DL_DECL_DEFAULT
#endif /* !DL_DECL_dlmalloc_max_footprint */
#ifndef DL_DECL_dlmalloc_footprint_limit
#define DL_DECL_dlmalloc_footprint_limit DL_DECL_DEFAULT
#endif /* !DL_DECL_dlmalloc_footprint_limit */
#ifndef DL_DECL_dlmalloc_set_footprint_limit
#define DL_DECL_dlmalloc_set_footprint_limit DL_DECL_DEFAULT
#endif /* !DL_DECL_dlmalloc_set_footprint_limit */
#ifndef DL_DECL_handler
#define DL_DECL_handler DL_DECL_DEFAULT
#endif /* !DL_DECL_handler */
#ifndef DL_DECL_dlmallinfo
#define DL_DECL_dlmallinfo DL_DECL_DEFAULT
#endif /* !DL_DECL_dlmallinfo */
#ifndef DL_DECL_dlindependent_calloc
#define DL_DECL_dlindependent_calloc DL_DECL_DEFAULT
#endif /* !DL_DECL_dlindependent_calloc */
#ifndef DL_DECL_dlindependent_comalloc
#define DL_DECL_dlindependent_comalloc DL_DECL_DEFAULT
#endif /* !DL_DECL_dlindependent_comalloc */
#ifndef DL_DECL_dlbulk_free
#define DL_DECL_dlbulk_free DL_DECL_DEFAULT
#endif /* !DL_DECL_dlbulk_free */
#ifndef DL_DECL_dlpvalloc
#define DL_DECL_dlpvalloc DL_DECL_DEFAULT
#endif /* !DL_DECL_dlpvalloc */
#ifndef DL_DECL_dlmalloc_trim
#define DL_DECL_dlmalloc_trim DL_DECL_DEFAULT
#endif /* !DL_DECL_dlmalloc_trim */
#ifndef DL_DECL_dlmalloc_stats
#define DL_DECL_dlmalloc_stats DL_DECL_DEFAULT
#endif /* !DL_DECL_dlmalloc_stats */
#ifndef DL_DECL_create_mspace
#define DL_DECL_create_mspace DL_DECL_DEFAULT
#endif /* !DL_DECL_create_mspace */
#ifndef DL_DECL_destroy_mspace
#define DL_DECL_destroy_mspace DL_DECL_DEFAULT
#endif /* !DL_DECL_destroy_mspace */
#ifndef DL_DECL_create_mspace_with_base
#define DL_DECL_create_mspace_with_base DL_DECL_DEFAULT
#endif /* !DL_DECL_create_mspace_with_base */
#ifndef DL_DECL_mspace_track_large_chunks
#define DL_DECL_mspace_track_large_chunks DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_track_large_chunks */
#ifndef DL_DECL_mspace_malloc
#define DL_DECL_mspace_malloc DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_malloc */
#ifndef DL_DECL_mspace_malloc_lockless
#define DL_DECL_mspace_malloc_lockless DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_malloc_lockless */
#ifndef DL_DECL_mspace_free
#define DL_DECL_mspace_free DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_free */
#ifndef DL_DECL_mspace_realloc
#define DL_DECL_mspace_realloc DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_realloc */
#ifndef DL_DECL_mspace_realloc_in_place
#define DL_DECL_mspace_realloc_in_place DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_realloc_in_place */
#ifndef DL_DECL_mspace_calloc
#define DL_DECL_mspace_calloc DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_calloc */
#ifndef DL_DECL_mspace_memalign
#define DL_DECL_mspace_memalign DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_memalign */
#ifndef DL_DECL_mspace_independent_calloc
#define DL_DECL_mspace_independent_calloc DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_independent_calloc */
#ifndef DL_DECL_mspace_independent_comalloc
#define DL_DECL_mspace_independent_comalloc DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_independent_comalloc */
#ifndef DL_DECL_mspace_bulk_free
#define DL_DECL_mspace_bulk_free DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_bulk_free */
#ifndef DL_DECL_mspace_footprint
#define DL_DECL_mspace_footprint DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_footprint */
#ifndef DL_DECL_mspace_max_footprint
#define DL_DECL_mspace_max_footprint DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_max_footprint */
#ifndef DL_DECL_mspace_footprint_limit
#define DL_DECL_mspace_footprint_limit DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_footprint_limit */
#ifndef DL_DECL_mspace_set_footprint_limit
#define DL_DECL_mspace_set_footprint_limit DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_set_footprint_limit */
#ifndef DL_DECL_mspace_mallinfo
#define DL_DECL_mspace_mallinfo DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_mallinfo */
#ifndef DL_DECL_mspace_usable_size
#define DL_DECL_mspace_usable_size DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_usable_size */
#ifndef DL_DECL_mspace_malloc_stats
#define DL_DECL_mspace_malloc_stats DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_malloc_stats */
#ifndef DL_DECL_mspace_trim
#define DL_DECL_mspace_trim DL_DECL_DEFAULT
#endif /* !DL_DECL_mspace_trim */
#ifndef DL_DECL_tls_mspace
#define DL_DECL_tls_mspace DL_DECL_DEFAULT
#endif /* !DL_DECL_tls_mspace */
#ifndef DL_DECL_tls_mspace_foreach
#define DL_DECL_tls_mspace_foreach DL_DECL_DEFAULT
#endif /* !DL_DECL_tls_mspace_foreach */
#ifndef DL_DECL_tls_mspace_destroy
#define DL_DECL_tls_mspace_destroy DL_DECL_DEFAULT
#endif /* !DL_DECL_tls_mspace_destroy */
#ifndef DL_IMPL_DEFAULT
#define DL_IMPL_DEFAULT /* nothing */
#endif /* !DL_IMPL_DEFAULT */
#ifndef DL_IMPL_dlmalloc
#define DL_IMPL_dlmalloc DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlmalloc */
#ifndef DL_IMPL_dlcalloc
#define DL_IMPL_dlcalloc DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlcalloc */
#ifndef DL_IMPL_dlfree
#define DL_IMPL_dlfree DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlfree */
#ifndef DL_IMPL_dlrealloc
#define DL_IMPL_dlrealloc DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlrealloc */
#ifndef DL_IMPL_dlmemalign
#define DL_IMPL_dlmemalign DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlmemalign */
#ifndef DL_IMPL_dlrealloc_in_place
#define DL_IMPL_dlrealloc_in_place DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlrealloc_in_place */
#ifndef DL_IMPL_dlposix_memalign
#define DL_IMPL_dlposix_memalign DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlposix_memalign */
#ifndef DL_IMPL_dlvalloc
#define DL_IMPL_dlvalloc DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlvalloc */
#ifndef DL_IMPL_dlmallopt
#define DL_IMPL_dlmallopt DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlmallopt */
#ifndef DL_IMPL_dlmalloc_footprint
#define DL_IMPL_dlmalloc_footprint DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlmalloc_footprint */
#ifndef DL_IMPL_dlmalloc_max_footprint
#define DL_IMPL_dlmalloc_max_footprint DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlmalloc_max_footprint */
#ifndef DL_IMPL_dlmalloc_footprint_limit
#define DL_IMPL_dlmalloc_footprint_limit DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlmalloc_footprint_limit */
#ifndef DL_IMPL_dlmalloc_set_footprint_limit
#define DL_IMPL_dlmalloc_set_footprint_limit DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlmalloc_set_footprint_limit */
#ifndef DL_IMPL_handler
#define DL_IMPL_handler DL_IMPL_DEFAULT
#endif /* !DL_IMPL_handler */
#ifndef DL_IMPL_dlmallinfo
#define DL_IMPL_dlmallinfo DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlmallinfo */
#ifndef DL_IMPL_dlindependent_calloc
#define DL_IMPL_dlindependent_calloc DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlindependent_calloc */
#ifndef DL_IMPL_dlindependent_comalloc
#define DL_IMPL_dlindependent_comalloc DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlindependent_comalloc */
#ifndef DL_IMPL_dlbulk_free
#define DL_IMPL_dlbulk_free DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlbulk_free */
#ifndef DL_IMPL_dlpvalloc
#define DL_IMPL_dlpvalloc DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlpvalloc */
#ifndef DL_IMPL_dlmalloc_trim
#define DL_IMPL_dlmalloc_trim DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlmalloc_trim */
#ifndef DL_IMPL_dlmalloc_stats
#define DL_IMPL_dlmalloc_stats DL_IMPL_DEFAULT
#endif /* !DL_IMPL_dlmalloc_stats */
#ifndef DL_IMPL_create_mspace
#define DL_IMPL_create_mspace DL_IMPL_DEFAULT
#endif /* !DL_IMPL_create_mspace */
#ifndef DL_IMPL_destroy_mspace
#define DL_IMPL_destroy_mspace DL_IMPL_DEFAULT
#endif /* !DL_IMPL_destroy_mspace */
#ifndef DL_IMPL_create_mspace_with_base
#define DL_IMPL_create_mspace_with_base DL_IMPL_DEFAULT
#endif /* !DL_IMPL_create_mspace_with_base */
#ifndef DL_IMPL_mspace_track_large_chunks
#define DL_IMPL_mspace_track_large_chunks DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_track_large_chunks */
#ifndef DL_IMPL_mspace_malloc
#define DL_IMPL_mspace_malloc DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_malloc */
#ifndef DL_IMPL_mspace_malloc_lockless
#define DL_IMPL_mspace_malloc_lockless DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_malloc_lockless */
#ifndef DL_IMPL_mspace_free
#define DL_IMPL_mspace_free DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_free */
#ifndef DL_IMPL_mspace_realloc
#define DL_IMPL_mspace_realloc DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_realloc */
#ifndef DL_IMPL_mspace_realloc_in_place
#define DL_IMPL_mspace_realloc_in_place DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_realloc_in_place */
#ifndef DL_IMPL_mspace_calloc
#define DL_IMPL_mspace_calloc DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_calloc */
#ifndef DL_IMPL_mspace_memalign
#define DL_IMPL_mspace_memalign DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_memalign */
#ifndef DL_IMPL_mspace_independent_calloc
#define DL_IMPL_mspace_independent_calloc DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_independent_calloc */
#ifndef DL_IMPL_mspace_independent_comalloc
#define DL_IMPL_mspace_independent_comalloc DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_independent_comalloc */
#ifndef DL_IMPL_mspace_bulk_free
#define DL_IMPL_mspace_bulk_free DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_bulk_free */
#ifndef DL_IMPL_mspace_footprint
#define DL_IMPL_mspace_footprint DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_footprint */
#ifndef DL_IMPL_mspace_max_footprint
#define DL_IMPL_mspace_max_footprint DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_max_footprint */
#ifndef DL_IMPL_mspace_footprint_limit
#define DL_IMPL_mspace_footprint_limit DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_footprint_limit */
#ifndef DL_IMPL_mspace_set_footprint_limit
#define DL_IMPL_mspace_set_footprint_limit DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_set_footprint_limit */
#ifndef DL_IMPL_mspace_mallinfo
#define DL_IMPL_mspace_mallinfo DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_mallinfo */
#ifndef DL_IMPL_mspace_usable_size
#define DL_IMPL_mspace_usable_size DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_usable_size */
#ifndef DL_IMPL_mspace_malloc_stats
#define DL_IMPL_mspace_malloc_stats DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_malloc_stats */
#ifndef DL_IMPL_mspace_trim
#define DL_IMPL_mspace_trim DL_IMPL_DEFAULT
#endif /* !DL_IMPL_mspace_trim */
#ifndef DL_IMPL_tls_mspace
#define DL_IMPL_tls_mspace DL_IMPL_DEFAULT
#endif /* !DL_IMPL_tls_mspace */
#ifndef DL_IMPL_tls_mspace_foreach
#define DL_IMPL_tls_mspace_foreach DL_IMPL_DEFAULT
#endif /* !DL_IMPL_tls_mspace_foreach */
#ifndef DL_IMPL_tls_mspace_destroy
#define DL_IMPL_tls_mspace_destroy DL_IMPL_DEFAULT
#endif /* !DL_IMPL_tls_mspace_destroy */
#ifndef DL_CC_DEFAULT
#define DL_CC_DEFAULT /* nothing */
#endif /* !DL_CC_DEFAULT */
#ifndef DL_CC_dlmalloc
#define DL_CC_dlmalloc DL_CC_DEFAULT
#endif /* !DL_CC_dlmalloc */
#ifndef DL_CC_dlcalloc
#define DL_CC_dlcalloc DL_CC_DEFAULT
#endif /* !DL_CC_dlcalloc */
#ifndef DL_CC_dlfree
#define DL_CC_dlfree DL_CC_DEFAULT
#endif /* !DL_CC_dlfree */
#ifndef DL_CC_dlrealloc
#define DL_CC_dlrealloc DL_CC_DEFAULT
#endif /* !DL_CC_dlrealloc */
#ifndef DL_CC_dlmemalign
#define DL_CC_dlmemalign DL_CC_DEFAULT
#endif /* !DL_CC_dlmemalign */
#ifndef DL_CC_dlrealloc_in_place
#define DL_CC_dlrealloc_in_place DL_CC_DEFAULT
#endif /* !DL_CC_dlrealloc_in_place */
#ifndef DL_CC_dlposix_memalign
#define DL_CC_dlposix_memalign DL_CC_DEFAULT
#endif /* !DL_CC_dlposix_memalign */
#ifndef DL_CC_dlvalloc
#define DL_CC_dlvalloc DL_CC_DEFAULT
#endif /* !DL_CC_dlvalloc */
#ifndef DL_CC_dlmallopt
#define DL_CC_dlmallopt DL_CC_DEFAULT
#endif /* !DL_CC_dlmallopt */
#ifndef DL_CC_dlmalloc_footprint
#define DL_CC_dlmalloc_footprint DL_CC_DEFAULT
#endif /* !DL_CC_dlmalloc_footprint */
#ifndef DL_CC_dlmalloc_max_footprint
#define DL_CC_dlmalloc_max_footprint DL_CC_DEFAULT
#endif /* !DL_CC_dlmalloc_max_footprint */
#ifndef DL_CC_dlmalloc_footprint_limit
#define DL_CC_dlmalloc_footprint_limit DL_CC_DEFAULT
#endif /* !DL_CC_dlmalloc_footprint_limit */
#ifndef DL_CC_dlmalloc_set_footprint_limit
#define DL_CC_dlmalloc_set_footprint_limit DL_CC_DEFAULT
#endif /* !DL_CC_dlmalloc_set_footprint_limit */
#ifndef DL_CC_handler
#define DL_CC_handler DL_CC_DEFAULT
#endif /* !DL_CC_handler */
#ifndef DL_CC_dlmallinfo
#define DL_CC_dlmallinfo DL_CC_DEFAULT
#endif /* !DL_CC_dlmallinfo */
#ifndef DL_CC_dlindependent_calloc
#define DL_CC_dlindependent_calloc DL_CC_DEFAULT
#endif /* !DL_CC_dlindependent_calloc */
#ifndef DL_CC_dlindependent_comalloc
#define DL_CC_dlindependent_comalloc DL_CC_DEFAULT
#endif /* !DL_CC_dlindependent_comalloc */
#ifndef DL_CC_dlbulk_free
#define DL_CC_dlbulk_free DL_CC_DEFAULT
#endif /* !DL_CC_dlbulk_free */
#ifndef DL_CC_dlpvalloc
#define DL_CC_dlpvalloc DL_CC_DEFAULT
#endif /* !DL_CC_dlpvalloc */
#ifndef DL_CC_dlmalloc_trim
#define DL_CC_dlmalloc_trim DL_CC_DEFAULT
#endif /* !DL_CC_dlmalloc_trim */
#ifndef DL_CC_dlmalloc_stats
#define DL_CC_dlmalloc_stats DL_CC_DEFAULT
#endif /* !DL_CC_dlmalloc_stats */
#ifndef DL_CC_create_mspace
#define DL_CC_create_mspace DL_CC_DEFAULT
#endif /* !DL_CC_create_mspace */
#ifndef DL_CC_destroy_mspace
#define DL_CC_destroy_mspace DL_CC_DEFAULT
#endif /* !DL_CC_destroy_mspace */
#ifndef DL_CC_create_mspace_with_base
#define DL_CC_create_mspace_with_base DL_CC_DEFAULT
#endif /* !DL_CC_create_mspace_with_base */
#ifndef DL_CC_mspace_track_large_chunks
#define DL_CC_mspace_track_large_chunks DL_CC_DEFAULT
#endif /* !DL_CC_mspace_track_large_chunks */
#ifndef DL_CC_mspace_malloc
#define DL_CC_mspace_malloc DL_CC_DEFAULT
#endif /* !DL_CC_mspace_malloc */
#ifndef DL_CC_mspace_malloc_lockless
#define DL_CC_mspace_malloc_lockless DL_CC_DEFAULT
#endif /* !DL_CC_mspace_malloc_lockless */
#ifndef DL_CC_mspace_free
#define DL_CC_mspace_free DL_CC_DEFAULT
#endif /* !DL_CC_mspace_free */
#ifndef DL_CC_mspace_realloc
#define DL_CC_mspace_realloc DL_CC_DEFAULT
#endif /* !DL_CC_mspace_realloc */
#ifndef DL_CC_mspace_realloc_in_place
#define DL_CC_mspace_realloc_in_place DL_CC_DEFAULT
#endif /* !DL_CC_mspace_realloc_in_place */
#ifndef DL_CC_mspace_calloc
#define DL_CC_mspace_calloc DL_CC_DEFAULT
#endif /* !DL_CC_mspace_calloc */
#ifndef DL_CC_mspace_memalign
#define DL_CC_mspace_memalign DL_CC_DEFAULT
#endif /* !DL_CC_mspace_memalign */
#ifndef DL_CC_mspace_independent_calloc
#define DL_CC_mspace_independent_calloc DL_CC_DEFAULT
#endif /* !DL_CC_mspace_independent_calloc */
#ifndef DL_CC_mspace_independent_comalloc
#define DL_CC_mspace_independent_comalloc DL_CC_DEFAULT
#endif /* !DL_CC_mspace_independent_comalloc */
#ifndef DL_CC_mspace_bulk_free
#define DL_CC_mspace_bulk_free DL_CC_DEFAULT
#endif /* !DL_CC_mspace_bulk_free */
#ifndef DL_CC_mspace_footprint
#define DL_CC_mspace_footprint DL_CC_DEFAULT
#endif /* !DL_CC_mspace_footprint */
#ifndef DL_CC_mspace_max_footprint
#define DL_CC_mspace_max_footprint DL_CC_DEFAULT
#endif /* !DL_CC_mspace_max_footprint */
#ifndef DL_CC_mspace_footprint_limit
#define DL_CC_mspace_footprint_limit DL_CC_DEFAULT
#endif /* !DL_CC_mspace_footprint_limit */
#ifndef DL_CC_mspace_set_footprint_limit
#define DL_CC_mspace_set_footprint_limit DL_CC_DEFAULT
#endif /* !DL_CC_mspace_set_footprint_limit */
#ifndef DL_CC_mspace_mallinfo
#define DL_CC_mspace_mallinfo DL_CC_DEFAULT
#endif /* !DL_CC_mspace_mallinfo */
#ifndef DL_CC_mspace_usable_size
#define DL_CC_mspace_usable_size DL_CC_DEFAULT
#endif /* !DL_CC_mspace_usable_size */
#ifndef DL_CC_mspace_malloc_stats
#define DL_CC_mspace_malloc_stats DL_CC_DEFAULT
#endif /* !DL_CC_mspace_malloc_stats */
#ifndef DL_CC_mspace_trim
#define DL_CC_mspace_trim DL_CC_DEFAULT
#endif /* !DL_CC_mspace_trim */
#ifndef DL_CC_tls_mspace
#define DL_CC_tls_mspace DL_CC_DEFAULT
#endif /* !DL_CC_tls_mspace */
#ifndef DL_CC_tls_mspace_foreach
#define DL_CC_tls_mspace_foreach DL_CC_DEFAULT
#endif /* !DL_CC_tls_mspace_foreach */
#ifndef DL_CC_tls_mspace_destroy
#define DL_CC_tls_mspace_destroy DL_CC_DEFAULT
#endif /* !DL_CC_tls_mspace_destroy */
/*[[[end]]]*/


/* Helper macros to declaring/implementing dlmalloc APIs */
#define DL_API_DECL(attr, Treturn, name, params) \
	DL_DECL_##name attr Treturn (DL_CC_##name name) params
#define DL_API_IMPL(attr, Treturn, name, params) \
	DL_IMPL_##name attr Treturn (DL_CC_##name name) params


/* Special behavioral modifications to make to various dlmalloc APIs */
#ifndef dlcalloc_SINGLE_PARAMETER
#define dlcalloc_SINGLE_PARAMETER 0
#endif /* !dlcalloc_SINGLE_PARAMETER */
#ifndef dlmallopt_SECOND_PARAMETER_IS_SIZE_T
#define dlmallopt_SECOND_PARAMETER_IS_SIZE_T 0
#endif /* !dlmallopt_SECOND_PARAMETER_IS_SIZE_T */
#ifndef dlmalloc_trim_RETURNS_SIZE_T
#define dlmalloc_trim_RETURNS_SIZE_T 0
#endif /* !dlmalloc_trim_RETURNS_SIZE_T */
#ifndef dlmalloc_set_footprint_limit_RETURNS_OLD_VALUE
#define dlmalloc_set_footprint_limit_RETURNS_OLD_VALUE 0
#endif /* !dlmalloc_set_footprint_limit_RETURNS_OLD_VALUE */
#ifndef dlrealloc_ZERO_BYTES_FREES
#define dlrealloc_ZERO_BYTES_FREES 0
#endif /* !dlrealloc_ZERO_BYTES_FREES */
#ifndef dlmalloc_ZERO_RETURNS_NULL
#define dlmalloc_ZERO_RETURNS_NULL 0
#endif /* !dlmalloc_ZERO_RETURNS_NULL */
#ifndef dlrealloc_CHECKS_NULL
#define dlrealloc_CHECKS_NULL 1
#endif /* !dlrealloc_CHECKS_NULL */
#ifndef dlfree_CHECKS_NULL
#define dlfree_CHECKS_NULL 1
#endif /* !dlfree_CHECKS_NULL */



/* DLMALLOC API */
#if !ONLY_MSPACES
DL_API_DECL(ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)), void *, dlmalloc, (size_t bytes));
#if dlcalloc_SINGLE_PARAMETER
DL_API_DECL(ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)), void *, dlcalloc, (size_t req));
#else /* dlcalloc_SINGLE_PARAMETER */
DL_API_DECL(ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1, 2)), void *, dlcalloc, (size_t n_elements, size_t elem_size));
#endif /* !dlcalloc_SINGLE_PARAMETER */
DL_API_DECL(, void, dlfree, (void *mem));
DL_API_DECL(WUNUSED, void *, dlrealloc, (void *oldmem, size_t bytes));
DL_API_DECL(ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)), void *, dlmemalign, (size_t alignment, size_t bytes));
DL_API_DECL(WUNUSED, void *, dlrealloc_in_place, (void *oldmem, size_t bytes));

#if !NO_POSIX_MEMALIGN
DL_API_DECL(, int, dlposix_memalign, (void **pp, size_t alignment, size_t bytes));
#endif /* !NO_POSIX_MEMALIGN */

#if !NO_VALLOC
DL_API_DECL(, void *, dlvalloc, (size_t bytes));
#endif /* !NO_VALLOC */

#if !NO_MALLOPT
#if dlmallopt_SECOND_PARAMETER_IS_SIZE_T
DL_API_DECL(, int, dlmallopt, (int option, size_t value));
#else /* dlmallopt_SECOND_PARAMETER_IS_SIZE_T */
DL_API_DECL(, int, dlmallopt, (int option, int value));
#endif /* !dlmallopt_SECOND_PARAMETER_IS_SIZE_T */
#endif /* !NO_MALLOPT */

#if !NO_MALLOC_FOOTPRINT
DL_API_DECL(ATTR_PURE WUNUSED, size_t, dlmalloc_footprint, (void));
DL_API_DECL(ATTR_PURE WUNUSED, size_t, dlmalloc_max_footprint, (void));
DL_API_DECL(ATTR_PURE WUNUSED, size_t, dlmalloc_footprint_limit, (void));
DL_API_DECL(, size_t, dlmalloc_set_footprint_limit, (size_t bytes));
#endif /* !NO_MALLOC_FOOTPRINT */

#if MALLOC_INSPECT_ALL
DL_API_DECL(, void, dlmalloc_inspect_all, (void (*handler)(void *start, void *end,
                                                           size_t used_bytes,
                                                           void *callback_arg),
                                           void *arg));
#endif /* MALLOC_INSPECT_ALL */

#if !NO_MALLINFO
DL_API_DECL(ATTR_PURE WUNUSED, STRUCT_mallinfo, dlmallinfo, (void));
#endif /* NO_MALLINFO */

#if !NO_INDEPENDENT_ALLOC
DL_API_DECL(, void **, dlindependent_calloc, (size_t n_elements, size_t elem_size, void *chunks[]));
DL_API_DECL(, void **, dlindependent_comalloc, (size_t n_elements, size_t sizes[], void *chunks[]));
#endif /* !NO_INDEPENDENT_ALLOC */

#if !NO_BULK_FREE
DL_API_DECL(, size_t, dlbulk_free, (void *array[], size_t nelem));
#endif /* !NO_BULK_FREE */

#if !NO_PVALLOC
DL_API_DECL(, void *, dlpvalloc, (size_t bytes));
#endif /* !NO_PVALLOC */

#if !NO_MALLOC_TRIM
#if dlmalloc_trim_RETURNS_SIZE_T
#define dlmalloc_trim_RETURN_TYPE size_t
#else /* dlmalloc_trim_RETURNS_SIZE_T */
#define dlmalloc_trim_RETURN_TYPE int
#endif /* !dlmalloc_trim_RETURNS_SIZE_T */
DL_API_DECL(, dlmalloc_trim_RETURN_TYPE, dlmalloc_trim, (size_t pad));
#endif /* !NO_MALLOC_TRIM */

#if !NO_MALLOC_STATS
DL_API_DECL(, void, dlmalloc_stats, (void));
#endif /* !NO_MALLOC_STATS */
#endif /* ONLY_MSPACES */

#if MSPACES
typedef void *mspace;

DL_API_DECL(, mspace, create_mspace, (size_t capacity/*, int locked*/));
#if !NO_DESTROY_MSPACE
DL_API_DECL(, size_t, destroy_mspace, (mspace msp));
#endif /* NO_DESTROY_MSPACE */
#if !NO_CREATE_MSPACE_WITH_BASE
DL_API_DECL(, mspace, create_mspace_with_base, (void *base, size_t capacity/*, int locked*/));
#endif /* !NO_CREATE_MSPACE_WITH_BASE */
#if !NO_MSPACE_TRACK_LARGE_CHUNKS
DL_API_DECL(, int, mspace_track_large_chunks, (mspace msp, int enable));
#endif /* !NO_MSPACE_TRACK_LARGE_CHUNKS */
DL_API_DECL(, void *, mspace_malloc, (mspace msp, size_t bytes));
#if USE_MSPACE_MALLOC_LOCKLESS
DL_API_DECL(, void *, mspace_malloc_lockless, (mspace msp, size_t bytes));
#endif /* USE_MSPACE_MALLOC_LOCKLESS */
#if !NO_MSPACE_FREE
DL_API_DECL(, void, mspace_free, (mspace msp, void *mem));
#endif /* !NO_MSPACE_FREE */
#if !NO_MSPACE_REALLOC
DL_API_DECL(, void *, mspace_realloc, (mspace msp, void *oldmem, size_t bytes));
#endif /* !NO_MSPACE_REALLOC */
#if !NO_MSPACE_REALLOC_IN_PLACE
DL_API_DECL(, void *, mspace_realloc_in_place, (mspace msp, void *oldmem, size_t bytes));
#endif /* !NO_MSPACE_REALLOC_IN_PLACE */
#if !NO_MSPACE_CALLOC
DL_API_DECL(, void *, mspace_calloc, (mspace msp, size_t n_elements, size_t elem_size));
#endif /* !NO_MSPACE_CALLOC */
#if !NO_MSPACE_MEMALIGN
DL_API_DECL(, void *, mspace_memalign, (mspace msp, size_t alignment, size_t bytes));
#endif /* !NO_MSPACE_MEMALIGN */
#if !NO_INDEPENDENT_ALLOC
DL_API_DECL(, void **, mspace_independent_calloc, (mspace msp, size_t n_elements, size_t elem_size, void *chunks[]));
DL_API_DECL(, void **, mspace_independent_comalloc, (mspace msp, size_t n_elements, size_t sizes[], void *chunks[]));
#endif /* !NO_INDEPENDENT_ALLOC */
#if !NO_BULK_FREE
DL_API_DECL(, size_t, mspace_bulk_free, (mspace msp, void *array[], size_t nelem));
#endif /* !NO_BULK_FREE */
#if !NO_MSPACE_FOOTPRINT
DL_API_DECL(, size_t, mspace_footprint, (mspace msp));
#endif /* !NO_MSPACE_FOOTPRINT */
#if !NO_MSPACE_MAX_FOOTPRINT
DL_API_DECL(, size_t, mspace_max_footprint, (mspace msp));
#endif /* !NO_MSPACE_MAX_FOOTPRINT */
#if !NO_MSPACE_FOOTPRINT_LIMIT
DL_API_DECL(, size_t, mspace_footprint_limit, (mspace msp));
#endif /* !NO_MSPACE_FOOTPRINT_LIMIT */
#if !NO_MSPACE_SET_FOOTPRINT_LIMIT
DL_API_DECL(, size_t, mspace_set_footprint_limit, (mspace msp, size_t bytes));
#endif /* !NO_MSPACE_SET_FOOTPRINT_LIMIT */
#if !NO_MSPACE_MALLINFO
DL_API_DECL(, STRUCT_mallinfo, mspace_mallinfo, (mspace msp));
#endif /* !NO_MSPACE_MALLINFO */
#if !NO_MSPACE_USABLE_SIZE
DL_API_DECL(, size_t, mspace_usable_size, (void const *mem));
#endif /* !NO_MSPACE_USABLE_SIZE */
#if !NO_MALLOC_STATS
DL_API_DECL(, void, mspace_malloc_stats, (mspace msp));
#endif /* NO_MALLOC_STATS */
#if !NO_MSPACE_TRIM
DL_API_DECL(, dlmalloc_trim_RETURN_TYPE, mspace_trim, (mspace msp, size_t pad));
#endif /* NO_MSPACE_TRIM */

#if USE_PER_THREAD_MSTATE
/* Return the calling thread's thread-local mspace (or "NULL" if not available) */
DL_API_DECL(, mspace, tls_mspace, (void));

/* Destroy a lazily-allocated TLS mspace. This function must be called by the
 * per-thread finalize for the TLS variable returned by `DL_TLS_GETHEAP()',
 * when said variable is non-NULL and the relevant thread has/is exiting. */
DL_API_DECL(NONNULL((1)), void, tls_mspace_destroy, (mspace ms));

/* Invoke `cb' for every existing `tls_mspace()' ("cb" must still PREACTION()-
 * lock said space if doing so is necessary) -- this function is used by some
 * of the other dlmalloc APIs to apply configurations / trim heaps for all TLS
 * heaps when the relevant action is being performed for the "gm" heap. */
DL_API_DECL(, size_t, tls_mspace_foreach, (size_t (*cb)(mspace ms, void *arg), void *arg));
#endif /* USE_PER_THREAD_MSTATE */
#endif /* MSPACES */



/* DLMALLOC API HOOKS */
#ifndef MALLOC_FAILURE_ACTION
#define MALLOC_FAILURE_ACTION errno = ENOMEM;
#endif /* !MALLOC_FAILURE_ACTION */
#ifndef dlmalloc_trim_PREHOOK
#define dlmalloc_trim_PREHOOK(p_result, pad) (void)0
#endif /* dlmalloc_trim_PREHOOK */
#ifndef dlmalloc_trim_POSTHOOK
#define dlmalloc_trim_POSTHOOK(p_result, pad) (void)0
#endif /* !dlmalloc_trim_POSTHOOK */



/* Data types for the heap region extension */
#if FLAG4_BIT_HEAP_REGION
#ifndef STRUCT_dlheapregion
#define STRUCT_dlheapchunk struct dlheapchunk
struct /*ATTR_ALIGNED(MALLOC_ALIGNMENT)*/ dlheapchunk {
	size_t hc_prevsize; /* Size of previous chunk, or "0" if this is the first chunk; must be multiple of "MALLOC_ALIGNMENT" */
	size_t hc_head;     /* Size of this chunk (including this header), but least significant
	                     * 3 bits have special meaning and (when the chunk is allocated, which
	                     * is the only mode explicitly defined by this binary format), must be
	                     * initialized to 0b100 (aka. "4") */
};

#define STRUCT_dlheaptail struct dlheaptail
struct /*ATTR_ALIGNED(MALLOC_ALIGNMENT)*/ dlheaptail {
	size_t ht_lastsize; /* Size of the last *real* chunk within the region (in bytes); must be multiple of "MALLOC_ALIGNMENT" */
	size_t ht_zero;     /* Always zero (but see `FLAG4_BIT_HEAP_REGION_DBG_DISPOSE') */
};

#define STRUCT_dlheapregion struct dlheapregion
struct /*ATTR_ALIGNED(MALLOC_ALIGNMENT)*/ dlheapregion {
	size_t             hr_size;  /* [const][== offsetafter(dlheapregion, hr_tail)]
	                              * Total region size (in bytes, including this header, and the tail) */
	/* [1..1][const] Destructor invoked once the region's last chunk is dlfree()'d */
	void (DCALL       *hr_destroy)(STRUCT_dlheapregion *__restrict self);
	STRUCT_dlheapchunk hr_first;                                /* First chunk of region */
//	byte_t             hr_data[hr_size - (6 * sizeof(void *))]; /* Region payload (containing more chunks...) */
//	STRUCT_dlheaptail  hr_tail;                                 /* Region tail (see above) */
};
#endif /* !STRUCT_dlheapregion */
#endif /* FLAG4_BIT_HEAP_REGION */



/* System configuration -- figure out how we want to implement the heap backend */
#undef HAVE_MMAP
#undef HAVE_MMAP_IS_VirtualAlloc
#undef HAVE_MMAP_IS_malloc
#undef HAVE_MREMAP
#undef HAVE_MORECORE
#undef HAVE_MMAP_CLEARS
#if DL_CONFIG_HAVE_VirtualAlloc
#define HAVE_MMAP 1
#define HAVE_MMAP_IS_VirtualAlloc /* Windows: use VirtualAlloc() */
#elif DL_CONFIG_HAVE_mmap || DL_CONFIG_HAVE_sbrk
#define HAVE_MMAP     DL_CONFIG_HAVE_mmap /* Unix: use mmap(2)... */
#define HAVE_MREMAP   DL_CONFIG_HAVE_mremap
#define HAVE_MORECORE DL_CONFIG_HAVE_sbrk /* Unix: ... and sbrk(2) */
#elif DL_CONFIG_HAVE_malloc || DL_CONFIG_HAVE_calloc
#define HAVE_MMAP 1
#define HAVE_MMAP_IS_malloc       /* Generic: use the host's native malloc(3) */
#define HAVE_MMAP_CLEARS DL_CONFIG_HAVE_calloc
#else /* ... */
/* No way to implement dlmalloc() -- memory allocation will just fail at runtime :( */
#endif /* !... */

#ifndef HAVE_MMAP_CLEARS
#define HAVE_MMAP_CLEARS 1
#endif /* !HAVE_MMAP_CLEARS */
#ifndef HAVE_MMAP
#define HAVE_MMAP 0
#endif /* !HAVE_MMAP */
#ifndef HAVE_MORECORE
#define HAVE_MORECORE 0
#endif /* !HAVE_MORECORE */
#ifndef HAVE_MREMAP
#define HAVE_MREMAP 0
#endif /* !HAVE_MREMAP */

#if !HAVE_MORECORE
#define MORECORE_CONTIGUOUS 0
#else /* !HAVE_MORECORE */
#define MORECORE_CONTIGUOUS 1
#endif /* HAVE_MORECORE */

#ifndef DEFAULT_GRANULARITY
#if (MORECORE_CONTIGUOUS || defined(HAVE_MMAP_IS_VirtualAlloc))
#define DEFAULT_GRANULARITY_IS_ZERO 1
#define DEFAULT_GRANULARITY 0 /* 0 means to compute in init_mparams */
#else /* MORECORE_CONTIGUOUS */
#define DEFAULT_GRANULARITY (SIZE_C(64) * SIZE_C(1024))
#endif /* MORECORE_CONTIGUOUS */
#endif /* DEFAULT_GRANULARITY */
#ifndef DEFAULT_GRANULARITY_IS_ZERO
#define DEFAULT_GRANULARITY_IS_ZERO 0
#endif /* !DEFAULT_GRANULARITY_IS_ZERO */

#ifndef MORECORE_CANNOT_TRIM
#define DEFAULT_TRIM_THRESHOLD (SIZE_C(2) * SIZE_C(1024) * SIZE_C(1024))
#else /* MORECORE_CANNOT_TRIM */
#define DEFAULT_TRIM_THRESHOLD SIZE_MAX
#endif /* MORECORE_CANNOT_TRIM */

#if HAVE_MMAP
#define DEFAULT_MMAP_THRESHOLD (SIZE_C(256) * SIZE_C(1024))
#define MAX_RELEASE_CHECK_RATE 4095
#else /* HAVE_MMAP */
#define DEFAULT_MMAP_THRESHOLD SIZE_MAX
#define MAX_RELEASE_CHECK_RATE SIZE_MAX
#endif /* !HAVE_MMAP */




/*------------------------------ internal #includes ---------------------- */
#ifdef __ARCH_PAGESIZE
#define malloc_getpagesize __ARCH_PAGESIZE
#define MALLOC_PAGESIZE    __ARCH_PAGESIZE
#elif DL_CONFIG_HAVE_getpagesize
#define malloc_getpagesize getpagesize()
#else /* __ARCH_PAGESIZE */
#if !defined(_SC_PAGE_SIZE) && defined(_SC_PAGESIZE)
#define _SC_PAGE_SIZE _SC_PAGESIZE
#endif /* !_SC_PAGE_SIZE && _SC_PAGESIZE */
#if defined(_SC_PAGE_SIZE) && DL_CONFIG_HAVE_sysconf
#define malloc_getpagesize sysconf(_SC_PAGE_SIZE)
#else /* _SC_PAGE_SIZE && DL_CONFIG_HAVE_sysconf */
#if DL_CONFIG_HAVE_SYS_PARAM_H
DECL_END
#include <sys/param.h>
DECL_BEGIN
#endif /* DL_CONFIG_HAVE_SYS_PARAM_H */
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
#endif /* !_SC_PAGE_SIZE || !DL_CONFIG_HAVE_sysconf */
#endif /* !__ARCH_PAGESIZE */

/* ------------------- size_t and alignment properties -------------------- */

/* The byte and bit size of a size_t */
#define SIZE_T_SIZE    __SIZEOF_SIZE_T__
#define SIZE_T_BITSIZE (__SIZEOF_SIZE_T__ << 3)

/* Some constants coerced to size_t */
/* Annoying but necessary to avoid errors on some platforms */
#define SIZE_T_ZERO       SIZE_C(0)
#define SIZE_T_ONE        SIZE_C(1)
#define SIZE_T_TWO        SIZE_C(2)
#define SIZE_T_FOUR       SIZE_C(4)
#define TWO_SIZE_T_SIZES  (SIZE_T_SIZE << 1)
#define FOUR_SIZE_T_SIZES (SIZE_T_SIZE << 2)
#define SIX_SIZE_T_SIZES  (FOUR_SIZE_T_SIZES + TWO_SIZE_T_SIZES)
#define HALF_MAX_SIZE_T   (SIZE_MAX / 2U)

/* The bit mask value corresponding to MALLOC_ALIGNMENT */
#define CHUNK_ALIGN_MASK (MALLOC_ALIGNMENT - SIZE_T_ONE)

/* True if address a has acceptable alignment */
#define is_aligned(A) (((size_t)((A)) & (CHUNK_ALIGN_MASK)) == 0)

/* the number of bytes to offset an address to align it */
#define align_offset(A)                      \
	((((size_t)(A) & CHUNK_ALIGN_MASK) == 0) \
	 ? 0                                     \
	 : ((MALLOC_ALIGNMENT - ((size_t)(A) & CHUNK_ALIGN_MASK)) & CHUNK_ALIGN_MASK))

/* -------------------------- MMAP preliminaries ------------------------- */

/* MORECORE and MMAP must return MFAIL on failure */
#define MFAIL  ((void *)(SIZE_MAX))
#define CMFAIL ((byte_t *)(MFAIL)) /* defined for convenience */

#if HAVE_MMAP
#ifdef HAVE_MMAP_IS_VirtualAlloc
/* Win32 MMAP via VirtualAlloc */
FORCELOCAL void *win32mmap(size_t size) {
	void *ptr;
	DBG_ALIGNMENT_DISABLE();
	ptr = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	DBG_ALIGNMENT_ENABLE();
	return (ptr != NULL) ? ptr : MFAIL;
}

/* For direct MMAP, use MEM_TOP_DOWN to minimize interference */
FORCELOCAL void *win32direct_mmap(size_t size) {
	void *ptr;
	DBG_ALIGNMENT_DISABLE();
	ptr = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE);
	DBG_ALIGNMENT_ENABLE();
	return (ptr != NULL) ? ptr : MFAIL;
}

/* This function supports releasing coalesced segments */
FORCELOCAL int win32munmap(void *ptr, size_t size) {
	MEMORY_BASIC_INFORMATION minfo;
	byte_t *cptr = (byte_t *)ptr;
	while (size) {
		DBG_ALIGNMENT_DISABLE();
		if (VirtualQuery((void *)cptr, &minfo, sizeof(minfo)) == 0) {
			DBG_ALIGNMENT_ENABLE();
			return -1;
		}
		DBG_ALIGNMENT_ENABLE();
		if (minfo.BaseAddress != (void *)cptr ||
		    minfo.AllocationBase != (void *)cptr ||
		    minfo.State != MEM_COMMIT || minfo.RegionSize > size)
			return -1;
		if (!VirtualFree((void *)cptr, 0, MEM_RELEASE))
			return -1;
		cptr += minfo.RegionSize;
		size -= minfo.RegionSize;
	}
	return 0;
}

#define DL_MMAP(s)        win32mmap(s)
#define DL_MUNMAP(a, s)   win32munmap((a), (s))
#define DL_DIRECT_MMAP(s) win32direct_mmap(s)
#elif defined(HAVE_MMAP_IS_malloc)

/* >>  NO_SEGMENT_TRAVERSAL       default: 0
 * >>    If non-zero, suppresses traversals of memory segments
 * >>    returned by either MORECORE or CALL_MMAP. This disables
 * >>    merging of segments that are contiguous, and selectively
 * >>    releasing them to the OS if unused, but bounds execution times.
 *
 * In this case, we **actually** want to disable merging of segments.
 * Because segments originate from another malloc() impl, we have to
 * free() every segment one-at-a-time, so merging segments would just
 * be counter-productive! */
#undef NO_SEGMENT_TRAVERSAL
#define NO_SEGMENT_TRAVERSAL 1

/* Use native malloc() to simulate mmap() */
#define DL_MMAP(s) native_malloc_mmap(s)
FORCELOCAL void *native_malloc_mmap(size_t size) {
	void *ptr;
	DBG_ALIGNMENT_DISABLE();
#if DL_CONFIG_HAVE_calloc
	ptr = calloc(1, size);
#else /* DL_CONFIG_HAVE_calloc */
	ptr = malloc(size);
#endif /* !DL_CONFIG_HAVE_calloc */
	DBG_ALIGNMENT_ENABLE();
	return (ptr != NULL) ? ptr : MFAIL;
}

/* NOTE: We can just use "free()" if it's available
 *       because we configure "NO_SEGMENT_TRAVERSAL"! */
#if DL_CONFIG_HAVE_free
#define DL_MUNMAP(a, s) native_malloc_munmap((a), (s))
FORCELOCAL int native_malloc_munmap(void *ptr, size_t size) {
	(void)size;
	DBG_ALIGNMENT_DISABLE();
	free(ptr);
	DBG_ALIGNMENT_ENABLE();
	return 0;
}
#endif /* DL_CONFIG_HAVE_free */
#else /* ... */
#if DL_CONFIG_HAVE_munmap
#define DL_MUNMAP(a, s) munmap((a), (s))
#endif /* DL_CONFIG_HAVE_munmap */
#if !DL_CONFIG_HAVE_PROT_READ
#undef PROT_READ
#define PROT_READ 0
#endif /* !DL_CONFIG_HAVE_PROT_READ */
#if !DL_CONFIG_HAVE_PROT_WRITE
#undef PROT_WRITE
#define PROT_WRITE 0
#endif /* !DL_CONFIG_HAVE_PROT_WRITE */
#if !DL_CONFIG_HAVE_MAP_PRIVATE
#undef MAP_PRIVATE
#define MAP_PRIVATE 0
#endif /* !DL_CONFIG_HAVE_MAP_PRIVATE */
#define MMAP_PROT (PROT_READ | PROT_WRITE)
#if DL_CONFIG_HAVE_MAP_ANONYMOUS
#define MMAP_FLAGS (MAP_PRIVATE | MAP_ANONYMOUS)
#define DL_MMAP(s) mmap(NULL, (s), MMAP_PROT, MMAP_FLAGS, -1, 0)
#else /* DL_CONFIG_HAVE_MAP_ANONYMOUS */
/* Nearly all versions of mmap support MAP_ANONYMOUS, so the following
 * is unlikely to be needed, but is supplied just in case. */
#define MMAP_FLAGS (MAP_PRIVATE)
static int dev_zero_fd = -1; /* Cached file descriptor for /dev/zero. */
#define DL_MMAP(s)                                                                \
	((dev_zero_fd < 0) ? (dev_zero_fd = open("/dev/zero", O_RDWR),                \
	                      mmap(NULL, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0)) \
	                   : mmap(NULL, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0))
#endif /* !DL_CONFIG_HAVE_MAP_ANONYMOUS */
#endif /* !... */
#endif /* HAVE_MMAP */


/**
 * Define DL_MMAP/DL_MUNMAP/DL_DIRECT_MMAP
 */
#if HAVE_MMAP
#define USE_MMAP_BIT    (SIZE_T_ONE)
#else /* HAVE_MMAP */
#define USE_MMAP_BIT    (SIZE_T_ZERO)
#define DL_MMAP(s)      MFAIL
#endif /* HAVE_MMAP */
#ifndef DL_MUNMAP
#define DL_MUNMAP(a, s) (-1)
#define DL_MUNMAP_ALWAYS_FAILS 1
#endif /* !DL_MUNMAP */
#ifndef DL_MUNMAP_ALWAYS_FAILS
#define DL_MUNMAP_ALWAYS_FAILS 0
#endif /* !DL_MUNMAP_ALWAYS_FAILS */
#ifndef DL_DIRECT_MMAP
#define DL_DIRECT_MMAP(s) DL_MMAP(s)
#endif /* !DL_DIRECT_MMAP */

/**
 * Define DL_MREMAP
 */
#if HAVE_MMAP && HAVE_MREMAP
#if DL_CONFIG_HAVE_MREMAP_MAYMOVE
#define DL_MREMAP(addr, osz, nsz, mv) mremap((addr), (osz), (nsz), (mv) ? MREMAP_MAYMOVE : 0)
#else /* DL_CONFIG_HAVE_MREMAP_MAYMOVE */
#define DL_MREMAP(addr, osz, nsz, mv) mremap((addr), (osz), (nsz), (mv))
#endif /* !DL_CONFIG_HAVE_MREMAP_MAYMOVE */
#else /* HAVE_MMAP && HAVE_MREMAP */
#define DL_MREMAP(addr, osz, nsz, mv) MFAIL
#define DL_MREMAP_ALWAYS_FAILS 1
#endif /* HAVE_MMAP && HAVE_MREMAP */
#ifndef DL_MREMAP_ALWAYS_FAILS
#define DL_MREMAP_ALWAYS_FAILS 0
#endif /* !DL_MREMAP_ALWAYS_FAILS */

#define USE_NONCONTIGUOUS_BIT (4U) /* mstate bit set if continguous morecore disabled or failed */
#if MSPACES && !NO_CREATE_MSPACE_WITH_BASE
#define EXTERN_BIT            (8U) /* segment bit set in create_mspace_with_base */
#else /* MSPACES && !NO_CREATE_MSPACE_WITH_BASE */
#define EXTERN_BIT            (0U)
#endif /* !MSPACES || NO_CREATE_MSPACE_WITH_BASE */


/* --------------------------- Lock preliminaries ------------------------ */
#ifndef DL_LOCK_T
DECL_END
#include <hybrid/sched/atomic-lock.h> /* ATOMIC_LOCK_INIT, atomic_lock, atomic_lock_* */
DECL_BEGIN
#define DL_LOCK_T           struct atomic_lock     /* Mand */
#define DL_LOCK_INIT_STATIC ATOMIC_LOCK_INIT       /* Mand */
#define DL_LOCK_INIT        atomic_lock_cinit      /* Mand */
#define DL_LOCK_ACQUIRE     atomic_lock_acquire    /* Mand */
#define DL_LOCK_RELEASE     atomic_lock_release    /* Mand */
#define DL_LOCK_TRYACQUIRE  atomic_lock_tryacquire /* Mand */
#define DL_LOCK_ACQUIRED    atomic_lock_acquired   /* Opt */
#define DL_LOCK_AVAILABLE   atomic_lock_available  /* Opt */
#define DL_LOCK_WAITFOR     atomic_lock_waitfor    /* Opt */
#endif /* !DL_LOCK_T */
#ifndef DL_LOCK_WAITFOR
#define DL_LOCK_WAITFOR(x)  \
	do {                    \
		DL_LOCK_ACQUIRE(x); \
		DL_LOCK_RELEASE(x); \
	} __WHILE0
#endif /* !DL_LOCK_WAITFOR */
#if !USE_LOCKS
#undef DL_LOCK_T
#undef DL_LOCK_INIT_STATIC
#undef DL_LOCK_INIT
#undef DL_LOCK_ACQUIRE
#undef DL_LOCK_RELEASE
#undef DL_LOCK_TRYACQUIRE
#undef DL_LOCK_ACQUIRED
#undef DL_LOCK_AVAILABLE
#undef DL_LOCK_WAITFOR
#define DL_LOCK_T             char
#define DL_LOCK_INIT_STATIC   0
#define DL_LOCK_INIT(x)       (void)0
#define DL_LOCK_ACQUIRE(x)    (void)0
#define DL_LOCK_RELEASE(x)    (void)0
#define DL_LOCK_TRYACQUIRE(x) 1
#define DL_LOCK_ACQUIRED(x)   1
#define DL_LOCK_AVAILABLE(x)  1
#define DL_LOCK_WAITFOR(x)    (void)0
#endif /* !USE_LOCKS */

#if GM_ONLY || !USE_LOCKS
#define ACQUIRE_MALLOC_GLOBAL_LOCK() (void)0
#define RELEASE_MALLOC_GLOBAL_LOCK() (void)0
#else /* GM_ONLY || !USE_LOCKS */
static DL_LOCK_T malloc_global_lock = DL_LOCK_INIT_STATIC;
#define ACQUIRE_MALLOC_GLOBAL_LOCK() DL_LOCK_ACQUIRE(&malloc_global_lock)
#define RELEASE_MALLOC_GLOBAL_LOCK() DL_LOCK_RELEASE(&malloc_global_lock)
#endif /* !GM_ONLY && USE_LOCKS */

/* -----------------------  Chunk representations ------------------------ */
struct malloc_chunk {
	size_t               prev_foot; /* Size of previous chunk (if free).  */
	size_t               head;      /* Size and inuse bits. */
	struct malloc_chunk *fd;        /* double links -- used only if free. */
	struct malloc_chunk *bk;
};

typedef struct malloc_chunk mchunk;
typedef struct malloc_chunk *mchunkptr;
typedef struct malloc_chunk *sbinptr; /* The type of bins of chunks */
typedef unsigned int bindex_t;        /* Described below */
typedef unsigned int binmap_t;        /* Described below */
typedef unsigned int flag_t;          /* The type of various bit flag sets */

/* ------------------- Chunks sizes and alignments ----------------------- */
#define MCHUNK_SIZE (sizeof(mchunk))
#if FOOTERS
#define CHUNK_OVERHEAD (TWO_SIZE_T_SIZES)
#else /* FOOTERS */
#define CHUNK_OVERHEAD (SIZE_T_SIZE)
#endif /* FOOTERS */

/* MMapped chunks need a second word of overhead ... */
#define MMAP_CHUNK_OVERHEAD (TWO_SIZE_T_SIZES)
/* ... and additional padding for fake next-chunk at foot */
#define MMAP_FOOT_PAD       (FOUR_SIZE_T_SIZES)

/* The smallest size we can malloc is an aligned minimal chunk */
#define MIN_CHUNK_SIZE ((MCHUNK_SIZE + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)

/* conversion from malloc headers to user pointers, and back */
#define chunk2mem(p)   ((void *)((byte_t *)(p) + TWO_SIZE_T_SIZES))
#define mem2chunk(mem) ((mchunkptr)((byte_t *)(mem) - TWO_SIZE_T_SIZES))

/* chunk associated with aligned address A */
#define align_as_chunk(A) (mchunkptr)((A) + align_offset(chunk2mem(A)))

/* Bounds on request (not chunk) sizes. */
#define MAX_REQUEST ((SIZE_T_ZERO - MIN_CHUNK_SIZE) << 2)
#define MIN_REQUEST (MIN_CHUNK_SIZE - CHUNK_OVERHEAD - SIZE_T_ONE)

/* pad request bytes into a usable size */
#define pad_request(req) \
	(((req) + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)

/* pad request, checking for minimum (but not maximum) */
#define request2size(req) \
	(((req) < MIN_REQUEST) ? MIN_CHUNK_SIZE : pad_request(req))


/* ------------------ Operations on head and foot fields ----------------- */

#define PINUSE_BIT (SIZE_T_ONE)
#define CINUSE_BIT (SIZE_T_TWO)
#define FLAG4_BIT  (SIZE_T_FOUR)
#define INUSE_BITS (PINUSE_BIT | CINUSE_BIT)
#define FLAG_BITS  (PINUSE_BIT | CINUSE_BIT | FLAG4_BIT)

/* Head value for fenceposts */
#define FENCEPOST_HEAD (INUSE_BITS | SIZE_T_SIZE)

/* extraction of fields from head words */
#define cinuse(p)     ((p)->head & CINUSE_BIT)
#define pinuse(p)     ((p)->head & PINUSE_BIT)
#define flag4inuse(p) ((p)->head & FLAG4_BIT)
#define is_inuse(p)   (((p)->head & INUSE_BITS) != PINUSE_BIT)
#define is_mmapped(p) (((p)->head & INUSE_BITS) == 0)

#define chunksize(p) ((p)->head & ~(FLAG_BITS))

#define clear_pinuse(p) ((p)->head &= ~PINUSE_BIT)
#define set_flag4(p)    ((p)->head |= FLAG4_BIT)
#define clear_flag4(p)  ((p)->head &= ~FLAG4_BIT)

/* Treat space at ptr +/- offset as a chunk */
#define chunk_plus_offset(p, s)  ((mchunkptr)((byte_t *)(p) + (s)))
#define chunk_minus_offset(p, s) ((mchunkptr)((byte_t *)(p) - (s)))

/* Ptr to next or previous physical malloc_chunk. */
#define next_chunk(p) chunk_plus_offset(p, chunksize(p))
#define prev_chunk(p) chunk_minus_offset(p, (p)->prev_foot)

/* extract next chunk's pinuse bit */
#define next_pinuse(p) ((next_chunk(p)->head) & PINUSE_BIT)

/* Get/set size at footer */
#define get_foot(p, s) (chunk_plus_offset(p, s)->prev_foot)
#define set_foot(p, s) (chunk_plus_offset(p, s)->prev_foot = (s))

/* Set size, pinuse bit, and foot */
#define set_size_and_pinuse_of_free_chunk(p, s) \
	((p)->head = (s | PINUSE_BIT), set_foot(p, s))

/* Set size, pinuse bit, foot, and clear next pinuse */
#define set_free_with_pinuse(p, s, n) \
	(clear_pinuse(n), set_size_and_pinuse_of_free_chunk(p, s))

/* Get the internal overhead associated with chunk p */
#if FLAG4_BIT_HEAP_REGION
#if MMAP_CHUNK_OVERHEAD == TWO_SIZE_T_SIZES && MMAP_CHUNK_OVERHEAD == CHUNK_OVERHEAD
#define overhead_for(p) MMAP_CHUNK_OVERHEAD
#elif MMAP_CHUNK_OVERHEAD == TWO_SIZE_T_SIZES
#define overhead_for(p) (is_mmapped(p) ? MMAP_CHUNK_OVERHEAD : CHUNK_OVERHEAD)
#else /* ... */
#define overhead_for(p) (is_mmapped(p) ? (flag4inuse(p) ? TWO_SIZE_T_SIZES : MMAP_CHUNK_OVERHEAD) : CHUNK_OVERHEAD)
#endif /* !... */
#elif MMAP_CHUNK_OVERHEAD == CHUNK_OVERHEAD
#define overhead_for(p) MMAP_CHUNK_OVERHEAD
#else /* ... */
#define overhead_for(p) (is_mmapped(p) ? MMAP_CHUNK_OVERHEAD : CHUNK_OVERHEAD)
#endif /* !... */

/* Return true if malloced space is not necessarily cleared */
#if HAVE_MMAP_CLEARS && !DL_DEBUG_MEMSET_ALLOC
#define calloc_must_clear(p) (!is_mmapped(p))
#else /* HAVE_MMAP_CLEARS && !DL_DEBUG_MEMSET_ALLOC */
#define calloc_must_clear(p) (1)
#endif /* !HAVE_MMAP_CLEARS || DL_DEBUG_MEMSET_ALLOC */

/* ---------------------- Overlaid data structures ----------------------- */

struct malloc_tree_chunk {
	/* The first four fields must be compatible with malloc_chunk */
	size_t                    prev_foot;
	size_t                    head;
	struct malloc_tree_chunk *fd;
	struct malloc_tree_chunk *bk;

	struct malloc_tree_chunk *child[2];
	struct malloc_tree_chunk *parent;
#if DL_DEBUG_MEMSET_FREE
	union {
		bindex_t              index;
		size_t                index_word;
	};
#else /* DL_DEBUG_MEMSET_FREE */
	bindex_t                  index;
#endif /* !DL_DEBUG_MEMSET_FREE */
};

typedef struct malloc_tree_chunk tchunk;
typedef struct malloc_tree_chunk *tchunkptr;
typedef struct malloc_tree_chunk *tbinptr; /* The type of bins of trees */

/* A little helper macro for trees */
#define leftmost_child(t) ((t)->child[0] != 0 ? (t)->child[0] : (t)->child[1])

/* ----------------------------- Segments -------------------------------- */

struct malloc_segment {
	byte_t                *base;   /* base address */
	size_t                 size;   /* allocated size */
	struct malloc_segment *next;   /* ptr to next segment */
	flag_t                 sflags; /* mmap and extern flag */
};

#define is_mmapped_segment(S) ((S)->sflags & USE_MMAP_BIT)
#if EXTERN_BIT
#define is_extern_segment(S) ((S)->sflags & EXTERN_BIT)
#else /* EXTERN_BIT */
#define is_extern_segment(S) 0
#endif /* !EXTERN_BIT */

typedef struct malloc_segment msegment;
typedef struct malloc_segment *msegmentptr;

/* ----------------------------- Free List -------------------------------- */

#if USE_PENDING_FREE_LIST
struct freelist_entry {
	SLIST_ENTRY(freelist_entry) fle_link;
};
SLIST_HEAD(freelist, freelist_entry);
#endif /* USE_PENDING_FREE_LIST */


/* ---------------------------- malloc_state ----------------------------- */

/* Bin types, widths and sizes */
#define NSMALLBINS        (32U)
#define NTREEBINS         (32U)
#define SMALLBIN_SHIFT    (3U)
#define SMALLBIN_WIDTH    (SIZE_T_ONE << SMALLBIN_SHIFT)
#define TREEBIN_SHIFT     (8U)
#define MIN_LARGE_SIZE    (SIZE_T_ONE << TREEBIN_SHIFT)
#define MAX_SMALL_SIZE    (MIN_LARGE_SIZE - SIZE_T_ONE)
#define MAX_SMALL_REQUEST (MAX_SMALL_SIZE - CHUNK_ALIGN_MASK - CHUNK_OVERHEAD)

#if GM_ONLY
static binmap_t /* */ dl_gm_smallmap = 0;
static binmap_t /* */ dl_gm_treemap = 0;
static size_t /*   */ dl_gm_dvsize = 0;
static size_t /*   */ dl_gm_topsize = 0;
static byte_t * /* */ dl_gm_least_addr = 0;
static mchunkptr /**/ dl_gm_dv = NULL;
static mchunkptr /**/ dl_gm_top = NULL;
static size_t /*   */ dl_gm_trim_check = 0;
static size_t /*   */ dl_gm_release_checks = 0;
static size_t /*   */ dl_gm_magic = 0;
static mchunkptr /**/ dl_gm_smallbins[(NSMALLBINS + 1) * 2] = {};
static tbinptr /*  */ dl_gm_treebins[NTREEBINS] = {};
static size_t /*   */ dl_gm_footprint = 0;
static size_t /*   */ dl_gm_max_footprint = 0;
static size_t /*   */ dl_gm_footprint_limit = 0; /* zero means no limit */
static flag_t /*   */ dl_gm_mflags = 0;
static msegment /* */ dl_gm_seg = {};
#if USE_LOCKS
static DL_LOCK_T dl_gm_mutex = DL_LOCK_INIT_STATIC;
#endif /* USE_LOCKS */
#if USE_PENDING_FREE_LIST
/* [0..n][lock(ATOMIC)] List of heap pointers that still need to be free'd */
static struct freelist dl_gm_flist = SLIST_HEAD_INITIALIZER(dl_gm_flist);
#endif /* USE_PENDING_FREE_LIST */

#define GM_STATIC_INIT_MUTEX 1 /* Static initialization is enough for locking to work */

#define mstate_smallmap(M)        dl_gm_smallmap
#define mstate_treemap(M)         dl_gm_treemap
#define mstate_dvsize(M)          dl_gm_dvsize
#define mstate_topsize(M)         dl_gm_topsize
#define mstate_least_addr(M)      dl_gm_least_addr
#define mstate_dv(M)              dl_gm_dv
#define mstate_top(M)             dl_gm_top
#define mstate_trim_check(M)      dl_gm_trim_check
#define mstate_release_checks(M)  dl_gm_release_checks
#define mstate_magic(M)           dl_gm_magic
#define mstate_smallbins(M)       dl_gm_smallbins
#define mstate_treebins(M)        dl_gm_treebins
#define mstate_footprint(M)       dl_gm_footprint
#define mstate_max_footprint(M)   dl_gm_max_footprint
#define mstate_footprint_limit(M) dl_gm_footprint_limit
#define mstate_mflags(M)          dl_gm_mflags
#define mstate_seg(M)             dl_gm_seg
#if USE_LOCKS
#define mstate_mutex(M)           dl_gm_mutex
#endif /* USE_LOCKS */
#if USE_PENDING_FREE_LIST
#define mstate_flist(M)           dl_gm_flist
#endif /* USE_PENDING_FREE_LIST */

#define PARAM_mstate_m_  /* nothing */
#define PARAM_mstate_m   void
#define ARG_mstate_m_    /* nothing */
#define ARG_mstate_m     /* nothing */
#define ARG_mstate_X_(x) /* nothing */
#define ARG_mstate_X(x)  /* nothing */

#else /* GM_ONLY */
struct malloc_state {
	binmap_t  smallmap;
	binmap_t  treemap;
	size_t    dvsize;
	size_t    topsize;
	byte_t   *least_addr;
	mchunkptr dv;
	mchunkptr top;
	size_t    trim_check;
	size_t    release_checks;
	size_t    magic;
	mchunkptr smallbins[(NSMALLBINS + 1) * 2];
	tbinptr   treebins[NTREEBINS];
	size_t    footprint;
	size_t    max_footprint;
	size_t    footprint_limit; /* zero means no limit */
	flag_t    mflags;
#if USE_LOCKS
	DL_LOCK_T mutex; /* locate lock among fields that rarely change */
#endif /* USE_LOCKS */
#if USE_PENDING_FREE_LIST
	struct freelist flist;
#endif /* USE_PENDING_FREE_LIST */
	msegment  seg;
#if USE_PER_THREAD_MSTATE
	SLIST_ENTRY(malloc_state) ms_link; /* Link for "free_tls_mspace" / "used_tls_mspace" */
	uintptr_t ms_foreach_bitset; /* Atomic bitset of visit contexts */
#endif /* USE_PER_THREAD_MSTATE */
#ifdef MSTATE_EXTRA_FIELDS_1
	MSTATE_EXTRA_FIELDS_1;
#endif /* MSTATE_EXTRA_FIELDS_1 */
#ifdef MSTATE_EXTRA_FIELDS_2
	MSTATE_EXTRA_FIELDS_2;
#endif /* MSTATE_EXTRA_FIELDS_2 */
#ifdef MSTATE_EXTRA_FIELDS_3
	MSTATE_EXTRA_FIELDS_3;
#endif /* MSTATE_EXTRA_FIELDS_3 */
#ifdef MSTATE_EXTRA_FIELDS_4
	MSTATE_EXTRA_FIELDS_4;
#endif /* MSTATE_EXTRA_FIELDS_4 */
#if 0
	void     *extp; /* Unused but available for extensions */
	size_t    exts;
#endif
};

typedef struct malloc_state *mstate;
#define mstate_smallmap(M)        (M)->smallmap
#define mstate_treemap(M)         (M)->treemap
#define mstate_dvsize(M)          (M)->dvsize
#define mstate_topsize(M)         (M)->topsize
#define mstate_least_addr(M)      (M)->least_addr
#define mstate_dv(M)              (M)->dv
#define mstate_top(M)             (M)->top
#define mstate_trim_check(M)      (M)->trim_check
#define mstate_release_checks(M)  (M)->release_checks
#define mstate_magic(M)           (M)->magic
#define mstate_smallbins(M)       (M)->smallbins
#define mstate_treebins(M)        (M)->treebins
#define mstate_footprint(M)       (M)->footprint
#define mstate_max_footprint(M)   (M)->max_footprint
#define mstate_footprint_limit(M) (M)->footprint_limit
#define mstate_mflags(M)          (M)->mflags
#define mstate_seg(M)             (M)->seg
#if USE_LOCKS
#define mstate_mutex(M) (M)->mutex
#endif /* USE_LOCKS */
#if USE_PENDING_FREE_LIST
#define mstate_flist(M) (M)->flist
#endif /* USE_PENDING_FREE_LIST */
#if 0
#define mstate_extp(M) (M)->extp
#define mstate_exts(M) (M)->exts
#endif

#define PARAM_mstate_m_  mstate m,
#define PARAM_mstate_m   mstate m
#define ARG_mstate_m_    m,
#define ARG_mstate_m     m
#define ARG_mstate_X_(x) x,
#define ARG_mstate_X(x)  x
#endif /* !GM_ONLY */


/* ------------- Global malloc_state and malloc_params ------------------- */

/*
  malloc_params holds global properties, including those that can be
  dynamically set using mallopt. There is a single instance, mparams,
  initialized in init_mparams. Note that the non-zeroness of "magic"
  also serves as an initialization flag.
*/

struct malloc_params {
	size_t magic;
#ifdef MALLOC_PAGESIZE
#define malloc_pagesize MALLOC_PAGESIZE
#else /* !MALLOC_PAGESIZE */
	size_t page_size;
#define malloc_pagesize mparams.page_size
#endif /* !MALLOC_PAGESIZE */
	size_t granularity;
	size_t mmap_threshold;
	size_t trim_threshold;
	flag_t default_mflags;
};

static struct malloc_params mparams;

/* Ensure mparams initialized */
#define ensure_initialization() (void)(likely(mparams.magic != 0) || init_mparams())
#define ensure_initialization_for(after_expr) \
	if unlikely(mparams.magic == 0) {         \
		if (!init_mparams())                  \
			return after_expr;                \
	}

#if !ONLY_MSPACES

/* The global malloc_state used for all non-"mspace" calls */
#if GM_ONLY
#define is_global(M)        1
#define gm__smallmap        dl_gm_smallmap
#define gm__treemap         dl_gm_treemap
#define gm__dvsize          dl_gm_dvsize
#define gm__topsize         dl_gm_topsize
#define gm__least_addr      dl_gm_least_addr
#define gm__dv              dl_gm_dv
#define gm__top             dl_gm_top
#define gm__trim_check      dl_gm_trim_check
#define gm__release_checks  dl_gm_release_checks
#define gm__magic           dl_gm_magic
#define gm__smallbins       dl_gm_smallbins
#define gm__treebins        dl_gm_treebins
#define gm__footprint       dl_gm_footprint
#define gm__max_footprint   dl_gm_max_footprint
#define gm__footprint_limit dl_gm_footprint_limit
#define gm__mflags          dl_gm_mflags
#define gm__seg             dl_gm_seg
#if USE_LOCKS
#define gm__mutex           dl_gm_mutex
#endif /* USE_LOCKS */
#if USE_PENDING_FREE_LIST
#define gm__flist           dl_gm_flist
#endif /* USE_PENDING_FREE_LIST */
#define ARG_mstate_gm_ /* nothing */
#define ARG_mstate_gm  /* nothing */
#else /* GM_ONLY */
static struct malloc_state _gm_;
#define GM_STATIC_INIT_MUTEX 1 /* Static initialization is enough for locking to work */
#define gm                  (&_gm_)
#define is_global(M)        ((M) == &_gm_)
#define ARG_mstate_gm_      gm,
#define ARG_mstate_gm       gm
#define gm__smallmap        _gm_.smallmap
#define gm__treemap         _gm_.treemap
#define gm__dvsize          _gm_.dvsize
#define gm__topsize         _gm_.topsize
#define gm__least_addr      _gm_.least_addr
#define gm__dv              _gm_.dv
#define gm__top             _gm_.top
#define gm__trim_check      _gm_.trim_check
#define gm__release_checks  _gm_.release_checks
#define gm__magic           _gm_.magic
#define gm__smallbins       _gm_.smallbins
#define gm__treebins        _gm_.treebins
#define gm__footprint       _gm_.footprint
#define gm__max_footprint   _gm_.max_footprint
#define gm__footprint_limit _gm_.footprint_limit
#define gm__mflags          _gm_.mflags
#define gm__seg             _gm_.seg
#if USE_LOCKS
#define gm__mutex           _gm_.mutex
#endif /* USE_LOCKS */
#if USE_PENDING_FREE_LIST
#define gm__flist           _gm_.flist
#endif /* USE_PENDING_FREE_LIST */
#endif /* !GM_ONLY */
#endif /* !ONLY_MSPACES */
#ifndef GM_STATIC_INIT_MUTEX
#define GM_STATIC_INIT_MUTEX 0
#endif /* !GM_STATIC_INIT_MUTEX */

#define is_initialized(M) (mstate_top(M) != NULL)

/* -------------------------- system alloc setup ------------------------- */

/* Operations on mflags */

#define use_mmap(M)    (mstate_mflags(M) & USE_MMAP_BIT)
#define enable_mmap(M) (mstate_mflags(M) |= USE_MMAP_BIT)
#if HAVE_MMAP
#define disable_mmap(M) (mstate_mflags(M) &= ~USE_MMAP_BIT)
#else /* HAVE_MMAP */
#define disable_mmap(M)
#endif /* !HAVE_MMAP */

#define use_noncontiguous(M)  (mstate_mflags(M) & USE_NONCONTIGUOUS_BIT)
#define disable_contiguous(M) (mstate_mflags(M) |= USE_NONCONTIGUOUS_BIT)

/* page-align a size */
#define page_align(S) \
	(((S) + (malloc_pagesize - SIZE_T_ONE)) & ~(malloc_pagesize - SIZE_T_ONE))

/* granularity-align a size */
#define granularity_align(S) \
	(((S) + (mparams.granularity - SIZE_T_ONE)) & ~(mparams.granularity - SIZE_T_ONE))


/* For mmap, use granularity alignment on windows, else page-align */
#ifdef HAVE_MMAP_IS_VirtualAlloc
#define mmap_align(S) granularity_align(S)
#else /* HAVE_MMAP_IS_VirtualAlloc */
#define mmap_align(S) page_align(S)
#endif /* !HAVE_MMAP_IS_VirtualAlloc */

/* For sys_alloc, enough padding to ensure can malloc request on success */
#define SYS_ALLOC_PADDING (TOP_FOOT_SIZE + MALLOC_ALIGNMENT)

#define is_page_aligned(S) \
	(((size_t)(S) & (malloc_pagesize - SIZE_T_ONE)) == 0)
#define is_granularity_aligned(S) \
	(((size_t)(S) & (mparams.granularity - SIZE_T_ONE)) == 0)

/*  True if segment S holds address A */
#define segment_holds(S, A) \
	((byte_t *)(A) >= S->base && (byte_t *)(A) < S->base + S->size)

/* Return segment holding given address */
static msegmentptr segment_holding(PARAM_mstate_m_ byte_t *addr) {
	msegmentptr sp = &mstate_seg(m);
	for (;;) {
		if (addr >= sp->base && addr < sp->base + sp->size)
			return sp;
		if ((sp = sp->next) == NULL)
			return 0;
	}
}

/* Return true if segment contains a segment link */
#if HAVE_MMAP
static int has_segment_link(PARAM_mstate_m_ msegmentptr ss) {
	msegmentptr sp = &mstate_seg(m);
	for (;;) {
		if ((byte_t *)sp >= ss->base && (byte_t *)sp < ss->base + ss->size)
			return 1;
		if ((sp = sp->next) == NULL)
			return 0;
	}
}
#endif /* HAVE_MMAP */

#ifndef MORECORE_CANNOT_TRIM
#define should_trim(M, s) ((s) > mstate_trim_check(M))
#else /* MORECORE_CANNOT_TRIM */
#define should_trim(M, s) (0)
#endif /* MORECORE_CANNOT_TRIM */

/*
  TOP_FOOT_SIZE is padding at the end of a segment, including space
  that may be needed to place segment records and fenceposts when new
  noncontiguous segments are added.
*/
#define TOP_FOOT_SIZE \
	(align_offset(chunk2mem(0)) + pad_request(sizeof(struct malloc_segment)) + MIN_CHUNK_SIZE)


/* -------------------------------  Hooks -------------------------------- */

/*
  PREACTION should be defined to return 0 on success, and nonzero on
  failure. If you are not using locking, you can redefine these to do
  anything you like.
*/
#if !USE_LOCKS
#define TRY_PREACTION(M) 1
#define PREACTION(M)     (void)0
#define POSTACTION(M)    (void)0
#elif USE_PENDING_FREE_LIST
static ATTR_NOINLINE void
dl_freelist_do_reap(PARAM_mstate_m_ struct freelist_entry *__restrict flist);
#define HAVE_dl_freelist_do_reap

static void dl_freelist_release_and_reap(PARAM_mstate_m) {
	struct freelist pending;
again:
	pending.slh_first = SLIST_ATOMIC_CLEAR(&mstate_flist(m));
	if (pending.slh_first)
		dl_freelist_do_reap(ARG_mstate_m_ pending.slh_first);
	DL_LOCK_RELEASE(&mstate_mutex(m));
	if unlikely(ATOMIC_READ(&mstate_flist(m).slh_first) != NULL) {
		if (DL_LOCK_TRYACQUIRE(&mstate_mutex(m)))
			goto again;
	}
}

#if defined(DL_LOCK_AVAILABLE) && 1
/* Do a (read-only) check if lock is available so our
 * CPU doesn't need to write-lock a highly shared part
 * of the memory bus all the time. */
#define TRY_PREACTION(M)                    \
	(DL_LOCK_AVAILABLE(&mstate_mutex(M)) && \
	 DL_LOCK_TRYACQUIRE(&mstate_mutex(M)))
#else
#define TRY_PREACTION(M) DL_LOCK_TRYACQUIRE(&mstate_mutex(M))
#endif

#define PREACTION(M)      DL_LOCK_ACQUIRE(&mstate_mutex(M))
#define POSTACTION(M)     dl_freelist_release_and_reap(ARG_mstate_X(M))

#define dl_freelist_append(M, p)                                   \
	{                                                              \
		/* Append to free list... */                               \
		struct freelist_entry *ent = (struct freelist_entry *)(p); \
		SLIST_ATOMIC_INSERT(&mstate_flist(M), ent, fle_link);      \
		/* Try to reap free list... */                             \
		if (TRY_PREACTION(M))                                      \
			POSTACTION(M);                                         \
	}
#else /* USE_PENDING_FREE_LIST */
#define TRY_PREACTION(M) DL_LOCK_TRYACQUIRE(&mstate_mutex(M))
#define PREACTION(M)     DL_LOCK_ACQUIRE(&mstate_mutex(M))
#define POSTACTION(M)    DL_LOCK_RELEASE(&mstate_mutex(M))
#endif /* !USE_PENDING_FREE_LIST */


#if PROCEED_ON_ERROR
/* A count of the number of corruption errors causing resets */
int malloc_corruption_error_count;
/* default corruption action */
static void reset_on_error(PARAM_mstate_m);
#endif /* PROCEED_ON_ERROR */


/* -------------------------- Debugging setup ---------------------------- */

#if DL_DEBUG_MEMSET_FREE
#define dl_setfree_word(W, T) (void)(W = (T)DL_DEBUG_MEMSET_FREE_PATTERN)
static void DCALL dl_setfree_data(void *p, size_t n) {
	size_t *iter = (size_t *)p;
	while (n >= sizeof(size_t)) {
		*iter++ = DL_DEBUG_MEMSET_FREE_PATTERN;
		n -= sizeof(size_t);
	}
}
#else /* DL_DEBUG_MEMSET_FREE */
#define dl_setfree_word(W, T) (void)0
#define dl_setfree_data(p, n) (void)0
#endif /* !DL_DEBUG_MEMSET_FREE */
#if DL_DEBUG_MEMSET_ALLOC
#define dl_setalloc_word(W, T) (void)(W = (T)DL_DEBUG_MEMSET_ALLOC_PATTERN)
static void DCALL dl_setalloc_data(void *p, size_t n) {
	size_t *iter = (size_t *)p;
	while (n >= sizeof(size_t)) {
		*iter++ = DL_DEBUG_MEMSET_ALLOC_PATTERN;
		n -= sizeof(size_t);
	}
}
#else /* DL_DEBUG_MEMSET_ALLOC */
#define dl_setalloc_word(W, T) (void)0
#define dl_setalloc_data(p, n) (void)0
#endif /* !DL_DEBUG_MEMSET_ALLOC */

#define dl_setfree_word_untested(W, T)  dl_setfree_word(W, T)
#define dl_setfree_data_untested(p, n)  dl_setfree_data(p, n)


#if !DL_DEBUG_EXTERNAL && !DL_DEBUG_INTERNAL
#define check_free_chunk(M, P)
#define check_inuse_chunk(M, P)
#define check_top_chunk(M, P)
#define check_malloced_chunk(M, P, N)
#define check_mmapped_chunk(M, P)
#define check_malloc_state(M)
#define check_memset_use_after_free(M, P)
#else /* !DL_DEBUG_EXTERNAL && !DL_DEBUG_INTERNAL */
#if DL_DEBUG_INTERNAL
#define check_free_chunk(M, P)        do_check_free_chunk(ARG_mstate_X_(M) P)
#define check_inuse_chunk(M, P)       do_check_inuse_chunk(ARG_mstate_X_(M) P)
#define check_top_chunk(M, P)         do_check_top_chunk(ARG_mstate_X_(M) P)
#define check_malloced_chunk(M, P, N) do_check_malloced_chunk(ARG_mstate_X_(M) P, N)
#define check_mmapped_chunk(M, P)     do_check_mmapped_chunk(ARG_mstate_X_(M) P)
#define check_malloc_state(M)         do_check_malloc_state(ARG_mstate_X(M))
#define check_memset_free(P)          do_check_memset_free(P)
#else /* DL_DEBUG_INTERNAL */
#define check_free_chunk(M, P)        /* nothing */
#define check_inuse_chunk(M, P)       /* nothing */
#define check_top_chunk(M, P)         /* nothing */
#define check_malloced_chunk(M, P, N) /* nothing */
#define check_mmapped_chunk(M, P)     /* nothing */
#define check_malloc_state(M)         /* nothing */
#define check_memset_free(P)          /* nothing */
#endif /* !DL_DEBUG_INTERNAL */
#if DETECT_USE_AFTER_FREE
#define check_memset_use_after_free(M, P) do_check_memset_free(ARG_mstate_X_(M) P)
#else /* DETECT_USE_AFTER_FREE */
#define check_memset_use_after_free(M, P) /* nothing */
#endif /* !DETECT_USE_AFTER_FREE */

#if DL_DEBUG_MEMSET_FREE
static void do_check_memset_free(PARAM_mstate_m_ mchunkptr p);
#else /* DL_DEBUG_MEMSET_FREE */
#define do_check_memset_free(p) (void)0
#endif /* !DL_DEBUG_MEMSET_FREE */
static void do_check_any_chunk(PARAM_mstate_m_ mchunkptr p);
static void do_check_top_chunk(PARAM_mstate_m_ mchunkptr p);
static void do_check_mmapped_chunk(PARAM_mstate_m_ mchunkptr p);
static void do_check_inuse_chunk(PARAM_mstate_m_ mchunkptr p);
static void do_check_free_chunk(PARAM_mstate_m_ mchunkptr p);
#if DL_DEBUG_INTERNAL
static void do_check_malloced_chunk(PARAM_mstate_m_ void *mem, size_t s);
#endif /* DL_DEBUG_INTERNAL */
static void do_check_tree(PARAM_mstate_m_ tchunkptr t);
static void do_check_treebin(PARAM_mstate_m_ bindex_t i);
static void do_check_smallbin(PARAM_mstate_m_ bindex_t i);
static void do_check_malloc_state(PARAM_mstate_m);
static int bin_find(PARAM_mstate_m_ mchunkptr x);
static size_t traverse_and_check(PARAM_mstate_m);
#endif /* DL_DEBUG_EXTERNAL || DL_DEBUG_INTERNAL */

/* ---------------------------- Indexing Bins ---------------------------- */

#define is_small(s)         (((s) >> SMALLBIN_SHIFT) < NSMALLBINS)
#define small_index(s)      (bindex_t)((s) >> SMALLBIN_SHIFT)
#define small_index2size(i) ((i) << SMALLBIN_SHIFT)
#define MIN_SMALL_INDEX     (small_index(MIN_CHUNK_SIZE))

/* addressing by index. See above about smallbin repositioning */
#define smallbin_at(M, i) ((sbinptr)((byte_t *)&(mstate_smallbins(M)[(i) << 1])))
#define treebin_at(M, i)  (&(mstate_treebins(M)[i]))

/* assign tree index for size S to variable I. Use x86 asm if possible  */
#define compute_tree_index(S, I)                                               \
	{                                                                          \
		size_t X = S >> TREEBIN_SHIFT;                                         \
		if (X == 0)                                                            \
			I = 0;                                                             \
		else if (X > 0xFFFF)                                                   \
			I = NTREEBINS - 1;                                                 \
		else {                                                                 \
			unsigned int K = (unsigned)sizeof(X) * __CHAR_BIT__ - 1 - CLZ(X);  \
			I = (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT - 1)) & 1))); \
		}                                                                      \
	}

/* Bit representing maximum resolved size in a treebin at i */
#define bit_for_tree_index(i) \
	(i == NTREEBINS - 1) ? (SIZE_T_BITSIZE - 1) : (((i) >> 1) + TREEBIN_SHIFT - 2)

/* Shift placing maximum resolved bit in a treebin at i as sign bit */
#define leftshift_for_tree_index(i) \
	((i == NTREEBINS - 1) ? 0 : ((SIZE_T_BITSIZE - SIZE_T_ONE) - (((i) >> 1) + TREEBIN_SHIFT - 2)))

/* The size of the smallest chunk held in bin with index i */
#define minsize_for_tree_index(i)                   \
	((SIZE_T_ONE << (((i) >> 1) + TREEBIN_SHIFT)) | \
	 (((size_t)((i) & SIZE_T_ONE)) << (((i) >> 1) + TREEBIN_SHIFT - 1)))


/* ------------------------ Operations on bin maps ----------------------- */

/* bit corresponding to given index */
#define idx2bit(i) ((binmap_t)(1) << (i))

/* Mark/Clear bits with given index */
#define mark_smallmap(M, i)      (mstate_smallmap(M) |= idx2bit(i))
#define clear_smallmap(M, i)     (mstate_smallmap(M) &= ~idx2bit(i))
#define smallmap_is_marked(M, i) (mstate_smallmap(M) & idx2bit(i))

#define mark_treemap(M, i)      (mstate_treemap(M) |= idx2bit(i))
#define clear_treemap(M, i)     (mstate_treemap(M) &= ~idx2bit(i))
#define treemap_is_marked(M, i) (mstate_treemap(M) & idx2bit(i))

/* isolate the least set bit of a bitmap */
#define least_bit(x) ((x) & (SIZE_T_ZERO - (x)))

/* mask with all bits to left of least bit of x on */
#define left_bits(x) ((x << 1) | (SIZE_T_ZERO - (x << 1)))

/* mask with all bits to left of or equal to least bit of x on */
#define same_or_left_bits(x) ((x) | (SIZE_T_ZERO - (x)))

#define compute_bit2idx(X, I) I = (bindex_t)CTZ(X)


/* ----------------------- Runtime Check Support ------------------------- */

#if !INSECURE
/* Check if address a is at least as high as any from MORECORE or MMAP */
#define ok_address(M, a) ((byte_t *)(a) >= mstate_least_addr(M))
/* Check if address of next chunk n is higher than base chunk p */
#define ok_next(p, n)    ((byte_t *)(p) < (byte_t *)(n))
/* Check if p has inuse status */
#define ok_inuse(p)      is_inuse(p)
/* Check if p has its pinuse bit on */
#define ok_pinuse(p)     pinuse(p)
#define ext_assert__ok_address(M, a) ext_assert(ok_address(M, a))
#define ext_assert__ok_next(p, n)    ext_assert(ok_next(p, n))
#define ext_assert__ok_inuse(p)      ext_assert(ok_inuse(p))
#define ext_assert__ok_pinuse(p)     ext_assert(ok_pinuse(p))
#else /* !INSECURE */
#define ok_address(M, a)             1
#define ok_next(b, n)                1
#define ok_inuse(p)                  1
#define ok_pinuse(p)                 1
#define ext_assert__ok_address(M, a) (void)0
#define ext_assert__ok_next(p, n)    (void)0
#define ext_assert__ok_inuse(p)      (void)0
#define ext_assert__ok_pinuse(p)     (void)0
#endif /* !INSECURE */

#if (FOOTERS && !INSECURE)
/* Check if (alleged) mstate m has expected magic field */
#define ok_magic(M) ((M)->magic == mparams.magic)
#define ext_assert__ok_magic(M) ext_assert(ok_magic(M))
#else /* (FOOTERS && !INSECURE) */
#define ok_magic(M)             1
#define ext_assert__ok_magic(M) (void)0
#endif /* (FOOTERS && !INSECURE) */


/* macros to set up inuse chunks with or without footers */

#if !FOOTERS

#define mark_inuse_foot(M, p, s)

/* Macros for setting head/foot of non-mmapped chunks */

/* Set cinuse bit and pinuse bit of next chunk */
#define set_inuse(M, p, s)                                    \
	((p)->head = (((p)->head & PINUSE_BIT) | s | CINUSE_BIT), \
	 chunk_plus_offset(p, s)->head |= PINUSE_BIT)

/* Set cinuse and pinuse of this chunk and pinuse of next chunk */
#define set_inuse_and_pinuse(M, p, s)           \
	((p)->head = (s | PINUSE_BIT | CINUSE_BIT), \
	 chunk_plus_offset(p, s)->head |= PINUSE_BIT)

/* Set size, cinuse and pinuse bit of this chunk */
#define set_size_and_pinuse_of_inuse_chunk(M, p, s) \
	((p)->head = (s | PINUSE_BIT | CINUSE_BIT))

#else /* FOOTERS */

/* Set foot of inuse chunk to be xor of mstate and seed */
#if GM_ONLY
#define mark_inuse_foot(M, p, s) (chunk_plus_offset(p, s)->prev_foot = mparams.magic)
#define get_mstate_for(p)
#else /* GM_ONLY */
#if XOR_MASK_MCHUNK_FOOT
#define mstate_encode_for_footer(M) ((size_t)(M) ^ mparams.magic)
#define mstate_decode_for_footer(M) ((mstate)((size_t)(M) ^ mparams.magic))
#else /* XOR_MASK_MCHUNK_FOOT */
#define mstate_encode_for_footer(M) ((size_t)(M))
#define mstate_decode_for_footer(M) ((mstate)(M))
#endif /* !XOR_MASK_MCHUNK_FOOT */
#define mark_inuse_foot(M, p, s) (chunk_plus_offset(p, s)->prev_foot = mstate_encode_for_footer(M))
#define get_mstate_for(p) mstate_decode_for_footer(next_chunk(p)->prev_foot)
#endif /* !GM_ONLY */


#define set_inuse(M, p, s)                                    \
	((p)->head = (((p)->head & PINUSE_BIT) | s | CINUSE_BIT), \
	 (chunk_plus_offset(p, s)->head |= PINUSE_BIT),           \
	 mark_inuse_foot(M, p, s))

#define set_inuse_and_pinuse(M, p, s)               \
	((p)->head = (s | PINUSE_BIT | CINUSE_BIT),     \
	 (chunk_plus_offset(p, s)->head |= PINUSE_BIT), \
	 mark_inuse_foot(M, p, s))

#define set_size_and_pinuse_of_inuse_chunk(M, p, s) \
	((p)->head = (s | PINUSE_BIT | CINUSE_BIT),     \
	 mark_inuse_foot(M, p, s))

#endif /* !FOOTERS */

#if FLAG4_BIT_HEAP_REGION
#define do_validate_flag4_footer(p)                                            \
	do {                                                                       \
		size_t _ps = chunksize(p);                                             \
		size_t *_pval = &chunk_plus_offset(p, _ps)->prev_foot;                 \
		ext_assertf(*_pval == _ps,                                             \
		            "flag4 mchunk footer at %p of chunk %p-%p has unexpected " \
		            "value %#" PRIxSIZ " when %#" PRIxSIZ " was expected",     \
		            _pval, p, (byte_t *)(p) + _ps - 1, *_pval, _ps);           \
	}	__WHILE0
#else /* FLAG4_BIT_HEAP_REGION */
#define do_validate_flag4_footer_IS_NOOP
#define do_validate_flag4_footer(p) (void)0
#endif /* !FLAG4_BIT_HEAP_REGION */
#if FOOTERS && GM_ONLY
#define do_validate_inuse_footer(p)                                            \
	do {                                                                       \
		size_t _ps = chunksize(p);                                             \
		size_t *_pval = &chunk_plus_offset(p, _ps)->prev_foot;                 \
		ext_assertf(*_pval == mparams.magic,                                   \
		            "inuse mchunk footer at %p of chunk %p-%p has unexpected " \
		            "value %#" PRIxSIZ " when %#" PRIxSIZ " was expected",     \
		            _pval, p, (byte_t *)(p) + _ps - 1, *_pval, mparams.magic); \
	}	__WHILE0
#else /* FOOTERS && GM_ONLY */
#define do_validate_inuse_footer_IS_NOOP
#define do_validate_inuse_footer(p) (void)0
#endif /* !FOOTERS || !GM_ONLY */

#if !defined(do_validate_flag4_footer_IS_NOOP) && !defined(do_validate_inuse_footer_IS_NOOP)
#define do_validate_footer(p)            \
	do {                                 \
		if (!is_mmapped(p)) {            \
			do_validate_inuse_footer(p); \
		} else if (flag4inuse(p)) {      \
			do_validate_flag4_footer(p); \
		}                                \
	}	__WHILE0
#elif !defined(do_validate_flag4_footer_IS_NOOP)
#define do_validate_footer(p)            \
	do {                                 \
		if (flag4inuse(p))               \
			do_validate_flag4_footer(p); \
	}	__WHILE0
#elif !defined(do_validate_inuse_footer_IS_NOOP)
#define do_validate_footer(p)            \
	do {                                 \
		if (!is_mmapped(p))              \
			do_validate_inuse_footer(p); \
	}	__WHILE0
#else /* ... */
#define do_validate_footer(p) (void)0
#endif /* !... */
#if DL_DEBUG_EXTERNAL
#define validate_footer(p) do_validate_footer(p)
#else /* DL_DEBUG_EXTERNAL */
#define validate_footer(p) (void)0
#endif /* !DL_DEBUG_EXTERNAL */


/* ---------------------------- setting mparams -------------------------- */

/* Initialize mparams */
static int init_mparams(void) {
#ifdef NEED_GLOBAL_LOCK_INIT
	if (malloc_global_mutex_status <= 0)
		init_malloc_global_mutex();
#endif

	ACQUIRE_MALLOC_GLOBAL_LOCK();
	if (mparams.magic == 0) {
		size_t magic;
		size_t psize;
		size_t gsize;

#ifndef HAVE_MMAP_IS_VirtualAlloc
		psize = malloc_getpagesize;
#if DEFAULT_GRANULARITY_IS_ZERO
		gsize = psize;
#else /* DEFAULT_GRANULARITY_IS_ZERO */
		gsize = ((DEFAULT_GRANULARITY != 0) ? DEFAULT_GRANULARITY : psize);
#endif /* !DEFAULT_GRANULARITY_IS_ZERO */
#else /* !HAVE_MMAP_IS_VirtualAlloc */
		{
			SYSTEM_INFO system_info;
			GetSystemInfo(&system_info);
			psize = system_info.dwPageSize;
#if DEFAULT_GRANULARITY_IS_ZERO
			gsize = system_info.dwAllocationGranularity;
#else /* DEFAULT_GRANULARITY_IS_ZERO */
			gsize = ((DEFAULT_GRANULARITY != 0) ? DEFAULT_GRANULARITY : system_info.dwAllocationGranularity);
#endif /* !DEFAULT_GRANULARITY_IS_ZERO */
		}
#endif /* HAVE_MMAP_IS_VirtualAlloc */

		/* Sanity-check configuration:
		   size_t must be unsigned and as wide as pointer type.
		   ints must be at least 4 bytes.
		   alignment must be at least 8.
		   Alignment, min chunk size, and page size must all be powers of 2.
		*/
#if 0
		if ((sizeof(size_t) != sizeof(byte_t*)) ||
		    (SIZE_MAX < MIN_CHUNK_SIZE)  ||
		    (sizeof(int) < 4)  ||
		    (MALLOC_ALIGNMENT < (size_t)8U) ||
		    ((MALLOC_ALIGNMENT & (MALLOC_ALIGNMENT-SIZE_T_ONE)) != 0) ||
		    ((MCHUNK_SIZE      & (MCHUNK_SIZE-SIZE_T_ONE))      != 0) ||
		    ((gsize            & (gsize-SIZE_T_ONE))            != 0) ||
		    ((psize            & (psize-SIZE_T_ONE))            != 0))
			ABORT;
#endif
		mparams.granularity = gsize;
#ifndef MALLOC_PAGESIZE
		malloc_pagesize = psize;
#endif /* !MALLOC_PAGESIZE */
		mparams.mmap_threshold = DEFAULT_MMAP_THRESHOLD;
		mparams.trim_threshold = DEFAULT_TRIM_THRESHOLD;
#if MORECORE_CONTIGUOUS
		mparams.default_mflags = USE_MMAP_BIT;
#else /* MORECORE_CONTIGUOUS */
		mparams.default_mflags = USE_MMAP_BIT | USE_NONCONTIGUOUS_BIT;
#endif /* MORECORE_CONTIGUOUS */

#if !ONLY_MSPACES
		/* Set up lock for main malloc area */
		gm__mflags = mparams.default_mflags;
#endif /* !ONLY_MSPACES */

		{
#ifdef CONFIG_HOST_WINDOWS
			magic = (size_t)(GetTickCount() ^ (size_t)0x55555555U);
#elif DL_CONFIG_HAVE_time
			magic = (size_t)(time(NULL) ^ (size_t)0x55555555U);
#else /* ... */
			magic = (size_t)&magic ^ (size_t)0x55555555U;
#endif /* !... */
			magic |= (size_t)8U;  /* ensure nonzero */
			magic &= ~(size_t)7U; /* improve chances of fault for bad values */
			/* Until memory modes commonly available, use volatile-write */
			(*(size_t volatile *)(&(mparams.magic))) = magic;
		}
#ifdef MALLOC_INIT_EXTRA_HOOK
		MALLOC_INIT_EXTRA_HOOK();
#endif /* MALLOC_INIT_EXTRA_HOOK */
	}

	RELEASE_MALLOC_GLOBAL_LOCK();
	return 1;
}

/* support for mallopt */
#if !NO_MALLOPT
#if dlmallopt_SECOND_PARAMETER_IS_SIZE_T
DL_API_IMPL(, int, dlmallopt, (int param_number, size_t val))
#else /* dlmallopt_SECOND_PARAMETER_IS_SIZE_T */
DL_API_IMPL(, int, dlmallopt, (int param_number, int value))
#endif /* !dlmallopt_SECOND_PARAMETER_IS_SIZE_T */
{
#if !dlmallopt_SECOND_PARAMETER_IS_SIZE_T
	size_t val = (value == -1) ? SIZE_MAX : (size_t)value;
#endif /* !dlmallopt_SECOND_PARAMETER_IS_SIZE_T */
#ifdef HOOK_AFTER_INIT_MALLOPT
	ensure_initialization_for(HOOK_AFTER_INIT_MALLOPT(param_number, value));
#else /* HOOK_AFTER_INIT_MALLOPT */
	ensure_initialization();
#endif /* !HOOK_AFTER_INIT_MALLOPT */
	switch (param_number) {
	case M_TRIM_THRESHOLD:
		mparams.trim_threshold = val;
		return 1;
	case M_GRANULARITY:
		if (val >= malloc_pagesize && ((val & (val - 1)) == 0)) {
			mparams.granularity = val;
			return 1;
		} else
			return 0;
	case M_MMAP_THRESHOLD:
		mparams.mmap_threshold = val;
		return 1;
	default:
		return 0;
	}
}
#endif /* !NO_MALLOPT */


#if DL_DEBUG_EXTERNAL || DL_DEBUG_INTERNAL
/* ------------------------- Debugging Support --------------------------- */

#if DL_DEBUG_MEMSET_FREE
static void do_check_memset_free(PARAM_mstate_m_ mchunkptr p) {
	size_t num_bytes = chunksize(p);
	size_t num_words = num_bytes / sizeof(size_t);
	size_t i, *words = (size_t *)p;
	if (p == mstate_dv(m) || p == mstate_top(m)) {
		words += 2;
		num_words -= 2;
	} else if (is_small(num_bytes)) {
		words += sizeof(mchunk) / sizeof(size_t);
		num_words -= sizeof(mchunk) / sizeof(size_t);
	} else {
		words += sizeof(tchunk) / sizeof(size_t);
		num_words -= sizeof(tchunk) / sizeof(size_t);
	}
	for (i = 0; i < num_words; ++i) {
		ASSERTF(words[i] == DL_DEBUG_MEMSET_FREE_PATTERN,
		        "Free pointer %p at offset %" PRIxSIZ " from %p-%p has bad pattern %#" PRFXSIZ "",
		        &words[i], i * sizeof(size_t), p, (byte_t *)p + num_bytes - 1, words[i]);
	}
}
#endif /* DL_DEBUG_MEMSET_FREE */

/* Check properties of any chunk, whether free, inuse, mmapped etc  */
static void do_check_any_chunk(PARAM_mstate_m_ mchunkptr p) {
	ASSERT((is_aligned(chunk2mem(p))) || (p->head == FENCEPOST_HEAD));
	ASSERT(ok_address(m, p));
}

/* Check properties of top chunk */
static void do_check_top_chunk(PARAM_mstate_m_ mchunkptr p) {
	msegmentptr sp = segment_holding(ARG_mstate_m_ (byte_t *)p);
	size_t sz      = p->head & ~INUSE_BITS; /* third-lowest bit can be set! */
	ASSERT(sp != NULL);
	ASSERT((is_aligned(chunk2mem(p))) || (p->head == FENCEPOST_HEAD));
	ASSERT(ok_address(m, p));
	ASSERT(sz == mstate_topsize(m));
	ASSERT(sz > 0);
	ASSERT(sz == ((sp->base + sp->size) - (byte_t *)p) - TOP_FOOT_SIZE);
	ASSERT(pinuse(p));
	ASSERT(!pinuse(chunk_plus_offset(p, sz)));
}

/* Check properties of (inuse) mmapped chunks */
static void do_check_mmapped_chunk(PARAM_mstate_m_ mchunkptr p) {
	size_t sz  = chunksize(p);
	size_t len = (sz + (p->prev_foot) + MMAP_FOOT_PAD);
	ASSERT(is_mmapped(p));
	ASSERT(use_mmap(m));
	ASSERT((is_aligned(chunk2mem(p))) || (p->head == FENCEPOST_HEAD));
	ASSERT(ok_address(m, p));
	ASSERT(!is_small(sz));
	ASSERT((len & (malloc_pagesize - SIZE_T_ONE)) == 0);
	ASSERT(chunk_plus_offset(p, sz)->head == FENCEPOST_HEAD);
	ASSERT(chunk_plus_offset(p, sz + SIZE_T_SIZE)->head == 0);
}

/* Check properties of inuse chunks */
static void do_check_inuse_chunk(PARAM_mstate_m_ mchunkptr p) {
	do_check_any_chunk(ARG_mstate_m_ p);
	ASSERT(is_inuse(p));
	ASSERT(next_pinuse(p));
	/* If not pinuse and not mmapped, previous chunk has OK offset */
	ASSERT(is_mmapped(p) || pinuse(p) || next_chunk(prev_chunk(p)) == p);
	if (is_mmapped(p))
		do_check_mmapped_chunk(ARG_mstate_m_ p);
}

/* Check properties of free chunks */
static void do_check_free_chunk(PARAM_mstate_m_ mchunkptr p) {
	size_t sz      = chunksize(p);
	mchunkptr next = chunk_plus_offset(p, sz);
	do_check_any_chunk(ARG_mstate_m_ p);
	ASSERT(!is_inuse(p));
	ASSERT(!next_pinuse(p));
	ASSERT(!is_mmapped(p));
	if (p != mstate_dv(m) && p != mstate_top(m)) {
		if (sz >= MIN_CHUNK_SIZE) {
			ASSERT((sz & CHUNK_ALIGN_MASK) == 0);
			ASSERT(is_aligned(chunk2mem(p)));
			ASSERT(next->prev_foot == sz);
			ASSERT(pinuse(p));
			ASSERT(next == mstate_top(m) || is_inuse(next));
			ASSERT(p->fd->bk == p);
			ASSERT(p->bk->fd == p);
		} else /* markers are always of size SIZE_T_SIZE */
			ASSERT(sz == SIZE_T_SIZE);
	}
	do_check_memset_free(ARG_mstate_m_ p);
}

/* Check properties of malloced chunks at the point they are malloced */
#if DL_DEBUG_INTERNAL
static void do_check_malloced_chunk(PARAM_mstate_m_ void *mem, size_t s) {
	if (mem != NULL) {
		mchunkptr p = mem2chunk(mem);
		size_t sz   = p->head & ~INUSE_BITS;
		do_check_inuse_chunk(ARG_mstate_m_ p);
		ASSERT((sz & CHUNK_ALIGN_MASK) == 0);
		ASSERT(sz >= MIN_CHUNK_SIZE);
		ASSERT(sz >= s);
		/* unless mmapped, size is less than MIN_CHUNK_SIZE more than request */
		ASSERT(is_mmapped(p) || sz < (s + MIN_CHUNK_SIZE));
	}
}
#endif /* DL_DEBUG_INTERNAL */

/* Check a tree and its subtrees.  */
static void do_check_tree(PARAM_mstate_m_ tchunkptr t) {
	tchunkptr head  = NULL;
	tchunkptr u     = t;
	bindex_t tindex = t->index;
	size_t tsize    = chunksize(t);
	bindex_t idx;
	compute_tree_index(tsize, idx);
	ASSERT(tindex == idx);
	ASSERT(tsize >= MIN_LARGE_SIZE);
	ASSERT(tsize >= minsize_for_tree_index(idx));
	ASSERT((idx == NTREEBINS - 1) || (tsize < minsize_for_tree_index((idx + 1))));

	do { /* traverse through chain of same-sized nodes */
		do_check_any_chunk(ARG_mstate_m_((mchunkptr)u));
		ASSERT(u->index == tindex);
		ASSERT(chunksize(u) == tsize);
		ASSERT(!is_inuse(u));
		ASSERT(!next_pinuse(u));
		ASSERT(u->fd->bk == u);
		ASSERT(u->bk->fd == u);
		if (u->parent == NULL) {
			ASSERT(u->child[0] == NULL);
			ASSERT(u->child[1] == NULL);
		} else {
			ASSERT(head == NULL); /* only one node on chain has parent */
			head = u;
			ASSERT(u->parent != u);
			ASSERT(u->parent->child[0] == u ||
			       u->parent->child[1] == u ||
			       *((tbinptr *)(u->parent)) == u);
			if (u->child[0] != NULL) {
				ASSERT(u->child[0]->parent == u);
				ASSERT(u->child[0] != u);
				do_check_tree(ARG_mstate_m_ u->child[0]);
			}
			if (u->child[1] != NULL) {
				ASSERT(u->child[1]->parent == u);
				ASSERT(u->child[1] != u);
				do_check_tree(ARG_mstate_m_ u->child[1]);
			}
			if (u->child[0] != NULL && u->child[1] != NULL) {
				ASSERT(chunksize(u->child[0]) < chunksize(u->child[1]));
			}
		}
		u = u->fd;
	} while (u != t);
	ASSERT(head != NULL);
}

/*  Check all the chunks in a treebin.  */
static void do_check_treebin(PARAM_mstate_m_ bindex_t i) {
	tbinptr *tb = treebin_at(m, i);
	tchunkptr t = *tb;
	int empty   = (mstate_treemap(m) & (1U << i)) == 0;
	if (t == NULL)
		ASSERT(empty);
	if (!empty)
		do_check_tree(ARG_mstate_m_ t);
}

/*  Check all the chunks in a smallbin.  */
static void do_check_smallbin(PARAM_mstate_m_ bindex_t i) {
	sbinptr b          = smallbin_at(m, i);
	mchunkptr p        = b->bk;
	unsigned int empty = (mstate_smallmap(m) & (1U << i)) == 0;
	if (p == b)
		ASSERT(empty);
	if (!empty) {
		for (; p != b; p = p->bk) {
			size_t size = chunksize(p);
			mchunkptr q;
			/* each chunk claims to be free */
			do_check_free_chunk(ARG_mstate_m_ p);
			/* chunk belongs in bin */
			ASSERT(small_index(size) == i);
			ASSERT(p->bk == b || chunksize(p->bk) == chunksize(p));
			/* chunk is followed by an inuse chunk */
			q = next_chunk(p);
			if (q->head != FENCEPOST_HEAD)
				do_check_inuse_chunk(ARG_mstate_m_ q);
		}
	}
}

/* Find x in a bin. Used in other check functions. */
static int bin_find(PARAM_mstate_m_ mchunkptr x) {
	size_t size = chunksize(x);
	if (is_small(size)) {
		bindex_t sidx = small_index(size);
		sbinptr b     = smallbin_at(m, sidx);
		if (smallmap_is_marked(m, sidx)) {
			mchunkptr p = b;
			do {
				if (p == x)
					return 1;
			} while ((p = p->fd) != b);
		}
	} else {
		bindex_t tidx;
		compute_tree_index(size, tidx);
		if (treemap_is_marked(m, tidx)) {
			tchunkptr t     = *treebin_at(m, tidx);
			size_t sizebits = size << leftshift_for_tree_index(tidx);
			while (t != NULL && chunksize(t) != size) {
				t = t->child[(sizebits >> (SIZE_T_BITSIZE - SIZE_T_ONE)) & 1];
				sizebits <<= 1;
			}
			if (t != NULL) {
				tchunkptr u = t;
				do {
					if (u == (tchunkptr)x)
						return 1;
				} while ((u = u->fd) != t);
			}
		}
	}
	return 0;
}

/* Traverse each chunk and check it; return total */
static size_t traverse_and_check(PARAM_mstate_m) {
	size_t sum = 0;
	if (is_initialized(m)) {
		msegmentptr s = &mstate_seg(m);
		sum += mstate_topsize(m) + TOP_FOOT_SIZE;
		while (s != NULL) {
			mchunkptr q     = align_as_chunk(s->base);
			mchunkptr lastq = NULL;
			ASSERT(pinuse(q));
			while (segment_holds(s, q) &&
			       q != mstate_top(m) && q->head != FENCEPOST_HEAD) {
				sum += chunksize(q);
				if (is_inuse(q)) {
					ASSERT(!bin_find(ARG_mstate_m_ q));
					do_check_inuse_chunk(ARG_mstate_m_ q);
				} else {
					ASSERT(q == mstate_dv(m) || bin_find(ARG_mstate_m_ q));
					ASSERT(lastq == NULL || is_inuse(lastq)); /* Not 2 consecutive free */
					do_check_free_chunk(ARG_mstate_m_ q);
				}
				lastq = q;
				q     = next_chunk(q);
			}
			s = s->next;
		}
	}
	return sum;
}


/* Check all properties of malloc_state. */
static void do_check_malloc_state(PARAM_mstate_m) {
	bindex_t i;
	size_t total;
	/*static int count = 0;
	if (((count++) % 256) != 0)
		return;*/

	/* check bins */
	for (i = 0; i < NSMALLBINS; ++i)
		do_check_smallbin(ARG_mstate_m_ i);
	for (i = 0; i < NTREEBINS; ++i)
		do_check_treebin(ARG_mstate_m_ i);

	if (mstate_dvsize(m) != 0) { /* check dv chunk */
		do_check_any_chunk(ARG_mstate_m_ mstate_dv(m));
		ASSERT(mstate_dvsize(m) == chunksize(mstate_dv(m)));
		ASSERT(mstate_dvsize(m) >= MIN_CHUNK_SIZE);
		ASSERT(bin_find(ARG_mstate_m_ mstate_dv(m)) == 0);
	}

	if (mstate_top(m) != 0) { /* check top chunk */
		do_check_top_chunk(ARG_mstate_m_ mstate_top(m));
		/*ASSERT(mstate_topsize(m) == chunksize(mstate_top(m))); redundant */
		ASSERT(mstate_topsize(m) > 0);
		ASSERT(bin_find(ARG_mstate_m_ mstate_top(m)) == 0);
	}

	total = traverse_and_check(ARG_mstate_m);
	ASSERT(total <= mstate_footprint(m));
	ASSERT(mstate_footprint(m) <= mstate_max_footprint(m));
}
#endif /* DL_DEBUG_EXTERNAL || DL_DEBUG_INTERNAL */

/* ----------------------------- statistics ------------------------------ */

#if !NO_MALLINFO
static void do_gather_internal_mallinfo(PARAM_mstate_m_ STRUCT_mallinfo *nm) {
	PREACTION(m);
	check_malloc_state(m);
	if (is_initialized(m)) {
		size_t nfree  = SIZE_T_ONE; /* top always free */
		size_t mfree  = mstate_topsize(m) + TOP_FOOT_SIZE;
		size_t sum    = mfree;
		msegmentptr s = &mstate_seg(m);
		while (s != NULL) {
			mchunkptr q = align_as_chunk(s->base);
			while (segment_holds(s, q) &&
			       q != mstate_top(m) && q->head != FENCEPOST_HEAD) {
				size_t sz = chunksize(q);
				sum += sz;
				if (!is_inuse(q)) {
					mfree += sz;
					++nfree;
				}
				q = next_chunk(q);
			}
			s = s->next;
		}
#define nm__hmi_account(x, v) STRUCT_mallinfo_set_##x(nm, STRUCT_mallinfo_get_##x(nm) + v)
		nm__hmi_account(arena, sum);
		nm__hmi_account(ordblks, nfree);
		nm__hmi_account(hblkhd, mstate_footprint(m) - sum);
		nm__hmi_account(usmblks, mstate_max_footprint(m));
		nm__hmi_account(uordblks, mstate_footprint(m) - mfree);
		nm__hmi_account(fordblks, mfree);
		nm__hmi_account(keepcost, mstate_topsize(m));
#undef nm__hmi_account
	}
	POSTACTION(m);
}

#if USE_PER_THREAD_MSTATE
static size_t do_gather_internal_mallinfo_cb(mspace ms, void *arg) {
	do_gather_internal_mallinfo(ARG_mstate_X_((mstate)ms) (STRUCT_mallinfo *)arg);
	return 0;
}
#endif /* USE_PER_THREAD_MSTATE */

DL_API_IMPL(ATTR_PURE WUNUSED, STRUCT_mallinfo, dlmallinfo, (void)) {
	STRUCT_mallinfo nm;
	bzero(&nm, sizeof(nm));
#ifdef HOOK_AFTER_INIT_MALLINFO
	ensure_initialization_for(HOOK_AFTER_INIT_MALLINFO());
#else /* HOOK_AFTER_INIT_MALLINFO */
	ensure_initialization();
#endif /* !HOOK_AFTER_INIT_MALLINFO */
	do_gather_internal_mallinfo(ARG_mstate_gm_ &nm);

	/* Include info from "tls_mspace()" */
#if USE_PER_THREAD_MSTATE
	tls_mspace_foreach(&do_gather_internal_mallinfo_cb, &nm);
#endif /* USE_PER_THREAD_MSTATE */
	return nm;
}
#endif /* !NO_MALLINFO */

#if !NO_MALLOC_STATS
static void internal_malloc_stats(PARAM_mstate_m) {
	size_t maxfp = 0;
	size_t fp    = 0;
	size_t used  = 0;
	ensure_initialization();
	PREACTION(m);
	check_malloc_state(m);
	if (is_initialized(m)) {
		msegmentptr s = &mstate_seg(m);
		maxfp         = mstate_max_footprint(m);
		fp            = mstate_footprint(m);
		used          = fp - (mstate_topsize(m) + TOP_FOOT_SIZE);

		while (s != NULL) {
			mchunkptr q = align_as_chunk(s->base);
			while (segment_holds(s, q) &&
			       q != mstate_top(m) && q->head != FENCEPOST_HEAD) {
				if (!is_inuse(q))
					used -= chunksize(q);
				q = next_chunk(q);
			}
			s = s->next;
		}
	}
	POSTACTION(m); /* drop lock */
	fprintf(stderr, "max system bytes = %10lu\n", (unsigned long)(maxfp));
	fprintf(stderr, "system bytes     = %10lu\n", (unsigned long)(fp));
	fprintf(stderr, "in use bytes     = %10lu\n", (unsigned long)(used));
}
#endif /* NO_MALLOC_STATS */

/* ----------------------- Operations on smallbins ----------------------- */

/*
  Various forms of linking and unlinking are defined as macros.  Even
  the ones for trees, which are very long but have very short typical
  paths.  This is ugly but reduces reliance on inlining support of
  compilers.
*/

/* Link a free chunk into a smallbin  */
#define insert_small_chunk(M, P, S)           \
	{                                         \
		bindex_t I  = small_index(S);         \
		mchunkptr B = smallbin_at(M, I);      \
		mchunkptr F = B;                      \
		dl_assert(S >= MIN_CHUNK_SIZE);       \
		if (!smallmap_is_marked(M, I))        \
			mark_smallmap(M, I);              \
		else {                                \
			ext_assert__ok_address(M, B->fd); \
			F = B->fd;                        \
		}                                     \
		B->fd = P;                            \
		F->bk = P;                            \
		P->fd = F;                            \
		P->bk = B;                            \
	}

/* Unlink a chunk from a smallbin  */
#define unlink_small_chunk(M, P, S)                                                 \
	{                                                                               \
		mchunkptr F = P->fd;                                                        \
		mchunkptr B = P->bk;                                                        \
		bindex_t I  = small_index(S);                                               \
		dl_assert(P != B);                                                          \
		dl_assert(P != F);                                                          \
		dl_assert(chunksize(P) == small_index2size(I));                             \
		ext_assert(F == smallbin_at(M, I) || (ok_address(M, F) && F->bk == P));     \
		if (B == F) {                                                               \
			clear_smallmap(M, I);                                                   \
		} else {                                                                    \
			ext_assert(B == smallbin_at(M, I) || (ok_address(M, B) && B->fd == P)); \
			F->bk = B;                                                              \
			B->fd = F;                                                              \
		}                                                                           \
		dl_setfree_word(P->fd, struct malloc_chunk *);                              \
		dl_setfree_word(P->bk, struct malloc_chunk *);                              \
	}

/* Unlink the first chunk from a smallbin */
#define unlink_first_small_chunk(M, B, P, I)            \
	{                                                   \
		mchunkptr F = P->fd;                            \
		dl_assert(P != B);                              \
		dl_assert(P != F);                              \
		dl_assert(chunksize(P) == small_index2size(I)); \
		if (B == F) {                                   \
			clear_smallmap(M, I);                       \
		} else {                                        \
			ext_assert__ok_address(M, F);               \
			ext_assert(F->bk == P);                     \
			F->bk = B;                                  \
			B->fd = F;                                  \
		}                                               \
		dl_setfree_word(P->fd, struct malloc_chunk *);  \
		dl_setfree_word(P->bk, struct malloc_chunk *);  \
	}

/* Replace dv node, binning the old one */
/* Used only when dvsize known to be small */
#define replace_dv(M, P, S)                 \
	{                                       \
		size_t DVS = mstate_dvsize(M);      \
		dl_assert(is_small(DVS));           \
		if (DVS != 0) {                     \
			mchunkptr DV = mstate_dv(M);    \
			insert_small_chunk(M, DV, DVS); \
		}                                   \
		mstate_dvsize(M) = S;               \
		mstate_dv(M)     = P;               \
	}

/* ------------------------- Operations on trees ------------------------- */

/* Insert chunk into tree */
#define insert_large_chunk(M, X, S)                                                       \
	{                                                                                     \
		tbinptr *H;                                                                       \
		bindex_t I;                                                                       \
		compute_tree_index(S, I);                                                         \
		H           = treebin_at(M, I);                                                   \
		X->index    = I;                                                                  \
		X->child[0] = X->child[1] = NULL;                                                 \
		if (!treemap_is_marked(M, I)) {                                                   \
			mark_treemap(M, I);                                                           \
			*H        = X;                                                                \
			X->parent = (tchunkptr)H;                                                     \
			X->fd = X->bk = X;                                                            \
		} else {                                                                          \
			tchunkptr T = *H;                                                             \
			size_t K    = S << leftshift_for_tree_index(I);                               \
			for (;;) {                                                                    \
				if (chunksize(T) != S) {                                                  \
					tchunkptr *C = &(T->child[(K >> (SIZE_T_BITSIZE - SIZE_T_ONE)) & 1]); \
					K <<= 1;                                                              \
					if (*C != NULL)                                                       \
						T = *C;                                                           \
					else {                                                                \
						ext_assert__ok_address(M, C);                                     \
						*C        = X;                                                    \
						X->parent = T;                                                    \
						X->fd = X->bk = X;                                                \
						break;                                                            \
					}                                                                     \
				} else {                                                                  \
					tchunkptr F = T->fd;                                                  \
					ext_assert__ok_address(M, T);                                         \
					ext_assert__ok_address(M, F);                                         \
					T->fd = F->bk = X;                                                    \
					X->fd         = F;                                                    \
					X->bk         = T;                                                    \
					X->parent     = NULL;                                                 \
					break;                                                                \
				}                                                                         \
			}                                                                             \
		}                                                                                 \
	}

/*
  Unlink steps:

  1. If x is a chained node, unlink it from its same-sized fd/bk links
     and choose its bk node as its replacement.
  2. If x was the last node of its size, but not a leaf node, it must
     be replaced with a leaf node (not merely one with an open left or
     right), to make sure that lefts and rights of descendents
     correspond properly to bit masks.  We use the rightmost descendent
     of x.  We could use any other leaf, but this is easy to locate and
     tends to counteract removal of leftmosts elsewhere, and so keeps
     paths shorter than minimally guaranteed.  This doesn't loop much
     because on average a node in a tree is near the bottom.
  3. If x is the base of a chain (i.e., has parent links) relink
     x's parent and children to x's replacement (or null if none).
*/

#define unlink_large_chunk(M, X)                                  \
	{                                                             \
		tchunkptr XP = X->parent;                                 \
		tchunkptr R;                                              \
		if (X->bk != X) {                                         \
			tchunkptr F = X->fd;                                  \
			R           = X->bk;                                  \
			ext_assert__ok_address(M, F);                         \
			ext_assert(F->bk == X);                               \
			ext_assert(R->fd == X);                               \
			F->bk = R;                                            \
			R->fd = F;                                            \
		} else {                                                  \
			tchunkptr *RP;                                        \
			if (((R = *(RP = &(X->child[1]))) != NULL) ||         \
			    ((R = *(RP = &(X->child[0]))) != NULL)) {         \
				tchunkptr *CP;                                    \
				while ((*(CP = &(R->child[1])) != NULL) ||        \
				       (*(CP = &(R->child[0])) != NULL)) {        \
					R = *(RP = CP);                               \
				}                                                 \
				ext_assert__ok_address(M, RP);                    \
				*RP = NULL;                                       \
			}                                                     \
		}                                                         \
		if (XP != NULL) {                                         \
			tbinptr *H = treebin_at(M, X->index);                 \
			if (X == *H) {                                        \
				if ((*H = R) == NULL)                             \
					clear_treemap(M, X->index);                   \
			} else {                                              \
				ext_assert__ok_address(M, XP);                    \
				if (XP->child[0] == X)                            \
					XP->child[0] = R;                             \
				else                                              \
					XP->child[1] = R;                             \
			}                                                     \
			if (R != NULL) {                                      \
				tchunkptr C0, C1;                                 \
				ext_assert__ok_address(M, R);                     \
				R->parent = XP;                                   \
				if ((C0 = X->child[0]) != NULL) {                 \
					ext_assert__ok_address(M, C0);                \
					R->child[0] = C0;                             \
					C0->parent  = R;                              \
				}                                                 \
				if ((C1 = X->child[1]) != NULL) {                 \
					ext_assert__ok_address(M, C1);                \
					R->child[1] = C1;                             \
					C1->parent  = R;                              \
				}                                                 \
			}                                                     \
		}                                                         \
		dl_setfree_word(X->fd, struct malloc_tree_chunk *);       \
		dl_setfree_word(X->bk, struct malloc_tree_chunk *);       \
		dl_setfree_word(X->child[0], struct malloc_tree_chunk *); \
		dl_setfree_word(X->child[1], struct malloc_tree_chunk *); \
		dl_setfree_word(X->parent, struct malloc_tree_chunk *);   \
		dl_setfree_word(X->index_word, size_t);                   \
	}

/* Relays to large vs small bin operations */

#define insert_chunk(M, P, S)          \
	if (is_small(S)) {                 \
		insert_small_chunk(M, P, S)    \
	} else {                           \
		tchunkptr TP = (tchunkptr)(P); \
		insert_large_chunk(M, TP, S);  \
	}

#define unlink_chunk(M, P, S)          \
	if (is_small(S)) {                 \
		unlink_small_chunk(M, P, S)    \
	} else {                           \
		tchunkptr TP = (tchunkptr)(P); \
		unlink_large_chunk(M, TP);     \
	}


/* Relays to internal calls to malloc/free from realloc, memalign etc */

#if ONLY_MSPACES
#define internal_malloc(m, b) mspace_malloc(m, b)
#define internal_free(m, mem) mspace_free(m, mem);
#elif MSPACES
#define internal_malloc(m, b) \
	(is_global(m) ? dlmalloc(b) : mspace_malloc(m, b))
#if NO_MSPACE_FREE
#define internal_free(m, mem) dlfree(mem)
#else /* NO_MSPACE_FREE */
#define internal_free(m, mem) \
	if (is_global(m))         \
		dlfree(mem);          \
	else                      \
		mspace_free(m, mem);
#endif /* !NO_MSPACE_FREE */
#else /* ... */
#define internal_malloc(m, b) dlmalloc(b)
#define internal_free(m, mem) dlfree(mem)
#endif /* ... */

/* -----------------------  Direct-mmapping chunks ----------------------- */

/*
  Directly mmapped chunks are set up with an offset to the start of
  the mmapped region stored in the prev_foot field of the chunk. This
  allows reconstruction of the required argument to MUNMAP when freed,
  and also allows adjustment of the returned chunk to meet alignment
  requirements (especially in memalign).
*/

/* Malloc using mmap */
static void *mmap_alloc(PARAM_mstate_m_ size_t nb) {
	size_t mmsize = mmap_align(nb + SIX_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
	if (mstate_footprint_limit(m) != 0) {
		size_t fp = mstate_footprint(m) + mmsize;
		if (fp <= mstate_footprint(m) || fp > mstate_footprint_limit(m))
			return NULL;
	}
	if (mmsize > nb) { /* Check for wrap around 0 */
		byte_t *mm = (byte_t *)(DL_DIRECT_MMAP(mmsize));
		if (mm != CMFAIL) {
			size_t offset = align_offset(chunk2mem(mm));
			size_t psize  = mmsize - offset - MMAP_FOOT_PAD;
			mchunkptr p   = (mchunkptr)(mm + offset);
			p->prev_foot  = offset;
			p->head       = psize;
			mark_inuse_foot(m, p, psize);
			chunk_plus_offset(p, psize)->head = FENCEPOST_HEAD;
			chunk_plus_offset(p, psize + SIZE_T_SIZE)->head = 0;

			if (mstate_least_addr(m) == NULL || mm < mstate_least_addr(m))
				mstate_least_addr(m) = mm;
			if ((mstate_footprint(m) += mmsize) > mstate_max_footprint(m))
				mstate_max_footprint(m) = mstate_footprint(m);
			dl_assert(is_aligned(chunk2mem(p)));
			check_mmapped_chunk(m, p);
			return chunk2mem(p);
		}
	}
	return NULL;
}

/* Realloc using mmap */
#undef mmap_resize
#if DL_MREMAP_ALWAYS_FAILS
static mchunkptr mmap_resize(mchunkptr oldp, size_t nb, int flags)
#define mmap_resize(m, oldp, nb, flags) mmap_resize(oldp, nb, flags)
#else /* DL_MREMAP_ALWAYS_FAILS */
static mchunkptr mmap_resize(PARAM_mstate_m_ mchunkptr oldp, size_t nb, int flags)
#define mmap_resize(m, oldp, nb, flags) mmap_resize(ARG_mstate_X_(m) oldp, nb, flags)
#endif /* !DL_MREMAP_ALWAYS_FAILS */
{
	size_t oldsize = chunksize(oldp);
	(void)flags;      /* placate people compiling -Wunused */
	if (is_small(nb)) /* Can't shrink mmap regions below small size */
		return NULL;

	/* Keep old chunk if big enough but not too big */
	if (oldsize >= nb + SIZE_T_SIZE &&
	    (oldsize - nb) <= (mparams.granularity << 1)) {
		return oldp;
	} else {
#if !DL_MREMAP_ALWAYS_FAILS
		size_t offset    = oldp->prev_foot;
		size_t oldmmsize = oldsize + offset + MMAP_FOOT_PAD;
		size_t newmmsize = mmap_align(nb + SIX_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
		byte_t *cp = (byte_t *)DL_MREMAP((byte_t *)oldp - offset, oldmmsize, newmmsize, flags);
		if (cp != CMFAIL) {
			mchunkptr newp = (mchunkptr)(cp + offset);
			size_t psize   = newmmsize - offset - MMAP_FOOT_PAD;
			newp->head     = psize;
			mark_inuse_foot(m, newp, psize);
			chunk_plus_offset(newp, psize)->head = FENCEPOST_HEAD;
			chunk_plus_offset(newp, psize + SIZE_T_SIZE)->head = 0;

			if (cp < mstate_least_addr(m))
				mstate_least_addr(m) = cp;
			if ((mstate_footprint(m) += newmmsize - oldmmsize) > mstate_max_footprint(m))
				mstate_max_footprint(m) = mstate_footprint(m);
			check_mmapped_chunk(m, newp);
			return newp;
		}
#endif /* !DL_MREMAP_ALWAYS_FAILS */
	}
	return NULL;
}


/* -------------------------- mspace management -------------------------- */

/* Initialize top chunk and its size */
static void init_top(PARAM_mstate_m_ mchunkptr p, size_t psize) {
	/* Ensure alignment */
	size_t offset = align_offset(chunk2mem(p));
	p = (mchunkptr)((byte_t *)p + offset);
	psize -= offset;

	mstate_top(m)     = p;
	mstate_topsize(m) = psize;
	p->head           = psize | PINUSE_BIT;
	/* set size of fake trailing chunk holding overhead space only once */
	chunk_plus_offset(p, psize)->head = TOP_FOOT_SIZE;
	mstate_trim_check(m) = mparams.trim_threshold; /* reset on each update */
}

/* Initialize bins for a new mstate that is otherwise zeroed out */
static void init_bins(PARAM_mstate_m) {
	/* Establish circular links for smallbins */
	bindex_t i;
	for (i = 0; i < NSMALLBINS; ++i) {
		sbinptr bin = smallbin_at(m, i);
		bin->fd = bin->bk = bin;
	}
}

#if PROCEED_ON_ERROR

/* default corruption action */
static void reset_on_error(PARAM_mstate_m) {
	int i;
	++malloc_corruption_error_count;
	/* Reinitialize fields to forget about all memory */
	mstate_smallmap(m) = mstate_treemap(m) = 0;
	mstate_dvsize(m) = mstate_topsize(m) = 0;
	mstate_seg(m).base = NULL;
	mstate_seg(m).size = 0;
	mstate_seg(m).next = NULL;
	mstate_top(m) = mstate_dv(m) = 0;
	for (i = 0; i < NTREEBINS; ++i)
		*treebin_at(m, i) = NULL;
	init_bins(m);
}
#endif /* PROCEED_ON_ERROR */

/* Allocate chunk and prepend remainder with chunk in successor base. */
static void *prepend_alloc(PARAM_mstate_m_ byte_t *newbase, byte_t *oldbase, size_t nb) {
	mchunkptr p        = align_as_chunk(newbase);
	mchunkptr oldfirst = align_as_chunk(oldbase);
	size_t psize       = (byte_t *)oldfirst - (byte_t *)p;
	mchunkptr q        = chunk_plus_offset(p, nb);
	size_t qsize       = psize - nb;
	set_size_and_pinuse_of_inuse_chunk(m, p, nb);

	dl_assert((byte_t *)oldfirst > (byte_t *)q);
	dl_assert(pinuse(oldfirst));
	dl_assert(qsize >= MIN_CHUNK_SIZE);

	/* consolidate remainder with first chunk of old base */
	if (oldfirst == mstate_top(m)) {
		size_t tsize  = mstate_topsize(m) += qsize;
		mstate_top(m) = q;
		q->head       = tsize | PINUSE_BIT;
		check_top_chunk(m, q);
		dl_setfree_word_untested(oldfirst->prev_foot, size_t);
		dl_setfree_word_untested(oldfirst->head, size_t);
	} else if (oldfirst == mstate_dv(m)) {
		size_t dsize = mstate_dvsize(m) += qsize;
		mstate_dv(m) = q;
		set_size_and_pinuse_of_free_chunk(q, dsize);
		dl_setfree_word_untested(oldfirst->prev_foot, size_t);
		dl_setfree_word_untested(oldfirst->head, size_t);
	} else {
		if (!is_inuse(oldfirst)) {
			size_t nsize = chunksize(oldfirst);
			unlink_chunk(m, oldfirst, nsize);
			dl_setfree_word_untested(oldfirst->prev_foot, size_t);
			dl_setfree_word_untested(oldfirst->head, size_t);
			oldfirst = chunk_plus_offset(oldfirst, nsize);
			qsize += nsize;
		}
		set_free_with_pinuse(q, qsize, oldfirst);
		insert_chunk(m, q, qsize);
		check_free_chunk(m, q);
	}

	check_malloced_chunk(m, chunk2mem(p), nb);
	return chunk2mem(p);
}

/* Add a segment to hold a new noncontiguous region */
static void add_segment(PARAM_mstate_m_ byte_t *tbase, size_t tsize, flag_t mmapped) {
	/* Determine locations and sizes of segment, fenceposts, old top */
	byte_t *old_top   = (byte_t *)mstate_top(m);
	msegmentptr oldsp = segment_holding(ARG_mstate_m_ old_top);
	byte_t *old_end   = oldsp->base + oldsp->size;
	size_t ssize      = pad_request(sizeof(struct malloc_segment));
	byte_t *rawsp     = old_end - (ssize + FOUR_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
	size_t offset     = align_offset(chunk2mem(rawsp));
	byte_t *asp       = rawsp + offset;
	byte_t *csp       = (asp < (old_top + MIN_CHUNK_SIZE)) ? old_top : asp;
	mchunkptr sp      = (mchunkptr)csp;
	msegmentptr ss    = (msegmentptr)(chunk2mem(sp));
	mchunkptr tnext   = chunk_plus_offset(sp, ssize);
	mchunkptr p       = tnext;
	int nfences       = 0;

	/* reset top to new space */
	init_top(ARG_mstate_m_ (mchunkptr)tbase, tsize - TOP_FOOT_SIZE);

	/* Set up segment record */
	dl_assert(is_aligned(ss));
	set_size_and_pinuse_of_inuse_chunk(m, sp, ssize);
	*ss                  = mstate_seg(m); /* Push current record */
	mstate_seg(m).base   = tbase;
	mstate_seg(m).size   = tsize;
	mstate_seg(m).sflags = mmapped;
	mstate_seg(m).next   = ss;

	/* Insert trailing fenceposts */
	for (;;) {
		mchunkptr nextp = chunk_plus_offset(p, SIZE_T_SIZE);
		p->head         = FENCEPOST_HEAD;
		++nfences;
		if ((byte_t *)(&(nextp->head)) < old_end)
			p = nextp;
		else
			break;
	}
	dl_assert(nfences >= 2);

	/* Insert the rest of old top into a bin as an ordinary free chunk */
	if (csp != old_top) {
		mchunkptr q  = (mchunkptr)old_top;
		size_t psize = csp - old_top;
		mchunkptr tn = chunk_plus_offset(q, psize);
		set_free_with_pinuse(q, psize, tn);
		insert_chunk(m, q, psize);
	}

	check_top_chunk(m, mstate_top(m));
}

/* -------------------------- System allocation -------------------------- */

/* Get memory from system using MORECORE or MMAP */
static void *sys_alloc(PARAM_mstate_m_ size_t nb) {
	byte_t *tbase    = CMFAIL;
	size_t tsize     = 0;
	flag_t mmap_flag = 0;
	size_t asize; /* allocation size */

#if !USE_LOCKS || MSPACES || GM_ONLY
	ensure_initialization();
#endif /* !USE_LOCKS || MSPACES || GM_ONLY */

	/* Directly map large chunks, but only if already initialized */
	if (use_mmap(m) && nb >= mparams.mmap_threshold && mstate_topsize(m) != 0) {
		void *mem = mmap_alloc(ARG_mstate_m_ nb);
		if (mem != NULL)
			return mem;
	}

	asize = granularity_align(nb + SYS_ALLOC_PADDING);
	if (asize <= nb)
		return NULL; /* wraparound */
	if (mstate_footprint_limit(m) != 0) {
		size_t fp = mstate_footprint(m) + asize;
		if (fp <= mstate_footprint(m) || fp > mstate_footprint_limit(m))
			return NULL;
	}

	/*
	   Try getting memory in any of three ways (in most-preferred to
	   least-preferred order):
	   1. A call to MORECORE that can normally contiguously extend memory.
	      (disabled if not MORECORE_CONTIGUOUS or not HAVE_MORECORE or
	      or main space is mmapped or a previous contiguous call failed)
	   2. A call to MMAP new space (disabled if not HAVE_MMAP).
	      Note that under the default settings, if MORECORE is unable to
	      fulfill a request, and HAVE_MMAP is true, then mmap is
	      used as a noncontiguous system allocator. This is a useful backup
	      strategy for systems with holes in address spaces -- in this case
	      sbrk cannot contiguously expand the heap, but mmap may be able to
	      find space.
	   3. A call to MORECORE that cannot usually contiguously extend memory.
	      (disabled if not HAVE_MORECORE)

	  In all cases, we need to request enough bytes from system to ensure
	  we can malloc nb bytes upon success, so pad with enough space for
	  top_foot, plus alignment-pad to make sure we don't lose bytes if
	  not on boundary, and round this up to a granularity unit.
	 */

#if MORECORE_CONTIGUOUS
	if (!use_noncontiguous(m)) {
		byte_t *br     = CMFAIL;
		size_t ssize   = asize; /* sbrk call size */
		msegmentptr ss = (mstate_top(m) == NULL) ? NULL : segment_holding(ARG_mstate_m_ (byte_t *)mstate_top(m));
		ACQUIRE_MALLOC_GLOBAL_LOCK();

		if (ss == NULL) { /* First time through or recovery */
			byte_t *base = (byte_t *)sbrk(0);
			if (base != CMFAIL) {
				size_t fp;
				/* Adjust to end on a page boundary */
				if (!is_page_aligned(base))
					ssize += (page_align((size_t)base) - (size_t)base);
				fp = mstate_footprint(m) + ssize; /* recheck limits */
				if (ssize > nb && ssize < HALF_MAX_SIZE_T &&
				    (mstate_footprint_limit(m) == 0 ||
				     (fp > mstate_footprint(m) && fp <= mstate_footprint_limit(m))) &&
				    (br = (byte_t *)(sbrk(ssize))) == base) {
					tbase = base;
					tsize = ssize;
				}
			}
		} else {
			/* Subtract out existing available top space from MORECORE request. */
			ssize = granularity_align(nb - mstate_topsize(m) + SYS_ALLOC_PADDING);
			/* Use mem here only if it did continuously extend old space */
			if (ssize < HALF_MAX_SIZE_T &&
			    (br = (byte_t *)(sbrk(ssize))) == ss->base + ss->size) {
				tbase = br;
				tsize = ssize;
			}
		}

		if (tbase == CMFAIL) {  /* Cope with partial failure */
			if (br != CMFAIL) { /* Try to use/extend the space we did get */
				if (ssize < HALF_MAX_SIZE_T &&
				    ssize < nb + SYS_ALLOC_PADDING) {
					size_t esize = granularity_align(nb + SYS_ALLOC_PADDING - ssize);
					if (esize < HALF_MAX_SIZE_T) {
						byte_t *end = (byte_t *)sbrk(esize);
						if (end != CMFAIL)
							ssize += esize;
						else { /* Can't use; try to release */
							(void)sbrk(-ssize);
							br = CMFAIL;
						}
					}
				}
			}
			if (br != CMFAIL) { /* Use the space we did get */
				tbase = br;
				tsize = ssize;
			} else
				disable_contiguous(m); /* Don't try contiguous path in the future */
		}

		RELEASE_MALLOC_GLOBAL_LOCK();
	}
#endif /* MORECORE_CONTIGUOUS */

#if HAVE_MMAP
	if (tbase == CMFAIL) { /* Try MMAP */
		byte_t *mp = (byte_t *)(DL_MMAP(asize));
		if (mp != CMFAIL) {
			tbase     = mp;
			tsize     = asize;
			mmap_flag = USE_MMAP_BIT;
		}
	}
#endif /* HAVE_MMAP */

#if HAVE_MORECORE
	if (tbase == CMFAIL) { /* Try noncontiguous MORECORE */
		if (asize < HALF_MAX_SIZE_T) {
			byte_t *br  = CMFAIL;
			byte_t *end = CMFAIL;
			ACQUIRE_MALLOC_GLOBAL_LOCK();
			br  = (byte_t *)(sbrk(asize));
			end = (byte_t *)(sbrk(0));
			RELEASE_MALLOC_GLOBAL_LOCK();
			if (br != CMFAIL && end != CMFAIL && br < end) {
				size_t ssize = end - br;
				if (ssize > nb + TOP_FOOT_SIZE) {
					tbase = br;
					tsize = ssize;
				}
			}
		}
	}
#endif /* HAVE_MORECORE */

	if (tbase != CMFAIL) {
		dl_setfree_data(tbase, tsize);

		if ((mstate_footprint(m) += tsize) > mstate_max_footprint(m))
			mstate_max_footprint(m) = mstate_footprint(m);

		if (!is_initialized(m)) { /* first-time initialization */
			if (mstate_least_addr(m) == NULL || tbase < mstate_least_addr(m))
				mstate_least_addr(m) = tbase;
			mstate_seg(m).base       = tbase;
			mstate_seg(m).size       = tsize;
			mstate_seg(m).sflags     = mmap_flag;
			mstate_magic(m)          = mparams.magic;
			mstate_release_checks(m) = MAX_RELEASE_CHECK_RATE;
			init_bins(ARG_mstate_m);
#if GM_ONLY
			init_top(ARG_mstate_m_ (mchunkptr)tbase, tsize - TOP_FOOT_SIZE);
#else /* GM_ONLY */
#if !ONLY_MSPACES
			if (is_global(m)) {
				init_top(ARG_mstate_m_ (mchunkptr)tbase, tsize - TOP_FOOT_SIZE);
			} else
#endif /* !ONLY_MSPACES */
			{
				/* Offset top by embedded malloc_state */
				mchunkptr mn = next_chunk(mem2chunk(m));
				init_top(ARG_mstate_m_ mn, (size_t)((tbase + tsize) - (byte_t *)mn) - TOP_FOOT_SIZE);
			}
#endif /* !GM_ONLY */
		} else {
			/* Try to merge with an existing segment */
			msegmentptr sp = &mstate_seg(m);
			/* Only consider most recent segment if traversal suppressed */
			while (sp != NULL && tbase != sp->base + sp->size)
				sp = (NO_SEGMENT_TRAVERSAL) ? NULL : sp->next;
			if (sp != NULL &&
#if EXTERN_BIT
			    !is_extern_segment(sp) &&
#endif /* EXTERN_BIT */
			    (sp->sflags & USE_MMAP_BIT) == mmap_flag &&
			    segment_holds(sp, mstate_top(m))) { /* append */
				sp->size += tsize;
				init_top(ARG_mstate_m_ mstate_top(m), mstate_topsize(m) + tsize);
			} else {
				if (tbase < mstate_least_addr(m))
					mstate_least_addr(m) = tbase;
				sp = &mstate_seg(m);
				while (sp != NULL && sp->base != tbase + tsize)
					sp = (NO_SEGMENT_TRAVERSAL) ? NULL : sp->next;
				if (sp != NULL &&
#if EXTERN_BIT
				    !is_extern_segment(sp) &&
#endif /* EXTERN_BIT */
				    (sp->sflags & USE_MMAP_BIT) == mmap_flag) {
					byte_t *oldbase = sp->base;
					sp->base      = tbase;
					sp->size += tsize;
					return prepend_alloc(ARG_mstate_m_ tbase, oldbase, nb);
				} else
					add_segment(ARG_mstate_m_ tbase, tsize, mmap_flag);
			}
		}

		if (nb < mstate_topsize(m)) { /* Allocate from new or extended top space */
			size_t rsize = mstate_topsize(m) -= nb;
			mchunkptr p  = mstate_top(m);
			mchunkptr r = mstate_top(m) = chunk_plus_offset(p, nb);
			r->head = rsize | PINUSE_BIT;
			set_size_and_pinuse_of_inuse_chunk(m, p, nb);
			check_top_chunk(m, mstate_top(m));
			check_malloced_chunk(m, chunk2mem(p), nb);
			return chunk2mem(p);
		}
	}

	MALLOC_FAILURE_ACTION;
	return NULL;
}

/* -----------------------  system deallocation -------------------------- */

/* Unmap and unlink any mmapped segments that don't contain used chunks */
static size_t release_unused_segments(PARAM_mstate_m) {
	size_t released  = 0;
	int nsegs        = 0;
	msegmentptr pred = &mstate_seg(m);
	msegmentptr sp   = pred->next;
	while (sp != NULL) {
		byte_t *base     = sp->base;
		size_t size      = sp->size;
		msegmentptr next = sp->next;
		++nsegs;
#if EXTERN_BIT
		if (is_mmapped_segment(sp) && !is_extern_segment(sp))
#else /* EXTERN_BIT */
		if (is_mmapped_segment(sp))
#endif /* !EXTERN_BIT */
		{
			mchunkptr p  = align_as_chunk(base);
			size_t psize = chunksize(p);
			/* Can unmap if first chunk holds entire segment and not pinned */
			if (!is_inuse(p) && (byte_t *)p + psize >= base + size - TOP_FOOT_SIZE) {
				tchunkptr tp = (tchunkptr)p;
				dl_assert(segment_holds(sp, (byte_t *)sp));
				check_memset_use_after_free(m, p);
				if (p == mstate_dv(m)) {
					mstate_dv(m)     = NULL;
					mstate_dvsize(m) = 0;
				} else {
					unlink_large_chunk(m, tp);
				}
#if !DL_MUNMAP_ALWAYS_FAILS
				if (DL_MUNMAP(base, size) == 0) {
					released += size;
					mstate_footprint(m) -= size;
					/* unlink obsoleted record */
					sp       = pred;
					sp->next = next;
				} else
#endif /* !DL_MUNMAP_ALWAYS_FAILS */
				{ /* back out if cannot unmap */
					insert_large_chunk(m, tp, psize);
				}
			}
		}
		if (NO_SEGMENT_TRAVERSAL) /* scan only first segment */
			break;
		pred = sp;
		sp   = next;
	}
	/* Reset check counter */
	mstate_release_checks(m) = (((size_t)nsegs > (size_t)MAX_RELEASE_CHECK_RATE)
	                            ? (size_t)nsegs
	                            : (size_t)MAX_RELEASE_CHECK_RATE);
	return released;
}

static dlmalloc_trim_RETURN_TYPE sys_trim(PARAM_mstate_m_ size_t pad) {
	size_t released = 0;
	if (pad < MAX_REQUEST && is_initialized(m)) {
		pad += TOP_FOOT_SIZE; /* ensure enough room for segment overhead */

		if (mstate_topsize(m) > pad) {
			/* Shrink top space in granularity-size units, keeping at least one */
			size_t unit  = mparams.granularity;
			size_t extra = ((mstate_topsize(m) - pad + (unit - SIZE_T_ONE)) / unit -
			                SIZE_T_ONE) *
			               unit;
			msegmentptr sp = segment_holding(ARG_mstate_m_ (byte_t *) mstate_top(m));
			(void)extra;

#if EXTERN_BIT
			if (!is_extern_segment(sp))
#endif /* EXTERN_BIT */
			{
				if (is_mmapped_segment(sp)) {
#if HAVE_MMAP && (!DL_MREMAP_ALWAYS_FAILS || !DL_MUNMAP_ALWAYS_FAILS)
					if (sp->size >= extra &&
					    !has_segment_link(ARG_mstate_m_ sp)) { /* can't shrink if pinned */
						size_t newsize = sp->size - extra;
						(void)newsize; /* placate people compiling -Wunused-variable */
						/* Prefer mremap, fall back to munmap */
						if (0 ||
#if !DL_MREMAP_ALWAYS_FAILS
						    (DL_MREMAP(sp->base, sp->size, newsize, 0) != MFAIL) ||
#endif /* !DL_MREMAP_ALWAYS_FAILS */
#if !DL_MUNMAP_ALWAYS_FAILS
						    (DL_MUNMAP(sp->base + newsize, extra) == 0) ||
#endif /* !DL_MUNMAP_ALWAYS_FAILS */
							0) {
							released = extra;
						}
					}
#endif /* HAVE_MMAP && (!DL_MREMAP_ALWAYS_FAILS || !DL_MUNMAP_ALWAYS_FAILS) */
				} else {
#if HAVE_MORECORE
					if (extra >= HALF_MAX_SIZE_T) /* Avoid wrapping negative */
						extra = (HALF_MAX_SIZE_T) + SIZE_T_ONE - unit;
					ACQUIRE_MALLOC_GLOBAL_LOCK();
					{
						/* Make sure end of memory is where we last set it. */
						byte_t *old_br = (byte_t *)(sbrk(0));
						if (old_br == sp->base + sp->size) {
							byte_t *rel_br = (byte_t *)(sbrk(-extra));
							byte_t *new_br = (byte_t *)(sbrk(0));
							if (rel_br != CMFAIL && new_br < old_br)
								released = old_br - new_br;
						}
					}
					RELEASE_MALLOC_GLOBAL_LOCK();
#endif /*  HAVE_MORECORE*/
				}
			}

			if (released != 0) {
				sp->size -= released;
				mstate_footprint(m) -= released;
				init_top(ARG_mstate_m_ mstate_top(m), mstate_topsize(m) - released);
				check_top_chunk(m, mstate_top(m));
			}
		}

		/* Unmap any unused mmapped segments */
#if HAVE_MMAP
		released += release_unused_segments(ARG_mstate_m);
#endif /* HAVE_MMAP */

		/* On failure, disable autotrim to avoid repeated failed future calls */
		if (released == 0 && mstate_topsize(m) > mstate_trim_check(m))
			mstate_trim_check(m) = SIZE_MAX;
	}

#if dlmalloc_trim_RETURNS_SIZE_T
	return released;
#else /* dlmalloc_trim_RETURNS_SIZE_T */
	return (released != 0) ? 1 : 0;
#endif /* !dlmalloc_trim_RETURNS_SIZE_T */
}

/* Consolidate and bin a chunk. Differs from exported versions
   of free mainly in that the chunk need not be marked as inuse.
*/
static void dispose_chunk(PARAM_mstate_m_ mchunkptr p, size_t psize) {
	mchunkptr next = chunk_plus_offset(p, psize);
	if (!pinuse(p)) {
		mchunkptr prev;
		size_t prevsize = p->prev_foot;
		if (is_mmapped(p)) {
			psize += prevsize + MMAP_FOOT_PAD;
#if !DL_MUNMAP_ALWAYS_FAILS
			if (DL_MUNMAP((byte_t *)p - prevsize, psize) == 0)
				mstate_footprint(m) -= psize;
#endif /* !DL_MUNMAP_ALWAYS_FAILS */
			return;
		}
		prev = chunk_minus_offset(p, prevsize);
		psize += prevsize;
		dl_setfree_word(p->prev_foot, size_t);
		dl_setfree_word(p->head, size_t);
		p = prev;
		ext_assert__ok_address(m, prev);
		/* consolidate backward */
		if (p != mstate_dv(m)) {
			unlink_chunk(m, p, prevsize);
		} else if ((next->head & INUSE_BITS) == INUSE_BITS) {
			mstate_dvsize(m) = psize;
			set_free_with_pinuse(p, psize, next);
			return;
		}
	}

	ext_assert__ok_address(m, next);
	if (!cinuse(next)) {
		/* consolidate forward */
		if (next == mstate_top(m)) {
			size_t tsize  = mstate_topsize(m) += psize;
			mstate_top(m) = p;
			p->head       = tsize | PINUSE_BIT;
			dl_setfree_word(next->prev_foot, size_t);
			dl_setfree_word(next->head, size_t);
			if (p == mstate_dv(m)) {
				mstate_dv(m)     = NULL;
				mstate_dvsize(m) = 0;
			}
			return;
		} else if (next == mstate_dv(m)) {
			size_t dsize = mstate_dvsize(m) += psize;
			mstate_dv(m) = p;
			dl_setfree_word(next->prev_foot, size_t);
			dl_setfree_word(next->head, size_t);
			set_size_and_pinuse_of_free_chunk(p, dsize);
			return;
		} else {
			size_t nsize = chunksize(next);
			psize += nsize;
			unlink_chunk(m, next, nsize);
			dl_setfree_word(next->prev_foot, size_t);
			dl_setfree_word(next->head, size_t);
			set_size_and_pinuse_of_free_chunk(p, psize);
			if (p == mstate_dv(m)) {
				mstate_dvsize(m) = psize;
				return;
			}
		}
	} else {
		set_free_with_pinuse(p, psize, next);
	}
	insert_chunk(m, p, psize);
}

/* ---------------------------- malloc --------------------------- */

/* allocate a large request from the best fitting chunk in a treebin */
static void *tmalloc_large(PARAM_mstate_m_ size_t nb) {
	tchunkptr v  = NULL;
	size_t rsize = SIZE_T_ZERO - nb; /* Unsigned negation */
	tchunkptr t;
	bindex_t idx;
	compute_tree_index(nb, idx);
	if ((t = *treebin_at(m, idx)) != NULL) {
		/* Traverse tree for this bin looking for node with size == nb */
		size_t sizebits = nb << leftshift_for_tree_index(idx);
		tchunkptr rst   = NULL; /* The deepest untaken right subtree */
		for (;;) {
			tchunkptr rt;
			size_t trem = chunksize(t) - nb;
			if (trem < rsize) {
				v = t;
				if ((rsize = trem) == 0)
					break;
			}
			rt = t->child[1];
			t  = t->child[(sizebits >> (SIZE_T_BITSIZE - SIZE_T_ONE)) & 1];
			if (rt != NULL && rt != t)
				rst = rt;
			if (t == NULL) {
				t = rst; /* set t to least subtree holding sizes > nb */
				break;
			}
			sizebits <<= 1;
		}
	}
	if (t == NULL && v == NULL) { /* set t to root of next non-empty treebin */
		binmap_t leftbits = left_bits(idx2bit(idx)) & mstate_treemap(m);
		if (leftbits != 0) {
			bindex_t i;
			binmap_t leastbit = least_bit(leftbits);
			compute_bit2idx(leastbit, i);
			t = *treebin_at(m, i);
		}
	}

	while (t != NULL) { /* find smallest of tree or subtree */
		size_t trem = chunksize(t) - nb;
		if (trem < rsize) {
			rsize = trem;
			v     = t;
		}
		t = leftmost_child(t);
	}

	/*  If dv is a better fit, return 0 so malloc will use it */
	if (v != NULL && rsize < (size_t)(mstate_dvsize(m) - nb)) {
		mchunkptr r;
		ext_assert__ok_address(m, v);
		r = chunk_plus_offset(v, nb); /* split */
		dl_assert(chunksize(v) == rsize + nb);
		ext_assert__ok_next(v, r);
		unlink_large_chunk(m, v);
		if (rsize < MIN_CHUNK_SIZE)
			set_inuse_and_pinuse(m, v, (rsize + nb));
		else {
			set_size_and_pinuse_of_inuse_chunk(m, v, nb);
			set_size_and_pinuse_of_free_chunk(r, rsize);
			insert_chunk(m, r, rsize);
		}
		check_memset_use_after_free(m, (mchunkptr)v);
		return chunk2mem(v);
	}
	return NULL;
}

/* allocate a small request from the best fitting chunk in a treebin */
static void *tmalloc_small(PARAM_mstate_m_ size_t nb) {
	tchunkptr t, v;
	mchunkptr r;
	size_t rsize;
	bindex_t i;
	binmap_t leastbit = least_bit(mstate_treemap(m));
	compute_bit2idx(leastbit, i);
	v = t = *treebin_at(m, i);
	rsize = chunksize(t) - nb;

	while ((t = leftmost_child(t)) != NULL) {
		size_t trem = chunksize(t) - nb;
		if (trem < rsize) {
			rsize = trem;
			v     = t;
		}
	}

	ext_assert__ok_address(m, v);
	r = chunk_plus_offset(v, nb);
	dl_assert(chunksize(v) == rsize + nb);
	ext_assert__ok_next(v, r);
	unlink_large_chunk(m, v);
	if (rsize < MIN_CHUNK_SIZE)
		set_inuse_and_pinuse(m, v, (rsize + nb));
	else {
		set_size_and_pinuse_of_inuse_chunk(m, v, nb);
		set_size_and_pinuse_of_free_chunk(r, rsize);
		replace_dv(m, r, rsize);
	}
	check_memset_use_after_free(m, (mchunkptr)v);
	return chunk2mem(v);
}

#if !ONLY_MSPACES

DL_API_IMPL(ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)), void *, dlmalloc, (size_t bytes)) {
	/*
	   Basic algorithm:
	   If a small request (< 256 bytes minus per-chunk overhead):
	     1. If one exists, use a remainderless chunk in associated smallbin.
	        (Remainderless means that there are too few excess bytes to
	        represent as a chunk.)
	     2. If it is big enough, use the dv chunk, which is normally the
	        chunk adjacent to the one used for the most recent small request.
	     3. If one exists, split the smallest available chunk in a bin,
	        saving remainder in dv.
	     4. If it is big enough, use the top chunk.
	     5. If available, get memory from system and use it
	   Otherwise, for a large request:
	     1. Find the smallest available binned chunk that fits, and use it
	        if it is better fitting than dv chunk, splitting if necessary.
	     2. If better fitting than any binned chunk, use the dv chunk.
	     3. If it is big enough, use the top chunk.
	     4. If request size >= mmap threshold, try to directly mmap this chunk.
	     5. If available, get memory from system and use it
	
	   The ugly goto's here ensure that postaction occurs along all paths.
	*/
	void *mem;
	size_t nb;

#if dlmalloc_ZERO_RETURNS_NULL
	if unlikely(!bytes)
		return NULL;
#endif /* dlmalloc_ZERO_RETURNS_NULL */
#ifdef HOOK_AFTER_INIT_MALLOC
	ensure_initialization_for(HOOK_AFTER_INIT_MALLOC(bytes));
#else /* HOOK_AFTER_INIT_MALLOC */
#if USE_LOCKS && !GM_STATIC_INIT_MUTEX
	ensure_initialization(); /* initialize in sys_alloc if not using locks */
#endif /* USE_LOCKS && !GM_STATIC_INIT_MUTEX */
#endif /* !HOOK_AFTER_INIT_MALLOC */

	/* This "PREACTION(gm)" right here was ****THE**** biggest
	 * bottleneck when it came to hosting heavily parallelized
	 * programs!
	 *
	 * Solution:
	 * - Thread-local heap
	 *   - including per-thread free lists, since it isn't
	 *     actually that likely for the allocating thread to
	 *     also be the one that free's memory, thanks to the
	 *     design of the "leaks" detector's pending lists
	 */
#if USE_PER_THREAD_MSTATE
	if (!TRY_PREACTION(gm)) {
		mspace tls = tls_mspace();
		if (tls) {
#if USE_MSPACE_MALLOC_LOCKLESS
			/* Lock whichever mspace becomes available first ("tls" or "gm") */
			for (;;) {
				if (TRY_PREACTION((mstate)tls)) {
					mem = mspace_malloc_lockless(tls, bytes);
					POSTACTION((mstate)tls);
					return mem;
				}
				if (TRY_PREACTION(gm))
					break;
				SCHED_YIELD();
			}
#else /* USE_MSPACE_MALLOC_LOCKLESS */
			return mspace_malloc(tls, bytes);
#endif /* !USE_MSPACE_MALLOC_LOCKLESS */
		} else {
			PREACTION(gm);
		}
	}
#else /* USE_PER_THREAD_MSTATE */
	PREACTION(gm);
#endif /* !USE_PER_THREAD_MSTATE */
	if (bytes <= MAX_SMALL_REQUEST) {
		bindex_t idx;
		binmap_t smallbits;
		nb        = (bytes < MIN_REQUEST) ? MIN_CHUNK_SIZE : pad_request(bytes);
		idx       = small_index(nb);
		smallbits = gm__smallmap >> idx;

		if ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
			mchunkptr b, p;
			idx += ~smallbits & 1; /* Uses next bin if idx empty */
			b = smallbin_at(gm, idx);
			p = b->fd;
			dl_assert(chunksize(p) == small_index2size(idx));
			unlink_first_small_chunk(gm, b, p, idx);
			set_inuse_and_pinuse(gm, p, small_index2size(idx));
			mem = chunk2mem(p);
			check_malloced_chunk(gm, mem, nb);
			check_memset_use_after_free(gm, p);
			goto postaction;
		}

		else if (nb > gm__dvsize) {
			if (smallbits != 0) { /* Use chunk in next nonempty smallbin */
				mchunkptr b, p, r;
				size_t rsize;
				bindex_t i;
				binmap_t leftbits = (smallbits << idx) & left_bits(idx2bit(idx));
				binmap_t leastbit = least_bit(leftbits);
				compute_bit2idx(leastbit, i);
				b = smallbin_at(gm, i);
				p = b->fd;
				dl_assert(chunksize(p) == small_index2size(i));
				unlink_first_small_chunk(gm, b, p, i);
				rsize = small_index2size(i) - nb;
				/* Fit here cannot be remainderless if 4byte sizes */
#if SIZE_T_SIZE != 4
				if (rsize < MIN_CHUNK_SIZE)
					set_inuse_and_pinuse(gm, p, small_index2size(i));
				else
#endif /* SIZE_T_SIZE != 4 */
				{
					set_size_and_pinuse_of_inuse_chunk(gm, p, nb);
					r = chunk_plus_offset(p, nb);
					set_size_and_pinuse_of_free_chunk(r, rsize);
					replace_dv(gm, r, rsize);
				}
				mem = chunk2mem(p);
				check_malloced_chunk(gm, mem, nb);
				check_memset_use_after_free(gm, p);
				goto postaction;
			}

			else if (gm__treemap != 0 && (mem = tmalloc_small(ARG_mstate_gm_ nb)) != NULL) {
				check_malloced_chunk(gm, mem, nb);
				goto postaction;
			}
		}
	} else if (bytes >= MAX_REQUEST)
		nb = SIZE_MAX; /* Too big to allocate. Force failure (in sys alloc) */
	else {
		nb = pad_request(bytes);
		if (gm__treemap != 0 && (mem = tmalloc_large(ARG_mstate_gm_ nb)) != NULL) {
			check_malloced_chunk(gm, mem, nb);
			goto postaction;
		}
	}

	if (nb <= gm__dvsize) {
		size_t rsize = gm__dvsize - nb;
		mchunkptr p  = gm__dv;
		if (rsize >= MIN_CHUNK_SIZE) { /* split dv */
			mchunkptr r = gm__dv = chunk_plus_offset(p, nb);
			gm__dvsize           = rsize;
			set_size_and_pinuse_of_free_chunk(r, rsize);
			set_size_and_pinuse_of_inuse_chunk(gm, p, nb);
		} else { /* exhaust dv */
			size_t dvs = gm__dvsize;
			gm__dvsize = 0;
			gm__dv     = NULL;
			set_inuse_and_pinuse(gm, p, dvs);
		}
		mem = chunk2mem(p);
		check_malloced_chunk(gm, mem, nb);
		check_memset_use_after_free(gm, p);
		goto postaction;
	}

	else if (nb < gm__topsize) { /* Split top */
		size_t rsize = gm__topsize -= nb;
		mchunkptr p  = gm__top;
		mchunkptr r = gm__top = chunk_plus_offset(p, nb);
		r->head = rsize | PINUSE_BIT;
		set_size_and_pinuse_of_inuse_chunk(gm, p, nb);
		mem = chunk2mem(p);
		check_top_chunk(gm, gm__top);
		check_malloced_chunk(gm, mem, nb);
		check_memset_use_after_free(gm, p);
		goto postaction;
	}

	mem = sys_alloc(ARG_mstate_gm_ nb);

postaction:
	POSTACTION(gm);
#if DL_DEBUG_MEMSET_ALLOC
	if (mem != NULL) {
		mchunkptr p = mem2chunk(mem);
		dl_setalloc_data(mem, chunksize(p) - overhead_for(p));
	}
#endif /* DL_DEBUG_MEMSET_ALLOC */
	return mem;
}

/* ---------------------------- free --------------------------- */
#if FLAG4_BIT_HEAP_REGION

#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
static ATTR_RETNONNULL NONNULL((1)) STRUCT_dlheapregion *DCALL
dl_dbg_heapregion_dispose(STRUCT_dlheapregion *__restrict region);
#endif /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */

/* Meaning of flags when "STRUCT_dlheapregion" comes into play:
 *                 ALLOCATED    FREED
 * - 1 PINUSE_BIT  clear        set
 * - 2 CINUSE_BIT  clear        undefined
 * - 4 FLAG4_BIT   set          undefined
 *
 * Head/first: prev_foot == 0
 *             head = size | FLAG4_BIT    (if allocated)
 *             head = size | PINUSE_BIT   (if freed)      (CINUSE_BIT and FLAG4_BIT don't matter)
 * Regular:    prev_foot == prev_size
 *             head = size | FLAG4_BIT    (if allocated)
 *             head = size | PINUSE_BIT   (if freed)      (CINUSE_BIT and FLAG4_BIT don't matter)
 * Tail:       prev_foot == prev_size
 *             head = 0
 */
static ATTR_NOINLINE void free_flag4_mem(mchunkptr p) {
	size_t psize    = chunksize(p);
	mchunkptr next  = chunk_plus_offset(p, psize);
	size_t prevsize = p->prev_foot;
	mchunkptr prev  = chunk_minus_offset(p, prevsize);
#if !FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
	ext_assert(pinuse(next) || flag4inuse(next) || next->head == 0);
#endif /* !FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
	ext_assert(pinuse(prev) || flag4inuse(prev));
#if 0
	dl_assert(p != gm__top);
	dl_assert(p != gm__dv);
	dl_assert(next != gm__top);
	dl_assert(next != gm__dv);
	dl_assert(prev != gm__top);
	dl_assert(prev != gm__dv);
#endif
	if (pinuse(prev) && prevsize != 0) {
		/* consolidate backward */
		dl_setfree_word(p->prev_foot, size_t);
		dl_setfree_word(p->head, size_t);
		psize += prevsize;
		p = prev;
		prevsize = p->prev_foot;
		prev = chunk_minus_offset(p, prevsize);
	}
	if (pinuse(next)) {
		/* consolidate forward */
		psize += chunksize(next);
		dl_setfree_word(next->prev_foot, size_t);
		dl_setfree_word(next->head, size_t);
		next = chunk_plus_offset(p, psize);
	}
	p->head = (psize | PINUSE_BIT);
	set_foot(p, psize);
#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
	if (prevsize == 0) {
		/* Last chunk of user-defined "STRUCT_dlheapregion" (have have) just got free'd! */
		STRUCT_dlheapregion *region;
#define REGION_OVERHEAD (offsetof(STRUCT_dlheapregion, hr_first) + sizeof(STRUCT_dlheaptail))
		region = COMPILER_CONTAINER_OF((STRUCT_dlheapchunk *)p, STRUCT_dlheapregion, hr_first);
		if (region->hr_size == (psize + REGION_OVERHEAD)) {
			ext_assert(region->hr_destroy);
			ext_assert(region->hr_first.hc_prevsize == 0);
			ext_assert(region->hr_first.hc_head == (psize | PINUSE_BIT));
			if (next->head != 0) {
				/* Free dynamically allocated debug info from `DeeDbgHeap_AddHeapRegion()',
				 * and remove "region" (or its debug info) from the global list of known,
				 * custom heap regions */
				region = dl_dbg_heapregion_dispose(region);
			}
			(*region->hr_destroy)(region);
		}
#undef REGION_OVERHEAD
	}
#else /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
	if (prevsize == 0 && next->head == 0) {
		/* Last chunk of user-defined "STRUCT_dlheapregion" just got free'd! */
		STRUCT_dlheapregion *region;
#define REGION_OVERHEAD (offsetof(STRUCT_dlheapregion, hr_first) + sizeof(STRUCT_dlheaptail))
		region = COMPILER_CONTAINER_OF((STRUCT_dlheapchunk *)p, STRUCT_dlheapregion, hr_first);
		ext_assert(region->hr_size == (psize + REGION_OVERHEAD));
		ext_assert(region->hr_destroy);
		ext_assert(region->hr_first.hc_prevsize == 0);
		ext_assert(region->hr_first.hc_head == (psize | PINUSE_BIT));
		(*region->hr_destroy)(region);
#undef REGION_OVERHEAD
	}
#endif /* !FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
}
#endif /* FLAG4_BIT_HEAP_REGION */

DL_API_IMPL(, void, dlfree, (void *mem)) {

	/*
	   Consolidate freed chunks with preceeding or succeeding bordering
	   free chunks, if they exist, and then place in a bin.  Intermixed
	   with special cases for top, dv, mmapped chunks, and usage errors.
	*/
	size_t psize;
	mchunkptr next;
	mchunkptr p;
#if FOOTERS && !GM_ONLY
	mstate fm;
#else /* FOOTERS && !GM_ONLY */
#define fm gm
#endif /* !FOOTERS || GM_ONLY */

	/* Ignore "NULL"-pointers */
#if dlfree_CHECKS_NULL
	if unlikely(!mem)
		return;
#endif /* dlfree_CHECKS_NULL */

	p = mem2chunk(mem);
	validate_footer(p);

	ext_assert__ok_inuse(p);
	psize = chunksize(p);
	next  = chunk_plus_offset(p, psize);

#if FLAG4_BIT_HEAP_REGION
	if unlikely(is_mmapped(p)) {
#if !DL_MUNMAP_ALWAYS_FAILS
		size_t prevsize;
#endif /* !DL_MUNMAP_ALWAYS_FAILS */
		if unlikely(flag4inuse(p)) {
#if DL_DEBUG_MEMSET_FREE && FLAG4_BIT_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE
#ifndef HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_WORD_T
#define HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_WORD_T size_t
#endif /* !HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_WORD_T */
			/* Special case: don't set the "DL_DEBUG_MEMSET_FREE_PATTERN" word
			 *               at one of the following offsets, if that
			 *               word is currently set to "0". */
			size_t usize = chunksize(p) - overhead_for(p);
			size_t i, n_words = usize / sizeof(HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_WORD_T);
			HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_WORD_T *iter = (HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_WORD_T *)mem;
			for (i = 0; i < n_words; ++i, ++iter) {
				if (HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_CANSET(i * sizeof(HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_WORD_T), iter)) {
					*iter = DL_DEBUG_MEMSET_FREE_PATTERN;
				} else {
					/* Skip this word */
				}
			}
#else /* DL_DEBUG_MEMSET_FREE && FLAG4_BIT_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE */
			dl_setfree_data(mem, chunksize(p) - overhead_for(p));
#endif /* !DL_DEBUG_MEMSET_FREE || !FLAG4_BIT_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE */
			free_flag4_mem(p);
			return;
		}
		dl_setfree_data(mem, chunksize(p) - overhead_for(p));
#if DL_MUNMAP_ALWAYS_FAILS
		return;
#else /* DL_MUNMAP_ALWAYS_FAILS */
#if FOOTERS && !GM_ONLY
		fm = get_mstate_for(p);
		ext_assert__ok_magic(fm);
#endif /* FOOTERS && !GM_ONLY */
#if USE_PENDING_FREE_LIST
		if (!TRY_PREACTION(fm))
			goto do_dl_freelist_append;
#define NEED_do_dl_freelist_append
#else /* USE_PENDING_FREE_LIST */
		PREACTION(fm);
#endif /* !USE_PENDING_FREE_LIST */
		check_inuse_chunk(fm, p);
		ext_assert__ok_address(fm, p);
		prevsize = p->prev_foot;
		psize += prevsize + MMAP_FOOT_PAD;
		if (DL_MUNMAP((byte_t *)p - prevsize, psize) == 0)
			mstate_footprint(fm) -= psize;
		goto postaction;
#endif /* !DL_MUNMAP_ALWAYS_FAILS */
	}
#endif /* FLAG4_BIT_HEAP_REGION */

	/* memset() the payload area to the debug pattern for FREE memory. */
	dl_setfree_data(mem, chunksize(p) - overhead_for(p));

#if FOOTERS && !GM_ONLY
	fm = get_mstate_for(p);
	ext_assert__ok_magic(fm);
#endif /* FOOTERS && !GM_ONLY */
#if USE_PENDING_FREE_LIST
	if (!TRY_PREACTION(fm)) {
#ifdef NEED_do_dl_freelist_append
#undef NEED_do_dl_freelist_append
do_dl_freelist_append:
#endif /* NEED_do_dl_freelist_append */
		dl_freelist_append(fm, mem);
		return;
	}
#else /* USE_PENDING_FREE_LIST */
	PREACTION(fm);
#endif /* !USE_PENDING_FREE_LIST */
#if FLAG4_BIT_HEAP_REGION
#if DL_DEBUG_INTERNAL
	if (!flag4inuse(p))
		check_inuse_chunk(fm, p);
#endif /* DL_DEBUG_INTERNAL */
#else /* FLAG4_BIT_HEAP_REGION */
	check_inuse_chunk(fm, p);
	ext_assert__ok_address(fm, p);
#endif /* !FLAG4_BIT_HEAP_REGION */

	if (!pinuse(p)) {
		mchunkptr prev;
		size_t prevsize = p->prev_foot;
#if !FLAG4_BIT_HEAP_REGION
		if unlikely(is_mmapped(p)) {
#if !DL_MUNMAP_ALWAYS_FAILS
			psize += prevsize + MMAP_FOOT_PAD;
			if (DL_MUNMAP((byte_t *)p - prevsize, psize) == 0)
				mstate_footprint(fm) -= psize;
#endif /* !DL_MUNMAP_ALWAYS_FAILS */
			goto postaction;
		}
#endif /* !FLAG4_BIT_HEAP_REGION */

		prev = chunk_minus_offset(p, prevsize);
		psize += prevsize;
		ext_assert__ok_address(fm, prev);
		/* consolidate backward */
		if (prev != mstate_dv(fm)) {
			unlink_chunk(fm, prev, prevsize);
		} else if ((next->head & INUSE_BITS) == INUSE_BITS) {
			mstate_dvsize(fm) = psize;
			set_free_with_pinuse(prev, psize, next);
			dl_setfree_word(p->prev_foot, size_t);
			dl_setfree_word(p->head, size_t);
			goto postaction;
		}
		dl_setfree_word(p->prev_foot, size_t);
		dl_setfree_word(p->head, size_t);
		p = prev;
	}

	ext_assert__ok_next(p, next);
	ext_assert__ok_pinuse(next);
	if (!cinuse(next)) { /* consolidate forward */
		if (next == mstate_top(fm)) {
			size_t tsize   = mstate_topsize(fm) += psize;
			mstate_top(fm) = p;
			p->head        = tsize | PINUSE_BIT;
			dl_setfree_word(next->prev_foot, size_t);
			dl_setfree_word(next->head, size_t);
			if (p == mstate_dv(fm)) {
				mstate_dv(fm)     = NULL;
				mstate_dvsize(fm) = 0;
			}
			if (should_trim(fm, tsize))
				sys_trim(ARG_mstate_X_(fm) 0);
			goto postaction;
		} else if (next == mstate_dv(fm)) {
			size_t dsize  = mstate_dvsize(fm) += psize;
			mstate_dv(fm) = p;
			dl_setfree_word(next->prev_foot, size_t);
			dl_setfree_word(next->head, size_t);
			set_size_and_pinuse_of_free_chunk(p, dsize);
			goto postaction;
		} else {
			size_t nsize = chunksize(next);
			psize += nsize;
			unlink_chunk(fm, next, nsize);
			dl_setfree_word(next->prev_foot, size_t);
			dl_setfree_word(next->head, size_t);
			set_size_and_pinuse_of_free_chunk(p, psize);
			if (p == mstate_dv(fm)) {
				mstate_dvsize(fm) = psize;
				goto postaction;
			}
		}
	} else
		set_free_with_pinuse(p, psize, next);

	if (is_small(psize)) {
		insert_small_chunk(fm, p, psize);
		check_free_chunk(fm, p);
	} else {
		tchunkptr tp = (tchunkptr)p;
		insert_large_chunk(fm, tp, psize);
		check_free_chunk(fm, p);
		if (--mstate_release_checks(fm) == 0)
			release_unused_segments(ARG_mstate_X(fm));
	}
postaction:
	POSTACTION(fm);
#undef fm
}

#ifdef HAVE_dl_freelist_do_reap
static ATTR_NOINLINE void
dl_freelist_do_reap_item(PARAM_mstate_m_ void *__restrict mem) {
	size_t psize;
	mchunkptr next;
	mchunkptr p = mem2chunk(mem);
#if FOOTERS && !GM_ONLY
	dl_assert(get_mstate_for(p) == m);
#endif /* FOOTERS && !GM_ONLY */

	/* BEGIN: Copy-paste from `dlfree()' above */
	check_inuse_chunk(m, p);
	ext_assert__ok_address(m, p);
	ext_assert__ok_inuse(p);
	psize = chunksize(p);
	next  = chunk_plus_offset(p, psize);
	if (!pinuse(p)) {
		mchunkptr prev;
		size_t prevsize = p->prev_foot;
		if unlikely(is_mmapped(p)) {
#if !DL_MUNMAP_ALWAYS_FAILS
			psize += prevsize + MMAP_FOOT_PAD;
			if (DL_MUNMAP((byte_t *)p - prevsize, psize) == 0)
				mstate_footprint(m) -= psize;
#endif /* !DL_MUNMAP_ALWAYS_FAILS */
			goto postaction;
		}
		prev = chunk_minus_offset(p, prevsize);
		psize += prevsize;
		ext_assert__ok_address(m, prev);
		/* consolidate backward */
		if (prev != mstate_dv(m)) {
			unlink_chunk(m, prev, prevsize);
		} else if ((next->head & INUSE_BITS) == INUSE_BITS) {
			mstate_dvsize(m) = psize;
			set_free_with_pinuse(prev, psize, next);
			dl_setfree_word(p->prev_foot, size_t);
			dl_setfree_word(p->head, size_t);
			goto postaction;
		}
		dl_setfree_word(p->prev_foot, size_t);
		dl_setfree_word(p->head, size_t);
		p = prev;
	}

	ext_assert__ok_next(p, next);
	ext_assert__ok_pinuse(next);
	if (!cinuse(next)) { /* consolidate forward */
		if (next == mstate_top(m)) {
			size_t tsize   = mstate_topsize(m) += psize;
			mstate_top(m) = p;
			p->head        = tsize | PINUSE_BIT;
			dl_setfree_word(next->prev_foot, size_t);
			dl_setfree_word(next->head, size_t);
			if (p == mstate_dv(m)) {
				mstate_dv(m)     = NULL;
				mstate_dvsize(m) = 0;
			}
			if (should_trim(m, tsize))
				sys_trim(ARG_mstate_m_ 0);
			goto postaction;
		} else if (next == mstate_dv(m)) {
			size_t dsize  = mstate_dvsize(m) += psize;
			mstate_dv(m) = p;
			dl_setfree_word(next->prev_foot, size_t);
			dl_setfree_word(next->head, size_t);
			set_size_and_pinuse_of_free_chunk(p, dsize);
			goto postaction;
		} else {
			size_t nsize = chunksize(next);
			psize += nsize;
			unlink_chunk(m, next, nsize);
			dl_setfree_word(next->prev_foot, size_t);
			dl_setfree_word(next->head, size_t);
			set_size_and_pinuse_of_free_chunk(p, psize);
			if (p == mstate_dv(m)) {
				mstate_dvsize(m) = psize;
				goto postaction;
			}
		}
	} else
		set_free_with_pinuse(p, psize, next);

	if (is_small(psize)) {
		insert_small_chunk(m, p, psize);
		check_free_chunk(m, p);
	} else {
		tchunkptr tp = (tchunkptr)p;
		insert_large_chunk(m, tp, psize);
		check_free_chunk(m, p);
		if (--mstate_release_checks(m) == 0)
			release_unused_segments(ARG_mstate_m);
	}
postaction:
	/* END: Copy-paste from `dlfree()' above */
	;
}

static ATTR_NOINLINE NONNULL((1)) void
dl_freelist_do_reap(PARAM_mstate_m_ struct freelist_entry *__restrict flist) {
	struct freelist_entry *next;
	do {
		next = flist->fle_link.sle_next;
		dl_setfree_word(flist->fle_link.sle_next, struct freelist_entry *);
		dl_freelist_do_reap_item(ARG_mstate_m_ flist);
	} while ((flist = next) != NULL);
}
#endif /* HAVE_dl_freelist_do_reap */

#if dlcalloc_SINGLE_PARAMETER
DL_API_IMPL(ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)), void *, dlcalloc, (size_t req)) {
	void *mem = dlmalloc(req);
	if (mem != NULL && calloc_must_clear(mem2chunk(mem))) {
		mchunkptr p = mem2chunk(mem);
		bzero(mem, chunksize(p) - overhead_for(p));
	}
	return mem;
}
#else /* dlcalloc_SINGLE_PARAMETER */
DL_API_IMPL(ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1, 2)), void *,
            dlcalloc, (size_t n_elements, size_t elem_size)) {
	void *mem;
	size_t req;
	if (OVERFLOW_UMUL(n_elements, elem_size, &req))
		req = (size_t)-1;
	mem = dlmalloc(req);
	if (mem != NULL && calloc_must_clear(mem2chunk(mem)))
		bzero(mem, req);
	return mem;
}
#endif /* !dlcalloc_SINGLE_PARAMETER */

#endif /* !ONLY_MSPACES */

/* ------------ Internal support for realloc, memalign, etc -------------- */

/* Try to realloc; only in-place unless can_move true */
#undef try_realloc_chunk
#if FLAG4_BIT_HEAP_REGION
static mchunkptr try_realloc_chunk(PARAM_mstate_m_ mchunkptr p, size_t nb)
#define try_realloc_chunk(m, p, nb, can_move) try_realloc_chunk(ARG_mstate_X_(m) p, nb)
#else /* FLAG4_BIT_HEAP_REGION */
static mchunkptr try_realloc_chunk(PARAM_mstate_m_ mchunkptr p, size_t nb, int can_move)
#define try_realloc_chunk(m, p, nb, can_move) try_realloc_chunk(ARG_mstate_X_(m) p, nb, can_move)
#endif /* !FLAG4_BIT_HEAP_REGION */
{
	mchunkptr newp = NULL;
	size_t oldsize = chunksize(p);
	mchunkptr next = chunk_plus_offset(p, oldsize);
#if DL_DEBUG_MEMSET_ALLOC
	size_t oldsize_usable = oldsize - overhead_for(p);
#endif /* DL_DEBUG_MEMSET_ALLOC */
	ext_assert__ok_inuse(p);
	ext_assert__ok_next(p, next);
	ext_assert__ok_address(m, p);
	ext_assert__ok_pinuse(next);
#if !FLAG4_BIT_HEAP_REGION
	if (is_mmapped(p)) {
		newp = mmap_resize(m, p, nb, can_move);
	} else
#endif /* !FLAG4_BIT_HEAP_REGION */
	if (oldsize >= nb) { /* already big enough */
		size_t rsize = oldsize - nb;
		if (rsize >= MIN_CHUNK_SIZE) { /* split off remainder */
			mchunkptr r = chunk_plus_offset(p, nb);
			dl_setfree_data((byte_t *)r + TWO_SIZE_T_SIZES, rsize - CHUNK_OVERHEAD);
			set_inuse(m, p, nb);
			set_inuse(m, r, rsize);
			dispose_chunk(ARG_mstate_m_ r, rsize);
		}
		newp = p;
	} else if (next == mstate_top(m)) { /* extend into top */
		if (oldsize + mstate_topsize(m) > nb) {
			size_t newsize    = oldsize + mstate_topsize(m);
			size_t newtopsize = newsize - nb;
			mchunkptr newtop  = chunk_plus_offset(p, nb);
			check_memset_use_after_free(m, next);
			set_inuse(m, p, nb);
			newtop->head      = newtopsize | PINUSE_BIT;
			mstate_top(m)     = newtop;
			mstate_topsize(m) = newtopsize;
			newp = p;
		}
	} else if (next == mstate_dv(m)) { /* extend into dv */
		size_t dvs = mstate_dvsize(m);
		if (oldsize + dvs >= nb) {
			size_t dsize = oldsize + dvs - nb;
			check_memset_use_after_free(m, next);
			if (dsize >= MIN_CHUNK_SIZE) {
				mchunkptr r = chunk_plus_offset(p, nb);
				mchunkptr n = chunk_plus_offset(r, dsize);
				set_inuse(m, p, nb);
				set_size_and_pinuse_of_free_chunk(r, dsize);
				clear_pinuse(n);
				mstate_dvsize(m) = dsize;
				mstate_dv(m)     = r;
			} else { /* exhaust dv */
				size_t newsize = oldsize + dvs;
				set_inuse(m, p, newsize);
				mstate_dvsize(m) = 0;
				mstate_dv(m)     = NULL;
			}
			newp = p;
		}
	} else if (!cinuse(next)) { /* extend into next free chunk */
		size_t nextsize = chunksize(next);
		if (oldsize + nextsize >= nb) {
			size_t rsize = oldsize + nextsize - nb;
			check_memset_use_after_free(m, next);
			unlink_chunk(m, next, nextsize);
			if (rsize < MIN_CHUNK_SIZE) {
				size_t newsize = oldsize + nextsize;
				set_inuse(m, p, newsize);
			} else {
				mchunkptr r = chunk_plus_offset(p, nb);
				set_inuse(m, p, nb);
				set_inuse(m, r, rsize);
				dispose_chunk(ARG_mstate_m_ r, rsize);
			}
			newp = p;
		}
	}
#if DL_DEBUG_MEMSET_ALLOC
	if (newp) {
		size_t newsize_usable = chunksize(newp) - overhead_for(newp);
		if (newsize_usable > oldsize_usable) {
			dl_setalloc_data((byte_t *)chunk2mem(newp) + oldsize_usable,
			                 newsize_usable - oldsize_usable);
		}
	}
#endif /* DL_DEBUG_MEMSET_ALLOC */
	return newp;
}

static void *internal_memalign(PARAM_mstate_m_ size_t alignment, size_t bytes) {
	void *mem = NULL;
	if (alignment < MIN_CHUNK_SIZE) /* must be at least a minimum chunk size */
		alignment = MIN_CHUNK_SIZE;
	if ((alignment & (alignment - SIZE_T_ONE)) != 0) { /* Ensure a power of 2 */
		size_t a = MALLOC_ALIGNMENT << 1;
		while (a < alignment)
			a <<= 1;
		alignment = a;
	}
	if (bytes >= MAX_REQUEST - alignment) {
#if !GM_ONLY
		(void)m;
#endif /* !GM_ONLY */
		MALLOC_FAILURE_ACTION;
	} else {
		size_t nb  = request2size(bytes);
		size_t req = nb + alignment + MIN_CHUNK_SIZE - CHUNK_OVERHEAD;
		mem = internal_malloc(m, req);
		if (mem != NULL) {
			mchunkptr p = mem2chunk(mem);
#if USE_PER_THREAD_MSTATE
			mstate fm = get_mstate_for(p);
#elif GM_ONLY
#define fm gm
#else /* ... */
#define fm m
#endif /* !... */
			/* Must re-acquire lock to relevant mstate. This shouldn't normally
			 * block, since our thread just had that lock a few moments ago when
			 * we were in `internal_malloc()'.
			 *
			 * Also: this part **must** happen while holding the lock, since we
			 *       can only modify our "p->head" with the lock, as another
			 *       thread may be looking at that field while consolidating
			 *       free heap chunks! (no unused parts here *CAN'T* be free'd
			 *       asynchronously) */
			PREACTION(fm);
			if ((((size_t)(mem)) & (alignment - 1)) != 0) { /* misaligned */
				/*
				   Find an aligned spot inside chunk.  Since we need to give
				   back leading space in a chunk of at least MIN_CHUNK_SIZE, if
				   the first calculation places us at a spot with less than
				   MIN_CHUNK_SIZE leader, we can move to the next aligned spot.
				   We've allocated enough total room so that this is always
				   possible.
				 */
				byte_t *br      = (byte_t *)mem2chunk((size_t)(((size_t)((byte_t *)mem + alignment - SIZE_T_ONE)) & ~(alignment - SIZE_T_ONE)));
				byte_t *pos     = ((size_t)(br - (byte_t *)(p)) >= MIN_CHUNK_SIZE) ? br : br + alignment;
				mchunkptr newp  = (mchunkptr)pos;
				size_t leadsize = pos - (byte_t *)(p);
				size_t newsize  = chunksize(p) - leadsize;

				if (is_mmapped(p)) { /* For mmapped chunks, just adjust offset */
					newp->prev_foot = p->prev_foot + leadsize;
					newp->head      = newsize;
				} else { /* Otherwise, give back leader, use the rest */
					set_inuse(fm, p, leadsize);
					set_inuse(fm, newp, newsize); /* OG dlmalloc had these in a weird order, where "newp->head" was left partially uninitialized... */
					dl_setfree_data_untested((byte_t *)p + TWO_SIZE_T_SIZES,
					                         leadsize - CHUNK_OVERHEAD);
					dispose_chunk(ARG_mstate_X_(fm) p, leadsize);
				}
				p = newp;
			}

			/* Give back spare room at the end */
			if (!is_mmapped(p)) {
				size_t size = chunksize(p);
				if (size > nb + MIN_CHUNK_SIZE) {
					size_t remainder_size = size - nb;
					mchunkptr remainder   = chunk_plus_offset(p, nb);
					set_inuse(fm, p, nb);
					set_inuse(fm, remainder, remainder_size);
					dl_setfree_data_untested((byte_t *)remainder + TWO_SIZE_T_SIZES,
					                         remainder_size - CHUNK_OVERHEAD);
					dispose_chunk(ARG_mstate_X_(fm) remainder, remainder_size);
				}
			}

			mem = chunk2mem(p);
			dl_assert(chunksize(p) >= nb);
			dl_assert(((size_t)mem & (alignment - 1)) == 0);
			check_inuse_chunk(fm, p);
			POSTACTION(fm);
#undef fm
		}
	}
	return mem;
}


#if !NO_INDEPENDENT_ALLOC
/*
  Common support for independent_X routines, handling
    all of the combinations that can result.
  The opts arg has:
    bit 0 set if all elements are same size (using sizes[0])
    bit 1 set if elements should be zeroed
*/
static void **ialloc(PARAM_mstate_m_
                     size_t n_elements,
                     size_t *sizes,
                     int opts,
                     void *chunks[]) {
	size_t element_size;   /* chunksize of each element, if all same */
	size_t contents_size;  /* total size of elements */
	size_t array_size;     /* request size of pointer array */
	void *mem;             /* malloced aggregate space */
	mchunkptr p;           /* corresponding chunk */
	size_t remainder_size; /* remaining bytes while splitting */
	void **marray;         /* either "chunks" or malloced ptr array */
	mchunkptr array_chunk; /* chunk for malloced ptr array */
	flag_t was_enabled;    /* to disable mmap */
	size_t size;
	size_t i;

	ensure_initialization();
	/* compute array length, if needed */
	if (chunks != NULL) {
		if (n_elements == 0)
			return chunks; /* nothing to do */
		marray     = chunks;
		array_size = 0;
	} else {
		/* if empty req, must still return chunk representing empty array */
		if (n_elements == 0)
			return (void **)internal_malloc(m, 0);
		marray     = NULL;
		array_size = request2size(n_elements * (sizeof(void *)));
	}

	/* compute total element size */
	if (opts & 0x1) { /* all-same-size */
		element_size  = request2size(*sizes);
		contents_size = n_elements * element_size;
	} else { /* add up all the sizes */
		element_size  = 0;
		contents_size = 0;
		for (i = 0; i != n_elements; ++i)
			contents_size += request2size(sizes[i]);
	}

	size = contents_size + array_size;

	/*
	   Allocate the aggregate chunk.  First disable direct-mmapping so
	   malloc won't use it, since we would not be able to later
	   free/realloc space internal to a segregated mmap region.
	 */
	was_enabled = use_mmap(m);
	disable_mmap(m);
	mem = internal_malloc(m, size - CHUNK_OVERHEAD);
	if (was_enabled)
		enable_mmap(m);
	if (mem == NULL)
		return NULL;

	PREACTION(m);
	p = mem2chunk(mem);
	remainder_size = chunksize(p);

	dl_assert(!is_mmapped(p));

	if (opts & 0x2) { /* optionally clear the elements */
		bzero(mem, remainder_size - SIZE_T_SIZE - array_size);
	}

	/* If not provided, allocate the pointer array as final part of chunk */
	if (marray == NULL) {
		size_t array_chunk_size;
		array_chunk      = chunk_plus_offset(p, contents_size);
		array_chunk_size = remainder_size - contents_size;
		marray           = (void **)(chunk2mem(array_chunk));
		set_size_and_pinuse_of_inuse_chunk(m, array_chunk, array_chunk_size);
		remainder_size = contents_size;
	}

	/* split out elements */
	for (i = 0;; ++i) {
		marray[i] = chunk2mem(p);
		if (i != n_elements - 1) {
			if (element_size != 0)
				size = element_size;
			else
				size = request2size(sizes[i]);
			remainder_size -= size;
			set_size_and_pinuse_of_inuse_chunk(m, p, size);
			p = chunk_plus_offset(p, size);
		} else { /* the final element absorbs any overallocation slop */
			set_size_and_pinuse_of_inuse_chunk(m, p, remainder_size);
			break;
		}
	}

#if DL_DEBUG_INTERNAL
	if (marray != chunks) {
		/* final element must have exactly exhausted chunk */
		if (element_size != 0) {
			dl_assert(remainder_size == element_size);
		} else {
			dl_assert(remainder_size == request2size(sizes[i]));
		}
		check_inuse_chunk(m, mem2chunk(marray));
	}
	for (i = 0; i != n_elements; ++i)
		check_inuse_chunk(m, mem2chunk(marray[i]));
#endif /* DL_DEBUG_INTERNAL */

	POSTACTION(m);
	return marray;
}
#endif /* !NO_INDEPENDENT_ALLOC */

#if !NO_BULK_FREE
/* Try to free all pointers in the given array.
   Note: this could be made faster, by delaying consolidation,
   at the price of disabling some user integrity checks, We
   still optimize some consolidations by combining adjacent
   chunks before freeing, which will occur often if allocated
   with ialloc or the array is sorted.
*/
static size_t internal_bulk_free(PARAM_mstate_m_ void *array[], size_t nelem) {
	void **a;
	void **fence;
	size_t unfreed = 0;
	PREACTION(m);
	fence = &(array[nelem]);
	for (a = array; a != fence; ++a) {
		void *mem = *a;
		if (mem != NULL) {
			void **b;
			mchunkptr next;
			mchunkptr p  = mem2chunk(mem);
			size_t psize = chunksize(p);
#if FOOTERS && !GM_ONLY
			if (get_mstate_for(p) != m) {
				++unfreed;
				continue;
			}
#endif /* FOOTERS && !GM_ONLY */
			check_inuse_chunk(m, p);
			*a = NULL;
			ext_assert__ok_address(m, p);
			ext_assert__ok_inuse(p);
			b = a + 1; /* try to merge with next chunk */
			next = next_chunk(p);
			if (b != fence && *b == chunk2mem(next)) {
				size_t newsize = chunksize(next) + psize;
				set_inuse(m, p, newsize);
				*b = chunk2mem(p);
			} else
				dispose_chunk(ARG_mstate_m_ p, psize);
		}
	}
	if (should_trim(m, mstate_topsize(m)))
		sys_trim(ARG_mstate_m_ 0);
	POSTACTION(m);
	return unfreed;
}
#endif /* !NO_BULK_FREE */

/* Traversal */
#if MALLOC_INSPECT_ALL
static void internal_inspect_all(PARAM_mstate_m_
                                 void (*handler)(void *start,
                                                 void *end,
                                                 size_t used_bytes,
                                                 void *callback_arg),
                                 void *arg) {
	if (is_initialized(m)) {
		mchunkptr top = mstate_top(m);
		msegmentptr s;
		for (s = &mstate_seg(m); s != NULL; s = s->next) {
			mchunkptr q = align_as_chunk(s->base);
			while (segment_holds(s, q) && q->head != FENCEPOST_HEAD) {
				mchunkptr next = next_chunk(q);
				size_t sz      = chunksize(q);
				size_t used;
				void *start;
				if (is_inuse(q)) {
					used  = sz - CHUNK_OVERHEAD; /* must not be mmapped */
					start = chunk2mem(q);
				} else {
					used = 0;
					if (is_small(sz)) { /* offset by possible bookkeeping */
						start = (void *)((byte_t *)q + sizeof(struct malloc_chunk));
					} else {
						start = (void *)((byte_t *)q + sizeof(struct malloc_tree_chunk));
					}
				}
				if (start < (void *)next) /* skip if all space is bookkeeping */
					handler(start, next, used, arg);
				if (q == top)
					break;
				q = next;
			}
		}
	}
}
#endif /* MALLOC_INSPECT_ALL */

/* ------------------ Exported realloc, memalign, etc -------------------- */

#if !ONLY_MSPACES

DL_API_IMPL(WUNUSED, void *, dlrealloc, (void *oldmem, size_t bytes)) {
	void *mem = NULL;
#if dlrealloc_CHECKS_NULL
	if (oldmem == NULL) {
#ifdef HOOK_AFTER_INIT_REALLOC
		ensure_initialization_for(HOOK_AFTER_INIT_REALLOC(oldmem, bytes));
#endif /* HOOK_AFTER_INIT_REALLOC */
		mem = dlmalloc(bytes);
	} else
#endif /* dlrealloc_CHECKS_NULL */
	if (bytes >= MAX_REQUEST) {
		MALLOC_FAILURE_ACTION;
	} else
#if dlrealloc_ZERO_BYTES_FREES
	if (bytes == 0) {
		dlfree(oldmem);
	} else
#endif /* dlrealloc_ZERO_BYTES_FREES */
	{
		size_t nb      = request2size(bytes);
		mchunkptr oldp = mem2chunk(oldmem);
		mchunkptr newp;
#if FOOTERS && !GM_ONLY
		mstate fm;
#else /* FOOTERS && !GM_ONLY */
#define fm gm
#endif /* !FOOTERS || GM_ONLY */
		validate_footer(oldp);
#if FLAG4_BIT_HEAP_REGION
		if unlikely(is_mmapped(oldp)) {
			/* Don't realloc pointers from user-defined heap region */
			if unlikely(flag4inuse(oldp)) {
#if FOOTERS && !GM_ONLY
				fm = gm;
#endif /* FOOTERS && !GM_ONLY */
				goto fallback_internal_malloc;
			}
#if FOOTERS && !GM_ONLY
			fm = get_mstate_for(oldp);
			ext_assert__ok_magic(fm);
#endif /* FOOTERS && !GM_ONLY */
#if USE_PER_THREAD_MSTATE
			if (!TRY_PREACTION(fm)) {
				mspace mtls;
				if (fm != gm) {
do_dlmalloc_fallback:
					mem = dlmalloc(bytes);
					goto after_internal_malloc;
				}
				mtls = tls_mspace();
				if (mtls) {
#if USE_MSPACE_MALLOC_LOCKLESS
					if (!TRY_PREACTION((mstate)mtls))
						goto do_dlmalloc_fallback;
					mem = mspace_malloc_lockless(mtls, bytes);
					POSTACTION((mstate)mtls);
#else /* USE_MSPACE_MALLOC_LOCKLESS */
					mem = mspace_malloc(mtls, bytes);
#endif /* !USE_MSPACE_MALLOC_LOCKLESS */
					goto after_internal_malloc;
				}
				PREACTION(fm);
			}
#else /* USE_PER_THREAD_MSTATE */
			PREACTION(fm);
#endif /* !USE_PER_THREAD_MSTATE */
			newp = mmap_resize(fm, oldp, nb, 1);
		} else
#endif /* FLAG4_BIT_HEAP_REGION */
		{
#if FOOTERS && !GM_ONLY
			fm = get_mstate_for(oldp);
			ext_assert__ok_magic(fm);
#endif /* FOOTERS && !GM_ONLY */
#if USE_PER_THREAD_MSTATE
			if (!TRY_PREACTION(fm)) {
				mspace mtls;
				if (fm != gm)
					goto do_dlmalloc_fallback;
				mtls = tls_mspace();
				if (mtls) {
#if USE_MSPACE_MALLOC_LOCKLESS
					if (!TRY_PREACTION((mstate)mtls))
						goto do_dlmalloc_fallback;
					mem = mspace_malloc_lockless(mtls, bytes);
					POSTACTION((mstate)mtls);
#else /* USE_MSPACE_MALLOC_LOCKLESS */
					mem = mspace_malloc(mtls, bytes);
#endif /* !USE_MSPACE_MALLOC_LOCKLESS */
					goto after_internal_malloc;
				}
				PREACTION(fm);
			}
#else /* USE_PER_THREAD_MSTATE */
			PREACTION(fm);
#endif /* !USE_PER_THREAD_MSTATE */
			newp = try_realloc_chunk(fm, oldp, nb, 1);
		}
		POSTACTION(fm);
		if (newp != NULL) {
			check_inuse_chunk(fm, newp);
			mem = chunk2mem(newp);
		} else {
#if FLAG4_BIT_HEAP_REGION
fallback_internal_malloc:
#endif /* FLAG4_BIT_HEAP_REGION */
			mem = internal_malloc(fm, bytes);
#if USE_PER_THREAD_MSTATE
after_internal_malloc:
#endif /* USE_PER_THREAD_MSTATE */
			if (mem != NULL) {
				size_t oc = chunksize(oldp) - overhead_for(oldp);
				mem = memcpy(mem, oldmem, (oc < bytes) ? oc : bytes);
				internal_free(fm, oldmem);
			}
		}
#undef fm
	}
	return mem;
}

DL_API_IMPL(WUNUSED, void *, dlrealloc_in_place, (void *oldmem, size_t bytes)) {
	void *mem = NULL;
#ifndef DLREALLOC_IN_PLACE_CHECKS_NULL
#define DLREALLOC_IN_PLACE_CHECKS_NULL 1
#endif /* !DLREALLOC_IN_PLACE_CHECKS_NULL */
#if DLREALLOC_IN_PLACE_CHECKS_NULL
	if (oldmem == NULL) {
		/* ... */
	} else
#endif /* DLREALLOC_IN_PLACE_CHECKS_NULL */
	if (bytes >= MAX_REQUEST) {
		MALLOC_FAILURE_ACTION;
	} else {
		size_t nb      = request2size(bytes);
		mchunkptr oldp = mem2chunk(oldmem);
		mchunkptr newp;
#if FOOTERS && !GM_ONLY
		mstate fm;
#else /* FOOTERS && !GM_ONLY */
#define fm gm
#endif /* !FOOTERS || GM_ONLY */
#if FLAG4_BIT_HEAP_REGION
		if unlikely(is_mmapped(oldp)) {
			/* Don't realloc pointers from user-defined heap region */
			if unlikely(flag4inuse(oldp))
				return NULL;
#if FOOTERS && !GM_ONLY
			fm = get_mstate_for(oldp);
			ext_assert__ok_magic(fm);
#endif /* FOOTERS && !GM_ONLY */
			PREACTION(fm);
			newp = mmap_resize(fm, oldp, nb, 0);
		} else
#endif /* FLAG4_BIT_HEAP_REGION */
		{
#if FOOTERS && !GM_ONLY
			fm = get_mstate_for(oldp);
			ext_assert__ok_magic(fm);
#endif /* FOOTERS && !GM_ONLY */
			PREACTION(fm);
			newp = try_realloc_chunk(fm, oldp, nb, 0);
		}
		POSTACTION(fm);
		dl_assert(newp == oldp || newp == NULL);
		if (newp == oldp) {
			check_inuse_chunk(fm, newp);
			mem = oldmem;
		}
#undef fm
	}
	return mem;
}

DL_API_IMPL(ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)),
            void *, dlmemalign, (size_t alignment, size_t bytes)){
#ifdef HOOK_AFTER_INIT_MEMALIGN
	ensure_initialization_for(HOOK_AFTER_INIT_MEMALIGN(alignment, bytes));
#endif /* HOOK_AFTER_INIT_MEMALIGN */
	if (alignment <= MALLOC_ALIGNMENT)
		return dlmalloc(bytes);
	return internal_memalign(ARG_mstate_gm_ alignment, bytes);
}

#if !NO_POSIX_MEMALIGN
DL_API_IMPL(, int, dlposix_memalign, (void **pp, size_t alignment, size_t bytes)) {
	void *mem = NULL;
#ifdef HOOK_AFTER_INIT_POSIX_MEMALIGN
	ensure_initialization_for(HOOK_AFTER_INIT_POSIX_MEMALIGN(pp, alignment, bytes));
#endif /* HOOK_AFTER_INIT_POSIX_MEMALIGN */
	if (alignment == MALLOC_ALIGNMENT)
		mem = dlmalloc(bytes);
	else {
		size_t d = alignment / sizeof(void *);
		size_t r = alignment % sizeof(void *);
		if (r != 0 || d == 0 || (d & (d - SIZE_T_ONE)) != 0)
			return EINVAL;
		else if (bytes <= MAX_REQUEST - alignment) {
			if (alignment < MIN_CHUNK_SIZE)
				alignment = MIN_CHUNK_SIZE;
			mem = internal_memalign(ARG_mstate_gm_ alignment, bytes);
		}
	}
	if (mem == NULL)
		return ENOMEM;
	else {
		*pp = mem;
		return 0;
	}
}
#endif /* !NO_POSIX_MEMALIGN */

#if !NO_VALLOC
DL_API_IMPL(, void *, dlvalloc, (size_t bytes)) {
	size_t pagesz;
#ifdef HOOK_AFTER_INIT_VALLOC
	ensure_initialization_for(HOOK_AFTER_INIT_VALLOC(bytes));
#else /* HOOK_AFTER_INIT_VALLOC */
#if !GM_STATIC_INIT_MUTEX
	ensure_initialization();
#endif /* !GM_STATIC_INIT_MUTEX */
#endif /* !HOOK_AFTER_INIT_VALLOC */
	pagesz = malloc_pagesize;
	return dlmemalign(pagesz, bytes);
}
#endif /* !NO_VALLOC */

#if !NO_PVALLOC
DL_API_IMPL(, void *, dlpvalloc, (size_t bytes)) {
	size_t pagesz;
#ifdef HOOK_AFTER_INIT_PVALLOC
	ensure_initialization_for(HOOK_AFTER_INIT_PVALLOC(bytes));
#else /* HOOK_AFTER_INIT_PVALLOC */
#if !GM_STATIC_INIT_MUTEX
	ensure_initialization();
#endif /* !GM_STATIC_INIT_MUTEX */
#endif /* !HOOK_AFTER_INIT_PVALLOC */
	pagesz = malloc_pagesize;
	return dlmemalign(pagesz, (bytes + pagesz - SIZE_T_ONE) & ~(pagesz - SIZE_T_ONE));
}
#endif /* !NO_PVALLOC */

#if !NO_INDEPENDENT_ALLOC
DL_API_IMPL(, void **, dlindependent_calloc, (size_t n_elements, size_t elem_size, void *chunks[])) {
	size_t sz = elem_size; /* serves as 1-element array */
	return ialloc(ARG_mstate_gm_ n_elements, &sz, 3, chunks);
}

DL_API_IMPL(, void **, dlindependent_comalloc, (size_t n_elements, size_t sizes[], void *chunks[])) {
	return ialloc(ARG_mstate_gm_ n_elements, sizes, 0, chunks);
}
#endif /* !NO_INDEPENDENT_ALLOC */

#if !NO_BULK_FREE
DL_API_IMPL(, size_t, dlbulk_free, (void *array[], size_t nelem)) {
	return internal_bulk_free(ARG_mstate_gm_ array, nelem);
}
#endif /* !NO_BULK_FREE */

#if MALLOC_INSPECT_ALL
DL_API_IMPL(, void, dlmalloc_inspect_all, (void (*handler)(void *start, void *end,
                                                           size_t used_bytes,
                                                           void *callback_arg),
                                           void *arg)) {
#if !GM_STATIC_INIT_MUTEX
	ensure_initialization();
#endif /* !GM_STATIC_INIT_MUTEX */
	PREACTION(gm);
	internal_inspect_all(ARG_mstate_gm_ handler, arg);
	POSTACTION(gm);
}
#endif /* MALLOC_INSPECT_ALL */

#if !NO_MALLOC_TRIM
static dlmalloc_trim_RETURN_TYPE do_mspace_sys_trim(PARAM_mstate_m_ size_t pad) {
	dlmalloc_trim_RETURN_TYPE result;
	PREACTION(m);
	result = sys_trim(ARG_mstate_m_ pad);
	POSTACTION(m);
	return result;
}

#if USE_PER_THREAD_MSTATE
static size_t do_mspace_sys_trim_cb(mspace ms, void *arg) {
	return do_mspace_sys_trim(ARG_mstate_X_((mstate)ms) (size_t)(uintptr_t)arg);
}
#endif /* USE_PER_THREAD_MSTATE */

DL_API_IMPL(, dlmalloc_trim_RETURN_TYPE, dlmalloc_trim, (size_t pad)) {
	dlmalloc_trim_RETURN_TYPE result = 0;
#ifdef HOOK_AFTER_INIT_MALLOC_TRIM
	ensure_initialization_for(HOOK_AFTER_INIT_MALLOC_TRIM(pad));
#else /* HOOK_AFTER_INIT_MALLOC_TRIM */
#if !GM_STATIC_INIT_MUTEX
	ensure_initialization();
#endif /* !GM_STATIC_INIT_MUTEX */
#endif /* !HOOK_AFTER_INIT_MALLOC_TRIM */

	/* Invoke custom trim pre-hook */
	dlmalloc_trim_PREHOOK(&result, pad);

	/* Trim the main (global) heap */
	result += do_mspace_sys_trim(ARG_mstate_gm_ pad);

	/* Also trim every "tls_mspace()" (add sum from that to result) */
#if USE_PER_THREAD_MSTATE
	result += (dlmalloc_trim_RETURN_TYPE)tls_mspace_foreach(&do_mspace_sys_trim_cb, (void *)(uintptr_t)pad);
#endif /* USE_PER_THREAD_MSTATE */

	/* Invoke custom trim post-hook */
	dlmalloc_trim_POSTHOOK(&result, pad);

	return result;
}
#endif /* !NO_MALLOC_TRIM */

#if !NO_MALLOC_FOOTPRINT
#if USE_PER_THREAD_MSTATE
static size_t do_mspace_footprint_cb(mspace ms, void *arg) {
	(void)arg;
	return ATOMIC_READ(&mstate_footprint((mstate)ms));
}

static size_t do_mspace_max_footprint_cb(mspace ms, void *arg) {
	(void)arg;
	return ATOMIC_READ(&mstate_max_footprint((mstate)ms));
}

static size_t do_mspace_set_footprint_limit_cb(mspace ms, void *arg) {
	size_t limit = (size_t)(uintptr_t)arg;
	ATOMIC_WRITE(&mstate_footprint_limit((mstate)ms), limit);
	return 0;
}
#endif /* USE_PER_THREAD_MSTATE */

DL_API_IMPL(ATTR_PURE WUNUSED, size_t, dlmalloc_footprint, (void)) {
	size_t result = ATOMIC_READ(&gm__footprint);
	/* Include info from "tls_mspace()" (add sum from tls) */
#if USE_PER_THREAD_MSTATE
	result += tls_mspace_foreach(&do_mspace_footprint_cb, NULL);
#endif /* USE_PER_THREAD_MSTATE */
	return result;
}

DL_API_IMPL(ATTR_PURE WUNUSED, size_t, dlmalloc_max_footprint, (void)) {
	size_t result = ATOMIC_READ(&gm__max_footprint);
	/* Include info from "tls_mspace()" (add sum from tls) */
#if USE_PER_THREAD_MSTATE
	result += tls_mspace_foreach(&do_mspace_max_footprint_cb, NULL);
#endif /* USE_PER_THREAD_MSTATE */
	return result;
}

DL_API_IMPL(ATTR_PURE WUNUSED, size_t, dlmalloc_footprint_limit, (void)) {
	size_t maf = gm__footprint_limit;
	return maf == 0 ? SIZE_MAX : maf;
}

DL_API_IMPL(, size_t, dlmalloc_set_footprint_limit, (size_t bytes)) {
	size_t result;
	size_t newval; /* invert sense of 0 */
	if (bytes == 0)
		newval = granularity_align(1); /* Use minimal size */
	if (bytes == SIZE_MAX)
		newval = 0; /* disable */
	else
		newval = granularity_align(bytes);
#if dlmalloc_set_footprint_limit_RETURNS_OLD_VALUE
	result = ATOMIC_XCH(&gm__footprint_limit, newval);
#else /* dlmalloc_set_footprint_limit_RETURNS_OLD_VALUE */
	ATOMIC_WRITE(&gm__footprint_limit, newval);
	result = newval;
#endif /* !dlmalloc_set_footprint_limit_RETURNS_OLD_VALUE */

	/* Also set value in every "tls_mspace()" (set the same value everywhere) */
#if USE_PER_THREAD_MSTATE
	tls_mspace_foreach(&do_mspace_set_footprint_limit_cb, (void *)(uintptr_t)newval);
#endif /* USE_PER_THREAD_MSTATE */

	return result;
}
#endif /* !NO_MALLOC_FOOTPRINT */

#if !NO_MALLOC_STATS
DL_API_IMPL(, void, dlmalloc_stats, (void) {
	internal_malloc_stats(ARG_mstate_gm);
}
#endif /* NO_MALLOC_STATS */
#endif /* !ONLY_MSPACES */

/* ----------------------------- user mspaces ---------------------------- */

#if MSPACES

static mstate init_user_mstate(byte_t *tbase, size_t tsize) {
	size_t msize;
	mchunkptr mn;
	mchunkptr msp;
	mstate m;
	dl_setfree_data(tbase, tsize);
	msize = pad_request(sizeof(struct malloc_state));
	msp = align_as_chunk(tbase);
	m = (mstate)(chunk2mem(msp));
	bzero(m, msize);
	msp->head = (msize | INUSE_BITS);
	mstate_seg(m).base = mstate_least_addr(m) = tbase;
	mstate_seg(m).size = mstate_footprint(m) = mstate_max_footprint(m) = tsize;
	mstate_magic(m) = mparams.magic;
	mstate_release_checks(m) = MAX_RELEASE_CHECK_RATE;
	m->mflags = mparams.default_mflags;
	DL_LOCK_INIT(&mstate_mutex(m));
#ifdef mstate_extp
	mstate_extp(m) = NULL;
#endif /* mstate_extp */
#ifdef mstate_exts
	mstate_exts(m) = 0;
#endif /* mstate_exts */
	disable_contiguous(m);
	init_bins(m);
	mn = next_chunk(mem2chunk(m));
	init_top(m, mn, (size_t)((tbase + tsize) - (byte_t *)mn) - TOP_FOOT_SIZE);
	check_top_chunk(m, mstate_top(m));
	return m;
}

DL_API_IMPL(, mspace, create_mspace, (size_t capacity/*, int locked*/)) {
	mstate m = 0;
	size_t msize;
	ensure_initialization();
	msize = pad_request(sizeof(struct malloc_state));
	if (capacity < (size_t)0 - (size_t)(msize + TOP_FOOT_SIZE + malloc_pagesize)) {
		size_t rs    = ((capacity == 0) ? mparams.granularity : (capacity + TOP_FOOT_SIZE + msize));
		size_t tsize = granularity_align(rs);
		byte_t *tbase  = (byte_t *)(DL_MMAP(tsize));
		if (tbase != CMFAIL) {
			m = init_user_mstate(tbase, tsize);
			mstate_seg(m).sflags = USE_MMAP_BIT;
		}
	}
	return (mspace)m;
}

#if !NO_CREATE_MSPACE_WITH_BASE
DL_API_IMPL(, mspace, create_mspace_with_base, (void *base, size_t capacity/*, int locked*/)) {
	mstate m = 0;
	size_t msize;
	ensure_initialization();
	msize = pad_request(sizeof(struct malloc_state));
	if (capacity > msize + TOP_FOOT_SIZE &&
	    capacity < (size_t)0 - (size_t)(msize + TOP_FOOT_SIZE + malloc_pagesize)) {
		m = init_user_mstate((byte_t *)base, capacity);
		mstate_seg(m).sflags = EXTERN_BIT;
	}
	return (mspace)m;
}
#endif /* !NO_CREATE_MSPACE_WITH_BASE */

#if !NO_MSPACE_TRACK_LARGE_CHUNKS
DL_API_IMPL(, int, mspace_track_large_chunks, (mspace msp, int enable)) {
	int ret   = 0;
	mstate ms = (mstate)msp;
	PREACTION(ms);
	if (!use_mmap(ms)) {
		ret = 1;
	}
	if (!enable) {
		enable_mmap(ms);
	} else {
		disable_mmap(ms);
	}
	POSTACTION(ms);
	return ret;
}
#endif /* !NO_MSPACE_TRACK_LARGE_CHUNKS */

#if !NO_DESTROY_MSPACE
DL_API_IMPL(, size_t, destroy_mspace, (mspace msp)) {
	size_t freed = 0;
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
#if !DL_MUNMAP_ALWAYS_FAILS
	msegmentptr sp = &ms->seg;
	while (sp != NULL) {
		byte_t *base = sp->base;
		size_t size  = sp->size;
		flag_t flag  = sp->sflags;
		(void)base; /* placate people compiling -Wunused-variable */
		sp = sp->next;
#if EXTERN_BIT
		if ((flag & USE_MMAP_BIT) && !(flag & EXTERN_BIT))
#else /* EXTERN_BIT */
		if ((flag & USE_MMAP_BIT))
#endif /* !EXTERN_BIT */
		{
			if (DL_MUNMAP(base, size) == 0)
				freed += size;
		}
	}
#endif /* !DL_MUNMAP_ALWAYS_FAILS */
	return freed;
}
#endif /* NO_DESTROY_MSPACE */


#if USE_PER_THREAD_MSTATE
SLIST_HEAD(malloc_state_slist, malloc_state);
static struct malloc_state_slist free_tls_mspace = SLIST_HEAD_INITIALIZER(free_tls_mspace);
static struct malloc_state_slist used_tls_mspace = SLIST_HEAD_INITIALIZER(used_tls_mspace);
static DL_LOCK_T tls_mspace_lock = DL_LOCK_INIT_STATIC;
#define tls_mspace_lock_tryacquire() DL_LOCK_TRYACQUIRE(&tls_mspace_lock)
#define tls_mspace_lock_acquire()    DL_LOCK_ACQUIRE(&tls_mspace_lock)
#define tls_mspace_lock_waitfor()    DL_LOCK_WAITFOR(&tls_mspace_lock)
#define tls_mspace_lock_release()    DL_LOCK_RELEASE(&tls_mspace_lock)

static WUNUSED mspace DCALL create_tls_mspace(void) {
	struct malloc_state *result;
	tls_mspace_lock_acquire();
	if (!SLIST_EMPTY(&free_tls_mspace)) {
		result = SLIST_FIRST(&free_tls_mspace);
		SLIST_REMOVE_HEAD(&free_tls_mspace, ms_link);
	} else {
		result = (struct malloc_state *)create_mspace(0);
	}
	if likely(result) {
		SLIST_INSERT(&used_tls_mspace, result, ms_link);

		/* Copy runtime-configurable settings from "gm" */
		result->footprint_limit = ATOMIC_READ(&gm__footprint_limit);
	}
	tls_mspace_lock_release();
	return (mspace)result;
}

DL_API_IMPL(NONNULL((1)), void, tls_mspace_destroy, (mspace ms)) {
	struct malloc_state *state = (struct malloc_state *)ms;
	/* Since the mstate is about to go on ice, trim whatever amount of memory we can from it. */
	PREACTION(state);
	sys_trim(state, 0);
	POSTACTION(state);

	tls_mspace_lock_acquire();
	SLIST_REMOVE(&used_tls_mspace, state, malloc_state, ms_link);
	SLIST_INSERT(&free_tls_mspace, state, ms_link);
	tls_mspace_lock_release();
}

#if !defined(DL_TLS_GETHEAP) || !defined(DL_TLS_SETHEAP)
#error "Must #define DL_TLS_GETHEAP() and DL_TLS_SETHEAP() when 'USE_PER_THREAD_MSTATE=1'"
#endif /* !DL_TLS_GETHEAP || !DL_TLS_SETHEAP */

#ifndef DL_TLS_VARS
#define DL_TLS_VARS /* nothing */
#endif /* !DL_TLS_VARS */

/* Return the calling thread's thread-local mspace (or "0" if not available) */
DL_API_IMPL(, mspace, tls_mspace, (void)) {
	void *result;
	DL_TLS_VARS
	DL_TLS_GETHEAP(&result);
	if unlikely(!result) {
		result = create_tls_mspace();
		DL_TLS_SETHEAP(result);
	}
	return (mspace)result;
}

#ifndef DL_REENTRANT_LOCK_T
#define DL_REENTRANT_LOCK_T           DL_LOCK_T
#define DL_REENTRANT_LOCK_INIT_STATIC DL_LOCK_INIT_STATIC
#define DL_REENTRANT_LOCK_ACQUIRE     DL_LOCK_ACQUIRE
#define DL_REENTRANT_LOCK_RELEASE     DL_LOCK_RELEASE
#endif /* !DL_REENTRANT_LOCK_T */

/* Try to use a recursive lock here because the callback of `tls_mspace_foreach()'
 * may call back into `tls_mspace_foreach()' in some situations. */
static DL_REENTRANT_LOCK_T tls_foreach_lock = DL_REENTRANT_LOCK_INIT_STATIC;
#define tls_foreach_lock_acquire_noint() DL_REENTRANT_LOCK_ACQUIRE(&tls_foreach_lock)
#define tls_foreach_lock_release()       DL_REENTRANT_LOCK_RELEASE(&tls_foreach_lock)

static uintptr_t tls_foreach_bitset_inuse = 0;
static uintptr_t tls_foreach_bitset_inuse_alloc(void) {
	unsigned int i;
	uintptr_t result, state;
again:
	state = ATOMIC_READ(&tls_foreach_bitset_inuse);
	for (i = 0, result = 1; i < (sizeof(state) * __CHAR_BIT__); ++i, result <<= 1) {
		if (!(state & result)) {
			if (!(ATOMIC_FETCHOR(&tls_foreach_bitset_inuse, result) & result))
				return result;
			goto again;
		}
	}
	return 0;
}

static void tls_foreach_bitset_inuse_free(uintptr_t mask) {
	ATOMIC_AND(&tls_foreach_bitset_inuse, ~mask);
}

static void reset_not_visited_tls_mspace(uintptr_t mask) {
	mstate ms;
	SLIST_FOREACH (ms, &free_tls_mspace, ms_link)
		ms->ms_foreach_bitset &= ~mask;
	SLIST_FOREACH (ms, &used_tls_mspace, ms_link)
		ms->ms_foreach_bitset &= ~mask;
}

static mstate find_not_visited_tls_mspace(uintptr_t mask) {
	mstate ms;
	SLIST_FOREACH (ms, &free_tls_mspace, ms_link) {
		if (!(ms->ms_foreach_bitset & mask)) {
			ms->ms_foreach_bitset |= mask;
			return ms;
		}
	}
	SLIST_FOREACH (ms, &used_tls_mspace, ms_link) {
		if (!(ms->ms_foreach_bitset & mask)) {
			ms->ms_foreach_bitset |= mask;
			return ms;
		}
	}
	return NULL;
}

DL_API_IMPL(, size_t, tls_mspace_foreach, (size_t (*cb)(mspace ms, void *arg), void *arg)) {
	uintptr_t mask;
	mstate ms;
	size_t result = 0;
	tls_foreach_lock_acquire_noint();
	mask = tls_foreach_bitset_inuse_alloc();
	tls_mspace_lock_acquire();
	reset_not_visited_tls_mspace(mask);
	while ((ms = find_not_visited_tls_mspace(mask)) != NULL) {
		tls_mspace_lock_release();
		result += (*cb)((mspace)ms, arg);
		tls_mspace_lock_acquire();
	}
	tls_mspace_lock_release();
	tls_foreach_bitset_inuse_free(mask);
	tls_foreach_lock_release();
	return result;
}
#endif /* USE_PER_THREAD_MSTATE */


/*
  mspace versions of routines are near-clones of the global
  versions. This is not so nice but better than the alternatives.
*/

#if USE_MSPACE_MALLOC_LOCKLESS
DL_API_IMPL(, void *, mspace_malloc, (mspace msp, size_t bytes)) {
	void *result;
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
#if dlmalloc_ZERO_RETURNS_NULL
	if unlikely(!bytes)
		return NULL;
#endif /* dlmalloc_ZERO_RETURNS_NULL */
	PREACTION(ms);
	result = mspace_malloc_lockless(ms, bytes);
	POSTACTION(ms);
	return result;
}
DL_API_IMPL(, void *, mspace_malloc_lockless, (mspace msp, size_t bytes))
#else /* USE_MSPACE_MALLOC_LOCKLESS */
DL_API_IMPL(, void *, mspace_malloc, (mspace msp, size_t bytes))
#endif /* !USE_MSPACE_MALLOC_LOCKLESS */
{
	void *mem;
	size_t nb;
	mstate ms = (mstate)msp;
#if !USE_MSPACE_MALLOC_LOCKLESS
	ext_assert__ok_magic(ms);
#if dlmalloc_ZERO_RETURNS_NULL
	if unlikely(!bytes)
		return NULL;
#endif /* dlmalloc_ZERO_RETURNS_NULL */
	PREACTION(ms);
#endif /* !USE_MSPACE_MALLOC_LOCKLESS */
	if (bytes <= MAX_SMALL_REQUEST) {
		bindex_t idx;
		binmap_t smallbits;
		nb        = (bytes < MIN_REQUEST) ? MIN_CHUNK_SIZE : pad_request(bytes);
		idx       = small_index(nb);
		smallbits = ms->smallmap >> idx;

		if ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
			mchunkptr b, p;
			idx += ~smallbits & 1; /* Uses next bin if idx empty */
			b = smallbin_at(ms, idx);
			p = b->fd;
			dl_assert(chunksize(p) == small_index2size(idx));
			unlink_first_small_chunk(ms, b, p, idx);
			set_inuse_and_pinuse(ms, p, small_index2size(idx));
			mem = chunk2mem(p);
			check_malloced_chunk(ms, mem, nb);
			goto postaction;
		}

		else if (nb > ms->dvsize) {
			if (smallbits != 0) { /* Use chunk in next nonempty smallbin */
				mchunkptr b, p, r;
				size_t rsize;
				bindex_t i;
				binmap_t leftbits = (smallbits << idx) & left_bits(idx2bit(idx));
				binmap_t leastbit = least_bit(leftbits);
				compute_bit2idx(leastbit, i);
				b = smallbin_at(ms, i);
				p = b->fd;
				dl_assert(chunksize(p) == small_index2size(i));
				unlink_first_small_chunk(ms, b, p, i);
				rsize = small_index2size(i) - nb;
				/* Fit here cannot be remainderless if 4byte sizes */
#if SIZE_T_SIZE != 4
				if (rsize < MIN_CHUNK_SIZE)
					set_inuse_and_pinuse(ms, p, small_index2size(i));
				else
#endif /* SIZE_T_SIZE != 4 */
				{
					set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
					r = chunk_plus_offset(p, nb);
					set_size_and_pinuse_of_free_chunk(r, rsize);
					replace_dv(ms, r, rsize);
				}
				mem = chunk2mem(p);
				check_malloced_chunk(ms, mem, nb);
				goto postaction;
			}

			else if (ms->treemap != 0 && (mem = tmalloc_small(ms, nb)) != NULL) {
				check_malloced_chunk(ms, mem, nb);
				goto postaction;
			}
		}
	} else if (bytes >= MAX_REQUEST)
		nb = SIZE_MAX; /* Too big to allocate. Force failure (in sys alloc) */
	else {
		nb = pad_request(bytes);
		if (ms->treemap != 0 && (mem = tmalloc_large(ms, nb)) != NULL) {
			check_malloced_chunk(ms, mem, nb);
			goto postaction;
		}
	}

	if (nb <= ms->dvsize) {
		size_t rsize = ms->dvsize - nb;
		mchunkptr p  = ms->dv;
		if (rsize >= MIN_CHUNK_SIZE) { /* split dv */
			mchunkptr r = ms->dv = chunk_plus_offset(p, nb);
			ms->dvsize           = rsize;
			set_size_and_pinuse_of_free_chunk(r, rsize);
			set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
		} else { /* exhaust dv */
			size_t dvs = ms->dvsize;
			ms->dvsize = 0;
			ms->dv     = NULL;
			set_inuse_and_pinuse(ms, p, dvs);
		}
		mem = chunk2mem(p);
		check_malloced_chunk(ms, mem, nb);
		goto postaction;
	}

	else if (nb < ms->topsize) { /* Split top */
		size_t rsize = ms->topsize -= nb;
		mchunkptr p  = ms->top;
		mchunkptr r = ms->top = chunk_plus_offset(p, nb);
		r->head               = rsize | PINUSE_BIT;
		set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
		mem = chunk2mem(p);
		check_top_chunk(ms, ms->top);
		check_malloced_chunk(ms, mem, nb);
		goto postaction;
	}

	mem = sys_alloc(ms, nb);

postaction:
#if !USE_MSPACE_MALLOC_LOCKLESS
	POSTACTION(ms);
#endif /* !USE_MSPACE_MALLOC_LOCKLESS */
	return mem;
}

#if !NO_MSPACE_FREE
DL_API_IMPL(, void, mspace_free, (mspace msp, void *mem)) {
	size_t psize;
	mchunkptr next;
	mchunkptr p;
#if FOOTERS
	mstate fm;
#endif /* !FOOTERS */
	if unlikely(!mem)
		return;
	p = mem2chunk(mem);
#if FOOTERS
	fm = get_mstate_for(p);
	(void)msp; /* placate people compiling -Wunused */
#else /* FOOTERS */
	mstate fm = (mstate)msp;
#endif /* !FOOTERS */
	ext_assert__ok_magic(fm);
	PREACTION(fm);
	check_inuse_chunk(fm, p);
	ext_assert__ok_address(fm, p);
	ext_assert__ok_inuse(p);
	psize = chunksize(p);
	next  = chunk_plus_offset(p, psize);
	if (!pinuse(p)) {
		size_t prevsize = p->prev_foot;
		if (is_mmapped(p)) {
#if !DL_MUNMAP_ALWAYS_FAILS
			psize += prevsize + MMAP_FOOT_PAD;
			if (DL_MUNMAP((byte_t *)p - prevsize, psize) == 0)
				fm->footprint -= psize;
#endif /* !DL_MUNMAP_ALWAYS_FAILS */
			goto postaction;
		} else {
			mchunkptr prev = chunk_minus_offset(p, prevsize);
			psize += prevsize;
			p = prev;
			ext_assert__ok_address(fm, prev);
			/* consolidate backward */
			if (p != mstate_dv(fm)) {
				unlink_chunk(fm, p, prevsize);
			} else if ((next->head & INUSE_BITS) == INUSE_BITS) {
				mstate_dvsize(fm) = psize;
				set_free_with_pinuse(p, psize, next);
				goto postaction;
			}
		}
	}

	ext_assert__ok_next(p, next);
	ext_assert__ok_pinuse(next);
	if (!cinuse(next)) { /* consolidate forward */
		if (next == mstate_top(fm)) {
			size_t tsize   = mstate_topsize(fm) += psize;
			mstate_top(fm) = p;
			p->head        = tsize | PINUSE_BIT;
			if (p == mstate_dv(fm)) {
				mstate_dv(fm)     = NULL;
				mstate_dvsize(fm) = 0;
			}
			if (should_trim(fm, tsize))
				sys_trim(fm, 0);
			goto postaction;
		} else if (next == mstate_dv(fm)) {
			size_t dsize  = mstate_dvsize(fm) += psize;
			mstate_dv(fm) = p;
			set_size_and_pinuse_of_free_chunk(p, dsize);
			goto postaction;
		} else {
			size_t nsize = chunksize(next);
			psize += nsize;
			unlink_chunk(fm, next, nsize);
			set_size_and_pinuse_of_free_chunk(p, psize);
			if (p == mstate_dv(fm)) {
				mstate_dvsize(fm) = psize;
				goto postaction;
			}
		}
	} else
		set_free_with_pinuse(p, psize, next);

	if (is_small(psize)) {
		insert_small_chunk(fm, p, psize);
		check_free_chunk(fm, p);
	} else {
		tchunkptr tp = (tchunkptr)p;
		insert_large_chunk(fm, tp, psize);
		check_free_chunk(fm, p);
		if (--mstate_release_checks(fm) == 0)
			release_unused_segments(fm);
	}
postaction:
	POSTACTION(fm);
}
#endif /* !NO_MSPACE_FREE */

#if !NO_MSPACE_CALLOC
DL_API_IMPL(, void *, mspace_calloc, (mspace msp, size_t n_elements, size_t elem_size)) {
	void *mem;
	size_t req;
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	if (OVERFLOW_UMUL(n_elements, elem_size, &req))
		req = (size_t)-1;
	mem = internal_malloc(ms, req);
	if (mem != NULL && calloc_must_clear(mem2chunk(mem)))
		bzero(mem, req);
	return mem;
}
#endif /* !NO_MSPACE_CALLOC */

#if !NO_MSPACE_REALLOC
DL_API_IMPL(, void *, mspace_realloc, (mspace msp, void *oldmem, size_t bytes)) {
	void *mem = NULL;
	if (oldmem == NULL) {
		mem = mspace_malloc(msp, bytes);
	} else if (bytes >= MAX_REQUEST) {
		MALLOC_FAILURE_ACTION;
	}
#if dlrealloc_ZERO_BYTES_FREES
	else if (bytes == 0) {
		mspace_free(msp, oldmem);
	}
#endif /* dlrealloc_ZERO_BYTES_FREES */
	else {
		size_t nb      = request2size(bytes);
		mchunkptr oldp = mem2chunk(oldmem);
		mchunkptr newp;
#if !FOOTERS
		mstate m = (mstate)msp;
#else /* FOOTERS */
		mstate m = get_mstate_for(oldp);
		ext_assert__ok_magic(m);
#endif /* FOOTERS */
		PREACTION(m);
		newp = try_realloc_chunk(m, oldp, nb, 1);
		POSTACTION(m);
		if (newp != NULL) {
			check_inuse_chunk(m, newp);
			mem = chunk2mem(newp);
		} else {
			mem = mspace_malloc(m, bytes);
			if (mem != NULL) {
				size_t oc = chunksize(oldp) - overhead_for(oldp);
				mem = memcpy(mem, oldmem, (oc < bytes) ? oc : bytes);
				mspace_free(m, oldmem);
			}
		}
	}
	return mem;
}
#endif /* !NO_MSPACE_REALLOC */

#if !NO_MSPACE_REALLOC_IN_PLACE
DL_API_IMPL(, void *, mspace_realloc_in_place, (mspace msp, void *oldmem, size_t bytes)) {
	void *mem = NULL;
	if (oldmem != NULL) {
		if (bytes >= MAX_REQUEST) {
			MALLOC_FAILURE_ACTION;
		} else {
			size_t nb      = request2size(bytes);
			mchunkptr oldp = mem2chunk(oldmem);
			mchunkptr newp;
#if !FOOTERS
			mstate m = (mstate)msp;
#else /* FOOTERS */
			mstate m = get_mstate_for(oldp);
			(void)msp; /* placate people compiling -Wunused */
			ext_assert__ok_magic(m);
#endif /* FOOTERS */
			PREACTION(m);
			newp = try_realloc_chunk(m, oldp, nb, 0);
			POSTACTION(m);
			if (newp == oldp) {
				check_inuse_chunk(m, newp);
				mem = oldmem;
			}
		}
	}
	return mem;
}
#endif /* !NO_MSPACE_REALLOC_IN_PLACE */

#if !NO_MSPACE_MEMALIGN
DL_API_IMPL(, void *, mspace_memalign, (mspace msp, size_t alignment, size_t bytes)) {
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	if (alignment <= MALLOC_ALIGNMENT)
		return mspace_malloc(msp, bytes);
	return internal_memalign(ms, alignment, bytes);
}
#endif /* !NO_MSPACE_MEMALIGN */

#if !NO_INDEPENDENT_ALLOC
DL_API_IMPL(, void **, mspace_independent_calloc, (mspace msp, size_t n_elements,
                                                   size_t elem_size, void *chunks[])) {
	size_t sz = elem_size; /* serves as 1-element array */
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	return ialloc(ms, n_elements, &sz, 3, chunks);
}

DL_API_IMPL(, void **, mspace_independent_comalloc, (mspace msp, size_t n_elements,
                                                     size_t sizes[], void *chunks[])) {
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	return ialloc(ms, n_elements, sizes, 0, chunks);
}
#endif /* !NO_INDEPENDENT_ALLOC */

#if !NO_BULK_FREE
DL_API_IMPL(, size_t, mspace_bulk_free, (mspace msp, void *array[], size_t nelem)) {
	return internal_bulk_free((mstate)msp, array, nelem);
}
#endif /* !NO_BULK_FREE */

#if MALLOC_INSPECT_ALL
static void mspace_inspect_all(mspace msp,
                               void (*handler)(void *start,
                                               void *end,
                                               size_t used_bytes,
                                               void *callback_arg),
                               void *arg) {
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	PREACTION(ms);
	internal_inspect_all(ms, handler, arg);
	POSTACTION(ms);
}
#endif /* MALLOC_INSPECT_ALL */

#if !NO_MSPACE_TRIM
DL_API_IMPL(, dlmalloc_trim_RETURN_TYPE, mspace_trim, (mspace msp, size_t pad)) {
	dlmalloc_trim_RETURN_TYPE result;
	mstate ms  = (mstate)msp;
	ext_assert__ok_magic(ms);
	PREACTION(ms);
	result = sys_trim(ms, pad);
	POSTACTION(ms);
	return result;
}
#endif /* NO_MSPACE_TRIM */

#if !NO_MALLOC_STATS
DL_API_IMPL(, void, mspace_malloc_stats, (mspace msp)) {
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	internal_malloc_stats(ms);
}
#endif /* NO_MALLOC_STATS */

#if !NO_MSPACE_FOOTPRINT
DL_API_IMPL(, size_t, mspace_footprint, (mspace msp)) {
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	return ms->footprint;
}
#endif /* !NO_MSPACE_FOOTPRINT */

#if !NO_MSPACE_MAX_FOOTPRINT
DL_API_IMPL(, size_t, mspace_max_footprint, (mspace msp)) {
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	return ms->max_footprint;
}
#endif /* !NO_MSPACE_MAX_FOOTPRINT */

#if !NO_MSPACE_FOOTPRINT_LIMIT
DL_API_IMPL(, size_t, mspace_footprint_limit, (mspace msp)) {
	size_t maf;
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	maf = ms->footprint_limit;
	return (maf == 0) ? SIZE_MAX : maf;
}
#endif /* !NO_MSPACE_FOOTPRINT_LIMIT */

#if !NO_MSPACE_SET_FOOTPRINT_LIMIT
DL_API_IMPL(, size_t, mspace_set_footprint_limit, (mspace msp, size_t bytes)) {
	size_t result = 0;
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	if (bytes == 0)
		result = granularity_align(1); /* Use minimal size */
	if (bytes == SIZE_MAX)
		result = 0; /* disable */
	else
		result = granularity_align(bytes);
	ms->footprint_limit = result;
	return result;
}
#endif /* !NO_MSPACE_SET_FOOTPRINT_LIMIT */

#if !NO_MSPACE_MALLINFO
DL_API_IMPL(, STRUCT_mallinfo, mspace_mallinfo, (mspace msp)) {
	STRUCT_mallinfo nm;
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	bzero(&nm, sizeof(nm));
	do_gather_internal_mallinfo(ARG_mstate_X_(ms) &nm);
	return nm;
}
#endif /* !NO_MSPACE_MALLINFO */

#if !NO_MSPACE_USABLE_SIZE
DL_API_IMPL(, size_t, mspace_usable_size, (void const *mem)) {
	if (mem != NULL) {
		mchunkptr p = mem2chunk(mem);
		if (is_inuse(p))
			return chunksize(p) - overhead_for(p);
	}
	return 0;
}
#endif /* !NO_MSPACE_USABLE_SIZE */
#endif /* MSPACES */

DECL_END

#endif /* !GUARD_LIBDLMALLOC_DLMALLOC_C_INL */
