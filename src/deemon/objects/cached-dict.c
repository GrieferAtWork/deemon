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
#ifndef GUARD_DEEMON_OBJECTS_CACHED_DICT_C
#define GUARD_DEEMON_OBJECTS_CACHED_DICT_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_CollectMemory, Dee_CollectMemoryc, Dee_Free, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, Dee_TryCallocc, Dee_TryMallocc */
#include <deemon/arg.h>                /* DeeArg_Unpack1 */
#include <deemon/bool.h>               /* return_bool */
#include <deemon/cached-dict.h>        /* DeeCachedDict*, Dee_cached_dict_item */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/dict.h>               /* DeeDict_CheckExact */
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/format.h>             /* DeeFormat_Printf */
#include <deemon/gc.h>                 /* DeeGCObject_MALLOC, DeeGC_TRACK */
#include <deemon/map.h>                /* DeeMapping_NewEmpty, DeeMapping_Type */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_*, Dee_COMPARE_ISEQ, Dee_COMPARE_ISERR, Dee_Decref, Dee_Decref_unlikely, Dee_HAS_*, Dee_Incref, Dee_XDecref_unlikely, Dee_XIncref, Dee_foreach_pair_t, Dee_foreach_t, Dee_formatprinter_t, Dee_hash_t, Dee_ssize_t, Dee_visit_t, ITER_DONE, ITER_ISOK, OBJECT_HEAD_INIT */
#include <deemon/serial.h>             /* DeeSerial*, Dee_SERADDR_ISOK, Dee_seraddr_t */
#include <deemon/string.h>             /* DeeString* */
#include <deemon/system-features.h>    /* DeeSystem_DEFINE_strcmp, memcpyc */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, Dee_Visit, TF_KW, TP_FGC, TP_FNORMAL, type_* */
#include <deemon/util/lock.h>          /* Dee_atomic_rwlock_init */

#include <hybrid/sched/yield.h> /* SCHED_YIELD */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */

DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

typedef DeeCachedDictObject CachedDict;

#define empty_cdict_items ((struct Dee_cached_dict_item *)DeeCachedDict_EmptyItems)



/************************************************************************/
/* CACHED DICT                                                          */
/************************************************************************/
PRIVATE WUNUSED NONNULL((1)) int DCALL
cdict_ctor(CachedDict *__restrict self) {
	self->cd_mask = 0;
	self->cd_size = 0;
	self->cd_map  = DeeMapping_NewEmpty();
	self->cd_elem = empty_cdict_items;
	Dee_atomic_rwlock_init(&self->cd_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cdict_copy(CachedDict *__restrict self,
           CachedDict *__restrict other) {
	struct Dee_cached_dict_item *iter, *end;
	Dee_atomic_rwlock_init(&self->cd_lock);
again:
	DeeCachedDict_LockRead(other);
	self->cd_mask = other->cd_mask;
	self->cd_size = other->cd_size;
	if ((self->cd_elem = other->cd_elem) != empty_cdict_items) {
		self->cd_elem = (struct Dee_cached_dict_item *)Dee_TryMallocc(other->cd_mask + 1,
		                                                              sizeof(struct Dee_cached_dict_item));
		if unlikely(!self->cd_elem) {
			DeeCachedDict_LockEndRead(other);
			if (Dee_CollectMemoryc(other->cd_mask + 1,
			                       sizeof(struct Dee_cached_dict_item)))
				goto again;
			goto err;
		}
		memcpyc(self->cd_elem, other->cd_elem,
		        self->cd_mask + 1,
		        sizeof(struct Dee_cached_dict_item));
		end = (iter = self->cd_elem) + (self->cd_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->cdi_key)
				continue;
			Dee_Incref(iter->cdi_key);
			Dee_XIncref(iter->cdi_value);
		}
	}
	DeeCachedDict_LockEndRead(other);
	self->cd_map = other->cd_map;
	Dee_Incref(self->cd_map);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
cdict_fini(CachedDict *__restrict self) {
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_mask == 0));
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_size == 0));
	if (self->cd_elem != empty_cdict_items) {
		struct Dee_cached_dict_item *iter, *end;
		end = (iter = self->cd_elem) + (self->cd_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->cdi_key)
				continue;
			Dee_Decref_unlikely(iter->cdi_key);
			Dee_XDecref_unlikely(iter->cdi_value);
		}
		Dee_Free(self->cd_elem);
	}
	Dee_Decref(self->cd_map);
}

