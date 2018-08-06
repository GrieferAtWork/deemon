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

struct gc_head {
    /* The structure that is prefixed before every GC-allocated object. */
    struct gc_head **gc_pself;  /* [1..1][== self][1..1][lock(INTERNAL(gc_lock))] Self-pointer in the global chain of GC objects. */
    struct gc_head  *gc_next;   /* [0..1][lock(INTERNAL(gc_lock))] Next GC object. */
    DeeObject        gc_object; /* The object that is being controlled by the GC. */
};
#define GC_OBJECT_OFFSET  offsetof(struct gc_head,gc_object)
#define GC_HEAD_SIZE      offsetof(struct gc_head,gc_object)
#define DeeGC_Head(ob)   ((struct gc_head *)((uintptr_t)(ob)-GC_OBJECT_OFFSET))
#define DeeGC_Object(ob) (&(ob)->gc_object)
#define DeeGC_Check(ob)  (Dee_TYPE(ob)->tp_flags&TP_FGC && (!DeeType_Check(ob) || (((DeeTypeObject *)(ob))->tp_flags&TP_FHEAP)))

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

#define DeeGCObject_MALLOC(T) ((T *)DeeGCObject_Malloc(sizeof(T)))
#define DeeGCObject_CALLOC(T) ((T *)DeeGCObject_Calloc(sizeof(T)))


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
