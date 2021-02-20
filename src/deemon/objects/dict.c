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
#ifndef GUARD_DEEMON_OBJECTS_DICT_C
#define GUARD_DEEMON_OBJECTS_DICT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/float.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/list.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/atomic.h>
#include <hybrid/sched/yield.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

/* A dummy object used by Dict and HashSet to refer to deleted
 * keys that are still apart of the hash chain.
 * DO NOT EXPOSE THIS OBJECT TO USER-CODE! */
PUBLIC DeeObject DeeDict_Dummy = {
	OBJECT_HEAD_INIT(&DeeObject_Type)
};
#define dummy (&DeeDict_Dummy)


typedef DeeDictObject Dict;

PRIVATE struct dict_item const empty_dict_items[1] = {
	{ NULL, NULL, 0 }
};

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeDict_NewKeyItemsInherited(size_t num_keyitems, DREF DeeObject **key_items) {
	DREF Dict *result;
	/* Allocate the Dict object. */
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		return NULL;
	if (!num_keyitems) {
		/* Special case: allocate an empty Dict. */
		result->d_mask = 0;
		result->d_size = 0;
		result->d_used = 0;
		result->d_elem = (struct dict_item *)empty_dict_items;
	} else {
		size_t min_mask = 16 - 1, mask;
		/* Figure out how large the mask of the Dict is going to be. */
		while ((num_keyitems & min_mask) != num_keyitems)
			min_mask = (min_mask << 1) | 1;
		/* Prefer using a mask of one greater level to improve performance. */
		mask           = (min_mask << 1) | 1;
		result->d_elem = (struct dict_item *)Dee_TryCalloc((mask + 1) * sizeof(struct dict_item));
		if unlikely(!result->d_elem) {
			/* Try one level less if that failed. */
			mask           = min_mask;
			result->d_elem = (struct dict_item *)Dee_Calloc((mask + 1) * sizeof(struct dict_item));
			if unlikely(!result->d_elem)
				goto err_r;
		}
		/* Without any dummy items, these are identical. */
		result->d_size = num_keyitems;
		result->d_used = num_keyitems;
		result->d_mask = mask;
		while (num_keyitems--) {
			DREF DeeObject *key   = *key_items++;
			DREF DeeObject *value = *key_items++;
			dhash_t i, perturb, hash = DeeObject_Hash(key);
			perturb = i = hash & mask;
			for (;; DeeDict_HashNx(i, perturb)) {
				struct dict_item *item = &result->d_elem[i & mask];
				if (item->di_key)
					continue; /* Already in use */
				item->di_hash  = hash;
				item->di_key   = key;   /* Inherit reference. */
				item->di_value = value; /* Inherit reference. */
				break;
			}
		}
	}
#ifndef CONFIG_NO_THREADS
	rwlock_init(&result->d_lock);
#endif /* !CONFIG_NO_THREADS */
	/* Initialize and start tracking the new Dict. */
	weakref_support_init(result);
	DeeObject_Init(result, &DeeDict_Type);
	DeeGC_Track((DeeObject *)result);
	return (DREF DeeObject *)result;
err_r:
	Dee_Free(result->d_elem);
	DeeGCObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
dict_setitem(Dict *self, DeeObject *key, DeeObject *value);
PRIVATE NONNULL((1)) void DCALL dict_fini(Dict *__restrict self);

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_insert_iterator(Dict *self, DeeObject *iterator) {
	DREF DeeObject *elem;
	DREF DeeObject *key_and_value[2];
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		/* Unpack the yielded element again. */
		if unlikely(DeeObject_Unpack(elem, 2, key_and_value))
			goto err_elem;
		Dee_Decref(elem);
		if unlikely(dict_setitem(self, key_and_value[0], key_and_value[1]))
			goto err_item;
		Dee_Decref(key_and_value[1]);
		Dee_Decref(key_and_value[0]);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if unlikely(!elem)
		goto err;
	return 0;
err_item:
	Dee_Decref(key_and_value[1]);
	Dee_Decref(key_and_value[0]);
err_elem:
	Dee_Decref(elem);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_init_iterator(Dict *self, DeeObject *iterator) {
	self->d_mask = 0;
	self->d_size = 0;
	self->d_used = 0;
	self->d_elem = (struct dict_item *)empty_dict_items;
#ifndef CONFIG_NO_THREADS
	rwlock_init(&self->d_lock);
#endif /* !CONFIG_NO_THREADS */
	weakref_support_init(self);
	if unlikely(dict_insert_iterator(self, iterator)) {
		dict_fini(self);
		goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_copy(Dict *__restrict self, Dict *__restrict other);

STATIC_ASSERT(sizeof(struct dict_item) == sizeof(struct rodict_item));
STATIC_ASSERT(offsetof(struct dict_item, di_key) == offsetof(struct rodict_item, di_key));
STATIC_ASSERT(offsetof(struct dict_item, di_value) == offsetof(struct rodict_item, di_value));
STATIC_ASSERT(offsetof(struct dict_item, di_hash) == offsetof(struct rodict_item, di_hash));

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_init_sequence(Dict *__restrict self,
                   DeeObject *__restrict sequence) {
	DREF DeeObject *iterator;
	int error;
	DeeTypeObject *tp = Dee_TYPE(sequence);
	if (tp == &DeeDict_Type)
		return dict_copy(self, (Dict *)sequence);
	/* Optimizations for `_rodict' */
	if (tp == &DeeRoDict_Type) {
		struct dict_item *iter, *end;
		DeeRoDictObject *src = (DeeRoDictObject *)sequence;
#ifndef CONFIG_NO_THREADS
		rwlock_init(&self->d_lock);
#endif /* !CONFIG_NO_THREADS */
		self->d_mask = src->rd_mask;
		self->d_used = self->d_size = src->rd_size;
		if unlikely(!self->d_size)
			self->d_elem = (struct dict_item *)empty_dict_items;
		else {
			self->d_elem = (struct dict_item *)Dee_Malloc((src->rd_mask + 1) *
			                                              sizeof(struct dict_item));
			if unlikely(!self->d_elem)
				goto err;
			memcpyc(self->d_elem, src->rd_elem,
			        self->d_mask + 1,
			        sizeof(struct dict_item));
			end = (iter = self->d_elem) + (self->d_mask + 1);
			for (; iter != end; ++iter) {
				if (!iter->di_key)
					continue;
				Dee_Incref(iter->di_key);
				Dee_Incref(iter->di_value);
			}
		}
		weakref_support_init(self);
		return 0;
	}
	/* TODO: Optimizations for `_sharedmap' */
	/* TODO: Fast-sequence support */

	iterator = DeeObject_IterSelf(sequence);
	if unlikely(!iterator)
		goto err;
	error = dict_init_iterator(self, iterator);
	Dee_Decref(iterator);
	return error;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDict_FromIterator(DeeObject *__restrict self) {
	DREF Dict *result;
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto done;
	if unlikely(dict_init_iterator(result, self))
		goto err_r;
	DeeObject_Init(result, &DeeDict_Type);
	DeeGC_Track((DeeObject *)result);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeGCObject_FREE(result);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDict_FromSequence(DeeObject *__restrict self) {
	DREF Dict *result;
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto done;
	if unlikely(dict_init_sequence(result, self))
		goto err_r;
	DeeObject_Init(result, &DeeDict_Type);
	DeeGC_Track((DeeObject *)result);
done:
	return (DREF DeeObject *)result;
err_r:
	DeeGCObject_FREE(result);
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_ctor(Dict *__restrict self) {
	self->d_mask = 0;
	self->d_size = 0;
	self->d_used = 0;
	self->d_elem = (struct dict_item *)empty_dict_items;
#ifndef CONFIG_NO_THREADS
	rwlock_init(&self->d_lock);
#endif /* !CONFIG_NO_THREADS */
	weakref_support_init(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_copy(Dict *__restrict self,
          Dict *__restrict other) {
	struct dict_item *iter, *end;
#ifndef CONFIG_NO_THREADS
	rwlock_init(&self->d_lock);
#endif /* !CONFIG_NO_THREADS */
again:
	DeeDict_LockRead(other);
	self->d_mask = other->d_mask;
	self->d_used = other->d_used;
	self->d_size = other->d_size;
	if ((self->d_elem = other->d_elem) != empty_dict_items) {
		self->d_elem = (struct dict_item *)Dee_TryMalloc((other->d_mask + 1) *
		                                                 sizeof(struct dict_item));
		if unlikely(!self->d_elem) {
			DeeDict_LockEndRead(other);
			if (Dee_CollectMemory((other->d_mask + 1) *
			                      sizeof(struct dict_item)))
				goto again;
			goto err;
		}
		memcpyc(self->d_elem, other->d_elem,
		        self->d_mask + 1,
		        sizeof(struct dict_item));
		end = (iter = self->d_elem) + (self->d_mask + 1);
		for (; iter != end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Incref(iter->di_key);
			Dee_XIncref(iter->di_value);
		}
	}
	DeeDict_LockEndRead(other);
	weakref_support_init(self);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_deepload(Dict *__restrict self) {
	typedef struct {
		DREF DeeObject *e_key;   /* [0..1][lock(:d_lock)] Dictionary item key. */
		DREF DeeObject *e_value; /* [1..1|if(di_key == dummy, 0..0)][valid_if(di_key)][lock(:d_lock)] Dictionary item value. */
	} Entry;
	/* #1 Allocate 2 new element-vector of the same size as `self'
	 *    One of them has a length `d_mask+1', the other `d_used'
	 * #2 Copy all key/value pairs from `self' into the d_used-one (create references)
	 *    NOTE: Skip NULL/dummy entries in the Dict vector.
	 * #3 Go through the vector and create deep copies of all keys and items.
	 *    For every key, hash it and insert it into the 2nd vector from before.
	 * #4 Clear and free the 1st vector.
	 * #5 Assign the 2nd vector to the Dict, extracting the old one at the same time.
	 * #6 Clear and free the old vector. */
	Entry *new_items, *items = NULL;
	size_t i, hash_i, item_count, old_item_count = 0;
	struct dict_item *new_map, *old_map;
	size_t new_mask;
	for (;;) {
		DeeDict_LockRead(self);
		/* Optimization: if the Dict is empty, then there's nothing to copy! */
		if (self->d_elem == empty_dict_items) {
			DeeDict_LockEndRead(self);
			return 0;
		}
		item_count = self->d_used;
		if (item_count <= old_item_count)
			break;
		DeeDict_LockEndRead(self);
		new_items = (Entry *)Dee_Realloc(items, item_count * sizeof(Entry));
		if unlikely(!new_items)
			goto err_items;
		old_item_count = item_count;
		items          = new_items;
	}
	/* Copy all used items. */
	for (i = 0, hash_i = 0; i < item_count; ++hash_i) {
		ASSERT(hash_i <= self->d_mask);
		if (self->d_elem[hash_i].di_key == NULL)
			continue;
		if (self->d_elem[hash_i].di_key == dummy)
			continue;
		items[i].e_key   = self->d_elem[hash_i].di_key;
		items[i].e_value = self->d_elem[hash_i].di_value;
		Dee_Incref(items[i].e_key);
		Dee_Incref(items[i].e_value);
		++i;
	}
	DeeDict_LockEndRead(self);
	/* With our own local copy of all items being
	 * used, replace all of them with deep copies. */
	for (i = 0; i < item_count; ++i) {
		if (DeeObject_InplaceDeepCopy(&items[i].e_key))
			goto err_items_v;
		if (DeeObject_InplaceDeepCopy(&items[i].e_value))
			goto err_items_v;
	}
	new_mask = 1;
	while ((item_count & new_mask) != item_count)
		new_mask = (new_mask << 1) | 1;
	new_map = (struct dict_item *)Dee_Calloc((new_mask + 1) * sizeof(struct dict_item));
	if unlikely(!new_map)
		goto err_items_v;
	/* Insert all the copied items into the new map. */
	for (i = 0; i < item_count; ++i) {
		dhash_t j, perturb, hash;
		hash    = DeeObject_Hash(items[i].e_key);
		perturb = j = hash & new_mask;
		for (;; DeeDict_HashNx(j, perturb)) {
			struct dict_item *item = &new_map[j & new_mask];
			if (item->di_key) {
				/* Check if deepcopy caused one of the elements to get duplicated. */
				if unlikely(item->di_key == items[i].e_key) {
remove_duplicate_key:
					Dee_Decref(items[i].e_key);
					Dee_Decref(items[i].e_value);
					--item_count;
					memmovedownc(&items[i],
					             &items[i + 1],
					             item_count - i,
					             sizeof(struct dict_item));
					break;
				}
				if (Dee_TYPE(item->di_key) == Dee_TYPE(items[i].e_key)) {
					int error;
					error = DeeObject_CompareEq(item->di_key, items[i].e_key);
					if unlikely(error < 0)
						goto err_items_v_new_map;
					if (error)
						goto remove_duplicate_key;
				}
				/* Slot already in use */
				continue;
			}
			item->di_hash  = hash;
			item->di_key   = items[i].e_key;   /* Inherit reference. */
			item->di_value = items[i].e_value; /* Inherit reference. */
			break;
		}
	}
	DeeDict_LockWrite(self);
	i            = self->d_mask + 1;
	self->d_mask = new_mask;
	self->d_used = item_count;
	self->d_size = item_count;
	old_map      = self->d_elem;
	self->d_elem = new_map;
	DeeDict_LockEndWrite(self);
	if (old_map != empty_dict_items) {
		while (i--) {
			if (!old_map[i].di_key)
				continue;
			Dee_Decref(old_map[i].di_value);
			Dee_Decref(old_map[i].di_key);
		}
		Dee_Free(old_map);
	}
	Dee_Free(items);
	return 0;
err_items_v_new_map:
	Dee_Free(new_map);
err_items_v:
	i = item_count;
	while (i--) {
		Dee_Decref(items[i].e_value);
		Dee_Decref(items[i].e_key);
	}
err_items:
	Dee_Free(items);
	return -1;
}

PRIVATE NONNULL((1)) void DCALL dict_fini(Dict *__restrict self) {
	weakref_support_fini(self);
	ASSERT((self->d_elem == empty_dict_items) == (self->d_mask == 0));
	ASSERT((self->d_elem == empty_dict_items) == (self->d_size == 0));
	ASSERT(self->d_used <= self->d_size);
	if (self->d_elem != empty_dict_items) {
		struct dict_item *iter, *end;
		end = (iter = self->d_elem) + (self->d_mask + 1);
		for (; iter != end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Decref(iter->di_key);
			Dee_XDecref(iter->di_value);
		}
		Dee_Free(self->d_elem);
	}
}

PRIVATE NONNULL((1)) void DCALL dict_clear(Dict *__restrict self) {
	struct dict_item *elem;
	size_t mask;
	DeeDict_LockWrite(self);
	ASSERT((self->d_elem == empty_dict_items) == (self->d_mask == 0));
	ASSERT((self->d_elem == empty_dict_items) == (self->d_size == 0));
	ASSERT(self->d_used <= self->d_size);
	/* Extract the vector and mask. */
	elem         = self->d_elem;
	mask         = self->d_mask;
	self->d_elem = (struct dict_item *)empty_dict_items;
	self->d_mask = 0;
	self->d_used = 0;
	self->d_size = 0;
	DeeDict_LockEndWrite(self);
	/* Destroy the vector. */
	if (elem != empty_dict_items) {
		struct dict_item *iter, *end;
		end = (iter = elem) + (mask + 1);
		for (; iter != end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Decref(iter->di_key);
			Dee_XDecref(iter->di_value);
		}
		Dee_Free(elem);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
dict_visit(Dict *__restrict self, dvisit_t proc, void *arg) {
	DeeDict_LockRead(self);
	ASSERT((self->d_elem == empty_dict_items) == (self->d_mask == 0));
	ASSERT((self->d_elem == empty_dict_items) == (self->d_size == 0));
	ASSERT(self->d_used <= self->d_size);
	if (self->d_elem != empty_dict_items) {
		struct dict_item *iter, *end;
		end = (iter = self->d_elem) + (self->d_mask + 1);
		for (; iter != end; ++iter) {
			if (!iter->di_key)
				continue;
			/* Visit all keys and associated values. */
			Dee_Visit(iter->di_key);
			Dee_XVisit(iter->di_value);
		}
	}
	DeeDict_LockEndRead(self);
}


/* Resize the hash size by a factor of 2 and re-insert all elements.
 * When `sizedir > 0', increase the hash; When `sizedir < 0', decrease it.
 * During this process all dummy items are discarded.
 * @return: true:  Successfully rehashed the Dict.
 * @return: false: Not enough memory. - The caller should collect some and try again. */
PRIVATE NONNULL((1)) bool DCALL
dict_rehash(Dict *__restrict self, int sizedir) {
	struct dict_item *new_vector, *iter, *end;
	size_t new_mask = self->d_mask;
	if (sizedir > 0) {
		new_mask = (new_mask << 1) | 1;
		if unlikely(new_mask == 1)
			new_mask = 16 - 1; /* Start out bigger than 2. */
	} else if (sizedir < 0) {
		if unlikely(!self->d_used) {
			ASSERT(!self->d_used);
			/* Special case: delete the vector. */
			if (self->d_size) {
				ASSERT(self->d_elem != empty_dict_items);
				/* Must discard dummy items. */
				end = (iter = self->d_elem) + (self->d_mask + 1);
				for (; iter != end; ++iter) {
					ASSERT(iter->di_key == NULL ||
					       iter->di_key == dummy);
					if (iter->di_key == dummy)
						Dee_DecrefNokill(dummy);
				}
			}
			if (self->d_elem != empty_dict_items)
				Dee_Free(self->d_elem);
			self->d_elem = (struct dict_item *)empty_dict_items;
			self->d_mask = 0;
			self->d_size = 0;
			return true;
		}
		new_mask = (new_mask >> 1);
		if (self->d_used >= new_mask)
			return true;
	}
	ASSERT(self->d_used < new_mask);
	ASSERT(self->d_used <= self->d_size);
	new_vector = (struct dict_item *)Dee_TryCalloc((new_mask + 1) * sizeof(struct dict_item));
	if unlikely(!new_vector)
		return false;
	ASSERT((self->d_elem == empty_dict_items) == (self->d_mask == 0));
	ASSERT((self->d_elem == empty_dict_items) == (self->d_size == 0));
	if (self->d_elem != empty_dict_items) {
		/* Re-insert all existing items into the new Dict vector. */
		end = (iter = self->d_elem) + (self->d_mask + 1);
		for (; iter != end; ++iter) {
			struct dict_item *item;
			dhash_t i, perturb;
			/* Skip dummy keys. */
			if (!iter->di_key || iter->di_key == dummy)
				continue;
			perturb = i = iter->di_hash & new_mask;
			for (;; DeeDict_HashNx(i, perturb)) {
				item = &new_vector[i & new_mask];
				if (!item->di_key)
					break; /* Empty slot found. */
			}
			/* Transfer this object. */
			item->di_key   = iter->di_key;
			item->di_hash  = iter->di_hash;
			item->di_value = iter->di_value;
		}
		Dee_Free(self->d_elem);
		/* With all dummy items gone, the size now equals what is actually used. */
		self->d_size = self->d_used;
	}
	ASSERT(self->d_size == self->d_used);
	self->d_mask = new_mask;
	self->d_elem = new_vector;
	return true;
}



INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeDict_GetItemString(DeeObject *__restrict self,
                      char const *__restrict key,
                      dhash_t hash) {
	DREF DeeObject *result;
	dhash_t i, perturb;
	DeeDict_LockRead(self);
	perturb = i = DeeDict_HashSt(self, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(self, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (!strcmp(DeeString_STR(item->di_key), key)) {
			result = item->di_value;
			Dee_Incref(result);
			DeeDict_LockEndRead(self);
			return result;
		}
	}
	DeeDict_LockEndRead(self);
	err_unknown_key_str(self, key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeDict_GetItemStringLen(DeeObject *__restrict self,
                         char const *__restrict key,
                         size_t keylen,
                         dhash_t hash) {
	DREF DeeObject *result;
	dhash_t i, perturb;
	DeeDict_LockRead(self);
	perturb = i = DeeDict_HashSt(self, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(self, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (DeeString_SIZE(item->di_key) != keylen)
			continue;
		if (memcmp(DeeString_STR(item->di_key), key, keylen * sizeof(char)) == 0) {
			result = item->di_value;
			Dee_Incref(result);
			DeeDict_LockEndRead(self);
			return result;
		}
	}
	DeeDict_LockEndRead(self);
	err_unknown_key_str_len(self, key, keylen);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
DeeDict_GetItemStringDef(DeeObject *self,
                         char const *__restrict key,
                         dhash_t hash,
                         DeeObject *def) {
	DREF DeeObject *result;
	dhash_t i, perturb;
	ASSERT_OBJECT(def);
	DeeDict_LockRead(self);
	perturb = i = DeeDict_HashSt(self, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(self, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (!strcmp(DeeString_STR(item->di_key), key)) {
			result = item->di_value;
			Dee_Incref(result);
			DeeDict_LockEndRead(self);
			return result;
		}
	}
	DeeDict_LockEndRead(self);
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeDict_GetItemStringLenDef(DeeObject *self,
                            char const *__restrict key,
                            size_t keylen,
                            dhash_t hash,
                            DeeObject *def) {
	DREF DeeObject *result;
	dhash_t i, perturb;
	ASSERT_OBJECT(def);
	DeeDict_LockRead(self);
	perturb = i = DeeDict_HashSt(self, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(self, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (DeeString_SIZE(item->di_key) != keylen)
			continue;
		if (memcmp(DeeString_STR(item->di_key), key, keylen * sizeof(char)) == 0) {
			result = item->di_value;
			Dee_Incref(result);
			DeeDict_LockEndRead(self);
			return result;
		}
	}
	DeeDict_LockEndRead(self);
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeDict_HasItemString(DeeObject *__restrict self,
                      char const *__restrict key,
                      dhash_t hash) {
	dhash_t i, perturb;
	DeeDict_LockRead(self);
	perturb = i = DeeDict_HashSt(self, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(self, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (!strcmp(DeeString_STR(item->di_key), key)) {
			DeeDict_LockEndRead(self);
			return true;
		}
	}
	DeeDict_LockEndRead(self);
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeDict_HasItemStringLen(DeeObject *__restrict self,
                         char const *__restrict key,
                         size_t keylen,
                         dhash_t hash) {
	dhash_t i, perturb;
	DeeDict_LockRead(self);
	perturb = i = DeeDict_HashSt(self, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(self, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (DeeString_SIZE(item->di_key) != keylen)
			continue;
		if (memcmp(DeeString_STR(item->di_key), key, keylen * sizeof(char)) == 0) {
			DeeDict_LockEndRead(self);
			return true;
		}
	}
	DeeDict_LockEndRead(self);
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeDict_DelItemString(DeeObject *__restrict self,
                      char const *__restrict key,
                      dhash_t hash) {
	Dict *me = (Dict *)self;
	DREF DeeObject *old_key, *old_value;
	dhash_t i, perturb;
#ifndef CONFIG_NO_THREADS
again_lock:
#endif /* !CONFIG_NO_THREADS */
	DeeDict_LockRead(self);
	perturb = i = DeeDict_HashSt(self, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(self, i);
		if (!item->di_key)
			break; // Not found
		if (item->di_hash != hash)
			continue; // Non-matching hash
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (strcmp(DeeString_STR(item->di_key), key) != 0)
			continue;
#ifndef CONFIG_NO_THREADS
		if (!DeeDict_LockUpgrade(me)) {
			DeeDict_LockEndWrite(me);
			SCHED_YIELD();
			goto again_lock;
		}
#endif /* !CONFIG_NO_THREADS */
		old_key   = item->di_key;
		old_value = item->di_value;
		Dee_Incref(dummy);
		item->di_key   = dummy;
		item->di_value = NULL;
		ASSERT(me->d_used);
		/* Try to rehash the Dict and get rid of dummy
		 * items if there are a lot of them now. */
		if (--me->d_used <= me->d_size / 3)
			dict_rehash(me, -1);
		DeeDict_LockEndWrite(self);
		Dee_Decref(old_value);
		Dee_Decref(old_key);
		return 0;
	}
	DeeDict_LockEndRead(self);
	return err_unknown_key_str(self, key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeDict_DelItemStringLen(DeeObject *__restrict self,
                         char const *__restrict key,
                         size_t keylen,
                         dhash_t hash) {
	Dict *me = (Dict *)self;
	DREF DeeObject *old_key, *old_value;
	dhash_t i, perturb;
#ifndef CONFIG_NO_THREADS
again_lock:
#endif /* !CONFIG_NO_THREADS */
	DeeDict_LockRead(self);
	perturb = i = DeeDict_HashSt(self, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(self, i);
		if (!item->di_key)
			break; // Not found
		if (item->di_hash != hash)
			continue; // Non-matching hash
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (DeeString_SIZE(item->di_key) != keylen)
			continue;
		if (memcmp(DeeString_STR(item->di_key), key, keylen * sizeof(char)) != 0)
			continue;
#ifndef CONFIG_NO_THREADS
		if (!DeeDict_LockUpgrade(me)) {
			DeeDict_LockEndWrite(me);
			SCHED_YIELD();
			goto again_lock;
		}
#endif /* !CONFIG_NO_THREADS */
		old_key   = item->di_key;
		old_value = item->di_value;
		Dee_Incref(dummy);
		item->di_key   = dummy;
		item->di_value = NULL;
		ASSERT(me->d_used);
		/* Try to rehash the Dict and get rid of dummy
		 * items if there are a lot of them now. */
		if (--me->d_used <= me->d_size / 3)
			dict_rehash(me, -1);
		DeeDict_LockEndWrite(self);
		Dee_Decref(old_value);
		Dee_Decref(old_key);
		return 0;
	}
	DeeDict_LockEndRead(self);
	return err_unknown_key_str_len(self, key, keylen);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
DeeDict_SetItemString(DeeObject *self,
                      char const *__restrict key,
                      dhash_t hash,
                      DeeObject *value) {
	Dict *me = (Dict *)self;
	DREF DeeObject *old_value;
	struct dict_item *first_dummy;
	dhash_t i, perturb;
again_lock:
	DeeDict_LockRead(self);
again:
	first_dummy = NULL;
	perturb = i = DeeDict_HashSt(self, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(self, i);
		if (!item->di_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->di_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue;
		if (strcmp(DeeString_STR(item->di_key), key) != 0)
			continue;
#ifndef CONFIG_NO_THREADS
		if (!DeeDict_LockUpgrade(me)) {
			DeeDict_LockEndWrite(me);
			SCHED_YIELD();
			goto again_lock;
		}
#endif /* !CONFIG_NO_THREADS */
		/* Override an existing entry. */
		old_value = item->di_value;
		Dee_Incref(value);
		item->di_value = value;
		DeeDict_LockEndWrite(self);
		Dee_Decref(old_value);
		return 0;
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeDict_LockUpgrade(me)) {
		DeeDict_LockEndWrite(me);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if (first_dummy && me->d_size + 1 < me->d_mask) {
		DREF DeeStringObject *key_ob;
		size_t key_len = strlen(key);
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);
		/* Write to the first dummy item. */
		key_ob = (DREF DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject, s_str) +
		                                                     (key_len + 1) * sizeof(char));
		if unlikely(!key_ob)
			goto collect_memory;
		DeeObject_Init(key_ob, &DeeString_Type);
		key_ob->s_data = NULL;
		key_ob->s_hash = hash;
		key_ob->s_len  = key_len;
		memcpyc(key_ob->s_str, key,
		        key_len + 1,
		        sizeof(char));
		if (first_dummy->di_key)
			Dee_DecrefNokill(first_dummy->di_key);
		/* Fill in the target slot. */
		first_dummy->di_key   = (DREF DeeObject *)key_ob; /* Inherit reference. */
		first_dummy->di_hash  = hash;
		first_dummy->di_value = value;
		Dee_Incref(value);
		++me->d_used;
		++me->d_size;
		/* Try to keep the Dict vector big at least twice as big as the element count. */
		if (me->d_size * 2 > me->d_mask)
			dict_rehash(me, 1);
		DeeDict_LockEndWrite(self);
		return 0;
	}
	/* Rehash the Dict and try again. */
	if (dict_rehash(me, 1)) {
		DeeDict_LockDowngrade(self);
		goto again;
	}
collect_memory:
	DeeDict_LockEndWrite(self);
	if (Dee_CollectMemory(1))
		goto again_lock;
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeDict_SetItemStringLen(DeeObject *self,
                         char const *__restrict key,
                         size_t keylen,
                         dhash_t hash,
                         DeeObject *value) {
	Dict *me = (Dict *)self;
	DREF DeeObject *old_value;
	struct dict_item *first_dummy;
	dhash_t i, perturb;
again_lock:
	DeeDict_LockRead(self);
again:
	first_dummy = NULL;
	perturb = i = DeeDict_HashSt(self, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(self, i);
		if (!item->di_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->di_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue;
		if (DeeString_SIZE(item->di_key) != keylen)
			continue;
		if (memcmp(DeeString_STR(item->di_key), key, keylen * sizeof(char)) != 0)
			continue;
#ifndef CONFIG_NO_THREADS
		if (!DeeDict_LockUpgrade(me)) {
			DeeDict_LockEndWrite(me);
			SCHED_YIELD();
			goto again_lock;
		}
#endif /* !CONFIG_NO_THREADS */
		/* Override an existing entry. */
		old_value = item->di_value;
		Dee_Incref(value);
		item->di_value = value;
		DeeDict_LockEndWrite(self);
		Dee_Decref(old_value);
		return 0;
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeDict_LockUpgrade(me)) {
		DeeDict_LockEndWrite(me);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if (first_dummy && me->d_size + 1 < me->d_mask) {
		DREF DeeStringObject *key_ob;
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);
		/* Write to the first dummy item. */
		key_ob = (DREF DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject, s_str) +
		                                                     (keylen + 1) * sizeof(char));
		if unlikely(!key_ob)
			goto collect_memory;
		DeeObject_Init(key_ob, &DeeString_Type);
		key_ob->s_data = NULL;
		key_ob->s_hash = hash;
		key_ob->s_len  = keylen;
		memcpyc(key_ob->s_str, key,
		        keylen, sizeof(char));
		key_ob->s_str[keylen] = '\0';
		if (first_dummy->di_key)
			Dee_DecrefNokill(first_dummy->di_key);
		/* Fill in the target slot. */
		first_dummy->di_key   = (DREF DeeObject *)key_ob; /* Inherit reference. */
		first_dummy->di_hash  = hash;
		first_dummy->di_value = value;
		Dee_Incref(value);
		++me->d_used;
		++me->d_size;
		/* Try to keep the Dict vector big at least twice as big as the element count. */
		if (me->d_size * 2 > me->d_mask)
			dict_rehash(me, 1);
		DeeDict_LockEndWrite(self);
		return 0;
	}
	/* Rehash the Dict and try again. */
	if (dict_rehash(me, 1)) {
		DeeDict_LockDowngrade(self);
		goto again;
	}
collect_memory:
	DeeDict_LockEndWrite(self);
	if (Dee_CollectMemory(1))
		goto again_lock;
	return -1;
}


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_size(Dict *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return DeeInt_NewSize(self->d_used);
#else /* CONFIG_NO_THREADS */
	return DeeInt_NewSize(ATOMIC_READ(self->d_used));
#endif /* !CONFIG_NO_THREADS */
}

/* This one's basically your hasitem operator. */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_contains(Dict *self,
              DeeObject *key) {
	size_t mask;
	struct dict_item *vector;
	dhash_t i, perturb;
	int error;
	dhash_t hash = DeeObject_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		DREF DeeObject *item_key;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		item_key = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_CompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error > 0)
			return_true; /* Found the item. */
		if unlikely(error < 0)
			return NULL; /* Error in compare operator. */
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return_false;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_getitem(Dict *self,
             DeeObject *key) {
	size_t mask;
	struct dict_item *vector;
	dhash_t i, perturb;
	int error;
	dhash_t hash = DeeObject_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		DREF DeeObject *item_key, *item_value;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		item_key   = item->di_key;
		item_value = item->di_value;
		Dee_Incref(item_key);
		Dee_Incref(item_value);
		DeeDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_CompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error > 0)
			return item_value; /* Found the item. */
		Dee_Decref(item_value);
		if unlikely(error < 0)
			return NULL; /* Error in compare operator. */
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key ||
		    item->di_value != item_value)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	err_unknown_key((DeeObject *)self, key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeDict_GetItemDef(DeeObject *self,
                   DeeObject *key,
                   DeeObject *def) {
	size_t mask;
	struct dict_item *vector;
	dhash_t i, perturb;
	int error;
	dhash_t hash = DeeObject_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = ((Dict *)self)->d_elem;
	mask    = ((Dict *)self)->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		DREF DeeObject *item_key, *item_value;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		item_key   = item->di_key;
		item_value = item->di_value;
		Dee_Incref(item_key);
		Dee_Incref(item_value);
		DeeDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_CompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error > 0)
			return item_value; /* Found the item. */
		Dee_Decref(item_value);
		if unlikely(error < 0)
			return NULL; /* Error in compare operator. */
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (((Dict *)self)->d_elem != vector ||
		    ((Dict *)self)->d_mask != mask ||
		    item->di_key != item_key ||
		    item->di_value != item_value)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDict_ByHash(DeeObject *__restrict self, Dee_hash_t hash, bool key_only) {
	DREF DeeObject *result;
	DREF DeeObject *match;
	size_t mask;
	struct dict_item *vector;
	dhash_t i, perturb;
	Dict *me = (Dict *)self;
again:
	match = NULL;
	DeeDict_LockRead(me);
	vector  = me->d_elem;
	mask    = me->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item;
		item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		if unlikely(match) {
			/* There are multiple matches for `hash'. */
			DeeDict_LockEndRead(me);
			Dee_Decref(match);
			/* TODO: Dict-specific optimizations? */
			return DeeMap_HashFilter(self, hash);
		}
		if (key_only) {
			match = item->di_key;
			Dee_Incref(match);
		} else {
			DREF DeeObject *key, *value;
			key   = item->di_key;
			value = item->di_value;
			Dee_Incref(key);
			Dee_Incref(value);
			DeeDict_LockEndRead(me);
			match = DeeTuple_NewUninitialized(2);
			if unlikely(!match) {
				Dee_Decref(key);
				Dee_Decref(value);
				goto err;
			}
			DeeTuple_SET(match, 0, key);   /* Inherit reference */
			DeeTuple_SET(match, 1, value); /* Inherit reference */
			DeeDict_LockRead(me);
			/* Check if the Dict was modified. */
			if (me->d_elem != vector || me->d_mask != mask ||
			    item->di_key != key || item->di_value != value) {
				DeeDict_LockEndRead(me);
				Dee_DecrefDokill(match);
				goto again;
			}
		}
	}
	DeeDict_LockEndRead(me);
	if (!match)
		return_empty_tuple;
	result = DeeTuple_NewUninitialized(1);
	if unlikely(!result)
		goto err_match;
	DeeTuple_SET(result, 0, match); /* Inherit reference */
	return result;
err_match:
	Dee_Decref(match);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_popitem(Dict *self, DeeObject *key, DeeObject *def) {
	size_t mask;
	struct dict_item *vector;
	dhash_t i, perturb;
	int error;
	dhash_t hash = DeeObject_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		DREF DeeObject *item_key;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (item->di_key == dummy)
			continue; /* Dummy key. */
		item_key = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_CompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error < 0)
			return NULL; /* Error in compare operator. */
		if (error > 0) {
			DREF DeeObject *item_value;
			/* Found it! */
			DeeDict_LockWrite(self);
			/* Check if the Dict was modified. */
			if (self->d_elem != vector ||
			    self->d_mask != mask ||
			    item->di_key != item_key) {
				DeeDict_LockDowngrade(self);
				goto restart;
			}
			item_value = item->di_value;
			Dee_Incref(dummy);
			item->di_key   = dummy;
			item->di_value = NULL;
			ASSERT(self->d_used);
			if (--self->d_used <= self->d_size / 3)
				dict_rehash(self, -1);
			DeeDict_LockEndWrite(self);
			Dee_Decref(item_key);
			return item_value;
		}
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	if (def)
		return_reference_(def);
	err_unknown_key((DeeObject *)self, key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_delitem(Dict *self, DeeObject *key) {
	DREF DeeObject *pop_item;
	pop_item = dict_popitem(self, key, NULL);
	Dee_XDecref(pop_item);
	return (likely(pop_item))
	       ? 0
	       : -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
dict_setitem(Dict *self, DeeObject *key, DeeObject *value) {
	size_t mask;
	struct dict_item *vector;
	int error;
	struct dict_item *first_dummy;
	dhash_t i, perturb, hash = DeeObject_Hash(key);
again_lock:
	DeeDict_LockRead(self);
again:
	first_dummy = NULL;
	vector      = self->d_elem;
	mask        = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		DREF DeeObject *item_key;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->di_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		item_key = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_CompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error < 0)
			goto err; /* Error in compare operator. */
		if (error > 0) {
			DREF DeeObject *item_value;
			/* Found an existing item. */
			DeeDict_LockWrite(self);
			/* Check if the Dict was modified. */
			if (self->d_elem != vector ||
			    self->d_mask != mask ||
			    item->di_key != item_key) {
				DeeDict_LockDowngrade(self);
				goto again;
			}
			item_value = item->di_value;
			Dee_Incref(key);
			Dee_Incref(value);
			item->di_key   = key;
			item->di_value = value;
			DeeDict_LockEndWrite(self);
			Dee_Decref(item_key);
			Dee_Decref(item_value);
			return 0;
		}
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto again;
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeDict_LockUpgrade(self)) {
		DeeDict_LockEndWrite(self);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if (first_dummy && self->d_size + 1 < self->d_mask) {
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);
		if (first_dummy->di_key)
			Dee_DecrefNokill(first_dummy->di_key);
		/* Fill in the target slot. */
		first_dummy->di_key   = key;
		first_dummy->di_hash  = hash;
		first_dummy->di_value = value;
		Dee_Incref(key);
		Dee_Incref(value);
		++self->d_used;
		++self->d_size;
		/* Try to keep the Dict vector big at least twice as big as the element count. */
		if (self->d_size * 2 > self->d_mask)
			dict_rehash(self, 1);
		DeeDict_LockEndWrite(self);
		return 0;
	}
	/* Rehash the Dict and try again. */
	if (dict_rehash(self, 1)) {
		DeeDict_LockDowngrade(self);
		goto again;
	}
	DeeDict_LockEndWrite(self);
	if (Dee_CollectMemory(1))
		goto again_lock;
err:
	return -1;
}


#define SETITEM_SETOLD 0 /* if_exists: *pold_value = GET_OLD_VALUE(); SET_OLD_ITEM(); return 1;
                          * else:      return 0; */
#define SETITEM_SETNEW 1 /* if_exists: *pold_value = GET_OLD_VALUE(); return 1;
                          * else:      ADD_NEW_ITEM(); return 0; */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
dict_setitem_ex(Dict *self,
                DeeObject *key,
                DeeObject *value,
                unsigned int mode,
                DREF DeeObject **pold_value) {
	size_t mask;
	struct dict_item *vector;
	int error;
	struct dict_item *first_dummy;
	dhash_t i, perturb, hash = DeeObject_Hash(key);
again_lock:
	DeeDict_LockRead(self);
again:
	first_dummy = NULL;
	vector      = self->d_elem;
	mask        = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		DREF DeeObject *item_key;
		struct dict_item *item = &vector[i & mask];
		if (!item->di_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->di_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		item_key = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);
		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_CompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error < 0)
			goto err; /* Error in compare operator. */
		if (error > 0) {
			DREF DeeObject *item_value;
			/* Found an existing key. */
			if (mode == SETITEM_SETOLD) {
				DeeDict_LockWrite(self);
				/* Check if the Dict was modified. */
				if (self->d_elem != vector ||
				    self->d_mask != mask ||
				    item->di_key != item_key) {
					DeeDict_LockDowngrade(self);
					goto again;
				}
				item_value = item->di_value;
				Dee_Incref(key);
				Dee_Incref(value);
				item->di_key   = key;
				item->di_value = value;
				DeeDict_LockEndWrite(self);
				if (pold_value) {
					*pold_value = item_value; /* Inherit reference */
				} else {
					Dee_Decref(item_value);
				}
			} else {
				DeeDict_LockRead(self);
				/* Check if the Dict was modified. */
				if (self->d_elem != vector ||
				    self->d_mask != mask ||
				    item->di_key != item_key)
					goto again;
				if (pold_value) {
					item_value = item->di_value;
					Dee_Incref(item_value);
					*pold_value = item_value;
				}
				DeeDict_LockEndRead(self);
			}
			return 1;
		}
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto again;
	}
	if (mode == SETITEM_SETOLD) {
		DeeDict_LockEndRead(self);
		return 0;
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeDict_LockUpgrade(self)) {
		DeeDict_LockEndWrite(self);
		SCHED_YIELD();
		goto again_lock;
	}
#endif
	if (first_dummy && self->d_size + 1 < self->d_mask) {
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);
		if (first_dummy->di_key)
			Dee_DecrefNokill(first_dummy->di_key);
		/* Fill in the target slot. */
		first_dummy->di_key   = key;
		first_dummy->di_hash  = hash;
		first_dummy->di_value = value;
		Dee_Incref(key);
		Dee_Incref(value);
		++self->d_used;
		++self->d_size;
		/* Try to keep the Dict vector big at least twice as big as the element count. */
		if (self->d_size * 2 > self->d_mask)
			dict_rehash(self, 1);
		DeeDict_LockEndWrite(self);
		return 0;
	}
	/* Rehash the Dict and try again. */
	if (dict_rehash(self, 1)) {
		DeeDict_LockDowngrade(self);
		goto again;
	}
	DeeDict_LockEndWrite(self);
	if (Dee_CollectMemory(1))
		goto again_lock;
err:
	return -1;
}


/* Implemented in `dictproxy.c' */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_iter(DeeDictObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL dictiterator_next_key(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL dictiterator_next_value(DeeObject *__restrict self);


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
dict_nsi_getsize(Dict *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return self->d_used;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->d_used);
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
dict_nsi_setdefault(Dict *self, DeeObject *key, DeeObject *defl) {
	DeeObject *old_value;
	int error;
	error = dict_setitem_ex(self, key, defl, SETITEM_SETNEW, &old_value);
	if unlikely(error < 0)
		return NULL;
	if (error == 1)
		return old_value;
	return_reference_(defl);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
dict_nsi_updateold(Dict *self, DeeObject *key,
                   DeeObject *value, DREF DeeObject **poldvalue) {
	return dict_setitem_ex(self, key, value, SETITEM_SETOLD, poldvalue);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
dict_nsi_insertnew(Dict *self, DeeObject *key,
                   DeeObject *value, DREF DeeObject **poldvalue) {
	int error;
	error = dict_setitem_ex(self, key, value, SETITEM_SETNEW, poldvalue);
	if unlikely(error < 0)
		goto err;
	return !error;
err:
	return -1;
}

PRIVATE struct type_nsi tpconst dict_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE | TYPE_SEQX_FRESIZABLE,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&dict_nsi_getsize,
			/* .nsi_nextkey    = */ (dfunptr_t)&dictiterator_next_key,
			/* .nsi_nextvalue  = */ (dfunptr_t)&dictiterator_next_value,
			/* .nsi_getdefault = */ (dfunptr_t)&DeeDict_GetItemDef,
			/* .nsi_setdefault = */ (dfunptr_t)&dict_nsi_setdefault,
			/* .nsi_updateold  = */ (dfunptr_t)&dict_nsi_updateold,
			/* .nsi_insertnew  = */ (dfunptr_t)&dict_nsi_insertnew
		}
	}
};

PRIVATE struct type_seq dict_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dict_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dict_getitem,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_delitem,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&dict_setitem,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &dict_nsi
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_repr(Dict *__restrict self) {
	struct unicode_printer p;
	dssize_t error;
	struct dict_item *iter, *end;
	bool is_first;
	struct dict_item *vector;
	size_t mask;
again:
	unicode_printer_init(&p);
	if (UNICODE_PRINTER_PRINT(&p, "{ ") < 0)
		goto err;
	DeeDict_LockRead(self);
	is_first = true;
	vector   = self->d_elem;
	mask     = self->d_mask;
	end      = (iter = vector) + (mask + 1);
	for (; iter != end; ++iter) {
		DREF DeeObject *key, *value;
		if (iter->di_key == NULL ||
		    iter->di_key == dummy)
			continue;
		key   = iter->di_key;
		value = iter->di_value;
		Dee_Incref(key);
		Dee_Incref(value);
		DeeDict_LockEndRead(self);
		/* Print this key/value pair. */
		error = unicode_printer_printf(&p, "%s%r: %r", is_first ? "" : ", ", key, value);
		Dee_Decref(value);
		Dee_Decref(key);
		if unlikely(error < 0)
			goto err;
		is_first = false;
		DeeDict_LockRead(self);
		if (self->d_elem != vector ||
		    self->d_mask != mask)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	if unlikely((is_first ? unicode_printer_putascii(&p, '}')
		                   : UNICODE_PRINTER_PRINT(&p, " }")) < 0)
	goto err;
	return unicode_printer_pack(&p);
restart:
	DeeDict_LockEndRead(self);
	unicode_printer_fini(&p);
	goto again;
err:
	unicode_printer_fini(&p);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_bool(Dict *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return self->d_used != 0;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->d_used) != 0;
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_init(Dict *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	if unlikely(DeeArg_Unpack(argc, argv, "o:Dict", &seq))
		goto err;
	return dict_init_sequence(self, seq);
err:
	return -1;
}


INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_newproxy(Dict *self,
              DeeTypeObject *proxy_type);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_keys(DeeDictObject *__restrict self) {
	return dict_newproxy(self, &DeeDictKeys_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_items(DeeDictObject *__restrict self) {
	return dict_newproxy(self, &DeeDictItems_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_values(DeeDictObject *__restrict self) {
	return dict_newproxy(self, &DeeDictValues_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_doclear(Dict *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	dict_clear(self);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_get(Dict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *def = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:get", &key, &def))
		goto err;
	return DeeDict_GetItemDef((DeeObject *)self, key, def);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_pop(Dict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *def = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:pop", &key, &def))
		goto err;
	return dict_popitem(self, key, def);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_popsomething(Dict *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	struct dict_item *iter;
	if (DeeArg_Unpack(argc, argv, ":popitem"))
		goto err;
	/* Allocate a tuple which we're going to fill with some key-value pair. */
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err;
	DeeDict_LockWrite(self);
	if unlikely(!self->d_used) {
		DeeDict_LockEndWrite(self);
		DeeTuple_FreeUninitialized(result);
		err_empty_sequence((DeeObject *)self);
		goto err;
	}
	iter = self->d_elem;
	while (!iter->di_key || iter->di_key == dummy) {
		ASSERT(iter != self->d_elem + self->d_mask);
		++iter;
	}
	DeeTuple_SET(result, 0, iter->di_key);   /* Inherit reference. */
	DeeTuple_SET(result, 1, iter->di_value); /* Inherit reference. */
	Dee_Incref(dummy);
	iter->di_key   = dummy;
	iter->di_value = NULL;
	ASSERT(self->d_used);
	if (--self->d_used <= self->d_size / 3)
		dict_rehash(self, -1);
	DeeDict_LockEndWrite(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_setdefault(Dict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *value = Dee_None, *old_value;
	int error;
	if (DeeArg_Unpack(argc, argv, "o|o:setdefault", &key, &value))
		goto err;
	error = dict_setitem_ex(self, key, value, SETITEM_SETNEW, &old_value);
	if unlikely(error < 0)
		goto err;
	if (error == 1)
		return old_value;
	return_reference_(value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_setold(Dict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *value;
	int error;
	if (DeeArg_Unpack(argc, argv, "oo:setold", &key, &value))
		goto err;
	error = dict_setitem_ex(self, key, value, SETITEM_SETOLD, NULL);
	if unlikely(error < 0)
		goto err;
	return_bool_(error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_setnew(Dict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *value;
	int error;
	if (DeeArg_Unpack(argc, argv, "oo:setnew", &key, &value))
		goto err;
	error = dict_setitem_ex(self, key, value, SETITEM_SETNEW, NULL);
	if unlikely(error < 0)
		goto err;
	return_bool_(!error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_setold_ex(Dict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *value, *old_value, *result;
	int error;
	if (DeeArg_Unpack(argc, argv, "oo:setold_ex", &key, &value))
		goto err;
	error = dict_setitem_ex(self, key, value, SETITEM_SETOLD, &old_value);
	if unlikely(error < 0)
		goto err;
	if (error == 1) {
		result = DeeTuple_Pack(2, Dee_True, old_value);
		Dee_Decref(old_value);
	} else {
		result = DeeTuple_Pack(2, Dee_False, Dee_None);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_setnew_ex(Dict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *value, *old_value, *result;
	int error;
	if (DeeArg_Unpack(argc, argv, "oo:setnew_ex", &key, &value))
		goto err;
	error = dict_setitem_ex(self, key, value, SETITEM_SETNEW, &old_value);
	if unlikely(error < 0)
		goto err;
	if (error == 1) {
		result = DeeTuple_Pack(2, Dee_False, old_value);
		Dee_Decref(old_value);
	} else {
		result = DeeTuple_Pack(2, Dee_True, Dee_None);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_update(Dict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *items, *iterator;
	int error;
	if (DeeArg_Unpack(argc, argv, "o:update", &items))
		goto err;
	iterator = DeeObject_IterSelf(items);
	error    = dict_insert_iterator(self, iterator);
	Dee_Decref(iterator);
	if unlikely(error)
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_sizeof(Dict *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":__sizeof__"))
		goto err;
	return DeeInt_NewSize(sizeof(Dict) +
	                      ((self->d_mask + 1) *
	                       sizeof(struct dict_item)));
err:
	return NULL;
}

INTDEF struct keyword seq_byhash_kwlist[];

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_byhash(Dict *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DeeObject *template_;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_byhash_kwlist, "o:byhash", &template_))
		goto err;
	return DeeDict_ByHash((DeeObject *)self,
	                      DeeObject_Hash(template_),
	                      false);
err:
	return NULL;
}


/* TODO: Introduce a function `__missing__(key)->object' that is called
 *       when a key can't be found (won't be called by GetItemDef()).
 *       The default implementation of this function should then throw
 *       a `KeyError', rather than `operator []' itself.
 *    -> User-classes can then override that function to implement
 *       some custom behavior for dealing with missing keys.
 */

DOC_REF(map_get_doc);
DOC_REF(map_byhash_doc);

PRIVATE struct type_method tpconst dict_methods[] = {
	{ DeeString_STR(&str_get),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_get,
	  DOC_GET(map_get_doc) },
	{ DeeString_STR(&str_pop),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_pop,
	  DOC("(key)->\n"
	      "(key,def)->\n"
	      "@throw KeyError No @def was given and @key was not found\n"
	      "Delete @key from @this and return its previously assigned value or @def when @key had no item associated") },
	{ DeeString_STR(&str_clear),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_doclear,
	  DOC("()\n"
	      "Clear all values from @this :Dict") },
	{ "popitem",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_popsomething,
	  DOC("->?T2?O?O\n"
	      "@return A random pair key-value pair that has been removed\n"
	      "@throw ValueError @this :Dict was empty") },
	{ "setdefault",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_setdefault,
	  DOC("(key,def=!N)->\n"
	      "@return The object currently assigned to @key\n"
	      "Lookup @key in @this Dict and return its value if found. Otherwise, assign @def to @key and return it instead") },
	{ "setold",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_setold,
	  DOC("(key,value)->?Dbool\n"
	      "@return Indicative of @value having been assigned to @key\n"
	      "Assign @value to @key, only succeeding when @key already existed to begin with") },
	{ "setnew",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_setnew,
	  DOC("(key,value)->?Dbool\n"
	      "@return Indicative of @value having been assigned to @key\n"
	      "Assign @value to @key, only succeeding when @key didn't exist before") },
	{ "setold_ex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_setold_ex,
	  DOC("(key,value)->?T2?Dbool?O\n"
	      "@return A pair of values (new-value-was-assigned, old-value-or-none)\n"
	      "Same as #setold but also return the previously assigned object") },
	{ "setnew_ex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_setnew_ex,
	  DOC("(key,value)->?T2?Dbool?O\n"
	      "@return A pair of values (new-value-was-assigned, old-value-or-none)\n"
	      "Same as #setnew but return the previously assigned object on failure") },
	{ "update",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_update,
	  DOC("(items:?S?T2?O?O)\n"
	      "Iterate @items and unpack each element into 2 others, using them as "
	      "key and value to insert into @this Dict") },
	{ "byhash",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_byhash,
	  DOC_GET(map_byhash_doc),
	  TYPE_METHOD_FKWDS },
	{ "__sizeof__",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_sizeof,
	  DOC("->?Dint") },
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	/* Old function names. */
	{ "insert_all", (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&dict_update,
	  DOC("(items:?S?T2?O?O)\n"
	      "A deprecated alias for ?#update") },
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	{ NULL }
};

#ifndef CONFIG_NO_DEEMON_100_COMPAT
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL set_get_maxloadfactor(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL set_del_maxloadfactor(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL set_set_maxloadfactor(DeeObject *__restrict self, DeeObject *__restrict value);
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

INTDEF struct type_getset tpconst dict_getsets[];
INTERN struct type_getset tpconst dict_getsets[] = {
	{ "keys",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_keys,
	  NULL,
	  NULL,
	  DOC("->?AKeys?.\n"
	      "@return A proxy sequence for viewing the keys of @this :Dict") },
	{ "values",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_values,
	  NULL,
	  NULL,
	  DOC("->?AValues?.\n"
	      "@return A proxy sequence for viewing the values of @this :Dict") },
	{ "items",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_items,
	  NULL,
	  NULL,
	  DOC("->?AItems?.\n"
	      "@return A proxy sequence for viewing the key-value pairs of @this :Dict") },
	{ "frozen",
	  &DeeRoDict_FromSequence,
	  NULL,
	  NULL,
	  DOC("->?Ert:RoDict\n"
	      "Returns a read-only (frozen) copy of @this Dict") },
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	{ "max_load_factor",
	  &set_get_maxloadfactor,
	  &set_del_maxloadfactor,
	  &set_set_maxloadfactor,
	  DOC("->?Dfloat\n"
	      "Deprecated. Always returns ${1.0}, with del/set being ignored") },
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	{ NULL }
};



INTDEF DeeTypeObject DictIterator_Type;
PRIVATE struct type_member tpconst dict_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DictIterator_Type),
	TYPE_MEMBER_CONST("Proxy", &DeeDictProxy_Type),
	TYPE_MEMBER_CONST("Keys", &DeeDictKeys_Type),
	TYPE_MEMBER_CONST("Items", &DeeDictItems_Type),
	TYPE_MEMBER_CONST("Values", &DeeDictValues_Type),
	TYPE_MEMBER_CONST("Frozen", &DeeRoDict_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_gc tpconst dict_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&dict_clear
};

PUBLIC DeeTypeObject DeeDict_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Dict),
	/* .tp_doc      = */ DOC("The builtin mapping object for translating keys to items\n"
	                         "\n"
	                         "()\n"
	                         "Create a new, empty Dict\n"
	                         "\n"
	                         "(items:?S?T2?O?O)\n"
	                         "Create a new Dict, using key-items pairs extracted from @items.\n"
	                         "Iterate @items and unpack each element into 2 others, using them "
	                         "as key and value to insert into @this Dict"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(Dict),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&dict_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&dict_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&dict_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&dict_init,
				TYPE_FIXED_ALLOCATOR_GC(Dict)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dict_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&dict_deepload
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&dict_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dict_visit,
	/* .tp_gc            = */ &dict_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dict_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ dict_methods,
	/* .tp_getsets       = */ dict_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dict_class_members
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_DICT_C */
