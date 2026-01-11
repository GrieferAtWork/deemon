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
#ifndef GUARD_DEEMON_RUNTIME_HEAP_C
#define GUARD_DEEMON_RUNTIME_HEAP_C 1

#include <deemon/api.h>
#include <deemon/alloc.h>
#include <deemon/heap.h>
#include <deemon/system-features.h>

#ifndef __INTELLISENSE__
#include "slab.c.inl"
#endif /* !__INTELLISENSE__ */

#ifdef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/thread.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>
#include <deemon/util/rlock.h>

#include <hybrid/align.h>
#include <hybrid/bit.h>
#include <hybrid/host.h>
#include <hybrid/limitcore.h>
#include <hybrid/overflow.h>
#include <hybrid/sequence/list.h>
#include <hybrid/typecore.h>

#include <stddef.h> /* uintptr_t */
#include <stdint.h> /* intX_t, uintX_t */

#undef Dee_TryMalloc
#undef Dee_TryCalloc
#undef Dee_TryRealloc
#undef Dee_TryReallocInPlace
#undef Dee_TryMemalign
#undef Dee_MallocUsableSize
#undef Dee_Free

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

/* Enable support for "CONFIG_HAVE_VirtualAlloc" on windows */
#undef CONFIG_HAVE_VirtualAlloc
#ifdef CONFIG_HOST_WINDOWS
#include <Windows.h>
#define CONFIG_HAVE_VirtualAlloc
#endif /* CONFIG_HOST_WINDOWS */

/* Always enable optimizations for this file... */
#ifdef _MSC_VER
#pragma optimize("ts", on)
#endif /* _MSC_VER */

DECL_BEGIN

/* Heavily modified version of dlmalloc (Doug Lea's malloc).
 *
 * dlmalloc is public domain, but I feel like it's just a
 * matter of respect to state this inspiration, and give
 * credit where credit is due ;)
 *
 *
 * The basis for this implementation is the already-modified version found in the KOS source tree here:
 * >> https://github.com/GrieferAtWork/KOSmk4/blob/2ec470169e5df4a9f0b28fd4ff0c105a4f731852/kos/src/libdlmalloc/dlmalloc.c
 *
 * That file in turn is based on the original dlmalloc:
 * >> v2.8.6 Wed Aug 29 06:57:58 2012  Doug Lea
 *
 * Heavy modifications have been made:
 * - DL_DEBUG_MEMSET_ALLOC / DL_DEBUG_MEMSET_FREE:
 *   Debug option to fill freshly malloc'd / free'd memory with patterns
 * - GM_ONLY: **only** provide a single, global mspace
 * - FLAG4_BIT_INDICATES_HEAP_REGION: define an ABI for serializable heap chunks
 * - Determine system implementation (VirtualAlloc / mmap / sbrk) based on CONFIG_HAVE_* macros
 * - Add support for a 4th system implementation: malloc(3)
 *   (iow: implement dlmalloc() using the system's native malloc(3))
 * - Split dlmalloc debug checks into INTERNAL and EXTERNAL
 * - Change dlcalloc (Dee_TryCalloc) to take only a single argument
 * - Assert in-use in dlmalloc_usable_size (Dee_MallocUsableSize)
 * - Use-after-free detection when re-used memory is allocated again
 * - USE_PENDING_FREE_LIST: Support for guarantied non-blocking dlfree()
 * - USE_PER_THREAD_MSTATE: Support for thread-local heaps
 */


/* Implementation Configuration */
#define ONLY_MSPACES            0
#define MALLOC_ALIGNMENT        (size_t)(__SIZEOF_POINTER__ * 2)
#define PROCEED_ON_ERROR        0
#define MALLOC_INSPECT_ALL      0
#if 1
#define MALLOC_FAILURE_ACTION   /* Nothing */
#else
#define MALLOC_FAILURE_ACTION   Dee_BREAKPOINT()
#endif

#ifdef CONFIG_NO_THREADS
#define USE_LOCKS             0
#else /* CONFIG_NO_THREADS */
#define USE_LOCKS             2
#define USE_PENDING_FREE_LIST 1
#if !defined(__OPTIMIZE_SIZE__) && 1
#define USE_PER_THREAD_MSTATE 1 /* Provide a per-thread "mstate" that is used when the global state is locked */
#endif /* !__OPTIMIZE_SIZE__ */
#endif /* !CONFIG_NO_THREADS */
#ifndef USE_PER_THREAD_MSTATE
#define USE_PER_THREAD_MSTATE 0
#endif /* !USE_PER_THREAD_MSTATE */

#define MSPACES  (USE_PER_THREAD_MSTATE)
#define GM_ONLY  (!USE_PER_THREAD_MSTATE)

/* Enable external debugging (footers, usage checks, debug-memset patterns, leak detector) */
#if !defined(NDEBUG) && 1
#define DL_DEBUG_EXTERNAL 1
#else
#define DL_DEBUG_EXTERNAL 0
#endif

/* Enable internal debugging (self-consistency assertions) */
#if !defined(NDEBUG) && 0
#define DL_DEBUG_INTERNAL 1
#else
#define DL_DEBUG_INTERNAL 0
#endif

/* CAUTION: Leak detection is a bit of a bottleneck:
 *
 * LEAK_DETECTION=0          time make computed-operators    real    0m4.482s
 * LEAK_DETECTION=1          time make computed-operators    real    0m27.545s  615% (left-leaning RBTREE)
 * LEAK_DETECTION=1          time make computed-operators    real    0m20.685s  462% (RBTREE)
 * LEAK_DETECTION_IN_TAIL=1  time make computed-operators    real    0m7.234s   161%
 *
 * LEAK_DETECTION=0          time deemon util/test.dee       real    0m0.881s   100%
 * LEAK_DETECTION=1          time deemon util/test.dee       real    0m1.207s   137% (left-leaning RBTREE)
 * LEAK_DETECTION=1          time deemon util/test.dee       real    0m1.013s   115% (RBTREE)
 * LEAK_DETECTION_IN_TAIL=1  time deemon util/test.dee       real    0m0.965s   110%
 *
 * As you can see, `LEAK_DETECTION_IN_TAIL=1' is ****MUCH**** faster than the old
 * method of using an out-of-band RBTREE to describe memory leaks. It is however
 * still noticeably slower (but not as painfully so) than LEAK_DETECTION=0.
 *
 * Inspecting of threads during heavy parallel computation (make computed-operators)
 * reveals that at any moment, about 20% of threads are in the "SLIST_ATOMIC_INSERT"
 * calls in `leak_insert_p()' and `DeeDbg_Free()' (iow: are failing to insert their
 * relevant descriptors into the appropriate async-list for the purpose of either
 * inserting, or removing a leak from said list).
 *
 * This observation actually makes, since the relevant lists:
 * - leaks_pending_insert
 * - leaks_pending_remove
 *
 * exist as global singletons, meaning that in situations where all 32 of my machine's
 * core try to insert elements at the same time, there is only a 1/32 chance for that
 * insert succeeding for any one of those threads.
 *
 * XXX: This could also be improved upon further by having multiple pending lists, and
 *      selecting one at random based on the address of the "leak_footer" that's being
 *      inserted into these lists (though that would require hard-coding the # of
 *      lists that should exist -- or maybe not: why not allocate the vector of lists
 *      dynamically once it is first needed, and then have it's element count match
 *      the next-power-of-2 of the # of CPU in the system?) */
#if 0
#define LEAK_DETECTION 0
#else
#define LEAK_DETECTION DL_DEBUG_EXTERNAL
#endif
#define DETECT_USE_AFTER_FREE  DL_DEBUG_EXTERNAL
#define INSECURE               (!DL_DEBUG_EXTERNAL)
#if 1 /* Use the new approach for "LEAK_DETECTION" that doesn't store data out-of-band */
#define LEAK_DETECTION_IN_TAIL LEAK_DETECTION
#else
#define LEAK_DETECTION_IN_TAIL 0
#endif
#undef REALLOC_ZERO_BYTES_FREES /* Nope: Dee_Realloc(p, 0) resizes to 0-sized block, but returns non-NULL */
#undef MALLOC_ZERO_RETURNS_NULL /* Nope: Dee_Malloc(0) returns 0-sized, but non-NULL, block */


 /* Always keep (unnecessary) footers disabled -- they don't really improve safety when on,
 * since their presence causes allocations to require an additional word of memory, there
 * is a chance that alignment requirements ceil this to enough memory such that buffer-
 * overruns don't end up being detected. */
#if USE_PER_THREAD_MSTATE
#define FOOTERS 1 /* Needed for `Dee_Free()' to detect source mspace! */
#elif LEAK_DETECTION && LEAK_DETECTION_IN_TAIL
#define FOOTERS 1 /* Needed for `DeeDbg_Malloc()' to store debug info in the tail of heap blocks */
#else
#define FOOTERS 0
#endif


 /* Custom dlmalloc extension:
 * FLAG4_BIT on an allocation means it's part of "struct Dee_heapregion" */
#define FLAG4_BIT_INDICATES_HEAP_REGION 1

/* Handler special flag: we need to be able to detect if a statically allocated
 * object from a heap region has been destroyed via its "ob_refcnt" in order to
 * safely handle (technically illegal) references to static, non-final, constant
 * objects within another module under "CONFIG_EXPERIMENTAL_MMAP_DEC". If during
 * object destruction, it is found that a "Dee_refcnt_t"-sized word at one of
 * the following offsets into a heap block's user-area is "0", then this "0"
 * must **NOT** be overwritten by "DL_DEBUG_MEMSET_FREE":
 * - offsetof(DeeObject, ob_refcnt)
 * - offsetof(struct Dee_gc_head, gc_object.ob_refcnt) */
#ifdef CONFIG_EXPERIMENTAL_MMAP_DEC
#define FLAG4_BIT_INDICATES_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE 1
#else /* CONFIG_EXPERIMENTAL_MMAP_DEC */
#define FLAG4_BIT_INDICATES_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE 0
#endif /* !CONFIG_EXPERIMENTAL_MMAP_DEC */


#if __SIZEOF_POINTER__ > 4
#define DEBUG_WORD4(x) UINT64_C(0x##x##x)
#else /* __SIZEOF_POINTER__ > 4 */
#define DEBUG_WORD4(x) UINT32_C(0x##x)
#endif /*__SIZEOF_POINTER__ <= 4 */

/* Custom dlmalloc extension:
 * fill free'd / newly allocated memory with patterns */
#undef DL_DEBUG_MEMSET_FREE
#undef DL_DEBUG_MEMSET_ALLOC
#if !defined(NDEBUG) && 1
#define DL_DEBUG_MEMSET_FREE  DEBUG_WORD4(DEADBEEF)
#define DL_DEBUG_MEMSET_ALLOC DEBUG_WORD4(CCCCCCCC)
#endif


/* API Configuration */
#define NO_MALLINFO          0
#define MALLINFO_FIELD_TYPE  size_t
#define NO_MALLOC_STATS      1
#define NO_SEGMENT_TRAVERSAL 0
#define NO_POSIX_MEMALIGN    1
#define NO_VALLOC            1
#define NO_PVALLOC           1
#define NO_MALLOPT           0
#define NO_MALLOC_FOOTPRINT  0
#define NO_INDEPENDENT_ALLOC 1
#define NO_BULK_FREE         1
#define NO_MALLOC_TRIM       0
#define EXPOSE_AS_DEEMON_API 1

/* Disable certain mspace-specific functions */
#define NO_MSPACE_FREE                1 /* 'Dee_Free()' is able to detect mspace chunks (via "FOOTERS"), so mspace_free() isn't needed */
#define NO_MSPACE_REALLOC             1 /* Like 'Dee_Free()', 'Dee_Realloc()' is also able to detect custom-mspace chunks */
#define NO_MSPACE_REALLOC_IN_PLACE    1 /* ... */
#define NO_MSPACE_USABLE_SIZE         1 /* Unnecessary dummy (same as `Dee_MallocUsableSize()') */
#define NO_MSPACE_TRACK_LARGE_CHUNKS  1 /* Not needed (default init by `create_mspace()' is what we want) */
#define NO_CREATE_MSPACE_WITH_BASE    1
#define NO_MSPACE_CALLOC              1
#define NO_MSPACE_MEMALIGN            1
#define NO_MSPACE_FOOTPRINT           1
#define NO_MSPACE_MAX_FOOTPRINT       1
#define NO_MSPACE_FOOTPRINT_LIMIT     1
#define NO_MSPACE_SET_FOOTPRINT_LIMIT 1
#define NO_MSPACE_TRIM                1
#define NO_DESTROY_MSPACE             1 /* Per-thread heaps are re-used as needed */

#undef M_TRIM_THRESHOLD
#undef M_GRANULARITY
#undef M_MMAP_THRESHOLD
#define M_TRIM_THRESHOLD Dee_HEAP_M_TRIM_THRESHOLD
#define M_GRANULARITY    Dee_HEAP_M_GRANULARITY
#define M_MMAP_THRESHOLD Dee_HEAP_M_MMAP_THRESHOLD

#undef ext_assert
#if DL_DEBUG_EXTERNAL
#define ext_assert Dee_ASSERT
#elif !defined(__NO_builtin_assume)
#define ext_assert __builtin_assume
#else /* ... */
#define ext_assert(expr) (void)0
#endif

#undef dl_assert
#if DL_DEBUG_INTERNAL
#define dl_assert Dee_ASSERT
#elif !defined(__NO_builtin_assume)
#define dl_assert __builtin_assume
#else /* ... */
#define dl_assert(expr) (void)0
#endif


#if !defined(CONFIG_HAVE_mmap) && defined(CONFIG_HAVE_mmap64)
#define CONFIG_HAVE_mmap 1
#undef mmap
#define mmap mmap64
#endif /* mmap = mmap64 */


/* System configuration -- figure out how we want to implement the heap backend */
#undef HAVE_MMAP
#undef HAVE_MMAP_IS_VirtualAlloc
#undef HAVE_MMAP_IS_malloc
#undef HAVE_MREMAP
#undef HAVE_MORECORE
#undef HAVE_MMAP_CLEARS
#if defined(CONFIG_HAVE_VirtualAlloc)
#define HAVE_MMAP 1
#define HAVE_MMAP_IS_VirtualAlloc /* Windows: use VirtualAlloc() */
#elif defined(CONFIG_HAVE_mmap) || defined(CONFIG_HAVE_sbrk)
#ifdef CONFIG_HAVE_mmap
#define HAVE_MMAP 1               /* Unix: use mmap()... */
#endif /* CONFIG_HAVE_mmap */
#ifdef CONFIG_HAVE_mremap
#define HAVE_MREMAP 1
#endif /* CONFIG_HAVE_mremap */
#ifdef CONFIG_HAVE_sbrk           /* Unix: ... and sbrk() */
#define HAVE_MORECORE 1
#endif /* CONFIG_HAVE_sbrk */
#elif defined(CONFIG_HAVE_malloc) || defined(CONFIG_HAVE_calloc)
#define HAVE_MMAP 1
#define HAVE_MMAP_IS_malloc       /* Generic: use the system's malloc() */
#ifndef CONFIG_HAVE_calloc
#define HAVE_MMAP_CLEARS 0
#endif /* !CONFIG_HAVE_calloc */
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
#define DEFAULT_GRANULARITY 0 /* 0 means to compute in init_mparams */
#else /* MORECORE_CONTIGUOUS */
#define DEFAULT_GRANULARITY (SIZE_C(64) * SIZE_C(1024))
#endif /* MORECORE_CONTIGUOUS */
#endif /* DEFAULT_GRANULARITY */

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





/************************************************************************/
/************************************************************************/
/************************************************************************/
/*                                                                      */
/* START OF DLMALLOC                                                    */
/*                                                                      */
/************************************************************************/
/************************************************************************/
/************************************************************************/

#if !NO_MALLINFO && !EXPOSE_AS_DEEMON_API
struct dlmalloc_mallinfo {
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
#endif /* !NO_MALLINFO && !EXPOSE_AS_DEEMON_API */

/* ------------------- Declarations of public routines ------------------- */

#if !ONLY_MSPACES
#define DIRECTLY_DEFINE_DEEMON_PUBLIC_API (!LEAK_DETECTION)
#if DIRECTLY_DEFINE_DEEMON_PUBLIC_API
#define dlmalloc           Dee_TryMalloc
#define dlcalloc           Dee_TryCalloc
#define dlfree             Dee_Free
#define dlrealloc          Dee_TryRealloc
#define dlmemalign         Dee_TryMemalign
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL Dee_TryMalloc)(size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL Dee_TryCalloc)(size_t n_bytes);
DFUNDEF void (DCALL Dee_Free)(void *ptr);
DFUNDEF WUNUSED void *(DCALL Dee_TryRealloc)(void *ptr, size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL Dee_TryMemalign)(size_t min_alignment, size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL Dee_TryReallocInPlace)(void *ptr, size_t n_bytes);
#else /* DIRECTLY_DEFINE_DEEMON_PUBLIC_API */
#define DLFREE_CHECKS_NULL    0 /* Not needed (already done by caller) */
#define DLREALLOC_CHECKS_NULL 0 /* Not needed (already done by caller) */
static ATTR_MALLOC WUNUSED void *dlmalloc(size_t n_bytes);
static ATTR_MALLOC WUNUSED void *dlcalloc(size_t n_bytes);
static void dlfree(void *ptr);
static WUNUSED void *dlrealloc(void *ptr, size_t n_bytes);
static ATTR_MALLOC WUNUSED void *dlmemalign(size_t min_alignment, size_t n_bytes);
#if LEAK_DETECTION_IN_TAIL
#define DLREALLOC_IN_PLACE_CHECKS_NULL 0 /* Not needed (already done by caller) */
static WUNUSED void *dlrealloc_in_place(void *ptr, size_t n_bytes);
#else /* LEAK_DETECTION_IN_TAIL */
DFUNDEF WUNUSED void *(DCALL Dee_TryReallocInPlace)(void *ptr, size_t n_bytes);
#endif /* !LEAK_DETECTION_IN_TAIL */
#endif /* !DIRECTLY_DEFINE_DEEMON_PUBLIC_API */

#if !NO_POSIX_MEMALIGN
static int dlposix_memalign(void **, size_t, size_t);
#endif /* !NO_POSIX_MEMALIGN */
#if !NO_VALLOC
static void *dlvalloc(size_t);
#endif /* !NO_VALLOC */
#if !NO_MALLOPT
#if EXPOSE_AS_DEEMON_API
DFUNDEF int DCALL DeeHeap_SetOpt(int option, size_t value);
#else /* EXPOSE_AS_DEEMON_API */
static int dlmallopt(int, int);
#endif /* !EXPOSE_AS_DEEMON_API */
#endif /* !NO_MALLOPT */
#if !NO_MALLOC_FOOTPRINT
#if EXPOSE_AS_DEEMON_API
DFUNDEF ATTR_PURE WUNUSED size_t DCALL DeeHeap_Footprint(void);
DFUNDEF ATTR_PURE WUNUSED size_t DCALL DeeHeap_MaxFootprint(void);
DFUNDEF ATTR_PURE WUNUSED size_t DCALL DeeHeap_GetFootprintLimit(void);
DFUNDEF size_t DCALL DeeHeap_SetFootprintLimit(size_t bytes);
#else /* EXPOSE_AS_DEEMON_API */
static size_t dlmalloc_footprint(void);
static size_t dlmalloc_max_footprint(void);
static size_t dlmalloc_footprint_limit();
static size_t dlmalloc_set_footprint_limit(size_t bytes);
#endif /* !EXPOSE_AS_DEEMON_API */
#endif /* !NO_MALLOC_FOOTPRINT */
#if MALLOC_INSPECT_ALL
static void dlmalloc_inspect_all(void (*handler)(void *, void *, size_t, void *), void *arg);
#endif /* MALLOC_INSPECT_ALL */
#if !NO_MALLINFO
#if EXPOSE_AS_DEEMON_API
DFUNDEF ATTR_PURE WUNUSED struct Dee_heap_mallinfo DCALL DeeHeap_MallInfo(void);
#else /* EXPOSE_AS_DEEMON_API */
static struct Dee_heap_mallinfo dlmallinfo(void);
#endif /* !EXPOSE_AS_DEEMON_API */
#endif /* NO_MALLINFO */
#if !NO_INDEPENDENT_ALLOC
static void **dlindependent_calloc(size_t, size_t, void **);
static void **dlindependent_comalloc(size_t, size_t *, void **);
#endif /* !NO_INDEPENDENT_ALLOC */
#if !NO_BULK_FREE
static size_t dlbulk_free(void **, size_t n_elements);
#endif /* !NO_BULK_FREE */
#if !NO_PVALLOC
static void *dlpvalloc(size_t);
#endif /* !NO_PVALLOC */
#if !NO_MALLOC_TRIM
#if EXPOSE_AS_DEEMON_API
DFUNDEF size_t DCALL DeeHeap_Trim(size_t pad);
#else /* EXPOSE_AS_DEEMON_API */
static int dlmalloc_trim(size_t);
#endif /* !EXPOSE_AS_DEEMON_API */
#endif /* !NO_MALLOC_TRIM */
#if !NO_MALLOC_STATS
static void dlmalloc_stats(void);
#endif /* !NO_MALLOC_STATS */
#endif /* ONLY_MSPACES */

#if MSPACES
typedef void *mspace;
static mspace create_mspace(size_t capacity/*, int locked*/);
#if !NO_DESTROY_MSPACE
static size_t destroy_mspace(mspace msp);
#endif /* NO_DESTROY_MSPACE */
#if !NO_CREATE_MSPACE_WITH_BASE
static mspace create_mspace_with_base(void *base, size_t capacity/*, int locked*/);
#endif /* !NO_CREATE_MSPACE_WITH_BASE */
#if !NO_MSPACE_TRACK_LARGE_CHUNKS
static int mspace_track_large_chunks(mspace msp, int enable);
#endif /* !NO_MSPACE_TRACK_LARGE_CHUNKS */
static void *mspace_malloc(mspace msp, size_t bytes);
#if !NO_MSPACE_FREE
static void mspace_free(mspace msp, void *mem);
#endif /* !NO_MSPACE_FREE */
#if !NO_MSPACE_REALLOC
static void *mspace_realloc(mspace msp, void *mem, size_t newsize);
#endif /* !NO_MSPACE_REALLOC */
#if !NO_MSPACE_REALLOC_IN_PLACE
static void *mspace_realloc_in_place(mspace msp, void *oldmem, size_t bytes);
#endif /* !NO_MSPACE_REALLOC_IN_PLACE */
#if !NO_MSPACE_CALLOC
static void *mspace_calloc(mspace msp, size_t n_elements, size_t elem_size);
#endif /* !NO_MSPACE_CALLOC */
#if !NO_MSPACE_MEMALIGN
static void *mspace_memalign(mspace msp, size_t alignment, size_t bytes);
#endif /* !NO_MSPACE_MEMALIGN */
#if !NO_INDEPENDENT_ALLOC
static void **mspace_independent_calloc(mspace msp, size_t n_elements, size_t elem_size, void *chunks[]);
static void **mspace_independent_comalloc(mspace msp, size_t n_elements, size_t sizes[], void *chunks[]);
#endif /* !NO_INDEPENDENT_ALLOC */
#if !NO_BULK_FREE
static size_t mspace_bulk_free(mspace msp, void *array[], size_t nelem);
#endif /* !NO_BULK_FREE */
#if !NO_MSPACE_FOOTPRINT
static size_t mspace_footprint(mspace msp);
#endif /* !NO_MSPACE_FOOTPRINT */
#if !NO_MSPACE_MAX_FOOTPRINT
static size_t mspace_max_footprint(mspace msp);
#endif /* !NO_MSPACE_MAX_FOOTPRINT */
#if !NO_MSPACE_FOOTPRINT_LIMIT
static size_t mspace_footprint_limit(mspace msp);
#endif /* !NO_MSPACE_FOOTPRINT_LIMIT */
#if !NO_MSPACE_SET_FOOTPRINT_LIMIT
static size_t mspace_set_footprint_limit(mspace msp, size_t bytes);
#endif /* !NO_MSPACE_SET_FOOTPRINT_LIMIT */
#if !NO_MALLINFO && !EXPOSE_AS_DEEMON_API
static struct dlmalloc_mallinfo mspace_mallinfo(mspace msp);
#endif /* NO_MALLINFO && !EXPOSE_AS_DEEMON_API */
#if !NO_MSPACE_USABLE_SIZE
static size_t mspace_usable_size(void const *mem);
#endif /* !NO_MSPACE_USABLE_SIZE */
#if !NO_MALLOC_STATS
static void mspace_malloc_stats(mspace msp);
#endif /* NO_MALLOC_STATS */
#if !NO_MSPACE_TRIM
static int mspace_trim(mspace msp, size_t pad);
#endif /* NO_MSPACE_TRIM */
#if !NO_MALLOPT && !EXPOSE_AS_DEEMON_API
static int mspace_mallopt(int param_number, int value);
#endif /* !NO_MALLOPT && !EXPOSE_AS_DEEMON_API */

#if USE_PER_THREAD_MSTATE
/* Return the calling thread's thread-local mspace (or "0" if not available) */
static mspace tls_mspace(void);
/* Invoke `cb' for every existing `tls_mspace()' ("cb" must still PREACTION()-lock said space) */
static size_t tls_mspace_foreach(size_t (*cb)(mspace ms, void *arg), void *arg);
#endif /* USE_PER_THREAD_MSTATE */

#endif /* MSPACES */


/*------------------------------ internal #includes ---------------------- */

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
DECL_END
#include <sys/param.h>
DECL_BEGIN
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
#define CMFAIL ((char *)(MFAIL)) /* defined for convenience */

#if HAVE_MMAP
#ifdef HAVE_MMAP_IS_VirtualAlloc
/* Win32 MMAP via VirtualAlloc */
FORCELOCAL void *win32mmap(size_t size) {
	void *ptr = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	return (ptr != 0) ? ptr : MFAIL;
}

/* For direct MMAP, use MEM_TOP_DOWN to minimize interference */
FORCELOCAL void *win32direct_mmap(size_t size) {
	void *ptr = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE);
	return (ptr != 0) ? ptr : MFAIL;
}

