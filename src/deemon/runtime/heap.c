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

/* Disable these attributes in here -- we must routinely write past the end
 * of our own allocations, because how else are we supposed to manage adjacent
 * heap blocks and/or initialize footers.
 *
 * Without this, we get crashes: "*** buffer overflow detected ***: terminated"
 *
 * I think this is due to GCC intentionally crashing the program in places where
 * we intentionally write past the end of a heap block in "heap.c" to overwrite
 * the block's footer with extra debug info... */
#undef ATTR_ALLOC_SIZE
#define ATTR_ALLOC_SIZE(ppars) /* Nothing */
#undef ATTR_ALLOC_ALIGN
#define ATTR_ALLOC_ALIGN(pari) /* Nothing */

#include <deemon/alloc.h>            /* Dee_CollectMemory */
#include <deemon/format.h>           /* PRFuSIZ */
#include <deemon/gc.h>               /* Dee_GC_OBJECT_OFFSET, Dee_gc_head */
#include <deemon/heap.h>             /* DeeHeap_*, Dee_HEAPCHUNK_ALIGN, Dee_HEAP_M_*, Dee_heap_mallinfo, Dee_heapchunk, Dee_heapchunk_getnext, Dee_heapregion, Dee_heapregion_*, Dee_heaptail */
#include <deemon/module.h>           /* DeeModule*, Dee_module_libentry, Dee_module_object, _Dee_MODULE_FNOADDR */
#include <deemon/system-features.h>  /* CONFIG_HAVE_*, bzero, memcpy, mmap64, remove, strlen */
#include <deemon/thread.h>           /* DeeThreadObject, DeeThread_Self */
#include <deemon/types.h>            /* DREF, DeeObject, Dee_TYPE, Dee_refcnt_t, Dee_ssize_t, ITER_DONE */
#include <deemon/util/atomic.h>      /* atomic_* */
#include <deemon/util/lock.h>        /* Dee_ATOMIC_LOCK_INIT, Dee_atomic_lock_*, Dee_atomic_rwlock_*, Dee_shared_lock_* */
#include <deemon/util/rlock.h>       /* Dee_RATOMIC_LOCK_INIT, Dee_RSHARED_LOCK_INIT, Dee_ratomic_lock_*, Dee_rshared_lock_* */
#include <deemon/util/slab-config.h> /* Dee_SLAB_CHUNKSIZE_MAX */
#include <deemon/util/slab.h>        /* Dee_slab_page, Dee_slab_page_rawtrim */

#include <hybrid/align.h>         /* CEILDIV, IS_ALIGNED */
#include <hybrid/host.h>          /* __ARCH_PAGESIZE, __arm__, __i386__, __linux__, __x86_64__ */
#include <hybrid/overflow.h>      /* OVERFLOW_UADD */
#include <hybrid/sequence/list.h> /* LIST_*, SLIST_* */
#include <hybrid/typecore.h>      /* __*_TYPE__, __CHAR_BIT__, __SIZEOF_POINTER__, __SIZE_C */

#include "slab.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* offsetof, size_t */
#include <stdint.h>  /* UINT32_C, uintptr_t */

/*  */
#include <deemon/util/kos-compat.h>

#undef Dee_TryMalloc
#undef Dee_TryCalloc
#undef Dee_TryRealloc
#undef Dee_TryReallocInPlace
#undef Dee_TryMemalign
#undef Dee_MallocUsableSize
#undef Dee_MallocUsableSizeNonNull
#undef Dee_Free

#ifndef SIZE_MAX
#include <hybrid/limitcore.h> /* __SIZE_MAX__ */
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

#undef container_of
#define container_of COMPILER_CONTAINER_OF
#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN


/* ============ Threading configuration ============ */
#if defined(CONFIG_NO_THREADS)
#define USE_LOCKS 0
#else /* CONFIG_NO_THREADS */
#define USE_LOCKS 1
#define USE_PENDING_FREE_LIST 1
/* Always enable in SMP -- performance gain is too significant to
 * ignore, even when compared to the slight increase in code size */
#if 1 /*&& !defined(__OPTIMIZE_SIZE__)*/
#define USE_PER_THREAD_MSTATE 1
#endif /* !__OPTIMIZE_SIZE__ */
#endif /* !CONFIG_NO_THREADS */
#ifndef USE_PER_THREAD_MSTATE
#define USE_PER_THREAD_MSTATE 0
#endif /* !USE_PER_THREAD_MSTATE */



/* ============ Debug configuration ============ */
#if !defined(NDEBUG) && !defined(__OPTIMIZE_SIZE__) && 1
#define DL_DEBUG_EXTERNAL 1
#else
#define DL_DEBUG_EXTERNAL 0
#endif


/* BEGIN --- Not part of dlmalloc core: leak detection */
/* BEGIN --- Not part of dlmalloc core: leak detection */
/* BEGIN --- Not part of dlmalloc core: leak detection */

/* - LEAK_DETECTION --------------------------------------------
 *   Enables a memory leak detector suitable for dumping all
 *   memory that is (still) allocated at the time of leaks being
 *   dumped. See below for more details and implementations.
 * -------------------------------------------------------------
 *
 * CAUTION: Leak detection is a bit of a bottleneck:
 *
 * LEAK_DETECTION_METHOD_NONE         time make computed-operators    real    0m4.482s   100%
 * LEAK_DETECTION_METHOD_IN_TAIL      time make computed-operators    real    0m7.234s   161%
 * LEAK_DETECTION_METHOD_OOB_RBTREE   time make computed-operators    real    0m20.685s  462%
 * LEAK_DETECTION_METHOD_OOB_LLRBTREE time make computed-operators    real    0m27.545s  615%
 *
 * LEAK_DETECTION_METHOD_NONE         time deemon util/test.dee       real    0m0.881s   100%
 * LEAK_DETECTION_METHOD_IN_TAIL      time deemon util/test.dee       real    0m0.965s   110%
 * LEAK_DETECTION_METHOD_OOB_RBTREE   time deemon util/test.dee       real    0m1.013s   115%
 * LEAK_DETECTION_METHOD_OOB_LLRBTREE time deemon util/test.dee       real    0m1.207s   137%
 *
 * As you can see, `LEAK_DETECTION_METHOD_IN_TAIL=1' is ****MUCH**** faster than the
 * old methods of using an out-of-band RBTREE to describe memory leaks. It is however
 * still noticeably slower (but not as painfully so) than LEAK_DETECTION_METHOD_NONE.
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
 * cores try to insert elements at the same time, there is only a 1/32 chance for that
 * insert succeeding for any one of them.
 *
 * XXX: This could also be improved upon further by having multiple pending lists, and
 *      selecting one at random based on the address of the "leak_footer" that's being
 *      inserted into these lists (though that would require hard-coding the # of
 *      lists that should exist -- or maybe not: why not allocate the vector of lists
 *      dynamically once it is first needed, and then have it's element count match
 *      the next-power-of-2 of the # of CPU in the system?) */
#define LEAK_DETECTION_METHOD_NONE         0 /* Don't do any leak detection */
#define LEAK_DETECTION_METHOD_IN_TAIL      1 /* Requires FOOTERS=1, and store debug info in a secondary heap block there */
#define LEAK_DETECTION_METHOD_OOB_RBTREE   2 /* Use an out-of-band R/B-Tree */
#define LEAK_DETECTION_METHOD_OOB_LLRBTREE 3 /* Use an out-of-band left-leaning R/B-Tree */
#if DL_DEBUG_EXTERNAL
#if 1 /* With how much faster this method is, there's really no reason to ever use the other methods,
       * except to maybe test with FOOTERS=0 after also disabling 'USE_PER_THREAD_MSTATE=1'. */
#define LEAK_DETECTION LEAK_DETECTION_METHOD_IN_TAIL
#elif 1
#define LEAK_DETECTION LEAK_DETECTION_METHOD_OOB_RBTREE
#elif 1
#define LEAK_DETECTION LEAK_DETECTION_METHOD_OOB_LLRBTREE
#endif
#endif /* DL_DEBUG_EXTERNAL */
#ifndef LEAK_DETECTION
#define LEAK_DETECTION LEAK_DETECTION_METHOD_NONE
#endif /* !LEAK_DETECTION */

/* Special extension to leak detection that uses a GC-based approach to discover leaks,
 * where the contents of static variables and the stack/registers of all threads are
 * searched for what (look like) pointers to heap structures, which are then used to
 * check which heap blocks are still reachable.
 *
 * Since this relies on some arch-specifics (mainly: stack/register enumeration), this
 * sort of leak detection might not be possible all the time, but when it is possible,
 * it allows one to check for memory leaks in a running system, as opposed to only at
 * the very end, just before exiting. */
#if (LEAK_DETECTION == LEAK_DETECTION_METHOD_IN_TAIL && \
     !defined(CONFIG_NO_DEX) && 0) /* TODO */
#define LEAK_DETECTION_GC 1
#endif /* ... */
#ifndef LEAK_DETECTION_GC
#define LEAK_DETECTION_GC 0
#endif /* !LEAK_DETECTION_GC */


/* ========== USE_PER_MSPACE_ALLOC_ID
 * With lots of running threads, allocating IDs from "alloc_id_count" tends to
 * become a major bottleneck since lots of thread will be doing lots of allocations
 * all the time (causing atomic conflicts), and all those allocations need to be
 * given their own (unique) allocation ID.
 *
 * The solution is to pre-allocate a bunch of IDs within the "struct malloc_state"
 * of the associated allocation (these pre-allocations still happen by doing atomic
 * operations on "alloc_id_count"), and then allocate IDs from said pre-allocation.
 *
 * This doesn't actually change anything in regards to the behavior of alloc IDs,
 * neither when using in a single-threaded environment, nor in a multi-threaded one:
 * - When only a single thread exists, "tls_mspace()" will never be used, and all
 *   allocations happen via the global "gm" mstate. Here, it doesn't matter if we'd
 *   directly use the "alloc_id_count" or "gm", either would be fine so-long as we
 *   behave consistently (which we do by always using "gm" in this case)
 * - When there are multiple thread doing allocations simultaneously, it is already
 *   undefined which thread will end up getting which ID (only that IDs will always
 *   be distinct across threads). This is still the case when each thread allocates
 *   a bunch of IDs prematurely, and then assigns them as needed. The only difference
 *   here is that some IDs may end up going unused.
 *
 * ========== USE_PER_MSPACE_LEAK_OPS
 * Same as `USE_PER_MSPACE_ALLOC_ID', but replaces:
 * - "leaks_pending_insert"
 * - "leaks_pending_remove"
 * with per-mspace atomic linked lists, thus also reducing atomic contention on the
 * async insert/remove task lists of memory leaks also.
 *
 * Performance stats  (time deemon util/scripts/fixincludes.dee)
 * 
 * USE_PER_MSPACE_*
 *   ALLOC_ID    LEAK_OPS     timings (real, 3 random runs each)
 *          1           1     0m16.774s, 0m16.904s, 0m17.061s
 *          1           0     0m17.278s, 0m17.361s, 0m17.732s
 *          0           0     0m17.855s, 0m18.644s, 0m18.658s
 */
#if (LEAK_DETECTION == LEAK_DETECTION_METHOD_IN_TAIL && USE_PER_THREAD_MSTATE)
#define USE_PER_MSPACE_ALLOC_ID 1
#if 0
/* TODO: Instead of all these different methods to speed up inserts/removes of leaks into the global
 *       ring of tracked memory leaks, why not just keep track of memory leaks on a per-mstate basis?
 *       I mean: there is an API to enumerate all TLS mspaces, and DeeHeap_DumpMemoryLeaks() can just
 *       be re-written to simply enumerate those spaces in order to find all tracked memory leaks! */
#else
#define USE_PER_MSPACE_LEAK_OPS 1
#endif
#endif /* ... */
#ifndef USE_PER_MSPACE_ALLOC_ID
#define USE_PER_MSPACE_ALLOC_ID 0
#endif /* !USE_PER_MSPACE_ALLOC_ID */
#ifndef USE_PER_MSPACE_LEAK_OPS
#define USE_PER_MSPACE_LEAK_OPS 0
#endif /* !USE_PER_MSPACE_LEAK_OPS */
#undef MSTATE_EXTRA_FIELDS_1
#if USE_PER_MSPACE_ALLOC_ID
#define MSTATE_ALLOC_MASK     0xffff /* Allocation mask (upper limit on how many IDs to pre-allocate) */
#define MSTATE_EXTRA_FIELDS_1 size_t ms_alloc_id /* [lock(ATOMIC)] Per-mstate "alloc_id_count" */
#endif /* USE_PER_MSPACE_ALLOC_ID */
#if USE_PER_MSPACE_LEAK_OPS
#ifndef LEAK_FOOTER_SLIST_DEFINED
#define LEAK_FOOTER_SLIST_DEFINED
DECL_BEGIN
struct leak_footer;
SLIST_HEAD(leak_footer_slist, leak_footer);
DECL_END
#endif /* !LEAK_FOOTER_SLIST_DEFINED */
#define MSTATE_LEAKS_PENDING_NEXT__UNBOUND ((struct malloc_state *)NULL)      /* mstate isn't linked into global list of mstates with pending leaks */
#define MSTATE_LEAKS_PENDING_NEXT__NONE    ((struct malloc_state *)ITER_DONE) /* mstate is the last element in the global list of mstates with pending leaks */
#define MSTATE_EXTRA_FIELDS_2                                                                                                    \
	struct malloc_state     *ms_leaks_pending_next;   /* [lock(ATOMIC)] Next mstate, or one of `MSTATE_LEAKS_PENDING_NEXT__*' */ \
	struct leak_footer_slist ms_leaks_pending_insert; /* [lock(ATOMIC)] Pending inserts */                                       \
	struct leak_footer_slist ms_leaks_pending_remove; /* [lock(ATOMIC)] Pending removes */
#endif /* USE_PER_MSPACE_LEAK_OPS */

/* END --- Not part of dlmalloc core: leak detection */
/* END --- Not part of dlmalloc core: leak detection */
/* END --- Not part of dlmalloc core: leak detection */


/* It may not be __ALIGNOF_MAX_ALIGN_T__, but it's "just what 'a needed" */
#define MALLOC_ALIGNMENT (size_t)(__SIZEOF_POINTER__ * 2)

/* We use dlmalloc() to implement Dee_TryMalloc() at its lowest
 * -- as such, nothing needs to be done as the "failure action" */
#if 1
#define MALLOC_FAILURE_ACTION /* Nothing */
#else
#define MALLOC_FAILURE_ACTION Dee_BREAKPOINT()
#endif


/* Always keep (unnecessary) footers disabled -- they don't really improve safety when on,
 * since their presence causes allocations to require an additional word of memory, there
 * is a chance that alignment requirements ceil this to enough memory such that buffer-
 * overruns don't end up being detected. */
#if USE_PER_THREAD_MSTATE
#define FOOTERS 1 /* Needed for `Dee_Free()' to detect source mspace! */
#elif LEAK_DETECTION == LEAK_DETECTION_METHOD_IN_TAIL
#define FOOTERS 1 /* Needed for `DeeDbg_Malloc()' to store debug info in the tail of heap blocks */
#else
#define FOOTERS 0
#endif



/* ============= Enable & configure support for "struct Dee_heapregion" */
#define FLAG4_BIT_HEAP_REGION 1

/* Handler special flag: we need to be able to detect if a statically allocated
 * object from a heap region has been destroyed via its "ob_refcnt" in order to
 * safely handle (technically illegal) references to static, non-final,
 * constant objects within another module. If during object destruction, it is
 * found that a "Dee_refcnt_t"-sized word at one of the following offsets into
 * a heap block's user-area is "0", then this "0" must **NOT** be overwritten
 * by "DL_DEBUG_MEMSET_FREE_PATTERN":
 * - offsetof(DeeObject, ob_refcnt)
 * - offsetof(struct Dee_gc_head, gc_object.ob_refcnt) */
STATIC_ASSERT(IS_ALIGNED(offsetof(DeeObject, ob_refcnt), sizeof(Dee_refcnt_t)));
STATIC_ASSERT(IS_ALIGNED(Dee_GC_OBJECT_OFFSET + offsetof(DeeObject, ob_refcnt), sizeof(Dee_refcnt_t)));
#define FLAG4_BIT_HEAP_REGION_REQUIRES_RESTRICTED_DL_DEBUG_MEMSET_FREE 1
#define HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_WORD_T Dee_refcnt_t
#define HEAP_REGION_RESTRICTED_DL_DEBUG_MEMSET_CANSET(chunk_offset, p_word)         \
	(!((chunk_offset) == offsetof(DeeObject, ob_refcnt) ||                          \
	   (chunk_offset) == (Dee_GC_OBJECT_OFFSET + offsetof(DeeObject, ob_refcnt)) || \
	   (atomic_read(iter) == 0)))

/* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE: Make provisioning for:
 * - DeeDbgHeap_AddHeapRegion
 * - DeeDbgHeap_DelHeapRegion */
#define FLAG4_BIT_HEAP_REGION_DBG_DISPOSE \
	(LEAK_DETECTION == LEAK_DETECTION_METHOD_IN_TAIL)
/* ============= */



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

/* Disable certain mspace-specific functions */
#define NO_MSPACE_FREE                1 /* 'Dee_Free()' is able to detect mspace chunks (via "FOOTERS"), so mspace_free() isn't needed */
#define NO_MSPACE_REALLOC             1 /* Like 'Dee_Free()', 'Dee_Realloc()' is also able to detect custom-mspace chunks */
#define NO_MSPACE_REALLOC_IN_PLACE    1 /* ... */
#define NO_MSPACE_USABLE_SIZE         1 /* Unnecessary dummy (same as `Dee_MallocUsableSize()') */
#define NO_MSPACE_TRACK_LARGE_CHUNKS  1 /* Not needed (default init by `create_mspace()' is what we want) */
#define NO_CREATE_MSPACE_WITH_BASE    1 /* Unused: only need create_mspace() */
#define NO_MSPACE_CALLOC              1 /* Unused: the dlcalloc() -> dlmalloc() -> mspace_malloc() chain already does the right thing */
#define NO_MSPACE_MEMALIGN            1 /* Unused: only dlmemalign() is needed */
#define NO_MSPACE_FOOTPRINT           1 /* Unused: integrated into dlmalloc_footprint() */
#define NO_MSPACE_MAX_FOOTPRINT       1 /* Unused: integrated into dlmalloc_max_footprint() */
#define NO_MSPACE_FOOTPRINT_LIMIT     1 /* Unused: integrated into dlmalloc_footprint_limit() */
#define NO_MSPACE_SET_FOOTPRINT_LIMIT 1 /* Unused: integrated into dlmalloc_set_footprint_limit() */
#define NO_MSPACE_TRIM                1 /* Unused: integrated into dlmalloc_trim() */
#define NO_MSPACE_MALLINFO            1
#define NO_DESTROY_MSPACE             1 /* Per-thread heaps must be re-used as needed and cannot be destroyed (if there's a way to get rid of Dee_UntrackAlloc(), this might come back one day...) */


