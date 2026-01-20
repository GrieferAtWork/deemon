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
#ifndef GUARD_DEEMON_HEAP_H
#define GUARD_DEEMON_HEAP_H 1

#include "api.h"

#ifdef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
#include <stddef.h> /* size_t */

DECL_BEGIN

/* Disclaimer:
 * The internal data-layout seen here is heavily inspired by dlmalloc (Doug Lea's malloc)
 * dlmalloc is public domain, but I feel like it's just a matter of respect to state this
 * inspiration, and give credit where credit is due ;)
 *
 * ==============================================================================
 *
 * Deemon heap regions define a binary format that can be used to read/write file mappings
 * in such a way that specific pointers into those mappings can be passed to Dee_Free(),
 * Dee_Realloc() and Dee_MallocUsableSize(), with the deemon heap system then managing
 * which chunks of your file mapping are still allocated, and once *all* chunks have been
 * free'd, a custom callback within the associated "struct Dee_heapregion" is invoked.
 *
 * ==============================================================================
 * === EXAMPLE
 * ==============================================================================
 *
 * The following is an example of how the binary data of a deemon heap region may
 * look like. Note that the existence of data formatted like this is enough for you
 * to be able to pass pointers to the "User data" areas of chunks to Dee_Free(). --
 * There is no need to "register" heap regions before they can be used in APIs.
 *
 * Assuming: sizeof(size_t) == sizeof(void *) == 4 && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
 *
 *    Offset Hex Data      Field                                         Descriptor / Comment
 *    ----   -----------   -------------------------------------------   --------------------
 *    0000   38 10 00 00   struct Dee_heapregion::hr_size                size = 312 (0x138)
 *    0004   87 65 43 21   struct Dee_heapregion::hr_destroy             (void (DCALL *)(struct Dee_heapregion *))0x12345678
 *
 *    0008   00 00 00 00   struct Dee_heapregion::hr_first.hc_prevsize   Must always be "0" for first chunk
 *    000C   0C 01 00 00   struct Dee_heapregion::hr_first.hc_head       Dee_HEAPCHUNK_HEAD(256)
 *    0010   ?? ?? ?? ??                                                 User data
 *    ...
 *    0108   ?? ?? ?? ??                                                 User data
 *
 *    0110   08 01 00 00   struct Dee_heapchunk::hc_prevsize             Dee_HEAPCHUNK_PREV(256)
 *    0114   14 00 00 00   struct Dee_heapchunk::hc_head                 Dee_HEAPCHUNK_HEAD(8)
 *    0118   ?? ?? ?? ??                                                 User data
 *    011C   ?? ?? ?? ??                                                 User data
 *
 *    0120   10 00 00 00   struct Dee_heapchunk::hc_prevsize             Dee_HEAPCHUNK_PREV(8)
 *    0124   14 00 00 00   struct Dee_heapchunk::hc_head                 Dee_HEAPCHUNK_HEAD(8)
 *    0128   ?? ?? ?? ??                                                 User data
 *    012C   ?? ?? ?? ??                                                 User data
 *
 *    0130   10 00 00 00   struct Dee_heaptail::ht_prevsize              Dee_HEAPCHUNK_PREV(8)
 *    0134   00 00 00 00   struct Dee_heaptail::ht_zero                  Must always be "0" in last chunk
 *
 * The above region features 3 addresses for heap chunks with associated user-data.
 * Once all 3 chunks are free'd (order of Dee_Free() operations doesn't matter),
 * the destructor callback at "0x12345678" will be invoked during the last free.
 * >> Dee_MallocUsableSize(REGION_BASE + 0x10);  // 256
 * >> Dee_MallocUsableSize(REGION_BASE + 0x118); // 16
 * >> Dee_MallocUsableSize(REGION_BASE + 0x128); // 16
 * >> Dee_Free(REGION_BASE + 0x10);
 * >> Dee_Free(REGION_BASE + 0x118);
 * >> Dee_Free(REGION_BASE + 0x128);             // This (last) free will invoke destructor at "0x12345678"
 *
 *
 * Of additional note should be:
 * - When read-from, or written-to a file, this format requires only a single
 *   relocation per region (specifically: for "hr_destroy"). No additional
 *   relocations are required for individual heap chunks.
 * - Trying to call `Dee_Realloc()' on a heap pointer from a custom heap region
 *   will always see the call emulated as `Dee_Malloc()+memcpy()+Dee_Free()',
 *   meaning a new chunk is allocated on the regular heap, and the old custom-
 *   region chunk is free'd.
 *
 */


