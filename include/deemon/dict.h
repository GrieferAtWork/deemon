/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_DICT_H
#define GUARD_DEEMON_DICT_H 1

#include "api.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include "object.h"
#include "util/lock.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_dict_item   dict_item
#define Dee_dict_object dict_object
#endif /* DEE_SOURCE */

typedef struct Dee_dict_object DeeDictObject;

struct Dee_dict_item {
	DREF DeeObject *di_key;   /* [0..1][lock(:d_lock)] Dictionary item key. */
	DREF DeeObject *di_value; /* [1..1|if(di_key == dummy, 0..0)][valid_if(di_key)][lock(:d_lock)] Dictionary item value. */
	Dee_hash_t      di_hash;  /* [valid_if(di_key)][lock(:d_lock)] Hash of `di_key' (with a starting value of `0').
	                           * NOTE: Some random value when `di_key' is the dummy key. */
};

struct Dee_dict_object {
	Dee_OBJECT_HEAD /* GC Object */
	size_t                d_mask; /* [lock(d_lock)][> d_size || d_mask == 0] Allocated dictionary size. */
	size_t                d_size; /* [lock(d_lock)][< d_mask || d_mask == 0] Amount of non-NULL key-item pairs. */
	size_t                d_used; /* [lock(d_lock)][<= d_size] Amount of key-item pairs actually in use.
	                               * HINT: The difference to `d_size' is the number of dummy keys currently in use. */
	struct Dee_dict_item *d_elem; /* [1..d_size|ALLOC(d_mask+1)][lock(d_lock)]
	                               * [owned_if(!= INTERNAL(empty_dict_items))] Dict key-item pairs (items). */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t   d_lock; /* Lock used for accessing this Dict. */
#endif /* !CONFIG_NO_THREADS */
	Dee_WEAKREF_SUPPORT
};

#ifdef CONFIG_NO_THREADS
#define Dee_DICT_INIT \
	{ Dee_OBJECT_HEAD_INIT(&DeeDict_Type), 0, 0, 0, (struct Dee_dict_item *)DeeDict_EmptyItems, Dee_WEAKREF_SUPPORT_INIT }
#else /* CONFIG_NO_THREADS */
#define Dee_DICT_INIT \
	{ Dee_OBJECT_HEAD_INIT(&DeeDict_Type), 0, 0, 0, (struct Dee_dict_item *)DeeDict_EmptyItems, DEE_ATOMIC_RWLOCK_INIT, Dee_WEAKREF_SUPPORT_INIT }
#endif /* !CONFIG_NO_THREADS */

/* The main `Dict' container class (and all related types):
 * >> class Dict: Mapping { ... };
 * >> class Dict.Proxy: Sequence { ... };
 * >> class Dict.Keys: Dict.Proxy { ... };
 * >> class Dict.Values: Dict.Proxy { ... };
 * >> class Dict.Items: Dict.Proxy { ... };
 * >> class Dict.Iterator: Iterator { ... };
 * >> class Dict.Proxy.Iterator: Dict.Iterator { ... };
 * >> class Dict.Keys.Iterator: Dict.Proxy.Iterator { ... };
 * >> class Dict.Values.Iterator: Dict.Proxy.Iterator { ... };
 * >> class Dict.Items.Iterator: Dict.Proxy.Iterator { ... };
 */
DDATDEF DeeTypeObject DeeDict_Type;
#define DeeDict_Check(ob)         DeeObject_InstanceOf(ob, &DeeDict_Type)
#define DeeDict_CheckExact(ob)    DeeObject_InstanceOfExact(ob, &DeeDict_Type)

/* Dict proxy types (implemented in `/src/deemon/objects/dictproxy.c') */
DDATDEF DeeTypeObject DeeDictProxy_Type;
DDATDEF DeeTypeObject DeeDictKeys_Type;
DDATDEF DeeTypeObject DeeDictItems_Type;
DDATDEF DeeTypeObject DeeDictValues_Type;

/* A dummy object used by Dict and HashSet to refer to
 * deleted keys that are still apart of the hash chain.
 * Only here to allow dex modules to correct work
 * DON'T USE THIS OBJECT AS KEY FOR DICTS OR HASHSETS!
 * DO NOT EXPOSE THIS OBJECT TO USER-CODE! (not even in `rt'!) */
DDATDEF DeeObject DeeDict_Dummy;
DDATDEF struct Dee_dict_item const DeeDict_EmptyItems[1];


#define DeeDict_New()  DeeObject_NewDefault(&DeeDict_Type)
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeDict_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeDict_FromIterator(DeeObject *__restrict self);


