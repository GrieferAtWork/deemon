/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_DICT_H
#define GUARD_DEEMON_DICT_H 1

#include "api.h"

#include "object.h"

#ifndef CONFIG_NO_THREADS
#include "util/rwlock.h"
#endif /* !CONFIG_NO_THREADS */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_dict_item   dict_item
#define Dee_dict_object dict_object
#endif /* DEE_SOURCE */

typedef struct Dee_dict_object DeeDictObject;

struct Dee_dict_item {
	DREF DeeObject *di_key;   /* [0..1][lock(:d_lock)] Dictionary item key. */
	DREF DeeObject *di_value; /* [1..1|if(di_key == dummy,0..0)][valid_if(di_key)][lock(:d_lock)] Dictionary item value. */
	Dee_hash_t      di_hash;  /* [valid_if(di_key)][lock(:d_lock)] Hash of `di_key' (with a starting value of `0').
	                           *   NOTE: Some random value when `di_key' is the dummy key. */
};

struct Dee_dict_object {
	Dee_OBJECT_HEAD /* GC Object */
	size_t                d_mask; /* [lock(d_lock)][> d_size || d_mask == 0] Allocated dictionary size. */
	size_t                d_size; /* [lock(d_lock)][< d_mask || d_mask == 0] Amount of non-NULL key-item pairs. */
	size_t                d_used; /* [lock(d_lock)][<= d_size] Amount of key-item pairs actually in use.
	                               *  HINT: The difference to `d_size' is the number of dummy keys currently in use. */
	struct Dee_dict_item *d_elem; /* [1..d_size|ALLOC(d_mask+1)][lock(d_lock)]
	                               * [owned_if(!= INTERNAL(empty_dict_items))] Dict key-item pairs (items). */
#ifndef CONFIG_NO_THREADS
	Dee_rwlock_t          d_lock; /* Lock used for accessing this Dict. */
#endif /* !CONFIG_NO_THREADS */
	Dee_WEAKREF_SUPPORT
};

/* The main `Dict' container class (and all related types):
 * >> class Dict: Mapping { ... };
 * >> class Dict.Proxy: sequence { ... };
 * >> class Dict.Keys: Dict.Proxy { ... };
 * >> class Dict.Values: Dict.Proxy { ... };
 * >> class Dict.Items: Dict.Proxy { ... };
 * >> class Dict.Iterator: iterator { ... };
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


#define DeeDict_New()  DeeObject_NewDefault(&DeeDict_Type)
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeDict_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeDict_FromIterator(DeeObject *__restrict self);


#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeDict_GetItemDef(DeeObject *__restrict self, DeeObject *__restrict key, DeeObject *__restrict def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeDict_GetItemString(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
INTDEF WUNUSED DREF DeeObject *DCALL DeeDict_GetItemStringDef(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash, DeeObject *__restrict def);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeDict_GetItemStringLen(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
INTDEF WUNUSED DREF DeeObject *DCALL DeeDict_GetItemStringLenDef(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash, DeeObject *__restrict def);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeDict_DelItemString(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeDict_DelItemStringLen(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
INTDEF int DCALL DeeDict_SetItemString(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash, DeeObject *__restrict value);
INTDEF int DCALL DeeDict_SetItemStringLen(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash, DeeObject *__restrict value);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeDict_HasItemString(DeeObject *__restrict self, char const *__restrict key, Dee_hash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeDict_HasItemStringLen(DeeObject *__restrict self, char const *__restrict key, size_t keylen, Dee_hash_t hash);
#else /* CONFIG_BUILDING_DEEMON */
#define DeeDict_GetItemDef(self, key, def)                        DeeObject_GetItemDef(self, key, def)
#define DeeDict_GetItemString(self, key, hash)                    DeeObject_GetItemString(self, key, hash)
#define DeeDict_GetItemStringDef(self, key, hash, def)            DeeObject_GetItemStringDef(self, key, hash, def)
#define DeeDict_GetItemStringLen(self, key, keylen, hash)         DeeObject_GetItemStringLen(self, key, keylen, hash)
#define DeeDict_GetItemStringLenDef(self, key, keylen, hash, def) DeeObject_GetItemStringLenDef(self, key, keylen, hash, def)
#define DeeDict_DelItemString(self, key, hash)                    DeeObject_DelItemString(self, key, hash)
#define DeeDict_DelItemStringLen(self, key, keylen, hash)         DeeObject_DelItemStringLen(self, key, keylen, hash)
#define DeeDict_SetItemString(self, key, hash, value)             DeeObject_SetItemString(self, key, hash, value)
#define DeeDict_SetItemStringLen(self, key, keylen, hash, value)  DeeObject_SetItemStringLen(self, key, keylen, hash, value)
#endif /* !CONFIG_BUILDING_DEEMON */