PRIVATE NONNULL((1)) void DCALL
cdict_clear(CachedDict *__restrict self) {
	/* FIXME: CachedDict implementing "operator clear()" is a problem:
	 * - It is needed because CachedDict is a GC object
	 * - CachedDict must be a GC object because it can potentially reference itself (see code example below)
	 * - It is possible to trigger "CachedDict.operator clear()" while there are still NOREF arguments
	 * So in other words:
	 * - We get problems by implementing it (breaking the invariant that CachedDict keeps objects alive during the call)
	 * - And we'd get problems if we didn't implement it (reference loops that can't be resolved)
	 *
	 * see also: "util/test/deemon-kwcall-reference-loophole.dee"
	 */
	struct Dee_cached_dict_item *elem;
	size_t mask;
	DeeCachedDict_LockWrite(self);
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_mask == 0));
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_size == 0));

	/* Extract the vector and mask. */
	elem          = self->cd_elem;
	mask          = self->cd_mask;
	self->cd_elem = empty_cdict_items;
	self->cd_mask = 0;
	self->cd_size = 0;
	DeeCachedDict_LockEndWrite(self);

	/* Destroy the vector. */
	if (elem != empty_cdict_items) {
		struct Dee_cached_dict_item *iter, *end;
		end = (iter = elem) + (mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->cdi_key)
				continue;
			Dee_Decref(iter->cdi_key);
			Dee_Decref(iter->cdi_value);
		}
		Dee_Free(elem);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
cdict_visit(CachedDict *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->cd_map);
	DeeCachedDict_LockRead(self);
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_mask == 0));
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_size == 0));
	if (self->cd_elem != empty_cdict_items) {
		struct Dee_cached_dict_item *iter, *end;
		end = (iter = self->cd_elem) + (self->cd_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->cdi_key)
				continue;
			/* Visit all keys and associated values. */
			Dee_Visit(iter->cdi_key);
			Dee_Visit(iter->cdi_value);
		}
	}
	DeeCachedDict_LockEndRead(self);
}


/* Resize the hash size by a factor of 2 and re-insert all elements.
 * @return: true:  Successfully rehashed the CachedDict.
 * @return: false: Not enough memory. - The caller should collect some and try again. */
PRIVATE NONNULL((1)) bool DCALL
dict_rehash(CachedDict *__restrict self) {
	struct Dee_cached_dict_item *new_vector, *iter, *end;
	size_t new_mask;
	new_mask = self->cd_mask;
	new_mask = (new_mask << 1) | 1;
	if unlikely(new_mask == 1)
		new_mask = 16 - 1; /* Start out bigger than 2. */
	ASSERT(self->cd_size < new_mask);
	new_vector = (struct Dee_cached_dict_item *)Dee_TryCallocc(new_mask + 1, sizeof(struct Dee_cached_dict_item));
	if unlikely(!new_vector)
		return false;
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_mask == 0));
	ASSERT((self->cd_elem == empty_cdict_items) == (self->cd_size == 0));
	if (self->cd_elem != empty_cdict_items) {
		/* Re-insert all existing items into the new CachedDict vector. */
		end = (iter = self->cd_elem) + (self->cd_mask + 1);
		for (; iter < end; ++iter) {
			struct Dee_cached_dict_item *item;
			Dee_hash_t i, perturb;

			/* Skip dummy keys. */
			if (!iter->cdi_key)
				continue;
			perturb = i = iter->cdi_hash & new_mask;
			for (;; DeeCachedDict_HashNx(i, perturb)) {
				item = &new_vector[i & new_mask];
				if (!item->cdi_key)
					break; /* Empty slot found. */
			}

			/* Transfer this object. */
			item->cdi_key   = iter->cdi_key;
			item->cdi_hash  = iter->cdi_hash;
			item->cdi_value = iter->cdi_value;
		}
		Dee_Free(self->cd_elem);
	}
	self->cd_mask = new_mask;
	self->cd_elem = new_vector;
	return true;
}