/* Use the configuration constants exposed by our API */
#undef M_TRIM_THRESHOLD
#undef M_GRANULARITY
#undef M_MMAP_THRESHOLD
#define M_TRIM_THRESHOLD Dee_HEAP_M_TRIM_THRESHOLD
#define M_GRANULARITY    Dee_HEAP_M_GRANULARITY
#define M_MMAP_THRESHOLD Dee_HEAP_M_MMAP_THRESHOLD

/* Configure API for deemon */
#define dlcalloc_SINGLE_PARAMETER 1
#define DL_DECL_DEFAULT static
#define DL_IMPL_DEFAULT static

/* When not doing any leak detection, then dlmalloc's internal methods can be exposed directly! */
#define DIRECTLY_DEFINE_DEEMON_PUBLIC_API (LEAK_DETECTION == LEAK_DETECTION_METHOD_NONE)
#if DIRECTLY_DEFINE_DEEMON_PUBLIC_API
#define DL_DECL_dlmalloc DFUNDEF
#define DL_IMPL_dlmalloc PUBLIC
#define DL_CC_dlmalloc   DCALL
#define dlmalloc         Dee_TryMalloc
#define Dee_TryMalloc_DEFINED

#define DL_DECL_dlcalloc DFUNDEF
#define DL_IMPL_dlcalloc PUBLIC
#define DL_CC_dlcalloc   DCALL
#define dlcalloc         Dee_TryCalloc
#define Dee_TryCalloc_DEFINED

#define DL_DECL_dlfree DFUNDEF
#define DL_IMPL_dlfree PUBLIC
#define DL_CC_dlfree   DCALL
#define dlfree         Dee_Free
#define Dee_Free_DEFINED

#define DL_DECL_dlrealloc DFUNDEF
#define DL_IMPL_dlrealloc PUBLIC
#define DL_CC_dlrealloc   DCALL
#define dlrealloc         Dee_TryRealloc
#define Dee_TryRealloc_DEFINED

#define DL_DECL_dlmemalign DFUNDEF
#define DL_IMPL_dlmemalign PUBLIC
#define DL_CC_dlmemalign   DCALL
#define dlmemalign         Dee_TryMemalign
#define Dee_TryMemalign_DEFINED

#define DL_DECL_dlrealloc_in_place DFUNDEF
#define DL_IMPL_dlrealloc_in_place PUBLIC
#define DL_CC_dlrealloc_in_place   DCALL
#define dlrealloc_in_place         Dee_TryReallocInPlace
#define Dee_TryReallocInPlace_DEFINED
#else /* DIRECTLY_DEFINE_DEEMON_PUBLIC_API */
#define dlfree_CHECKS_NULL    0 /* Not needed (already done by caller) */
#define dlrealloc_CHECKS_NULL 0 /* Not needed (already done by caller) */
#if LEAK_DETECTION == LEAK_DETECTION_METHOD_IN_TAIL
#define DLREALLOC_IN_PLACE_CHECKS_NULL 0 /* Not needed (already done by caller) */
#else /* LEAK_DETECTION == LEAK_DETECTION_METHOD_IN_TAIL */
#define DL_DECL_dlrealloc_in_place DFUNDEF
#define DL_IMPL_dlrealloc_in_place PUBLIC
#define DL_CC_dlrealloc_in_place   DCALL
#define dlrealloc_in_place         Dee_TryReallocInPlace
#define Dee_TryReallocInPlace_DEFINED
#endif /* LEAK_DETECTION != LEAK_DETECTION_METHOD_IN_TAIL */
#endif /* !DIRECTLY_DEFINE_DEEMON_PUBLIC_API */

#define dlrealloc_ZERO_BYTES_FREES 0 /* Nope: Dee_Realloc(p, 0) resizes to 0-sized block, but returns non-NULL */
#define dlmalloc_ZERO_RETURNS_NULL 0 /* Nope: Dee_Malloc(0) returns 0-sized, but non-NULL, block */

#define DL_DECL_dlmallopt DFUNDEF
#define DL_IMPL_dlmallopt PUBLIC
#define DL_CC_dlmallopt   DCALL
#define dlmallopt DeeHeap_SetOpt
#define dlmallopt_SECOND_PARAMETER_IS_SIZE_T 1

#if !NO_MALLOC_FOOTPRINT
#define DL_DECL_dlmalloc_footprint DFUNDEF
#define DL_IMPL_dlmalloc_footprint PUBLIC
#define DL_CC_dlmalloc_footprint   DCALL
#define dlmalloc_footprint DeeHeap_Footprint

#define DL_DECL_dlmalloc_max_footprint DFUNDEF
#define DL_IMPL_dlmalloc_max_footprint PUBLIC
#define DL_CC_dlmalloc_max_footprint   DCALL
#define dlmalloc_max_footprint DeeHeap_MaxFootprint

#define DL_DECL_dlmalloc_footprint_limit DFUNDEF
#define DL_IMPL_dlmalloc_footprint_limit PUBLIC
#define DL_CC_dlmalloc_footprint_limit   DCALL
#define dlmalloc_footprint_limit DeeHeap_GetFootprintLimit

#define DL_DECL_dlmalloc_set_footprint_limit DFUNDEF
#define DL_IMPL_dlmalloc_set_footprint_limit PUBLIC
#define DL_CC_dlmalloc_set_footprint_limit   DCALL
#define dlmalloc_set_footprint_limit DeeHeap_SetFootprintLimit
#define dlmalloc_set_footprint_limit_RETURNS_OLD_VALUE 1
#endif /* !NO_MALLOC_FOOTPRINT */

#if !NO_MALLINFO
#define DL_DECL_dlmallinfo DFUNDEF
#define DL_IMPL_dlmallinfo PUBLIC
#define DL_CC_dlmallinfo   DCALL
#define dlmallinfo      DeeHeap_MallInfo
#define STRUCT_mallinfo struct Dee_heap_mallinfo

#define STRUCT_mallinfo_get_arena(x)       (x)->hmi_arena
#define STRUCT_mallinfo_set_arena(x, v)    (void)((x)->hmi_arena = (v))
#define STRUCT_mallinfo_get_ordblks(x)     (x)->hmi_ordblks
#define STRUCT_mallinfo_set_ordblks(x, v)  (void)((x)->hmi_ordblks = (v))
#define STRUCT_mallinfo_get_hblkhd(x)      (x)->hmi_hblkhd
#define STRUCT_mallinfo_set_hblkhd(x, v)   (void)((x)->hmi_hblkhd = (v))
#define STRUCT_mallinfo_get_usmblks(x)     (x)->hmi_usmblks
#define STRUCT_mallinfo_set_usmblks(x, v)  (void)((x)->hmi_usmblks = (v))
#define STRUCT_mallinfo_get_uordblks(x)    (x)->hmi_uordblks
#define STRUCT_mallinfo_set_uordblks(x, v) (void)((x)->hmi_uordblks = (v))
#define STRUCT_mallinfo_get_fordblks(x)    (x)->hmi_fordblks
#define STRUCT_mallinfo_set_fordblks(x, v) (void)((x)->hmi_fordblks = (v))
#define STRUCT_mallinfo_get_keepcost(x)    (x)->hmi_keepcost
#define STRUCT_mallinfo_set_keepcost(x, v) (void)((x)->hmi_keepcost = (v))
#endif /* !NO_MALLINFO */

#if !NO_MALLOC_TRIM
#define DL_DECL_dlmalloc_trim DFUNDEF
#define DL_IMPL_dlmalloc_trim PUBLIC
#define DL_CC_dlmalloc_trim   DCALL
#define dlmalloc_trim DeeHeap_Trim
#define dlmalloc_trim_RETURNS_SIZE_T 1
#endif /* !NO_MALLOC_TRIM */

#ifdef Dee_SLAB_CHUNKSIZE_MAX
/* Trim the slab page cache (do this first, in case said cache has
 * been implemented in terms of the fallback `Dee_Memalign()' impl) */
#define dlmalloc_trim_PREHOOK(p_result, pad) \
	*(p_result) += Dee_slab_page_rawtrim(pad)
#endif /* Dee_SLAB_CHUNKSIZE_MAX */

/* Integrate assert checks */
#define DL_ASSERT  Dee_ASSERT
#define DL_ASSERTF Dee_ASSERTF

/* Integrate lock type */
#define DL_LOCK_T           Dee_atomic_lock_t
#define DL_LOCK_INIT_STATIC Dee_ATOMIC_LOCK_INIT
#define DL_LOCK_INIT        Dee_atomic_lock_cinit
#define DL_LOCK_ACQUIRED    Dee_atomic_lock_acquired
#define DL_LOCK_AVAILABLE   Dee_atomic_lock_available
#define DL_LOCK_ACQUIRE     Dee_atomic_lock_acquire
#define DL_LOCK_RELEASE     Dee_atomic_lock_release
#define DL_LOCK_TRYACQUIRE  Dee_atomic_lock_tryacquire
#define DL_LOCK_WAITFOR     Dee_atomic_lock_waitfor

/* Integrate re-entrant lock type */
#if 1 /* Use atomic locks -- shared locks use futex, which might require Dee_TryMalloc() for its impl! */
#define DL_REENTRANT_LOCK_T           Dee_ratomic_lock_t
#define DL_REENTRANT_LOCK_INIT_STATIC Dee_RATOMIC_LOCK_INIT
#define DL_REENTRANT_LOCK_ACQUIRE     Dee_ratomic_lock_acquire
#define DL_REENTRANT_LOCK_RELEASE     Dee_ratomic_lock_release
#else
#define DL_REENTRANT_LOCK_T           Dee_rshared_lock_t
#define DL_REENTRANT_LOCK_INIT_STATIC Dee_RSHARED_LOCK_INIT
#define DL_REENTRANT_LOCK_ACQUIRE     Dee_rshared_lock_acquire_noint
#define DL_REENTRANT_LOCK_RELEASE     Dee_rshared_lock_release
#endif

/* Integrate TLS heap context */
#define DL_TLS_VARS       DeeThreadObject *const _tls_me = DeeThread_Self();
#define DL_TLS_GETHEAP(r) (void)(*(r) = _tls_me->t_heap)
#define DL_TLS_SETHEAP(v) (void)(_tls_me->t_heap = (v))

/* Integrate heap region ABI */
#define STRUCT_dlheapchunk  struct Dee_heapchunk
#define STRUCT_dlheaptail   struct Dee_heaptail
#define STRUCT_dlheapregion struct Dee_heapregion

/* Tell dlmalloc() which system features are available... */
#if !defined(CONFIG_HAVE_mmap) && defined(CONFIG_HAVE_mmap64)
#define CONFIG_HAVE_mmap 1
#undef mmap
#define mmap mmap64
#endif /* mmap = mmap64 */
#ifdef CONFIG_HAVE_VirtualAlloc
#define DL_CONFIG_HAVE_VirtualAlloc 1
#endif /* CONFIG_HAVE_VirtualAlloc */
#ifdef CONFIG_HAVE_mmap
#define DL_CONFIG_HAVE_mmap 1
#endif /* CONFIG_HAVE_mmap */
#ifdef CONFIG_HAVE_munmap
#define DL_CONFIG_HAVE_munmap 1
#endif /* CONFIG_HAVE_munmap */
#ifdef CONFIG_HAVE_PROT_READ
#define DL_CONFIG_HAVE_PROT_READ 1
#endif /* CONFIG_HAVE_PROT_READ */
#ifdef CONFIG_HAVE_PROT_WRITE
#define DL_CONFIG_HAVE_PROT_WRITE 1
#endif /* CONFIG_HAVE_PROT_WRITE */
#ifdef CONFIG_HAVE_MAP_PRIVATE
#define DL_CONFIG_HAVE_MAP_PRIVATE 1
#endif /* CONFIG_HAVE_MAP_PRIVATE */
#ifdef CONFIG_HAVE_MAP_ANONYMOUS
#define DL_CONFIG_HAVE_MAP_ANONYMOUS 1
#endif /* CONFIG_HAVE_MAP_ANONYMOUS */
#ifdef CONFIG_HAVE_sbrk
#define DL_CONFIG_HAVE_sbrk 1
#endif /* CONFIG_HAVE_sbrk */
#ifdef CONFIG_HAVE_mremap
#define DL_CONFIG_HAVE_mremap 1
#endif /* CONFIG_HAVE_mremap */
#ifdef CONFIG_HAVE_MREMAP_MAYMOVE
#define DL_CONFIG_HAVE_MREMAP_MAYMOVE 1
#endif /* CONFIG_HAVE_MREMAP_MAYMOVE */
#ifdef CONFIG_HAVE_malloc
#define DL_CONFIG_HAVE_malloc 1
#endif /* CONFIG_HAVE_malloc */
#ifdef CONFIG_HAVE_calloc
#define DL_CONFIG_HAVE_calloc 1
#endif /* CONFIG_HAVE_calloc */
#ifdef CONFIG_HAVE_free
#define DL_CONFIG_HAVE_free 1
#endif /* CONFIG_HAVE_free */
#ifdef CONFIG_HAVE_time
#define DL_CONFIG_HAVE_time 1
#endif /* CONFIG_HAVE_time */
#ifdef CONFIG_HAVE_getpagesize
#define DL_CONFIG_HAVE_getpagesize 1
#endif /* CONFIG_HAVE_getpagesize */
#ifdef CONFIG_HAVE_sysconf
#define DL_CONFIG_HAVE_sysconf 1
#endif /* CONFIG_HAVE_sysconf */
#ifdef CONFIG_HAVE_SYS_PARAM_H
#define DL_CONFIG_HAVE_SYS_PARAM_H 1
#endif /* CONFIG_HAVE_SYS_PARAM_H */


/* Pull in (heavily modified) dlmalloc sources */
DECL_END
#include "../../libdlmalloc/dlmalloc.c.inl"
DECL_BEGIN






