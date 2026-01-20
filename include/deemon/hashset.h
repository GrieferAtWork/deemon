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
#ifndef GUARD_DEEMON_HASHSET_H
#define GUARD_DEEMON_HASHSET_H 1

#include "api.h"

#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
#include "dict.h" /* Dee_dict_vidx_t, Dee_dict_gethidx_t, Dee_dict_sethidx_t */
#endif /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */
#include "types.h"
#include "util/lock.h"
#ifndef __INTELLISENSE__
#include "util/atomic.h"
#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
#include "method-hints.h"
#endif /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */
#endif /* !__INTELLISENSE__ */
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN


#ifdef DEE_SOURCE
#define Dee_hashset_item   hashset_item
#define Dee_hashset_object hashset_object
#endif /* DEE_SOURCE */

typedef struct Dee_hashset_object DeeHashSetObject;


#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
struct Dee_hashset_item {
	Dee_hash_t      hsi_hash; /* [valis_if(hsi_key)] Hash of `hsi_key' (undefined, but readable when "di_key == NULL") */
	DREF DeeObject *hsi_key;  /* [0..1] Item key, or "NULL" if deleted and not cleaned up (yet) */
};

/* Static initializer for `struct Dee_hashset_item' */
#define Dee_HASHSET_ITEM_INIT(hash, key) { hash, key }

struct Dee_hashset_object {
	Dee_OBJECT_HEAD /* GC Object */
	/*real*/Dee_dict_vidx_t  hs_valloc;  /* [lock(hs_lock)][<= hs_hmask] Allocated size of "hs_vtab" (should be ~2/3rd of `hs_hmask + 1') */
	/*real*/Dee_dict_vidx_t  hs_vsize;   /* [lock(hs_lock)][<= hs_valloc] 1+ the greatest index in "hs_vtab" that was ever initialized (and also the index of the next item in "hs_vtab" to-be populated). */
	size_t                   hs_vused;   /* [lock(hs_lock)][<= hs_vsize] # of non-NULL keys in "hs_vtab". */
	struct Dee_hashset_item *hs_vtab;    /* [lock(hs_lock)][0..hs_vsize][ownehs_if(!= INTERNAL(DeeDict_EmptyTab))]
	                                      * [OWNED_AT(. + 1)] Value-table (offset by 1 to account for special meaning of index==Dee_DICT_HTAB_EOF) */
	Dee_hash_t               hs_hmask;   /* [lock(hs_lock)] Hash mask (allocated hash-map size, minus 1). */
	Dee_dict_gethidx_t       hs_hidxget; /* [lock(hs_lock)][1..1] Getter for "hs_htab" (always depends on "hs_valloc") */
	Dee_dict_sethidx_t       hs_hidxset; /* [lock(hs_lock)][1..1] Setter for "hs_htab" (always depends on "hs_valloc") */
	void                    *hs_htab;    /* [lock(hs_lock)][== (byte_t *)(_DeeDict_GetRealVTab(this) + hs_valloc)] Hash-table (contains indices into "hs_vtab", index==Dee_DICT_HTAB_EOF means END-OF-CHAIN) */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t      hs_lock;    /* Lock used for accessing this Dict. */
#endif /* !CONFIG_NO_THREADS */
	Dee_WEAKREF_SUPPORT
};

#define DeeHashSet_EmptyVTab /*virt*/ ((struct Dee_hashset_item *)_DeeDict_EmptyTab - 1)
#define DeeHashSet_EmptyHTab ((void *)_DeeDict_EmptyTab)

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
		/* .hs_hidxget = */ &Dee_dict_gethidx8,   \
		/* .hs_hidxset = */ &Dee_dict_sethidx8,   \
		/* .hs_htab    = */ DeeHashSet_EmptyHTab  \
		_Dee_HASHSET_INIT_LOCK,                   \
		Dee_WEAKREF_SUPPORT_INIT                  \
	}

#ifdef DEE_SOURCE

/* Helper macros for converting between "virt" and "real" hashset VIDX indices. */
#define Dee_hashset_vidx_virt2real    Dee_dict_vidx_virt2real
#define Dee_hashset_vidx_real2virt    Dee_dict_vidx_real2virt
#define Dee_hashset_vidx_toreal       Dee_dict_vidx_toreal
#define Dee_hashset_vidx_tovirt       Dee_dict_vidx_tovirt
#define Dee_hashset_vidx_virt_lt_real Dee_dict_vidx_virt_lt_real

/* NOTE: HIDXIO indices can also used as <<shifts to multiply some value by the size of an index:
 * >> size_t htab_size = (hs_hmask + 1) << DEE_HASHSET_HIDXIO_FROMALLOC(...); */
