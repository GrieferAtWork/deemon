/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_GC_H
#define GUARD_DEEMON_GC_H 1

#include "api.h"
/**/

#include "types.h"

#ifndef __INTELLISENSE__
#include "alloc.h"
#else /* !__INTELLISENSE__ */
DECL_BEGIN
#define _Dee_MalloccBufsize(elem_count, elem_size)                              ((elem_count) * (elem_size))
#define _Dee_MallococBufsize(base_offset, elem_count, elem_size)                ((base_offset) + ((elem_count) * (elem_size)))
#define _Dee_MalloccBufsizeDbg(elem_count, elem_size, file, line)               _Dee_MalloccBufsize(elem_count, elem_size)
#define _Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line) _Dee_MallococBufsize(base_offset, elem_count, elem_size)
LOCAL ATTR_CONST WUNUSED size_t DCALL _Dee_MalloccBufsizeSafe(size_t elem_count, size_t elem_size);
LOCAL ATTR_CONST WUNUSED size_t DCALL _Dee_MallococBufsizeSafe(size_t base_offset, size_t elem_count, size_t elem_size);
DECL_END
#endif /* __INTELLISENSE__ */
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintptr_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_gc_head_link gc_head_link
#define Dee_gc_head      gc_head
#endif /* DEE_SOURCE */

#if defined(DEE_SOURCE) && defined(__INTELLISENSE__)
struct gc_head;
struct gc_head_link {
	/* The structure that is prefixed before every GC-allocated object. */
	struct gc_head  *gc_next;   /* [0..1][lock(INTERNAL(gc_lock))] Next GC object. */
	struct gc_head **gc_pself;  /* [1..1][== self][1..1][lock(INTERNAL(gc_lock))] Self-pointer in the global chain of GC objects. */
};
struct gc_head {
	struct gc_head  *gc_next;   /* [0..1][lock(INTERNAL(gc_lock))] Next GC object. */
	struct gc_head **gc_pself;  /* [1..1][== self][1..1][lock(INTERNAL(gc_lock))] Self-pointer in the global chain of GC objects. */
	DeeObject        gc_object; /* The object that is being controlled by the GC. */
};
#else /* DEE_SOURCE && __INTELLISENSE__ */
struct Dee_gc_head;
struct Dee_gc_head_link {
	/* The structure that is prefixed before every GC-allocated object. */
	struct Dee_gc_head  *gc_next;   /* [0..1][lock(INTERNAL(gc_lock))] Next GC object. */
	struct Dee_gc_head **gc_pself;  /* [1..1][== self][1..1][lock(INTERNAL(gc_lock))] Self-pointer in the global chain of GC objects. */
};
struct Dee_gc_head {
	struct Dee_gc_head  *gc_next;   /* [0..1][lock(INTERNAL(gc_lock))] Next GC object. */
	struct Dee_gc_head **gc_pself;  /* [1..1][== self][1..1][lock(INTERNAL(gc_lock))] Self-pointer in the global chain of GC objects. */
	DeeObject            gc_object; /* The object that is being controlled by the GC. */
};
#endif /* !DEE_SOURCE || !__INTELLISENSE__ */

#ifndef NDEBUG
#if __SIZEOF_POINTER__ == 4
#define _Dee_GC_HEAD_UNTRACKED_MARKER ((struct gc_head **)UINT32_C(0xcccccccc))
#elif __SIZEOF_POINTER__ == 8
#define _Dee_GC_HEAD_UNTRACKED_MARKER ((struct gc_head **)UINT64_C(0xcccccccccccccccc))
#endif /* __SIZEOF_POINTER__ == ... */
#endif /* !NDEBUG */

#ifdef _Dee_GC_HEAD_UNTRACKED_MARKER
#define _Dee_GC_HEAD_UNTRACKED_INIT NULL, _Dee_GC_HEAD_UNTRACKED_MARKER
#else /* _Dee_GC_HEAD_UNTRACKED_MARKER */
#define _Dee_GC_HEAD_UNTRACKED_INIT NULL, NULL
#endif /* !_Dee_GC_HEAD_UNTRACKED_MARKER */



#define DEE_GC_OBJECT_OFFSET COMPILER_OFFSETOF(struct Dee_gc_head, gc_object)
#define DEE_GC_HEAD_SIZE     COMPILER_OFFSETOF(struct Dee_gc_head, gc_object)
#define DeeGC_Head(ob)       ((struct Dee_gc_head *)((uintptr_t)Dee_REQUIRES_ANYOBJECT(ob) - DEE_GC_OBJECT_OFFSET))
#define DeeGC_Object(ob)     (&(ob)->gc_object)
#define DeeGC_Check(ob)      ((Dee_TYPE(ob)->tp_flags & TP_FGC) && (!DeeType_Check(ob) || (Dee_REQUIRES_OBJECT(DeeTypeObject, ob)->tp_flags & TP_FHEAP)))

