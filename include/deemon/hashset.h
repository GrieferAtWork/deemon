/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_HASHSET_H
#define GUARD_DEEMON_HASHSET_H 1

#include "api.h"

#include <stdbool.h>
#include <stddef.h>

#include "object.h"
#include "util/lock.h"

DECL_BEGIN


#ifdef DEE_SOURCE
#define Dee_hashset_item   hashset_item
#define Dee_hashset_object hashset_object
#endif /* DEE_SOURCE */

typedef struct Dee_hashset_object DeeHashSetObject;

struct Dee_hashset_item {
	DREF DeeObject *hsi_key;  /* [0..1][lock(:hs_lock)] Set item key. */
	Dee_hash_t      hsi_hash; /* [valis_if(hsi_key)][lock(:hs_lock)] Hash of `hsi_key' (with a starting value of `0').
	                           * NOTE: Some random value when `hsi_key' is the dummy key. */
};

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

#define DeeHashSet_EmptyItems ((struct Dee_hashset_item const *)DeeDict_EmptyItems)

/* The main `HashSet' container class. */
DDATDEF DeeTypeObject DeeHashSet_Type;
#define DeeHashSet_Check(ob)      DeeObject_InstanceOf(ob, &DeeHashSet_Type)
#define DeeHashSet_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeHashSet_Type)

#define DeeHashSet_New() DeeObject_NewDefault(&DeeHashSet_Type)
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeHashSet_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeHashSet_FromIterator(DeeObject *__restrict self);


/* @return:  1: Successfully inserted/removed the object.
 * @return:  0: An identical object already exists/was already removed.
 * @return: -1: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeHashSet_Insert(DeeObject *self, DeeObject *item);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeHashSet_Remove(DeeObject *self, DeeObject *item);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeHashSet_InsertString(DeeObject *__restrict self,
                        char const *__restrict item, size_t item_length);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeHashSet_RemoveString(DeeObject *__restrict self,
                        char const *__restrict item, size_t item_length);

/* @return:  1/true:  The object exists.
 * @return:  0/false: No such object exists.
 * @return: -1:       An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL
DeeHashSet_Contains(DeeObject *self, DeeObject *item);
DFUNDEF WUNUSED NONNULL((1, 2)) bool DCALL
DeeHashSet_ContainsString(DeeObject *__restrict self,
                          char const *__restrict item, size_t item_length);

/* Unifies a given object, either inserting it into the set and re-returning
 * it, or returning another, identical instance already apart of the set. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeHashSet_Unify(DeeObject *self, DeeObject *search_item);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF /*String*/ DeeObject *DCALL
DeeHashSet_UnifyString(DeeObject *__restrict self,
                       char const *__restrict item, size_t item_length);

/* Create a new HashSet by inheriting a set of passed key-item pairs.
 * @param: items:     A vector containing `num_items' elements,
 *                    even ones being keys and odd ones being items.
 * @param: num_items: The number of items passed.
 * WARNING: This function does _NOT_ inherit the passed vector, but _ONLY_ its elements! */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeHashSet_NewItemsInherited(size_t num_items,
                             /*inherit(on_success)*/ DREF DeeObject **items);

/* The basic HashSet item lookup algorithm:
 * >> DeeObject *get_item(DeeObject *self, DeeObject *key) {
 * >>     Dee_hash_t i, perturb;
 * >>     Dee_hash_t hash = DeeObject_Hash(0, key);
 * >>     perturb = i = DeeHashSet_HashSt(self, hash);
 * >>     for (;; i = DeeHashSet_HashNx(i, perturb), DeeHashSet_HashPt(perturb)) {
 * >>          struct hashset_item *item = DeeHashSet_HashIt(self, i);
 * >>          if (!item->hsi_key)
 * >>              break; // Not found
 * >>          if (item->hsi_hash != hash)
 * >>              continue; // Non-matching hash
 * >>          if (DeeObject_CompareEq(key, item->hsi_key))
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

DECL_END

#endif /* !GUARD_DEEMON_HASHSET_H */