/* Special flag values needed for chunks in different positions */
#define Dee_HEAPCHUNK_ALIGN         (2 * __SIZEOF_POINTER__)
#define Dee_HEAPCHUNK_PREV(n_bytes) (((n_bytes) + (2 * __SIZEOF_POINTER__)))
#define Dee_HEAPCHUNK_HEAD(n_bytes) (((n_bytes) + (2 * __SIZEOF_POINTER__)) | 4)


#ifdef __CC__
struct /*ATTR_ALIGNED(Dee_HEAPCHUNK_ALIGN)*/ Dee_heapchunk {
	size_t hc_prevsize; /* Size of previous chunk, or "0" if this is the first chunk; must be multiple of "Dee_HEAPCHUNK_ALIGN" */
	size_t hc_head;     /* Size of this chunk (including this header), but least significant
	                     * 3 bits have special meaning and (when the chunk is allocated, which
	                     * is the only mode explicitly defined by this binary format), must be
	                     * initialized to 0b100 (aka. "4") */
};

struct /*ATTR_ALIGNED(Dee_HEAPCHUNK_ALIGN)*/ Dee_heaptail {
	size_t ht_lastsize; /* Size of the last *real* chunk within the region (in bytes); must be multiple of "Dee_HEAPCHUNK_ALIGN" */
	size_t ht_zero;     /* Always zero (but see `DeeDbgHeap_AddHeapRegion()') */
};

struct /*ATTR_ALIGNED(Dee_HEAPCHUNK_ALIGN)*/ Dee_heapregion {
	size_t               hr_size;  /* [const][== offsetafter(Dee_heapregion, hr_tail)]
	                                * Total region size (in bytes, including this header, and the tail) */
	/* [1..1][const] Destructor invoked once the region's last chunk is Dee_Free()'d */
	void (DCALL         *hr_destroy)(struct Dee_heapregion *__restrict self);
	struct Dee_heapchunk hr_first;                                /* First chunk of region */
//	byte_t               hr_data[hr_size - (6 * sizeof(void *))]; /* Region payload (containing more chunks...) */
//	struct Dee_heaptail  hr_tail;                                 /* Region tail (see above) */
};

#define Dee_heapregion_gettail(self) \
	((struct Dee_heaptail *)((__BYTE_TYPE__ *)(self) + (self)->hr_size - sizeof(struct Dee_heaptail)))
#define Dee_heapregion_gethead(self)  (&(self)->hr_first)
#define Dee_heapregion_getstart(self) Dee_heapregion_gethead(self)
#define Dee_heapregion_getend(self)   ((struct Dee_heapchunk *)Dee_heapregion_gettail(self))
#define Dee_heapchunk_getnext(self)   ((struct Dee_heapchunk *)((__BYTE_TYPE__ *)(self) + ((self)->hc_head & ~7)))

#define Dee_heapregion_getdata(self)     ((__BYTE_TYPE__ *)(&(self)->hr_first + 1))
#define Dee_heapregion_getdatasize(self) ((self)->hr_size - (sizeof(struct Dee_heapregion) + sizeof(struct Dee_heaptail)))

/* ============================================================================== */




/************************************************************************/
/* DEEMON HEAP API                                                      */
/************************************************************************/

/* Validate heap memory, asserting the absence of corruptions from
 * various common heap mistakes (write-past-end, use-after-free, etc.).
 *
 * When deemon was not built for debugging, this is a no-op. */
DFUNDEF void DCALL DeeHeap_CheckMemory(void);