/* Begin/end tracking a given GC-allocated object.
 * `DeeGC_Track()' must be called explicitly when the object
 * has been allocated using `DeeGCObject_Malloc' and friends,
 * though constructions of non-variadic GC objects don't need
 * to call this function on the passed object. - That call will
 * automatically be done when the function returns successfully.
 * @return: * : == ob */
DFUNDEF ATTR_RETNONNULL NONNULL((1)) DeeObject *DCALL DeeGC_Track(DeeObject *__restrict ob);
DFUNDEF ATTR_RETNONNULL NONNULL((1)) DeeObject *DCALL DeeGC_Untrack(DeeObject *__restrict ob);

/* Track all GC objects in range [first,last], all of which have
 * already been linked together using their `struct gc_head_link' */
DFUNDEF NONNULL((1, 2)) void DCALL
DeeGC_TrackAll(DeeObject *first, DeeObject *last);

#define DeeGC_TRACK(T, ob) ((DREF T *)DeeGC_Track(Dee_REQUIRES_OBJECT(DREF DeeObject, ob)))

/* Try to collect at most `max_objects' GC-objects,
 * returning the actual amount collected. */
DFUNDEF size_t DCALL DeeGC_Collect(size_t max_objects);

#ifdef CONFIG_BUILDING_DEEMON
/* Return `true' if any GC objects with a non-zero reference
 * counter is being tracked.
 * NOTE: In addition, this function does not return `true' when
 *       all that's left are dex objects (which are destroyed
 *       at a later point during deemon shutdown, than the point
 *       when this function is called to determine if the GC must
 *       continue to run) */
INTDEF bool DCALL DeeGC_IsEmptyWithoutDex(void);
#endif /* CONFIG_BUILDING_DEEMON */

