/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_HASHSET_H
#define GUARD_DEEMON_HASHSET_H 1

#include "api.h"

#include "object.h"

#ifndef CONFIG_NO_THREADS
#include "util/rwlock.h"
#endif /* !CONFIG_NO_THREADS */

#include <stdbool.h>
#include <stddef.h>

DECL_BEGIN


#ifdef DEE_SOURCE
#define Dee_hashset_item   hashset_item
#define Dee_hashset_object hashset_object
#endif /* DEE_SOURCE */

typedef struct Dee_hashset_object DeeHashSetObject;

struct Dee_hashset_item {
	DREF DeeObject *si_key;   /* [0..1][lock(:s_lock)] Set item key. */
	Dee_hash_t      si_hash;  /* [valis_if(si_key)][lock(:s_lock)] Hash of `si_key' (with a starting value of `0').
	                           * NOTE: Some random value when `si_key' is the dummy key. */
};

struct Dee_hashset_object {
	Dee_OBJECT_HEAD /* GC Object */
	size_t                   s_mask; /* [lock(s_lock)][> s_size || s_mask == 0] Allocated set size. */
	size_t                   s_size; /* [lock(s_lock)][< s_mask || s_mask == 0] Amount of non-NULL keys. */
	size_t                   s_used; /* [lock(s_lock)][<= s_size] Amount of keys actually in use.
	                                  * HINT: The difference to `s_size' is the number of dummy keys currently in use. */
	struct Dee_hashset_item *s_elem; /* [1..s_size|ALLOC(s_mask+1)][lock(s_lock)]
	                                  * [ownes_if(!= INTERNAL(empty_set_items))] Set keys. */
#ifndef CONFIG_NO_THREADS
	Dee_rwlock_t             s_lock; /* Lock used for accessing this set. */
#endif /* !CONFIG_NO_THREADS */
	Dee_WEAKREF_SUPPORT
};

/* The main `HashSet' container class. */
DDATDEF DeeTypeObject DeeHashSet_Type;
#define DeeHashSet_Check(ob)       DeeObject_InstanceOf(ob, &DeeHashSet_Type)
#define DeeHashSet_CheckExact(ob)  DeeObject_InstanceOfExact(ob, &DeeHashSet_Type)

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
 * >>          if (!item->si_key)
 * >>              break; // Not found
 * >>          if (item->si_hash != hash)
 * >>              continue; // Non-matching hash
 * >>          if (DeeObject_CompareEq(key, item->si_key))
 * >>              return item->si_item;
 * >>     }
 * >>     return NULL;
 * >> }
 * Requirements to prevent an infinite loop:
 *   - `DeeHashSet_HashNx()' must be able to eventually enumerate all integers `<= s_mask'
 *   - `s_size' must always be lower than `s_mask', ensuring that at least one(1)
 *      entry exists that no value has been assigned to (Acting as a sentinel to
 *      terminate a search for an existing element).
 *   - `s_elem' must never be empty (or `NULL' for that matter)
 * NOTE: I can't say that I came up with the way that this mapping
 *       algorithm works (but noone can really claim to have invented
 *       something ~new~ nowadays. - It's always been done already).
 *       Yet the point here is, that this is similar to what python
 *       does for its dictionary lookup.
 */
#define DeeHashSet_HashSt(self, hash)  ((hash) & ((DeeHashSetObject *)Dee_REQUIRES_OBJECT(self))->s_mask)
#define DeeHashSet_HashNx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define DeeHashSet_HashIt(self, i)     (((DeeHashSetObject *)Dee_REQUIRES_OBJECT(self))->s_elem + ((i) & ((DeeHashSetObject *)(self))->s_mask))



#ifndef CONFIG_NO_THREADS
#define DeeHashSet_LockReading(x)    rwlock_reading(&((DeeHashSetObject *)Dee_REQUIRES_OBJECT(x))->s_lock)
#define DeeHashSet_LockWriting(x)    rwlock_writing(&((DeeHashSetObject *)Dee_REQUIRES_OBJECT(x))->s_lock)
#define DeeHashSet_LockTryread(x)    rwlock_tryread(&((DeeHashSetObject *)Dee_REQUIRES_OBJECT(x))->s_lock)
#define DeeHashSet_LockTrywrite(x)   rwlock_trywrite(&((DeeHashSetObject *)Dee_REQUIRES_OBJECT(x))->s_lock)
#define DeeHashSet_LockRead(x)       rwlock_read(&((DeeHashSetObject *)Dee_REQUIRES_OBJECT(x))->s_lock)
#define DeeHashSet_LockWrite(x)      rwlock_write(&((DeeHashSetObject *)Dee_REQUIRES_OBJECT(x))->s_lock)
#define DeeHashSet_LockTryUpgrade(x) rwlock_tryupgrade(&((DeeHashSetObject *)Dee_REQUIRES_OBJECT(x))->s_lock)
#define DeeHashSet_LockUpgrade(x)    rwlock_upgrade(&((DeeHashSetObject *)Dee_REQUIRES_OBJECT(x))->s_lock)
#define DeeHashSet_LockDowngrade(x)  rwlock_downgrade(&((DeeHashSetObject *)Dee_REQUIRES_OBJECT(x))->s_lock)
#define DeeHashSet_LockEndWrite(x)   rwlock_endwrite(&((DeeHashSetObject *)Dee_REQUIRES_OBJECT(x))->s_lock)
#define DeeHashSet_LockEndRead(x)    rwlock_endread(&((DeeHashSetObject *)Dee_REQUIRES_OBJECT(x))->s_lock)
#define DeeHashSet_LockEnd(x)        rwlock_end(&((DeeHashSetObject *)Dee_REQUIRES_OBJECT(x))->s_lock)
#else /* !CONFIG_NO_THREADS */
#define DeeHashSet_LockReading(x)          1
#define DeeHashSet_LockWriting(x)          1
#define DeeHashSet_LockTryread(x)          1
#define DeeHashSet_LockTrywrite(x)         1
#define DeeHashSet_LockRead(x)       (void)0
#define DeeHashSet_LockWrite(x)      (void)0
#define DeeHashSet_LockTryUpgrade(x)       1
#define DeeHashSet_LockUpgrade(x)          1
#define DeeHashSet_LockDowngrade(x)  (void)0
#define DeeHashSet_LockEndWrite(x)   (void)0
#define DeeHashSet_LockEndRead(x)    (void)0
#define DeeHashSet_LockEnd(x)        (void)0
#endif /* CONFIG_NO_THREADS */

DECL_END

#endif /* !GUARD_DEEMON_HASHSET_H */
