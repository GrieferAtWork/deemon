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
#ifndef GUARD_DEEMON_OBJECTS_GC_INSPECT_H
#define GUARD_DEEMON_OBJECTS_GC_INSPECT_H 1

#include <deemon/api.h>

#include <deemon/object.h>
#include <deemon/types.h>

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, size_t */

DECL_BEGIN

/* GC inspection provides things such as sequences implemented using the `tp_visit'
 * interface, allowing user-code to determine if object `a' is reachable from `b',
 * as well as enumerate objects reachable from some point, as well as determine how
 * far one would have to travel before getting to the destination.
 * Note that only non-trivial objects are guarantied to be testable for this
 * purpose, meaning that the implementation is allowed to not account for objects
 * that are unable to ever cause a reference loop, the main example here being
 * strings, or conversely: sequences of string, as well as integer members that
 * are only created when the associated attribute is accessed. */

typedef struct {
	OBJECT_HEAD
	/* Some sequence of objects somehow derived from, or related to a target, such as:
	 *  - <referred to by X>, <reachable from X>
	 *  - <GC objects referred to by X>, <GC objects reachable from X>
	 *  - <GC objects referring to X> */
	size_t                                    gs_size;  /* [const] Number of objects apart of the hash-vector. */
	size_t                                    gs_mask;  /* [const] Mask of hash-vector. */
	COMPILER_FLEXIBLE_ARRAY(DREF DeeObject *, gs_elem); /* [0..1][const][gs_mask+1] Hash-vector. */
} GCSet;

INTDEF DeeTypeObject DeeGCSet_Type;

typedef struct {
	OBJECT_HEAD
	size_t          gs_size;    /* [const] Number of objects apart of the hash-vector. */
	size_t          gs_mask;    /* [const] Mask of hash-vector. */
	DREF DeeObject *gs_elem[1]; /* [0..1][const][gs_mask+1] Hash-vector. */
} GCSet_Empty;
INTDEF GCSet_Empty DeeGCSet_Empty;
#define DeeGCSet_NewEmpty() (Dee_Incref(&DeeGCSet_Empty), (GCSet *)&DeeGCSet_Empty)

#define GCSET_HASHOBJ(x)          Dee_HashPointer(x)
#define GCSET_HASHNXT(i, perturb) ((i) = ((i) << 2) + (i) + (perturb) + 1, (perturb) >>= 5)

typedef struct {
	GCSet           *gs_set;  /* [0..1] The set being constructed. */
	size_t           gs_err;  /* When non-zero, an error occurred after the
	                           * runtime failed to allocate this many bytes.
	                           * Once everything has been unlocked, try to collect
	                           * this much memory, then restart. */
} GCSetMaker;

#define GCSETMAKER_INIT { NULL, 0 }

/* Finalize the given GC-set maker. */
INTDEF NONNULL((1)) void DCALL GCSetMaker_Fini(GCSetMaker *__restrict self);

/* @return:  1: Object was already inserted into the set.
 * @return:  0: Object was newly inserted into the set.
 * @return: -1: An allocation failed (release all locks and to collect `self->gs_err' bytes of memory) */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
GCSetMaker_Insert(GCSetMaker *__restrict self,
                  /*inherit(return == 0)*/ DREF DeeObject *__restrict ob);

/* Remove all non-GC objects from the given set. */
INTDEF WUNUSED NONNULL((1)) int DCALL GCSetMaker_RemoveNonGC(GCSetMaker *__restrict self);

/* Pack the current set of objects and return them after (always) finalizing `self' */
INTDEF WUNUSED NONNULL((1)) DREF GCSet *DCALL GCSetMaker_Pack(/*inherit(always)*/ GCSetMaker *__restrict self);


INTDEF WUNUSED NONNULL((1)) DREF GCSet *DCALL DeeGC_NewReferred(DeeObject *__restrict start);                                 /* Objects immediately referred to by X */
INTDEF WUNUSED NONNULL((1)) DREF GCSet *DCALL DeeGC_NewReachable(DeeObject *__restrict start);                                /* Objects reachable by X */
INTDEF WUNUSED NONNULL((1)) DREF GCSet *DCALL DeeGC_NewReferredGC(DeeObject *__restrict start);                               /* GC objects immediately referred to by X */
INTDEF WUNUSED NONNULL((1)) DREF GCSet *DCALL DeeGC_NewReachableGC(DeeObject *__restrict start);                              /* GC objects reachable by X */
INTDEF WUNUSED NONNULL((1, 2)) DREF GCSet *DCALL DeeGC_TNewReferred(DeeTypeObject *tp_start, DeeObject *__restrict start);    /* Objects immediately referred to by X */
INTDEF WUNUSED NONNULL((1, 2)) DREF GCSet *DCALL DeeGC_TNewReachable(DeeTypeObject *tp_start, DeeObject *__restrict start);   /* Objects reachable by X */
INTDEF WUNUSED NONNULL((1, 2)) DREF GCSet *DCALL DeeGC_TNewReferredGC(DeeTypeObject *tp_start, DeeObject *__restrict start);  /* GC objects immediately referred to by X */
INTDEF WUNUSED NONNULL((1, 2)) DREF GCSet *DCALL DeeGC_TNewReachableGC(DeeTypeObject *tp_start, DeeObject *__restrict start); /* GC objects reachable by X */
INTDEF WUNUSED NONNULL((1)) DREF GCSet *DCALL DeeGC_NewGCReferred(DeeObject *__restrict target);                              /* GC objects referring to X */


/* Collect referred, or reachable objects.
 * @return:  0: Collection was OK.
 * @return: -1: An error occurred. (not enough memory) */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeGC_CollectReferred(GCSetMaker *__restrict self, DeeTypeObject *tp_start, DeeObject *__restrict start);    /* Objects immediately referred to by X */
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeGC_CollectReachable(GCSetMaker *__restrict self, DeeTypeObject *tp_start, DeeObject *__restrict start);   /* Objects reachable by X */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeGC_CollectGCReferred(GCSetMaker *__restrict self, DeeObject *__restrict target);                             /* GC objects referring to X */

/* Returns `true' if `target' is referred to by `source' */
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeGC_ReferredBy(DeeObject *source, DeeObject *target);
#define DeeGC_IsReachable(object, from) DeeGC_ReferredBy(from, object)


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_GC_INSPECT_H */