/* GC object alloc/free.
 * Don't you think these functions allocate some magical memory
 * that can somehow track what objects it references. - No!
 * All these do is allocate a block of memory of `n_bytes' that
 * includes some storage at negative offsets to hold a `struct gc_head',
 * as is required for objects that should later be tracked by the GC. */
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_Malloc)(size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_Calloc)(size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL DeeGCObject_Realloc)(void *p, size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_TryMalloc)(size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_TryCalloc)(size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL DeeGCObject_TryRealloc)(void *p, size_t n_bytes);
DFUNDEF void (DCALL DeeGCObject_Free)(void *p);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgGCObject_Malloc)(size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgGCObject_Calloc)(size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED void *(DCALL DeeDbgGCObject_Realloc)(void *p, size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgGCObject_TryMalloc)(size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgGCObject_TryCalloc)(size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED void *(DCALL DeeDbgGCObject_TryRealloc)(void *p, size_t n_bytes, char const *file, int line);
DFUNDEF void (DCALL DeeDbgGCObject_Free)(void *p, char const *file, int line);

#define DeeGCObject_Mallocc(base_offset, elem_count, elem_size)                           DeeGCObject_Malloc(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define DeeGCObject_Callocc(base_offset, elem_count, elem_size)                           DeeGCObject_Calloc(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define DeeGCObject_Reallocc(p, base_offset, elem_count, elem_size)                       DeeGCObject_Realloc(p, _Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define DeeGCObject_TryMallocc(base_offset, elem_count, elem_size)                        DeeGCObject_TryMalloc(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define DeeGCObject_TryCallocc(base_offset, elem_count, elem_size)                        DeeGCObject_TryCalloc(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define DeeGCObject_TryReallocc(p, base_offset, elem_count, elem_size)                    DeeGCObject_TryRealloc(p, _Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define DeeDbgGCObject_Mallocc(base_offset, elem_count, elem_size, file, line)            DeeDbgGCObject_Malloc(_Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbgGCObject_Callocc(base_offset, elem_count, elem_size, file, line)            DeeDbgGCObject_Calloc(_Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbgGCObject_Reallocc(p, base_offset, elem_count, elem_size, file, line)        DeeDbgGCObject_Realloc(p, _Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbgGCObject_TryMallocc(base_offset, elem_count, elem_size, file, line)         DeeDbgGCObject_TryMalloc(_Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbgGCObject_TryCallocc(base_offset, elem_count, elem_size, file, line)         DeeDbgGCObject_TryCalloc(_Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbgGCObject_TryReallocc(p, base_offset, elem_count, elem_size, file, line)     DeeDbgGCObject_TryRealloc(p, _Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeGCObject_MalloccSafe(base_offset, elem_count, elem_size)                       DeeGCObject_Malloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define DeeGCObject_CalloccSafe(base_offset, elem_count, elem_size)                       DeeGCObject_Calloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define DeeGCObject_RealloccSafe(p, base_offset, elem_count, elem_size)                   DeeGCObject_Realloc(p, _Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define DeeGCObject_TryMalloccSafe(base_offset, elem_count, elem_size)                    DeeGCObject_TryMalloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define DeeGCObject_TryCalloccSafe(base_offset, elem_count, elem_size)                    DeeGCObject_TryCalloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define DeeGCObject_TryRealloccSafe(p, base_offset, elem_count, elem_size)                DeeGCObject_TryRealloc(p, _Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define DeeDbgGCObject_MalloccSafe(base_offset, elem_count, elem_size, file, line)        DeeDbgGCObject_Malloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbgGCObject_CalloccSafe(base_offset, elem_count, elem_size, file, line)        DeeDbgGCObject_Calloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbgGCObject_RealloccSafe(p, base_offset, elem_count, elem_size, file, line)    DeeDbgGCObject_Realloc(p, _Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbgGCObject_TryMalloccSafe(base_offset, elem_count, elem_size, file, line)     DeeDbgGCObject_TryMalloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbgGCObject_TryCalloccSafe(base_offset, elem_count, elem_size, file, line)     DeeDbgGCObject_TryCalloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbgGCObject_TryRealloccSafe(p, base_offset, elem_count, elem_size, file, line) DeeDbgGCObject_TryRealloc(p, _Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)

#ifndef NDEBUG
#undef DeeGCObject_Mallocc
#undef DeeGCObject_Callocc
#undef DeeGCObject_Reallocc
#undef DeeGCObject_TryMallocc
#undef DeeGCObject_TryCallocc
#undef DeeGCObject_TryReallocc
#undef DeeGCObject_MalloccSafe
#undef DeeGCObject_CalloccSafe
#undef DeeGCObject_RealloccSafe
#undef DeeGCObject_TryMalloccSafe
#undef DeeGCObject_TryCalloccSafe
#undef DeeGCObject_TryRealloccSafe
#define DeeGCObject_Malloc(n_bytes)                                          DeeDbgGCObject_Malloc(n_bytes, __FILE__, __LINE__)
#define DeeGCObject_Calloc(n_bytes)                                          DeeDbgGCObject_Calloc(n_bytes, __FILE__, __LINE__)
#define DeeGCObject_Realloc(ptr, n_bytes)                                    DeeDbgGCObject_Realloc(ptr, n_bytes, __FILE__, __LINE__)
#define DeeGCObject_TryMalloc(n_bytes)                                       DeeDbgGCObject_TryMalloc(n_bytes, __FILE__, __LINE__)
#define DeeGCObject_TryCalloc(n_bytes)                                       DeeDbgGCObject_TryCalloc(n_bytes, __FILE__, __LINE__)
#define DeeGCObject_TryRealloc(ptr, n_bytes)                                 DeeDbgGCObject_TryRealloc(ptr, n_bytes, __FILE__, __LINE__)
#define DeeGCObject_Free(ptr)                                                DeeDbgGCObject_Free(ptr, __FILE__, __LINE__)
#define DeeGCObject_Mallocc(base_offset, elem_count, elem_size)              DeeDbgGCObject_Mallocc(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_Callocc(base_offset, elem_count, elem_size)              DeeDbgGCObject_Callocc(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_Reallocc(ptr, base_offset, elem_count, elem_size)        DeeDbgGCObject_Reallocc(ptr, base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_TryMallocc(base_offset, elem_count, elem_size)           DeeDbgGCObject_TryMallocc(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_TryCallocc(base_offset, elem_count, elem_size)           DeeDbgGCObject_TryCallocc(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_TryReallocc(ptr, base_offset, elem_count, elem_size)     DeeDbgGCObject_TryReallocc(ptr, base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_MalloccSafe(base_offset, elem_count, elem_size)          DeeDbgGCObject_MalloccSafe(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_CalloccSafe(base_offset, elem_count, elem_size)          DeeDbgGCObject_CalloccSafe(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_RealloccSafe(ptr, base_offset, elem_count, elem_size)    DeeDbgGCObject_RealloccSafe(ptr, base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_TryMalloccSafe(base_offset, elem_count, elem_size)       DeeDbgGCObject_TryMalloccSafe(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_TryCalloccSafe(base_offset, elem_count, elem_size)       DeeDbgGCObject_TryCalloccSafe(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_TryRealloccSafe(ptr, base_offset, elem_count, elem_size) DeeDbgGCObject_TryRealloccSafe(ptr, base_offset, elem_count, elem_size, __FILE__, __LINE__)
#else /* !NDEBUG */
#undef DeeDbgGCObject_Mallocc
#undef DeeDbgGCObject_Callocc
#undef DeeDbgGCObject_Reallocc
#undef DeeDbgGCObject_TryMallocc
#undef DeeDbgGCObject_TryCallocc
#undef DeeDbgGCObject_TryReallocc
#undef DeeDbgGCObject_MalloccSafe
#undef DeeDbgGCObject_CalloccSafe
#undef DeeDbgGCObject_RealloccSafe
#undef DeeDbgGCObject_TryMalloccSafe
#undef DeeDbgGCObject_TryCalloccSafe
#undef DeeDbgGCObject_TryRealloccSafe
#define DeeDbgGCObject_Malloc(n_bytes, file, line)                                          DeeGCObject_Malloc(n_bytes)
#define DeeDbgGCObject_Calloc(n_bytes, file, line)                                          DeeGCObject_Calloc(n_bytes)
#define DeeDbgGCObject_Realloc(ptr, n_bytes, file, line)                                    DeeGCObject_Realloc(ptr, n_bytes)
#define DeeDbgGCObject_TryMalloc(n_bytes, file, line)                                       DeeGCObject_TryMalloc(n_bytes)
#define DeeDbgGCObject_TryCalloc(n_bytes, file, line)                                       DeeGCObject_TryCalloc(n_bytes)
#define DeeDbgGCObject_TryRealloc(ptr, n_bytes, file, line)                                 DeeGCObject_TryRealloc(ptr, n_bytes)
#define DeeDbgGCObject_Free(ptr, file, line)                                                DeeGCObject_Free(ptr)
#define DeeDbgGCObject_Mallocc(base_offset, elem_count, elem_size, file, line)              DeeGCObject_Mallocc(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_Callocc(base_offset, elem_count, elem_size, file, line)              DeeGCObject_Callocc(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_Reallocc(ptr, base_offset, elem_count, elem_size, file, line)        DeeGCObject_Reallocc(ptr, base_offset, elem_count, elem_size)
#define DeeDbgGCObject_TryMallocc(base_offset, elem_count, elem_size, file, line)           DeeGCObject_TryMallocc(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_TryCallocc(base_offset, elem_count, elem_size, file, line)           DeeGCObject_TryCallocc(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_TryReallocc(ptr, base_offset, elem_count, elem_size, file, line)     DeeGCObject_TryReallocc(ptr, base_offset, elem_count, elem_size)
#define DeeDbgGCObject_MalloccSafe(base_offset, elem_count, elem_size, file, line)          DeeGCObject_MalloccSafe(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_CalloccSafe(base_offset, elem_count, elem_size, file, line)          DeeGCObject_CalloccSafe(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_RealloccSafe(ptr, base_offset, elem_count, elem_size, file, line)    DeeGCObject_RealloccSafe(ptr, base_offset, elem_count, elem_size)
#define DeeDbgGCObject_TryMalloccSafe(base_offset, elem_count, elem_size, file, line)       DeeGCObject_TryMalloccSafe(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_TryCalloccSafe(base_offset, elem_count, elem_size, file, line)       DeeGCObject_TryCalloccSafe(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_TryRealloccSafe(ptr, base_offset, elem_count, elem_size, file, line) DeeGCObject_TryRealloccSafe(ptr, base_offset, elem_count, elem_size)
#endif /* NDEBUG */


/* Allocate fixed-size, gc-object-purposed slab memory.
 * NOTE: This memory must be freed by one of:
 *   - DeeGCObject_FFree(return, size2)    | size2 <= size
 *   - DeeGCObject_SlabFree<size2>(return) | size2 <= size
 *   - DeeGCObject_Free(return) */
#ifdef CONFIG_NO_OBJECT_SLABS
#define DeeGCObject_FMalloc(size)    DeeGCObject_Malloc(size)
#define DeeGCObject_FCalloc(size)    DeeGCObject_Calloc(size)
#define DeeGCObject_FTryMalloc(size) DeeGCObject_TryMalloc(size)
#define DeeGCObject_FTryCalloc(size) DeeGCObject_TryCalloc(size)
#define DeeGCObject_FFree(ptr, size) DeeGCObject_Free(ptr)
#elif defined(__INTELLISENSE__)
#define DeeGCObject_FMalloc(size)    DeeGCObject_Malloc(size)
#define DeeGCObject_FCalloc(size)    DeeGCObject_Calloc(size)
#define DeeGCObject_FTryMalloc(size) DeeGCObject_TryMalloc(size)
#define DeeGCObject_FTryCalloc(size) DeeGCObject_TryCalloc(size)
#define DeeGCObject_FFree(ptr, size) (DeeGCObject_Free(ptr), (void)(size))
#else /* CONFIG_NO_OBJECT_SLABS */
#define DEE_PRIVATE_DEFINE_SLAB_FUNCTIONS(index, size)                               \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_SlabMalloc##size)(void);    \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_SlabCalloc##size)(void);    \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_SlabTryMalloc##size)(void); \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_SlabTryCalloc##size)(void); \
	DFUNDEF void (DCALL DeeGCObject_SlabFree##size)(void *__restrict ptr);
DeeSlab_ENUMERATE(DEE_PRIVATE_DEFINE_SLAB_FUNCTIONS)
#undef DEE_PRIVATE_DEFINE_SLAB_FUNCTIONS
#define DeeGCObject_FMalloc(size)    DeeSlab_Invoke(DeeGCObject_SlabMalloc, size, (), DeeGCObject_Malloc(size))
#define DeeGCObject_FCalloc(size)    DeeSlab_Invoke(DeeGCObject_SlabCalloc, size, (), DeeGCObject_Calloc(size))
#define DeeGCObject_FTryMalloc(size) DeeSlab_Invoke(DeeGCObject_SlabTryMalloc, size, (), DeeGCObject_TryMalloc(size))
#define DeeGCObject_FTryCalloc(size) DeeSlab_Invoke(DeeGCObject_SlabTryCalloc, size, (), DeeGCObject_TryCalloc(size))
#define DeeGCObject_FFree(ptr, size) DeeSlab_Invoke(DeeGCObject_SlabFree, size, (ptr), DeeGCObject_Free(ptr))
#endif /* !CONFIG_NO_OBJECT_SLABS */

/* Same as the regular malloc functions, but use the same allocation methods that
 * would be used by `TYPE_FIXED_ALLOCATOR_GC' and `TYPE_FIXED_ALLOCATOR_GC_S', meaning
 * that pointers returned by these macros have binary compatibility with them. */
#define DeeGCObject_MALLOC(T)       ((T *)DeeGCObject_FMalloc(sizeof(T)))
#define DeeGCObject_CALLOC(T)       ((T *)DeeGCObject_FCalloc(sizeof(T)))
#define DeeGCObject_TRYMALLOC(T)    ((T *)DeeGCObject_FTryMalloc(sizeof(T)))
#define DeeGCObject_TRYCALLOC(T)    ((T *)DeeGCObject_FTryCalloc(sizeof(T)))
#define DeeGCObject_FREE(typed_ptr)       DeeGCObject_FFree(typed_ptr, sizeof(*(typed_ptr)))

#ifdef CONFIG_FIXED_ALLOCATOR_S_IS_AUTO
#define DeeGCObject_MALLOC_S(T)    ((T *)DeeGCObject_Malloc(sizeof(T)))
#define DeeGCObject_CALLOC_S(T)    ((T *)DeeGCObject_Calloc(sizeof(T)))
#define DeeGCObject_TRYMALLOC_S(T) ((T *)DeeGCObject_TryMalloc(sizeof(T)))
#define DeeGCObject_TRYCALLOC_S(T) ((T *)DeeGCObject_TryCalloc(sizeof(T)))
#define DeeGCObject_FREE_S               DeeGCObject_Free
#else /* CONFIG_FIXED_ALLOCATOR_S_IS_AUTO */
#define DeeGCObject_MALLOC_S      DeeGCObject_MALLOC
#define DeeGCObject_CALLOC_S      DeeGCObject_CALLOC
#define DeeGCObject_TRYMALLOC_S   DeeGCObject_TRYMALLOC
#define DeeGCObject_TRYCALLOC_S   DeeGCObject_TRYCALLOC
#define DeeGCObject_FREE_S        DeeGCObject_FREE
#endif /* !CONFIG_FIXED_ALLOCATOR_S_IS_AUTO */

/* An generic sequence singleton that can be
 * iterated to yield all tracked GC objects.
 * This object also offers a hand full of member functions
 * that user-space an invoke to trigger various GC-related
 * functionality:
 *   - collect(max: int = -1): int;
 * Also: remember that this derives from `Sequence', so you
 *       can use all its attributes, like `empty', etc.
 * NOTE: This object is exported as `gc from deemon' */
DDATDEF DeeObject DeeGCEnumTracked_Singleton;


DECL_END

#endif /* !GUARD_DEEMON_GC_H */
