/* Copyright (c) 2018 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_GC_H
#define GUARD_DEEMON_GC_H 1

#include "api.h"
#include "object.h"
#include <stddef.h>
#include <stdint.h>

DECL_BEGIN

struct gc_head_raw {
    /* The structure that is prefixed before every GC-allocated object. */
    struct gc_head **gc_pself;  /* [1..1][== self][1..1][lock(INTERNAL(gc_lock))] Self-pointer in the global chain of GC objects. */
    struct gc_head  *gc_next;   /* [0..1][lock(INTERNAL(gc_lock))] Next GC object. */
};
struct gc_head {
    struct gc_head **gc_pself;  /* [1..1][== self][1..1][lock(INTERNAL(gc_lock))] Self-pointer in the global chain of GC objects. */
    struct gc_head  *gc_next;   /* [0..1][lock(INTERNAL(gc_lock))] Next GC object. */
    DeeObject        gc_object; /* The object that is being controlled by the GC. */
};
#define GC_OBJECT_OFFSET  COMPILER_OFFSETOF(struct gc_head,gc_object)
#define GC_HEAD_SIZE      COMPILER_OFFSETOF(struct gc_head,gc_object)
#define DeeGC_Head(ob)   ((struct gc_head *)((uintptr_t)REQUIRES_OBJECT(ob)-GC_OBJECT_OFFSET))
#define DeeGC_Object(ob) (&(ob)->gc_object)
#define DeeGC_Check(ob)  (Dee_TYPE(ob)->tp_flags&TP_FGC && (!DeeType_Check(ob) || (((DeeTypeObject *)REQUIRES_OBJECT(ob))->tp_flags&TP_FHEAP)))

/* Begin/end tracking a given GC-allocated object.
 * `DeeGC_Track()' must be called explicitly when the object
 * has been allocated using `DeeGCObject_Malloc' and friends,
 * though constructions of non-variadic GC objects don't need
 * to call this function on the passed object. - That call will
 * automatically be done when the function returns successfully.
 * @return: * : == ob */
DFUNDEF ATTR_RETNONNULL DeeObject *DCALL DeeGC_Track(DeeObject *__restrict ob);
DFUNDEF ATTR_RETNONNULL DeeObject *DCALL DeeGC_Untrack(DeeObject *__restrict ob);

/* Try to collect at most `max_objects' GC-objects,
 * returning the actual amount collected. */
DFUNDEF size_t DCALL DeeGC_Collect(size_t max_objects);

#ifdef CONFIG_BUILDING_DEEMON
/* Return `true' if there any GC objects with a
 * non-zero reference counter are being tracked.
 * NOTE: In addition, this function does not return `true' when
 *       all that's left are dex objects (which are destroyed
 *       at a later point during deemon shutdown, than the point
 *       when this function is called to determine if the GC must
 *       continue to run) */
INTDEF bool DCALL DeeGC_IsEmptyWithoutDex(void);
#endif

/* GC object alloc/free.
 * Don't you think these functions allocate some magical memory
 * that can somehow track what objects it references. - No!
 * All these do is allocate a block of memory of `n_bytes' that
 * includes some storage at negative offsets to hold a `struct gc_head',
 * as is required for objects that should later be tracked by the GC. */
DFUNDEF ATTR_MALLOC void *(DCALL DeeGCObject_Malloc)(size_t n_bytes);
DFUNDEF ATTR_MALLOC void *(DCALL DeeGCObject_Calloc)(size_t n_bytes);
DFUNDEF void *(DCALL DeeGCObject_Realloc)(void *p, size_t n_bytes);
DFUNDEF ATTR_MALLOC void *(DCALL DeeGCObject_TryMalloc)(size_t n_bytes);
DFUNDEF ATTR_MALLOC void *(DCALL DeeGCObject_TryCalloc)(size_t n_bytes);
DFUNDEF void *(DCALL DeeGCObject_TryRealloc)(void *p, size_t n_bytes);
DFUNDEF void (DCALL DeeGCObject_Free)(void *p);

/* Allocate fixed-size, gc-object-purposed slab memory.
 * NOTE: This memory must be freed by one of:
 *   - DeeGCObject_FFree(return,size2)     | size2 <= size
 *   - DeeGCObject_SlabFree<size2>(return) | size2 <= size
 *   - DeeGCObject_Free(return) */