/* Deemon public heap API */
#if !DIRECTLY_DEFINE_DEEMON_PUBLIC_API
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL Dee_TryMalloc)(size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL Dee_TryCalloc)(size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL Dee_TryRealloc)(void *ptr, size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL Dee_TryReallocInPlace)(void *ptr, size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *(DCALL Dee_TryMemalign)(size_t min_alignment, size_t n_bytes);
DFUNDEF ATTR_PURE WUNUSED size_t (DCALL Dee_MallocUsableSize)(void *ptr);
DFUNDEF void (DCALL Dee_Free)(void *ptr);
#endif /* !DIRECTLY_DEFINE_DEEMON_PUBLIC_API */
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL DeeDbg_TryMalloc)(size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL DeeDbg_TryCalloc)(size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED void *(DCALL DeeDbg_TryRealloc)(void *ptr, size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *(DCALL DeeDbg_TryMemalign)(size_t min_alignment, size_t n_bytes, char const *file, int line);

DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL Dee_Malloc)(size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL Dee_Calloc)(size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL Dee_Realloc)(void *ptr, size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *(DCALL Dee_Memalign)(size_t min_alignment, size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL DeeDbg_Malloc)(size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL DeeDbg_Calloc)(size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED void *(DCALL DeeDbg_Realloc)(void *ptr, size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *(DCALL DeeDbg_Memalign)(size_t min_alignment, size_t n_bytes, char const *file, int line);


#if !DIRECTLY_DEFINE_DEEMON_PUBLIC_API

/* Memory leak detector:
 * - Built directory on-top of dlmalloc(), dlrealloc(), dlfree() (meaning no
 *   extra dependencies on any internals of the underlying malloc implementation)
 * - No extra headers added to- or required for heap chunks allocated by underlying
 *   memory allocator.
 */
#if LEAK_DETECTION == LEAK_DETECTION_METHOD_IN_TAIL

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
 *   >>     int                 lf_line;  // Line where "lf_chunk" was allocated
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
 *   >> void *tail = (byte_t *)p + s;
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
#if LEAK_DETECTION_GC || FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
#define leak_footer_line_t __INTPTR_HALF_TYPE__
	__INTPTR_HALF_TYPE__  lf_line;  /* [lock(OWNER(lf_chunk))] Line where "lf_chunk" was allocated */
	__UINTPTR_HALF_TYPE__ lf_gcflags; /* [lock(leaks_lock)] Flags used to track state during GC leak search */
#define LEAK_FOOTER_GCFLAG_NORMAL  0x0000 /* Normal flags */
#define LEAK_FOOTER_GCFLAG_REACH   0x0001 /* Reachable */
#define LEAK_FOOTER_GCFLAG_SCANNED 0x0002 /* Scanned for pointers */
#define LEAK_FOOTER_GCFLAG_RED     ((__UINTPTR_HALF_TYPE__)1 << (sizeof(__UINTPTR_HALF_TYPE__) * __CHAR_BIT__ - 1)) /* Red node */
#define LEAK_FOOTER_GCFLAG_HITMASK (~LEAK_FOOTER_GCFLAG_RED)
#define LEAK_FOOTER_GCFLAG_ONEHIT  1
#else /* LEAK_DETECTION_GC || FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
#define leak_footer_line_t int
	int                   lf_line;  /* [lock(OWNER(lf_chunk))] Line where "lf_chunk" was allocated */
#endif /* !LEAK_DETECTION_GC && !FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
	size_t                lf_id;    /* Allocation ID (I hate that this field prevents this thing from aligning nicely) */
};

#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
#define leak_footer_isregion(self)  ((self)->lf_foot == 1) /* Values [1,3] can never be set due to: alignment of "malloc_state", and initialization of "mparams.magic" */
#define leak_footer_setregion(self) (void)((self)->lf_foot = 1)
#define leak_footer_asregion(self)  ((struct region_leak_footer *)(self))

struct region_leak_footer {
	/* Underlying leak footer:
	 * - "leak_footer_isregion(&rlf_leak) == true"
	 * - "lf_chunk" is actually "struct Dee_heapregion *"
	 * - "lf_id" is the ID of the region's first (still-allocated) chunk
	 * - "lf_file" is actually [1..1][const][owned]
	 */
	struct leak_footer                     rlf_leak;       /* Underlying leak footer */
	LIST_ENTRY(region_leak_footer)         rlf_link;       /* [1..1] Link in global list of region leaks */
#if LEAK_DETECTION_GC
	COMPILER_FLEXIBLE_ARRAY(unsigned char, rlf_reachable); /* [CEILDIV(region_leak_footer_getwordcount(self), __CHAR_BIT__)]
	                                                        * Bitset of reachable word in "Dee_heapregion::hr_data" (used by
	                                                        * GC leak detector) */
#endif /* LEAK_DETECTION_GC */
};

#if LEAK_DETECTION_GC
#define region_leak_footer_sizeof(bitcnt) \
	(offsetof(struct region_leak_footer, rlf_reachable) + CEILDIV(bitcnt, __CHAR_BIT__))
#else /* LEAK_DETECTION_GC */
#define region_leak_footer_sizeof(bitcnt) sizeof(struct region_leak_footer)
#endif /* !LEAK_DETECTION_GC */
#define region_leak_footer_alloc(bitcnt) \
	((struct region_leak_footer *)dlmalloc(region_leak_footer_sizeof(bitcnt)))
#define region_leak_footer_free(self) \
	dlfree(self)
#define region_leak_footer_destroy(self)       \
	(dlfree((char *)(self)->rlf_leak.lf_file), \
	 region_leak_footer_free(self))

#define Dee_heapregion_getusize(self)      ((self)->hr_size - sizeof(struct Dee_heaptail))
#define Dee_heapregion_getuminaddr(self)   ((byte_t *)Dee_heapregion_gethead(self))
#define Dee_heapregion_getumaxaddr(self)   (Dee_heapregion_getuminaddr(self) + Dee_heapregion_getusize(self) - 1)
#define Dee_heapregion_getuwordcount(self) (Dee_heapregion_getusize(self) / sizeof(void *))

#define chunk2region(self) container_of(&(self)->head, struct Dee_heapregion, hr_size)
#define region2chunk(self) container_of(&(self)->hr_size, struct malloc_chunk, head)

#define region_leak_footer_getregion(self)    chunk2region(mem2chunk((self)->rlf_leak.lf_chunk))
#define region_leak_footer_getsize(self)      Dee_heapregion_getusize(region_leak_footer_getregion(self))
#define region_leak_footer_getminaddr(self)   Dee_heapregion_getuminaddr(region_leak_footer_getregion(self))
#define region_leak_footer_getmaxaddr(self)   Dee_heapregion_getumaxaddr(region_leak_footer_getregion(self))
#define region_leak_footer_getwordcount(self) Dee_heapregion_getuwordcount(region_leak_footer_getregion(self))
#if LEAK_DETECTION_GC
#define region_leak_footer_clearreachable(self) \
	bzero((self)->rlf_reachable, CEILDIV(region_leak_footer_getwordcount(self), __CHAR_BIT__))
#define region_leak_footer_getreachable(self, word) ((self)->rlf_reachable[(word) / __CHAR_BIT__] & ((unsigned char)1 << ((word) % __CHAR_BIT__)))
#define region_leak_footer_setreachable(self, word) (void)((self)->rlf_reachable[(word) / __CHAR_BIT__] |= ((unsigned char)1 << ((word) % __CHAR_BIT__)))
#endif /* LEAK_DETECTION_GC */

#endif /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */


/* Return pointer to "prev_foot" field of next chunk (i.e. the "footer" field of this chunk) */
#define chunk_p_foot(X) ((size_t *)chunk_plus_offset(X, chunksize(X)))

#ifndef LEAK_FOOTER_SLIST_DEFINED
#define LEAK_FOOTER_SLIST_DEFINED
SLIST_HEAD(leak_footer_slist, leak_footer);
#endif /* !LEAK_FOOTER_SLIST_DEFINED */

#define leak_footer_alloc_uncached() \
	((struct leak_footer *)dlmalloc(sizeof(struct leak_footer)))
#define leak_footer_free_uncached(self) \
	dlfree(self)

/* XXX: Somehow, the idea of "leak_footer_freelist" is **SLOWER** than the alternative
 *      of just always using dlmalloc() / dlfree() directly:
 *
 * Disabled: time make computed-operators    real    0m7.234s   161%
 * Enabled:  time make computed-operators    real    0m8.745s   195%
 *
 * I'm guessing it's once again that problem where there's only a **single**, global
 * leak footers cache, whereas dlmalloc() and dlfree() will automatically make use of
 * per-thread caches (meaning that there's much less conflicts when doing atomic ops) */
#if 0
PRIVATE size_t leak_footer_freesize_max = 4096; /* XXX: Configure somehow? */

/* Lazily updated to reflect length of "leak_footer_freelist" (may not be up-to-date) */
PRIVATE size_t leak_footer_freesize = 0;

/* [0..n][lock(APPEND(ATOMIC), REMOVE(leak_footer_freelock))] */
PRIVATE struct leak_footer_slist leak_footer_freelist = SLIST_HEAD_INITIALIZER(leak_footer_freelist);

/* Lock needed to remove items from "leak_footer_freelist" */
#if USE_LOCKS
PRIVATE DL_LOCK_T leak_footer_freelock = DL_LOCK_INIT_STATIC;
#endif /* USE_LOCKS */
#define leak_footer_freelock_tryacquire() DL_LOCK_TRYACQUIRE(&leak_footer_freelock)
#define leak_footer_freelock_acquire()    DL_LOCK_ACQUIRE(&leak_footer_freelock)
#define leak_footer_freelock_release()    DL_LOCK_RELEASE(&leak_footer_freelock)

PRIVATE ATTR_HOT ATTR_MALLOC WUNUSED struct leak_footer *DCALL
leak_footer_alloc(void) {
	struct leak_footer *result;
	result = atomic_read(&leak_footer_freelist.slh_first);
	if (result && leak_footer_freelock_tryacquire()) {
		result = atomic_read(&leak_footer_freelist.slh_first);
		if (result && atomic_cmpxch(&leak_footer_freelist.slh_first,
		                            result, result->lf_unused.sle_next)) {
			leak_footer_freelock_release();
			atomic_dec(&leak_footer_freesize);
			return result;
		}
		leak_footer_freelock_release();
	}
	return leak_footer_alloc_uncached();
}

PRIVATE ATTR_HOT NONNULL((1)) void DCALL
leak_footer_free(struct leak_footer *__restrict self) {
	if (atomic_read(&leak_footer_freesize) < leak_footer_freesize_max) {
		/* Only try to append to free list **once**
		 * atomic_cmpxch-loops can become a bottleneck when many CPUs are hammering away! */
		self->lf_unused.sle_next = atomic_read(&leak_footer_freelist.slh_first);
		if (atomic_cmpxch(&leak_footer_freelist.slh_first,
		                  self->lf_unused.sle_next, self)) {
			atomic_inc(&leak_footer_freesize);
			return;
		}
	}

	/* Just free the footer and ignore the cache */
	leak_footer_free_uncached(self);
}
#else
#define leak_footer_alloc()    leak_footer_alloc_uncached()
#define leak_footer_free(self) leak_footer_free_uncached(self)
#endif

PRIVATE size_t alloc_id_count = 0; /* Last-assigned allocation id (for alloc breakpoints) */
PRIVATE size_t alloc_id_break = 0; /* ID of allocation which (when allocated) must break into a debugger */

#if USE_PER_MSPACE_ALLOC_ID
PRIVATE WUNUSED size_t DCALL do_get_alloc_id(size_t orig_foot) {
	mstate ms = mstate_decode_for_footer(orig_foot);
	size_t old_id, new_id;
	do {
retry:
		old_id = atomic_read(&ms->ms_alloc_id);
		if ((old_id & MSTATE_ALLOC_MASK) == 0) {
			/* Allocation exhausted -- must allocate a new chunk from "alloc_id_count" */
			size_t old_count, new_count;
			do {
				old_count = atomic_read(&alloc_id_count);
				new_count = (old_count + 1 + MSTATE_ALLOC_MASK) & ~MSTATE_ALLOC_MASK;
				ASSERT(new_count > old_count);
				++new_count;
			} while (!atomic_cmpxch_weak(&alloc_id_count, old_count, new_count));
			++old_count;
			if (!atomic_cmpxch(&ms->ms_alloc_id, old_id, old_count)) {
				(void)atomic_cmpxch(&alloc_id_count, new_count, old_count);
				goto retry;
			}
			return old_count;
		}
		new_id = old_id + 1;
	} while (!atomic_cmpxch_weak(&ms->ms_alloc_id, old_id, new_id));
	return new_id;
}
#define get_alloc_id(mchunkptr_p, orig_foot) do_get_alloc_id(orig_foot)
#else /* USE_PER_MSPACE_ALLOC_ID */
#define get_alloc_id(mchunkptr_p, orig_foot) atomic_incfetch(&alloc_id_count)
#endif /* !USE_PER_MSPACE_ALLOC_ID */


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


#if USE_LOCKS
PRIVATE DL_LOCK_T leaks_lock = DL_LOCK_INIT_STATIC;
#endif /* USE_LOCKS */
#ifdef DL_LOCK_ACQUIRED
#define leaks_lock_acquired()   DL_LOCK_ACQUIRED(&leaks_lock)
#endif /* DL_LOCK_ACQUIRED */
#define leaks_lock_tryacquire() DL_LOCK_TRYACQUIRE(&leaks_lock)
#define leaks_lock_acquire()    DL_LOCK_ACQUIRE(&leaks_lock)
#define _leaks_lock_release()   DL_LOCK_RELEASE(&leaks_lock)

/* [lock(leaks_lock)] Ring of memory leaks */
PRIVATE struct leak_footer leaks = {
	/* .lf_foot  = */ { 0 },      /* [const] */
	/* .lf_chunk = */ NULL,       /* [const] */
	/* .lf_prev  = */ &leaks,     /* [1..1][lock(leaks_lock)] */
	/* .lf_next  = */ { &leaks }, /* [1..1][lock(leaks_lock)] */
	/* .lf_file  = */ { NULL },   /* [const] */
	/* .lf_line  = */ 0,          /* [const] */
#if LEAK_DETECTION_GC || FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
	/* .lf_gcflags = */ 0,        /* [const] */
#endif /* LEAK_DETECTION_GC || FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
	/* .lf_id    = */ 0           /* [const] */
};

#define leaks_assert_linkage(leak)                  \
	(dl_assert((leak)->lf_prev->lf_next == (leak)), \
	 dl_assert((leak)->lf_next->lf_prev == (leak)))

#define leaks_remove(leak)                             \
	(void)(leaks_assert_linkage(leak),                 \
	       (leak)->lf_prev->lf_next = (leak)->lf_next, \
	       (leak)->lf_next->lf_prev = (leak)->lf_prev)
#if 1
#define leaks_insert_begin(leak)         (void)((leak)->lf_prev = NULL)
#define leaks_insert_is_unfinished(leak) (atomic_read(&(leak)->lf_prev) == NULL)
#define leaks_insert_finish(leak)                                          \
	(void)(dl_assertf((leak)->lf_prev == NULL,                             \
	                  "Should have been done by leaks_insert_begin()"),    \
	       (leak)->lf_prev = &leaks,                            /* 1 */    \
	       ((leak)->lf_next = leaks.lf_next)->lf_prev = (leak), /* 2, 3 */ \
	       leaks.lf_next = (leak))                              /* 4 */
#else
#define leaks_insert_begin(leak) \
	(void)((leak)->lf_prev = &leaks) /* 1 */
#define leaks_insert_finish(leak)                                                  \
	(void)(dl_assertf((leak)->lf_prev == &leaks,                                   \
	                  "Should have been done by leaks_insert_begin()"), /* 1 */    \
	       ((leak)->lf_next = leaks.lf_next)->lf_prev = (leak),         /* 2, 3 */ \
	       leaks.lf_next = (leak))                                      /* 4 */
#endif

#define leaks_insert(leak) (leaks_insert_begin(leak), leaks_insert_finish(leak))


#if USE_PER_MSPACE_LEAK_OPS
/* [0..n][lock(ATOMIC)] m-states with pending leak insert/remove operations */
PRIVATE mstate leaks_pending_states = MSTATE_LEAKS_PENDING_NEXT__NONE;

#define leaks_pending_mustreap() \
	(atomic_read(&leaks_pending_states) != MSTATE_LEAKS_PENDING_NEXT__NONE)

LOCAL WUNUSED NONNULL((1)) mstate
leak_get_pending_mstate(struct leak_footer *__restrict leak) {
#if 1
	/* Must use this variant, since the linked mstate mustn't
	 * change, even if the memory of "leak" is realloc'd!
	 *
	 * Otherwise, necessary INSERT jobs may not be executed
	 * before an associated REMOVE job is run. */
	mchunkptr p = mem2chunk(leak);
	size_t *p_foot = chunk_p_foot(p);
	return mstate_decode_for_footer(*p_foot);
#else
	return mstate_decode_for_footer(leak->lf_foot);
#endif
}

LOCAL void mstate_set_haspending(mstate ms) {
	mstate old_next, new_next;
	do {
		old_next = atomic_read(&ms->ms_leaks_pending_next);
		if (old_next != MSTATE_LEAKS_PENDING_NEXT__UNBOUND)
			return; /* Already registered (or currently being registered) */
		new_next = atomic_read(&leaks_pending_states);
		/* Once this cmpxch succeeds, we have to commit to adding
		 * "ms" to "leaks_pending_states" in all cases! */
	} while (!atomic_cmpxch_weak(&ms->ms_leaks_pending_next, old_next, new_next));

	/* WARNING: At this point, "ms" is in a sort-of "quantum" state, where
	 *          it both is- and isn't part of "leaks_pending_states":
	 * - as far as it itself is concerned, it is already part of the
	 *   global "leaks_pending_states" list
	 * - but if one were to enumerate "leaks_pending_states", one would
	 *   not be able to encounter "ms"
	 *
	 * Despite this weirdness, this actually only poses a problem within
	 * "DeeDbg_UntrackAlloc()", which has a "#if USE_PER_MSPACE_LEAK_OPS"
	 * block to handle this race condition */

	/* Insert mstate into global list of states with pending leak tasks. */
	while (!atomic_cmpxch_weak(&leaks_pending_states, new_next, ms)) {
		new_next = atomic_read(&leaks_pending_states);
		atomic_write(&ms->ms_leaks_pending_next, new_next);
	}
}

#define leaks_pending_insert_add(leak)                                        \
	do {                                                                      \
		mstate ms = leak_get_pending_mstate(leak);                            \
		struct leak_footer *_lins_next;                                       \
		do {                                                                  \
			_lins_next = atomic_read(&ms->ms_leaks_pending_insert.slh_first); \
			(leak)->lf_inslink.sle_next = _lins_next;                         \
			COMPILER_WRITE_BARRIER();                                         \
		} while (!atomic_cmpxch_weak(&ms->ms_leaks_pending_insert.slh_first,  \
		                             _lins_next, leak));                      \
		if (_lins_next == NULL)                                               \
			mstate_set_haspending(ms);                                        \
	}	__WHILE0
#define leaks_pending_remove_add(leak)                                        \
	do {                                                                      \
		mstate ms = leak_get_pending_mstate(leak);                            \
		struct leak_footer *_lrem_next;                                       \
		do {                                                                  \
			_lrem_next = atomic_read(&ms->ms_leaks_pending_remove.slh_first); \
			(leak)->lf_remlink.sle_next = _lrem_next;                         \
			COMPILER_WRITE_BARRIER();                                         \
		} while (!atomic_cmpxch_weak(&ms->ms_leaks_pending_remove.slh_first,  \
		                             _lrem_next, leak));                      \
		if (_lrem_next == NULL)                                               \
			mstate_set_haspending(ms);                                        \
	}	__WHILE0


/* Reap "leaks_pending_insert" only */
PRIVATE void DCALL mspace_leaks_lock_reap_insert_locked(mstate ms) {
	struct leak_footer_slist insert;
	struct leak_footer *leak;
	insert.slh_first = SLIST_ATOMIC_CLEAR(&ms->ms_leaks_pending_insert);
	while (!SLIST_EMPTY(&insert)) {
		leak = SLIST_FIRST(&insert);
		SLIST_REMOVE_HEAD(&insert, lf_inslink);
		leaks_insert_finish(leak);
	}
}

PRIVATE ATTR_NOINLINE void DCALL leaks_lock_reap_and_unlock(void) {
	struct leak_footer_slist remove;
	struct leak_footer **p_remove_tail;
	struct leak_footer *leak;
	mstate ms, next;

	/* Load list of mstates with pending jobs. */
	ms = atomic_xch(&leaks_pending_states, MSTATE_LEAKS_PENDING_NEXT__NONE);

	SLIST_INIT(&remove);
	p_remove_tail = SLIST_PFIRST(&remove);
	while (ms != MSTATE_LEAKS_PENDING_NEXT__NONE) {
		/* Mark as unlinked from "leaks_pending_states" (must happen **before**
		 * we consume pending sets, so the mspace can re-add itself in case they
		 * receive a new leak operation) */
		next = ms->ms_leaks_pending_next;
		COMPILER_READ_BARRIER();
		atomic_write(&ms->ms_leaks_pending_next, MSTATE_LEAKS_PENDING_NEXT__UNBOUND);

		/* Order here is important: must reap "remove" **BEFORE** "insert"!
		 *
		 * In case a pointer ends up added to both lists, it is guarantied
		 * that if it appears in "remove", it will either:
		 * - Already be within "leaks"
		 * - or: have been added to "insert" previously
		 *
		 * Combine that with this executing "insert" first, the removal code
		 * is allowed to assume that everything from "remove" will currently
		 * be present somewhere within "leaks"! */
		*p_remove_tail = SLIST_ATOMIC_CLEAR(&ms->ms_leaks_pending_remove);

		/* Execute "insert" tasks */
		mspace_leaks_lock_reap_insert_locked(ms);

		/* Execute "remove" tasks */
		while ((leak = *p_remove_tail) != NULL) {
			leaks_remove(leak);
			p_remove_tail = SLIST_PNEXT(leak, lf_remlink);
		}
		ms = next;
	}

	/* Release lock */
	_leaks_lock_release();

	/* Free underlying memory blocks from "remove" */
	ASSERT(*p_remove_tail == NULL);
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
#else /* USE_PER_MSPACE_LEAK_OPS */
PRIVATE struct leak_footer_slist leaks_pending_insert = SLIST_HEAD_INITIALIZER(leaks_pending_insert);
PRIVATE struct leak_footer_slist leaks_pending_remove = SLIST_HEAD_INITIALIZER(leaks_pending_remove);

#define leaks_pending_mustreap()                             \
	(atomic_read(&leaks_pending_insert.slh_first) != NULL || \
	 atomic_read(&leaks_pending_remove.slh_first) != NULL)
#define leaks_pending_mustreap_insert() \
	(atomic_read(&leaks_pending_insert.slh_first) != NULL)

#define leaks_pending_insert_add(leak) \
	SLIST_ATOMIC_INSERT(&leaks_pending_insert, leak, lf_inslink)
#define leaks_pending_remove_add(leak) \
	SLIST_ATOMIC_INSERT(&leaks_pending_remove, leak, lf_remlink)


/* Reap "leaks_pending_insert" only */
PRIVATE void DCALL leaks_lock_reap_insert_locked(void) {
	struct leak_footer_slist insert;
	struct leak_footer *leak;
	insert.slh_first = SLIST_ATOMIC_CLEAR(&leaks_pending_insert);
	while (!SLIST_EMPTY(&insert)) {
		leak = SLIST_FIRST(&insert);
		SLIST_REMOVE_HEAD(&insert, lf_inslink);
		leaks_insert_finish(leak);
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
	 * Combine that with this executing "insert" first, the removal code
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
#endif /* !USE_PER_MSPACE_LEAK_OPS */


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

#ifdef Dee_BREAKPOINT_IS_NOOP
#define fatal _DeeAssert_XFailf
#else /* Dee_BREAKPOINT_IS_NOOP */
#define fatal(expr, file, line, ...) \
	(_DeeAssert_Failf(expr, file, line, __VA_ARGS__), Dee_BREAKPOINT())
#endif /* !Dee_BREAKPOINT_IS_NOOP */

#ifndef __ARCH_PAGESIZE_MIN
#ifdef __ARCH_PAGESIZE
#define __ARCH_PAGESIZE_MIN __ARCH_PAGESIZE
#endif /* __ARCH_PAGESIZE */
#endif /* !__ARCH_PAGESIZE_MIN */

#ifdef __ARCH_PAGESIZE_MIN
#define _lo_leak_addrok(p) ((uintptr_t)(p) >= __ARCH_PAGESIZE_MIN)
#else /* __ARCH_PAGESIZE_MIN */
#define _lo_leak_addrok(p) ((void *)(p) != NULL)
#endif /* !__ARCH_PAGESIZE_MIN */

/* Try to figure out some known-bad address ranges so we can detect bad pointers. */
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__)
#if (defined(__i386__) && __SIZEOF_POINTER__ == 4) && (defined(__linux__) || defined(__KOS__))
/* Kernel lives in high 3/4 of address space */
#define _hi_leak_addrok(p) ((uintptr_t)(p) < UINT32_C(0xC0000000))
#elif defined(CONFIG_HOST_WINDOWS) || (!defined(__arm__) && (defined(__linux__) || defined(__KOS__)))
/* Kernel lives in high 1/2 of address space */
#define _hi_leak_addrok(p) ((uintptr_t)(p) < ((((size_t)-1) >> 1) + 1))
#endif /* ... */
#endif /* ... */

#ifdef _hi_leak_addrok
#define _leak_addrok(p) (_lo_leak_addrok(p) && _hi_leak_addrok(p))
#else /* _hi_leak_addrok */
#define _leak_addrok(p) _lo_leak_addrok(p)
#endif /* !_hi_leak_addrok */

#define leak_addrok(p) (_leak_addrok(p) && IS_ALIGNED((uintptr_t)(p), sizeof(void *)))


#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE

#define _Dee_heapregion__p_uptr(T, self) \
	(*(T *)((byte_t *)(self) + (self)->hr_size - sizeof(size_t)))
#define Dee_heapregion_hasfooter_or_next(self) (_Dee_heapregion__p_uptr(uintptr_t, self) != 0)
#define Dee_heapregion_hasnext(self)           (_Dee_heapregion__p_uptr(uintptr_t, self) & 1)
#define Dee_heapregion_getfooter(self)    _Dee_heapregion__p_uptr(struct region_leak_footer *const, self)
#define Dee_heapregion_setfooter(self, v) (void)(_Dee_heapregion__p_uptr(struct region_leak_footer *, self) = (v))
#define Dee_heapregion_getnext(self)      ((struct Dee_heapregion *)(_Dee_heapregion__p_uptr(uintptr_t const, self) & ~(Dee_HEAPCHUNK_ALIGN - 1)))
#define Dee_heapregion_setnext(self, v)   (void)(_Dee_heapregion__p_uptr(uintptr_t, self) = (uintptr_t)(v) | 1)
#define Dee_heapregion_unsetfooter(self)  (void)(_Dee_heapregion__p_uptr(uintptr_t, self) = 0)
#if 1 /* Does the same... */
#define Dee_heapregion_getnext_opt(self) Dee_heapregion_getnext(self)
#else
#define Dee_heapregion_getnext_opt(self) (Dee_heapregion_hasnext(self) ? Dee_heapregion_getnext(self) : NULL;)
#endif

#if LEAK_DETECTION_GC
STATIC_ASSERT_MSG(Dee_HEAPCHUNK_ALIGN > 4, "bit 0 is needed for 'Dee_heapregion_hasnext'; bit 1,2 are needed for gcleak-visit");
#define Dee_heapregion_cleargcflags(self)     (void)(_Dee_heapregion__p_uptr(uintptr_t, self) &= ~(2 | 4))
#define Dee_heapregion_get_gc_reachable(self) (_Dee_heapregion__p_uptr(uintptr_t const, self) & 2)
#define Dee_heapregion_get_gc_visited(self)   (_Dee_heapregion__p_uptr(uintptr_t const, self) & 4)
#define Dee_heapregion_set_gc_reachable(self) (void)(_Dee_heapregion__p_uptr(uintptr_t, self) |= 2)
#define Dee_heapregion_set_gc_visited(self)   (void)(_Dee_heapregion__p_uptr(uintptr_t, self) |= 4)
#endif /* LEAK_DETECTION_GC */


LIST_HEAD(region_leak_list, region_leak_footer);

/* [0..n][lock(leaks_lock)][LINK(rlf_link)] List of heap region leak footers */
PRIVATE struct region_leak_list region_leak_flist = LIST_HEAD_INITIALIZER(region_leak_flist);

/* [0..n][lock(leaks_lock)][LINK(Dee_heapregion_getnext)]
 * List of heap regions without leak footers (used as fallback
 * when 'DeeDbgHeap_AddHeapRegion()' can't allocate memory, so
 * reachable-semantics can be retained in a sufficient manner) */
PRIVATE struct Dee_heapregion *region_leak_mlist = NULL;

PRIVATE NONNULL((1)) struct Dee_heapregion *DCALL
region_leak_mlist_insert(struct Dee_heapregion *__restrict region) {
	Dee_heapregion_setnext(region, region_leak_mlist);
	region_leak_mlist = region;
	return region;
}

PRIVATE NONNULL((1)) void DCALL
region_leak_mlist_remove(struct Dee_heapregion *__restrict region) {
	if (region_leak_mlist == region) {
		region_leak_mlist = Dee_heapregion_getnext(region);
	} else {
		struct Dee_heapregion *next;
		struct Dee_heapregion *prev = region_leak_mlist;
		ASSERT(prev);
		for (;;) {
			ASSERT(Dee_heapregion_hasnext(prev));
			next = Dee_heapregion_getnext(prev);
			if (next == region)
				break;
			prev = next;
		}
		next = Dee_heapregion_getnext_opt(region);
		Dee_heapregion_setnext(prev, next);
	}
}

/* Remove debug info attached to "region" -- caller must be holding "leaks_lock" */
PRIVATE ATTR_RETNONNULL NONNULL((1)) struct Dee_heapregion *DCALL
dl_dbg_heapregion_dispose(struct Dee_heapregion *__restrict region) {
#ifdef leaks_lock_acquired
	ASSERT(leaks_lock_acquired());
#endif /* leaks_lock_acquired */
	if (Dee_heapregion_hasfooter_or_next(region)) {
		if (Dee_heapregion_hasnext(region)) {
			region_leak_mlist_remove(region);
		} else {
			struct region_leak_footer *footer;
			footer = Dee_heapregion_getfooter(region);
			leaks_remove(&footer->rlf_leak); /* Remove from "leaks" */
			LIST_REMOVE(footer, rlf_link);   /* Remove from "region_leak_flist" */
			region_leak_footer_destroy(footer);
		}
		Dee_heapregion_unsetfooter(region);
	}
	return region;
}

/* Return the # of chunks defined by "self" */
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) size_t DCALL
Dee_heapregion_chunkcount(struct Dee_heapregion *__restrict self) {
	size_t result = 0;
	mchunkptr tail = (mchunkptr)((byte_t *)self + self->hr_size - sizeof(struct Dee_heaptail));
	mchunkptr iter = (mchunkptr)&self->hr_first;
	while (iter < tail) {
		iter = chunk_plus_offset(iter, chunksize(iter));
		++result;
	}
	return result;
}

PRIVATE ATTR_RETNONNULL NONNULL((1, 2)) struct Dee_heapregion *DCALL
DeeDbgHeap_AddHeapRegion_locked_p(struct Dee_heapregion *__restrict region,
                                  struct region_leak_footer *__restrict footer,
                                  /*inherit(always)*/ char const *file) {
	size_t chunk_count;
	mchunkptr region_as_chunk = region2chunk(region);
	ASSERT(is_mmapped(region_as_chunk));
	ASSERT(!flag4inuse(region_as_chunk));
	ASSERT(overhead_for(region_as_chunk) == MMAP_CHUNK_OVERHEAD);

	/* Hook "leak" into "ptr" */
	leak_footer_setregion(&footer->rlf_leak);
	footer->rlf_leak.lf_chunk = chunk2mem(region_as_chunk);
	footer->rlf_leak.lf_file  = file; /* Inherit */
	footer->rlf_leak.lf_line  = (leak_footer_line_t)-1; /* Unused by region leaks */
	Dee_heapregion_setfooter(region, footer);
	ASSERT(leak_footer_isregion(&footer->rlf_leak));

	/* Allocate ID(s) */
	chunk_count = Dee_heapregion_chunkcount(region);
	footer->rlf_leak.lf_id = atomic_fetchadd(&alloc_id_count, chunk_count) + 1;
	if (((size_t)(footer->rlf_leak.lf_id /*         */) <= alloc_id_break) &&
	    ((size_t)(footer->rlf_leak.lf_id + chunk_count) > alloc_id_break) &&
	    (alloc_id_break != 0)) {
#ifndef Dee_BREAKPOINT_IS_NOOP
		Dee_BREAKPOINT();
#elif __has_builtin(__builtin_trap)
		__builtin_trap();
#else /* ... */
		byte_t volatile *volatile P = (byte_t volatile *)NULL;
		*P = 'B';
#endif /* !... */
	}

	/* Insert into list of memory leaks + regions */
	leaks_insert(&footer->rlf_leak);
	LIST_INSERT_HEAD(&region_leak_flist, footer, rlf_link);
	return region;
}

/* Check if any tracked "leaks" contains "ptr". If so, untrack and return it. */
PRIVATE struct leak_footer *DCALL
leaks_untrack_chunk_containing(void const *ptr) {
	struct leak_footer *iter;
	for (iter = leaks.lf_next; iter != &leaks; iter = iter->lf_next) {
		/* XXX: This read may return an invalid pointer if another thread just did a dlrealloc(chunk)! */
		void *mem = atomic_read(&iter->lf_chunk);
		mchunkptr p = mem2chunk(mem);
		size_t size = chunksize(p) - overhead_for(p);
		if ((byte_t *)ptr >= (byte_t *)mem &&
		    (byte_t *)ptr < ((byte_t *)mem + size)) {
			/* Do the same that's done by `Dee_UntrackAlloc()' below. */
			leaks_remove(iter);
			iter->lf_prev = iter;
			iter->lf_next = iter;
			return iter;
		}
	}
	return NULL;
}

PRIVATE ATTR_MALLOC WUNUSED NONNULL((1)) char *DCALL try_strdup(char const *str) {
	size_t len = (strlen(str) + 1) * sizeof(char);
	char *result = (char *)dlmalloc(len);
	if (result)
		result = (char *)memcpy(result, str, len);
	return result;
}

#define DeeDbgHeap_AddHeapRegion_DEFINED
/* Attach debug info (for the sake of memory leaks as reported by `DeeHeap_DumpMemoryLeaks()',
 * as well as `DeeHeap_DumpMemoryLeaks_GC' being able to recursively scan the payload areas of
 * reachable heap chunks) to a custom "struct Dee_heapregion"
 *
 * These calls are entirely OPTIONAL, but if not called, `DeeHeap_DumpMemoryLeaks()' will not
 * be able to inform you about heap chunks from `region' that are never free'd, or be able to
 * identify `Dee_Malloc()' pointers stored in the payload areas of reachable chunks within the
 * given `region' when those chunks are reachable and called using `DeeHeap_DumpMemoryLeaks_GC'
 *
 * WARNING: `DeeDbgHeap_DelHeapRegion()' is thread-safe, but only in those cases where you can
 *          guaranty that at least 1 of `region's heap-chunks has not yet been freed, and will
 *          not be freed by another thread during the call to this function. (iow: it may only
 *          be called when there is chance that `hr_destroy' has been- or will be called before
 *          the call has a chance to return)
 *
 * @param: file:   A filename that should appear when memory leaks are dumped.
 *                 Note that unlike other debug-heap functions, this string is actually
 *                 strdup()'d, meaning it's allowed to point to a dynamically allocated
 *                 memory location.
 * @param: region: The region to register/unregister debug information for.
 *                 Even when not registered, `Dee_Free()' works as it should!
 *                 These functions are only necessary for `DeeHeap_DumpMemoryLeaks()'!
 * @return: * : Always re-returns "region". These APIs are intentionally designed to never fail
 *              (or rather: to fail-safe), never block (indefinitely), and never return an error.
 *              These API *may* however modify the given `region's `hr_tail.ht_zero' field. */
PUBLIC ATTR_RETNONNULL NONNULL((1)) struct Dee_heapregion *DCALL
DeeDbgHeap_AddHeapRegion(struct Dee_heapregion *__restrict region, char const *file) {
	struct region_leak_footer *footer;
	footer = region_leak_footer_alloc(Dee_heapregion_getuwordcount(region));
	file = file ? try_strdup(file) : NULL;
	leaks_lock_acquire_and_reap();

	/* Check if "region" is embedded within some larger heap chunk,
	 * as is the case when it is part of a file mapping that could
	 * not be mmap'd, and had to be malloc()+read()'d. */
	{
		struct leak_footer *embedded_leak;
		embedded_leak = leaks_untrack_chunk_containing((void *)region);
		if (embedded_leak && !file) {
			file = embedded_leak->lf_file; /* Inherit debug info if caller didn't specify any */
			file = file ? try_strdup(file) : NULL;
		}
	}

	region = dl_dbg_heapregion_dispose(region); /* For safety -- should not be bound, yet */
	if likely(footer) {
		region = DeeDbgHeap_AddHeapRegion_locked_p(region, footer, file);
		file = NULL; /* Inherited */
	} else {
		region = region_leak_mlist_insert(region);
	}
	leaks_lock_release();
#if !dlfree_CHECKS_NULL
	if (file)
#endif /* !dlfree_CHECKS_NULL */
	{
		dlfree((char *)file);
	}
	return region;
}

PUBLIC ATTR_RETNONNULL NONNULL((1)) struct Dee_heapregion *DCALL
DeeDbgHeap_DelHeapRegion(struct Dee_heapregion *__restrict region) {
	leaks_lock_acquire_and_reap();
	region = dl_dbg_heapregion_dispose(region);
	leaks_lock_release();
	return region;
}
#endif /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */



#if LEAK_DETECTION_GC
PRIVATE WUNUSED NONNULL((1)) bool DCALL
leaks_pending_remove__contains(struct leak_footer *leak) {
	struct leak_footer *remove;
	remove = atomic_read(&leaks_pending_remove.slh_first);
	for (; remove; remove = SLIST_NEXT(remove, lf_remlink)) {
		if (remove == leak)
			return true;
	}
	return false;
}
#endif /* LEAK_DETECTION_GC */



PRIVATE void DCALL dumpleaks_acquire_locks(void) {
	/* Need a whole bunch of locks here:
	 * - leaks_lock_acquire_and_reap()     -- prevent tracked leaks from changing
	 * - PREACTION(gm)                     -- prevent dlrealloc() from changing "leak_footer::lf_chunk"
	 * - tls_mspace_lock_acquire()         -- prevent dlrealloc() from changing "leak_footer::lf_chunk"
	 * - PREACTION(used_tls_mspace.each)   -- prevent dlrealloc() from changing "leak_footer::lf_chunk"
	 * - Locks for all slab allocators
	 *
	 * For GC only:
	 * - gc_lock                           -- need to skew GC list and links
	 * - module_abstree_lock               -- need to skew "module_abstree_root"
	 * - module_libtree_lock               -- need to skew "module_libtree_root"
	 * - module_byaddr_lock                -- need to skew "module_byaddr_tree"
	 */
#define local_unlock_0() (void)0
#define local_unlock_1() (void)0
#define local_unlock_2() (void)0
#define local_unlock_3() (void)0
#define local_unlock_4() (void)0
#define local_unlock_5() (void)0
#define local_unlock_6() (void)0
#define local_unlock_7() (void)0
#define local_unlock_8() (void)0
#define local_unlock_9() (void)0
#define local_unlock()    \
	do {                  \
		local_unlock_9(); \
		local_unlock_8(); \
		local_unlock_7(); \
		local_unlock_6(); \
		local_unlock_5(); \
		local_unlock_4(); \
		local_unlock_3(); \
		local_unlock_2(); \
		local_unlock_1(); \
		local_unlock_0(); \
	}	__WHILE0


again:
	/* Lock leak system */
	leaks_lock_acquire_and_reap();
#undef local_unlock_0
#define local_unlock_0() leaks_lock_release()

	/* Lock "gm" */
	if (!TRY_PREACTION(gm)) {
		local_unlock();
		DL_LOCK_WAITFOR(&mstate_mutex(gm));
		goto again;
#undef local_unlock_1
#define local_unlock_1() POSTACTION(gm)
	}

	/* Lock TLS mspace allocator (prevent more mspaces from appearing) */
#if USE_PER_THREAD_MSTATE
	if (!tls_mspace_lock_tryacquire()) {
		local_unlock();
		tls_mspace_lock_waitfor();
		goto again;
#undef local_unlock_2
#define local_unlock_2() tls_mspace_lock_release()
	}

	/* Lock every (in-use) TLS mspace */
	{
		struct malloc_state *tls;
		SLIST_FOREACH (tls, &used_tls_mspace, ms_link) {
			if (!TRY_PREACTION(tls)) {
				struct malloc_state *tls2;
				SLIST_FOREACH (tls2, &used_tls_mspace, ms_link) {
					if (tls2 == tls)
						break;
					POSTACTION(tls2);
				}
				local_unlock();
				/* TLS mspaces are never free'd, so no need to worry about "tls" going away here... */
				DL_LOCK_WAITFOR(&mstate_mutex(tls));
			}
		}
	}
#undef local_unlock_3
#define local_unlock_3()                                 \
	do {                                                 \
		struct malloc_state *tls;                        \
		SLIST_FOREACH (tls, &used_tls_mspace, ms_link) { \
			POSTACTION(tls);                             \
		}                                                \
	}	__WHILE0
#endif /* USE_PER_THREAD_MSTATE */

#ifdef HAVE_Dee_slab_leaks_tryacquire
	{
		Dee_atomic_rwlock_t *blocking;
		blocking = Dee_slab_leaks_tryacquire();
		if unlikely(blocking) {
			local_unlock();
			Dee_atomic_rwlock_waitwrite(blocking);
			goto again;
#undef local_unlock_4
#define local_unlock_4() Dee_slab_leaks_release()
		}
	}
#endif /* HAVE_Dee_slab_leaks_tryacquire */
}

PRIVATE void DCALL dumpleaks_release_locks(void) {
	local_unlock();
}

#if LEAK_DETECTION_GC
#ifdef CONFIG_NO_THREADS
#define dumpleaks_acquire_locks_gc() dumpleaks_acquire_locks()
#define dumpleaks_release_locks_gc() dumpleaks_release_locks()
#else /* CONFIG_NO_THREADS */
INTDEF Dee_rshared_lock_t gc_lock;
INTDEF Dee_atomic_rwlock_t module_abstree_lock;
INTDEF Dee_atomic_rwlock_t module_libtree_lock;
INTDEF Dee_atomic_rwlock_t module_byaddr_lock;
INTDEF Dee_shared_lock_t thread_list_lock;


PRIVATE void DCALL dumpleaks_acquire_locks_gc(void) {
	/* Need a whole bunch of locks here:
	 * - gc_lock                           -- need to skew GC list and links
	 * - module_abstree_lock               -- need to skew "module_abstree_root"
	 * - module_libtree_lock               -- need to skew "module_libtree_root"
	 * - module_byaddr_lock                -- need to skew "module_byaddr_tree"
	 * - thread_list_lock                  -- needed to enumerate threads and prevent more from appearing
	 */
again:
	dumpleaks_acquire_locks();

	if (!Dee_rshared_lock_tryacquire(&gc_lock)) {
		local_unlock();
		Dee_rshared_lock_waitfor_noint(&gc_lock);
#undef local_unlock_5
#define local_unlock_5() Dee_rshared_lock_release(&gc_lock)
		goto again;
	}

	if (!Dee_atomic_rwlock_trywrite(&module_abstree_lock)) {
		local_unlock();
		Dee_atomic_rwlock_waitwrite(&module_abstree_lock);
#undef local_unlock_6
#define local_unlock_6() Dee_atomic_rwlock_endwrite(&module_abstree_lock)
		goto again;
	}

	if (!Dee_atomic_rwlock_trywrite(&module_libtree_lock)) {
		local_unlock();
		Dee_atomic_rwlock_waitwrite(&module_libtree_lock);
#undef local_unlock_7
#define local_unlock_7() Dee_atomic_rwlock_endwrite(&module_libtree_lock)
		goto again;
	}

	if (!Dee_atomic_rwlock_trywrite(&module_byaddr_lock)) {
		local_unlock();
		Dee_atomic_rwlock_waitwrite(&module_byaddr_lock);
#undef local_unlock_8
#define local_unlock_8() Dee_atomic_rwlock_endwrite(&module_byaddr_lock)
		goto again;
	}

	if (!Dee_shared_lock_tryacquire(&thread_list_lock)) {
		local_unlock();
		Dee_shared_lock_waitfor(&thread_list_lock);
#undef local_unlock_9
#define local_unlock_9() Dee_shared_lock_release(&thread_list_lock)
		goto again;
	}
}

PRIVATE void DCALL dumpleaks_release_locks_gc(void) {
	local_unlock();
}
#endif /* !CONFIG_NO_THREADS */
#endif /* LEAK_DETECTION_GC */

#undef local_unlock
#undef local_unlock_9
#undef local_unlock_8
#undef local_unlock_7
#undef local_unlock_6
#undef local_unlock_5
#undef local_unlock_4
#undef local_unlock_3
#undef local_unlock_2
#undef local_unlock_1
#undef local_unlock_0


PRIVATE NONNULL((1)) size_t DCALL
do_DeeHeap_DumpMemoryLeaks_one(struct leak_footer *leak
#if LEAK_DETECTION_GC
                               , bool print_gc_hits
#endif /* LEAK_DETECTION_GC */
                               ) {
	/* XXX: This read may return an invalid pointer if another thread just did a dlrealloc(chunk)! */
	void *mem = atomic_read(&leak->lf_chunk);
	mchunkptr p = mem2chunk(mem);
	size_t size = chunksize(p) - overhead_for(p);
	char const *file = atomic_read(&leak->lf_file);

#if LEAK_DETECTION_GC
	/* If "leak" is part of "leaks_pending_remove", skip it.
	 * This must be checked **AFTER** reading  */
	if (print_gc_hits && leaks_pending_remove__contains(leak))
		return 0;
#endif /* LEAK_DETECTION_GC */

#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
	if (leak_footer_isregion(leak)) {
		/* Print 1 line for every unreachable chunk within the region of "leak" */
		struct region_leak_footer *footer = leak_footer_asregion(leak);
		struct Dee_heapregion *region = region_leak_footer_getregion(footer);
		struct Dee_heapchunk *start = Dee_heapregion_getstart(region);
		struct Dee_heapchunk *end = Dee_heapregion_getend(region);
		struct Dee_heapchunk *iter;
		byte_t *region_maxaddr = (byte_t *)region + region->hr_size - 1;
		size_t index = 0;
		ASSERT((byte_t *)start == Dee_heapregion_getuminaddr(region));
		size = 0;
		for (iter = start; iter < end; iter = Dee_heapchunk_getnext(iter)) {
			size_t offset = (size_t)((byte_t *)iter - (byte_t *)start);
			size_t chunk_size = iter->hc_head & ~7;
			size_t chunk_head_word = offset / sizeof(void *);
			size_t chunk_body_word = chunk_head_word + (sizeof(struct Dee_heapchunk) / sizeof(void *));
			byte_t *payload_start = (byte_t *)start + (chunk_body_word * sizeof(void *));
			size_t payload_size = chunk_size - sizeof(struct Dee_heapchunk);
#if LEAK_DETECTION_GC
			if (!(iter->hc_head & PINUSE_BIT) &&
			    (!print_gc_hits || !region_leak_footer_getreachable(footer, chunk_head_word + 0)))
#else /* LEAK_DETECTION_GC */
			if (!(iter->hc_head & PINUSE_BIT))
#endif /* !LEAK_DETECTION_GC */
			{
				Dee_DPRINTF("%s : %" PRFuSIZ " : Leaked %" PRFuSIZ " bytes: %p-%p [in Dee_heapregion %p-%p]\n",
				            file, leak->lf_id + index, payload_size,
				            payload_start, (byte_t *)payload_start + payload_size - 1,
				            region, region_maxaddr);
				size += payload_size;
			}
			++index;
		}
	} else
#endif /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
#if LEAK_DETECTION_GC
	if (print_gc_hits) {
		Dee_DPRINTF("%s(%d) : %" PRFuSIZ " : Leaked %" PRFuSIZ " bytes of memory: "
		            /**/ "%p-%p (referenced %" PRFuSIZ " by other leaks)\n",
		            file, (int)leak->lf_line, leak->lf_id,
		            size, (byte_t *)mem, (byte_t *)mem + size - 1,
		            (size_t)(leak->lf_gcflags & LEAK_FOOTER_GCFLAG_HITMASK));
	} else
#endif /* LEAK_DETECTION_GC */
	{
		Dee_DPRINTF("%s(%d) : %" PRFuSIZ " : Leaked %" PRFuSIZ " bytes of memory: %p-%p\n",
		            file, (int)leak->lf_line, leak->lf_id,
		            size, (byte_t *)mem, (byte_t *)mem + size - 1);
	}
	return size;
}

#ifdef HAVE_Dee_slab_leaks_foreach_page
PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
do_DeeHeap_DumpMemoryLeaks_slabpage_cb(void *UNUSED(arg),
                                       struct Dee_slab_page *page,
                                       struct pagespecs const *specs) {
	size_t bitno;
	Dee_ssize_t result = 0;
	struct pageleaks *pl = (struct pageleaks *)page->sp_meta.spm_leak;
	slab_bitword_t *page__sp_used = (slab_bitword_t *)page->sp_used_and_data;
	byte_t *page__sp_data = (byte_t *)page->sp_used_and_data + specs->ps_sizeof__sp_used;
	if unlikely(!pl)
		goto done; /* Page has no leak information -> nothing in here can be a leak! */
	/* Sanity check info from "pl" is correct */
	if (pl->pl_chnksiz != specs->ps_chunksize ||
	    pl->pl_chnkcnt != specs->ps_chunkcount) {
		Dee_DPRINTF("? : ? : Slab page at %p has bad leaks metadata "
		            "[chnksiz:{expected:%" PRFuSIZ ", actual:%" PRFuSIZ "}"
		            ",chnkcnt:{expected:%" PRFuSIZ ", actual:%" PRFuSIZ "}]\n",
		            specs->ps_chunksize, pl->pl_chnksiz,
		            specs->ps_chunkcount, pl->pl_chnkcnt);
		result += 1;
	} else {
		for (bitno = 0; bitno < specs->ps_chunkcount; ++bitno) {
			size_t bit_indx = slab_bitword_indx(bitno);
			slab_bitword_t bit_mask = slab_bitword_mask(bitno);
			slab_bitword_t bit_word = atomic_read(&page__sp_used[bit_indx]);
			if (bit_word & bit_mask) {
				/* Found an allocated chunk -- look at its leak descriptor */
				struct chunkleak *leak = &pl->pl_chunks[bitno];
				if (leak->cl_file == NULL && leak->cl_line == -1) {
					/* Explicitly untracked leak */
				} else {
					byte_t *chunk = page__sp_data + (bitno * specs->ps_chunksize);
					Dee_DPRINTF("%s(%d) : ? : Leaked %" PRFuSIZ " bytes of memory: %p-%p [in slab page %p]\n",
					            leak->cl_file, (int)leak->cl_line, specs->ps_chunksize,
					            chunk, chunk + specs->ps_chunksize - 1);
					result += 1;
				}
			}
		}
	}
done:
	return result;
}
#endif /* HAVE_Dee_slab_leaks_foreach_page */

PRIVATE size_t DCALL do_DeeHeap_DumpMemoryLeaks_ALL(void) {
	size_t result = 0;
	struct leak_footer *iter;
	dumpleaks_acquire_locks();
#if 1 /* 1: Enumerate from oldest-to-newest; 0: newest-to-oldest */
	for (iter = leaks.lf_prev; iter != &leaks; iter = iter->lf_prev)
#else
	for (iter = leaks.lf_next; iter != &leaks; iter = iter->lf_next)
#endif
	{
#if LEAK_DETECTION_GC
		result += do_DeeHeap_DumpMemoryLeaks_one(iter, false);
#else /* LEAK_DETECTION_GC */
		result += do_DeeHeap_DumpMemoryLeaks_one(iter);
#endif /* !LEAK_DETECTION_GC */
	}

#ifdef HAVE_Dee_slab_leaks_foreach_page
	result += (size_t)Dee_slab_leaks_foreach_page(&do_DeeHeap_DumpMemoryLeaks_slabpage_cb, NULL);
#endif /* HAVE_Dee_slab_leaks_foreach_page */

#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
	if (region_leak_mlist) {
		struct Dee_heapregion *region = region_leak_mlist;
		do {
			result += region->hr_size;
			Dee_DPRINTF("? : ? : Leaked %" PRFuSIZ " bytes: %p-%p [in Dee_heapregion %p-%p]\n",
			            region->hr_size,
			            (byte_t *)region, (byte_t *)region + region->hr_size - 1,
			            (byte_t *)region, (byte_t *)region + region->hr_size - 1);
		} while ((region = Dee_heapregion_getnext_opt(region)) != NULL);
	}
#endif /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
	dumpleaks_release_locks();
	return result;
}

#if LEAK_DETECTION_GC
LOCAL ATTR_PURE WUNUSED NONNULL((1))
size_t DFCALL fast_usable_size(void *mem) {
	mchunkptr p = mem2chunk(mem);
	return chunksize(p) - overhead_for(p);
}

DECL_END
#include <hybrid/sequence/rbtree.h> /* LLRBTREE_NODE, LLRBTREE_ROOT, RBTREE_NODE, RBTREE_ROOT */

#define RBTREE_LEFT_LEANING
#define RBTREE(name)            gcleak_byaddr_##name
#define RBTREE_T                struct leak_footer
#define RBTREE_Tkey             byte_t *
#define RBTREE_GETLHS(self)     (self)->lf_prev
#define RBTREE_GETRHS(self)     (self)->lf_next
#define RBTREE_SETLHS(self, v)  (void)((self)->lf_prev = (v))
#define RBTREE_SETRHS(self, v)  (void)((self)->lf_next = (v))
#define RBTREE_GETMINKEY(self)  ((byte_t *)(self)->lf_chunk)
#define RBTREE_GETMAXKEY(self)  ((byte_t *)(self)->lf_chunk + fast_usable_size((self)->lf_chunk) - 1)
#define RBTREE_REDFIELD         lf_gcflags
#define RBTREE_REDBIT           LEAK_FOOTER_GCFLAG_RED
#define RBTREE_CC               DFCALL
#define RBTREE_NOTHROW          NOTHROW
#define RBTREE_DECL             PRIVATE
#define RBTREE_IMPL             PRIVATE
#define RBTREE_OMIT_REMOVE
#define RBTREE_DEBUG
#include <hybrid/sequence/rbtree-abi.h>

#define RBTREE_LEFT_LEANING
#define RBTREE(name)            gcleak_byhit_##name
#define RBTREE_T                struct leak_footer
#define RBTREE_Tkey             struct leak_footer const *
#define RBTREE_GETLHS(self)     (self)->lf_prev
#define RBTREE_GETRHS(self)     (self)->lf_next
#define RBTREE_SETLHS(self, v)  (void)((self)->lf_prev = (v))
#define RBTREE_SETRHS(self, v)  (void)((self)->lf_next = (v))
#define RBTREE_GETKEY(self)     (self)
#define RBTREE_KEY_LO(a, b)                                                                         \
	((((a)->lf_gcflags & ~LEAK_FOOTER_GCFLAG_RED) < ((b)->lf_gcflags & ~LEAK_FOOTER_GCFLAG_RED)) || \
	 (((a)->lf_gcflags & ~LEAK_FOOTER_GCFLAG_RED) == ((b)->lf_gcflags & ~LEAK_FOOTER_GCFLAG_RED) && ((a) < (b))))
#define RBTREE_KEY_EQ(a, b)     ((a) == (b))
#define RBTREE_REDFIELD         lf_gcflags
#define RBTREE_REDBIT           LEAK_FOOTER_GCFLAG_RED
#define RBTREE_CC               DFCALL
#define RBTREE_NOTHROW          NOTHROW
#define RBTREE_DECL             PRIVATE
#define RBTREE_IMPL             PRIVATE
#define RBTREE_WANT_NEXTNODE
#define RBTREE_OMIT_REMOVE
#define RBTREE_OMIT_LOCATE
#define RBTREE_OMIT_REMOVENODE
#include <hybrid/sequence/rbtree-abi.h>

#define RBTREE_LEFT_LEANING
#define RBTREE(name)            gcleak_byid_##name
#define RBTREE_T                struct leak_footer
#define RBTREE_Tkey             struct leak_footer const *
#define RBTREE_GETLHS(self)     (self)->lf_prev
#define RBTREE_GETRHS(self)     (self)->lf_next
#define RBTREE_SETLHS(self, v)  (void)((self)->lf_prev = (v))
#define RBTREE_SETRHS(self, v)  (void)((self)->lf_next = (v))
#define RBTREE_GETKEY(self)     (self)
#define RBTREE_KEY_LO(a, b)     (((a)->lf_id < (b)->lf_id) || ((a)->lf_id == (b)->lf_id && ((a) < (b))))
#define RBTREE_KEY_EQ(a, b)     ((a) == (b))
#define RBTREE_REDFIELD         lf_gcflags
#define RBTREE_REDBIT           LEAK_FOOTER_GCFLAG_RED
#define RBTREE_CC               DFCALL
#define RBTREE_NOTHROW          NOTHROW
#define RBTREE_DECL             PRIVATE
#define RBTREE_IMPL             PRIVATE
#define RBTREE_OMIT_REMOVE
#define RBTREE_OMIT_LOCATE
#include <hybrid/sequence/rbtree-abi.h>
DECL_BEGIN

PRIVATE LLRBTREE_ROOT(leak_footer) gcleak_byaddr_tree = NULL;        /* [0..n] gcleak_byaddr_* -- Used internally by GC leak detection */
PRIVATE LLRBTREE_ROOT(leak_footer) gcleak_byaddr_nreach_tree = NULL; /* [0..n] gcleak_byaddr_* -- Used internally by GC leak detection */
PRIVATE LLRBTREE_ROOT(leak_footer) gcleak_byhit_tree = NULL;         /* [0..n] gcleak_byhit_*  -- Used internally by GC leak detection */
PRIVATE LLRBTREE_ROOT(leak_footer) gcleak_byid_tree = NULL;          /* [0..n] gcleak_byid_*   -- Used internally by GC leak detection */


PRIVATE void DCALL gcmove__leaks__into__gcleak_byaddr_tree(void) {
	struct leak_footer *iter, *next;
	for (iter = leaks.lf_next; iter != &leaks; iter = next) {
		next = iter->lf_next;
		iter->lf_gcflags = LEAK_FOOTER_GCFLAG_NORMAL; /* Reset visibility flags */
		gcleak_byaddr_insert(&gcleak_byaddr_tree, iter);
#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
		if (leak_footer_isregion(iter)) {
			struct region_leak_footer *footer = leak_footer_asregion(iter);
			region_leak_footer_clearreachable(footer);
		}
#endif /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
	}
	leaks.lf_next = &leaks;
	leaks.lf_prev = &leaks;
#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
	if (region_leak_mlist) {
		struct Dee_heapregion *region = region_leak_mlist;
		do {
			Dee_heapregion_cleargcflags(region);
		} while ((region = Dee_heapregion_getnext_opt(region)) != NULL);
	}
#endif /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
}


#if !defined(NDEBUG) && 1
#define GCSCAN_LOGF(...) Dee_DPRINTF(__VA_ARGS__)
#else
#define GCSCAN_LOGF(...) (void)0
#endif


PRIVATE void DFCALL gcscan__range(void const *base, size_t num_bytes) {
	size_t i, num_words;
	if (!IS_ALIGNED((uintptr_t)base, sizeof(void *))) {
		size_t miss = sizeof(void *) - ((uintptr_t)base & (sizeof(void *) - 1));
		base = (byte_t *)base + miss;
		if (miss >= num_bytes)
			return;
		num_bytes -= miss;
	}
	num_words = num_bytes / sizeof(void *);
	for (i = 0; i < num_words; ++i) {
		void const *const *p_pointer = &((void const *const *)base)[i];
		void const *pointer = atomic_read(p_pointer);
		if (leak_addrok(pointer)) {
			struct leak_footer *leak;
			leak = gcleak_byaddr_locate(gcleak_byaddr_tree, (byte_t *)pointer);
			if (leak) {
				GCSCAN_LOGF("%s(%d) : HIT @%p -> %p (in %p-%p)\n",
				            leak->lf_file, leak->lf_line, p_pointer, pointer,
				            (byte_t *)leak->lf_chunk,
				            (byte_t *)leak->lf_chunk + fast_usable_size(leak->lf_chunk) - 1);
				leak->lf_gcflags |= LEAK_FOOTER_GCFLAG_REACH;
#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
				if (leak_footer_isregion(leak)) {
					struct region_leak_footer *footer = leak_footer_asregion(leak);
					struct Dee_heapregion *region = region_leak_footer_getregion(footer);
					byte_t *umin = Dee_heapregion_getuminaddr(region);
					if likely((byte_t *)pointer >= umin) {
						size_t uoffset = (size_t)((byte_t *)pointer - umin);
						size_t uword = uoffset / sizeof(void *);
						size_t words = Dee_heapregion_getuwordcount(region);
						if likely(uword < words)
							region_leak_footer_setreachable(footer, uword);
					}
				}
#endif /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
			}
#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
			if (region_leak_mlist) {
				struct Dee_heapregion *region = region_leak_mlist;
				do {
					if ((byte_t *)pointer > ((byte_t *)region) &&
					    (byte_t *)pointer < ((byte_t *)region + region->hr_size)) {
						Dee_heapregion_set_gc_reachable(region);
						break;
					}
				} while ((region = Dee_heapregion_getnext_opt(region)) != NULL);
			}
#endif /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
		}
	}
}

#define SKEWMASK             WORD_4BYTE(FFFFAAAA)
#define skew(T, p)           ((T)((uintptr_t)(p) ^ SKEWMASK))
#define unskew(T, p)         ((T)((uintptr_t)(p) ^ SKEWMASK))
#define inplace_skew(T, p)   (void)((p) = skew(T, p))
#define inplace_unskew(T, p) (void)((p) = unskew(T, p))


/* SKEW: gc_root */
INTDEF struct Dee_gc_head *gc_root;
PRIVATE void DCALL gcscan__skew__gc_root(void) {
	struct Dee_gc_head *next, *iter = gc_root;
	while (iter) {
		next = iter->gc_next;
		inplace_skew(struct Dee_gc_head *, iter->gc_next);
		inplace_skew(struct Dee_gc_head **, iter->gc_pself);
		iter = next;
	}
	inplace_skew(struct Dee_gc_head *, gc_root);
}
PRIVATE void DCALL gcscan__unskew__gc_root(void) {
	struct Dee_gc_head *iter;
	inplace_unskew(struct Dee_gc_head *, gc_root);
	iter = gc_root;
	while (iter) {
		inplace_unskew(struct Dee_gc_head *, iter->gc_next);
		inplace_unskew(struct Dee_gc_head **, iter->gc_pself);
		iter = iter->gc_next;
	}
}


/* SKEW: module_abstree_root */
INTDEF RBTREE_ROOT(Dee_module_object) module_abstree_root;
PRIVATE NONNULL((1)) void DCALL
gcscan__skew__module_abstree_root_at(DeeModuleObject *mod) {
	DeeModuleObject *lhs, *rhs;
again:
	lhs = mod->mo_absnode.rb_lhs;
	rhs = mod->mo_absnode.rb_rhs;
	inplace_skew(DeeModuleObject *, mod->mo_absnode.rb_lhs);
	inplace_skew(DeeModuleObject *, mod->mo_absnode.rb_rhs);
	if (lhs) {
		if (rhs)
			gcscan__skew__module_abstree_root_at(rhs);
		mod = lhs;
		goto again;
	}
	if (rhs) {
		mod = rhs;
		goto again;
	}
}
PRIVATE NONNULL((1)) void DCALL
gcscan__unskew__module_abstree_root_at(DeeModuleObject *mod) {
again:
	inplace_unskew(DeeModuleObject *, mod->mo_absnode.rb_lhs);
	inplace_unskew(DeeModuleObject *, mod->mo_absnode.rb_rhs);
	if (mod->mo_absnode.rb_lhs) {
		if (mod->mo_absnode.rb_rhs)
			gcscan__unskew__module_abstree_root_at(mod->mo_absnode.rb_rhs);
		mod = mod->mo_absnode.rb_lhs;
		goto again;
	}
	if (mod->mo_absnode.rb_rhs) {
		mod = mod->mo_absnode.rb_rhs;
		goto again;
	}
}
PRIVATE void DCALL gcscan__skew__module_abstree_root(void) {
	if (module_abstree_root)
		gcscan__skew__module_abstree_root_at(module_abstree_root);
	inplace_skew(DeeModuleObject *, module_abstree_root);
}
PRIVATE void DCALL gcscan__unskew__module_abstree_root(void) {
	inplace_unskew(DeeModuleObject *, module_abstree_root);
	if (module_abstree_root)
		gcscan__unskew__module_abstree_root_at(module_abstree_root);
}


/* SKEW: module_libtree_root */
INTDEF RBTREE_ROOT(Dee_module_libentry) module_libtree_root;
PRIVATE NONNULL((1)) void DCALL
gcscan__skew__module_libtree_root_at(struct Dee_module_libentry *mod) {
	struct Dee_module_libentry *lhs, *rhs;
again:
	lhs = mod->mle_node.rb_lhs;
	rhs = mod->mle_node.rb_rhs;
	inplace_skew(struct Dee_module_libentry *, mod->mle_node.rb_lhs);
	inplace_skew(struct Dee_module_libentry *, mod->mle_node.rb_rhs);
	if (lhs) {
		if (rhs)
			gcscan__skew__module_libtree_root_at(rhs);
		mod = lhs;
		goto again;
	}
	if (rhs) {
		mod = rhs;
		goto again;
	}
}
PRIVATE NONNULL((1)) void DCALL
gcscan__unskew__module_libtree_root_at(struct Dee_module_libentry *mod) {
again:
	inplace_unskew(struct Dee_module_libentry *, mod->mle_node.rb_lhs);
	inplace_unskew(struct Dee_module_libentry *, mod->mle_node.rb_rhs);
	if (mod->mle_node.rb_lhs) {
		if (mod->mle_node.rb_rhs)
			gcscan__unskew__module_libtree_root_at(mod->mle_node.rb_rhs);
		mod = mod->mle_node.rb_lhs;
		goto again;
	}
	if (mod->mle_node.rb_rhs) {
		mod = mod->mle_node.rb_rhs;
		goto again;
	}
}
PRIVATE void DCALL gcscan__skew__module_libtree_root(void) {
	if (module_libtree_root)
		gcscan__skew__module_libtree_root_at(module_libtree_root);
	inplace_skew(struct Dee_module_libentry *, module_libtree_root);
}
PRIVATE void DCALL gcscan__unskew__module_libtree_root(void) {
	inplace_unskew(struct Dee_module_libentry *, module_libtree_root);
	if (module_libtree_root)
		gcscan__unskew__module_libtree_root_at(module_libtree_root);
}


/* SKEW: module_abstree_root */
INTDEF RBTREE_ROOT(/*maybe(DREF)*/ Dee_module_object) module_byaddr_tree;
PRIVATE NONNULL((1)) void DCALL
gcscan__skew__module_byaddr_tree_at(DeeModuleObject *mod) {
	DeeModuleObject *lhs, *rhs;
again:
	lhs = mod->mo_adrnode.rb_lhs;
	rhs = mod->mo_adrnode.rb_rhs;
	inplace_skew(DeeModuleObject *, mod->mo_adrnode.rb_lhs);
	inplace_skew(DeeModuleObject *, mod->mo_adrnode.rb_rhs);
	if (lhs) {
		if (rhs)
			gcscan__skew__module_byaddr_tree_at(rhs);
		mod = lhs;
		goto again;
	}
	if (rhs) {
		mod = rhs;
		goto again;
	}
}
PRIVATE NONNULL((1)) void DCALL
gcscan__unskew__module_byaddr_tree_at(DeeModuleObject *mod) {
again:
	inplace_unskew(DeeModuleObject *, mod->mo_adrnode.rb_lhs);
	inplace_unskew(DeeModuleObject *, mod->mo_adrnode.rb_rhs);
	if (mod->mo_adrnode.rb_lhs) {
		if (mod->mo_adrnode.rb_rhs)
			gcscan__unskew__module_byaddr_tree_at(mod->mo_adrnode.rb_rhs);
		mod = mod->mo_adrnode.rb_lhs;
		goto again;
	}
	if (mod->mo_adrnode.rb_rhs) {
		mod = mod->mo_adrnode.rb_rhs;
		goto again;
	}
}
PRIVATE void DCALL gcscan__skew__module_byaddr_tree(void) {
	if (module_byaddr_tree)
		gcscan__skew__module_byaddr_tree_at(module_byaddr_tree);
	inplace_skew(DeeModuleObject *, module_byaddr_tree);
}
PRIVATE void DCALL gcscan__unskew__module_byaddr_tree(void) {
	inplace_unskew(DeeModuleObject *, module_byaddr_tree);
	if (module_byaddr_tree)
		gcscan__unskew__module_byaddr_tree_at(module_byaddr_tree);
}




PRIVATE void DCALL gcscan__skew(void) {
	/* (intentionally) skew certain globals that shouldn't be considered
	 * as reasons why some allocations should be reachable:
	 *
	 * - module_abstree_root, and "DeeModuleObject::mo_absnode"
	 *   doesn't contain object references
	 *
	 * - module_libtree_root, and "Dee_module_libentry::mle_node"
	 *   doesn't contain object references
	 *
	 * - module_byaddr_tree, and "DeeModuleObject::mo_adrnode"
	 *   doesn't contain object references in case of DeeModuleDee_Type,
	 *   and even though it does contain references for DeeModuleDex_Type,
	 *   native dex modules are explicitly scanned anyways, so nothing is
	 *   gained if these objects were reachable.
	 *
	 * - gc_root, and "struct Dee_gc_head" of every tracked GC object */
	gcscan__skew__gc_root();
	gcscan__skew__module_abstree_root();
	gcscan__skew__module_libtree_root();
	gcscan__skew__module_byaddr_tree();
}

PRIVATE void DCALL gcscan__unskew(void) {
	gcscan__unskew__module_byaddr_tree();
	gcscan__unskew__module_libtree_root();
	gcscan__unskew__module_abstree_root();
	gcscan__unskew__gc_root();
}

PRIVATE bool DCALL gcscan__statics_from_core(void) {
	if (DeeModule_Deemon.mo_flags & _Dee_MODULE_FNOADDR)
		return false;
	gcscan__skew();
	GCSCAN_LOGF("DEEMON CORE: Scan %p-%p\n", DeeModule_Deemon.mo_minaddr, DeeModule_Deemon.mo_maxaddr);
	gcscan__range(DeeModule_Deemon.mo_minaddr, (DeeModule_Deemon.mo_maxaddr - DeeModule_Deemon.mo_minaddr) + 1);
	GCSCAN_LOGF("Scan end %p-%p\n", DeeModule_Deemon.mo_minaddr, DeeModule_Deemon.mo_maxaddr);
	gcscan__unskew();
	return true;
}

#ifndef CONFIG_NO_DEX
PRIVATE void DCALL gcscan__statics_at(DeeModuleObject *mod) {
again:
	if (Dee_TYPE(mod) == &DeeModuleDex_Type && mod != &DeeModule_Deemon) {
		GCSCAN_LOGF("DEX %q: Scan %p-%p\n", mod->mo_absname, mod->mo_minaddr, mod->mo_maxaddr);
		gcscan__range(mod->mo_minaddr, (mod->mo_maxaddr - mod->mo_minaddr) + 1);
		GCSCAN_LOGF("Scan end %p-%p\n", mod->mo_minaddr, mod->mo_maxaddr);
	}
	if (mod->mo_adrnode.rb_lhs) {
		if (mod->mo_adrnode.rb_rhs)
			gcscan__statics_at(mod->mo_adrnode.rb_rhs);
		mod = mod->mo_adrnode.rb_lhs;
		goto again;
	}
	if (mod->mo_adrnode.rb_rhs) {
		mod = mod->mo_adrnode.rb_rhs;
		goto again;
	}
}
#endif /* !CONFIG_NO_DEX */

#ifndef CONFIG_NO_DEX
INTDEF RBTREE_ROOT(DREF Dee_module_object) dex_byaddr_tree;
#endif /* !CONFIG_NO_DEX */

PRIVATE bool DCALL gcscan__statics(void) {
#ifndef CONFIG_NO_DEX
	if (dex_byaddr_tree) {
		/* Address ranges for these modules can't be
		 * determined, so scanning is impossible :( */
		return false;
	}
	if (module_byaddr_tree)
		gcscan__statics_at(module_byaddr_tree);
#endif /* !CONFIG_NO_DEX */
	gcscan__statics_from_core();
	return true;
}

PRIVATE NONNULL((1)) bool DCALL
gcscan__thread(DeeThreadObject *thread) {
	/* Scan memory from thread's stack base to its current SP
	 * (it's OK if the SP becomes outdated during the scan, so
	 * long as the thread doesn't switch to a different thread
	 * and deallocate its current one) */
	/* TODO */

	/* Scan the thread's register file */
	/* TODO */
	return true;
}

INTDEF DeeThreadObject DeeThread_Main;
#ifdef CONFIG_NO_THREADS
#define gcscan__threads() gcscan__thread(&DeeThread_Main)
#else /* CONFIG_NO_THREADS */
PRIVATE bool DCALL gcscan__threads(void) {
	DeeThreadObject *iter = &DeeThread_Main;
	do {
		if (!gcscan__thread(iter))
			return false;
	} while ((iter = iter->t_global.le_next) != NULL);
	return true;
}
#endif /* !CONFIG_NO_THREADS */

PRIVATE void DCALL gcscan__visit__leaks_pending_remove(void) {
	struct leak_footer *remove;
	remove = atomic_read(&leaks_pending_remove.slh_first);
	for (; remove; remove = SLIST_NEXT(remove, lf_remlink))
		remove->lf_gcflags |= LEAK_FOOTER_GCFLAG_REACH;
}

PRIVATE NONNULL((1)) bool DCALL
gcscan__visit__reachable_at(struct leak_footer *root) {
	bool result = false;
again:
	if ((root->lf_gcflags & (LEAK_FOOTER_GCFLAG_REACH | LEAK_FOOTER_GCFLAG_SCANNED)) ==
	    /*                */(LEAK_FOOTER_GCFLAG_REACH)) {
		void *chunk;
		root->lf_gcflags |= LEAK_FOOTER_GCFLAG_SCANNED;
		/* XXX: This read may return an invalid pointer if another thread just did a dlrealloc(chunk)! */
		chunk = atomic_read(&root->lf_chunk);
#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
		if (leak_footer_isregion(root)) {
			/* Use reachable bits associated with "hc_prevsize" and
			 * "hc_head" of  payload chunks as GC control bits:
			 * - hc_prevsize: reachable
			 * - hc_head:     visited */
			struct region_leak_footer *footer = leak_footer_asregion(root);
			struct Dee_heapregion *region = region_leak_footer_getregion(footer);
			struct Dee_heapchunk *start = Dee_heapregion_getstart(region);
			struct Dee_heapchunk *end = Dee_heapregion_getend(region);
			struct Dee_heapchunk *iter;
			bool has_unreachable_chunk = false;
			ASSERT((byte_t *)start == Dee_heapregion_getuminaddr(region));
			for (iter = start; iter < end; iter = Dee_heapchunk_getnext(iter)) {
				size_t offset = (size_t)((byte_t *)iter - (byte_t *)start);
				size_t chunk_size = iter->hc_head & ~FLAG_BITS;
				size_t chunk_head_word = offset / sizeof(void *);
				size_t chunk_body_word = chunk_head_word + (sizeof(struct Dee_heapchunk) / sizeof(void *));
				size_t chunk_tail_word = chunk_head_word + (chunk_size / sizeof(void *));
				if (!region_leak_footer_getreachable(footer, chunk_head_word + 0)) {
					bool payload_reachable = false;
					size_t i;
					for (i = chunk_body_word; i < chunk_tail_word; ++i) {
						if (region_leak_footer_getreachable(footer, i)) {
							payload_reachable = true;
							break;
						}
					}
					if (!payload_reachable) {
						has_unreachable_chunk = true;
						continue;
					}
					/* Remember that this chunk is reachable */
					region_leak_footer_setreachable(footer, chunk_head_word + 0);
				}
				if (!region_leak_footer_getreachable(footer, chunk_head_word + 1)) {
					/* Visit payload area now */
					byte_t *payload_start = (byte_t *)start + (chunk_body_word * sizeof(void *));
					size_t payload_size = chunk_size - sizeof(struct Dee_heapchunk);
					region_leak_footer_setreachable(footer, chunk_head_word + 1);
					gcscan__range(payload_start, payload_size);
					result = true;
				}
			}
			if (has_unreachable_chunk) /* Keep checking if there are unscanned parts... */
				root->lf_gcflags &= ~LEAK_FOOTER_GCFLAG_SCANNED;
		} else
#endif /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
		{
			GCSCAN_LOGF("%s(%d) : Scan %p-%p\n", root->lf_file, root->lf_line,
			            chunk, (byte_t *)chunk + fast_usable_size(chunk) - 1);
			gcscan__range(chunk, fast_usable_size(chunk));
			GCSCAN_LOGF("Scan end %p-%p\n", chunk, (byte_t *)chunk + fast_usable_size(chunk) - 1);
			result = true;
		}
	}
	if (root->lf_prev) {
		if (root->lf_next)
			result |= gcscan__visit__reachable_at(root->lf_next);
		root = root->lf_prev;
		goto again;
	}
	if (root->lf_next) {
		root = root->lf_next;
		goto again;
	}
	return result;
}

PRIVATE bool DCALL gcscan__visit__reachable(void) {
	bool result = false;
	if (gcleak_byaddr_tree)
		result = gcscan__visit__reachable_at(gcleak_byaddr_tree);
#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
	if (region_leak_mlist) {
		struct Dee_heapregion *region = region_leak_mlist;
		do {
			if (Dee_heapregion_get_gc_reachable(region) &&
			    !Dee_heapregion_get_gc_visited(region)) {
				Dee_heapregion_set_gc_visited(region);
				gcscan__range(Dee_heapregion_getdata(region),
				              Dee_heapregion_getdatasize(region));
				result = true;
			}
		} while ((region = Dee_heapregion_getnext_opt(region)) != NULL);
	}
#endif /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
	return result;
}


PRIVATE NONNULL((1)) bool DCALL
gcmove__byaddr__into__nreach_at(struct leak_footer *root) {
again:
	/* Check the "LEAK_FOOTER_GCFLAG_SCANNED" flag instead of "LEAK_FOOTER_GCFLAG_REACH".
	 * This way, when `leak_footer_isregion(root)', we move the region into the set of
	 * unreachable chunks if at least one of the embedded chunk is unreachable, since
	 * `gcscan__visit__reachable_at()' will only set "LEAK_FOOTER_GCFLAG_SCANNED" if
	 * **all** embedded chunks are reachable. */
	if (!(root->lf_gcflags & LEAK_FOOTER_GCFLAG_SCANNED)) {
		gcleak_byaddr_removenode(&gcleak_byaddr_tree, root);
		root->lf_gcflags = 0; /* Reset hit-counter (though should already be 0) */
		gcleak_byaddr_insert(&gcleak_byaddr_nreach_tree, root);
		return true;
	}
	if (root->lf_prev) {
		if (root->lf_next && gcmove__byaddr__into__nreach_at(root->lf_next))
			return true;
		root = root->lf_prev;
		goto again;
	}
	if (root->lf_next) {
		root = root->lf_next;
		goto again;
	}
	return false;
}

PRIVATE void DCALL gcmove__byaddr__into__nreach(void) {
	while (gcleak_byaddr_tree) {
		if (!gcmove__byaddr__into__nreach_at(gcleak_byaddr_tree))
			break;
	}
}


PRIVATE NONNULL((1)) void DCALL
gccount__nreach__crossrefs_at(struct leak_footer *root) {
	void *chunk;
	size_t i, csize, words;
again:
	chunk = atomic_read(&root->lf_chunk);
	csize = fast_usable_size(chunk);
	ASSERT(IS_ALIGNED((uintptr_t)chunk, sizeof(void *)));
	ASSERT(IS_ALIGNED((uintptr_t)csize, sizeof(void *)));
	words = csize / sizeof(void *);
	for (i = 0; i < words; ++i) {
		void const *const *p_pointer = &((void const *const *)chunk)[i];
		void const *pointer = atomic_read(p_pointer);
		if (leak_addrok(pointer)) {
			struct leak_footer *leak;
			leak = gcleak_byaddr_locate(gcleak_byaddr_nreach_tree, (byte_t *)pointer);
			if (leak && (leak->lf_gcflags & LEAK_FOOTER_GCFLAG_HITMASK) != LEAK_FOOTER_GCFLAG_HITMASK)
				leak->lf_gcflags += LEAK_FOOTER_GCFLAG_ONEHIT;
		}
	}
	if (root->lf_prev) {
		if (root->lf_next)
			gccount__nreach__crossrefs_at(root->lf_next);
		root = root->lf_prev;
		goto again;
	}
	if (root->lf_next) {
		root = root->lf_next;
		goto again;
	}
}

PRIVATE void DCALL gccount__nreach__crossrefs(void) {
	if (gcleak_byaddr_nreach_tree)
		gccount__nreach__crossrefs_at(gcleak_byaddr_nreach_tree);
}


PRIVATE NONNULL((1)) void DCALL
gcmove__nreach__into__byhit_at(struct leak_footer *root) {
	struct leak_footer *lhs, *rhs;
again:
	lhs = root->lf_prev;
	rhs = root->lf_next;
	gcleak_byhit_insert(&gcleak_byhit_tree, root);
	if (lhs) {
		if (rhs)
			gcmove__nreach__into__byhit_at(rhs);
		root = lhs;
		goto again;
	}
	if (rhs) {
		root = rhs;
		goto again;
	}
}

PRIVATE void DCALL gcmove__nreach__into__byhit(void) {
	if (gcleak_byaddr_nreach_tree) {
		gcmove__nreach__into__byhit_at(gcleak_byaddr_nreach_tree);
		gcleak_byaddr_nreach_tree = NULL;
	}
}


PRIVATE NONNULL((1)) void DCALL
gcmove__foo__into__byid_at(struct leak_footer *root) {
	struct leak_footer *lhs, *rhs;
again:
	lhs = root->lf_prev;
	rhs = root->lf_next;
	gcleak_byid_insert(&gcleak_byid_tree, root);
	if (lhs) {
		if (rhs)
			gcmove__foo__into__byid_at(rhs);
		root = lhs;
		goto again;
	}
	if (rhs) {
		root = rhs;
		goto again;
	}
}

PRIVATE void DCALL gcmove__byhit__into__byid(void) {
	if (gcleak_byhit_tree) {
		gcmove__foo__into__byid_at(gcleak_byhit_tree);
		gcleak_byhit_tree = NULL;
	}
}

PRIVATE void DCALL gcmove__byaddr__into__byid(void) {
	if (gcleak_byaddr_tree) {
		gcmove__foo__into__byid_at(gcleak_byaddr_tree);
		gcleak_byaddr_tree = NULL;
	}
}

PRIVATE void DCALL gcmove__byid__into__leaks(void) {
	struct leak_footer *iter;
	while ((iter = gcleak_byid_tree) != NULL) {
		while (iter->lf_prev)
			iter = iter->lf_prev;
		gcleak_byid_removenode(&gcleak_byid_tree, iter);
		leaks_insert(iter);
	}
}


PRIVATE size_t DCALL gcdump__byhit(void) {
	size_t result = 0;
	if (gcleak_byhit_tree) {
		struct leak_footer *iter = gcleak_byhit_tree;
		while (iter->lf_prev)
			iter = iter->lf_prev;
		do {
			result += do_DeeHeap_DumpMemoryLeaks_one(iter, true);
		} while ((iter = gcleak_byhit_nextnode(gcleak_byhit_tree, iter)) != NULL);
	}

#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
	if (region_leak_mlist) {
		struct Dee_heapregion *region = region_leak_mlist;
		do {
			if (!Dee_heapregion_get_gc_reachable(region)) {
				result += region->hr_size;
				Dee_DPRINTF("? : ? : Leaked %" PRFuSIZ " bytes from a custom 'Dee_heapregion': %p-%p\n",
				            region->hr_size, (byte_t *)region, (byte_t *)region + region->hr_size - 1);
			}
		} while ((region = Dee_heapregion_getnext_opt(region)) != NULL);
	}
#endif /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
	return result;
}



/* @return: (size_t)-1: Unsupported :( */
PRIVATE size_t DCALL do_DeeHeap_DumpMemoryLeaks_GC_locked(void) {
	size_t result = (size_t)-1;

	/* #0: Reset state */
	gcleak_byaddr_tree        = NULL;
	gcleak_byaddr_nreach_tree = NULL;
	gcleak_byhit_tree         = NULL;
	gcleak_byid_tree          = NULL;

	/* #1: Convert "leaks" into a by-address R/B-tree "gcleak_byaddr_tree" */
	gcmove__leaks__into__gcleak_byaddr_tree();

#ifdef Dee_SLAB_CHUNKSIZE_MAX
	/* TODO: Move allocated slab pages into "gcleak_byaddr_tree" */
#endif /* Dee_SLAB_CHUNKSIZE_MAX */

	/* #2: Scan static memory locations of the deemon core and all dex modules for
	 *     stuff that looks like pointers. Every pointer found is checked for being
	 *     located within "gcleak_byaddr_tree". If so: "LEAK_FOOTER_GCFLAG_REACH" */
	if (!gcscan__statics())
		goto convert_trees_to_byid;

	/* #3: Same as #2, but repeat for stack memory, and contents of GP registers
	 *     of every running thread. This is the aforementioned arch-/os-specific
	 *     part of this whole process. */
	if (!gcscan__threads())
		goto convert_trees_to_byid;

	/* #4: Enumerate "leaks_pending_remove" (by atomically reading its current head
	 *     and walking its "lf_remlink" chain) and set "LEAK_FOOTER_GCFLAG_REACH"
	 *     of every contained leak. -- this is needed to exclude pointers that were
	 *     since freed in other threads from the list of leaks. */
	gcscan__visit__leaks_pending_remove();

	/* #5: Recursively check the bodies of "LEAK_FOOTER_GCFLAG_REACH"-nodes for
	 *     other more pointers to other (not already reached) nodes apart of
	 *     "gcleak_byaddr_tree". Only do this if "LEAK_FOOTER_GCFLAG_SCANNED"
	 *     is not set, and always set "LEAK_FOOTER_GCFLAG_SCANNED" during this
	 *     part. Repeat until no reachable, but not-scanned nodes remain. */
	do {
		/* ... */
	} while (gcscan__visit__reachable());

	/* #6: Take all nodes without "LEAK_FOOTER_GCFLAG_REACH" set out of
	 *     "gcleak_byaddr_tree" and move them into "gcleak_byaddr_nreach_tree" */
	gcmove__byaddr__into__nreach();

	/* #7: Go every element of "gcleak_byaddr_nreach_tree" exactly once, and scan the
	 *     body of every leak found within for nested pointers to other elements also
	 *     found within "gcleak_byaddr_nreach_tree". For every such hit, increment the
	 *     referenced leak's `lf_gcflags' field by 1, but don't increment so far that
	 *     "LEAK_FOOTER_GCFLAG_RED" would be modified (stop one short of that
	 *     potentially happening) */
	gccount__nreach__crossrefs();

	/* #8: Convert "gcleak_byaddr_nreach_tree" into "gcleak_byhit_tree", thus
	 *     sorting all memory leaks by how often they reference each other. Here,
	 *     leaks that are referenced less often than others are more interesting
	 *     since those leaks represent leaked control structures, as opposed to
	 *     leaked strings or similarly boring things... */
	gcmove__nreach__into__byhit();

	/* #9: Enumerate the elements of "gcleak_byhit_tree" from its minimum to
	 *     its maximum, and print every encountered element as a leak. Also
	 *     include info about how often each leak was referenced by some other
	 *     leak, but otherwise do the same as `do_DeeHeap_DumpMemoryLeaks_ALL()'.
	 *     Also like that function, must explicitly check "leaks_pending_remove"
	 *     for containing discovered leaks **after** reading the "lf_file"
	 *     pointer to every leak, so-as to exclude leaks that were since freed. */
	result = gcdump__byhit();

convert_trees_to_byid:
	/* #10: Convert both "gcleak_byhit_tree" and "gcleak_byaddr_tree" (from #6)
	 *      into "gcleak_byid_tree", thus re-sorting all still-allocated nodes
	 *      into a form where they can easily be enumerated from oldest->newest
	 *      in terms of when the specific allocation happened. */
	gcmove__byhit__into__byid();
	gcmove__byaddr__into__byid();

	/* #11: Convert "gcleak_byid_tree" back into "leaks" such that the most-
	 *      recently allocated chunk is at `leaks.lf_next', and the least-
	 *      recently allocated chunk is at `leaks.lf_prev'. */
	gcmove__byid__into__leaks();

	return result;
}

/* @return: (size_t)-1: Unsupported :( */
PRIVATE size_t DCALL do_DeeHeap_DumpMemoryLeaks_GC(void) {
	size_t result;
	dumpleaks_acquire_locks_gc();
	result = do_DeeHeap_DumpMemoryLeaks_GC_locked();
	dumpleaks_release_locks_gc();
	return result;
}
#endif /* LEAK_DETECTION_GC */

/* Dump info about all heap allocations that were allocated, but
 * never Dee_Free()'d, nor untracked using `Dee_UntrackAlloc()'.
 * Information about leaks is printed using `Dee_DPRINTF()'.
 *
 * @param: method: One of `DeeHeap_DumpMemoryLeaks_*'
 * @return: * : The total amount of memory leaked (in bytes) */
#define DeeHeap_DumpMemoryLeaks_DEFINED
PUBLIC size_t DCALL DeeHeap_DumpMemoryLeaks(unsigned int method) {
	size_t result = 0;
	switch (method) {
	case DeeHeap_DumpMemoryLeaks_GC_OR_ALL:
#if LEAK_DETECTION_GC
	case DeeHeap_DumpMemoryLeaks_GC:
		result = do_DeeHeap_DumpMemoryLeaks_GC();
		if (result != (size_t)-1)
			break;
		result = 0;
		if (method == DeeHeap_DumpMemoryLeaks_GC)
			break;
		ATTR_FALLTHROUGH
#endif /* LEAK_DETECTION_GC */
	case DeeHeap_DumpMemoryLeaks_ALL:
		result = do_DeeHeap_DumpMemoryLeaks_ALL();
		break;
	default: break;
	}
	return result;
}




PRIVATE ATTR_HOT NONNULL((1, 2)) void DCALL
leak_insert_p(void *ptr, struct leak_footer *leak,
              char const *file, int line) {
	/* Hook "leak" into "ptr" */
	mchunkptr p = mem2chunk(ptr);
	size_t *p_tail = chunk_p_foot(p);
	size_t alloc_break;
	leak->lf_foot  = *p_tail;
	leak->lf_chunk = ptr;
	leak->lf_file  = file;
	leak->lf_line  = (leak_footer_line_t)line;
	*p_tail = (size_t)(uintptr_t)leak;

	/* Allocate ID */
	leak->lf_id = get_alloc_id(p, leak->lf_foot);
	alloc_break = atomic_read(&alloc_id_break);
	if (leak->lf_id == alloc_break && alloc_break != 0) {
#ifndef Dee_BREAKPOINT_IS_NOOP
		Dee_BREAKPOINT();
#elif __has_builtin(__builtin_trap)
		__builtin_trap();
#else /* ... */
		byte_t volatile *volatile P = (byte_t volatile *)NULL;
		*P = 'B';
#endif /* !... */
	}

	/* Insert into list of memory leaks (without ever blocking) */
	leaks_insert_begin(leak);
	if (leaks_lock_tryacquire()) {
		leaks_insert_finish(leak);
		leaks_lock_release();
	} else {
		/* Schedule "insert" to happen asynchronously */
		leaks_pending_insert_add(leak);
		leaks_lock_reap();
	}
}

/* Amend debug into to "ptr" and re-return. On error, dlfree(ptr) and return "NULL" */
PRIVATE ATTR_HOT WUNUSED NONNULL((1)) void *DCALL
leak_insert(void *ptr, char const *file, int line) {
	struct leak_footer *foot = leak_footer_alloc();
	if likely(foot) {
		leak_insert_p(ptr, foot, file, line);
		return ptr;
	}
	Dee_DPRINTF("[RT] Failed to allocate 'leak_footer' for %" PRFuSIZ "-byte large heap pointer %p\n",
	            Dee_MallocUsableSizeNonNull(ptr), ptr);
	dlfree(ptr);
	return NULL;
}


#define DeeDbg_TryMalloc_DEFINED
PUBLIC ATTR_HOT ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *
(DCALL DeeDbg_TryMalloc)(size_t n_bytes, char const *file, int line) {
	void *result = dlmalloc(n_bytes);
	if (result)
		result = leak_insert(result, file, line);
	return result;
}

#define DeeDbg_TryCalloc_DEFINED
PUBLIC ATTR_HOT ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *
(DCALL DeeDbg_TryCalloc)(size_t n_bytes, char const *file, int line) {
	void *result = dlcalloc(n_bytes);
	if (result)
		result = leak_insert(result, file, line);
	return result;
}

#define DeeDbg_TryMemalign_DEFINED
PUBLIC ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *
(DCALL DeeDbg_TryMemalign)(size_t min_alignment, size_t n_bytes,
                           char const *file, int line) {
	void *result = dlmemalign(min_alignment, n_bytes);
	if (result)
		result = leak_insert(result, file, line);
	return result;
}


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
#if FLAG4_BIT_HEAP_REGION
	if unlikely(flag4inuse(p)) {
#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
		leaks_lock_acquire_and_reap(); /* Acquire+reap required for "dl_dbg_heapregion_dispose()" */
		dlfree(ptr);
		leaks_lock_release();
#else /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
		dlfree(ptr);
#endif /* !FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
		return;
	}
#endif /* FLAG4_BIT_HEAP_REGION */

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
		leaks_pending_remove_add(leak);
		leaks_lock_reap();
		return;
	}

	/* If the leak hasn't finished being inserted into the leaks list,
	 * then we have to ensure that all pending inserts have been reaped! */
#ifdef leaks_insert_is_unfinished
	if (leaks_insert_is_unfinished(leak)) {
#if USE_PER_MSPACE_LEAK_OPS
		mstate ms = leak_get_pending_mstate(leak);
		mspace_leaks_lock_reap_insert_locked(ms);
#else /* USE_PER_MSPACE_LEAK_OPS */
		leaks_lock_reap_insert_locked();
#endif /* !USE_PER_MSPACE_LEAK_OPS */
		dl_assert(!leaks_insert_is_unfinished(leak));
	}
#else /* leaks_insert_is_unfinished */
	if (leaks_pending_mustreap_insert())
		leaks_lock_reap_insert_locked();
#endif /* !leaks_insert_is_unfinished */

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
#if FLAG4_BIT_HEAP_REGION
	if unlikely(flag4inuse(p)) {
		/* Muse use "DeeDbg_TryMalloc()" so the new chunk gets a "struct leak_footer" */
		result = DeeDbg_TryMalloc(n_bytes, file, line);
		if likely(result) {
			size_t oc = chunksize(p) - overhead_for(p);
			result = memcpy(result, ptr, (oc < n_bytes) ? oc : n_bytes);
#if FLAG4_BIT_HEAP_REGION_DBG_DISPOSE
			leaks_lock_acquire_and_reap(); /* Acquire+reap required for "dl_dbg_heapregion_dispose()" */
			dlfree(ptr);
			leaks_lock_release();
#else /* FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
			dlfree(ptr);
#endif /* !FLAG4_BIT_HEAP_REGION_DBG_DISPOSE */
		}
		return result;
	}
#endif /* FLAG4_BIT_HEAP_REGION */

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
		leak->lf_line = (leak_footer_line_t)line;
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
#if FLAG4_BIT_HEAP_REGION
	if unlikely(flag4inuse(p))
		return NULL; /* Cannot be done */
#endif /* FLAG4_BIT_HEAP_REGION */

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
	dl_assert(result == NULL || result == ptr);
	dl_assert(p == mem2chunk(ptr));
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
#if FLAG4_BIT_HEAP_REGION
	if unlikely(flag4inuse(p)) {
		fatal("Dee_UntrackAlloc(ptr)", file, line,
		      "%p: Not allowed to untrack FLAG4 memory", ptr);
	}
#endif /* FLAG4_BIT_HEAP_REGION */

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
#if USE_PER_MSPACE_LEAK_OPS
	if unlikely(leaks_insert_is_unfinished(leak)) {
		/* This can happen under a rare race condition, where one thread is currently
		 * still within "mstate_set_haspending()", having just set that mspace's
		 * "ms_leaks_pending_next" field such that it indicates that the mspace is
		 * part of the global list of spaces with pending tasks.
		 *
		 * Then, our thread makes the same call after using leaks_pending_insert_add()
		 * to insert the leak which we're trying to untrack here, but sees that
		 * "ms_leaks_pending_next" indicates that the mspace is already part of the
		 * global list (even though it isn't yet, as the other thread hasn't fully
		 * added it yet).
		 *
		 * Then our thread continues up to here, trying to untrack the leak, but that
		 * ends up failing because the leak hasn't actually been fully tracked, yet.
		 * But the leak's mspace isn't part of "leaks_pending_states" either, since
		 * it's still in the process of being added!
		 *
		 * So to prevent issues in this situation, simply do a man insert-reap of
		 * the relevant mspace at this point. */
		mstate ms = leak_get_pending_mstate(leak);
		mspace_leaks_lock_reap_insert_locked(ms);
		dl_assert(!leaks_insert_is_unfinished(leak));
	}
#endif /* USE_PER_MSPACE_LEAK_OPS */
	leaks_remove(leak);
	leaks_lock_release();

	/* Form a dummy loop that will satisfy `Dee_Free()' and `Dee_Realloc()',
	 * but won't show up in `DeeHeap_DumpMemoryLeaks()' */
	leak->lf_prev = leak;
	leak->lf_next = leak;
	return ptr;
}


#elif (LEAK_DETECTION == LEAK_DETECTION_METHOD_OOB_RBTREE || \
       LEAK_DETECTION == LEAK_DETECTION_METHOD_OOB_LLRBTREE)
DECL_END
#include <hybrid/sequence/rbtree.h>
DECL_BEGIN

#if LEAK_DETECTION == LEAK_DETECTION_METHOD_OOB_LLRBTREE
#define RBTREE_LEFT_LEANING /* Significantly slower */
#endif /* LEAK_DETECTION == LEAK_DETECTION_METHOD_OOB_LLRBTREE */

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
#define RBTREE_Tkey             byte_t *
#define RBTREE_GETNODE(self)    (self)->ln_link.ln_node
#define RBTREE_GETMINKEY(self)  ((byte_t *)(self)->ln_chunk)
#define RBTREE_GETMAXKEY(self)  ((byte_t *)(self)->ln_chunk + chunksize((self)->ln_chunk) - 1)
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
PRIVATE DL_LOCK_T leak_lock = DL_LOCK_INIT_STATIC;
#define leak_lock_tryacquire() DL_LOCK_TRYACQUIRE(&leak_lock)
#define leak_lock_acquire()    DL_LOCK_ACQUIRE(&leak_lock)
#define _leak_lock_release()   DL_LOCK_RELEASE(&leak_lock)
#define leak_lock_release()    DL_LOCK_RELEASE(&leak_lock)
#else /* USE_LOCKS */
#define leak_lock_tryacquire() 1
#define leak_lock_acquire()    (void)0
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
		node = leaknode_tree_remove(&leak_nodes, (byte_t *)ptr);
		if unlikely(node && (void *)ptr != chunk2mem(node->ln_chunk)) {
			/* This can happen if "ptr" is a custom flag4 sub-region of a bigger heap region */
			leaknode_tree_insert(&leak_nodes, node);
			node = NULL;
		}
		if (!node) {
			mchunkptr p = mem2chunk(ptr);
			if (!flag4inuse(p)) {
				/* X-X-X: This check right here has been seen to fail for unexplained reasons in the past
				 *        When it failed, "file" was 0xCCCCCCCC or 0xDEADBEEF (or file=0xDEADBEEF, and
				 *        ptr->lfle_file=0xDEADBEEF, indicating another thread having free'd the pointer
				 *        while our thread was trying to do the same).
				 * It seemed to happen under:
				 * - USE_PER_THREAD_MSTATE=0
				 * - FOOTERS=0/1
				 * when running `make computed-operators`, and then usually during the app finalization
				 *
				 * X-X-X: After further testing, I can no longer re-create this :/
				 *        Combined with the fact that "LEAK_DETECTION" will probably
				 *        need to be re-written anyways (since it's way too slow for
				 *        SMP), I think further research is unnecessary.
				 * -----------------------------------------------------------------------------
				 * Irrelevant: new "LEAK_DETECTION_METHOD_IN_TAIL" impl never exhibited this problem */
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
				                 (byte_t *)chunk2mem(node->ln_chunk) +
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
		byte_t *base = (byte_t *)chunk2mem(self->ln_chunk);
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
 * @param: method: One of `DeeHeap_DumpMemoryLeaks_*'
 * @return: * : The total amount of memory leaked (in bytes) */
#define DeeHeap_DumpMemoryLeaks_DEFINED
PUBLIC size_t DCALL DeeHeap_DumpMemoryLeaks(unsigned int method) {
	size_t result = 0;
	if (method == DeeHeap_DumpMemoryLeaks_ALL ||
	    method == DeeHeap_DumpMemoryLeaks_GC_OR_ALL) {
		leak_lock_acquire_and_reap();
		if (leak_nodes)
			result = dump_leaks_node(leak_nodes);
		leak_lock_release();
	}
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
PUBLIC ATTR_HOT ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *
(DCALL DeeDbg_TryMalloc)(size_t n_bytes, char const *file, int line) {
	void *result = dlmalloc(n_bytes);
	if (result)
		result = leaks_insert(result, file, line);
	return result;
}

#define DeeDbg_TryCalloc_DEFINED
PUBLIC ATTR_HOT ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *
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
		ext_assert(is_inuse(p));
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
	node = leaknode_tree_remove(&leak_nodes, (byte_t *)ptr);
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
			                 (byte_t *)chunk2mem(node->ln_chunk) +
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
			ext_assert(is_inuse(p));
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
	node = leaknode_tree_remove(&leak_nodes, (byte_t *)ptr);
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
			                 (byte_t *)chunk2mem(node->ln_chunk) +
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
PUBLIC ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *
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
		node = leaknode_tree_locate(leak_nodes, (byte_t *)ptr);
		if (!node) {
			_DeeAssert_Failf("Dee_UntrackAlloc(ptr)", file, line,
			                 "Bad pointer %p does not map to any node");
			Dee_BREAKPOINT();
		} else if (ptr != chunk2mem(node->ln_chunk)) {
			_DeeAssert_Failf("Dee_UntrackAlloc(ptr)", file, line,
			                 "Bad pointer %p does not map to start of node at %p-%p",
			                 ptr, chunk2mem(node->ln_chunk),
			                 (byte_t *)chunk2mem(node->ln_chunk) +
			                 (chunksize(node->ln_chunk) - overhead_for(node->ln_chunk) - 1));
			Dee_BREAKPOINT();
		} else {
			node->ln_flags |= LEAKNODE_F_NOLEAK;
		}
		leak_lock_release();
	}
	return ptr;
}

#else /* LEAK_DETECTION == ... */

#if LEAK_DETECTION != LEAK_DETECTION_METHOD_NONE
#error "Unknown 'LEAK_DETECTION' method selection"
#endif /* LEAK_DETECTION != LEAK_DETECTION_METHOD_NONE */


#ifndef Dee_TryMalloc_DEFINED
#define Dee_TryMalloc_DEFINED
PUBLIC ATTR_HOT ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *
(DCALL Dee_TryMalloc)(size_t n_bytes) {
	return dlmalloc(n_bytes);
}
#endif /* !Dee_TryMalloc_DEFINED */

#ifndef Dee_TryCalloc_DEFINED
#define Dee_TryCalloc_DEFINED
PUBLIC ATTR_HOT ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *
(DCALL Dee_TryCalloc)(size_t n_bytes) {
	return dlcalloc(n_bytes);
}
#endif /* !Dee_TryCalloc_DEFINED */

#ifndef Dee_Free_DEFINED
#define Dee_Free_DEFINED
PUBLIC ATTR_HOT void
(DCALL Dee_Free)(void *ptr) {
#if !dlfree_CHECKS_NULL
	if unlikely(!ptr)
		return;
#endif /* !dlfree_CHECKS_NULL */
	dlfree(ptr);
}
#endif /* !Dee_Free_DEFINED */

#ifndef Dee_TryRealloc_DEFINED
#define Dee_TryRealloc_DEFINED
PUBLIC ATTR_HOT WUNUSED void *
(DCALL Dee_TryRealloc)(void *ptr, size_t n_bytes) {
#if !dlfree_CHECKS_NULL
	if unlikely(!ptr)
		return dlmalloc(n_bytes);
#endif /* !dlfree_CHECKS_NULL */
	return dlrealloc(ptr, n_bytes);
}
#endif /* !Dee_TryRealloc_DEFINED */

#ifndef Dee_TryMemalign_DEFINED
#define Dee_TryMemalign_DEFINED
PUBLIC ATTR_HOT ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *
(DCALL Dee_TryMemalign)(size_t min_alignment, size_t n_bytes) {
	return dlmemalign(min_alignment, n_bytes);
}
#endif /* !Dee_TryMemalign_DEFINED */

#endif /* LEAK_DETECTION != ... */
#endif /* !DIRECTLY_DEFINE_DEEMON_PUBLIC_API */


/* Given a heap pointer (as could also be passed to `Dee_Free()' or
 * `Dee_MallocUsableSize()'), check if that pointer belongs to a custom
 * heap region, and if so: return a pointer to said heap region.
 * - If `ptr' is `NULL' or a heap pointer that does not belong
 *   to a custom heap region, `NULL' is returned instead.
 * - If `ptr' isn't a heap pointer, behavior is undefined.
 *
 * @return: * :   The heap region belonging to `ptr'
 * @return: NULL: Given `ptr' is `NULL' or does not belong to a custom heap region */
PUBLIC ATTR_PURE WUNUSED struct Dee_heapregion *DCALL
DeeHeap_GetRegionOf(void *ptr) {
#if FLAG4_BIT_HEAP_REGION
	struct Dee_heapregion *result = NULL;
	mchunkptr p = mem2chunk(ptr);
	if (flag4inuse(p)) {
		/* Traverse the chain of chunks until we find one
		 * with "prev_foot" (aka. "hc_prevsize") == 0 */
		while (p->prev_foot) {
			ext_assert(IS_ALIGNED(p->prev_foot, Dee_HEAPCHUNK_ALIGN));
			p = chunk_minus_offset(p, p->prev_foot);
		}
		result = COMPILER_CONTAINER_OF((struct Dee_heapchunk *)p,
		                               struct Dee_heapregion, hr_first);
		ext_assert(result->hr_size >= (size_t)((byte_t *)mem2chunk(ptr) - (byte_t *)result));
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
	return result;
#else /* FLAG4_BIT_HEAP_REGION */
	(void)ptr;
	return NULL;
#endif /* FLAG4_BIT_HEAP_REGION */
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

	/* Also check slab memory (~ala `slab_chkfree_data()') */
#ifdef HAVE_DeeSlab_CheckMemory
	DeeSlab_CheckMemory();
#endif /* HAVE_DeeSlab_CheckMemory */
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
PUBLIC ATTR_HOT_NDEBUG ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *
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
PUBLIC ATTR_HOT_NDEBUG ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *
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
#if !dlfree_CHECKS_NULL
	if unlikely(!ptr)
		return dlmalloc(n_bytes);
#endif /* !dlfree_CHECKS_NULL */
	return dlrealloc(ptr, n_bytes);
#endif /* !DeeDbg_TryRealloc_DEFINED */
}
#endif /* !Dee_TryRealloc_DEFINED */

#ifndef Dee_TryMemalign_DEFINED
#define Dee_TryMemalign_DEFINED
PUBLIC /*ATTR_HOT_NDEBUG*/ ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *
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
	if (mem != NULL) {
		mchunkptr p = mem2chunk(mem);
#if 1
		ext_assert(is_inuse(p));
		return chunksize(p) - overhead_for(p);
#else
		if (is_inuse(p))
			return chunksize(p) - overhead_for(p);
#endif
	}
	return 0;
}

PUBLIC ATTR_PURE WUNUSED NONNULL((1)) size_t
(DCALL Dee_MallocUsableSizeNonNull)(void *mem) {
	mchunkptr p;
	ASSERT(mem);
	p = mem2chunk(mem);
#if 1
	ext_assert(is_inuse(p));
	return chunksize(p) - overhead_for(p);
#else
	if (is_inuse(p))
		return chunksize(p) - overhead_for(p);
#endif
}


/* Default malloc/free functions used for heap allocation.
 * NOTE: Upon allocation failure, caches are cleared and an `Error.NoMemory' is thrown. */
#ifndef Dee_Malloc_DEFINED
#define Dee_Malloc_DEFINED
PUBLIC ATTR_HOT_NDEBUG ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *
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
PUBLIC ATTR_HOT_NDEBUG ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *
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
PUBLIC /*ATTR_HOT_NDEBUG*/ ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *
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
PUBLIC ATTR_HOT_DEBUG ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *
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
PUBLIC ATTR_HOT_DEBUG ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *
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
PUBLIC /*ATTR_HOT_DEBUG*/ ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *
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
PUBLIC ATTR_HOT_DEBUG ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *
(DCALL DeeDbg_TryMalloc)(size_t n_bytes, char const *file, int line) {
	(void)file;
	(void)line;
	return (Dee_TryMalloc)(n_bytes);
}
#endif /* !DeeDbg_TryMalloc_DEFINED */

#ifndef DeeDbg_TryCalloc_DEFINED
#define DeeDbg_TryCalloc_DEFINED
PUBLIC ATTR_HOT_DEBUG ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *
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
PUBLIC /*ATTR_HOT_DEBUG*/ ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *
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
 * @param: method: One of `DeeHeap_DumpMemoryLeaks_*'
 * @return: * : The total amount of memory leaked (in bytes) */
PUBLIC size_t DCALL DeeHeap_DumpMemoryLeaks(unsigned int method) {
	(void)method;
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
	COMPILER_IMPURE();
	(void)id;
	return 0;
}
#endif /* !DeeHeap_GetAllocBreakpoint_DEFINED */


#ifndef DeeDbgHeap_AddHeapRegion_DEFINED
#define DeeDbgHeap_AddHeapRegion_DEFINED
/* Attach debug info (for the sake of memory leaks as reported by `DeeHeap_DumpMemoryLeaks()',
 * as well as `DeeHeap_DumpMemoryLeaks_GC' being able to recursively scan the payload areas of
 * reachable heap chunks) to a custom "struct Dee_heapregion"
 *
 * These calls are entirely OPTIONAL, but if not called, `DeeHeap_DumpMemoryLeaks()' will not
 * be able to inform you about heap chunks from `region' that are never free'd, or be able to
 * identify `Dee_Malloc()' pointers stored in the payload areas of reachable chunks within the
 * given `region' when those chunks are reachable and called using `DeeHeap_DumpMemoryLeaks_GC'
 *
 * WARNING: `DeeDbgHeap_DelHeapRegion()' is thread-safe, but only in those cases where you can
 *          guaranty that at least 1 of `region's heap-chunks has not yet been freed, and will
 *          not be freed by another thread during the call to this function. (iow: it may only
 *          be called when there is chance that `hr_destroy' has been- or will be called before
 *          the call has a chance to return)
 *
 * @param: file:   A filename that should appear when memory leaks are dumped.
 *                 Note that unlike other debug-heap functions, this string is actually
 *                 strdup()'d, meaning it's allowed to point to a dynamically allocated
 *                 memory location.
 * @param: region: The region to register/unregister debug information for.
 *                 Even when not registered, `Dee_Free()' works as it should!
 *                 These functions are only necessary for `DeeHeap_DumpMemoryLeaks()'!
 * @return: * : Always re-returns "region". These APIs are intentionally designed to never fail
 *              (or rather: to fail-safe), never block (indefinitely), and never return an error.
 *              These API *may* however modify the given `region's `hr_tail.ht_zero' field. */
PUBLIC ATTR_RETNONNULL NONNULL((1)) struct Dee_heapregion *DCALL
DeeDbgHeap_AddHeapRegion(struct Dee_heapregion *__restrict region, char const *file) {
	(void)file;
	return region;
}

PUBLIC ATTR_RETNONNULL NONNULL((1)) struct Dee_heapregion *DCALL
DeeDbgHeap_DelHeapRegion(struct Dee_heapregion *__restrict region) {
	return region;
}
#endif /* !DeeDbgHeap_AddHeapRegion_DEFINED */



#ifndef CONFIG_NO_THREADS
INTERN NONNULL((1)) void DCALL
thread_heap_destroy(void *heap) {
#if USE_PER_THREAD_MSTATE
	tls_mspace_destroy((mspace)heap);
#endif /* USE_PER_THREAD_MSTATE */
	COMPILER_IMPURE();
	(void)heap;
}
#endif /* !CONFIG_NO_THREADS */

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_HEAP_C */
