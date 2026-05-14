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
/*!export **/
/*!export Dee_HASHSET_HIDXIO_**/
/*!export DeeHashSet_**/
/*!export Dee_HASHSET_**/
/*!export Dee_hashset_**/
/*!export _DeeHashSet_**/
/*!export _Dee_HASHSET_**/
/*!export __DeeHashSet_HashIdx**/
#ifndef GUARD_DEEMON_HASHSET_H
#define GUARD_DEEMON_HASHSET_H 1 /*!export-*/

#include "api.h"

#include "object.h"       /* DeeObject_NewDefault */
#include "types.h"        /* DREF, DeeObject, DeeObject_InstanceOf, DeeObject_InstanceOfExact, DeeTypeObject, Dee_AsObject, Dee_OBJECT_HEAD, Dee_OBJECT_HEAD_INIT, Dee_WEAKREF_SUPPORT, Dee_WEAKREF_SUPPORT_INIT, Dee_hash_t */
#include "util/hash-io.h" /* Dee_hash_*, _DeeHash_* */
#include "util/lock.h"    /* Dee_ATOMIC_RWLOCK_INIT, Dee_atomic_read_with_atomic_rwlock, Dee_atomic_rwlock_* */

#include <stddef.h> /* size_t */

#ifndef __INTELLISENSE__
#include "method-hints.h" /* DeeObject_InvokeMethodHint */
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

struct Dee_hashset_item {
	Dee_hash_t      hsi_hash; /* [valis_if(hsi_key)] Hash of `hsi_key' (undefined, but readable when "di_key == NULL") */
	DREF DeeObject *hsi_key;  /* [0..1] Item key, or "NULL" if deleted and not cleaned up (yet) */
};

/* Static initializer for `struct Dee_hashset_item' */
#define Dee_HASHSET_ITEM_INIT(hash, key) { hash, key }

typedef struct Dee_hashset_object {
	Dee_OBJECT_HEAD /* GC Object */
	/*real*/Dee_hash_vidx_t           hs_valloc;  /* [lock(hs_lock)][<= hs_hmask] Allocated size of "hs_vtab" (should be ~2/3rd of `hs_hmask + 1') */
	/*real*/Dee_hash_vidx_t           hs_vsize;   /* [lock(hs_lock)][<= hs_valloc] 1+ the greatest index in "hs_vtab" that was ever initialized (and also the index of the next item in "hs_vtab" to-be populated). */
	size_t                            hs_vused;   /* [lock(hs_lock)][<= hs_vsize] # of non-NULL keys in "hs_vtab". */
	struct Dee_hashset_item          *hs_vtab;    /* [lock(hs_lock)][0..hs_vsize][ownehs_if(!= INTERNAL(DeeDict_EmptyTab))]
	                                               * [OWNED_AT(. + 1)] Value-table (offset by 1 to account for special meaning of index==Dee_HASH_HTAB_EOF) */
	Dee_hash_t                        hs_hmask;   /* [lock(hs_lock)] Hash mask (allocated hash-map size, minus 1). */
	struct Dee_hash_hidxio_ops const *hs_hidxops; /* [lock(d_lock)][1..1] Operators for "d_htab" (always depends on "d_valloc") */
	union Dee_hash_htab              *hs_htab;    /* [lock(hs_lock)][== (byte_t *)(_DeeDict_GetRealVTab(this) + hs_valloc)] Hash-table (contains indices into "hs_vtab", index==Dee_HASH_HTAB_EOF means END-OF-CHAIN) */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t               hs_lock;    /* Lock used for accessing this Dict. */
#endif /* !CONFIG_NO_THREADS */
	Dee_WEAKREF_SUPPORT
} DeeHashSetObject;

#define DeeHashSet_EmptyVTab _DeeHash_VIRT_EmptyVTab(struct Dee_hashset_item)
#define DeeHashSet_EmptyHTab _DeeHash_EmptyHTab

#ifdef CONFIG_NO_THREADS
#define _Dee_HASHSET_INIT_LOCK /* nothing */
#else /* CONFIG_NO_THREADS */
#define _Dee_HASHSET_INIT_LOCK , Dee_ATOMIC_RWLOCK_INIT
#endif /* !CONFIG_NO_THREADS */
#define Dee_HASHSET_INIT                          \
	{                                             \
		Dee_OBJECT_HEAD_INIT(&DeeHashSet_Type),   \
		/* .hs_valloc  = */ 0,                    \
		/* .hs_vsize   = */ 0,                    \
		/* .hs_vused   = */ 0,                    \
		/* .hs_vtab    = */ DeeHashSet_EmptyVTab, \
		/* .hs_hmask   = */ 0,                    \
		/* .hs_hidxops = */ &Dee_hash_hidxio[0],  \
		/* .hs_htab    = */ DeeHashSet_EmptyHTab  \
		_Dee_HASHSET_INIT_LOCK,                   \
		Dee_WEAKREF_SUPPORT_INIT                  \
	}