#define DEE_HASHSET_HIDXIO_COUNT     DEE_DICT_HIDXIO_COUNT
#define DEE_HASHSET_HIDXIO_IS8       DEE_DICT_HIDXIO_IS8
#define DEE_HASHSET_HIDXIO_IS16      DEE_DICT_HIDXIO_IS16
#define DEE_HASHSET_HIDXIO_IS32      DEE_DICT_HIDXIO_IS32
#define DEE_HASHSET_HIDXIO_FROMALLOC DEE_DICT_HIDXIO_FROMALLOC
#define Dee_hashset_hidxio           Dee_dict_hidxio

/* Index value found in "hs_htab" when end-of-chain is encountered. */
#define Dee_HASHSET_HTAB_EOF Dee_DICT_HTAB_EOF

/* Get/set "hs_vtab" in both its:
 * - virt[ual] (index starts at 1), and
 * - real (index starts at 0) form */
#define _DeeHashSet_GetVirtVTab(self)    ((self)->hs_vtab)
#define _DeeHashSet_SetVirtVTab(self, v) (void)((self)->hs_vtab = (v))
#define _DeeHashSet_GetRealVTab(self)    ((self)->hs_vtab + 1)
#define _DeeHashSet_SetRealVTab(self, v) (void)((self)->hs_vtab = (v) - 1)

/* Advance hash-index */
#define _DeeHashSet_HashIdxInit(self, p_hs, p_perturb, hash) \
	__DeeHashSet_HashIdxInitEx(p_hs, p_perturb, hash, (self)->hs_hmask)
#define _DeeHashSet_HashIdxAdv(self, p_hs, p_perturb) \
	__DeeHashSet_HashIdxAdvEx(p_hs, p_perturb)

#define __DeeHashSet_HashIdxInitEx(p_hs, p_perturb, hash, hmask) \
	(void)(*(p_hs) = (*(p_perturb) = (hash)) & (hmask))
#define __DeeHashSet_HashIdxAdvEx(p_hs, p_perturb) \
	(void)(*(p_hs) = (*(p_hs) << 2) + *(p_hs) + *(p_perturb) + 1, *(p_perturb) >>= 5)

/* Get/set vtab-index "i" of htab at a given "hs" */
#define /*virt*/_DeeHashSet_HTabGet(self, hs)    (*(self)->hs_hidxget)((self)->hs_htab, (hs) & (self)->hs_hmask)
#define _DeeHashSet_HTabSet(self, hs, /*virt*/i) (*(self)->hs_hidxset)((self)->hs_htab, (hs) & (self)->hs_hmask, i)
#endif /* DEE_SOURCE */

#else /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */
struct Dee_hashset_item {
	DREF DeeObject *hsi_key;  /* [0..1][lock(:hs_lock)] Set item key. */
	Dee_hash_t      hsi_hash; /* [valis_if(hsi_key)][lock(:hs_lock)] Hash of `hsi_key'.
	                           * NOTE: Some random value when `hsi_key' is the dummy key. */
};

/* Static initializer for `struct Dee_hashset_item' */
#define Dee_HASHSET_ITEM_INIT(hash, key) { key, hash }

struct Dee_hashset_object {
	Dee_OBJECT_HEAD /* GC Object */
	size_t                   hs_mask; /* [lock(hs_lock)][> hs_size || hs_mask == 0] Allocated set size minus 1. */
	size_t                   hs_size; /* [lock(hs_lock)][< hs_mask || hs_mask == 0] Amount of non-NULL keys. */
	size_t                   hs_used; /* [lock(hs_lock)][<= hs_size] Amount of keys actually in use.
	                                   * HINT: The difference to `hs_size' is the number of dummy keys currently in use. */
	struct Dee_hashset_item *hs_elem; /* [1..hs_size|ALLOC(hs_mask+1)][lock(hs_lock)]
	                                   * [ownes_if(!= INTERNAL(empty_set_items))] Set keys. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t      hs_lock; /* Lock used for accessing this set. */
#endif /* !CONFIG_NO_THREADS */
	Dee_WEAKREF_SUPPORT
};