/* Insert "value" into the cache for "key" */
PRIVATE WUNUSED NONNULL((1, 2, 3)) DeeObject *DCALL
DeeCachedDict_Remember(DeeCachedDictObject *__restrict self,
                       /*inherit(always)*/ DREF DeeObject *key,
                       /*inherit(always)*/ DREF DeeObject *value, Dee_hash_t hash) {
	size_t mask;
	struct Dee_cached_dict_item *vector;
	int error;
	struct Dee_cached_dict_item *first_dummy;
	Dee_hash_t i, perturb;
again_lock:
	DeeCachedDict_LockRead(self);
again:
	first_dummy = NULL;
	vector      = self->cd_elem;
	mask        = self->cd_mask;
	perturb = i = hash & mask;
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		DeeObject *item_key;
		struct Dee_cached_dict_item *item = &vector[i & mask];
		if (!item->cdi_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		item_key = item->cdi_key;
		Dee_Incref(item_key);
		DeeCachedDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (Dee_COMPARE_ISERR(error))
			goto err; /* Error in compare operator. */
		if (Dee_COMPARE_ISEQ(error)) {
			DREF DeeObject *item_value;
			/* Found an existing item. */
			DeeCachedDict_LockWrite(self);
			/* Check if the CachedDict was modified. */
			if (self->cd_elem != vector ||
			    self->cd_mask != mask ||
			    item->cdi_key != item_key) {
				DeeCachedDict_LockDowngrade(self);
				goto again;
			}
			item_value = item->cdi_value;
			item->cdi_key   = key;
			item->cdi_value = value;
			DeeCachedDict_LockEndWrite(self);
			Dee_Decref(item_key);
			Dee_Decref(item_value);
			return value;
		}
		DeeCachedDict_LockRead(self);
		/* Check if the CachedDict was modified. */
		if (self->cd_elem != vector ||
		    self->cd_mask != mask ||
		    item->cdi_key != item_key)
			goto again;
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeCachedDict_LockUpgrade(self)) {
		DeeCachedDict_LockEndWrite(self);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if ((first_dummy != NULL) &&
	    (self->cd_size + 1 < self->cd_mask ||
	     first_dummy->cdi_key != NULL)) {
		ASSERT(first_dummy != empty_cdict_items);
		ASSERT(!first_dummy->cdi_key);

		/* Fill in the target slot. */
		first_dummy->cdi_key   = key; /* Inherit */
		first_dummy->cdi_hash  = hash;
		first_dummy->cdi_value = value; /* Inherit */
		++self->cd_size;

		/* Try to keep the CachedDict vector big at least twice as big as the element count. */
		if (self->cd_size * 2 > self->cd_mask)
			dict_rehash(self);
		DeeCachedDict_LockEndWrite(self);
		return value;
	}

	/* Rehash the CachedDict and try again. */
	if (dict_rehash(self)) {
		DeeCachedDict_LockDowngrade(self);
		goto again;
	}
	DeeCachedDict_LockEndWrite(self);
	if (Dee_CollectMemory(1))
		goto again_lock;
err:
	Dee_Decref(key);
	Dee_Decref(value);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeCachedDict_LookupAndRemember(DeeCachedDictObject *__restrict self,
                                DeeObject *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_GetItem(self->cd_map, key);
	if likely(result) {
		Dee_Incref(key);
		result = DeeCachedDict_Remember(self, key, result, hash);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeCachedDict_LookupAndRememberStringHash(DeeCachedDictObject *__restrict self,
                                          char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	DeeObject *basemap = self->cd_map;
	if (DeeDict_CheckExact(basemap)) {
		/* TODO: Special optimization here so "keyob" will be shared with the underlying dict */
	}
	result = DeeObject_GetItemStringHash(basemap, key, hash);
	if likely(result) {
		DREF DeeObject *keyob;
		keyob = DeeString_NewWithHash(key, hash);
		if unlikely(!keyob)
			goto err_r;
		result = DeeCachedDict_Remember(self, keyob, result, hash);
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeCachedDict_LookupAndRememberStringLenHash(DeeCachedDictObject *__restrict self,
                                             char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	DeeObject *basemap = self->cd_map;
	if (DeeDict_CheckExact(basemap)) {
		/* TODO: Special optimization here so "keyob" will be shared with the underlying dict */
	}
	result = DeeObject_GetItemStringLenHash(basemap, key, keylen, hash);
	if likely(result) {
		DREF DeeObject *keyob;
		keyob = DeeString_NewSizedWithHash(key, keylen, hash);
		if unlikely(!keyob)
			goto err_r;
		result = DeeCachedDict_Remember(self, keyob, result, hash);
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeCachedDict_TryLookupAndRememberDef(DeeCachedDictObject *__restrict self,
                                      DeeObject *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_TryGetItem(self->cd_map, key);
	if likely(result) {
		if (result == ITER_DONE)
			return result;
		Dee_Incref(key);
		result = DeeCachedDict_Remember(self, key, result, hash);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeCachedDict_TryLookupAndRememberStringHash(DeeCachedDictObject *__restrict self,
                                             char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	DeeObject *basemap = self->cd_map;
	if (DeeDict_CheckExact(basemap)) {
		/* TODO: Special optimization here so "keyob" will be shared with the underlying dict */
	}
	result = DeeObject_TryGetItemStringHash(basemap, key, hash);
	if likely(result) {
		DREF DeeObject *keyob;
		if (result == ITER_DONE)
			return result;
		keyob = DeeString_NewWithHash(key, hash);
		if unlikely(!keyob)
			goto err_r;
		result = DeeCachedDict_Remember(self, keyob, result, hash);
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
DeeCachedDict_TryLookupAndRememberStringLenHash(DeeCachedDictObject *__restrict self,
                                                char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	DeeObject *basemap = self->cd_map;
	if (DeeDict_CheckExact(basemap)) {
		/* TODO: Special optimization here so "keyob" will be shared with the underlying dict */
	}
	result = DeeObject_TryGetItemStringLenHash(basemap, key, keylen, hash);
	if likely(result) {
		DREF DeeObject *keyob;
		if (result == ITER_DONE)
			return result;
		keyob = DeeString_NewSizedWithHash(key, keylen, hash);
		if unlikely(!keyob)
			goto err_r;
		result = DeeCachedDict_Remember(self, keyob, result, hash);
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
cdict_getitemnr(DeeCachedDictObject *__restrict self,
                /*string*/ DeeObject *__restrict key) {
	size_t mask;
	struct Dee_cached_dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeObject_Hash(key);
	DeeCachedDict_LockRead(self);
restart:
	vector  = self->cd_elem;
	mask    = self->cd_mask;
	perturb = i = hash & mask;
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		DeeObject *item_key, *item_value;
		struct Dee_cached_dict_item *item = &vector[i & mask];
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		item_key   = item->cdi_key;
		item_value = item->cdi_value;
		DeeCachedDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		if (Dee_COMPARE_ISERR(error))
			goto err; /* Error in compare operator. */
		if (Dee_COMPARE_ISEQ(error))
			return item_value; /* Found the item. */
		DeeCachedDict_LockRead(self);

		/* Check if the CachedDict was modified. */
		if (self->cd_elem != vector ||
		    self->cd_mask != mask ||
		    item->cdi_key != item_key ||
		    item->cdi_value != item_value)
			goto restart;
	}
	DeeCachedDict_LockEndRead(self);
	return DeeCachedDict_LookupAndRemember(self, key, hash);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
cdict_getitemnr_string_hash(DeeCachedDictObject *__restrict self,
                            char const *__restrict key,
                            Dee_hash_t hash) {
	DeeObject *result;
	Dee_hash_t i, perturb;
	DeeCachedDict_LockRead(self);
	perturb = i = DeeCachedDict_HashSt(self, hash);
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		struct Dee_cached_dict_item *item = DeeCachedDict_HashIt(self, i);
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->cdi_key))
			continue;
		if (strcmp(DeeString_STR(item->cdi_key), key) == 0) {
			result = item->cdi_value;
			DeeCachedDict_LockEndRead(self);
			return result;
		}
	}
	DeeCachedDict_LockEndRead(self);
	return DeeCachedDict_LookupAndRememberStringHash(self, key, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
cdict_getitemnr_string_len_hash(DeeCachedDictObject *__restrict self,
                                char const *__restrict key,
                                size_t keylen, Dee_hash_t hash) {
	DeeObject *result;
	Dee_hash_t i, perturb;
	DeeCachedDict_LockRead(self);
	perturb = i = DeeCachedDict_HashSt(self, hash);
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		struct Dee_cached_dict_item *item = DeeCachedDict_HashIt(self, i);
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->cdi_key))
			continue;
		if (DeeString_EqualsBuf(item->cdi_key, key, keylen)) {
			result = item->cdi_value;
			DeeCachedDict_LockEndRead(self);
			return result;
		}
	}
	DeeCachedDict_LockEndRead(self);
	return DeeCachedDict_LookupAndRememberStringLenHash(self, key, keylen, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
cdict_trygetitemnr(DeeCachedDictObject *__restrict self,
                   /*string*/ DeeObject *__restrict key) {
	size_t mask;
	struct Dee_cached_dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeObject_Hash(key);
	DeeCachedDict_LockRead(self);
restart:
	vector  = self->cd_elem;
	mask    = self->cd_mask;
	perturb = i = hash & mask;
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		DeeObject *item_key, *item_value;
		struct Dee_cached_dict_item *item = &vector[i & mask];
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		item_key   = item->cdi_key;
		item_value = item->cdi_value;
		DeeCachedDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		if (Dee_COMPARE_ISERR(error))
			goto err; /* Error in compare operator. */
		if (Dee_COMPARE_ISEQ(error))
			return item_value; /* Found the item. */
		DeeCachedDict_LockRead(self);

		/* Check if the CachedDict was modified. */
		if (self->cd_elem != vector ||
		    self->cd_mask != mask ||
		    item->cdi_key != item_key ||
		    item->cdi_value != item_value)
			goto restart;
	}
	DeeCachedDict_LockEndRead(self);
	return DeeCachedDict_TryLookupAndRememberDef(self, key, hash);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
cdict_trygetitemnr_string_hash(DeeCachedDictObject *__restrict self,
                               char const *__restrict key, Dee_hash_t hash) {
	DeeObject *result;
	Dee_hash_t i, perturb;
	DeeCachedDict_LockRead(self);
	perturb = i = DeeCachedDict_HashSt(self, hash);
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		struct Dee_cached_dict_item *item = DeeCachedDict_HashIt(self, i);
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->cdi_key))
			continue;
		if (strcmp(DeeString_STR(item->cdi_key), key) == 0) {
			result = item->cdi_value;
			DeeCachedDict_LockEndRead(self);
			return result;
		}
	}
	DeeCachedDict_LockEndRead(self);
	return DeeCachedDict_TryLookupAndRememberStringHash(self, key, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL
cdict_trygetitemnr_string_len_hash(DeeCachedDictObject *__restrict self,
                                   char const *__restrict key,
                                   size_t keylen, Dee_hash_t hash) {
	DeeObject *result;
	Dee_hash_t i, perturb;
	DeeCachedDict_LockRead(self);
	perturb = i = DeeCachedDict_HashSt(self, hash);
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		struct Dee_cached_dict_item *item = DeeCachedDict_HashIt(self, i);
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->cdi_key))
			continue;
		if (DeeString_EqualsBuf(item->cdi_key, key, keylen)) {
			result = item->cdi_value;
			DeeCachedDict_LockEndRead(self);
			return result;
		}
	}
	DeeCachedDict_LockEndRead(self);
	return DeeCachedDict_TryLookupAndRememberStringLenHash(self, key, keylen, hash);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cdict_iscached(DeeCachedDictObject *self, DeeObject *key, Dee_hash_t hash) {
	size_t mask;
	struct Dee_cached_dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	DeeCachedDict_LockRead(self);
restart:
	vector  = self->cd_elem;
	mask    = self->cd_mask;
	perturb = i = hash & mask;
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		DeeObject *item_key;
		struct Dee_cached_dict_item *item = &vector[i & mask];
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		item_key = item->cdi_key;
		DeeCachedDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		if (Dee_COMPARE_ISERR(error))
			goto err; /* Error in compare operator. */
		if (Dee_COMPARE_ISEQ(error))
			return Dee_HAS_YES; /* Found the item. */
		DeeCachedDict_LockRead(self);

		/* Check if the CachedDict was modified. */
		if (self->cd_elem != vector ||
		    self->cd_mask != mask ||
		    item->cdi_key != item_key)
			goto restart;
	}
	DeeCachedDict_LockEndRead(self);
	return Dee_HAS_NO;
err:
	return Dee_HAS_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
cdict_iscached_string_hash(DeeCachedDictObject *self, char const *key, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	DeeCachedDict_LockRead(self);
	perturb = i = DeeCachedDict_HashSt(self, hash);
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		struct Dee_cached_dict_item *item = DeeCachedDict_HashIt(self, i);
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->cdi_key))
			continue;
		if (strcmp(DeeString_STR(item->cdi_key), key) == 0) {
			DeeCachedDict_LockEndRead(self);
			return true;
		}
	}
	DeeCachedDict_LockEndRead(self);
	return false;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
cdict_iscached_string_len_hash(DeeCachedDictObject *self, char const *key,
                               size_t keylen, Dee_hash_t hash) {
	Dee_hash_t i, perturb;
	DeeCachedDict_LockRead(self);
	perturb = i = DeeCachedDict_HashSt(self, hash);
	for (;; DeeCachedDict_HashNx(i, perturb)) {
		struct Dee_cached_dict_item *item = DeeCachedDict_HashIt(self, i);
		if (!item->cdi_key)
			break; /* Not found */
		if (item->cdi_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->cdi_key))
			continue;
		if (DeeString_EqualsBuf(item->cdi_key, key, keylen)) {
			DeeCachedDict_LockEndRead(self);
			return true;
		}
	}
	DeeCachedDict_LockEndRead(self);
	return false;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cdict_sizeob(CachedDict *__restrict self) {
	return DeeObject_SizeOb(self->cd_map);
}

/* This one's basically your hasitem operator. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_contains(CachedDict *self, DeeObject *key) {
	DeeObject *value = cdict_trygetitemnr(self, key);
	if unlikely(!value)
		goto err;
	return_bool(value != ITER_DONE);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cdict_size(CachedDict *__restrict self) {
	return DeeObject_Size(self->cd_map);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cdict_size_fast(CachedDict *__restrict self) {
	return DeeObject_SizeFast(self->cd_map);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cdict_printrepr(CachedDict *__restrict self,
                Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "CachedDict(%r)", self->cd_map);
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
cdict_hash(CachedDict *__restrict self) {
	return DeeObject_Hash(self->cd_map);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cdict_compare_eq(CachedDict *self, DeeObject *other) {
	return DeeObject_CompareEq(self->cd_map, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cdict_compare(CachedDict *self, DeeObject *other) {
	return DeeObject_Compare(self->cd_map, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cdict_trycompare_eq(CachedDict *self, DeeObject *other) {
	return DeeObject_TryCompareEq(self->cd_map, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_eq(CachedDict *self, DeeObject *other) {
	return DeeObject_CmpEq(self->cd_map, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_ne(CachedDict *self, DeeObject *other) {
	return DeeObject_CmpNe(self->cd_map, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_lo(CachedDict *self, DeeObject *other) {
	return DeeObject_CmpLo(self->cd_map, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_le(CachedDict *self, DeeObject *other) {
	return DeeObject_CmpLe(self->cd_map, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_gr(CachedDict *self, DeeObject *other) {
	return DeeObject_CmpGr(self->cd_map, other);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_ge(CachedDict *self, DeeObject *other) {
	return DeeObject_CmpGe(self->cd_map, other);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cdict_iter(CachedDict *__restrict self) {
	return DeeObject_Iter(self->cd_map);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cdict_foreach(CachedDict *self, Dee_foreach_t proc, void *arg) {
	return DeeObject_Foreach(self->cd_map, proc, arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cdict_foreach_pair(CachedDict *self, Dee_foreach_pair_t proc, void *arg) {
	return DeeObject_ForeachPair(self->cd_map, proc, arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_getitem(CachedDict *self, DeeObject *key) {
	DeeObject *result;
	result = cdict_getitemnr(self, key);
	Dee_XIncref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_getitem_string_hash(CachedDict *self, char const *key, Dee_hash_t hash) {
	DeeObject *result;
	result = cdict_getitemnr_string_hash(self, key, hash);
	Dee_XIncref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_getitem_string_len_hash(CachedDict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DeeObject *result;
	result = cdict_getitemnr_string_len_hash(self, key, keylen, hash);
	Dee_XIncref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_trygetitem(CachedDict *self, DeeObject *key) {
	DeeObject *result;
	result = cdict_trygetitemnr(self, key);
	if (ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_trygetitem_string_hash(CachedDict *self, char const *key, Dee_hash_t hash) {
	DeeObject *result;
	result = cdict_trygetitemnr_string_hash(self, key, hash);
	if (ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cdict_trygetitem_string_len_hash(CachedDict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DeeObject *result;
	result = cdict_trygetitemnr_string_len_hash(self, key, keylen, hash);
	if (ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cdict_bounditem(CachedDict *self, DeeObject *key) {
	DREF DeeObject *value;
	Dee_hash_t hash = DeeObject_Hash(key);
	int result = cdict_iscached(self, key, hash);
	if unlikely(Dee_HAS_ISERR(result))
		goto err;
	if (result)
		return Dee_BOUND_YES;
	value = DeeObject_GetItem(self->cd_map, key);
	if (value) {
		Dee_Incref(key);
		if (!DeeCachedDict_Remember(self, key, value, hash))
			goto err;
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError))
		return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cdict_bounditem_string_hash(CachedDict *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *value;
	if (cdict_iscached_string_hash(self, key, hash))
		return Dee_BOUND_YES;
	value = DeeObject_GetItemStringHash(self->cd_map, key, hash);
	if (value) {
		DREF DeeObject *keyob;
		keyob = DeeString_NewWithHash(key, hash);
		if unlikely(!keyob) {
			Dee_Decref(value);
			goto err;
		}
		if (!DeeCachedDict_Remember(self, keyob, value, hash))
			goto err;
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError))
		return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cdict_bounditem_string_len_hash(CachedDict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *value;
	if (cdict_iscached_string_len_hash(self, key, keylen, hash))
		return Dee_BOUND_YES;
	value = DeeObject_GetItemStringLenHash(self->cd_map, key, keylen, hash);
	if (value) {
		DREF DeeObject *keyob;
		keyob = DeeString_NewSizedWithHash(key, keylen, hash);
		if unlikely(!keyob) {
			Dee_Decref(value);
			goto err;
		}
		if (!DeeCachedDict_Remember(self, keyob, value, hash))
			goto err;
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundItem))
		return Dee_BOUND_NO;
	if (DeeError_Catch(&DeeError_KeyError))
		return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

PRIVATE struct type_cmp cdict_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&cdict_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&cdict_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&cdict_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&cdict_trycompare_eq,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_ge,
};

PRIVATE struct type_seq cdict_seq = {
	/* .tp_iter                         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cdict_iter,
	/* .tp_sizeob                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cdict_sizeob,
	/* .tp_contains                     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_contains,
	/* .tp_getitem                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_getitem,
	/* .tp_delitem                      = */ DEFIMPL(&default__map_operator_delitem__unsupported),
	/* .tp_setitem                      = */ DEFIMPL(&default__map_operator_setitem__unsupported),
	/* .tp_getrange                     = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                     = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                     = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                      = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&cdict_foreach,
	/* .tp_foreach_pair                 = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&cdict_foreach_pair,
	/* .tp_bounditem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&cdict_bounditem,
	/* .tp_hasitem                      = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size                         = */ (size_t (DCALL *)(DeeObject *__restrict))&cdict_size,
	/* .tp_size_fast                    = */ (size_t (DCALL *)(DeeObject *__restrict))&cdict_size_fast,
	/* .tp_getitem_index                = */ DEFIMPL(&default__getitem_index__with__getitem),
	/* .tp_getitem_index_fast           = */ NULL,
	/* .tp_delitem_index                = */ DEFIMPL(&default__map_operator_delitem_index__unsupported),
	/* .tp_setitem_index                = */ DEFIMPL(&default__map_operator_setitem_index__unsupported),
	/* .tp_bounditem_index              = */ DEFIMPL(&default__bounditem_index__with__bounditem),
	/* .tp_hasitem_index                = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index               = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index               = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index               = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cdict_trygetitem,
	/* .tp_trygetitem_index             = */ DEFIMPL(&default__trygetitem_index__with__trygetitem),
	/* .tp_trygetitem_string_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&cdict_trygetitem_string_hash,
	/* .tp_getitem_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&cdict_getitem_string_hash,
	/* .tp_delitem_string_hash          = */ DEFIMPL(&default__map_operator_delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash          = */ DEFIMPL(&default__map_operator_setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&cdict_bounditem_string_hash,
	/* .tp_hasitem_string_hash          = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&cdict_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&cdict_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash      = */ DEFIMPL(&default__map_operator_delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash      = */ DEFIMPL(&default__map_operator_setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&cdict_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash      = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
	/* .tp_asvector                     = */ NULL,
	/* .tp_asvector_nothrow             = */ NULL,
	/* .tp_trygetitemnr                 = */ (DeeObject *(DCALL *)(DeeObject *__restrict, /*string*/ DeeObject *__restrict))&cdict_trygetitemnr,
	/* .tp_trygetitemnr_string_hash     = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, Dee_hash_t))&cdict_trygetitemnr_string_hash,
	/* .tp_trygetitemnr_string_len_hash = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, size_t, Dee_hash_t))&cdict_trygetitemnr_string_len_hash,
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
cdict_bool(CachedDict *__restrict self) {
	return DeeObject_Bool(self->cd_map);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cdict_init(CachedDict *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "CachedDict", &self->cd_map);
	Dee_Incref(self->cd_map);
	self->cd_mask = 0;
	self->cd_size = 0;
	self->cd_elem = empty_cdict_items;
	Dee_atomic_rwlock_init(&self->cd_lock); /* Lock used for accessing this Dict. */
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
cdict_serialize(CachedDict *__restrict self,
                DeeSerial *__restrict writer,
                Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(CachedDict, field))
	CachedDict *out;
	size_t used_mask, used_size;
	Dee_seraddr_t addrof_elem;
	size_t sizeof_elem;
again:
	DeeCachedDict_LockRead(self);
	used_mask = self->cd_mask;
	used_size = self->cd_size;
	if (self->cd_elem == DeeCachedDict_EmptyItems) {
		DeeCachedDict_LockEndRead(self);
		if (DeeSerial_PutStaticDeemon(writer, ADDROF(cd_elem), DeeCachedDict_EmptyItems))
			goto err;
	} else {
		size_t i;
		struct Dee_cached_dict_item *out_elem;
		struct Dee_cached_dict_item const *in_elem;
		sizeof_elem = (used_mask + 1) * sizeof(struct Dee_cached_dict_item);
		addrof_elem = DeeSerial_TryMalloc(writer, sizeof_elem, NULL);
		if (!Dee_SERADDR_ISOK(addrof_elem)) {
			DeeCachedDict_LockEndRead(self);
			addrof_elem = DeeSerial_Malloc(writer, sizeof_elem, NULL);
			if (!Dee_SERADDR_ISOK(addrof_elem))
				goto err;
			DeeCachedDict_LockRead(self);
			if unlikely(used_mask != self->cd_mask ||
			            used_size != self->cd_size) {
				DeeCachedDict_LockEndRead(self);
				DeeSerial_Free(writer, addrof_elem, NULL);
				goto again;
			}
		}
		out_elem = DeeSerial_Addr2Mem(writer, addrof_elem, struct Dee_cached_dict_item);
		in_elem  = self->cd_elem;
		for (i = 0; i <= used_mask; ++i) {
			*out_elem = *in_elem;
			if (out_elem->cdi_key) {
				Dee_Incref(out_elem->cdi_key);
				Dee_Incref(out_elem->cdi_value);
			}
			++out_elem;
			++in_elem;
		}
		DeeCachedDict_LockEndRead(self);
		if (DeeSerial_PutAddr(writer, ADDROF(cd_elem), addrof_elem))
			goto err;

		/* Serialize cached items. */
		for (i = 0; i <= used_mask; ++i) {
			Dee_seraddr_t addrof_out_elem;
			addrof_out_elem = addrof_elem + i * sizeof(struct Dee_cached_dict_item);
			out_elem = DeeSerial_Addr2Mem(writer, addrof_out_elem, struct Dee_cached_dict_item);
			if (out_elem->cdi_key) {
				DREF DeeObject *key   = out_elem->cdi_key;
				DREF DeeObject *value = out_elem->cdi_value;
				int error = DeeSerial_PutObject(writer, addrof_out_elem + offsetof(struct Dee_cached_dict_item, cdi_key), key);
				if likely(error == 0)
					error = DeeSerial_PutObject(writer, addrof_out_elem + offsetof(struct Dee_cached_dict_item, cdi_value), value);
				Dee_Decref_unlikely(value);
				Dee_Decref_unlikely(key);
				if unlikely(error) {
					for (; i <= used_mask; ++i) {
						addrof_out_elem = addrof_elem + i * sizeof(struct Dee_cached_dict_item);
						out_elem = DeeSerial_Addr2Mem(writer, addrof_out_elem, struct Dee_cached_dict_item);
						if (out_elem->cdi_key) {
							Dee_Decref_unlikely(out_elem->cdi_value);
							Dee_Decref_unlikely(out_elem->cdi_key);
						}
					}
					goto err;
				}
			}
		}
	}

	/* Fill in non-pointer fields. */
	out = DeeSerial_Addr2Mem(writer, addr, CachedDict);
	out->cd_mask = used_mask;
	out->cd_size = used_size;
	Dee_atomic_rwlock_init(&out->cd_lock);
	return DeeSerial_PutObject(writer, ADDROF(cd_map), self->cd_map);
err:
	return -1;
#undef ADDROF
}

PRIVATE struct type_gc tpconst cdict_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&cdict_clear
};

PUBLIC DeeTypeObject DeeCachedDict_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "CachedDict",
	/* .tp_doc      = */ DOC("Cached mapping that remembers the bindings of keys as they are "
	                         /**/ "queried from an underlying ?DMapping (meaning every distinct "
	                         /**/ "key is only ever queried once in the underlying ?DMapping)\n"
	                         "\n"

	                         "()\n"
	                         "Create a new, empty ?.\n"
	                         "\n"

	                         "(map:?DMapping)\n"
	                         "Create a cache mapping for @map"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_KW,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ CachedDict,
			/* tp_ctor:        */ &cdict_ctor,
			/* tp_copy_ctor:   */ &cdict_copy,
			/* tp_any_ctor:    */ &cdict_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &cdict_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cdict_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&cdict_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&cdict_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&cdict_visit,
	/* .tp_gc            = */ &cdict_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__2E23147A197C0EE6),
	/* .tp_cmp           = */ &cdict_cmp,
	/* .tp_seq           = */ &cdict_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL, /* TODO: "property cache: Mapping = { get() }" -- Proxy dict that operates on the cache only (but mustn't allow removing items from the cache). */
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


PUBLIC_CONST struct Dee_cached_dict_item const DeeCachedDict_EmptyItems[1] = {
	{ NULL, NULL, 0 }
};

/* Construct a cached dict wrapper around "mapping" */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCachedDict_New(DeeObject *__restrict mapping) {
	DREF CachedDict *result;
	result = DeeGCObject_MALLOC(CachedDict);
	if unlikely(!result)
		goto done;
	result->cd_mask = 0;
	result->cd_size = 0;
	result->cd_map  = mapping;
	Dee_Incref(mapping);
	result->cd_elem = empty_cdict_items;
	Dee_atomic_rwlock_init(&result->cd_lock);
	DeeObject_Init(result, &DeeCachedDict_Type);
	result = DeeGC_TRACK(CachedDict, result);
done:
	return Dee_AsObject(result);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCachedDict_NewInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict mapping) {
	DREF CachedDict *result;
	result = DeeGCObject_MALLOC(CachedDict);
	if unlikely(!result)
		goto done;
	result->cd_mask = 0;
	result->cd_size = 0;
	result->cd_map  = mapping; /* Inherit reference */
	result->cd_elem = empty_cdict_items;
	Dee_atomic_rwlock_init(&result->cd_lock);
	DeeObject_Init(result, &DeeCachedDict_Type);
	result = DeeGC_TRACK(CachedDict, result);
done:
	return Dee_AsObject(result);
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CACHED_DICT_C */