/* This function supports releasing coalesced segments */
FORCELOCAL int win32munmap(void *ptr, size_t size) {
	MEMORY_BASIC_INFORMATION minfo;
	char *cptr = (char *)ptr;
	while (size) {
		if (VirtualQuery(cptr, &minfo, sizeof(minfo)) == 0)
			return -1;
		if (minfo.BaseAddress != cptr || minfo.AllocationBase != cptr ||
		    minfo.State != MEM_COMMIT || minfo.RegionSize > size)
			return -1;
		if (VirtualFree(cptr, 0, MEM_RELEASE) == 0)
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
#ifdef CONFIG_HAVE_calloc
	void *ptr = calloc(1, size);
#else /* CONFIG_HAVE_calloc */
	void *ptr = malloc(size);
#endif /* !CONFIG_HAVE_calloc */
	return (ptr != 0) ? ptr : MFAIL;
}

/* NOTE: We can just use "free()" if it's available
 *       because we configure "NO_SEGMENT_TRAVERSAL"! */
#ifdef CONFIG_HAVE_free
#define DL_MUNMAP(a, s) native_malloc_munmap((a), (s))
FORCELOCAL int native_malloc_munmap(void *ptr, size_t size) {
	(void)size;
	free(ptr);
	return 0;
}
#endif /* CONFIG_HAVE_free */
#else /* ... */
#ifdef CONFIG_HAVE_munmap
#define DL_MUNMAP(a, s) munmap((a), (s))
#endif /* CONFIG_HAVE_munmap */
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
#define MMAP_PROT (PROT_READ | PROT_WRITE)
#ifdef CONFIG_HAVE_MAP_ANONYMOUS
#define MMAP_FLAGS (MAP_PRIVATE | MAP_ANONYMOUS)
#define DL_MMAP(s) mmap(0, (s), MMAP_PROT, MMAP_FLAGS, -1, 0)
#else /* CONFIG_HAVE_MAP_ANONYMOUS */
/* Nearly all versions of mmap support MAP_ANONYMOUS, so the following
 * is unlikely to be needed, but is supplied just in case. */
#define MMAP_FLAGS (MAP_PRIVATE)
static int dev_zero_fd = -1; /* Cached file descriptor for /dev/zero. */
#define DL_MMAP(s)                                                             \
	((dev_zero_fd < 0) ? (dev_zero_fd = open("/dev/zero", O_RDWR),             \
	                      mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0)) \
	                   : mmap(0, (s), MMAP_PROT, MMAP_FLAGS, dev_zero_fd, 0))
#endif /* !CONFIG_HAVE_MAP_ANONYMOUS */
#endif /* !... */
#endif /* HAVE_MMAP */


/**
 * Define DL_MMAP/DL_MUNMAP/DL_DIRECT_MMAP
 */
#if HAVE_MMAP
#define USE_MMAP_BIT (SIZE_T_ONE)
#else /* HAVE_MMAP */
#define USE_MMAP_BIT    (SIZE_T_ZERO)
#define DL_MMAP(s)      MFAIL
#endif /* HAVE_MMAP */
#ifndef DL_MUNMAP
#define DL_MUNMAP(a, s) (-1)
#define DL_MUNMAP_ALWAYS_FAILS
#endif /* !DL_MUNMAP */
#ifndef DL_DIRECT_MMAP
#define DL_DIRECT_MMAP(s) DL_MMAP(s)
#endif /* !DL_DIRECT_MMAP */

/**
 * Define DL_MREMAP
 */
#if HAVE_MMAP && HAVE_MREMAP
#ifdef CONFIG_HAVE_MREMAP_MAYMOVE
#define DL_MREMAP(addr, osz, nsz, mv) mremap((addr), (osz), (nsz), (mv) ? MREMAP_MAYMOVE : 0)
#else /* CONFIG_HAVE_MREMAP_MAYMOVE */
#define DL_MREMAP(addr, osz, nsz, mv) mremap((addr), (osz), (nsz), (mv))
#endif /* !CONFIG_HAVE_MREMAP_MAYMOVE */
#else /* HAVE_MMAP && HAVE_MREMAP */
#define DL_MREMAP(addr, osz, nsz, mv) MFAIL
#define DL_MREMAP_ALWAYS_FAILS
#endif /* HAVE_MMAP && HAVE_MREMAP */

#define USE_NONCONTIGUOUS_BIT (4U) /* mstate bit set if continguous morecore disabled or failed */
#if MSPACES && !NO_CREATE_MSPACE_WITH_BASE
#define EXTERN_BIT            (8U) /* segment bit set in create_mspace_with_base */
#endif /* MSPACES && !NO_CREATE_MSPACE_WITH_BASE */


/* --------------------------- Lock preliminaries ------------------------ */
#if GM_ONLY || !USE_LOCKS
#define ACQUIRE_MALLOC_GLOBAL_LOCK() (void)0
#define RELEASE_MALLOC_GLOBAL_LOCK() (void)0
#else /* GM_ONLY || !USE_LOCKS */
static Dee_atomic_lock_t malloc_global_lock = Dee_ATOMIC_LOCK_INIT;
#define ACQUIRE_MALLOC_GLOBAL_LOCK() Dee_atomic_lock_acquire(&malloc_global_lock)
#define RELEASE_MALLOC_GLOBAL_LOCK() Dee_atomic_lock_release(&malloc_global_lock)
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
#define chunk2mem(p)   ((void *)((char *)(p) + TWO_SIZE_T_SIZES))
#define mem2chunk(mem) ((mchunkptr)((char *)(mem) - TWO_SIZE_T_SIZES))

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
#define chunk_plus_offset(p, s)  ((mchunkptr)(((char *)(p)) + (s)))
#define chunk_minus_offset(p, s) ((mchunkptr)(((char *)(p)) - (s)))

/* Ptr to next or previous physical malloc_chunk. */
#define next_chunk(p) ((mchunkptr)(((char *)(p)) + ((p)->head & ~FLAG_BITS)))
#define prev_chunk(p) ((mchunkptr)(((char *)(p)) - ((p)->prev_foot)))

/* extract next chunk's pinuse bit */
#define next_pinuse(p) ((next_chunk(p)->head) & PINUSE_BIT)

/* Get/set size at footer */
#define get_foot(p, s) (((mchunkptr)((char *)(p) + (s)))->prev_foot)
#define set_foot(p, s) (((mchunkptr)((char *)(p) + (s)))->prev_foot = (s))

/* Set size, pinuse bit, and foot */
#define set_size_and_pinuse_of_free_chunk(p, s) \
	((p)->head = (s | PINUSE_BIT), set_foot(p, s))

/* Set size, pinuse bit, foot, and clear next pinuse */
#define set_free_with_pinuse(p, s, n) \
	(clear_pinuse(n), set_size_and_pinuse_of_free_chunk(p, s))

/* Get the internal overhead associated with chunk p */
#if FLAG4_BIT_INDICATES_HEAP_REGION
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
#if HAVE_MMAP_CLEARS && !defined(DL_DEBUG_MEMSET_ALLOC)
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
#ifdef DL_DEBUG_MEMSET_FREE
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
	char                  *base;   /* base address */
	size_t                 size;   /* allocated size */
	struct malloc_segment *next;   /* ptr to next segment */
	flag_t                 sflags; /* mmap and extern flag */
};

#define is_mmapped_segment(S) ((S)->sflags & USE_MMAP_BIT)
#ifdef EXTERN_BIT
#define is_extern_segment(S) ((S)->sflags & EXTERN_BIT)
#else /* EXTERN_BIT */
#define is_extern_segment(S) 0
#endif /* !EXTERN_BIT */

typedef struct malloc_segment msegment;
typedef struct malloc_segment *msegmentptr;

/* ----------------------------- Free List -------------------------------- */

#ifndef USE_PENDING_FREE_LIST
#define USE_PENDING_FREE_LIST 0
#endif /* !USE_PENDING_FREE_LIST */
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
PRIVATE binmap_t /* */ dl_gm_smallmap = 0;
PRIVATE binmap_t /* */ dl_gm_treemap = 0;
PRIVATE size_t /*   */ dl_gm_dvsize = 0;
PRIVATE size_t /*   */ dl_gm_topsize = 0;
PRIVATE char * /*   */ dl_gm_least_addr = 0;
PRIVATE mchunkptr /**/ dl_gm_dv = 0;
PRIVATE mchunkptr /**/ dl_gm_top = 0;
PRIVATE size_t /*   */ dl_gm_trim_check = 0;
PRIVATE size_t /*   */ dl_gm_release_checks = 0;
PRIVATE size_t /*   */ dl_gm_magic = 0;
PRIVATE mchunkptr /**/ dl_gm_smallbins[(NSMALLBINS + 1) * 2] = {};
PRIVATE tbinptr /*  */ dl_gm_treebins[NTREEBINS] = {};
PRIVATE size_t /*   */ dl_gm_footprint = 0;
PRIVATE size_t /*   */ dl_gm_max_footprint = 0;
PRIVATE size_t /*   */ dl_gm_footprint_limit = 0; /* zero means no limit */
PRIVATE flag_t /*   */ dl_gm_mflags = 0;
PRIVATE msegment /* */ dl_gm_seg = {};
#if USE_LOCKS
PRIVATE Dee_atomic_lock_t dl_gm_mutex = Dee_ATOMIC_LOCK_INIT;
#endif /* USE_LOCKS */
#if USE_PENDING_FREE_LIST
/* [0..n][lock(ATOMIC)] List of heap pointers that still need to be free'd */
PRIVATE struct freelist dl_gm_flist = SLIST_HEAD_INITIALIZER(dl_gm_flist);
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
	char     *least_addr;
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
	Dee_atomic_lock_t mutex; /* locate lock among fields that rarely change */
#endif /* USE_LOCKS */
#if USE_PENDING_FREE_LIST
	struct freelist flist;
#endif /* USE_PENDING_FREE_LIST */
	msegment  seg;
#if USE_PER_THREAD_MSTATE
	SLIST_ENTRY(malloc_state) ms_link; /* Link for "free_tls_mspace" / "used_tls_mspace" */
	uintptr_t ms_foreach_bitset; /* Atomic bitset of visit contexts */
#endif /* USE_PER_THREAD_MSTATE */
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

#define is_initialized(M) (mstate_top(M) != 0)

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
	((char *)(A) >= S->base && (char *)(A) < S->base + S->size)

/* Return segment holding given address */
static msegmentptr segment_holding(PARAM_mstate_m_ char *addr) {
	msegmentptr sp = &mstate_seg(m);
	for (;;) {
		if (addr >= sp->base && addr < sp->base + sp->size)
			return sp;
		if ((sp = sp->next) == 0)
			return 0;
	}
}