/* Dump info about all heap allocations that were allocated, but
 * never Dee_Free()'d, nor untracked using `Dee_UntrackAlloc()'.
 * Information about leaks is printed using `Dee_DPRINTF()'.
 *
 * @param: method: How One of `DeeHeap_DumpMemoryLeaks_*'
 * @return: * : The total amount of memory leaked (in bytes) */
DFUNDEF size_t DCALL DeeHeap_DumpMemoryLeaks(unsigned int method);
#define DeeHeap_DumpMemoryLeaks_ALL       0 /* Consider all allocated memory (except Dee_UntrackAlloc) as leaks */
#define DeeHeap_DumpMemoryLeaks_GC        1 /* Try to do some arch-/os- specific GC-style checking to exclude chunks that
                                             * *appear* referenced by host "static", "stack", or "registers" storage locations
                                             * May not be supported by all hosts. If unsupported, this method is a no-op. */
#define DeeHeap_DumpMemoryLeaks_GC_OR_ALL 2 /* `DeeHeap_DumpMemoryLeaks_GC' (if supported), else `DeeHeap_DumpMemoryLeaks_ALL' */

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
DFUNDEF ATTR_PURE WUNUSED size_t DCALL DeeHeap_GetAllocBreakpoint(void);
DFUNDEF size_t DCALL DeeHeap_SetAllocBreakpoint(size_t id);

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
DFUNDEF ATTR_PURE WUNUSED struct Dee_heapregion *DCALL
DeeHeap_GetRegionOf(void *ptr);



struct Dee_heap_mallinfo {
	size_t hmi_arena;    /* non-mmapped space allocated from system */
	size_t hmi_ordblks;  /* number of free chunks */
	size_t hmi_hblkhd;   /* space in mmapped regions */
	size_t hmi_usmblks;  /* maximum total allocated space */
	size_t hmi_uordblks; /* total allocated space */
	size_t hmi_fordblks; /* total free space */
	size_t hmi_keepcost; /* releasable (via malloc_trim) space */
};

/* Expose information returned by dlmalloc's internal `dlmallinfo()' */
DFUNDEF ATTR_PURE WUNUSED struct Dee_heap_mallinfo DCALL DeeHeap_MallInfo(void);

/* Expose controls for dlmalloc's internal "footprint" mechanism. */
DFUNDEF ATTR_PURE WUNUSED size_t DCALL DeeHeap_Footprint(void);
DFUNDEF ATTR_PURE WUNUSED size_t DCALL DeeHeap_MaxFootprint(void);
DFUNDEF ATTR_PURE WUNUSED size_t DCALL DeeHeap_GetFootprintLimit(void);
/* @param: bytes: [== 0]        Use minimal size
 * @param: bytes: [== SIZE_MAX] Disable limit
 * @param: bytes: [== *]        Limit (in bytes)
 * @return: * : Old footprint limit */
DFUNDEF size_t DCALL DeeHeap_SetFootprintLimit(size_t bytes);


/* Possible values for `DeeHeap_SetOpt()' */
#define Dee_HEAP_M_TRIM_THRESHOLD (-1)
#define Dee_HEAP_M_GRANULARITY    (-2)
#define Dee_HEAP_M_MMAP_THRESHOLD (-3)

/* Set heap configuration `option' to `value'
 * @return: 0 : Failure (bad `option' or `value')
 * @return: 1 : Success */
DFUNDEF int DCALL DeeHeap_SetOpt(int option, size_t value);

/* Trim pre-allocated heap buffers, but keep at least `pad' bytes allocated (if already allocated)
 * @return: * : The # of bytes released back to the system. */
DFUNDEF size_t DCALL DeeHeap_Trim(size_t pad);

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
DFUNDEF ATTR_RETNONNULL NONNULL((1)) struct Dee_heapregion *DCALL
DeeDbgHeap_AddHeapRegion(struct Dee_heapregion *__restrict region, char const *file);
DFUNDEF ATTR_RETNONNULL NONNULL((1)) struct Dee_heapregion *DCALL
DeeDbgHeap_DelHeapRegion(struct Dee_heapregion *__restrict region);

#endif /* __CC__ */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */

#endif /* !GUARD_DEEMON_HEAP_H */