#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeDict_GetItemDef(DeeObject *self, DeeObject *key, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeDict_GetItemString(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL DeeDict_GetItemStringDef(DeeObject *self, char const *__restrict key, Dee_hash_t hash, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeDict_GetItemStringLen(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeDict_GetItemStringLenDef(DeeObject *self, char const *__restrict key, size_t keylen, Dee_hash_t hash, DeeObject *def);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeDict_DelItemString(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeDict_DelItemStringLen(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeDict_SetItemString(DeeObject *self, char const *__restrict key, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeDict_SetItemStringLen(DeeObject *self, char const *__restrict key, size_t keylen, Dee_hash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeDict_HasItemString(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeDict_HasItemStringLen(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeDict_ByHash(DeeObject *__restrict self, Dee_hash_t hash, bool key_only);
#else /* CONFIG_BUILDING_DEEMON */
#define DeeDict_GetItemDef(self, key, def)                        DeeObject_GetItemDef(self, key, def)
#define DeeDict_GetItemString(self, key, hash)                    DeeObject_GetItemStringHash(self, key, hash)
#define DeeDict_GetItemStringDef(self, key, hash, def)            DeeObject_GetItemStringHashDef(self, key, hash, def)
#define DeeDict_GetItemStringLen(self, key, keylen, hash)         DeeObject_GetItemStringLenHash(self, key, keylen, hash)
#define DeeDict_GetItemStringLenDef(self, key, keylen, hash, def) DeeObject_GetItemStringLenHashDef(self, key, keylen, hash, def)
#define DeeDict_DelItemString(self, key, hash)                    DeeObject_DelItemStringHash(self, key, hash)
#define DeeDict_DelItemStringLen(self, key, keylen, hash)         DeeObject_DelItemStringLenHash(self, key, keylen, hash)
#define DeeDict_SetItemString(self, key, hash, value)             DeeObject_SetItemStringHash(self, key, hash, value)
#define DeeDict_SetItemStringLen(self, key, keylen, hash, value)  DeeObject_SetItemStringLenHash(self, key, keylen, hash, value)
#endif /* !CONFIG_BUILDING_DEEMON */
#define DeeDict_GetItem(self, key)        DeeObject_GetItem(self, key)
#define DeeDict_DelItem(self, key)        DeeObject_DelItem(self, key)
#define DeeDict_SetItem(self, key, value) DeeObject_SetItem(self, key, value)
#define DeeDict_Clear(self)               ((*DeeDict_Type.tp_gc->tp_clear)(self))

/* Create a new Dict by inheriting a set of passed key-item pairs.
 * @param: key_items:    A vector containing `num_keyitems*2' elements,
 *                       even ones being keys and odd ones being items.
 * @param: num_keyitems: The number of key-item pairs passed.
 * WARNING: This function does _NOT_ inherit the passed vector, but _ONLY_ its elements! */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeDict_NewKeyItemsInherited(size_t num_keyitems,
                             DREF DeeObject **key_items);

/* The basic dictionary item lookup algorithm:
 * >> DeeObject *get_item(DeeObject *self, DeeObject *key) {
 * >>     Dee_hash_t i, perturb;
 * >>     Dee_hash_t hash = DeeObject_Hash(0, key);
 * >>     perturb = i = DeeDict_HashSt(self, hash);
 * >>     for (;; i = DeeDict_HashNx(i, perturb), DeeDict_HashPt(perturb)) {
 * >>          struct Dee_dict_item *item = DeeDict_HashIt(self, i);
 * >>          if (!item->di_key)
 * >>              break; // Not found
 * >>          if (item->di_hash != hash)
 * >>              continue; // Non-matching hash
 * >>          if (DeeObject_CompareEq(key, item->di_key))
 * >>              return item->di_item;
 * >>     }
 * >>     return NULL;
 * >> }
 * Requirements to prevent an infinite loop:
 * - `DeeDict_HashNx()' must be able to eventually enumerate all integers `<= d_mask'
 * - `d_size' must always be lower than `d_mask', ensuring that at least one(1)
 *   entry exists that no value has been assigned to (Acting as a sentinel to
 *   terminate a search for an existing element).
 * - `d_elem' must never be empty (or `NULL' for that matter)
 * NOTE: I can't say that I came up with the way that this mapping
 *       algorithm works (but noone can really claim to have invented
 *       something ~new~ nowadays. - It's always been done already).
 *       Yet the point here is, that this is similar to what python
 *       does for its dictionary lookup.
 */
#define DeeDict_HashSt(self, hash)  ((hash) & (self)->d_mask)
#define DeeDict_HashNx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define DeeDict_HashIt(self, i)     ((self)->d_elem + ((i) & (self)->d_mask))


/* Locking helpers for `DeeDictObject' */
#define DeeDict_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->d_lock)
#define DeeDict_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->d_lock)
#define DeeDict_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->d_lock)
#define DeeDict_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->d_lock)
#define DeeDict_LockRead(self)       Dee_atomic_rwlock_read(&(self)->d_lock)
#define DeeDict_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->d_lock)
#define DeeDict_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->d_lock)
#define DeeDict_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->d_lock)
#define DeeDict_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->d_lock)
#define DeeDict_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->d_lock)
#define DeeDict_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->d_lock)
#define DeeDict_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->d_lock)

DECL_END

#endif /* !GUARD_DEEMON_DICT_H */