/* The basic HashSet item lookup algorithm:
 * >> DeeObject *get_item(DeeObject *self, DeeObject *key) {
 * >>     Dee_hash_t i, perturb;
 * >>     Dee_hash_t hash = DeeObject_Hash(key);
 * >>     perturb = i = DeeHashSet_HashSt(self, hash);
 * >>     for (;; i = DeeHashSet_HashNx(i, perturb), DeeHashSet_HashPt(perturb)) {
 * >>          struct hashset_item *item = DeeHashSet_HashIt(self, i);
 * >>          if (!item->hsi_key)
 * >>              break; // Not found
 * >>          if (item->hsi_hash != hash)
 * >>              continue; // Non-matching hash
 * >>          if (DeeObject_TryCompareEq(key, item->hsi_key) == 0)
 * >>              return item->si_item;
 * >>     }
 * >>     return NULL;
 * >> }
 * Requirements to prevent an infinite loop:
 * - `DeeHashSet_HashNx()' must be able to eventually enumerate all integers `<= hs_mask'
 * - `hs_size' must always be lower than `hs_mask', ensuring that at least one(1)
 *   entry exists that no value has been assigned to (Acting as a sentinel to
 *   terminate a search for an existing element).
 * - `hs_elem' must never be empty (or `NULL' for that matter)
 * NOTE: I can't say that I came up with the way that this mapping
 *       algorithm works (but no-one can really claim to have invented
 *       something ~new~ nowadays. - It's always been done already).
 *       Yet the point here is, that this is similar to what python
 *       does for its dictionary lookup.
 */
#define DeeHashSet_HashSt(self, hash)  ((hash) & (self)->hs_mask)
#define DeeHashSet_HashNx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define DeeHashSet_HashIt(self, i)     ((self)->hs_elem + ((i) & (self)->hs_mask))

DDATDEF struct Dee_hashset_item const DeeHashSet_EmptyItems[1];
#endif /* !CONFIG_EXPERIMENTAL_ORDERED_HASHSET */

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


#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
#define _DeeHashSet_SIZEFIELD hs_vused
#else /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */
#define _DeeHashSet_SIZEFIELD hs_used
#endif /* !CONFIG_EXPERIMENTAL_ORDERED_HASHSET */

/* Return the # of bound key within "self". */
#define DeeHashSet_SIZE(self) ((self)->_DeeHashSet_SIZEFIELD)
#define DeeHashSet_SIZE_ATOMIC(self) \
	Dee_atomic_read_with_atomic_rwlock(&(self)->_DeeHashSet_SIZEFIELD, &(self)->hs_lock)


/* The main `HashSet' container class. */
DDATDEF DeeTypeObject DeeHashSet_Type;
#define DeeHashSet_Check(ob)      DeeObject_InstanceOf(ob, &DeeHashSet_Type)
#define DeeHashSet_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeHashSet_Type)

#define DeeHashSet_New() DeeObject_NewDefault(&DeeHashSet_Type)
#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeHashSet_TryNewWithHint(size_t num_items);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeHashSet_TryNewWithWeakHint(size_t num_items);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeHashSet_NewWithHint(size_t num_items);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeHashSet_NewWithWeakHint(size_t num_items);
#else /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeHashSet_NewWithHint(size_t num_items);
#endif /* !CONFIG_EXPERIMENTAL_ORDERED_HASHSET */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeHashSet_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeHashSet_FromSequenceInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeHashSet_FromRoSet(DeeObject *__restrict self);

/* Create a new HashSet by inheriting a set of passed key-item pairs.
 * @param: items:     A vector containing `num_items' elements,
 *                    even ones being keys and odd ones being items.
 * @param: num_items: The number of items passed.
 * WARNING: This function does _NOT_ inherit the passed vector, but _ONLY_ its elements! */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeHashSet_NewItemsInherited(size_t num_items,
                             /*inherit(on_success)*/ DREF DeeObject **items);


#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeHashSet_Insert(DeeObject *self, DeeObject *item);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeHashSet_Remove(DeeObject *self, DeeObject *item);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeHashSet_Contains(DeeObject *self, DeeObject *item);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeHashSet_Unify(DeeObject *self, DeeObject *item);
#else /* __INTELLISENSE__ */
#define DeeHashSet_Insert(self, item)   DeeObject_InvokeMethodHint(set_insert, self, item)
#define DeeHashSet_Remove(self, item)   DeeObject_InvokeMethodHint(set_remove, self, item)
#define DeeHashSet_Contains(self, item) DeeObject_InvokeMethodHint(seq_contains, self, item)
#define DeeHashSet_Unify(self, item)    DeeObject_InvokeMethodHint(set_unify, self, item)
#endif /* !__INTELLISENSE__ */
#else /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */
/* @return:  1: Successfully inserted/removed the object.
 * @return:  0: An identical object already exists/was already removed.
 * @return: -1: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeHashSet_Insert(DeeObject *self, DeeObject *item);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeHashSet_Remove(DeeObject *self, DeeObject *item);

/* @return:  1/true:  The object exists.
 * @return:  0/false: No such object exists.
 * @return: -1:       An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeHashSet_Contains(DeeObject *self, DeeObject *item);

/* Unifies a given object, either inserting it into the set and re-returning
 * it, or returning another, identical instance already apart of the set. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeHashSet_Unify(DeeObject *self, DeeObject *item);
#endif /* !CONFIG_EXPERIMENTAL_ORDERED_HASHSET */

DECL_END

#endif /* !GUARD_DEEMON_HASHSET_H */