#ifdef CONFIG_NO_OBJECT_SLABS
#define DeeGCObject_FMalloc(size)    DeeGCObject_Malloc(size)
#define DeeGCObject_FCalloc(size)    DeeGCObject_Calloc(size)
#define DeeGCObject_FTryMalloc(size) DeeGCObject_TryMalloc(size)
#define DeeGCObject_FTryCalloc(size) DeeGCObject_TryCalloc(size)
#define DeeGCObject_FFree(ptr,size)  DeeGCObject_Free(ptr)
#else /* CONFIG_NO_OBJECT_SLABS */
#define DEFINE_SLAB_FUNCTIONS(size) \
DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeGCObject_SlabMalloc##size)(void); \
DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeGCObject_SlabCalloc##size)(void); \
DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeGCObject_SlabTryMalloc##size)(void); \
DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeGCObject_SlabTryCalloc##size)(void); \
DFUNDEF void (DCALL DeeGCObject_SlabFree##size)(void *__restrict ptr); \
/**/
DEE_ENUMERATE_SLAB_SIZES(DEFINE_SLAB_FUNCTIONS)
#undef DEFINE_SLAB_FUNCTIONS
#define DeeGCObject_FMalloc(size)    DeeSlab_Invoke(DeeGCObject_SlabMalloc,size,(),DeeGCObject_Malloc(size))
#define DeeGCObject_FCalloc(size)    DeeSlab_Invoke(DeeGCObject_SlabCalloc,size,(),DeeGCObject_Calloc(size))
#define DeeGCObject_FTryMalloc(size) DeeSlab_Invoke(DeeGCObject_SlabTryMalloc,size,(),DeeGCObject_TryMalloc(size))
#define DeeGCObject_FTryCalloc(size) DeeSlab_Invoke(DeeGCObject_SlabTryCalloc,size,(),DeeGCObject_TryCalloc(size))
#define DeeGCObject_FFree(ptr,size)  DeeSlab_Invoke(DeeGCObject_SlabFree,size,(ptr),DeeGCObject_Free(ptr))
#endif /* !CONFIG_NO_OBJECT_SLABS */

/* Same as the regular malloc functions, but use the same allocation methods that
 * would be used by `TYPE_FIXED_ALLOCATOR_GC' and `TYPE_FIXED_ALLOCATOR_GC_S', meaning
 * that pointers returned by these macros have binary compatibility with them. */
#define DeeGCObject_MALLOC(T)       ((T *)DeeGCObject_FMalloc(sizeof(T)))
#define DeeGCObject_CALLOC(T)       ((T *)DeeGCObject_FCalloc(sizeof(T)))
#define DeeGCObject_TRYMALLOC(T)    ((T *)DeeGCObject_FTryMalloc(sizeof(T)))
#define DeeGCObject_TRYCALLOC(T)    ((T *)DeeGCObject_FTryCalloc(sizeof(T)))
#define DeeGCObject_FREE(typed_ptr)       DeeGCObject_FFree(typed_ptr,sizeof(*(typed_ptr)))

#ifdef CONFIG_FIXED_ALLOCATOR_S_IS_AUTO
#define DeeGCObject_MALLOC_S(T)    ((T *)DeeGCObject_Malloc(sizeof(T)))
#define DeeGCObject_CALLOC_S(T)    ((T *)DeeGCObject_Calloc(sizeof(T)))
#define DeeGCObject_TRYMALLOC_S(T) ((T *)DeeGCObject_TryMalloc(sizeof(T)))
#define DeeGCObject_TRYCALLOC_S(T) ((T *)DeeGCObject_TryCalloc(sizeof(T)))
#define DeeGCObject_FREE_S               DeeGCObject_Free
#else
#define DeeGCObject_MALLOC_S      DeeGCObject_MALLOC
#define DeeGCObject_CALLOC_S      DeeGCObject_CALLOC
#define DeeGCObject_TRYMALLOC_S   DeeGCObject_TRYMALLOC
#define DeeGCObject_TRYCALLOC_S   DeeGCObject_TRYCALLOC
#define DeeGCObject_FREE_S        DeeGCObject_FREE
#endif

/* An generic sequence singleton that can be
 * iterated to yield all tracked GC objects.
 * This object also offsets a hand full of member functions
 * that user-space an invoke to trigger various GC-related
 * functionality:
 *   - collect(int max = -1) -> int;
 * Also: remember that this derives from `sequence', so you
 *       can use all its functions, like `empty()', etc.
 * NOTE: This object is exported as `gc from deemon' */
DDATDEF DeeObject DeeGCEnumTracked_Singleton;


DECL_END

#endif /* !GUARD_DEEMON_GC_H */