/* Create a new Dict by inheriting a set of passed key-item pairs.
 * @param: key_items:    A vector containing `num_keyitems*2' elements,
 *                       even ones being keys and odd ones being items.
 * @param: num_keyitems: The number of key-item pairs passed.
 * WARNING: This function does _NOT_ inherit the passed vector, but _ONLY_ its elements! */
DFUNDEF DREF DeeObject *DCALL
DeeDict_NewKeyItemsInherited(size_t num_keyitems,
                             DREF DeeObject **__restrict key_items);

/* The basic dictionary item lookup algorithm:
 * >> DeeObject *get_item(DeeObject *self, DeeObject *key) {
 * >>     Dee_hash_t i,perturb;
 * >>     Dee_hash_t hash = DeeObject_Hash(0,key);
 * >>     perturb = i = DeeDict_HashSt(self,hash);
 * >>     for (;; i = DeeDict_HashNx(i,perturb),DeeDict_HashPt(perturb)) {
 * >>          struct Dee_dict_item *item = DeeDict_HashIt(self,i);
 * >>          if (!item->di_key) break; // Not found
 * >>          if (item->di_hash != hash) continue; // Non-matching hash
 * >>          if (DeeObject_CompareEq(key,item->di_key))
 * >>              return item->di_item;
 * >>     }
 * >>     return NULL;
 * >> }
 * Requirements to prevent an infinite loop:
 *   - `DeeDict_HashNx()' must be able to eventually enumerate all integers `<= d_mask'
 *   - `d_size' must always be lower than `d_mask', ensuring that at least one(1)
 *      entry exists that no value has been assigned to (Acting as a sentinel to
 *      terminate a search for an existing element).
 *   - `d_elem' must never be empty (or `NULL' for that matter)
 * NOTE: I can't say that I came up with the way that this mapping
 *       algorithm works (but noone can really claim to have invented
 *       something ~new~ nowadays. - It's always been done already).
 *       Yet the point here is, that this is similar to what python
 *       does for its dictionary lookup.
 */
#define DeeDict_HashSt(self, hash)  ((hash) & ((DeeDictObject *)Dee_REQUIRES_OBJECT(self))->d_mask)
#define DeeDict_HashNx(hs, perturb) (((hs) << 2) + (hs) + (perturb) + 1)
#define DeeDict_HashPt(perturb)     ((perturb) >>= 5) /* This `5' is tunable. */
#define DeeDict_HashIt(self, i)     (((DeeDictObject *)Dee_REQUIRES_OBJECT(self))->d_elem + ((i) & ((DeeDictObject *)(self))->d_mask))



#ifndef CONFIG_NO_THREADS
#define DeeDict_LockReading(x)    Dee_rwlock_reading(&((DeeDictObject *)Dee_REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockWriting(x)    Dee_rwlock_writing(&((DeeDictObject *)Dee_REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockTryread(x)    Dee_rwlock_tryread(&((DeeDictObject *)Dee_REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockTrywrite(x)   Dee_rwlock_trywrite(&((DeeDictObject *)Dee_REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockRead(x)       Dee_rwlock_read(&((DeeDictObject *)Dee_REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockWrite(x)      Dee_rwlock_write(&((DeeDictObject *)Dee_REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockTryUpgrade(x) Dee_rwlock_tryupgrade(&((DeeDictObject *)Dee_REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockUpgrade(x)    Dee_rwlock_upgrade(&((DeeDictObject *)Dee_REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockDowngrade(x)  Dee_rwlock_downgrade(&((DeeDictObject *)Dee_REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockEndWrite(x)   Dee_rwlock_endwrite(&((DeeDictObject *)Dee_REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockEndRead(x)    Dee_rwlock_endread(&((DeeDictObject *)Dee_REQUIRES_OBJECT(x))->d_lock)
#define DeeDict_LockEnd(x)        Dee_rwlock_end(&((DeeDictObject *)Dee_REQUIRES_OBJECT(x))->d_lock)
#else /* CONFIG_NO_THREADS */
#define DeeDict_LockReading(x)          1
#define DeeDict_LockWriting(x)          1
#define DeeDict_LockTryread(x)          1
#define DeeDict_LockTrywrite(x)         1
#define DeeDict_LockRead(x)       (void)0
#define DeeDict_LockWrite(x)      (void)0
#define DeeDict_LockTryUpgrade(x)       1
#define DeeDict_LockUpgrade(x)          1
#define DeeDict_LockDowngrade(x)  (void)0
#define DeeDict_LockEndWrite(x)   (void)0
#define DeeDict_LockEndRead(x)    (void)0
#define DeeDict_LockEnd(x)        (void)0
#endif /* !CONFIG_NO_THREADS */

DECL_END

#endif /* !GUARD_DEEMON_DICT_H */