/* Return true if segment contains a segment link */
#if HAVE_MMAP
static int has_segment_link(PARAM_mstate_m_ msegmentptr ss) {
	msegmentptr sp = &mstate_seg(m);
	for (;;) {
		if ((char *)sp >= ss->base && (char *)sp < ss->base + ss->size)
			return 1;
		if ((sp = sp->next) == 0)
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
PRIVATE ATTR_NOINLINE void
dl_freelist_do_reap(PARAM_mstate_m_ struct freelist_entry *__restrict flist);
#define NEED_dl_freelist_do_reap

PRIVATE void dl_freelist_release_and_reap(PARAM_mstate_m) {
	struct freelist pending;
again:
	pending.slh_first = SLIST_ATOMIC_CLEAR(&mstate_flist(m));
	if unlikely(pending.slh_first)
		dl_freelist_do_reap(ARG_mstate_m_ pending.slh_first);
	Dee_atomic_lock_release(&mstate_mutex(m));
	if unlikely(atomic_read(&mstate_flist(m).slh_first) != NULL) {
		if (Dee_atomic_lock_tryacquire(&mstate_mutex(m)))
			goto again;
	}
}

#define TRY_PREACTION(M)  Dee_atomic_lock_tryacquire(&mstate_mutex(M))
#define PREACTION(M)      Dee_atomic_lock_acquire(&mstate_mutex(M))
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
#define TRY_PREACTION(M) Dee_atomic_lock_tryacquire(&mstate_mutex(M))
#define PREACTION(M)     Dee_atomic_lock_acquire(&mstate_mutex(M))
#define POSTACTION(M)    Dee_atomic_lock_release(&mstate_mutex(M))
#endif /* !USE_PENDING_FREE_LIST */


#if PROCEED_ON_ERROR
/* A count of the number of corruption errors causing resets */
int malloc_corruption_error_count;
/* default corruption action */
static void reset_on_error(PARAM_mstate_m);
#endif /* PROCEED_ON_ERROR */


/* -------------------------- Debugging setup ---------------------------- */

#ifdef DL_DEBUG_MEMSET_FREE
#define dl_setfree_word(W, T) (void)(W = (T)DL_DEBUG_MEMSET_FREE)
PRIVATE void DCALL dl_setfree_data(void *p, size_t n) {
	size_t *iter = (size_t *)p;
	while (n >= sizeof(size_t)) {
		*iter++ = DL_DEBUG_MEMSET_FREE;
		n -= sizeof(size_t);
	}
}
#else /* DL_DEBUG_MEMSET_FREE */
#define dl_setfree_word(W, T) (void)0
#define dl_setfree_data(p, n) (void)0
#endif /* !DL_DEBUG_MEMSET_FREE */
#ifdef DL_DEBUG_MEMSET_ALLOC
#define dl_setalloc_word(W, T) (void)(W = (T)DL_DEBUG_MEMSET_ALLOC)
PRIVATE void DCALL dl_setalloc_data(void *p, size_t n) {
	size_t *iter = (size_t *)p;
	while (n >= sizeof(size_t)) {
		*iter++ = DL_DEBUG_MEMSET_ALLOC;
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

#ifdef DL_DEBUG_MEMSET_FREE
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
#define smallbin_at(M, i) ((sbinptr)((char *)&(mstate_smallbins(M)[(i) << 1])))
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
#define ok_address(M, a) ((char *)(a) >= mstate_least_addr(M))
/* Check if address of next chunk n is higher than base chunk p */
#define ok_next(p, n)    ((char *)(p) < (char *)(n))
/* Check if p has inuse status */
#define ok_inuse(p)      is_inuse(p)
/* Check if p has its pinuse bit on */
#define ok_pinuse(p)     pinuse(p)
#else /* !INSECURE */
#define ok_address(M, a) (1)
#define ok_next(b, n)    (1)
#define ok_inuse(p)      (1)
#define ok_pinuse(p)     (1)
#endif /* !INSECURE */
#define ext_assert__ok_address(M, a) ext_assert(ok_address(M, a))
#define ext_assert__ok_next(p, n)    ext_assert(ok_next(p, n))
#define ext_assert__ok_inuse(p)      ext_assert(ok_inuse(p))
#define ext_assert__ok_pinuse(p)     ext_assert(ok_pinuse(p))


#if (FOOTERS && !INSECURE)
/* Check if (alleged) mstate m has expected magic field */
#define ok_magic(M) ((M)->magic == mparams.magic)
#else /* (FOOTERS && !INSECURE) */
#define ok_magic(M) (1)
#endif /* (FOOTERS && !INSECURE) */

#define ext_assert__ok_magic(M) ext_assert(ok_magic(M))


/* macros to set up inuse chunks with or without footers */

#if !FOOTERS

#define mark_inuse_foot(M, p, s)

/* Macros for setting head/foot of non-mmapped chunks */

/* Set cinuse bit and pinuse bit of next chunk */
#define set_inuse(M, p, s)                                    \
	((p)->head = (((p)->head & PINUSE_BIT) | s | CINUSE_BIT), \
	 ((mchunkptr)(((char *)(p)) + (s)))->head |= PINUSE_BIT)

/* Set cinuse and pinuse of this chunk and pinuse of next chunk */
#define set_inuse_and_pinuse(M, p, s)           \
	((p)->head = (s | PINUSE_BIT | CINUSE_BIT), \
	 ((mchunkptr)(((char *)(p)) + (s)))->head |= PINUSE_BIT)

/* Set size, cinuse and pinuse bit of this chunk */
#define set_size_and_pinuse_of_inuse_chunk(M, p, s) \
	((p)->head = (s | PINUSE_BIT | CINUSE_BIT))

#else /* FOOTERS */

/* Set foot of inuse chunk to be xor of mstate and seed */
#if GM_ONLY
#define mark_inuse_foot(M, p, s) \
	(((mchunkptr)((char *)(p) + (s)))->prev_foot = mparams.magic)
#define get_mstate_for(p)
#else /* GM_ONLY */
#define mark_inuse_foot(M, p, s) \
	(((mchunkptr)((char *)(p) + (s)))->prev_foot = ((size_t)(M) ^ mparams.magic))
#define get_mstate_for(p)                   \
	((mstate)(((mchunkptr)((char *)(p) +    \
	                       (chunksize(p)))) \
	          ->prev_foot ^                 \
	          mparams.magic))
#endif /* !GM_ONLY */


#define set_inuse(M, p, s)                                     \
	((p)->head = (((p)->head & PINUSE_BIT) | s | CINUSE_BIT),  \
	 (((mchunkptr)(((char *)(p)) + (s)))->head |= PINUSE_BIT), \
	 mark_inuse_foot(M, p, s))

#define set_inuse_and_pinuse(M, p, s)                          \
	((p)->head = (s | PINUSE_BIT | CINUSE_BIT),                \
	 (((mchunkptr)(((char *)(p)) + (s)))->head |= PINUSE_BIT), \
	 mark_inuse_foot(M, p, s))

#define set_size_and_pinuse_of_inuse_chunk(M, p, s) \
	((p)->head = (s | PINUSE_BIT | CINUSE_BIT),     \
	 mark_inuse_foot(M, p, s))

#endif /* !FOOTERS */

#if FLAG4_BIT_INDICATES_HEAP_REGION
#define do_validate_flag4_footer(p)                                        \
	do {                                                                   \
		size_t _ps = chunksize(p);                                         \
		size_t *_pval = &chunk_plus_offset(p, _ps)->prev_foot;             \
		ASSERTF(*_pval == _ps,                                             \
		        "flag4 mchunk footer at %p of chunk %p-%p has unexpected " \
		        "value %#" PRFxSIZ " when %#" PRFxSIZ " was expected",     \
		        _pval, p, (char *)(p) + _ps - 1, *_pval, _ps);             \
	}	__WHILE0
#else /* FLAG4_BIT_INDICATES_HEAP_REGION */
#define do_validate_flag4_footer_IS_NOOP
#define do_validate_flag4_footer(p) (void)0
#endif /* !FLAG4_BIT_INDICATES_HEAP_REGION */
#if FOOTERS && GM_ONLY
#define do_validate_inuse_footer(p)                                        \
	do {                                                                   \
		size_t _ps = chunksize(p);                                         \
		size_t *_pval = &chunk_plus_offset(p, _ps)->prev_foot;             \
		ASSERTF(*_pval == mparams.magic,                                   \
		        "inuse mchunk footer at %p of chunk %p-%p has unexpected " \
		        "value %#" PRFxSIZ " when %#" PRFxSIZ " was expected",     \
		        _pval, p, (char *)(p) + _ps - 1, *_pval, mparams.magic);   \
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
		gsize = ((DEFAULT_GRANULARITY != 0) ? DEFAULT_GRANULARITY : psize);
#else /* !HAVE_MMAP_IS_VirtualAlloc */
		{
			SYSTEM_INFO system_info;
			GetSystemInfo(&system_info);
			psize = system_info.dwPageSize;
			gsize = ((DEFAULT_GRANULARITY != 0) ? DEFAULT_GRANULARITY : system_info.dwAllocationGranularity);
		}
#endif /* HAVE_MMAP_IS_VirtualAlloc */

		/* Sanity-check configuration:
		   size_t must be unsigned and as wide as pointer type.
		   ints must be at least 4 bytes.
		   alignment must be at least 8.
		   Alignment, min chunk size, and page size must all be powers of 2.
		*/
#if 0
		if ((sizeof(size_t) != sizeof(char*)) ||
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
#elif defined(CONFIG_HAVE_time)
			magic = (size_t)(time(0) ^ (size_t)0x55555555U);
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
#if !EXPOSE_AS_DEEMON_API
static int change_mparam(int param_number, int value)
#else /* !EXPOSE_AS_DEEMON_API */
DFUNDEF int DCALL DeeHeap_SetOpt(int param_number, size_t val)
#endif /* EXPOSE_AS_DEEMON_API */
{
#if !EXPOSE_AS_DEEMON_API
	size_t val = (value == -1) ? SIZE_MAX : (size_t)value;
#endif /* !EXPOSE_AS_DEEMON_API */
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

#ifdef DL_DEBUG_MEMSET_FREE
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
		ASSERTF(words[i] == DL_DEBUG_MEMSET_FREE,
		        "Free pointer %p at offset %" PRFxSIZ " from %p-%p has bad pattern %#" PRFXSIZ "",
		        &words[i], i * sizeof(size_t), p, (char *)p + num_bytes - 1, words[i]);
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
	msegmentptr sp = segment_holding(ARG_mstate_m_(char *) p);
	size_t sz      = p->head & ~INUSE_BITS; /* third-lowest bit can be set! */
	ASSERT(sp != 0);
	ASSERT((is_aligned(chunk2mem(p))) || (p->head == FENCEPOST_HEAD));
	ASSERT(ok_address(m, p));
	ASSERT(sz == mstate_topsize(m));
	ASSERT(sz > 0);
	ASSERT(sz == ((sp->base + sp->size) - (char *)p) - TOP_FOOT_SIZE);
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
	if (mem != 0) {
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
	tchunkptr head  = 0;
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
		if (u->parent == 0) {
			ASSERT(u->child[0] == 0);
			ASSERT(u->child[1] == 0);
		} else {
			ASSERT(head == 0); /* only one node on chain has parent */
			head = u;
			ASSERT(u->parent != u);
			ASSERT(u->parent->child[0] == u ||
			          u->parent->child[1] == u ||
			          *((tbinptr *)(u->parent)) == u);
			if (u->child[0] != 0) {
				ASSERT(u->child[0]->parent == u);
				ASSERT(u->child[0] != u);
				do_check_tree(ARG_mstate_m_ u->child[0]);
			}
			if (u->child[1] != 0) {
				ASSERT(u->child[1]->parent == u);
				ASSERT(u->child[1] != u);
				do_check_tree(ARG_mstate_m_ u->child[1]);
			}
			if (u->child[0] != 0 && u->child[1] != 0) {
				ASSERT(chunksize(u->child[0]) < chunksize(u->child[1]));
			}
		}
		u = u->fd;
	} while (u != t);
	ASSERT(head != 0);
}

/*  Check all the chunks in a treebin.  */
static void do_check_treebin(PARAM_mstate_m_ bindex_t i) {
	tbinptr *tb = treebin_at(m, i);
	tchunkptr t = *tb;
	int empty   = (mstate_treemap(m) & (1U << i)) == 0;
	if (t == 0)
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
			while (t != 0 && chunksize(t) != size) {
				t = t->child[(sizebits >> (SIZE_T_BITSIZE - SIZE_T_ONE)) & 1];
				sizebits <<= 1;
			}
			if (t != 0) {
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
		while (s != 0) {
			mchunkptr q     = align_as_chunk(s->base);
			mchunkptr lastq = 0;
			ASSERT(pinuse(q));
			while (segment_holds(s, q) &&
			       q != mstate_top(m) && q->head != FENCEPOST_HEAD) {
				sum += chunksize(q);
				if (is_inuse(q)) {
					ASSERT(!bin_find(ARG_mstate_m_ q));
					do_check_inuse_chunk(ARG_mstate_m_ q);
				} else {
					ASSERT(q == mstate_dv(m) || bin_find(ARG_mstate_m_ q));
					ASSERT(lastq == 0 || is_inuse(lastq)); /* Not 2 consecutive free */
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
#if !EXPOSE_AS_DEEMON_API
#define STRUCT_dlmalloc_mallinfo struct dlmalloc_mallinfo
#else /* !EXPOSE_AS_DEEMON_API */
#define STRUCT_dlmalloc_mallinfo struct Dee_heap_mallinfo
#endif /* EXPOSE_AS_DEEMON_API */

static void do_gather_internal_mallinfo(PARAM_mstate_m_ STRUCT_dlmalloc_mallinfo *nm) {
	PREACTION(m);
	check_malloc_state(m);
	if (is_initialized(m)) {
		size_t nfree  = SIZE_T_ONE; /* top always free */
		size_t mfree  = mstate_topsize(m) + TOP_FOOT_SIZE;
		size_t sum    = mfree;
		msegmentptr s = &mstate_seg(m);
		while (s != 0) {
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
#if EXPOSE_AS_DEEMON_API
#define nm__hmi_(x) nm->hmi_##x
#else /* EXPOSE_AS_DEEMON_API */
#define nm__hmi_(x) nm->x
#endif /* !EXPOSE_AS_DEEMON_API */
#if USE_PER_THREAD_MSTATE
#define nm__hmi_account(x, v) nm__hmi_(x) += v
#else /* USE_PER_THREAD_MSTATE */
#define nm__hmi_account(x, v) nm__hmi_(x) = v
#endif /* !USE_PER_THREAD_MSTATE */
		nm__hmi_account(arena, sum);
		nm__hmi_account(ordblks, nfree);
		nm__hmi_account(hblkhd, mstate_footprint(m) - sum);
		nm__hmi_account(usmblks, mstate_max_footprint(m));
		nm__hmi_account(uordblks, mstate_footprint(m) - mfree);
		nm__hmi_account(fordblks, mfree);
		nm__hmi_account(keepcost, mstate_topsize(m));
#undef nm__hmi_account
#undef nm__hmi_
	}
	POSTACTION(m);
}

#if USE_PER_THREAD_MSTATE
PRIVATE size_t do_gather_internal_mallinfo_cb(mspace ms, void *arg) {
	do_gather_internal_mallinfo(ARG_mstate_X_((mstate)ms) (STRUCT_dlmalloc_mallinfo *)arg);
	return 0;
}
#endif /* USE_PER_THREAD_MSTATE */

#if !EXPOSE_AS_DEEMON_API
static STRUCT_dlmalloc_mallinfo internal_mallinfo(PARAM_mstate_m)
#define lm m
#else /* !EXPOSE_AS_DEEMON_API */
PUBLIC ATTR_PURE WUNUSED STRUCT_dlmalloc_mallinfo DCALL DeeHeap_MallInfo(void)
#define lm gm
#endif /* EXPOSE_AS_DEEMON_API */
{
#if EXPOSE_AS_DEEMON_API
	STRUCT_dlmalloc_mallinfo nm = { 0, 0, 0, 0, 0, 0, 0 };
#else /* EXPOSE_AS_DEEMON_API */
	STRUCT_dlmalloc_mallinfo nm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#endif /* !EXPOSE_AS_DEEMON_API */
#ifdef HOOK_AFTER_INIT_MALLINFO
	ensure_initialization_for(HOOK_AFTER_INIT_MALLINFO());
#else /* HOOK_AFTER_INIT_MALLINFO */
	ensure_initialization();
#endif /* !HOOK_AFTER_INIT_MALLINFO */
	do_gather_internal_mallinfo(ARG_mstate_X_(lm) &nm);

	/* Include info from "tls_mspace()" */
#if USE_PER_THREAD_MSTATE
	tls_mspace_foreach(&do_gather_internal_mallinfo_cb, &nm);
#endif /* USE_PER_THREAD_MSTATE */

	return nm;
}
#undef lm
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

		while (s != 0) {
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
		X->child[0] = X->child[1] = 0;                                                    \
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
					if (*C != 0)                                                          \
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
					X->parent     = 0;                                                    \
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
			if (((R = *(RP = &(X->child[1]))) != 0) ||            \
			    ((R = *(RP = &(X->child[0]))) != 0)) {            \
				tchunkptr *CP;                                    \
				while ((*(CP = &(R->child[1])) != 0) ||           \
				       (*(CP = &(R->child[0])) != 0)) {           \
					R = *(RP = CP);                               \
				}                                                 \
				ext_assert__ok_address(M, RP);                    \
				*RP = 0;                                          \
			}                                                     \
		}                                                         \
		if (XP != 0) {                                            \
			tbinptr *H = treebin_at(M, X->index);                 \
			if (X == *H) {                                        \
				if ((*H = R) == 0)                                \
					clear_treemap(M, X->index);                   \
			} else {                                              \
				ext_assert__ok_address(M, XP);                    \
				if (XP->child[0] == X)                            \
					XP->child[0] = R;                             \
				else                                              \
					XP->child[1] = R;                             \
			}                                                     \
			if (R != 0) {                                         \
				tchunkptr C0, C1;                                 \
				ext_assert__ok_address(M, R);                     \
				R->parent = XP;                                   \
				if ((C0 = X->child[0]) != 0) {                    \
					ext_assert__ok_address(M, C0);                \
					R->child[0] = C0;                             \
					C0->parent  = R;                              \
				}                                                 \
				if ((C1 = X->child[1]) != 0) {                    \
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
			return 0;
	}
	if (mmsize > nb) { /* Check for wrap around 0 */
		char *mm = (char *)(DL_DIRECT_MMAP(mmsize));
		if (mm != CMFAIL) {
			size_t offset = align_offset(chunk2mem(mm));
			size_t psize  = mmsize - offset - MMAP_FOOT_PAD;
			mchunkptr p   = (mchunkptr)(mm + offset);
			p->prev_foot  = offset;
			p->head       = psize;
			mark_inuse_foot(m, p, psize);
			chunk_plus_offset(p, psize)->head = FENCEPOST_HEAD;
			chunk_plus_offset(p, psize + SIZE_T_SIZE)->head = 0;

			if (mstate_least_addr(m) == 0 || mm < mstate_least_addr(m))
				mstate_least_addr(m) = mm;
			if ((mstate_footprint(m) += mmsize) > mstate_max_footprint(m))
				mstate_max_footprint(m) = mstate_footprint(m);
			dl_assert(is_aligned(chunk2mem(p)));
			check_mmapped_chunk(m, p);
			return chunk2mem(p);
		}
	}
	return 0;
}

/* Realloc using mmap */
#undef mmap_resize
#ifdef DL_MREMAP_ALWAYS_FAILS
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
		return 0;
	/* Keep old chunk if big enough but not too big */
	if (oldsize >= nb + SIZE_T_SIZE &&
	    (oldsize - nb) <= (mparams.granularity << 1)) {
		return oldp;
	} else {
#ifndef DL_MREMAP_ALWAYS_FAILS
		size_t offset    = oldp->prev_foot;
		size_t oldmmsize = oldsize + offset + MMAP_FOOT_PAD;
		size_t newmmsize = mmap_align(nb + SIX_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
		char *cp = (char *)DL_MREMAP((char *)oldp - offset, oldmmsize, newmmsize, flags);
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
	return 0;
}


/* -------------------------- mspace management -------------------------- */

/* Initialize top chunk and its size */
static void init_top(PARAM_mstate_m_ mchunkptr p, size_t psize) {
	/* Ensure alignment */
	size_t offset = align_offset(chunk2mem(p));
	p             = (mchunkptr)((char *)p + offset);
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
	mstate_seg(m).base = 0;
	mstate_seg(m).size = 0;
	mstate_seg(m).next = 0;
	mstate_top(m) = mstate_dv(m) = 0;
	for (i = 0; i < NTREEBINS; ++i)
		*treebin_at(m, i) = 0;
	init_bins(m);
}
#endif /* PROCEED_ON_ERROR */

/* Allocate chunk and prepend remainder with chunk in successor base. */
static void *prepend_alloc(PARAM_mstate_m_ char *newbase, char *oldbase,
                           size_t nb) {
	mchunkptr p        = align_as_chunk(newbase);
	mchunkptr oldfirst = align_as_chunk(oldbase);
	size_t psize       = (char *)oldfirst - (char *)p;
	mchunkptr q        = chunk_plus_offset(p, nb);
	size_t qsize       = psize - nb;
	set_size_and_pinuse_of_inuse_chunk(m, p, nb);

	dl_assert((char *)oldfirst > (char *)q);
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
static void add_segment(PARAM_mstate_m_ char *tbase, size_t tsize, flag_t mmapped) {
	/* Determine locations and sizes of segment, fenceposts, old top */
	char *old_top     = (char *)mstate_top(m);
	msegmentptr oldsp = segment_holding(ARG_mstate_m_ old_top);
	char *old_end     = oldsp->base + oldsp->size;
	size_t ssize      = pad_request(sizeof(struct malloc_segment));
	char *rawsp       = old_end - (ssize + FOUR_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
	size_t offset     = align_offset(chunk2mem(rawsp));
	char *asp         = rawsp + offset;
	char *csp         = (asp < (old_top + MIN_CHUNK_SIZE)) ? old_top : asp;
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
		if ((char *)(&(nextp->head)) < old_end)
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
	char *tbase      = CMFAIL;
	size_t tsize     = 0;
	flag_t mmap_flag = 0;
	size_t asize; /* allocation size */

#if !USE_LOCKS || MSPACES || GM_ONLY
	ensure_initialization();
#endif

	/* Directly map large chunks, but only if already initialized */
	if (use_mmap(m) && nb >= mparams.mmap_threshold && mstate_topsize(m) != 0) {
		void *mem = mmap_alloc(ARG_mstate_m_ nb);
		if (mem != 0)
			return mem;
	}

	asize = granularity_align(nb + SYS_ALLOC_PADDING);
	if (asize <= nb)
		return 0; /* wraparound */
	if (mstate_footprint_limit(m) != 0) {
		size_t fp = mstate_footprint(m) + asize;
		if (fp <= mstate_footprint(m) || fp > mstate_footprint_limit(m))
			return 0;
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
		char *br       = CMFAIL;
		size_t ssize   = asize; /* sbrk call size */
		msegmentptr ss = (mstate_top(m) == 0) ? 0 : segment_holding(ARG_mstate_m_ (char *)mstate_top(m));
		ACQUIRE_MALLOC_GLOBAL_LOCK();

		if (ss == 0) { /* First time through or recovery */
			char *base = (char *)sbrk(0);
			if (base != CMFAIL) {
				size_t fp;
				/* Adjust to end on a page boundary */
				if (!is_page_aligned(base))
					ssize += (page_align((size_t)base) - (size_t)base);
				fp = mstate_footprint(m) + ssize; /* recheck limits */
				if (ssize > nb && ssize < HALF_MAX_SIZE_T &&
				    (mstate_footprint_limit(m) == 0 ||
				     (fp > mstate_footprint(m) && fp <= mstate_footprint_limit(m))) &&
				    (br = (char *)(sbrk(ssize))) == base) {
					tbase = base;
					tsize = ssize;
				}
			}
		} else {
			/* Subtract out existing available top space from MORECORE request. */
			ssize = granularity_align(nb - mstate_topsize(m) + SYS_ALLOC_PADDING);
			/* Use mem here only if it did continuously extend old space */
			if (ssize < HALF_MAX_SIZE_T &&
			    (br = (char *)(sbrk(ssize))) == ss->base + ss->size) {
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
						char *end = (char *)sbrk(esize);
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
		char *mp = (char *)(DL_MMAP(asize));
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
			char *br  = CMFAIL;
			char *end = CMFAIL;
			ACQUIRE_MALLOC_GLOBAL_LOCK();
			br  = (char *)(sbrk(asize));
			end = (char *)(sbrk(0));
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
			if (mstate_least_addr(m) == 0 || tbase < mstate_least_addr(m))
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
				init_top(ARG_mstate_m_ mn, (size_t)((tbase + tsize) - (char *)mn) - TOP_FOOT_SIZE);
			}
#endif /* !GM_ONLY */
		} else {
			/* Try to merge with an existing segment */
			msegmentptr sp = &mstate_seg(m);
			/* Only consider most recent segment if traversal suppressed */
			while (sp != 0 && tbase != sp->base + sp->size)
				sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->next;
			if (sp != 0 &&
			    !is_extern_segment(sp) &&
			    (sp->sflags & USE_MMAP_BIT) == mmap_flag &&
			    segment_holds(sp, mstate_top(m))) { /* append */
				sp->size += tsize;
				init_top(ARG_mstate_m_ mstate_top(m), mstate_topsize(m) + tsize);
			} else {
				if (tbase < mstate_least_addr(m))
					mstate_least_addr(m) = tbase;
				sp = &mstate_seg(m);
				while (sp != 0 && sp->base != tbase + tsize)
					sp = (NO_SEGMENT_TRAVERSAL) ? 0 : sp->next;
				if (sp != 0 &&
				    !is_extern_segment(sp) &&
				    (sp->sflags & USE_MMAP_BIT) == mmap_flag) {
					char *oldbase = sp->base;
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
	return 0;
}

/* -----------------------  system deallocation -------------------------- */

/* Unmap and unlink any mmapped segments that don't contain used chunks */
static size_t release_unused_segments(PARAM_mstate_m) {
	size_t released  = 0;
	int nsegs        = 0;
	msegmentptr pred = &mstate_seg(m);
	msegmentptr sp   = pred->next;
	while (sp != 0) {
		char *base       = sp->base;
		size_t size      = sp->size;
		msegmentptr next = sp->next;
		++nsegs;
		if (is_mmapped_segment(sp) && !is_extern_segment(sp)) {
			mchunkptr p  = align_as_chunk(base);
			size_t psize = chunksize(p);
			/* Can unmap if first chunk holds entire segment and not pinned */
			if (!is_inuse(p) && (char *)p + psize >= base + size - TOP_FOOT_SIZE) {
				tchunkptr tp = (tchunkptr)p;
				dl_assert(segment_holds(sp, (char *)sp));
				check_memset_use_after_free(m, p);
				if (p == mstate_dv(m)) {
					mstate_dv(m)     = 0;
					mstate_dvsize(m) = 0;
				} else {
					unlink_large_chunk(m, tp);
				}
#ifndef DL_MUNMAP_ALWAYS_FAILS
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

#if !EXPOSE_AS_DEEMON_API
static int sys_trim(PARAM_mstate_m_ size_t pad)
#else /* !EXPOSE_AS_DEEMON_API */
static size_t sys_trim(PARAM_mstate_m_ size_t pad)
#endif /* EXPOSE_AS_DEEMON_API */
{
	size_t released = 0;
	if (pad < MAX_REQUEST && is_initialized(m)) {
		pad += TOP_FOOT_SIZE; /* ensure enough room for segment overhead */

		if (mstate_topsize(m) > pad) {
			/* Shrink top space in granularity-size units, keeping at least one */
			size_t unit  = mparams.granularity;
			size_t extra = ((mstate_topsize(m) - pad + (unit - SIZE_T_ONE)) / unit -
			                SIZE_T_ONE) *
			               unit;
			msegmentptr sp = segment_holding(ARG_mstate_m_(char *) mstate_top(m));
			(void)extra;

			if (!is_extern_segment(sp)) {
				if (is_mmapped_segment(sp)) {
#if HAVE_MMAP && (!defined(DL_MREMAP_ALWAYS_FAILS) || !defined(DL_MUNMAP_ALWAYS_FAILS))
					if (sp->size >= extra &&
					    !has_segment_link(ARG_mstate_m_ sp)) { /* can't shrink if pinned */
						size_t newsize = sp->size - extra;
						(void)newsize; /* placate people compiling -Wunused-variable */
						/* Prefer mremap, fall back to munmap */
						if (0 ||
#ifndef DL_MREMAP_ALWAYS_FAILS
						    (DL_MREMAP(sp->base, sp->size, newsize, 0) != MFAIL) ||
#endif /* !DL_MREMAP_ALWAYS_FAILS */
#ifndef DL_MUNMAP_ALWAYS_FAILS
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
						char *old_br = (char *)(sbrk(0));
						if (old_br == sp->base + sp->size) {
							char *rel_br = (char *)(sbrk(-extra));
							char *new_br = (char *)(sbrk(0));
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

#if EXPOSE_AS_DEEMON_API
	return released;
#else /* EXPOSE_AS_DEEMON_API */
	return (released != 0) ? 1 : 0;
#endif /* !EXPOSE_AS_DEEMON_API */
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
#ifndef DL_MUNMAP_ALWAYS_FAILS
			if (DL_MUNMAP((char *)p - prevsize, psize) == 0)
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
				mstate_dv(m)     = 0;
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
	tchunkptr v  = 0;
	size_t rsize = SIZE_T_ZERO - nb; /* Unsigned negation */
	tchunkptr t;
	bindex_t idx;
	compute_tree_index(nb, idx);
	if ((t = *treebin_at(m, idx)) != 0) {
		/* Traverse tree for this bin looking for node with size == nb */
		size_t sizebits = nb << leftshift_for_tree_index(idx);
		tchunkptr rst   = 0; /* The deepest untaken right subtree */
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
			if (rt != 0 && rt != t)
				rst = rt;
			if (t == 0) {
				t = rst; /* set t to least subtree holding sizes > nb */
				break;
			}
			sizebits <<= 1;
		}
	}
	if (t == 0 && v == 0) { /* set t to root of next non-empty treebin */
		binmap_t leftbits = left_bits(idx2bit(idx)) & mstate_treemap(m);
		if (leftbits != 0) {
			bindex_t i;
			binmap_t leastbit = least_bit(leftbits);
			compute_bit2idx(leastbit, i);
			t = *treebin_at(m, i);
		}
	}

	while (t != 0) { /* find smallest of tree or subtree */
		size_t trem = chunksize(t) - nb;
		if (trem < rsize) {
			rsize = trem;
			v     = t;
		}
		t = leftmost_child(t);
	}

	/*  If dv is a better fit, return 0 so malloc will use it */
	if (v != 0 && rsize < (size_t)(mstate_dvsize(m) - nb)) {
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
	return 0;
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

	while ((t = leftmost_child(t)) != 0) {
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

#if !DIRECTLY_DEFINE_DEEMON_PUBLIC_API
static ATTR_MALLOC WUNUSED void *dlmalloc(size_t bytes)
#else /* !DIRECTLY_DEFINE_DEEMON_PUBLIC_API */
#define Dee_TryMalloc_DEFINED
PUBLIC ATTR_MALLOC WUNUSED void *(DCALL Dee_TryMalloc)(size_t bytes)
#endif /* DIRECTLY_DEFINE_DEEMON_PUBLIC_API */
{
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

#ifdef MALLOC_ZERO_RETURNS_NULL
	if unlikely(!bytes)
		return NULL;
#endif /* MALLOC_ZERO_RETURNS_NULL */
#ifdef HOOK_AFTER_INIT_MALLOC
	ensure_initialization_for(HOOK_AFTER_INIT_MALLOC(bytes));
#else /* HOOK_AFTER_INIT_MALLOC */
#if USE_LOCKS && !GM_STATIC_INIT_MUTEX
	ensure_initialization(); /* initialize in sys_alloc if not using locks */
#endif
#endif /* !HOOK_AFTER_INIT_MALLOC */

	/* This "PREACTION(gm)" right here is ****THE**** biggest bottleneck in
	 * all of deemon when it comes to executing heavily parallelized user-code!
	 *
	 * Solution:
	 * - Thread-local heap [cache]?
	 *   - including per-thread free lists, since it isn't
	 *     actually that likely for the allocating thread to
	 *     also be the one that free's memory, thanks to the
	 *     design of the "leaks" detector's pending lists
	 */
#if USE_PER_THREAD_MSTATE
	if (!TRY_PREACTION(gm)) {
		mspace tls = tls_mspace();
		if (tls)
			return mspace_malloc(tls, bytes);
		PREACTION(gm);
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

			else if (gm__treemap != 0 && (mem = tmalloc_small(ARG_mstate_gm_ nb)) != 0) {
				check_malloced_chunk(gm, mem, nb);
				goto postaction;
			}
		}
	} else if (bytes >= MAX_REQUEST)
		nb = SIZE_MAX; /* Too big to allocate. Force failure (in sys alloc) */
	else {
		nb = pad_request(bytes);
		if (gm__treemap != 0 && (mem = tmalloc_large(ARG_mstate_gm_ nb)) != 0) {
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
			gm__dv     = 0;
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
#ifdef DL_DEBUG_MEMSET_ALLOC
	if (mem != 0) {
		mchunkptr p = mem2chunk(mem);
		dl_setalloc_data(mem, chunksize(p) - overhead_for(p));
	}
#endif /* DL_DEBUG_MEMSET_ALLOC */
	return mem;
}

/* ---------------------------- free --------------------------- */

#if FLAG4_BIT_INDICATES_HEAP_REGION
/* Meaning of flags when "struct Dee_heapregion" comes into play:
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
	ASSERT(pinuse(next) || flag4inuse(next) || next->head == 0);
	ASSERT(pinuse(prev) || flag4inuse(prev));
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
	if (prevsize == 0 && next->head == 0) {
		/* Last chunk of user-defined "struct Dee_heapregion" just got free'd! */
		struct Dee_heapregion *region;
#define REGION_OVERHEAD (offsetof(struct Dee_heapregion, hr_first) + sizeof(struct Dee_heaptail))
		region = COMPILER_CONTAINER_OF((struct Dee_heapchunk *)p,
		                               struct Dee_heapregion, hr_first);
		ASSERT(region->hr_size == (psize + REGION_OVERHEAD));
		ASSERT(region->hr_destroy);
		ASSERT(region->hr_first.hc_prevsize == 0);
		ASSERT(region->hr_first.hc_head == (psize | PINUSE_BIT));
		(*region->hr_destroy)(region);
#undef REGION_OVERHEAD
	}
}
#endif /* FLAG4_BIT_INDICATES_HEAP_REGION */

#if !DIRECTLY_DEFINE_DEEMON_PUBLIC_API
static void dlfree(void *mem)
#else /* !DIRECTLY_DEFINE_DEEMON_PUBLIC_API */
#define Dee_Free_DEFINED
PUBLIC void (DCALL Dee_Free)(void *mem)
#endif /* DIRECTLY_DEFINE_DEEMON_PUBLIC_API */
{
#ifndef DLFREE_CHECKS_NULL
#define DLFREE_CHECKS_NULL 1
#endif /* !DLFREE_CHECKS_NULL */

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
#if DLFREE_CHECKS_NULL
	if unlikely(!mem)
		return;
#endif /* DLFREE_CHECKS_NULL */

	p = mem2chunk(mem);
	validate_footer(p);

	ext_assert__ok_inuse(p);
	psize = chunksize(p);
	next  = chunk_plus_offset(p, psize);

#if FLAG4_BIT_INDICATES_HEAP_REGION
	if unlikely(is_mmapped(p)) {
#ifndef DL_MUNMAP_ALWAYS_FAILS
		size_t prevsize;
#endif /* !DL_MUNMAP_ALWAYS_FAILS */
		if unlikely(flag4inuse(p)) {
#if defined(DL_DEBUG_MEMSET_FREE) && FLAG4_BIT_INDICATES_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE
			/* Special case: don't set the "DL_DEBUG_MEMSET_FREE" word
			 *               at one of the following offsets, if that
			 *               word is currently set to "0". */
#define INDEXOF_WORD(T, field) (offsetof(T, field) / sizeof(Dee_refcnt_t))
			size_t usize = chunksize(p) - overhead_for(p);
			STATIC_ASSERT(IS_ALIGNED(offsetof(DeeObject, ob_refcnt), sizeof(Dee_refcnt_t)));
			STATIC_ASSERT(IS_ALIGNED(offsetof(struct Dee_gc_head, gc_object.ob_refcnt), sizeof(Dee_refcnt_t)));
			size_t i, n_words = usize / sizeof(Dee_refcnt_t);
			Dee_refcnt_t *iter = (Dee_refcnt_t *)mem;
			for (i = 0; i < n_words; ++i, ++iter) {
				if ((i == INDEXOF_WORD(DeeObject, ob_refcnt) ||
				     i == INDEXOF_WORD(struct Dee_gc_head, gc_object.ob_refcnt)) &&
				    (atomic_read(iter) == 0)) {
					/* Skip this word */
				} else {
					*iter = DL_DEBUG_MEMSET_FREE;
				}
			}
#undef INDEXOF_WORD
#else /* DL_DEBUG_MEMSET_FREE && FLAG4_BIT_INDICATES_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE */
			dl_setfree_data(mem, chunksize(p) - overhead_for(p));
#endif /* !DL_DEBUG_MEMSET_FREE || !FLAG4_BIT_INDICATES_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE */
			free_flag4_mem(p);
			return;
		}
		dl_setfree_data(mem, chunksize(p) - overhead_for(p));
#ifdef DL_MUNMAP_ALWAYS_FAILS
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
		if (DL_MUNMAP((char *)p - prevsize, psize) == 0)
			mstate_footprint(fm) -= psize;
		goto postaction;
#endif /* !DL_MUNMAP_ALWAYS_FAILS */
	}
#endif /* FLAG4_BIT_INDICATES_HEAP_REGION */

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
#if FLAG4_BIT_INDICATES_HEAP_REGION
#if DL_DEBUG_INTERNAL
	if (!flag4inuse(p))
		check_inuse_chunk(fm, p);
#endif /* DL_DEBUG_INTERNAL */
#else /* FLAG4_BIT_INDICATES_HEAP_REGION */
	check_inuse_chunk(fm, p);
	ext_assert__ok_address(fm, p);
#endif /* !FLAG4_BIT_INDICATES_HEAP_REGION */

	if (!pinuse(p)) {
		mchunkptr prev;
		size_t prevsize = p->prev_foot;
#if !FLAG4_BIT_INDICATES_HEAP_REGION
		if unlikely(is_mmapped(p)) {
#ifndef DL_MUNMAP_ALWAYS_FAILS
			psize += prevsize + MMAP_FOOT_PAD;
			if (DL_MUNMAP((char *)p - prevsize, psize) == 0)
				mstate_footprint(fm) -= psize;
#endif /* !DL_MUNMAP_ALWAYS_FAILS */
			goto postaction;
		}
#endif /* !FLAG4_BIT_INDICATES_HEAP_REGION */

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
				mstate_dv(fm)     = 0;
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

#ifdef NEED_dl_freelist_do_reap
PRIVATE ATTR_NOINLINE void
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
		size_t prevsize = p->prev_foot;
		mchunkptr prev;
		if unlikely(is_mmapped(p)) {
#ifndef DL_MUNMAP_ALWAYS_FAILS
			psize += prevsize + MMAP_FOOT_PAD;
			if (DL_MUNMAP((char *)p - prevsize, psize) == 0)
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
				mstate_dv(m)     = 0;
				mstate_dvsize(m) = 0;
			}
			if (should_trim(m, tsize))
				sys_trim(ARG_mstate_X_(m) 0);
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
			release_unused_segments(ARG_mstate_X(m));
	}
postaction:
	/* END: Copy-paste from `dlfree()' above */
	;
}

PRIVATE ATTR_NOINLINE NONNULL((1)) void
dl_freelist_do_reap(PARAM_mstate_m_ struct freelist_entry *__restrict flist) {
	struct freelist_entry *next;
	do {
		next = flist->fle_link.sle_next;
		dl_setfree_word(flist->fle_link.sle_next, struct freelist_entry *);
		dl_freelist_do_reap_item(ARG_mstate_m_ flist);
	} while ((flist = next) != NULL);
}
#endif /* NEED_dl_freelist_do_reap */

#if 1
#if !DIRECTLY_DEFINE_DEEMON_PUBLIC_API
static ATTR_MALLOC WUNUSED void *dlcalloc(size_t req)
#else /* !DIRECTLY_DEFINE_DEEMON_PUBLIC_API */
#define Dee_TryCalloc_DEFINED
PUBLIC ATTR_MALLOC WUNUSED void *(DCALL Dee_TryCalloc)(size_t req)
#endif /* DIRECTLY_DEFINE_DEEMON_PUBLIC_API */
{
	void *mem = dlmalloc(req);
	if (mem != 0 && calloc_must_clear(mem2chunk(mem))) {
		mchunkptr p = mem2chunk(mem);
		bzero(mem, chunksize(p) - overhead_for(p));
	}
	return mem;
}
#else
void *dlcalloc(size_t n_elements, size_t elem_size) {
	void *mem;
	size_t req = 0;
	if (n_elements != 0) {
		req = n_elements * elem_size;
		if (((n_elements | elem_size) & ~(size_t)0xffff) &&
		    (req / n_elements != elem_size))
			req = SIZE_MAX; /* force downstream failure on overflow */
	}
	mem = dlmalloc(req);
	if (mem != 0 && calloc_must_clear(mem2chunk(mem)))
		bzero(mem, req);
	return mem;
}
#endif

#endif /* !ONLY_MSPACES */

/* ------------ Internal support for realloc, memalign, etc -------------- */

/* Try to realloc; only in-place unless can_move true */
#undef try_realloc_chunk
#if FLAG4_BIT_INDICATES_HEAP_REGION
static mchunkptr try_realloc_chunk(PARAM_mstate_m_ mchunkptr p, size_t nb)
#define try_realloc_chunk(m, p, nb, can_move) try_realloc_chunk(ARG_mstate_X_(m) p, nb)
#else /* FLAG4_BIT_INDICATES_HEAP_REGION */
static mchunkptr try_realloc_chunk(PARAM_mstate_m_ mchunkptr p, size_t nb, int can_move)
#define try_realloc_chunk(m, p, nb, can_move) try_realloc_chunk(ARG_mstate_X_(m) p, nb, can_move)
#endif /* !FLAG4_BIT_INDICATES_HEAP_REGION */
{
	mchunkptr newp = 0;
	size_t oldsize = chunksize(p);
	mchunkptr next = chunk_plus_offset(p, oldsize);
#ifdef DL_DEBUG_MEMSET_ALLOC
	size_t oldsize_usable = oldsize - overhead_for(p);
#endif /* DL_DEBUG_MEMSET_ALLOC */
	ext_assert__ok_inuse(p);
	ext_assert__ok_next(p, next);
	ext_assert__ok_address(m, p);
	ext_assert__ok_pinuse(next);
#if !FLAG4_BIT_INDICATES_HEAP_REGION
	if (is_mmapped(p)) {
		newp = mmap_resize(m, p, nb, can_move);
	} else
#endif /* !FLAG4_BIT_INDICATES_HEAP_REGION */
	if (oldsize >= nb) { /* already big enough */
		size_t rsize = oldsize - nb;
		if (rsize >= MIN_CHUNK_SIZE) { /* split off remainder */
			mchunkptr r = chunk_plus_offset(p, nb);
			dl_setfree_data((char *)r + TWO_SIZE_T_SIZES, rsize - CHUNK_OVERHEAD);
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
				mstate_dv(m)     = 0;
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
#ifdef DL_DEBUG_MEMSET_ALLOC
	if (newp) {
		size_t newsize_usable = chunksize(newp) - overhead_for(newp);
		if (newsize_usable > oldsize_usable) {
			dl_setalloc_data((char *)chunk2mem(newp) + oldsize_usable,
			                 newsize_usable - oldsize_usable);
		}
	}
#endif /* DL_DEBUG_MEMSET_ALLOC */
	return newp;
}

static void *internal_memalign(PARAM_mstate_m_ size_t alignment, size_t bytes) {
	void *mem = 0;
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
		if (mem != 0) {
			mchunkptr p = mem2chunk(mem);
#if USE_PER_THREAD_MSTATE
			mstate fm = get_mstate_for(p);
#elif GM_ONLY
#define fm gm
#else /* ... */
#define fm m
#endif /* !... */
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
				char *br        = (char *)mem2chunk((size_t)(((size_t)((char *)mem + alignment - SIZE_T_ONE)) & ~(alignment - SIZE_T_ONE)));
				char *pos       = ((size_t)(br - (char *)(p)) >= MIN_CHUNK_SIZE) ? br : br + alignment;
				mchunkptr newp  = (mchunkptr)pos;
				size_t leadsize = pos - (char *)(p);
				size_t newsize  = chunksize(p) - leadsize;

				if (is_mmapped(p)) { /* For mmapped chunks, just adjust offset */
					newp->prev_foot = p->prev_foot + leadsize;
					newp->head      = newsize;
				} else { /* Otherwise, give back leader, use the rest */
					dl_setfree_data_untested((char *)p + TWO_SIZE_T_SIZES,
					                         leadsize - CHUNK_OVERHEAD);
					set_inuse(fm, newp, newsize);
					set_inuse(fm, p, leadsize);
					dispose_chunk(ARG_mstate_m_ p, leadsize);
				}
				p = newp;
			}

			/* Give back spare room at the end */
			if (!is_mmapped(p)) {
				size_t size = chunksize(p);
				if (size > nb + MIN_CHUNK_SIZE) {
					size_t remainder_size = size - nb;
					mchunkptr remainder   = chunk_plus_offset(p, nb);
					dl_setfree_data_untested((char *)remainder + TWO_SIZE_T_SIZES,
					                         remainder_size - CHUNK_OVERHEAD);
					set_inuse(fm, p, nb);
					set_inuse(fm, remainder, remainder_size);
					dispose_chunk(ARG_mstate_m_ remainder, remainder_size);
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
	if (chunks != 0) {
		if (n_elements == 0)
			return chunks; /* nothing to do */
		marray     = chunks;
		array_size = 0;
	} else {
		/* if empty req, must still return chunk representing empty array */
		if (n_elements == 0)
			return (void **)internal_malloc(m, 0);
		marray     = 0;
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
	if (mem == 0)
		return 0;

	PREACTION(m);
	p = mem2chunk(mem);
	remainder_size = chunksize(p);

	dl_assert(!is_mmapped(p));

	if (opts & 0x2) { /* optionally clear the elements */
		bzero(mem, remainder_size - SIZE_T_SIZE - array_size);
	}

	/* If not provided, allocate the pointer array as final part of chunk */
	if (marray == 0) {
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
		if (mem != 0) {
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
			*a = 0;
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
		for (s = &mstate_seg(m); s != 0; s = s->next) {
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
						start = (void *)((char *)q + sizeof(struct malloc_chunk));
					} else {
						start = (void *)((char *)q + sizeof(struct malloc_tree_chunk));
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

#if !DIRECTLY_DEFINE_DEEMON_PUBLIC_API
static WUNUSED void *dlrealloc(void *oldmem, size_t bytes)
#else /* !DIRECTLY_DEFINE_DEEMON_PUBLIC_API */
#define Dee_TryRealloc_DEFINED
PUBLIC WUNUSED void *(DCALL Dee_TryRealloc)(void *oldmem, size_t bytes)
#endif /* DIRECTLY_DEFINE_DEEMON_PUBLIC_API */
{
#ifndef DLREALLOC_CHECKS_NULL
#define DLREALLOC_CHECKS_NULL 1
#endif /* !DLREALLOC_CHECKS_NULL */
	void *mem = 0;
#if DLREALLOC_CHECKS_NULL
	if (oldmem == 0) {
#ifdef HOOK_AFTER_INIT_REALLOC
		ensure_initialization_for(HOOK_AFTER_INIT_REALLOC(oldmem, bytes));
#endif /* HOOK_AFTER_INIT_REALLOC */
		mem = dlmalloc(bytes);
	} else
#endif /* DLREALLOC_CHECKS_NULL */
	if (bytes >= MAX_REQUEST) {
		MALLOC_FAILURE_ACTION;
	} else
#ifdef REALLOC_ZERO_BYTES_FREES
	if (bytes == 0) {
		dlfree(oldmem);
	} else
#endif /* REALLOC_ZERO_BYTES_FREES */
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
#if FLAG4_BIT_INDICATES_HEAP_REGION
		if unlikely(is_mmapped(oldp)) {
			/* Don't realloc pointers from user-defined "struct Dee_heapregion" */
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
				if (fm != gm) {
					mem = dlmalloc(bytes);
					goto after_internal_malloc;
				} else {
					mspace mtls = tls_mspace();
					if (mtls) {
						mem = mspace_malloc(mtls, bytes);
						goto after_internal_malloc;
					}
				}
				PREACTION(fm);
			}
#else /* USE_PER_THREAD_MSTATE */
			PREACTION(fm);
#endif /* !USE_PER_THREAD_MSTATE */
			newp = mmap_resize(fm, oldp, nb, 1);
		} else
#endif /* FLAG4_BIT_INDICATES_HEAP_REGION */
		{
#if FOOTERS && !GM_ONLY
			fm = get_mstate_for(oldp);
			ext_assert__ok_magic(fm);
#endif /* FOOTERS && !GM_ONLY */
#if USE_PER_THREAD_MSTATE
			if (!TRY_PREACTION(fm)) {
				if (fm != gm) {
					mem = dlmalloc(bytes);
					goto after_internal_malloc;
				} else {
					mspace mtls = tls_mspace();
					if (mtls) {
						mem = mspace_malloc(mtls, bytes);
						goto after_internal_malloc;
					}
				}
				PREACTION(fm);
			}
#else /* USE_PER_THREAD_MSTATE */
			PREACTION(fm);
#endif /* !USE_PER_THREAD_MSTATE */
			newp = try_realloc_chunk(fm, oldp, nb, 1);
		}
		POSTACTION(fm);
		if (newp != 0) {
			check_inuse_chunk(fm, newp);
			mem = chunk2mem(newp);
		} else {
#if FLAG4_BIT_INDICATES_HEAP_REGION
fallback_internal_malloc:
#endif /* FLAG4_BIT_INDICATES_HEAP_REGION */
			mem = internal_malloc(fm, bytes);
#if USE_PER_THREAD_MSTATE
after_internal_malloc:
#endif /* USE_PER_THREAD_MSTATE */
			if (mem != 0) {
				size_t oc = chunksize(oldp) - overhead_for(oldp);
				mem = memcpy(mem, oldmem, (oc < bytes) ? oc : bytes);
				internal_free(fm, oldmem);
			}
		}
#undef fm
	}
	return mem;
}

#if !DIRECTLY_DEFINE_DEEMON_PUBLIC_API && LEAK_DETECTION_IN_TAIL
static WUNUSED void *dlrealloc_in_place(void *oldmem, size_t bytes)
#else /* LEAK_DETECTION_IN_TAIL */
#define Dee_TryReallocInPlace_DEFINED
PUBLIC WUNUSED void *(DCALL Dee_TryReallocInPlace)(void *oldmem, size_t bytes)
#endif /* !LEAK_DETECTION_IN_TAIL */
{
	void *mem = 0;
#ifndef DLREALLOC_IN_PLACE_CHECKS_NULL
#define DLREALLOC_IN_PLACE_CHECKS_NULL 1
#endif /* !DLREALLOC_IN_PLACE_CHECKS_NULL */
#if DLREALLOC_IN_PLACE_CHECKS_NULL
	if (oldmem == 0) {
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
#if FLAG4_BIT_INDICATES_HEAP_REGION
		if unlikely(is_mmapped(oldp)) {
			/* Don't realloc pointers from user-defined "struct Dee_heapregion" */
			if unlikely(flag4inuse(oldp))
				return NULL;
#if FOOTERS && !GM_ONLY
			fm = get_mstate_for(oldp);
			ext_assert__ok_magic(fm);
#endif /* FOOTERS && !GM_ONLY */
			PREACTION(fm);
			newp = mmap_resize(fm, oldp, nb, 0);
		} else
#endif /* FLAG4_BIT_INDICATES_HEAP_REGION */
		{
#if FOOTERS && !GM_ONLY
			fm = get_mstate_for(oldp);
			ext_assert__ok_magic(fm);
#endif /* FOOTERS && !GM_ONLY */
			PREACTION(fm);
			newp = try_realloc_chunk(fm, oldp, nb, 0);
		}
		POSTACTION(fm);
		dl_assert(newp == oldp || newp == 0);
		if (newp == oldp) {
			check_inuse_chunk(fm, newp);
			mem = oldmem;
		}
#undef fm
	}
	return mem;
}

#if !DIRECTLY_DEFINE_DEEMON_PUBLIC_API
static ATTR_MALLOC WUNUSED void *dlmemalign(size_t alignment, size_t bytes)
#else /* !DIRECTLY_DEFINE_DEEMON_PUBLIC_API */
#define Dee_TryMemalign_DEFINED
PUBLIC ATTR_MALLOC WUNUSED void *(DCALL Dee_TryMemalign)(size_t alignment, size_t bytes)
#endif /* DIRECTLY_DEFINE_DEEMON_PUBLIC_API */
{
#ifdef HOOK_AFTER_INIT_MEMALIGN
	ensure_initialization_for(HOOK_AFTER_INIT_MEMALIGN(alignment, bytes));
#endif /* HOOK_AFTER_INIT_MEMALIGN */
	if (alignment <= MALLOC_ALIGNMENT)
		return dlmalloc(bytes);
	return internal_memalign(ARG_mstate_gm_ alignment, bytes);
}

#if !NO_POSIX_MEMALIGN
static int dlposix_memalign(void **pp, size_t alignment, size_t bytes) {
	void *mem = 0;
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
	if (mem == 0)
		return ENOMEM;
	else {
		*pp = mem;
		return 0;
	}
}
#endif /* !NO_POSIX_MEMALIGN */

#if !NO_VALLOC
static void *dlvalloc(size_t bytes) {
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
static void *dlpvalloc(size_t bytes) {
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
static void **dlindependent_calloc(size_t n_elements, size_t elem_size, void *chunks[]) {
	size_t sz = elem_size; /* serves as 1-element array */
	return ialloc(ARG_mstate_gm_ n_elements, &sz, 3, chunks);
}

void **dlindependent_comalloc(size_t n_elements, size_t sizes[],
                              void *chunks[]) {
	return ialloc(ARG_mstate_gm_ n_elements, sizes, 0, chunks);
}
#endif /* !NO_INDEPENDENT_ALLOC */

#if !NO_BULK_FREE
static size_t dlbulk_free(void *array[], size_t nelem) {
	return internal_bulk_free(ARG_mstate_gm_ array, nelem);
}
#endif /* !NO_BULK_FREE */

#if MALLOC_INSPECT_ALL
static void dlmalloc_inspect_all(void (*handler)(void *start,
                                                 void *end,
                                                 size_t used_bytes,
                                                 void *callback_arg),
                                 void *arg) {
#if !GM_STATIC_INIT_MUTEX
	ensure_initialization();
#endif /* !GM_STATIC_INIT_MUTEX */
	PREACTION(gm);
	internal_inspect_all(ARG_mstate_gm_ handler, arg);
	POSTACTION(gm);
}
#endif /* MALLOC_INSPECT_ALL */

#if !NO_MALLOC_TRIM

PRIVATE size_t do_mspace_sys_trim(PARAM_mstate_m_ size_t pad) {
	size_t result;
	PREACTION(gm);
	result = sys_trim(ARG_mstate_m_ pad);
	POSTACTION(gm);
	return result;
}

#if USE_PER_THREAD_MSTATE
PRIVATE size_t do_mspace_sys_trim_cb(mspace ms, void *arg) {
	return do_mspace_sys_trim(ARG_mstate_X_((mstate)ms) (size_t)(uintptr_t)arg);
}
#endif /* USE_PER_THREAD_MSTATE */

#if !EXPOSE_AS_DEEMON_API
static int dlmalloc_trim(size_t pad)
#else /* !EXPOSE_AS_DEEMON_API */
PUBLIC size_t DCALL DeeHeap_Trim(size_t pad)
#endif /* EXPOSE_AS_DEEMON_API */
{
#if !EXPOSE_AS_DEEMON_API
	int result;
#else /* !EXPOSE_AS_DEEMON_API */
	size_t result;
#endif /* EXPOSE_AS_DEEMON_API */
#ifdef HOOK_AFTER_INIT_MALLOC_TRIM
	ensure_initialization_for(HOOK_AFTER_INIT_MALLOC_TRIM(pad));
#else /* HOOK_AFTER_INIT_MALLOC_TRIM */
#if !GM_STATIC_INIT_MUTEX
	ensure_initialization();
#endif /* !GM_STATIC_INIT_MUTEX */
#endif /* !HOOK_AFTER_INIT_MALLOC_TRIM */
	result = do_mspace_sys_trim(ARG_mstate_gm_ pad);

	/* Also trim every "tls_mspace()" (add sum from that to result) */
#if USE_PER_THREAD_MSTATE
	result += tls_mspace_foreach(&do_mspace_sys_trim_cb, (void *)(uintptr_t)pad);
#endif /* USE_PER_THREAD_MSTATE */

	return result;
}
#endif /* !NO_MALLOC_TRIM */

#if !NO_MALLOC_FOOTPRINT
#if USE_PER_THREAD_MSTATE
PRIVATE size_t do_mspace_footprint_cb(mspace ms, void *arg) {
	(void)arg;
	return atomic_read(&mstate_footprint((mstate)ms));
}

PRIVATE size_t do_mspace_max_footprint_cb(mspace ms, void *arg) {
	(void)arg;
	return atomic_read(&mstate_max_footprint((mstate)ms));
}

PRIVATE size_t do_mspace_set_footprint_limit_cb(mspace ms, void *arg) {
	size_t limit = (size_t)(uintptr_t)arg;
	atomic_write(&mstate_footprint_limit((mstate)ms), limit);
	return 0;
}
#endif /* USE_PER_THREAD_MSTATE */

#if !EXPOSE_AS_DEEMON_API
static size_t dlmalloc_footprint(void)
#else /* !EXPOSE_AS_DEEMON_API */
PUBLIC ATTR_PURE WUNUSED size_t DCALL DeeHeap_Footprint(void)
#endif /* EXPOSE_AS_DEEMON_API */
{
	size_t result = atomic_read(&gm__footprint);
	/* Include info from "tls_mspace()" (add sum from tls) */
#if USE_PER_THREAD_MSTATE
	result += tls_mspace_foreach(&do_mspace_footprint_cb, NULL);
#endif /* USE_PER_THREAD_MSTATE */
	return result;
}

#if !EXPOSE_AS_DEEMON_API
static size_t dlmalloc_max_footprint(void)
#else /* !EXPOSE_AS_DEEMON_API */
PUBLIC ATTR_PURE WUNUSED size_t DCALL DeeHeap_MaxFootprint(void)
#endif /* EXPOSE_AS_DEEMON_API */
{
	size_t result = atomic_read(&gm__max_footprint);
	/* Include info from "tls_mspace()" (add sum from tls) */
#if USE_PER_THREAD_MSTATE
	result += tls_mspace_foreach(&do_mspace_max_footprint_cb, NULL);
#endif /* USE_PER_THREAD_MSTATE */
	return result;
}

#if !EXPOSE_AS_DEEMON_API
static size_t dlmalloc_footprint_limit(void)
#else /* !EXPOSE_AS_DEEMON_API */
PUBLIC ATTR_PURE WUNUSED size_t DCALL DeeHeap_GetFootprintLimit(void)
#endif /* EXPOSE_AS_DEEMON_API */
{
	size_t maf = gm__footprint_limit;
	return maf == 0 ? SIZE_MAX : maf;
}

#if !EXPOSE_AS_DEEMON_API
static size_t dlmalloc_set_footprint_limit(size_t bytes)
#else /* !EXPOSE_AS_DEEMON_API */
PUBLIC size_t DCALL DeeHeap_SetFootprintLimit(size_t bytes)
#endif /* EXPOSE_AS_DEEMON_API */
{
	size_t result;
	size_t newval; /* invert sense of 0 */
	if (bytes == 0)
		newval = granularity_align(1); /* Use minimal size */
	if (bytes == SIZE_MAX)
		newval = 0; /* disable */
	else
		newval = granularity_align(bytes);
#if EXPOSE_AS_DEEMON_API
	result = atomic_xch(&gm__footprint_limit, newval);
#else /* EXPOSE_AS_DEEMON_API */
	atomic_write(&gm__footprint_limit, newval);
	result = newval;
#endif /* !EXPOSE_AS_DEEMON_API */

	/* Also set value in every "tls_mspace()" (set the same value everywhere) */
#if USE_PER_THREAD_MSTATE
	tls_mspace_foreach(&do_mspace_set_footprint_limit_cb, (void *)(uintptr_t)newval);
#endif /* USE_PER_THREAD_MSTATE */

	return result;
}
#endif /* !NO_MALLOC_FOOTPRINT */

#if !NO_MALLINFO && !EXPOSE_AS_DEEMON_API
static struct dlmalloc_mallinfo dlmallinfo(void) {
	return internal_mallinfo(ARG_mstate_gm);
}
#endif /* NO_MALLINFO && !EXPOSE_AS_DEEMON_API */

#if !NO_MALLOC_STATS
static void dlmalloc_stats() {
	internal_malloc_stats(ARG_mstate_gm);
}
#endif /* NO_MALLOC_STATS */

#if !NO_MALLOPT && !EXPOSE_AS_DEEMON_API
static int dlmallopt(int param_number, int value) {
	return change_mparam(param_number, value);
}
#endif /* !NO_MALLOPT && !EXPOSE_AS_DEEMON_API */
#endif /* !ONLY_MSPACES */

/* ----------------------------- user mspaces ---------------------------- */

#if MSPACES

static mstate init_user_mstate(char *tbase, size_t tsize) {
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
#ifdef mstate_extp
	mstate_extp(m) = 0;
#endif /* mstate_extp */
#ifdef mstate_exts
	mstate_exts(m) = 0;
#endif /* mstate_exts */
	disable_contiguous(m);
	init_bins(m);
	mn = next_chunk(mem2chunk(m));
	init_top(m, mn, (size_t)((tbase + tsize) - (char *)mn) - TOP_FOOT_SIZE);
	check_top_chunk(m, mstate_top(m));
	return m;
}

static mspace create_mspace(size_t capacity/*, int locked*/) {
	mstate m = 0;
	size_t msize;
	ensure_initialization();
	msize = pad_request(sizeof(struct malloc_state));
	if (capacity < (size_t)0 - (size_t)(msize + TOP_FOOT_SIZE + malloc_pagesize)) {
		size_t rs    = ((capacity == 0) ? mparams.granularity : (capacity + TOP_FOOT_SIZE + msize));
		size_t tsize = granularity_align(rs);
		char *tbase  = (char *)(DL_MMAP(tsize));
		if (tbase != CMFAIL) {
			m = init_user_mstate(tbase, tsize);
			mstate_seg(m).sflags = USE_MMAP_BIT;
		}
	}
	return (mspace)m;
}

#if !NO_CREATE_MSPACE_WITH_BASE
static mspace create_mspace_with_base(void *base, size_t capacity/*, int locked*/) {
	mstate m = 0;
	size_t msize;
	ensure_initialization();
	msize = pad_request(sizeof(struct malloc_state));
	if (capacity > msize + TOP_FOOT_SIZE &&
	    capacity < (size_t)0 - (size_t)(msize + TOP_FOOT_SIZE + malloc_pagesize)) {
		m = init_user_mstate((char *)base, capacity);
		mstate_seg(m).sflags = EXTERN_BIT;
	}
	return (mspace)m;
}
#endif /* !NO_CREATE_MSPACE_WITH_BASE */

#if !NO_MSPACE_TRACK_LARGE_CHUNKS
static int mspace_track_large_chunks(mspace msp, int enable) {
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
static size_t destroy_mspace(mspace msp) {
	size_t freed = 0;
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
#ifndef DL_MUNMAP_ALWAYS_FAILS
	msegmentptr sp = &ms->seg;
	while (sp != 0) {
		char *base  = sp->base;
		size_t size = sp->size;
		flag_t flag = sp->sflags;
		(void)base; /* placate people compiling -Wunused-variable */
		sp = sp->next;
#ifdef EXTERN_BIT
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
PRIVATE struct malloc_state_slist free_tls_mspace = SLIST_HEAD_INITIALIZER(free_tls_mspace);
PRIVATE struct malloc_state_slist used_tls_mspace = SLIST_HEAD_INITIALIZER(used_tls_mspace);
#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_lock_t tls_mspace_lock = Dee_ATOMIC_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define tls_mspace_lock_available()  Dee_atomic_lock_available(&tls_mspace_lock)
#define tls_mspace_lock_acquired()   Dee_atomic_lock_acquired(&tls_mspace_lock)
#define tls_mspace_lock_tryacquire() Dee_atomic_lock_tryacquire(&tls_mspace_lock)
#define tls_mspace_lock_acquire()    Dee_atomic_lock_acquire(&tls_mspace_lock)
#define tls_mspace_lock_waitfor()    Dee_atomic_lock_waitfor(&tls_mspace_lock)
#define tls_mspace_lock_release()    Dee_atomic_lock_release(&tls_mspace_lock)

PRIVATE WUNUSED mspace DCALL create_tls_mspace(void) {
	struct malloc_state *result;
	tls_mspace_lock_acquire();
	if (!SLIST_EMPTY(&free_tls_mspace)) {
		result = SLIST_FIRST(&free_tls_mspace);
		SLIST_REMOVE_HEAD(&free_tls_mspace, ms_link);
	} else {
		result = (struct malloc_state *)create_mspace(0);
	}
	if likely(result)
		SLIST_INSERT(&used_tls_mspace, result, ms_link);
	tls_mspace_lock_release();
	return (mspace)result;
}

PRIVATE NONNULL((1)) void DCALL destroy_tls_mspace(mspace ms) {
	struct malloc_state *state = (struct malloc_state *)ms;
	tls_mspace_lock_acquire();
	SLIST_REMOVE(&used_tls_mspace, state, malloc_state, ms_link);
	SLIST_INSERT(&free_tls_mspace, state, ms_link);
	tls_mspace_lock_release();
}

/* Return the calling thread's thread-local mspace (or "0" if not available) */
static mspace tls_mspace(void) {
	DeeThreadObject *me = DeeThread_Self();
	mspace result = (mspace)me->t_heap;
	if unlikely(!result) {
		result = create_tls_mspace();
		me->t_heap = (void *)result;
	}
	return result;
}

#ifndef CONFIG_NO_THREADS
PRIVATE Dee_rshared_lock_t foreach_lock = Dee_RSHARED_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */

PRIVATE uintptr_t foreach_bitset_inuse = 0;
PRIVATE uintptr_t foreach_bitset_inuse_alloc(void) {
	unsigned int i;
	uintptr_t result, state;
again:
	state = atomic_read(&foreach_bitset_inuse);
	for (i = 0, result = 1; i < (sizeof(state) * __CHAR_BIT__); ++i, result <<= 1) {
		if (!(state & result)) {
			if (!(atomic_fetchor(&foreach_bitset_inuse, result) & result))
				return result;
			goto again;
		}
	}
	return 0;
}

PRIVATE void foreach_bitset_inuse_free(uintptr_t mask) {
	atomic_and(&foreach_bitset_inuse, ~mask);
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

static size_t tls_mspace_foreach(size_t (*cb)(mspace ms, void *arg), void *arg) {
	uintptr_t mask;
	mstate ms;
	size_t result = 0;
	Dee_rshared_lock_acquire_noint(&foreach_lock);
	mask = foreach_bitset_inuse_alloc();
	tls_mspace_lock_acquire();
	reset_not_visited_tls_mspace(mask);
	while ((ms = find_not_visited_tls_mspace(mask)) != NULL) {
		tls_mspace_lock_release();
		result += (*cb)((mspace)ms, arg);
		tls_mspace_lock_acquire();
	}
	tls_mspace_lock_release();
	foreach_bitset_inuse_free(mask);
	Dee_rshared_lock_release(&foreach_lock);
	return result;
}

#define thread_heap_destroy_DEFINED
INTERN NONNULL((1)) void DCALL
thread_heap_destroy(void *heap) {
	destroy_tls_mspace((mspace)heap);
}
#endif /* USE_PER_THREAD_MSTATE */


/*
  mspace versions of routines are near-clones of the global
  versions. This is not so nice but better than the alternatives.
*/

static void *mspace_malloc(mspace msp, size_t bytes) {
	void *mem;
	size_t nb;
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
#ifdef MALLOC_ZERO_RETURNS_NULL
	if unlikely(!bytes)
		return NULL;
#endif /* MALLOC_ZERO_RETURNS_NULL */
	PREACTION(ms);
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

			else if (ms->treemap != 0 && (mem = tmalloc_small(ms, nb)) != 0) {
				check_malloced_chunk(ms, mem, nb);
				goto postaction;
			}
		}
	} else if (bytes >= MAX_REQUEST)
		nb = SIZE_MAX; /* Too big to allocate. Force failure (in sys alloc) */
	else {
		nb = pad_request(bytes);
		if (ms->treemap != 0 && (mem = tmalloc_large(ms, nb)) != 0) {
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
			ms->dv     = 0;
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
	POSTACTION(ms);
	return mem;
}

#if !NO_MSPACE_FREE
static void mspace_free(mspace msp, void *mem) {
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
#ifndef DL_MUNMAP_ALWAYS_FAILS
			psize += prevsize + MMAP_FOOT_PAD;
			if (DL_MUNMAP((char *)p - prevsize, psize) == 0)
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
				mstate_dv(fm)     = 0;
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
static void *mspace_calloc(mspace msp, size_t n_elements, size_t elem_size) {
	void *mem;
	size_t req = 0;
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	if (n_elements != 0) {
		req = n_elements * elem_size;
		if (((n_elements | elem_size) & ~(size_t)0xffff) &&
		    (req / n_elements != elem_size))
			req = SIZE_MAX; /* force downstream failure on overflow */
	}
	mem = internal_malloc(ms, req);
	if (mem != 0 && calloc_must_clear(mem2chunk(mem)))
		bzero(mem, req);
	return mem;
}
#endif /* !NO_MSPACE_CALLOC */

#if !NO_MSPACE_REALLOC
static void *mspace_realloc(mspace msp, void *oldmem, size_t bytes) {
	void *mem = 0;
	if (oldmem == 0) {
		mem = mspace_malloc(msp, bytes);
	} else if (bytes >= MAX_REQUEST) {
		MALLOC_FAILURE_ACTION;
	}
#ifdef REALLOC_ZERO_BYTES_FREES
	else if (bytes == 0) {
		mspace_free(msp, oldmem);
	}
#endif /* REALLOC_ZERO_BYTES_FREES */
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
		if (newp != 0) {
			check_inuse_chunk(m, newp);
			mem = chunk2mem(newp);
		} else {
			mem = mspace_malloc(m, bytes);
			if (mem != 0) {
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
static void *mspace_realloc_in_place(mspace msp, void *oldmem, size_t bytes) {
	void *mem = 0;
	if (oldmem != 0) {
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
static void *mspace_memalign(mspace msp, size_t alignment, size_t bytes) {
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	if (alignment <= MALLOC_ALIGNMENT)
		return mspace_malloc(msp, bytes);
	return internal_memalign(ms, alignment, bytes);
}
#endif /* !NO_MSPACE_MEMALIGN */

#if !NO_INDEPENDENT_ALLOC
static void **mspace_independent_calloc(mspace msp, size_t n_elements,
                                        size_t elem_size, void *chunks[]) {
	size_t sz = elem_size; /* serves as 1-element array */
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	return ialloc(ms, n_elements, &sz, 3, chunks);
}

static void **mspace_independent_comalloc(mspace msp, size_t n_elements,
                                          size_t sizes[], void *chunks[]) {
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	return ialloc(ms, n_elements, sizes, 0, chunks);
}
#endif /* !NO_INDEPENDENT_ALLOC */

#if !NO_BULK_FREE
static size_t mspace_bulk_free(mspace msp, void *array[], size_t nelem) {
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
static int mspace_trim(mspace msp, size_t pad) {
	int result = 0;
	mstate ms  = (mstate)msp;
	ext_assert__ok_magic(ms);
	PREACTION(ms);
	result = sys_trim(ms, pad);
	POSTACTION(ms);
	return result;
}
#endif /* NO_MSPACE_TRIM */

#if !NO_MALLOC_STATS
static void mspace_malloc_stats(mspace msp) {
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	internal_malloc_stats(ms);
}
#endif /* NO_MALLOC_STATS */

#if !NO_MSPACE_FOOTPRINT
static size_t mspace_footprint(mspace msp) {
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	return ms->footprint;
}
#endif /* !NO_MSPACE_FOOTPRINT */

#if !NO_MSPACE_MAX_FOOTPRINT
static size_t mspace_max_footprint(mspace msp) {
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	return ms->max_footprint;
}
#endif /* !NO_MSPACE_MAX_FOOTPRINT */

#if !NO_MSPACE_FOOTPRINT_LIMIT
static size_t mspace_footprint_limit(mspace msp) {
	size_t maf;
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	maf = ms->footprint_limit;
	return (maf == 0) ? SIZE_MAX : maf;
}
#endif /* !NO_MSPACE_FOOTPRINT_LIMIT */

#if !NO_MSPACE_SET_FOOTPRINT_LIMIT
static size_t mspace_set_footprint_limit(mspace msp, size_t bytes) {
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

#if !NO_MALLINFO && !EXPOSE_AS_DEEMON_API
static struct dlmalloc_mallinfo mspace_mallinfo(mspace msp) {
	mstate ms = (mstate)msp;
	ext_assert__ok_magic(ms);
	return internal_mallinfo(ms);
}
#endif /* NO_MALLINFO && !EXPOSE_AS_DEEMON_API */

#if !NO_MSPACE_USABLE_SIZE
static size_t mspace_usable_size(void const *mem) {
	if (mem != 0) {
		mchunkptr p = mem2chunk(mem);
		if (is_inuse(p))
			return chunksize(p) - overhead_for(p);
	}
	return 0;
}
#endif /* !NO_MSPACE_USABLE_SIZE */

#if !NO_MALLOPT && !EXPOSE_AS_DEEMON_API
static int mspace_mallopt(int param_number, int value) {
	return change_mparam(param_number, value);
}
#endif /* !NO_MALLOPT && !EXPOSE_AS_DEEMON_API */
#endif /* MSPACES */

/************************************************************************/
/************************************************************************/
/************************************************************************/
/*                                                                      */
/* END OF DLMALLOC                                                      */
/*                                                                      */
/************************************************************************/
/************************************************************************/
/************************************************************************/






/* Deemon public heap API */
#if !DIRECTLY_DEFINE_DEEMON_PUBLIC_API
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL Dee_TryMalloc)(size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL Dee_TryCalloc)(size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL Dee_TryRealloc)(void *ptr, size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL Dee_TryReallocInPlace)(void *ptr, size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL Dee_TryMemalign)(size_t min_alignment, size_t n_bytes);
DFUNDEF ATTR_PURE WUNUSED size_t (DCALL Dee_MallocUsableSize)(void *ptr);
DFUNDEF void (DCALL Dee_Free)(void *ptr);
#endif /* !DIRECTLY_DEFINE_DEEMON_PUBLIC_API */
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbg_TryMalloc)(size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbg_TryCalloc)(size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED void *(DCALL DeeDbg_TryRealloc)(void *ptr, size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbg_TryMemalign)(size_t min_alignment, size_t n_bytes, char const *file, int line);

DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL Dee_Malloc)(size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL Dee_Calloc)(size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL Dee_Realloc)(void *ptr, size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL Dee_Memalign)(size_t min_alignment, size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbg_Malloc)(size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbg_Calloc)(size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED void *(DCALL DeeDbg_Realloc)(void *ptr, size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbg_Memalign)(size_t min_alignment, size_t n_bytes, char const *file, int line);


#if !DIRECTLY_DEFINE_DEEMON_PUBLIC_API

/* Memory leak detector:
 * - Built directory on-top of dlmalloc(), dlrealloc(), dlfree() (meaning no
 *   extra dependencies on any internals of the underlying malloc implementation)
 * - No extra headers added to- or required for heap chunks allocated by underlying
 *   memory allocator.
 */
#if LEAK_DETECTION && LEAK_DETECTION_IN_TAIL

/* Alternative implementation to the leak detector below, that:
 * - Stores debug info in-band, by
 * - overriding dlmalloc's heap tail
 *
 * ------------------------------------------------------------------------
 *
 * Force-enable `FOOTERS', thus causing dlmalloc to always make space for
 * a pointer-sized field at the tail of an malloc chunk (the address of
 * this tail can easily be calculated as it is `p + Dee_MallocUsableSize(p)').
 * - Then, allocate another, small heap block that will contain the debug info
 *   needed to track the pointer as a potential memory leak. This struct has
 *   a fixed length (meaning it can also have a dedicated slab-style cache),
 *   and looks like this:
 *   >> struct leak_footer {
 *   >>     size_t              lf_foot;  // Original value of the chunk's footer
 *   >>     void               *lf_chunk; // [1..1] Pointer, as returned by dlmalloc() for linked memory
 *   >>     struct leak_footer *lf_prev;  // [1..1] Prev tracked heap chunk
 *   >>     struct leak_footer *lf_next;  // [1..1] Next tracked heap chunk
 *   >>     char const         *lf_file;  // [0..1] File where "lf_chunk" was allocated
 *   >>     unsigned int        lf_line;  // Line where "lf_chunk" was allocated
 *   >> };
 * - Dee_Malloc(size) is then implemented like (simplified):
 *   >> void *result = dlmalloc(size);
 *   >> struct leak_footer *foot = dlmalloc(sizeof(struct leak_footer));
 *   >> size_t usable = Dee_MallocUsableSize(result);
 *   >> size_t *p_foot = (size_t *)((char *)result + usable);
 *   >> foot->lf_foot = *p_foot;
 *   >> *p_foot = (size_t)(uintptr_t)foot;
 *   >> foot->lf_chunk = result;
 *   >> foot->lf_file = __FILE__;
 *   >> foot->lf_line = __LINE__;
 *   >> // When this blocks, have an ATOMIC_SLIST of pending inserts (the
 *   >> // "foot->lf_prev" field can be re-used as linked list pointer)
 *   >> LOCK_TRACKED_LEAKS();
 *   >> INSERT_INTO_GLOBAL_LIST_OF_LEAKS(foot);
 *   >> UNLOCK_TRACKED_LEAKS();
 *   >> return result;
 * - Dee_Free(ptr) is then implemented like (simplified):
 *   >> mchunkptr p = mem2chunk(ptr);
 *   >> if (!flag4inuse(p)) {
 *   >>     size_t *p_foot = (size_t *)chunk_plus_offset(p, chunksize(p) - sizeof(size_t));
 *   >>     struct leak_footer *foot = (struct leak_footer *)(uintptr_t)(*p_foot);
 *   >>     // When this blocks, have an ATOMIC_SLIST of pending removes (the
 *   >>     // "foot->lf_file" field can be re-used as linked list pointer)
 *   >>     LOCK_TRACKED_LEAKS();
 *   >>     REMOVE_FROM_GLOBAL_LIST_OF_LEAKS(foot);
 *   >>     UNLOCK_TRACKED_LEAKS();
 *   >>     *p_foot = foot->lf_foot; // Restore, so dlfree() doesn't break
 *   >>     dlfree(foot);
 *   >> }
 *   >> dlfree(ptr);
 *
 * - NOTE: I've already checked, and since dlmalloc has no way of tracking currently
 *         allocated heap chunks, so-long as the original value is always restored,
 *         there is no harm in temporarily overriding one's own chunk footer:
 *   >> void *a = Dee_Malloc(0);
 *   >> void *p = Dee_Malloc(16);
 *   >> void *c = Dee_Malloc(0);
 *   >> size_t s = Dee_MallocUsableSize(p);
 *   >> void *tail = (char *)p + s;
 *   >> size_t tail_ptr = *(size_t *)tail;
 *   >> *(size_t *)tail = 42;
 *   >> Dee_CHECKMEMORY();
 *   >> Dee_Free(a);
 *   >> Dee_CHECKMEMORY();
 *   >> Dee_Free(c);
 *   >> Dee_CHECKMEMORY();                   // This never fails (under FOOTERS=1), even though our "tail" is corrupt
 *   >> *(size_t *)tail = tail_ptr;          // ... but without this, the following Dee_Free() *would* fail
 *   >> Dee_Free(p);
 */

struct leak_footer {
	union {
		size_t            lf_foot;  /* [lock(OWNER(lf_chunk))] Original value of the chunk's footer */
		SLIST_ENTRY(leak_footer) lf_unused; /* Used internally for cache of unused footers */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion1
#define lf_foot   _dee_aunion1.lf_foot
#define lf_unused _dee_aunion1.lf_unused
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	void                 *lf_chunk; /* [1..1][lock(OWNER(lf_chunk))] Pointer, as returned by dlmalloc() for linked memory */
	struct leak_footer   *lf_prev;  /* [1..1][lock(leaks_lock)] Prev tracked heap chunk (forming a ring) */
	union {
		struct leak_footer *lf_next;  /* [1..1][lock(leaks_lock)]  Next tracked heap chunk (forming a ring) */
		SLIST_ENTRY(leak_footer) lf_inslink; /* Used internally for async insertion */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion2
#define lf_next    _dee_aunion2.lf_next
#define lf_inslink _dee_aunion2.lf_inslink
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	union {
		char const       *lf_file;  /* [0..1][lock(OWNER(lf_chunk))] File where "lf_chunk" was allocated */
		SLIST_ENTRY(leak_footer) lf_remlink; /* Used internally for async removal */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion3
#define lf_file    _dee_aunion3.lf_file
#define lf_remlink _dee_aunion3.lf_remlink
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
	unsigned int          lf_line;  /* [lock(OWNER(lf_chunk))] Line where "lf_chunk" was allocated */
	size_t                lf_id;    /* Allocation ID (I hate that this field prevents this thing from aligning nicely) */
};

/* Return pointer to "prev_foot" field of next chunk (i.e. the "footer" field of this chunk) */
#define chunk_p_foot(X) ((size_t *)chunk_plus_offset(X, chunksize(X)))

SLIST_HEAD(leak_footer_slist, leak_footer);

//TODO:PRIVATE struct leak_footer_slist leak_footer_freelist = SLIST_HEAD_INITIALIZER(leak_footer_freelist);

PRIVATE ATTR_MALLOC WUNUSED struct leak_footer *DCALL
leak_footer_alloc(void) {
	/* TODO: Use cache */
	return (struct leak_footer *)dlmalloc(sizeof(struct leak_footer));
}

PRIVATE NONNULL((1)) void DCALL
leak_footer_free(struct leak_footer *__restrict self) {
	/* TODO: Use cache */
	dlfree(self);
}

PRIVATE size_t alloc_id_count = 0;
PRIVATE size_t alloc_id_break = 0;

/* Get/set the memory allocation breakpoint.
 * - When the deemon heap was built to track memory leaks, an optional
 *   allocation breakpoint can be defined which, when reached, causes
 *   an attached debugger to break, allowing you to inspect the stack
 *   at the point where the `id'th allocation happened
 * - Allocation IDs are assigned in ascending order during every call
 *   to Dee_Malloc(), Dee_Calloc() and Dee_Realloc() (when ptr==NULL),
 *   as well as their Dee_Try* equivalents.
 * - When the deemon heap was not built with this feature, this API
 *   is a no-op, and always returns `0'
 * @return: * : The previously set allocation breakpoint */
#define DeeHeap_GetAllocBreakpoint_DEFINED
PUBLIC ATTR_COLD ATTR_PURE WUNUSED size_t DCALL
DeeHeap_GetAllocBreakpoint(void) {
	return atomic_read(&alloc_id_break);
}

PUBLIC ATTR_COLD size_t DCALL
DeeHeap_SetAllocBreakpoint(size_t id) {
	return atomic_xch(&alloc_id_break, id);
}


#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_lock_t leaks_lock = Dee_ATOMIC_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define leaks_lock_available()  Dee_atomic_lock_available(&leaks_lock)
#define leaks_lock_acquired()   Dee_atomic_lock_acquired(&leaks_lock)
#define leaks_lock_tryacquire() Dee_atomic_lock_tryacquire(&leaks_lock)
#define leaks_lock_acquire()    Dee_atomic_lock_acquire(&leaks_lock)
#define leaks_lock_waitfor()    Dee_atomic_lock_waitfor(&leaks_lock)
#define _leaks_lock_release()   Dee_atomic_lock_release(&leaks_lock)

/* [lock(leaks_lock)] Ring of memory leaks */
PRIVATE struct leak_footer leaks = {
	/* .lf_foot  = */ { 0 },      /* [const] */
	/* .lf_chunk = */ NULL,       /* [const] */
	/* .lf_prev  = */ &leaks,     /* [1..1][lock(leaks_lock)] */
	/* .lf_next  = */ { &leaks }, /* [1..1][lock(leaks_lock)] */
	/* .lf_file  = */ { NULL },   /* [const] */
	/* .lf_line  = */ 0,          /* [const] */
	/* .lf_id    = */ 0           /* [const] */
};

#define leaks_assert_linkage(leak)               \
	(ASSERT((leak)->lf_prev->lf_next == (leak)), \
	 ASSERT((leak)->lf_next->lf_prev == (leak)))

#define leaks_remove(leak)                             \
	(void)(leaks_assert_linkage(leak),                 \
	       (leak)->lf_prev->lf_next = (leak)->lf_next, \
	       (leak)->lf_next->lf_prev = (leak)->lf_prev)
#if 1
#define leaks_insert_begin(leak)         (void)((leak)->lf_prev = NULL)
#define leaks_insert_is_unfinished(leak) (atomic_read(&(leak)->lf_prev) == NULL)
#define leaks_insert_finish(leak)                                          \
	(void)(ASSERTF((leak)->lf_prev == NULL,                                \
	               "Should have been done by leaks_insert_begin()"),       \
	       (leak)->lf_prev = &leaks,                            /* 1 */    \
	       ((leak)->lf_next = leaks.lf_next)->lf_prev = (leak), /* 2, 3 */ \
	       leaks.lf_next = (leak), leaks_assert_linkage(leak))  /* 4 */
#else
#define leaks_insert_begin(leak) \
	(void)((leak)->lf_prev = &leaks) /* 1 */
#define leaks_insert_finish(leak)                                               \
	(void)(ASSERTF((leak)->lf_prev == &leaks,                                   \
	               "Should have been done by leaks_insert_begin()"), /* 1 */    \
	       ((leak)->lf_next = leaks.lf_next)->lf_prev = (leak),      /* 2, 3 */ \
	       leaks.lf_next = (leak), leaks_assert_linkage(leak))                                   /* 4 */
#endif
	


PRIVATE struct leak_footer_slist leaks_pending_insert = SLIST_HEAD_INITIALIZER(leaks_pending_insert);
PRIVATE struct leak_footer_slist leaks_pending_remove = SLIST_HEAD_INITIALIZER(leaks_pending_remove);

#define leaks_pending_mustreap()                             \
	(atomic_read(&leaks_pending_insert.slh_first) != NULL || \
	 atomic_read(&leaks_pending_remove.slh_first) != NULL)
#define leaks_pending_mustreap_insert() \
	(atomic_read(&leaks_pending_insert.slh_first) != NULL)

/* Reap "leaks_pending_insert" only */
PRIVATE void DCALL leaks_lock_reap_insert_locked(void) {
	struct leak_footer_slist insert;
	struct leak_footer *leak;
	insert.slh_first = SLIST_ATOMIC_CLEAR(&leaks_pending_insert);
	while (!SLIST_EMPTY(&insert)) {
		leak = SLIST_FIRST(&insert);
		SLIST_REMOVE_HEAD(&insert, lf_inslink);
		leaks_insert_finish(leak);
//		leaks_assert_linkage(leak);
	}
}

PRIVATE ATTR_NOINLINE void DCALL leaks_lock_reap_and_unlock(void) {
	struct leak_footer_slist remove;
	struct leak_footer *leak;

	/* Order here is important: must reap "remove" **BEFORE** "insert"!
	 *
	 * In case a pointer ends up added to both lists, it is guarantied
	 * that if it appears in "remove", it will either:
	 * - Already be within "leaks"
	 * - or: have been added to "insert" previously
	 *
	 * Combine that with is executing "insert" first, the removal code
	 * is allowed to assume that everything from "remove" will currently
	 * be present somewhere within "leaks"! */
	remove.slh_first = SLIST_ATOMIC_CLEAR(&leaks_pending_remove);

	/* Execute "insert" tasks */
	leaks_lock_reap_insert_locked();

	/* Execute "remove" tasks */
	SLIST_FOREACH (leak, &remove, lf_remlink) {
		leaks_remove(leak);
	}

	/* Release lock */
	_leaks_lock_release();

	/* Free underlying memory blocks from "remove" */
	while (!SLIST_EMPTY(&remove)) {
		void *mem;
		mchunkptr p;
		size_t *p_foot;
		leak = SLIST_FIRST(&remove);
		SLIST_REMOVE_HEAD(&remove, lf_remlink);
		mem    = leak->lf_chunk;
		p      = mem2chunk(mem);
		p_foot = chunk_p_foot(p);
		*p_foot = leak->lf_foot; /* Restore original footer */
		leak_footer_free(leak);
		dlfree(mem);
	}
}


PRIVATE void DCALL leaks_lock_release(void) {
#ifdef __OPTIMIZE_SIZE__
	if (0)
#else /* __OPTIMIZE_SIZE__ */
	if (leaks_pending_mustreap())
#endif /* !__OPTIMIZE_SIZE__ */
	{
again_reap:
		leaks_lock_reap_and_unlock();
	} else {
		_leaks_lock_release();
	}
	/* Check if more needs to be reaped... */
	if (leaks_pending_mustreap()) {
		if (leaks_lock_tryacquire())
			goto again_reap;
	}
}

PRIVATE void DCALL leaks_lock_reap(void) {
	if (leaks_lock_tryacquire())
		leaks_lock_reap_and_unlock();
}


PRIVATE void DCALL leaks_lock_acquire_and_reap(void) {
	leaks_lock_acquire();
	if (leaks_pending_mustreap()) {
		leaks_lock_reap_and_unlock();
		leaks_lock_acquire();
	}
}

/* Dump info about all heap allocations that were allocated, but
 * never Dee_Free()'d, nor untracked using `Dee_UntrackAlloc()'.
 * Information about leaks is printed using `Dee_DPRINTF()'.
 *
 * @return: * : The total amount of memory leaked (in bytes) */
#define DeeHeap_DumpMemoryLeaks_DEFINED
PUBLIC size_t DCALL DeeHeap_DumpMemoryLeaks(void) {
	size_t result = 0;
	struct leak_footer *iter;
	leaks_lock_acquire_and_reap();
	for (iter = leaks.lf_next; iter != &leaks; iter = iter->lf_next) {
		void *mem = atomic_read(&iter->lf_chunk);
		mchunkptr p = mem2chunk(mem);
		size_t size = chunksize(p) - overhead_for(p);
		char const *file = atomic_read(&iter->lf_file);
		/* FIXME: If "iter" was added to "leaks_pending_remove", then the "file" we must read is invalid!
		 * Solution is: reap "leaks_pending_remove" after reading "lf_file" (iow: right here),
		 *              but don't release lock and keep skip ahead to next chunk when "iter"
		 *              ends up being removed via "leaks_pending_remove" */
		Dee_DPRINTF("%s(%d) : %" PRFuSIZ " : Leaked %" PRFuSIZ " bytes of memory: %p-%p\n",
		            file, iter->lf_line, iter->lf_id, size, (char *)mem, (char *)mem + size - 1);
		result += size;
	}
	return result;
}






PRIVATE NONNULL((1, 2)) void DCALL
leak_insert_p(void *ptr, struct leak_footer *leak,
              char const *file, unsigned int line) {
	/* Hook "leak" into "ptr" */
	mchunkptr p = mem2chunk(ptr);
	size_t *p_tail = chunk_p_foot(p);
	leak->lf_foot  = *p_tail;
	leak->lf_chunk = ptr;
	leak->lf_file  = file;
	leak->lf_line  = line;
	*p_tail = (size_t)(uintptr_t)leak;

	/* Allocate ID */
	leak->lf_id = atomic_incfetch(&alloc_id_count);
	if (leak->lf_id == alloc_id_break)
		Dee_BREAKPOINT();

	/* Insert into list of memory leaks (without ever blocking) */
	leaks_insert_begin(leak);
	if (leaks_lock_tryacquire()) {
		leaks_insert_finish(leak);
		leaks_lock_release();
	} else {
		/* Schedule "insert" to happen asynchronously */
		SLIST_ATOMIC_INSERT(&leaks_pending_insert, leak, lf_inslink);
		leaks_lock_reap();
	}
}

/* Amend debug into to "ptr" and re-return. On error, dlfree(ptr) and return "NULL" */
PRIVATE WUNUSED NONNULL((1)) void *DCALL
leak_insert(void *ptr, char const *file, unsigned int line) {
	struct leak_footer *foot = leak_footer_alloc();
	if likely(foot) {
		leak_insert_p(ptr, foot, file, line);
		return ptr;
	}
	Dee_DPRINTF("[RT] Failed to allocate 'leak_footer' for %" PRFuSIZ "-byte large heap pointer %p\n",
	            Dee_MallocUsableSize(ptr), ptr);
	dlfree(ptr);
	return NULL;
}


#define DeeDbg_TryMalloc_DEFINED
PUBLIC ATTR_HOT ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryMalloc)(size_t n_bytes, char const *file, int line) {
	void *result = dlmalloc(n_bytes);
	if (result)
		result = leak_insert(result, file, line);
	return result;
}

#define DeeDbg_TryCalloc_DEFINED
PUBLIC ATTR_HOT ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryCalloc)(size_t n_bytes, char const *file, int line) {
	void *result = dlcalloc(n_bytes);
	if (result)
		result = leak_insert(result, file, line);
	return result;
}

#define DeeDbg_TryMemalign_DEFINED
PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryMemalign)(size_t min_alignment, size_t n_bytes,
                           char const *file, int line) {
	void *result = dlmemalign(min_alignment, n_bytes);
	if (result)
		result = leak_insert(result, file, line);
	return result;
}


#ifdef Dee_BREAKPOINT_IS_NOOP
#define fatal _DeeAssert_XFailf
#else /* Dee_BREAKPOINT_IS_NOOP */
#define fatal(expr, file, line, ...) \
	(_DeeAssert_Failf(expr, file, line, __VA_ARGS__), Dee_BREAKPOINT())
#endif /* !Dee_BREAKPOINT_IS_NOOP */

#define leak_addrok(p) ((uintptr_t)(p) != 0) /* TODO: Exclude more known-non-sense addresses */

#define DeeDbg_Free_DEFINED
PUBLIC ATTR_HOT void
(DCALL DeeDbg_Free)(void *ptr, char const *file, int line) {
	struct leak_footer *leak;
	size_t *p_foot;
	mchunkptr p;
	if unlikely(!ptr)
		return;
	p = mem2chunk(ptr);

	/* Special handling required for flag4 memory */
#if FLAG4_BIT_INDICATES_HEAP_REGION
	if unlikely(flag4inuse(p)) {
		dlfree(ptr);
		return;
	}
#endif /* FLAG4_BIT_INDICATES_HEAP_REGION */

	p_foot = chunk_p_foot(p);
	leak = *(struct leak_footer **)p_foot;
	if unlikely(!leak_addrok(leak)) {
		fatal("Dee_Free(ptr)", file, line,
		      "%p: Linked chunk %p-%p has bad footer=%p",
		      ptr, p, chunk_plus_offset(p, chunksize(p) - 1), leak);
	}
	if unlikely(leak->lf_chunk != ptr) {
		fatal("Dee_Free(ptr)", file, line,
		      "%p: Linked chunk %p-%p footer %p has bad chunk pointer %p",
		      ptr, p, chunk_plus_offset(p, chunksize(p) - 1),
		      leak, leak->lf_chunk);
	}

	if (!leaks_lock_tryacquire()) {
		/* Schedule "remove" to happen asynchronously */
		SLIST_ATOMIC_INSERT(&leaks_pending_remove, leak, lf_remlink);
		leaks_lock_reap();
		return;
	}

	/* If the leak hasn't finished being inserted into the leaks list,
	 * then we have to ensure that all pending inserts have been reaped! */
#ifdef leaks_insert_is_unfinished
	if (leaks_insert_is_unfinished(leak)) {
		leaks_lock_reap_insert_locked();
		ASSERT(!leaks_insert_is_unfinished(leak));
//		leaks_assert_linkage(leak);
	}
#else /* leaks_insert_is_unfinished */
	if (leaks_pending_mustreap_insert())
		leaks_lock_reap_insert_locked();
#endif /* !leaks_insert_is_unfinished */
	leaks_assert_linkage(leak); // TODO: Comment out

	/* Remove from list of leaks */
	leaks_remove(leak);
	leaks_lock_release();

	/* Restore original footer */
	*p_foot = leak->lf_foot;

	/* Free memory... */
	leak_footer_free(leak);
	dlfree(ptr);
}

#define DeeDbg_TryRealloc_DEFINED
PUBLIC ATTR_HOT WUNUSED void *
(DCALL DeeDbg_TryRealloc)(void *ptr, size_t n_bytes,
                          char const *file, int line) {
	void *result;
	struct leak_footer *leak;
	size_t *p_foot;
	mchunkptr p;
	if (!ptr)
		return DeeDbg_TryMalloc(n_bytes, file, line);
	p = mem2chunk(ptr);

	/* Special handling required for flag4 memory */
#if FLAG4_BIT_INDICATES_HEAP_REGION
	if unlikely(flag4inuse(p))
		return dlrealloc(ptr, n_bytes);
#endif /* FLAG4_BIT_INDICATES_HEAP_REGION */

	p_foot = chunk_p_foot(p);
	leak = *(struct leak_footer **)p_foot;
	if unlikely(!leak_addrok(leak)) {
		fatal("Dee_Realloc(ptr, n_bytes)", file, line,
		      "%p: Linked chunk %p-%p has bad footer=%p",
		      ptr, p, chunk_plus_offset(p, chunksize(p) - 1), leak);
	}
	if unlikely(leak->lf_chunk != ptr) {
		fatal("Dee_Realloc(ptr, n_bytes)", file, line,
		      "%p: Linked chunk %p-%p footer %p has bad chunk pointer %p",
		      ptr, p, chunk_plus_offset(p, chunksize(p) - 1),
		      leak, leak->lf_chunk);
	}

	/* Amend debug info if "leak" doesn't have any, yet.
	 * Since we're the owner of the heap chunk, we can just do this! */
	if (leak->lf_file == NULL) {
		leak->lf_file = file;
		leak->lf_line = line;
	}

	/* Restore original footer */
	*p_foot = leak->lf_foot;

	/* Do the realloc */
	result = dlrealloc(ptr, n_bytes);

	/* Check for error */
	if likely(result) {
		p = mem2chunk(result);
		p_foot = chunk_p_foot(p);
		leak->lf_chunk = result;
	}

	/* Re-override the footer (after backing up its (possibly changed) current value) */
	leak->lf_foot = *p_foot;
	*p_foot = (size_t)(uintptr_t)leak;
	return result;
}

#define Dee_TryReallocInPlace_DEFINED
PUBLIC WUNUSED void *
(DCALL Dee_TryReallocInPlace)(void *ptr, size_t n_bytes) {
	void *result;
	struct leak_footer *leak;
	size_t *p_foot;
	mchunkptr p;
	if unlikely(!ptr)
		return NULL;
	p = mem2chunk(ptr);

	/* Special handling required for flag4 memory */
#if FLAG4_BIT_INDICATES_HEAP_REGION
	if unlikely(flag4inuse(p))
		return NULL; /* Cannot be done */
#endif /* FLAG4_BIT_INDICATES_HEAP_REGION */

	p_foot = chunk_p_foot(p);
	leak = *(struct leak_footer **)p_foot;
	if unlikely(!leak_addrok(leak)) {
		fatal("Dee_TryReallocInPlace(ptr, n_bytes)", NULL, 0,
		      "%p: Linked chunk %p-%p has bad footer=%p",
		      ptr, p, chunk_plus_offset(p, chunksize(p) - 1), leak);
	}
	if unlikely(leak->lf_chunk != ptr) {
		fatal("Dee_TryReallocInPlace(ptr, n_bytes)", NULL, 0,
		      "%p: Linked chunk %p-%p footer %p has bad chunk pointer %p",
		      ptr, p, chunk_plus_offset(p, chunksize(p) - 1),
		      leak, leak->lf_chunk);
	}

	/* Restore original footer */
	*p_foot = leak->lf_foot;

	/* Do the realloc */
	result = dlrealloc_in_place(ptr, n_bytes);
	ASSERT(result == NULL || result == ptr);
	ASSERT(p == mem2chunk(ptr));
	p_foot = chunk_p_foot(p);

	/* Re-override the footer (after backing up its (possibly changed) current value) */
	leak->lf_foot = *p_foot;
	*p_foot = (size_t)(uintptr_t)leak;
	return result;
}

#define DeeDbg_UntrackAlloc_DEFINED
PUBLIC void *
(DCALL DeeDbg_UntrackAlloc)(void *ptr, char const *file, int line) {
	struct leak_footer *leak;
	size_t *p_foot;
	mchunkptr p;
	if unlikely(!ptr)
		return NULL;
	p = mem2chunk(ptr);

	/* Special handling required for flag4 memory */
#if FLAG4_BIT_INDICATES_HEAP_REGION
	if unlikely(flag4inuse(p)) {
		fatal("Dee_UntrackAlloc(ptr)", file, line,
		      "%p: Not allowed to untrack FLAG4 memory", ptr);
	}
#endif /* FLAG4_BIT_INDICATES_HEAP_REGION */

	p_foot = chunk_p_foot(p);
	leak = *(struct leak_footer **)p_foot;
	if unlikely(!leak_addrok(leak)) {
		fatal("Dee_UntrackAlloc(ptr)", file, line,
		      "%p: Linked chunk %p-%p has bad footer=%p",
		      ptr, p, chunk_plus_offset(p, chunksize(p) - 1), leak);
	}
	if unlikely(leak->lf_chunk != ptr) {
		fatal("Dee_UntrackAlloc(ptr)", file, line,
		      "%p: Linked chunk %p-%p footer %p has bad chunk pointer %p",
		      ptr, p, chunk_plus_offset(p, chunksize(p) - 1),
		      leak, leak->lf_chunk);
	}

	/* This operation cannot be done asynchronously (but
	 * that is fine, since it doesn't happen that often) */
	leaks_lock_acquire_and_reap();
	leaks_remove(leak);
	leaks_lock_release();

	/* For a dummy loop that will satisfy `Dee_Free()' and `Dee_Realloc()',
	 * but won't show up in `DeeHeap_DumpMemoryLeaks()' */
	leak->lf_prev = leak;
	leak->lf_next = leak;
	return ptr;
}


#elif LEAK_DETECTION
DECL_END
#include <hybrid/sequence/rbtree.h>
DECL_BEGIN

#if 0 /* Significantly slower */
#define RBTREE_LEFT_LEANING
#endif

struct leaknode {
	/* NOTE: We reference the heap chunks directly. That way, we don't have to do
	 *       anything extra for `Dee_TryReallocInPlace', since the only way that
	 *       call can succeed if there wouldn't be a node where it wants to expand
	 *       to, meaning there'd never be any reason to change anything about the
	 *       leaknode-tree, no matter what `Dee_TryReallocInPlace' ends up doing. */
	union {
#ifdef RBTREE_LEFT_LEANING
		LLRBTREE_NODE(leaknode) ln_node;    /* [1..1] Node in tree of allocations */
#else /* RBTREE_LEFT_LEANING */
		RBTREE_NODE(leaknode)   ln_node;    /* [1..1] Node in tree of allocations */
#endif /* !RBTREE_LEFT_LEANING */
#if USE_PENDING_FREE_LIST
		SLIST_ENTRY(leaknode)   ln_pend;    /* [0..1] Link in internal list of pending nodes */
#endif /* USE_PENDING_FREE_LIST */
	} ln_link;
	mchunkptr                   ln_chunk;   /* Chunk pointer */
	size_t                      ln_id;      /* Allocation ID */
	char const                 *ln_file;    /* Allocation source file */
	__UINTPTR_HALF_TYPE__       ln_line;    /* Allocation source line */
	__UINTPTR_HALF_TYPE__       ln_flags;   /* Allocation flags (set of `LEAKNODE_F_*') */
#define LEAKNODE_F_NORMAL 0x0000 /* Default flags */
#define LEAKNODE_F_RED    0x0001 /* Red node (for LLRBTREE) */
#define LEAKNODE_F_NOLEAK 0x0002 /* Do not consider as a leak */
};

DECL_END
#define RBTREE(name)            leaknode_tree_##name
#define RBTREE_T                struct leaknode
#define RBTREE_Tkey             char *
#define RBTREE_GETNODE(self)    (self)->ln_link.ln_node
#define RBTREE_GETMINKEY(self)  ((char *)(self)->ln_chunk)
#define RBTREE_GETMAXKEY(self)  ((char *)(self)->ln_chunk + chunksize((self)->ln_chunk) - 1)
#define RBTREE_REDFIELD         ln_flags
#define RBTREE_REDBIT           LEAKNODE_F_RED
#define RBTREE_CC               DFCALL
#define RBTREE_NOTHROW          NOTHROW
#define RBTREE_DECL             PRIVATE
#define RBTREE_IMPL             PRIVATE
#include <hybrid/sequence/rbtree-abi.h>
DECL_BEGIN

/* [0..n][lock(leak_lock)] Tree of memory nodes tracked
 * for the purpose of being considered leaks if not freed
 * at application exit. */
PRIVATE LLRBTREE_ROOT(leaknode) leak_nodes = NULL;
#if USE_LOCKS
PRIVATE Dee_atomic_lock_t leak_lock = Dee_ATOMIC_LOCK_INIT;
#define leak_lock_available()  Dee_atomic_lock_available(&leak_lock)
#define leak_lock_acquired()   Dee_atomic_lock_acquired(&leak_lock)
#define leak_lock_tryacquire() Dee_atomic_lock_tryacquire(&leak_lock)
#define leak_lock_acquire()    Dee_atomic_lock_acquire(&leak_lock)
#define leak_lock_waitfor()    Dee_atomic_lock_waitfor(&leak_lock)
#define _leak_lock_release()   Dee_atomic_lock_release(&leak_lock)
#define leak_lock_release()    Dee_atomic_lock_release(&leak_lock)
#else /* USE_LOCKS */
#define leak_lock_available()  1
#define leak_lock_acquired()   1
#define leak_lock_tryacquire() 1
#define leak_lock_acquire()    (void)0
#define leak_lock_waitfor()    (void)0
#define _leak_lock_release()   (void)0
#define leak_lock_release()    (void)0
#endif /* !USE_LOCKS */

#if USE_PENDING_FREE_LIST
SLIST_HEAD(leaklist, leaknode);
struct lfrelist_entry {
	SLIST_ENTRY(lfrelist_entry) lfle_link;
	char const                 *lfle_file;
	int                         lfle_line;
};
SLIST_HEAD(lfrelist, lfrelist_entry);

PRIVATE struct leaklist leaks_pending_insert = SLIST_HEAD_INITIALIZER(leaks_pending_insert);
PRIVATE struct lfrelist leaks_pending_remove = SLIST_HEAD_INITIALIZER(leaks_pending_remove);

#define leaks_pending_mustreap()                             \
	(atomic_read(&leaks_pending_insert.slh_first) != NULL || \
	 atomic_read(&leaks_pending_remove.slh_first) != NULL)

PRIVATE ATTR_NOINLINE void DCALL
_leaks_pending_reap_and_unlock(void) {
	struct freelist freeme;
	struct leaklist insert;
	struct lfrelist remove;
	/* Important: must reap "leaks_pending_remove" **BEFORE** "leaks_pending_insert"!
	 * Reason: "leaks_pending_remove" may contain pointers to blocks that are also
	 *         present within "leaks_pending_insert" (this is the case for short-
	 *         lived allocated). As such, if we were to clear "leaks_pending_remove"
	 *         only **after** "leaks_pending_insert", some other thread may have added
	 *         another pointer to both "leaks_pending_insert" and "leaks_pending_remove"
	 *         (meaning we'd also be freeing that one), but then we would only see the
	 *         remove request, without ever having complied with the insert request! */
	remove.slh_first = SLIST_ATOMIC_CLEAR(&leaks_pending_remove);

	/* Reap "leaks_pending_insert" */
	insert.slh_first = SLIST_ATOMIC_CLEAR(&leaks_pending_insert);
	while (!SLIST_EMPTY(&insert)) {
		struct leaknode *node = SLIST_FIRST(&insert);
		SLIST_REMOVE_HEAD(&insert, ln_link.ln_pend);
		leaknode_tree_insert(&leak_nodes, node);
	}

	SLIST_INIT(&freeme);
	while (!SLIST_EMPTY(&remove)) {
		struct leaknode *node;
		struct lfrelist_entry *ptr = SLIST_FIRST(&remove);
		char const *file = ptr->lfle_file;
		int line = ptr->lfle_line;
		SLIST_REMOVE_HEAD(&remove, lfle_link);
		node = leaknode_tree_remove(&leak_nodes, (char *)ptr);
		if unlikely(node && (void *)ptr != chunk2mem(node->ln_chunk)) {
			/* This can happen if "ptr" is a custom flag4 sub-region of a bigger heap region */
			leaknode_tree_insert(&leak_nodes, node);
			node = NULL;
		}
		if (!node) {
			mchunkptr p = mem2chunk(ptr);
			if (!flag4inuse(p)) {
				/* XXX: This check right here has been seen to fail for unexplained reasons in the past
				 *      When it failed, "file" was 0xCCCCCCCC or 0xDEADBEEF (or file=0xDEADBEEF, and 
				 *      ptr->lfle_file=0xDEADBEEF, indicating another thread having free'd the pointer
				 *      while our thread was trying to do the same).
				 * It seemed to happen under:
				 * - USE_PER_THREAD_MSTATE=0
				 * - FOOTERS=0/1
				 * when running `make computed-operators`, and then usually during the app finalization
				 *
				 * XXX: After further testing, I can no longer re-create this :/
				 *      Combined with the fact that "LEAK_DETECTION" will probably
				 *      need to be re-written anyways (since it's way too slow for
				 *      SMP), I think further research is unnecessary.
				 */
				_DeeAssert_Failf("Dee_Free(ptr)", file, line,
				                 "Bad pointer %p does not map to any node, or custom heap region",
				                 ptr);
				Dee_BREAKPOINT();
			}
		} else {
#if 0 /* Never happens -- see "leaknode_tree_insert" above */
			if (ptr != chunk2mem(node->ln_chunk)) {
				_DeeAssert_Failf("Dee_Free(ptr)", file, line,
				                 "Bad pointer %p does not map to start of node at %p-%p",
				                 ptr, chunk2mem(node->ln_chunk),
				                 (char *)chunk2mem(node->ln_chunk) +
				                 (chunksize(node->ln_chunk) - overhead_for(node->ln_chunk) - 1));
				Dee_BREAKPOINT();
			}
#endif
			SLIST_INSERT(&freeme, (struct freelist_entry *)node, fle_link); /* dlfree(node); */
		}
#if 0
		COMPILER_BARRIER();
		ASSERT(file == atomic_read(&ptr->lfle_file));
		ASSERT(*file == 'E');
		COMPILER_BARRIER();
#endif
		SLIST_INSERT(&freeme, (struct freelist_entry *)ptr, fle_link); /* dlfree(ptr); */
	}
	_leak_lock_release();

	/* Pass along pointers that need to be free'd to the dlmalloc core */
	while (!SLIST_EMPTY(&freeme)) {
		struct freelist_entry *ent = SLIST_FIRST(&freeme);
		SLIST_REMOVE_HEAD(&freeme, fle_link);
		dlfree(ent);
	}
}

LOCAL void DCALL
leak_lock_acquire_and_reap(void) {
	leak_lock_acquire();
	if (leaks_pending_mustreap()) {
		_leaks_pending_reap_and_unlock();
		leak_lock_acquire();
	}
}

LOCAL void DCALL
_leak_lock_reap(void) {
	do {
		if (!leak_lock_tryacquire())
			break;
		_leaks_pending_reap_and_unlock();
	} while (leaks_pending_mustreap());
}

#define leak_lock_reap() \
	(void)(!leaks_pending_mustreap() || (_leak_lock_reap(), 0))
#undef leak_lock_release
#define leak_lock_release() (_leak_lock_release(), leak_lock_reap())
#else /* USE_PENDING_FREE_LIST */
#define leak_lock_reap()             (void)0
#define leak_lock_acquire_and_reap() leak_lock_acquire()
#endif /* !USE_PENDING_FREE_LIST */



PRIVATE WUNUSED NONNULL((1)) size_t DCALL
dump_leaks_node(struct leaknode *__restrict self) {
	size_t result = 0;
again:
	if (!(self->ln_flags & LEAKNODE_F_NOLEAK)) {
		char *base = (char *)(chunk2mem(self->ln_chunk));
		size_t size = chunksize(self->ln_chunk) - overhead_for(self->ln_chunk);
		Dee_DPRINTF("%s(%d) : %" PRFuSIZ " : Leaked %" PRFuSIZ " bytes of memory: %p-%p\n",
		            self->ln_file, self->ln_line, self->ln_id, size, base, base + size - 1);
		result += size;
	}
	if (self->ln_link.ln_node.rb_lhs) {
		if (self->ln_link.ln_node.rb_rhs)
			result += dump_leaks_node(self->ln_link.ln_node.rb_rhs);
		self = self->ln_link.ln_node.rb_lhs;
		goto again;
	}
	if (self->ln_link.ln_node.rb_rhs) {
		self = self->ln_link.ln_node.rb_rhs;
		goto again;
	}
	return result;
}

/* Dump info about all heap allocations that were allocated, but
 * never Dee_Free()'d, nor untracked using `Dee_UntrackAlloc()'.
 * Information about leaks is printed using `Dee_DPRINTF()'.
 *
 * @return: * : The total amount of memory leaked (in bytes) */
#define DeeHeap_DumpMemoryLeaks_DEFINED
PUBLIC size_t DCALL DeeHeap_DumpMemoryLeaks(void) {
	size_t result = 0;
	leak_lock_acquire_and_reap();
	if (leak_nodes)
		result = dump_leaks_node(leak_nodes);
	leak_lock_release();
	return result;
}



PRIVATE size_t alloc_id_count = 0;
PRIVATE size_t alloc_id_break = 0;

/* Get/set the memory allocation breakpoint.
 * - When the deemon heap was built to track memory leaks, an optional
 *   allocation breakpoint can be defined which, when reached, causes
 *   an attached debugger to break, allowing you to inspect the stack
 *   at the point where the `id'th allocation happened
 * - Allocation IDs are assigned in ascending order during every call
 *   to Dee_Malloc(), Dee_Calloc() and Dee_Realloc() (when ptr==NULL),
 *   as well as their Dee_Try* equivalents.
 * - When the deemon heap was not built with this feature, this API
 *   is a no-op, and always returns `0'
 * @return: * : The previously set allocation breakpoint */
#define DeeHeap_GetAllocBreakpoint_DEFINED
PUBLIC ATTR_COLD ATTR_PURE WUNUSED size_t DCALL
DeeHeap_GetAllocBreakpoint(void) {
	return atomic_read(&alloc_id_break);
}

PUBLIC ATTR_COLD size_t DCALL
DeeHeap_SetAllocBreakpoint(size_t id) {
	return atomic_xch(&alloc_id_break, id);
}

PRIVATE ATTR_HOT NONNULL((1, 2)) void DCALL
leaks_insert_p(struct leaknode *node, void *ptr,
               char const *file, int line) {
	node->ln_id = atomic_incfetch(&alloc_id_count);
	if (node->ln_id == alloc_id_break)
		Dee_BREAKPOINT();
	node->ln_chunk = mem2chunk(ptr);
	node->ln_file  = file;
	node->ln_line  = (__UINTPTR_HALF_TYPE__)line;
	node->ln_flags = LEAKNODE_F_NORMAL;
#if USE_PENDING_FREE_LIST
	if (!leak_lock_tryacquire()) {
		SLIST_ATOMIC_INSERT(&leaks_pending_insert, node, ln_link.ln_pend);
		leak_lock_reap();
		return;
	}
#else /* USE_PENDING_FREE_LIST */
	leak_lock_acquire();
#endif /* !USE_PENDING_FREE_LIST */
	leaknode_tree_insert(&leak_nodes, node);
	leak_lock_release();
}

PRIVATE ATTR_HOT WUNUSED NONNULL((1)) void *DCALL
leaks_insert(void *ptr, char const *file, int line) {
	struct leaknode *node = (struct leaknode *)dlmalloc(sizeof(struct leaknode));
	if unlikely(!node) {
		dlfree(ptr);
		return NULL;
	}
	leaks_insert_p(node, ptr, file, line);
	return ptr;
}

#define DeeDbg_TryMalloc_DEFINED
PUBLIC ATTR_HOT ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryMalloc)(size_t n_bytes, char const *file, int line) {
	void *result = dlmalloc(n_bytes);
	if (result)
		result = leaks_insert(result, file, line);
	return result;
}

#define DeeDbg_TryCalloc_DEFINED
PUBLIC ATTR_HOT ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryCalloc)(size_t n_bytes, char const *file, int line) {
	void *result = dlcalloc(n_bytes);
	if (result)
		result = leaks_insert(result, file, line);
	return result;
}

#define DeeDbg_Free_DEFINED
PUBLIC ATTR_HOT void
(DCALL DeeDbg_Free)(void *ptr, char const *file, int line) {
	struct leaknode *node;
	if (!ptr)
		return;
#if 0
	if unlikely(!file)
		file = "E:dummy";
	ASSERT(*file == 'E');
#endif
#if USE_PENDING_FREE_LIST
	if (!leak_lock_tryacquire()) {
		mchunkptr p;
		size_t usable;
handle_leak_lock_blocking:
		p = mem2chunk(ptr);
		ASSERT(is_inuse(p));
		usable = chunksize(p) - overhead_for(p);
		if (usable >= sizeof(struct lfrelist_entry)) {
			struct lfrelist_entry *ent = (struct lfrelist_entry *)ptr;
			ent->lfle_file = file;
			ent->lfle_line = line;
			SLIST_ATOMIC_INSERT(&leaks_pending_remove, ent, lfle_link);
			leak_lock_reap();
			return;
		}
		leak_lock_acquire();
	}
	/* Make sure that "ptr" isn't still pending insertion! */
	if (atomic_read(&leaks_pending_insert.slh_first) != NULL) {
		_leaks_pending_reap_and_unlock();
		if (!leak_lock_tryacquire())
			goto handle_leak_lock_blocking;
	}
#else /* USE_PENDING_FREE_LIST */
	leak_lock_acquire();
#endif /* !USE_PENDING_FREE_LIST */
	node = leaknode_tree_remove(&leak_nodes, (char *)ptr);
	if unlikely(node && ptr != chunk2mem(node->ln_chunk)) {
		/* This can happen if "ptr" is a custom flag4 sub-region of a bigger heap region */
		leaknode_tree_insert(&leak_nodes, node);
		node = NULL;
	}
	leak_lock_release();
	if (!node) {
		mchunkptr p = mem2chunk(ptr);
		if (!flag4inuse(p)) {
			_DeeAssert_Failf("Dee_Free(ptr)", file, line,
			                 "Bad pointer %p does not map to any node, or custom heap region",
			                 ptr);
			Dee_BREAKPOINT();
		}
	} else {
#if 0 /* Never happens -- see "leaknode_tree_insert" above */
		if (ptr != chunk2mem(node->ln_chunk)) {
			_DeeAssert_Failf("Dee_Free(ptr)", file, line,
			                 "Bad pointer %p does not map to start of node at %p-%p",
			                 ptr, chunk2mem(node->ln_chunk),
			                 (char *)chunk2mem(node->ln_chunk) +
			                 (chunksize(node->ln_chunk) - overhead_for(node->ln_chunk) - 1));
			Dee_BREAKPOINT();
		}
#endif
		dlfree(node);
	}
	dlfree(ptr);
}

#define DeeDbg_TryRealloc_DEFINED
PUBLIC ATTR_HOT WUNUSED void *
(DCALL DeeDbg_TryRealloc)(void *ptr, size_t n_bytes,
                          char const *file, int line) {
	struct leaknode *node;
	if (!ptr)
		return DeeDbg_TryMalloc(n_bytes, file, line);
#if USE_PENDING_FREE_LIST
	if (!leak_lock_tryacquire()) {
		size_t usable;
handle_leak_lock_blocking:
		{
			mchunkptr p = mem2chunk(ptr);
			ASSERT(is_inuse(p));
			usable = chunksize(p) - overhead_for(p);
		}

		/* See how "usable" compares to "n_bytes". Dependent on that,
		 * we might be able to fulfill the request without blocking */
		if (n_bytes <= usable) {
			size_t unused = usable - n_bytes;

			/* When very little memory is being free'd, * skip the call for the sake of performance */
			if (unused <= (usable / 256) || unused <= (8 * sizeof(void *)))
				return ptr;
		} else if (usable >= sizeof(struct lfrelist_entry)/* && (n_bytes < 32 * sizeof(void *))*/) {
			/* Try to emulate as "malloc()+memcpy()+free()" (all of
			 * which can be done without blocking at the "leaks" layer) */
			void *new_block = dlmalloc(n_bytes);
			if (new_block) {
				node = (struct leaknode *)dlmalloc(sizeof(struct leaknode));
				if (node) {
					size_t common;

					/* Insert new block */
					leaks_insert_p(node, new_block, file, line);

					/* Copy over data into new block */
					common = n_bytes < usable ? n_bytes : usable;
					memcpy(new_block, ptr, common);

					/* Free the old block */
					DeeDbg_Free(ptr, file, line);
					return new_block;
				}
				dlfree(new_block);
			}
		}
		/* Fallback: must get a proper lock and use the serialized code-path below... */
		leak_lock_acquire();
	}
	/* Make sure that "ptr" isn't still pending insertion! */
	if (atomic_read(&leaks_pending_insert.slh_first) != NULL) {
		_leaks_pending_reap_and_unlock();
		if (!leak_lock_tryacquire())
			goto handle_leak_lock_blocking;
	}
#else /* USE_PENDING_FREE_LIST */
	leak_lock_acquire();
#endif /* !USE_PENDING_FREE_LIST */
	node = leaknode_tree_remove(&leak_nodes, (char *)ptr);
	if unlikely(node && ptr != chunk2mem(node->ln_chunk)) {
		/* This can happen if "ptr" is a custom flag4 sub-region of a bigger heap region */
		leaknode_tree_insert(&leak_nodes, node);
		node = NULL;
	}
	leak_lock_release();
	if (!node) {
		/* This should only happen when "ptr" is part of a "custom" heap region */
		mchunkptr p = mem2chunk(ptr);
		if (!flag4inuse(p)) {
			_DeeAssert_Failf("Dee_Realloc(ptr, n_bytes)", file, line,
			                 "Bad pointer %p does not map to any node, or custom heap region",
			                 ptr);
			Dee_BREAKPOINT();
		}
		node = (struct leaknode *)dlmalloc(sizeof(struct leaknode));
		if unlikely(!node)
			return NULL;
		ptr = dlrealloc(ptr, n_bytes);
		if (ptr) {
			leaks_insert_p(node, ptr, file, line);
		} else {
			dlfree(node);
		}
	} else {
#if 0 /* Never happens -- see "leaknode_tree_insert" above */
		if (ptr != chunk2mem(node->ln_chunk)) {
			_DeeAssert_Failf("Dee_Realloc(ptr)", file, line,
			                 "Bad pointer %p does not map to start of node at %p-%p",
			                 ptr, chunk2mem(node->ln_chunk),
			                 (char *)chunk2mem(node->ln_chunk) +
			                 (chunksize(node->ln_chunk) - overhead_for(node->ln_chunk) - 1));
			Dee_BREAKPOINT();
		}
#endif
		ptr = dlrealloc(ptr, n_bytes);
		if (ptr) {
			if (!node->ln_file) {
				node->ln_file = file;
				node->ln_line = (__UINTPTR_HALF_TYPE__)line;
			}
			node->ln_chunk = mem2chunk(ptr);
#if USE_PENDING_FREE_LIST
			if (!leak_lock_tryacquire()) {
				SLIST_ATOMIC_INSERT(&leaks_pending_insert, node, ln_link.ln_pend);
				leak_lock_reap();
			} else
#else /* USE_PENDING_FREE_LIST */
			leak_lock_acquire();
#endif /* !USE_PENDING_FREE_LIST */
			{
				leaknode_tree_insert(&leak_nodes, node);
				leak_lock_release();
			}
		} else {
			dlfree(node);
		}
	}
	return ptr;
}

#define DeeDbg_TryMemalign_DEFINED
PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryMemalign)(size_t min_alignment, size_t n_bytes,
                           char const *file, int line) {
	void *result = dlmemalign(min_alignment, n_bytes);
	if (result)
		result = leaks_insert(result, file, line);
	return result;
}

#define DeeDbg_UntrackAlloc_DEFINED
PUBLIC void *
(DCALL DeeDbg_UntrackAlloc)(void *ptr, char const *file, int line) {
	if (ptr) {
		struct leaknode *node;
		leak_lock_acquire_and_reap();
		node = leaknode_tree_locate(leak_nodes, (char *)ptr);
		if (!node) {
			_DeeAssert_Failf("Dee_UntrackAlloc(ptr)", file, line,
			                 "Bad pointer %p does not map to any node");
			Dee_BREAKPOINT();
		} else if (ptr != chunk2mem(node->ln_chunk)) {
			_DeeAssert_Failf("Dee_UntrackAlloc(ptr)", file, line,
			                 "Bad pointer %p does not map to start of node at %p-%p",
			                 ptr, chunk2mem(node->ln_chunk),
			                 (char *)chunk2mem(node->ln_chunk) +
			                 (chunksize(node->ln_chunk) - overhead_for(node->ln_chunk) - 1));
			Dee_BREAKPOINT();
		} else {
			node->ln_flags |= LEAKNODE_F_NOLEAK;
		}
		leak_lock_release();
	}
	return ptr;
}

#else /* LEAK_DETECTION */

#ifndef Dee_TryMalloc_DEFINED
#define Dee_TryMalloc_DEFINED
PUBLIC ATTR_HOT ATTR_MALLOC WUNUSED void *
(DCALL Dee_TryMalloc)(size_t n_bytes) {
	return dlmalloc(n_bytes);
}
#endif /* !Dee_TryMalloc_DEFINED */

#ifndef Dee_TryCalloc_DEFINED
#define Dee_TryCalloc_DEFINED
PUBLIC ATTR_HOT ATTR_MALLOC WUNUSED void *
(DCALL Dee_TryCalloc)(size_t n_bytes) {
	return dlcalloc(n_bytes);
}
#endif /* !Dee_TryCalloc_DEFINED */

#ifndef Dee_Free_DEFINED
#define Dee_Free_DEFINED
PUBLIC ATTR_HOT void
(DCALL Dee_Free)(void *ptr) {
#if !DLFREE_CHECKS_NULL
	if unlikely(!ptr)
		return;
#endif /* !DLFREE_CHECKS_NULL */
	dlfree(ptr);
}
#endif /* !Dee_Free_DEFINED */

#ifndef Dee_TryRealloc_DEFINED
#define Dee_TryRealloc_DEFINED
PUBLIC ATTR_HOT WUNUSED void *
(DCALL Dee_TryRealloc)(void *ptr, size_t n_bytes) {
#if !DLFREE_CHECKS_NULL
	if unlikely(!ptr)
		return dlmalloc(n_bytes);
#endif /* !DLFREE_CHECKS_NULL */
	return dlrealloc(ptr, n_bytes);
}
#endif /* !Dee_TryRealloc_DEFINED */

#ifndef Dee_TryMemalign_DEFINED
#define Dee_TryMemalign_DEFINED
PUBLIC ATTR_HOT ATTR_MALLOC WUNUSED void *
(DCALL Dee_TryMemalign)(size_t min_alignment, size_t n_bytes) {
	return dlmemalign(min_alignment, n_bytes);
}
#endif /* !Dee_TryMemalign_DEFINED */

#endif /* LEAK_DETECTION */
#endif /* !DIRECTLY_DEFINE_DEEMON_PUBLIC_API */


/* Given a heap pointer (as could also be passed to `Dee_Free()' or
 * `Dee_MallocUsableSize()'), check if that pointer belongs to a custom
 * heap region, and if so: return a pointer to said heap region.
 * - If `ptr' is `NULL' or a heap pointer that does not belong
 *   to a custom heap region, `NULL' is returned instead.
 * - If `ptr' isn't a heap pointer, behavior is undefined.
 * - Unlike `Dee_MallocUsableSize()', this function has another special
 *   case where behavior is also well-defined: if deemon was built with
 *   object slabs enabled (!CONFIG_NO_OBJECT_SLABS), and the given `ptr'
 *   points into the special slab-heap, then `NULL' is always returned.
 *
 * @return: * :   The heap region belonging to `ptr'
 * @return: NULL: Given `ptr' is `NULL' or does not belong to a custom heap region */
PUBLIC ATTR_PURE WUNUSED struct Dee_heapregion *DCALL
DeeHeap_GetRegionOf(void *ptr) {
#if FLAG4_BIT_INDICATES_HEAP_REGION
	struct Dee_heapregion *result = NULL;
#ifdef IS_SLAB_POINTER
	if (!IS_SLAB_POINTER(ptr))
#endif /* IS_SLAB_POINTER */
	{
		mchunkptr p = mem2chunk(ptr);
		if (flag4inuse(p)) {
			/* Traverse the chain of chunks until we find one
			 * with "prev_foot" (aka. "hc_prevsize") == 0 */
			while (p->prev_foot) {
				ASSERT(IS_ALIGNED(p->prev_foot, Dee_HEAPCHUNK_ALIGN));
				p = chunk_minus_offset(p, p->prev_foot);
			}
			result = COMPILER_CONTAINER_OF((struct Dee_heapchunk *)p,
			                               struct Dee_heapregion, hr_first);
			ASSERT(result->hr_size >= (size_t)((char *)mem2chunk(ptr) - (char *)result));
		} else {
#if DL_DEBUG_INTERNAL
#if FOOTERS && !GM_ONLY
			mstate fm = get_mstate_for(p);
			ext_assert__ok_magic(fm);
#else /* FOOTERS && !GM_ONLY */
#define fm gm
#endif /* !FOOTERS || GM_ONLY */
			PREACTION(fm);
			check_inuse_chunk(fm, p);
			POSTACTION(fm);
#undef fm
#endif /* DL_DEBUG_INTERNAL */
		}
	}
	return result;
#else /* FLAG4_BIT_INDICATES_HEAP_REGION */
	(void)ptr;
	return NULL;
#endif /* FLAG4_BIT_INDICATES_HEAP_REGION */
}

#if DL_DEBUG_EXTERNAL
PRIVATE void lock_and_do_check_malloc_state(PARAM_mstate_m) {
	PREACTION(m);
	do_check_malloc_state(ARG_mstate_m);
	POSTACTION(m);
}

#if USE_PER_THREAD_MSTATE
PRIVATE size_t lock_and_do_check_malloc_state_foreach_cb(mspace ms, void *arg) {
	(void)arg;
	lock_and_do_check_malloc_state(ARG_mstate_X((mstate)ms));
	return 0;
}
#endif /* USE_PER_THREAD_MSTATE */

/* Validate heap memory, asserting the absence of corruptions from
 * various common heap mistakes (write-past-end, use-after-free, etc.).
 *
 * When deemon was not built for debugging, this is a no-op. */
#define DeeHeap_CheckMemory_DEFINED
PUBLIC ATTR_COLD void DCALL DeeHeap_CheckMemory(void) {
	lock_and_do_check_malloc_state(ARG_mstate_gm);
#if USE_PER_THREAD_MSTATE
	tls_mspace_foreach(&lock_and_do_check_malloc_state_foreach_cb, NULL);
#endif /* USE_PER_THREAD_MSTATE */
}
#endif /* DL_DEBUG_EXTERNAL */


#if defined(DeeDbg_TryMalloc_DEFINED) || defined(DeeDbg_Malloc_DEFINED)
#define ATTR_HOT_DEBUG  ATTR_HOT
#define ATTR_HOT_NDEBUG ATTR_COLD
#else /* DeeDbg_TryMalloc_DEFINED || DeeDbg_Malloc_DEFINED */
#define ATTR_HOT_DEBUG  ATTR_COLD
#define ATTR_HOT_NDEBUG ATTR_HOT
#endif /* !DeeDbg_TryMalloc_DEFINED && !DeeDbg_Malloc_DEFINED */


#ifndef Dee_TryMalloc_DEFINED
#define Dee_TryMalloc_DEFINED
PUBLIC ATTR_HOT_NDEBUG ATTR_MALLOC WUNUSED void *
(DCALL Dee_TryMalloc)(size_t n_bytes) {
#ifdef DeeDbg_TryMalloc_DEFINED
	return (DeeDbg_TryMalloc)(n_bytes, NULL, 0);
#else /* DeeDbg_TryMalloc_DEFINED */
	return dlmalloc(n_bytes);
#endif /* !DeeDbg_TryMalloc_DEFINED */
}
#endif /* !Dee_TryMalloc_DEFINED */

#ifndef Dee_TryCalloc_DEFINED
#define Dee_TryCalloc_DEFINED
PUBLIC ATTR_HOT_NDEBUG ATTR_MALLOC WUNUSED void *
(DCALL Dee_TryCalloc)(size_t n_bytes) {
#ifdef DeeDbg_TryCalloc_DEFINED
	return (DeeDbg_TryCalloc)(n_bytes, NULL, 0);
#else /* DeeDbg_TryCalloc_DEFINED */
	return dlcalloc(n_bytes);
#endif /* !DeeDbg_TryCalloc_DEFINED */
}
#endif /* !Dee_TryCalloc_DEFINED */

#ifndef Dee_TryRealloc_DEFINED
#define Dee_TryRealloc_DEFINED
PUBLIC ATTR_HOT_NDEBUG WUNUSED void *
(DCALL Dee_TryRealloc)(void *ptr, size_t n_bytes) {
#ifdef DeeDbg_TryRealloc_DEFINED
	return (DeeDbg_TryRealloc)(ptr, n_bytes, NULL, 0);
#else /* DeeDbg_TryRealloc_DEFINED */
#if !DLFREE_CHECKS_NULL
	if unlikely(!ptr)
		return dlmalloc(n_bytes);
#endif /* !DLFREE_CHECKS_NULL */
	return dlrealloc(ptr, n_bytes);
#endif /* !DeeDbg_TryRealloc_DEFINED */
}
#endif /* !Dee_TryRealloc_DEFINED */

#ifndef Dee_TryMemalign_DEFINED
#define Dee_TryMemalign_DEFINED
PUBLIC /*ATTR_HOT_NDEBUG*/ ATTR_MALLOC WUNUSED void *
(DCALL Dee_TryMemalign)(size_t min_alignment, size_t n_bytes) {
#ifdef DeeDbg_TryMemalign_DEFINED
	return (DeeDbg_TryMemalign)(min_alignment, n_bytes, NULL, 0);
#else /* DeeDbg_TryMemalign_DEFINED */
	return dlmemalign(min_alignment, n_bytes);
#endif /* !DeeDbg_TryMemalign_DEFINED */
}
#endif /* !Dee_TryMemalign_DEFINED */



PUBLIC ATTR_PURE WUNUSED size_t
(DCALL Dee_MallocUsableSize)(void *mem) {
	if (mem != 0) {
		mchunkptr p = mem2chunk(mem);
#if 1
		ASSERT(is_inuse(p));
		return chunksize(p) - overhead_for(p);
#else
		if (is_inuse(p))
			return chunksize(p) - overhead_for(p);
#endif
	}
	return 0;
}

/* Default malloc/free functions used for heap allocation.
 * NOTE: Upon allocation failure, caches are cleared and an `Error.NoMemory' is thrown. */
#ifndef Dee_Malloc_DEFINED
#define Dee_Malloc_DEFINED
PUBLIC ATTR_HOT_NDEBUG ATTR_MALLOC WUNUSED void *
(DCALL Dee_Malloc)(size_t n_bytes) {
#ifdef DeeDbg_Malloc_DEFINED
	return (DeeDbg_Malloc)(n_bytes, NULL, 0);
#else /* DeeDbg_Malloc_DEFINED */
	void *result;
again:
#ifdef DeeDbg_TryMalloc_DEFINED
	result = (DeeDbg_TryMalloc)(n_bytes, NULL, 0);
#else /* DeeDbg_TryMalloc_DEFINED */
	result = (Dee_TryMalloc)(n_bytes);
#endif /* !DeeDbg_TryMalloc_DEFINED */
	if unlikely(!result) {
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
#endif /* !DeeDbg_Malloc_DEFINED */
}
#endif /* !Dee_Malloc_DEFINED */

#ifndef Dee_Calloc_DEFINED
#define Dee_Calloc_DEFINED
PUBLIC ATTR_HOT_NDEBUG ATTR_MALLOC WUNUSED void *
(DCALL Dee_Calloc)(size_t n_bytes) {
#ifdef DeeDbg_Calloc_DEFINED
	return (DeeDbg_Calloc)(n_bytes, NULL, 0);
#else /* DeeDbg_Calloc_DEFINED */
	void *result;
again:
#ifdef DeeDbg_TryCalloc_DEFINED
	result = (DeeDbg_TryCalloc)(n_bytes, NULL, 0);
#else /* DeeDbg_TryCalloc_DEFINED */
	result = (Dee_TryCalloc)(n_bytes);
#endif /* !DeeDbg_TryCalloc_DEFINED */
	if unlikely(!result) {
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
#endif /* !DeeDbg_Calloc_DEFINED */
}
#endif /* !Dee_Calloc_DEFINED */

#ifndef Dee_Realloc_DEFINED
#define Dee_Realloc_DEFINED
PUBLIC ATTR_HOT_NDEBUG WUNUSED void *
(DCALL Dee_Realloc)(void *ptr, size_t n_bytes) {
#ifdef DeeDbg_Realloc_DEFINED
	return (DeeDbg_Realloc)(ptr, n_bytes, NULL, 0);
#else /* DeeDbg_Realloc_DEFINED */
	void *result;
again:
#ifdef DeeDbg_TryRealloc_DEFINED
	result = (DeeDbg_TryRealloc)(ptr, n_bytes, NULL, 0);
#else /* DeeDbg_TryRealloc_DEFINED */
	result = (Dee_TryRealloc)(ptr, n_bytes);
#endif /* !DeeDbg_TryRealloc_DEFINED */
	if unlikely(!result) {
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
#endif /* !DeeDbg_Realloc_DEFINED */
}
#endif /* !Dee_Realloc_DEFINED */

#ifndef Dee_Memalign_DEFINED
#define Dee_Memalign_DEFINED
PUBLIC /*ATTR_HOT_NDEBUG*/ ATTR_MALLOC WUNUSED void *
(DCALL Dee_Memalign)(size_t min_alignment, size_t n_bytes) {
#ifdef DeeDbg_Memalign_DEFINED
	return (DeeDbg_Memalign)(min_alignment, n_bytes, NULL, 0);
#else /* DeeDbg_Memalign_DEFINED */
	void *result;
again:
#ifdef DeeDbg_TryMemalign_DEFINED
	result = (DeeDbg_TryMemalign)(min_alignment, n_bytes, NULL, 0);
#else /* DeeDbg_TryMemalign_DEFINED */
	result = (Dee_TryMemalign)(min_alignment, n_bytes);
#endif /* !DeeDbg_TryMemalign_DEFINED */
	if unlikely(!result) {
		size_t total;
		if (OVERFLOW_UADD(min_alignment, n_bytes, &total))
			total = (size_t)-1;
		if (Dee_CollectMemory(total))
			goto again;
	}
	return result;
#endif /* !DeeDbg_Memalign_DEFINED */
}
#endif /* !Dee_Memalign_DEFINED */

#ifndef Dee_Free_DEFINED
#define Dee_Free_DEFINED
PUBLIC ATTR_HOT_NDEBUG void (DCALL Dee_Free)(void *ptr) {
	return (DeeDbg_Free)(ptr, NULL, 0);
}
#endif /* !Dee_Free_DEFINED */



/* Fallback: The host does not provide a debug-allocation API. */
#ifndef DeeDbg_Malloc_DEFINED
#define DeeDbg_Malloc_DEFINED
PUBLIC ATTR_HOT_DEBUG ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_Malloc)(size_t n_bytes, char const *file, int line) {
#ifdef DeeDbg_TryMalloc_DEFINED
	void *result;
again:
	result = (DeeDbg_TryMalloc)(n_bytes, file, line);
	if unlikely(!result) {
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
#else /* DeeDbg_TryMalloc_DEFINED */
	(void)file;
	(void)line;
	return (Dee_Malloc)(n_bytes);
#endif /* !DeeDbg_TryMalloc_DEFINED */
}
#endif /* !DeeDbg_Malloc_DEFINED */

#ifndef DeeDbg_Calloc_DEFINED
#define DeeDbg_Calloc_DEFINED
PUBLIC ATTR_HOT_DEBUG ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_Calloc)(size_t n_bytes, char const *file, int line) {
#ifdef DeeDbg_TryCalloc_DEFINED
	void *result;
again:
	result = (DeeDbg_TryCalloc)(n_bytes, file, line);
	if unlikely(!result) {
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
#else /* DeeDbg_TryCalloc_DEFINED */
	(void)file;
	(void)line;
	return (Dee_Calloc)(n_bytes);
#endif /* !DeeDbg_TryCalloc_DEFINED */
}
#endif /* !DeeDbg_Calloc_DEFINED */

#ifndef DeeDbg_Realloc_DEFINED
#define DeeDbg_Realloc_DEFINED
PUBLIC ATTR_HOT_DEBUG WUNUSED void *
(DCALL DeeDbg_Realloc)(void *ptr, size_t n_bytes, char const *file, int line) {
#ifdef DeeDbg_TryRealloc_DEFINED
	void *result;
again:
	result = (DeeDbg_TryRealloc)(ptr, n_bytes, file, line);
	if unlikely(!result) {
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
#else /* DeeDbg_TryRealloc_DEFINED */
	(void)file;
	(void)line;
	return (Dee_Realloc)(ptr, n_bytes);
#endif /* !DeeDbg_TryRealloc_DEFINED */
}
#endif /* !DeeDbg_Realloc_DEFINED */

#ifndef DeeDbg_Memalign_DEFINED
#define DeeDbg_Memalign_DEFINED
PUBLIC /*ATTR_HOT_DEBUG*/ ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_Memalign)(size_t min_alignment, size_t n_bytes, char const *file, int line) {
#ifdef DeeDbg_TryMemalign_DEFINED
	void *result;
again:
	result = (DeeDbg_TryMemalign)(min_alignment, n_bytes, file, line);
	if unlikely(!result) {
		size_t total;
		if (OVERFLOW_UADD(min_alignment, n_bytes, &total))
			total = (size_t)-1;
		if (Dee_CollectMemory(total))
			goto again;
	}
	return result;
#else /* DeeDbg_TryMemalign_DEFINED */
	(void)file;
	(void)line;
	return (Dee_Memalign)(min_alignment, n_bytes);
#endif /* !DeeDbg_TryMemalign_DEFINED */
}
#endif /* !DeeDbg_Memalign_DEFINED */

#ifndef DeeDbg_TryMalloc_DEFINED
#define DeeDbg_TryMalloc_DEFINED
PUBLIC ATTR_HOT_DEBUG ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryMalloc)(size_t n_bytes, char const *file, int line) {
	(void)file;
	(void)line;
	return (Dee_TryMalloc)(n_bytes);
}
#endif /* !DeeDbg_TryMalloc_DEFINED */

#ifndef DeeDbg_TryCalloc_DEFINED
#define DeeDbg_TryCalloc_DEFINED
PUBLIC ATTR_HOT_DEBUG ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryCalloc)(size_t n_bytes, char const *file, int line) {
	(void)file;
	(void)line;
	return (Dee_TryCalloc)(n_bytes);
}
#endif /* !DeeDbg_TryCalloc_DEFINED */

#ifndef DeeDbg_TryRealloc_DEFINED
#define DeeDbg_TryRealloc_DEFINED
PUBLIC ATTR_HOT_DEBUG WUNUSED void *
(DCALL DeeDbg_TryRealloc)(void *ptr, size_t n_bytes,
                          char const *file, int line) {
	(void)file;
	(void)line;
	return (Dee_TryRealloc)(ptr, n_bytes);
}
#endif /* !DeeDbg_TryRealloc_DEFINED */

#ifndef DeeDbg_TryMemalign_DEFINED
#define DeeDbg_TryMemalign_DEFINED
PUBLIC /*ATTR_HOT_DEBUG*/ ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryMemalign)(size_t min_alignment, size_t n_bytes, char const *file, int line) {
	(void)file;
	(void)line;
	return (Dee_TryMemalign)(min_alignment, n_bytes);
}
#endif /* !DeeDbg_TryMemalign_DEFINED */

#ifndef DeeDbg_Free_DEFINED
#define DeeDbg_Free_DEFINED
PUBLIC ATTR_HOT_DEBUG void
(DCALL DeeDbg_Free)(void *ptr, char const *file, int line) {
	(void)file;
	(void)line;
	(Dee_Free)(ptr);
}
#endif /* !DeeDbg_Free_DEFINED */

#ifndef DeeDbg_UntrackAlloc_DEFINED
#define DeeDbg_UntrackAlloc_DEFINED
PUBLIC void *
(DCALL DeeDbg_UntrackAlloc)(void *ptr, char const *file, int line) {
	(void)file;
	(void)line;
	return ptr;
}
#endif /* !DeeDbg_UntrackAlloc_DEFINED */

#undef ATTR_HOT_DEBUG
#undef ATTR_HOT_NDEBUG


#ifndef DeeHeap_CheckMemory_DEFINED
#define DeeHeap_CheckMemory_DEFINED
/* Validate heap memory, asserting the absence of corruptions from
 * various common heap mistakes (write-past-end, use-after-free, etc.).
 *
 * When deemon was not built for debugging, this is a no-op. */
PUBLIC ATTR_COLD void DCALL DeeHeap_CheckMemory(void) {
	/* nothing */
}
#endif /* !DeeHeap_CheckMemory_DEFINED */


#ifndef DeeHeap_DumpMemoryLeaks_DEFINED
#define DeeHeap_DumpMemoryLeaks_DEFINED
/* Dump info about all heap allocations that were allocated, but
 * never Dee_Free()'d, nor untracked using `Dee_UntrackAlloc()'.
 * Information about leaks is printed using `Dee_DPRINTF()'.
 *
 * @return: * : The total amount of memory leaked (in bytes) */
PUBLIC size_t DCALL DeeHeap_DumpMemoryLeaks(void) {
	/* nothing */
	return 0;
}
#endif /* !DeeHeap_DumpMemoryLeaks_DEFINED */


#ifndef DeeHeap_GetAllocBreakpoint_DEFINED
#define DeeHeap_GetAllocBreakpoint_DEFINED

/* Get/set the memory allocation breakpoint.
 * - When the deemon heap was built to track memory leaks, an optional
 *   allocation breakpoint can be defined which, when reached, causes
 *   an attached debugger to break, allowing you to inspect the stack
 *   at the point where the `id'th allocation happened
 * - Allocation IDs are assigned in ascending order during every call
 *   to Dee_Malloc(), Dee_Calloc() and Dee_Realloc() (when ptr==NULL),
 *   as well as their Dee_Try* equivalents.
 * - When the deemon heap was not built with this feature, this API
 *   is a no-op, and always returns `0'
 * @return: * : The previously set allocation breakpoint */
PUBLIC ATTR_COLD ATTR_PURE WUNUSED size_t DCALL
DeeHeap_GetAllocBreakpoint(void) {
	return 0;
}

PUBLIC ATTR_COLD size_t DCALL
DeeHeap_SetAllocBreakpoint(size_t id) {
	(void)id;
	return 0;
}
#endif /* !DeeHeap_GetAllocBreakpoint_DEFINED */



#ifndef CONFIG_NO_THREADS
#ifndef thread_heap_destroy_DEFINED
#define thread_heap_destroy_DEFINED
INTERN NONNULL((1)) void DCALL
thread_heap_destroy(void *heap) {
	COMPILER_IMPURE();
	(void)heap;
}
#endif /* !thread_heap_destroy_DEFINED */
#endif /* !CONFIG_NO_THREADS */


DECL_END
#else /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */

#include <hybrid/align.h>
#include <hybrid/debug-alignment.h>
#include <hybrid/typecore.h>
/**/

#include <stddef.h> /* uintptr_t */
#include <stdint.h> /* UINT16_C, UINT32_C */

#ifdef CONFIG_HAVE_STDLIB_H
#include <stdlib.h>
#endif /* CONFIG_HAVE_STDLIB_H */
#ifdef CONFIG_HAVE_MALLOC_H
#include <malloc.h>
#endif /* CONFIG_HAVE_MALLOC_H */
#ifdef CONFIG_HAVE_CRTDBG_H
#include <crtdbg.h>
#endif /* CONFIG_HAVE_CRTDBG_H */

/* Whitelist of some C libraries where we know that `malloc(0)'
 * doesn't return `NULL' unless it's *actually* out-of-memory. */
#ifndef __MALLOC_ZERO_IS_NONNULL
#ifndef __KOS_SYSTEM_HEADERS__
#if defined(_MSC_VER)
#define __MALLOC_ZERO_IS_NONNULL
#undef __REALLOC_ZERO_IS_NONNULL /* Nope, `realloc(p, 0)' acts like `free(p)'... */
#elif defined(__GLIBC__) || defined(__GNU_LIBRARY__)
#define __MALLOC_ZERO_IS_NONNULL
#define __REALLOC_ZERO_IS_NONNULL
#endif /* ... */
#endif /* !__KOS_SYSTEM_HEADERS__ */
#endif /* !__MALLOC_ZERO_IS_NONNULL */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

#if 0
#define HEAP_CHECK()  Dee_CHECKMEMORY()
#else
#define HEAP_CHECK()  (void)0
#endif

#if 0
#define BEGIN_ALLOC() (DBG_ALIGNMENT_DISABLE(), DeeMem_ClearCaches((size_t)-1))
#else
#define BEGIN_ALLOC() DBG_ALIGNMENT_DISABLE()
#endif
#define END_ALLOC()      DBG_ALIGNMENT_ENABLE()
#define BEGIN_TRYALLOC() DBG_ALIGNMENT_DISABLE()
#define END_TRYALLOC()   DBG_ALIGNMENT_ENABLE()

/* Default malloc/free functions used for heap allocation.
 * NOTE: Upon allocation failure, caches are cleared and an `Error.NoMemory' is thrown. */
PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL Dee_Malloc)(size_t n_bytes) {
	void *result;
again:
	BEGIN_ALLOC();
	HEAP_CHECK();
	result = (malloc)(n_bytes);
	END_ALLOC();
	if unlikely(!result) {
#ifndef __MALLOC_ZERO_IS_NONNULL
		if (!n_bytes) {
			BEGIN_ALLOC();
			result = (calloc)(1, 1);
			END_ALLOC();
			if (result)
				return result;
		}
#endif /* !__MALLOC_ZERO_IS_NONNULL */
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL Dee_Calloc)(size_t n_bytes) {
	void *result;
again:
	BEGIN_ALLOC();
	HEAP_CHECK();
	result = (calloc)(1, n_bytes);
	END_ALLOC();
	if unlikely(!result) {
#ifndef __MALLOC_ZERO_IS_NONNULL
		if (!n_bytes) {
			BEGIN_ALLOC();
			result = (calloc)(1, 1);
			END_ALLOC();
			if (result)
				return result;
		}
#endif /* !__MALLOC_ZERO_IS_NONNULL */
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
}

PUBLIC WUNUSED void *
(DCALL Dee_Realloc)(void *ptr, size_t n_bytes) {
	void *result;
#ifndef __REALLOC_ZERO_IS_NONNULL
	if unlikely(!n_bytes)
		n_bytes = 1;
#endif /* !__REALLOC_ZERO_IS_NONNULL */
again:
	BEGIN_ALLOC();
	HEAP_CHECK();
	result = (realloc)(ptr, n_bytes);
	END_ALLOC();
	if unlikely(!result) {
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
}




PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL Dee_TryMalloc)(size_t n_bytes) {
	void *result;
	BEGIN_TRYALLOC();
	HEAP_CHECK();
	result = (malloc)(n_bytes);
	END_TRYALLOC();
#ifndef __MALLOC_ZERO_IS_NONNULL
	if unlikely(!result && !n_bytes) {
		BEGIN_TRYALLOC();
		result = (malloc)(1);
		END_TRYALLOC();
	}
#endif /* !__MALLOC_ZERO_IS_NONNULL */
	return result;
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL Dee_TryCalloc)(size_t n_bytes) {
	void *result;
	BEGIN_TRYALLOC();
	HEAP_CHECK();
	result = (calloc)(1, n_bytes);
	END_TRYALLOC();
#ifndef __MALLOC_ZERO_IS_NONNULL
	if unlikely(!result && !n_bytes) {
		BEGIN_TRYALLOC();
		result = (calloc)(1, 1);
		END_TRYALLOC();
	}
#endif /* !__MALLOC_ZERO_IS_NONNULL */
	return result;
}

PUBLIC WUNUSED void *
(DCALL Dee_TryRealloc)(void *ptr, size_t n_bytes) {
	void *result;
#ifndef __REALLOC_ZERO_IS_NONNULL
	if unlikely(!n_bytes)
		n_bytes = 1;
#endif /* !__REALLOC_ZERO_IS_NONNULL */
	BEGIN_TRYALLOC();
	HEAP_CHECK();
	result = (realloc)(ptr, n_bytes);
	END_TRYALLOC();
	return result;
}

PUBLIC void (DCALL Dee_Free)(void *ptr) {
	DBG_ALIGNMENT_DISABLE();
	HEAP_CHECK();
	(free)(ptr);
	DBG_ALIGNMENT_ENABLE();
}

#ifndef NDEBUG
#ifdef __KOS_SYSTEM_HEADERS__
#define HAVE_DEEDBG_MALLOC 1

#if __KOS_VERSION__ >= 400 && !defined(___malloc_d_defined) && !defined(_malloc_d)
#define DO_MALLOC_D(n_bytes, file, line)       malloc(n_bytes)
#define DO_CALLOC_D(n_bytes, file, line)       calloc(1, n_bytes)
#define DO_REALLOC_D(ptr, n_bytes, file, line) realloc(ptr, n_bytes)
#define DO_FREE_D(ptr, file, line)             free(ptr)
#elif defined(__KERNEL__) && __KOS_VERSION__ < 400
#define DO_MALLOC_D(n_bytes, file, line)       _malloc_d(n_bytes, file, line, NULL, NULL)
#define DO_CALLOC_D(n_bytes, file, line)       _calloc_d(1, n_bytes, file, line, NULL, NULL)
#define DO_REALLOC_D(ptr, n_bytes, file, line) _realloc_d(ptr, n_bytes, file, line, NULL, NULL)
#define DO_FREE_D(ptr, file, line)             _free_d(ptr, file, line, NULL, NULL)
#else /* __KERNEL__ && __KOS_VERSION__ < 400 */
#define DO_MALLOC_D(n_bytes, file, line)       _malloc_d(n_bytes, file, line, NULL)
#define DO_CALLOC_D(n_bytes, file, line)       _calloc_d(1, n_bytes, file, line, NULL)
#define DO_REALLOC_D(ptr, n_bytes, file, line) _realloc_d(ptr, n_bytes, file, line, NULL)
#define DO_FREE_D(ptr, file, line)             _free_d(ptr, file, line, NULL)
#endif /* !__KERNEL__ || __KOS_VERSION__ >= 400 */

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryMalloc)(size_t n_bytes, char const *file, int line) {
	void *result;
	(void)file;
	(void)line;
	HEAP_CHECK();
	BEGIN_TRYALLOC();
	result = DO_MALLOC_D(n_bytes, file, line);
	END_TRYALLOC();
#ifndef __MALLOC_ZERO_IS_NONNULL
	if unlikely(!result && !n_bytes) {
		BEGIN_TRYALLOC();
		result = DO_MALLOC_D(1, file, line);
		END_TRYALLOC();
	}
#endif /* !__MALLOC_ZERO_IS_NONNULL */
	return result;
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryCalloc)(size_t n_bytes, char const *file, int line) {
	void *result;
	(void)file;
	(void)line;
	HEAP_CHECK();
	BEGIN_TRYALLOC();
	result = DO_CALLOC_D(n_bytes, file, line);
	END_TRYALLOC();
#ifndef __MALLOC_ZERO_IS_NONNULL
	if unlikely(!result && !n_bytes) {
		BEGIN_TRYALLOC();
		result = DO_CALLOC_D(1, file, line);
		END_TRYALLOC();
	}
#endif /* !__MALLOC_ZERO_IS_NONNULL */
	return result;
}

PUBLIC WUNUSED void *
(DCALL DeeDbg_TryRealloc)(void *ptr, size_t n_bytes, char const *file, int line) {
	void *result;
	(void)file;
	(void)line;
#ifndef __REALLOC_ZERO_IS_NONNULL
	if unlikely(!n_bytes)
		n_bytes = 1;
#endif /* !__REALLOC_ZERO_IS_NONNULL */
	HEAP_CHECK();
	BEGIN_TRYALLOC();
	result = DO_REALLOC_D(ptr, n_bytes, file, line);
	END_TRYALLOC();
	return result;
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_Malloc)(size_t n_bytes, char const *file, int line) {
	void *result;
	(void)file;
	(void)line;
	HEAP_CHECK();
again:
	BEGIN_ALLOC();
	result = DO_MALLOC_D(n_bytes, file, line);
	END_ALLOC();
	if unlikely(!result) {
#ifndef __MALLOC_ZERO_IS_NONNULL
		if (!n_bytes) {
			BEGIN_ALLOC();
			result = DO_MALLOC_D(1, file, line);
			END_ALLOC();
			if (result)
				return result;
		}
#endif /* !__MALLOC_ZERO_IS_NONNULL */
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_Calloc)(size_t n_bytes, char const *file, int line) {
	void *result;
	(void)file;
	(void)line;
	HEAP_CHECK();
again:
	BEGIN_ALLOC();
	result = DO_CALLOC_D(n_bytes, file, line);
	END_ALLOC();
	if unlikely(!result) {
#ifndef __MALLOC_ZERO_IS_NONNULL
		if (n_bytes) {
			BEGIN_ALLOC();
			result = DO_CALLOC_D(1, file, line);
			END_ALLOC();
			if (result)
				return result;
		}
#endif /* !__MALLOC_ZERO_IS_NONNULL */
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
}

PUBLIC WUNUSED void *
(DCALL DeeDbg_Realloc)(void *ptr, size_t n_bytes, char const *file, int line) {
	void *result;
	(void)file;
	(void)line;
	HEAP_CHECK();
#ifndef __REALLOC_ZERO_IS_NONNULL
	if unlikely(!n_bytes)
		n_bytes = 1;
#endif /* !__REALLOC_ZERO_IS_NONNULL */
again:
	BEGIN_ALLOC();
	result = DO_REALLOC_D(ptr, n_bytes, file, line);
	END_ALLOC();
	if unlikely(!result) {
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
}

PUBLIC void
(DCALL DeeDbg_Free)(void *ptr, char const *file, int line) {
	(void)file;
	(void)line;
	DBG_ALIGNMENT_DISABLE();
	HEAP_CHECK();
	DO_FREE_D(ptr, file, line);
	DBG_ALIGNMENT_ENABLE();
}

#elif defined(_MSC_VER)
#define HAVE_DEEDBG_MALLOC 1


PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryMalloc)(size_t n_bytes, char const *file, int line) {
	void *result;
	(void)file;
	(void)line;
	BEGIN_TRYALLOC();
	HEAP_CHECK();
	result = _malloc_dbg(n_bytes, _NORMAL_BLOCK, file, line);
	END_TRYALLOC();
#ifndef __MALLOC_ZERO_IS_NONNULL
	if unlikely(!result && !n_bytes) {
		BEGIN_TRYALLOC();
		result = _malloc_dbg(1, _NORMAL_BLOCK, file, line);
		END_TRYALLOC();
	}
#endif /* !__MALLOC_ZERO_IS_NONNULL */
	return result;
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryCalloc)(size_t n_bytes, char const *file, int line) {
	void *result;
	(void)file;
	(void)line;
	BEGIN_TRYALLOC();
	HEAP_CHECK();
	result = _calloc_dbg(1, n_bytes, _NORMAL_BLOCK, file, line);
	END_TRYALLOC();
#ifndef __MALLOC_ZERO_IS_NONNULL
	if unlikely(!result && !n_bytes) {
		BEGIN_TRYALLOC();
		result = _calloc_dbg(1, 1, _NORMAL_BLOCK, file, line);
		END_TRYALLOC();
	}
#endif /* !__MALLOC_ZERO_IS_NONNULL */
	return result;
}

PUBLIC WUNUSED void *
(DCALL DeeDbg_TryRealloc)(void *ptr, size_t n_bytes, char const *file, int line) {
	void *result;
	(void)file;
	(void)line;
#ifndef __REALLOC_ZERO_IS_NONNULL
	if unlikely(!n_bytes)
		n_bytes = 1;
#endif /* !__REALLOC_ZERO_IS_NONNULL */
	BEGIN_TRYALLOC();
	HEAP_CHECK();
	result = _realloc_dbg(ptr, n_bytes, _NORMAL_BLOCK, file, line);
	END_TRYALLOC();
	return result;
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_Malloc)(size_t n_bytes, char const *file, int line) {
	void *result;
	(void)file;
	(void)line;
again:
	BEGIN_ALLOC();
	HEAP_CHECK();
	result = _malloc_dbg(n_bytes, _NORMAL_BLOCK, file, line);
	END_ALLOC();
	if unlikely(!result) {
#ifndef __MALLOC_ZERO_IS_NONNULL
		if (!n_bytes) {
			BEGIN_ALLOC();
			result = _malloc_dbg(1, _NORMAL_BLOCK, file, line);
			END_ALLOC();
			if (result)
				return result;
		}
#endif /* !__MALLOC_ZERO_IS_NONNULL */
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_Calloc)(size_t n_bytes, char const *file, int line) {
	void *result;
	(void)file;
	(void)line;
again:
	BEGIN_ALLOC();
	HEAP_CHECK();
	result = _calloc_dbg(1, n_bytes, _NORMAL_BLOCK, file, line);
	END_ALLOC();
	if unlikely(!result) {
#ifndef __MALLOC_ZERO_IS_NONNULL
		if (!n_bytes) {
			BEGIN_ALLOC();
			result = _calloc_dbg(1, 1, _NORMAL_BLOCK, file, line);
			END_ALLOC();
			if (result)
				return result;
		}
#endif /* !__MALLOC_ZERO_IS_NONNULL */
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
}

PUBLIC WUNUSED void *
(DCALL DeeDbg_Realloc)(void *ptr, size_t n_bytes, char const *file, int line) {
	void *result;
	(void)file;
	(void)line;
#ifndef __REALLOC_ZERO_IS_NONNULL
	if unlikely(!n_bytes)
		n_bytes = 1;
#endif /* !__REALLOC_ZERO_IS_NONNULL */
again:
	BEGIN_ALLOC();
	HEAP_CHECK();
	result = _realloc_dbg(ptr, n_bytes, _NORMAL_BLOCK, file, line);
	END_ALLOC();
	if unlikely(!result) {
		if (Dee_CollectMemory(n_bytes))
			goto again;
	}
	return result;
}

PUBLIC void
(DCALL DeeDbg_Free)(void *ptr, char const *file, int line) {
	(void)file;
	(void)line;
	DBG_ALIGNMENT_DISABLE();
	HEAP_CHECK();
	_free_dbg(ptr, _NORMAL_BLOCK);
	DBG_ALIGNMENT_ENABLE();
}

#if defined(_MSC_VER) && _MSC_VER <= 1942 /* Might work for newer versions, too. But untested */

#define no_mans_land_size 4
struct _CrtMemBlockHeader {
	struct _CrtMemBlockHeader *_block_header_next;
	struct _CrtMemBlockHeader *_block_header_prev;
	char const                *_file_name;
	int                        _line_number;
	int                        _block_use;
	size_t                     _data_size;
	long                       _request_number;
	unsigned char              gap[no_mans_land_size];
};

#ifndef _CRT_BLOCK
#define _CRT_BLOCK 2
#endif /* !_CRT_BLOCK */

#define header_from_block(pbData) (((struct _CrtMemBlockHeader *)(pbData)) - 1)

#ifndef EXCEPTION_EXECUTE_HANDLER
#define EXCEPTION_EXECUTE_HANDLER 1
#endif /* !EXCEPTION_EXECUTE_HANDLER */

PRIVATE WUNUSED NONNULL((1)) bool DCALL
validate_header(struct _CrtMemBlockHeader *__restrict hdr) {
	unsigned int i;
	__try {
		struct _CrtMemBlockHeader *neighbor;
		for (i = 0; i < no_mans_land_size; ++i)
			if (hdr->gap[i] != 0xFD)
				goto nope;
		neighbor = hdr->_block_header_next;
		if (neighbor && neighbor->_block_header_prev != hdr)
			goto nope;
		neighbor = hdr->_block_header_prev;
		if (neighbor && neighbor->_block_header_next != hdr)
			goto nope;
		/* Badly named. - Should be `_COUNT_BLOCKS' or `_NUM_BLOCKS'!
		 * `_MAX_BLOCKS' would be the max-valid-block, but this is
		 * number of block types! */
		if (hdr->_block_use >= _MAX_BLOCKS)
			goto nope;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		/* If we failed to access the header for some reason, assume
		 * that something went wrong because the header was malformed. */
		goto nope;
	}
	return true;
nope:
	return false;
}


#define DeeDbg_UntrackAlloc_DEFINED
PUBLIC void *
(DCALL DeeDbg_UntrackAlloc)(void *ptr, char const *file, int line) {
	(void)file;
	(void)line;
	if (ptr) {
		struct _CrtMemBlockHeader *hdr = header_from_block(ptr);
		if (validate_header(hdr)) {
			hdr->_block_use = _CRT_BLOCK;
		}
	}
	return ptr;
}

#elif defined(_MSC_VER) && _MSC_VER <= 1900
extern ATTR_DLLIMPORT void __cdecl _lock(_In_ int _File);
extern ATTR_DLLIMPORT void __cdecl _unlock(_Inout_ int _File);
#define _HEAP_LOCK 4
#define nNoMansLandSize 4

typedef struct _CrtMemBlockHeader {
	struct _CrtMemBlockHeader *pBlockHeaderNext;
	struct _CrtMemBlockHeader *pBlockHeaderPrev;
	char                      *szFileName;
	int                        nLine;
#if __SIZEOF_POINTER__ >= 8 || _MSC_VER >= 1900
	int                        nBlockUse;
	size_t                     nDataSize;
#else /* __SIZEOF_POINTER__ >= 8 || _MSC_VER >= 1900 */
	size_t                     nDataSize;
	int                        nBlockUse;
#endif /* __SIZEOF_POINTER__ < 8 && _MSC_VER < 1900 */
	long                       lRequest;
	unsigned char              gap[nNoMansLandSize];
} _CrtMemBlockHeader;

#define pbData(pblock) ((unsigned char *)((_CrtMemBlockHeader *)pblock + 1))
#define pHdr(pbData)   (((_CrtMemBlockHeader *)pbData) - 1)

#define IGNORE_REQ  0L              /* Request number for ignore block */
#define IGNORE_LINE 0xFEDCBABC      /* Line number for ignore block */
#ifndef _IGNORE_BLOCK
#define _IGNORE_BLOCK 3
#endif /* !_IGNORE_BLOCK */

PRIVATE WUNUSED NONNULL((1)) bool DCALL
validate_header(_CrtMemBlockHeader *__restrict hdr) {
	unsigned int i;
	__try {
		for (i = 0; i < nNoMansLandSize; ++i)
			if (hdr->gap[i] != 0xFD)
				goto nope;
		if (hdr->pBlockHeaderNext->pBlockHeaderPrev != hdr)
			goto nope;
		if (hdr->pBlockHeaderPrev &&
		    hdr->pBlockHeaderPrev->pBlockHeaderNext != hdr)
			goto nope;
		/* Badly named. - Should be `_COUNT_BLOCKS' or `_NUM_BLOCKS'!
		 * `_MAX_BLOCKS' would be the max-valid-block, but this is
		 * number of block types! */
		if (hdr->nBlockUse >= _MAX_BLOCKS)
			goto nope;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		/* If we failed to access the header for some reason, assume
		 * that something went wrong because the header was malformed. */
		goto nope;
	}
	return true;
nope:
	return false;
}

PRIVATE NONNULL((1)) void DCALL
do_unhook(_CrtMemBlockHeader *__restrict hdr) {
	hdr->pBlockHeaderPrev->pBlockHeaderNext = hdr->pBlockHeaderNext;
	hdr->pBlockHeaderNext->pBlockHeaderPrev = hdr->pBlockHeaderPrev;
	hdr->pBlockHeaderNext                   = NULL;
	hdr->pBlockHeaderPrev                   = NULL;
	hdr->szFileName                         = NULL;
	hdr->nLine                              = IGNORE_LINE;
	hdr->nBlockUse                          = _IGNORE_BLOCK;
	hdr->lRequest                           = IGNORE_REQ;
}

#define DeeDbg_UntrackAlloc_DEFINED
PUBLIC void *
(DCALL DeeDbg_UntrackAlloc)(void *ptr, char const *file, int line) {
	(void)file;
	(void)line;
	if (ptr) {
		_CrtMemBlockHeader *hdr;
		DBG_ALIGNMENT_DISABLE();
		HEAP_CHECK();
		_lock(_HEAP_LOCK);
		__try {
			hdr = pHdr(ptr);
			/* We can't untrack the first allocation ever made... */
			if likely(hdr->pBlockHeaderNext) {
				/* Validate the header in case something changes in MSVC,
				 * and our untracking code would SEGFAULT. */
				if (validate_header(hdr)) {
					if (!hdr->pBlockHeaderPrev) {
						void *temp;
						/* Allocate another block, which should be pre-pended before the one we're trying to delete.
						 * ... This work-around is required because we can't actually access the debug-allocation
						 *     list head, which is a PRIVATE symbol `_pFirstBlock'
						 * (This wouldn't have been a problem if MSVC used a self-pointer, instead of a prev-pointer...) */
						temp = _malloc_dbg(1, _NORMAL_BLOCK, file, line);
						if (hdr->pBlockHeaderPrev)
							do_unhook(hdr);
						_free_dbg(temp, _NORMAL_BLOCK);
					} else {
						do_unhook(hdr);
					}
				}
			}
		} __finally {
			_unlock(_HEAP_LOCK);
		}
		DBG_ALIGNMENT_ENABLE();
	}
	return ptr;
}
#endif /* _MSC_VER */
#endif
#endif /* !NDEBUG */

#ifndef HAVE_DEEDBG_MALLOC
/* Fallback: The host does not provide a debug-allocation API. */
PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_Malloc)(size_t n_bytes, char const *file, int line) {
	(void)file;
	(void)line;
	return (Dee_Malloc)(n_bytes);
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_Calloc)(size_t n_bytes, char const *file, int line) {
	(void)file;
	(void)line;
	return (Dee_Calloc)(n_bytes);
}

PUBLIC WUNUSED void *
(DCALL DeeDbg_Realloc)(void *ptr, size_t n_bytes, char const *file, int line) {
	(void)file;
	(void)line;
	return (Dee_Realloc)(ptr, n_bytes);
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryMalloc)(size_t n_bytes, char const *file, int line) {
	(void)file;
	(void)line;
	return (Dee_TryMalloc)(n_bytes);
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbg_TryCalloc)(size_t n_bytes, char const *file, int line) {
	(void)file;
	(void)line;
	return (Dee_TryCalloc)(n_bytes);
}

PUBLIC WUNUSED void *
(DCALL DeeDbg_TryRealloc)(void *ptr, size_t n_bytes,
                          char const *file, int line) {
	(void)file;
	(void)line;
	return (Dee_TryRealloc)(ptr, n_bytes);
}

PUBLIC void
(DCALL DeeDbg_Free)(void *ptr, char const *file, int line) {
	(void)file;
	(void)line;
	(Dee_Free)(ptr);
}
#endif /* !HAVE_DEEDBG_MALLOC */

#ifndef DeeDbg_UntrackAlloc_DEFINED
PUBLIC void *
(DCALL DeeDbg_UntrackAlloc)(void *ptr, char const *file, int line) {
	(void)file;
	(void)line;
	return ptr;
}
#endif /* !DeeDbg_UntrackAlloc_DEFINED */

DECL_END
#endif /* !CONFIG_EXPERIMENTAL_CUSTOM_HEAP */

#endif /* !GUARD_DEEMON_RUNTIME_HEAP_C */