#ifdef DEE_SOURCE
#define _DeeHashSet_GetVirtVTab(self)    _DeeHash_VIRT_GetVirtVTab((self)->hs_vtab)
#define _DeeHashSet_SetVirtVTab(self, v) _DeeHash_VIRT_SetVirtVTab((self)->hs_vtab, v)
#define _DeeHashSet_GetRealVTab(self)    _DeeHash_VIRT_GetRealVTab((self)->hs_vtab)
#define _DeeHashSet_SetRealVTab(self, v) _DeeHash_VIRT_SetRealVTab((self)->hs_vtab, v)

/* Advance hash-index */
#define _DeeHashSet_HashIdxInit(self, p_hs, p_perturb, hash) _DeeHash_HashIdxInit(p_hs, p_perturb, hash, (self)->hs_hmask)
#define _DeeHashSet_HashIdxNext(self, p_hs, p_perturb, hash) _DeeHash_HashIdxNext(p_hs, p_perturb, hash, (self)->hs_hmask)

#define _DeeHashSet_HTabGet(self, htab_index)    (*(self)->hs_hidxops->hxio_get)((self)->hs_htab, htab_index)
#define _DeeHashSet_HTabSet(self, htab_index, v) (*(self)->hs_hidxops->hxio_set)((self)->hs_htab, htab_index, v)
#endif /* DEE_SOURCE */

/* Locking helpers. */
#define DeeHashSet_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->hs_lock)
#define DeeHashSet_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->hs_lock)
#define DeeHashSet_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->hs_lock)
#define DeeHashSet_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->hs_lock)
#define DeeHashSet_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->hs_lock)
#define DeeHashSet_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->hs_lock)
#define DeeHashSet_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->hs_lock)
#define DeeHashSet_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->hs_lock)
#define DeeHashSet_LockRead(self)       Dee_atomic_rwlock_read(&(self)->hs_lock)
#define DeeHashSet_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->hs_lock)
#define DeeHashSet_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->hs_lock)
#define DeeHashSet_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->hs_lock)
#define DeeHashSet_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->hs_lock)
#define DeeHashSet_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->hs_lock)
#define DeeHashSet_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->hs_lock)
#define DeeHashSet_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->hs_lock)


/* Return the # of bound key within "self". */
#define DeeHashSet_SIZE(self) ((self)->hs_vused)
#define DeeHashSet_SIZE_ATOMIC(self) \
	Dee_atomic_read_with_atomic_rwlock(&(self)->hs_vused, &(self)->hs_lock)


/* The main `HashSet' container class. */
DDATDEF DeeTypeObject DeeHashSet_Type;
#define DeeHashSet_Check(ob)      DeeObject_InstanceOf(ob, &DeeHashSet_Type)
#define DeeHashSet_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeHashSet_Type)

#define DeeHashSet_New() DeeObject_NewDefault(&DeeHashSet_Type)
DFUNDEF WUNUSED DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_TryNewWithHint(size_t num_items);
DFUNDEF WUNUSED DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_TryNewWithWeakHint(size_t num_items);
DFUNDEF WUNUSED DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_NewWithHint(size_t num_items);
DFUNDEF WUNUSED DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_NewWithWeakHint(size_t num_items);
DFUNDEF WUNUSED NONNULL((1)) DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_FromSequenceInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_FromRoSet(/*RoSet*/ DeeObject *__restrict self);

/* Create a new HashSet by inheriting a set of passed keys.
 * @param: items:    A vector containing `num_keys' objects.
 * @param: num_keys: The number of keys passed.
 * WARNING: This function does _NOT_ inherit the passed vector, but _ONLY_ references to its elements! */
DFUNDEF WUNUSED DREF /*Dict*/ DeeObject *DCALL
DeeHashSet_NewItemsInherited(size_t num_keys,
                             /*inherit(on_success)*/ DREF DeeObject **items);


#ifdef __INTELLISENSE__
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeHashSet_Insert(DeeObject *self, DeeObject *item);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeHashSet_Remove(DeeObject *self, DeeObject *item);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeHashSet_Contains(DeeObject *self, DeeObject *item);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeHashSet_Unify(DeeObject *self, DeeObject *item);
#define DeeHashSet_Insert(self, item)   DeeHashSet_Insert(Dee_AsObject(self), item)
#define DeeHashSet_Remove(self, item)   DeeHashSet_Remove(Dee_AsObject(self), item)
#define DeeHashSet_Contains(self, item) DeeHashSet_Contains(Dee_AsObject(self), item)
#define DeeHashSet_Unify(self, item)    DeeHashSet_Unify(Dee_AsObject(self), item)
#else /* __INTELLISENSE__ */
#define DeeHashSet_Insert(self, item)   DeeObject_InvokeMethodHint(set_insert, self, item)
#define DeeHashSet_Remove(self, item)   DeeObject_InvokeMethodHint(set_remove, self, item)
#define DeeHashSet_Contains(self, item) DeeObject_InvokeMethodHint(seq_contains, self, item)
#define DeeHashSet_Unify(self, item)    DeeObject_InvokeMethodHint(set_unify, self, item)
#endif /* !__INTELLISENSE__ */

DECL_END

#endif /* !GUARD_DEEMON_HASHSET_H */
