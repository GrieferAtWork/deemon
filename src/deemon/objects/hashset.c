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
#ifndef GUARD_DEEMON_OBJECTS_HASHSET_C
#define GUARD_DEEMON_OBJECTS_HASHSET_C 1

#include <deemon/hashset.h>
#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dict.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/system-features.h> /* memcpyc(), ... */
#include <deemon/util/atomic.h>

#include <hybrid/sched/yield.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

typedef DeeHashSetObject HashSet;

#define empty_hashset_items ((struct Dee_hashset_item *)DeeHashSet_EmptyItems)
#define dummy               (&DeeDict_Dummy)

/* Create a new HashSet by inheriting a set of passed key-item pairs.
 * @param: items:     A vector containing `num_items' elements,
 *                    even ones being keys and odd ones being items.
 * @param: num_items: The number of items passed.
 * WARNING: This function does _NOT_ inherit the passed vector, but _ONLY_ its elements! */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeHashSet_NewItemsInherited(size_t num_items,
                             /*inherit(on_success)*/ DREF DeeObject **items) {
	DREF HashSet *result;

	/* Allocate the set object. */
	result = DeeGCObject_MALLOC(HashSet);
	if unlikely(!result)
		goto done;
	if unlikely(!num_items) {
		/* Special case: allocate an empty set. */
		result->hs_mask = 0;
		result->hs_size = 0;
		result->hs_used = 0;
		result->hs_elem = empty_hashset_items;
	} else {
		size_t min_mask = 16 - 1, mask;

		/* Figure out how large the mask of the set is going to be. */
		while ((num_items & min_mask) != num_items)
			min_mask = (min_mask << 1) | 1;

		/* Prefer using a mask of one greater level to improve performance. */
		mask           = (min_mask << 1) | 1;
		result->hs_elem = (struct hashset_item *)Dee_TryCallocc(mask + 1, sizeof(struct hashset_item));
		if unlikely(!result->hs_elem) {
			/* Try one level less if that failed. */
			mask           = min_mask;
			result->hs_elem = (struct hashset_item *)Dee_Callocc(mask + 1, sizeof(struct hashset_item));
			if unlikely(!result->hs_elem)
				goto err_r;
		}

		/* Without any dummy items, these are identical. */
		result->hs_mask = mask;
		result->hs_used = num_items;
		result->hs_size = num_items;
next_key:
		while (num_items--) {
			DREF DeeObject *key = *items++;
			dhash_t i, perturb, hash = DeeObject_Hash(key);
			perturb = i = hash & mask;
			for (;; DeeHashSet_HashNx(i, perturb)) {
				struct hashset_item *item = &result->hs_elem[i & mask];
				if (item->hsi_key) { /* Already in use */
					int temp;
					if likely(item->hsi_hash != hash)
						continue;
					temp = DeeObject_CompareEq(item->hsi_key, key);
					if likely(temp == 0)
						continue;
					if unlikely(temp < 0)
						goto err_r;

					/* Duplicate key. */
					--result->hs_used;
					--result->hs_size;
					goto next_key;
				}
				item->hsi_hash = hash;
				item->hsi_key  = key; /* Inherit reference. */
				break;
			}
		}
	}
	Dee_atomic_rwlock_init(&result->hs_lock);

	/* Initialize and start tracking the new set. */
	weakref_support_init(result);
	DeeObject_Init(result, &DeeHashSet_Type);
	DeeGC_Track((DeeObject *)result);
done:
	return (DREF DeeObject *)result;
err_r:
	Dee_Free(result->hs_elem);
	DeeGCObject_FREE(result);
	return NULL;
}


PRIVATE NONNULL((1)) void DCALL
hashset_fini(HashSet *__restrict self);

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashset_init_iterator(HashSet *__restrict self,
                      DeeObject *__restrict iterator) {
	DREF DeeObject *elem;
	self->hs_mask = 0;
	self->hs_size = 0;
	self->hs_used = 0;
	self->hs_elem = empty_hashset_items;
	Dee_atomic_rwlock_init(&self->hs_lock);
	weakref_support_init(self);
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		if unlikely(DeeHashSet_Insert((DeeObject *)self, elem) < 0)
			goto err_elem;
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	if unlikely(!elem)
		goto err;
	return 0;
err_elem:
	Dee_Decref(elem);
err:
	hashset_fini(self);
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL hashset_copy(HashSet *__restrict self, HashSet *__restrict other);

STATIC_ASSERT(sizeof(struct hashset_item) == sizeof(struct roset_item));
STATIC_ASSERT(offsetof(struct hashset_item, hsi_key) == offsetof(struct roset_item, rsi_key));
STATIC_ASSERT(offsetof(struct hashset_item, hsi_hash) == offsetof(struct roset_item, rsi_hash));

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashset_init_sequence(HashSet *__restrict self,
                      DeeObject *__restrict sequence) {
	DREF DeeObject *iterator;
	int error;
	DeeTypeObject *tp = Dee_TYPE(sequence);
	if (tp == &DeeHashSet_Type)
		return hashset_copy(self, (HashSet *)sequence);

	/* Optimizations for `_RoSet' */
	if (tp == &DeeRoSet_Type) {
		struct hashset_item *iter, *end;
		DeeRoSetObject *src = (DeeRoSetObject *)sequence;
		Dee_atomic_rwlock_init(&self->hs_lock);
		self->hs_mask = src->rs_mask;
		self->hs_used = self->hs_size = src->rs_size;
		if unlikely(!self->hs_size) {
			self->hs_elem = (struct hashset_item *)empty_hashset_items;
		} else {
			self->hs_elem = (struct hashset_item *)Dee_Mallocc(src->rs_mask + 1,
			                                                  sizeof(struct hashset_item));
			if unlikely(!self->hs_elem)
				goto err;
			iter = (struct hashset_item *)memcpyc(self->hs_elem, src->rs_elem,
			                                      self->hs_mask + 1,
			                                      sizeof(struct hashset_item));
			end  = iter + (self->hs_mask + 1);
			for (; iter < end; ++iter)
				Dee_XIncref(iter->hsi_key);
		}
		weakref_support_init(self);
		return 0;
	}
	/* TODO: Fast-sequence support */
	iterator = DeeObject_IterSelf(sequence);
	if unlikely(!iterator)
		goto err;
	error = hashset_init_iterator(self, iterator);
	Dee_Decref(iterator);
	return error;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeHashSet_FromIterator(DeeObject *__restrict self) {
	DREF HashSet *result;
	result = DeeGCObject_MALLOC(HashSet);
	if unlikely(!result)
		goto done;
	if unlikely(hashset_init_iterator(result, self))
		goto err;
	DeeObject_Init(result, &DeeHashSet_Type);
	DeeGC_Track((DeeObject *)result);
done:
	return (DREF DeeObject *)result;
err:
	DeeGCObject_FREE(result);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeHashSet_FromSequence(DeeObject *__restrict self) {
	DREF HashSet *result;
	result = DeeGCObject_MALLOC(HashSet);
	if unlikely(!result)
		goto done;
	if unlikely(hashset_init_sequence(result, self))
		goto err;
	DeeObject_Init(result, &DeeHashSet_Type);
	DeeGC_Track((DeeObject *)result);
done:
	return (DREF DeeObject *)result;
err:
	DeeGCObject_FREE(result);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
hashset_ctor(HashSet *__restrict self) {
	self->hs_mask = 0;
	self->hs_size = 0;
	self->hs_used = 0;
	self->hs_elem = empty_hashset_items;
	Dee_atomic_rwlock_init(&self->hs_lock);
	weakref_support_init(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashset_copy(HashSet *__restrict self,
         HashSet *__restrict other) {
	struct hashset_item *iter, *end;
	Dee_atomic_rwlock_init(&self->hs_lock);
again:
	DeeHashSet_LockRead(other);
	self->hs_mask = other->hs_mask;
	self->hs_size = other->hs_size;
	self->hs_used = other->hs_used;
	if ((self->hs_elem = other->hs_elem) != empty_hashset_items) {
		self->hs_elem = (struct hashset_item *)Dee_TryMallocc(other->hs_mask + 1,
		                                                      sizeof(struct hashset_item));
		if unlikely(!self->hs_elem) {
			DeeHashSet_LockEndRead(other);
			if (Dee_CollectMemory((other->hs_mask + 1) * sizeof(struct hashset_item)))
				goto again;
			goto err;
		}
		iter = (struct hashset_item *)memcpyc(self->hs_elem, other->hs_elem,
		                                      self->hs_mask + 1,
		                                      sizeof(struct hashset_item));
		end  = iter + (self->hs_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->hsi_key)
				continue;
			Dee_Incref(iter->hsi_key);
		}
	}
	DeeHashSet_LockEndRead(other);
	weakref_support_init(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hashset_deepload(HashSet *__restrict self) {
	DREF DeeObject **new_items, **items = NULL;
	size_t i, hash_i, item_count, ols_item_count = 0;
	struct hashset_item *new_map, *ols_map;
	size_t new_mask;
	for (;;) {
		DeeHashSet_LockRead(self);
		/* Optimization: if the Set is empty, then there's nothing to copy! */
		if (self->hs_elem == empty_hashset_items) {
			DeeHashSet_LockEndRead(self);
			return 0;
		}
		item_count = self->hs_used;
		if (item_count <= ols_item_count)
			break;
		DeeHashSet_LockEndRead(self);
		new_items = (DREF DeeObject **)Dee_Reallocc(items, item_count,
		                                            sizeof(DREF DeeObject *));
		if unlikely(!new_items)
			goto err_items;
		ols_item_count = item_count;
		items          = new_items;
	}

	/* Copy all used items. */
	for (i = 0, hash_i = 0; i < item_count; ++hash_i) {
		ASSERT(hash_i <= self->hs_mask);
		if (self->hs_elem[hash_i].hsi_key == NULL)
			continue;
		if (self->hs_elem[hash_i].hsi_key == dummy)
			continue;
		items[i] = self->hs_elem[hash_i].hsi_key;
		Dee_Incref(items[i]);
		++i;
	}
	DeeHashSet_LockEndRead(self);

	/* With our own local copy of all items being
	 * used, replace all of them with deep copies. */
	for (i = 0; i < item_count; ++i) {
		if (DeeObject_InplaceDeepCopy(&items[i]))
			goto err_items_v;
	}
	new_mask = 1;
	while ((item_count & new_mask) != item_count)
		new_mask = (new_mask << 1) | 1;
	new_map = (struct hashset_item *)Dee_Callocc(new_mask + 1,
	                                             sizeof(struct hashset_item));
	if unlikely(!new_map)
		goto err_items_v;

	/* Insert all the copied items into the new map. */
	for (i = 0; i < item_count; ++i) {
		dhash_t j, perturb, hash;
		hash    = DeeObject_Hash(items[i]);
		perturb = j = hash & new_mask;
		for (;; DeeHashSet_HashNx(j, perturb)) {
			struct hashset_item *item = &new_map[j & new_mask];
			if (item->hsi_key) {
				/* Check if deepcopy caused one of the elements to get duplicated. */
				if unlikely(item->hsi_key == items[i]) {
remove_duplicate_key:
					Dee_Decref(items[i]);
					--item_count;
					memmovedownc(&items[i],
					             &items[i + 1],
					             item_count - i,
					             sizeof(struct dict_item));
					break;
				}
				if (Dee_TYPE(item->hsi_key) == Dee_TYPE(items[i])) {
					int error;
					error = DeeObject_CompareEq(item->hsi_key, items[i]);
					if unlikely(error < 0)
						goto err_items_v_new_map;
					if (error)
						goto remove_duplicate_key;
				}

				/* Slot already in use */
				continue;
			}
			item->hsi_hash = hash;
			item->hsi_key  = items[i]; /* Inherit reference. */
			break;
		}
	}
	DeeHashSet_LockWrite(self);
	i             = self->hs_mask + 1;
	self->hs_mask = new_mask;
	self->hs_used = item_count;
	self->hs_size = item_count;
	ols_map       = self->hs_elem;
	self->hs_elem = new_map;
	DeeHashSet_LockEndWrite(self);
	if (ols_map != empty_hashset_items) {
		while (i--)
			Dee_XDecref(ols_map[i].hsi_key);
		Dee_Free(ols_map);
	}
	Dee_Free(items);
	return 0;
err_items_v_new_map:
	Dee_Free(new_map);
err_items_v:
	Dee_Decrefv(items, item_count);
err_items:
	Dee_Free(items);
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
hashset_fini(HashSet *__restrict self) {
	weakref_support_fini(self);
	ASSERT((self->hs_elem == empty_hashset_items) == (self->hs_mask == 0));
	ASSERT((self->hs_elem == empty_hashset_items) == (self->hs_size == 0));
	ASSERT(self->hs_used <= self->hs_size);
	if (self->hs_elem != empty_hashset_items) {
		struct hashset_item *iter, *end;
		end = (iter = self->hs_elem) + (self->hs_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->hsi_key)
				continue;
			Dee_Decref(iter->hsi_key);
		}
		Dee_Free(self->hs_elem);
	}
}

PRIVATE NONNULL((1)) void DCALL
hashset_clear(HashSet *__restrict self) {
	struct hashset_item *elem;
	size_t mask;

	/* Extract the vector and mask. */
	DeeHashSet_LockWrite(self);
	ASSERT((self->hs_elem == empty_hashset_items) == (self->hs_mask == 0));
	ASSERT((self->hs_elem == empty_hashset_items) == (self->hs_used == 0));
	ASSERT((self->hs_elem == empty_hashset_items) == (self->hs_size == 0));
	ASSERT(self->hs_used <= self->hs_size);
	elem          = self->hs_elem;
	mask          = self->hs_mask;
	self->hs_elem = empty_hashset_items;
	self->hs_mask = 0;
	self->hs_used = 0;
	self->hs_size = 0;
	DeeHashSet_LockEndWrite(self);

	/* Destroy the vector. */
	if (elem != empty_hashset_items) {
		struct hashset_item *iter, *end;
		end = (iter = elem) + (mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->hsi_key)
				continue;
			Dee_Decref(iter->hsi_key);
		}
		Dee_Free(elem);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
hashset_visit(HashSet *__restrict self, dvisit_t proc, void *arg) {
	DeeHashSet_LockRead(self);
	ASSERT((self->hs_elem == empty_hashset_items) == (self->hs_mask == 0));
	ASSERT((self->hs_elem == empty_hashset_items) == (self->hs_used == 0));
	ASSERT((self->hs_elem == empty_hashset_items) == (self->hs_size == 0));
	ASSERT(self->hs_used <= self->hs_size);
	if (self->hs_elem != empty_hashset_items) {
		struct hashset_item *iter, *end;
		end = (iter = self->hs_elem) + (self->hs_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->hsi_key)
				continue;
			/* Visit all keys and associated values. */
			Dee_Visit(iter->hsi_key);
		}
	}
	DeeHashSet_LockEndRead(self);
}


/* Resize the hash size by a factor of 2 and re-insert all elements.
 * When `sizedir > 0', increase the hash; When `sizedir < 0', decrease it.
 * During this process all dummy items are discarded.
 * @return: true:  Successfully rehashed the set.
 * @return: false: Not enough memory. - The caller should collect some and try again. */
PRIVATE NONNULL((1)) bool DCALL
hashset_rehash(HashSet *__restrict self, int sizedir) {
	struct hashset_item *new_vector, *iter, *end;
	size_t new_mask = self->hs_mask;
	if (sizedir > 0) {
		new_mask = (new_mask << 1) | 1;
		if unlikely(new_mask == 1)
			new_mask = 16 - 1; /* Start out bigger than 2. */
	} else if (sizedir < 0) {
		if unlikely(!self->hs_used) {
			ASSERT(!self->hs_used);

			/* Special case: delete the vector. */
			if (self->hs_size) {
				ASSERT(self->hs_elem != empty_hashset_items);
				/* Must discard dummy items. */
				end = (iter = self->hs_elem) + (self->hs_mask + 1);
				for (; iter < end; ++iter) {
					ASSERT(iter->hsi_key == NULL ||
					       iter->hsi_key == dummy);
					if (iter->hsi_key == dummy)
						Dee_DecrefNokill(dummy);
				}
			}
			if (self->hs_elem != empty_hashset_items)
				Dee_Free(self->hs_elem);
			self->hs_elem = empty_hashset_items;
			self->hs_mask = 0;
			self->hs_size = 0;
			return true;
		}
		new_mask = (new_mask >> 1);
		if (self->hs_used >= new_mask)
			return true;
	}
	ASSERT(self->hs_used < new_mask);
	ASSERT(self->hs_used <= self->hs_size);
	new_vector = (struct hashset_item *)Dee_TryCallocc(new_mask + 1, sizeof(struct hashset_item));
	if unlikely(!new_vector)
		return false;
	ASSERT((self->hs_elem == empty_hashset_items) == (self->hs_mask == 0));
	ASSERT((self->hs_elem == empty_hashset_items) == (self->hs_used == 0));
	ASSERT((self->hs_elem == empty_hashset_items) == (self->hs_size == 0));
	if (self->hs_elem != empty_hashset_items) {
		/* Re-insert all existing items into the new set vector. */
		end = (iter = self->hs_elem) + (self->hs_mask + 1);
		for (; iter < end; ++iter) {
			struct hashset_item *item;
			dhash_t i, perturb;

			/* Skip dummy keys. */
			if (iter->hsi_key == dummy)
				continue;
			perturb = i = iter->hsi_hash & new_mask;
			for (;; DeeHashSet_HashNx(i, perturb)) {
				item = &new_vector[i & new_mask];
				if (!item->hsi_key)
					break; /* Empty slot found. */
			}

			/* Transfer this object. */
			item->hsi_key  = iter->hsi_key;
			item->hsi_hash = iter->hsi_hash;
		}
		Dee_Free(self->hs_elem);

		/* With all dummy items gone, the size now equals what is actually used. */
		self->hs_size = self->hs_used;
	}
	ASSERT(self->hs_size == self->hs_used);
	self->hs_mask = new_mask;
	self->hs_elem = new_vector;
	return true;
}






/* Unifies a given object, either inserting it into the set and re-returning
 * it, or returning another, identical instance already apart of the set. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeHashSet_Unify(DeeObject *self, DeeObject *search_item) {
	HashSet *me = (HashSet *)self;
	size_t mask;
	struct hashset_item *vector;
	int error;
	struct hashset_item *first_dummy;
	dhash_t i, perturb, hash = DeeObject_Hash(search_item);
again_lock:
	DeeHashSet_LockRead(me);
again:
	first_dummy = NULL;
	vector      = me->hs_elem;
	mask        = me->hs_mask;
	perturb = i = hash & mask;
	for (;; DeeHashSet_HashNx(i, perturb)) {
		DREF DeeObject *item_key;
		struct hashset_item *item = &vector[i & mask];
		if (!item->hsi_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->hsi_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (item->hsi_hash != hash)
			continue; /* Non-matching hash */
		item_key = item->hsi_key;
		Dee_Incref(item_key);
		DeeHashSet_LockEndRead(me);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_CompareEq(search_item, item_key);
		Dee_Decref(item_key);
		if unlikely(error < 0)
			goto err; /* Error in compare operator. */
		if (error > 0) {
			DeeHashSet_LockWrite(me);

			/* Check if the set was modified. */
			if (me->hs_elem != vector ||
			    me->hs_mask != mask ||
			    item->hsi_key != item_key) {
				DeeHashSet_LockDowngrade(me);
				goto again;
			}
			Dee_Incref(item_key);
			DeeHashSet_LockEndWrite(me);
			return item_key; /* Already exists. */
		}
		DeeHashSet_LockRead(me);

		/* Check if the set was modified. */
		if (me->hs_elem != vector ||
		    me->hs_mask != mask ||
		    item->hsi_key != item_key)
			goto again;
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeHashSet_LockUpgrade(me)) {
		DeeHashSet_LockEndWrite(me);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if (first_dummy && me->hs_size + 1 < me->hs_mask) {
		ASSERT(first_dummy != empty_hashset_items);
		ASSERT(!first_dummy->hsi_key ||
		       first_dummy->hsi_key == dummy);
		if (first_dummy->hsi_key)
			Dee_DecrefNokill(first_dummy->hsi_key);

		/* Fill in the target slot. */
		first_dummy->hsi_key  = search_item;
		first_dummy->hsi_hash = hash;
		Dee_Incref(search_item);
		++me->hs_used;
		++me->hs_size;

		/* Try to keep the set vector big at least twice as big as the element count. */
		if (me->hs_size * 2 > me->hs_mask)
			hashset_rehash(me, 1);
		DeeHashSet_LockEndWrite(me);
		Dee_Incref(search_item);
		return search_item; /* New item. */
	}

	/* Rehash the set and try again. */
	if (hashset_rehash(me, 1)) {
		DeeHashSet_LockDowngrade(me);
		goto again;
	}
	DeeHashSet_LockEndWrite(me);
	if (Dee_CollectMemory(1))
		goto again_lock;
err:
	return NULL;
}

/* @return:  1: Successfully inserted/removed the object.
 * @return:  0: An identical object already exists/was already removed.
 * @return: -1: An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeHashSet_Insert(DeeObject *self,
                  DeeObject *search_item) {
	HashSet *me = (HashSet *)self;
	size_t mask;
	struct hashset_item *vector;
	int error;
	struct hashset_item *first_dummy;
	dhash_t i, perturb, hash = DeeObject_Hash(search_item);
again_lock:
	DeeHashSet_LockRead(me);
again:
	first_dummy = NULL;
	vector      = me->hs_elem;
	mask        = me->hs_mask;
	perturb = i = hash & mask;
	for (;; DeeHashSet_HashNx(i, perturb)) {
		DREF DeeObject *item_key;
		struct hashset_item *item = &vector[i & mask];
		if (!item->hsi_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->hsi_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (item->hsi_hash != hash)
			continue; /* Non-matching hash */
		item_key = item->hsi_key;
		Dee_Incref(item_key);
		DeeHashSet_LockEndRead(me);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_CompareEq(search_item, item_key);
		Dee_Decref(item_key);
		if unlikely(error < 0)
			goto err; /* Error in compare operator. */
		DeeHashSet_LockRead(me);

		/* Check if the set was modified. */
		if (me->hs_elem != vector ||
		    me->hs_mask != mask ||
		    item->hsi_key != item_key)
			goto again;
		if (error > 0) {
			DeeHashSet_LockEndRead(me);
			return 0; /* Already exists. */
		}
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeHashSet_LockUpgrade(me)) {
		DeeHashSet_LockEndWrite(me);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if (first_dummy && me->hs_size + 1 < me->hs_mask) {
		ASSERT(first_dummy != empty_hashset_items);
		ASSERT(!first_dummy->hsi_key ||
		       first_dummy->hsi_key == dummy);
		if (first_dummy->hsi_key)
			Dee_DecrefNokill(first_dummy->hsi_key);

		/* Fill in the target slot. */
		first_dummy->hsi_key  = search_item;
		first_dummy->hsi_hash = hash;
		Dee_Incref(search_item);
		++me->hs_used;
		++me->hs_size;

		/* Try to keep the set vector big at least twice as big as the element count. */
		if (me->hs_size * 2 > me->hs_mask)
			hashset_rehash(me, 1);
		DeeHashSet_LockEndWrite(me);
		return 1; /* New item. */
	}

	/* Rehash the set and try again. */
	if (hashset_rehash(me, 1)) {
		DeeHashSet_LockDowngrade(me);
		goto again;
	}
	DeeHashSet_LockEndWrite(me);
	if (Dee_CollectMemory(1))
		goto again_lock;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeHashSet_Remove(DeeObject *self,
                  DeeObject *search_item) {
	HashSet *me = (HashSet *)self;
	size_t mask;
	struct hashset_item *vector;
	dhash_t i, perturb;
	int error;
	dhash_t hash = DeeObject_Hash(search_item);
	DeeHashSet_LockRead(me);
restart:
	vector  = me->hs_elem;
	mask    = me->hs_mask;
	perturb = i = hash & mask;
	for (;; DeeHashSet_HashNx(i, perturb)) {
		DREF DeeObject *item_key;
		struct hashset_item *item = &vector[i & mask];
		if (!item->hsi_key)
			break; /* Not found */
		if (item->hsi_hash != hash)
			continue; /* Non-matching hash */
		if (item->hsi_key == dummy)
			continue; /* Dummy key. */
		item_key = item->hsi_key;
		Dee_Incref(item_key);
		DeeHashSet_LockEndRead(me);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_CompareEq(search_item, item_key);
		Dee_Decref(item_key);
		if unlikely(error < 0)
			goto err; /* Error in compare operator. */
		if (error > 0) {
			/* Found it! */
			DeeHashSet_LockWrite(me);

			/* Check if the set was modified. */
			if (me->hs_elem != vector ||
			    me->hs_mask != mask ||
			    item->hsi_key != item_key) {
				DeeHashSet_LockDowngrade(me);
				goto restart;
			}
			item->hsi_key = dummy;
			Dee_Incref(dummy);
			ASSERT(me->hs_used);
			if (--me->hs_used <= me->hs_size / 2)
				hashset_rehash(me, -1);
			ASSERT((me->hs_elem == empty_hashset_items) == (me->hs_mask == 0));
			ASSERT((me->hs_elem == empty_hashset_items) == (me->hs_used == 0));
			ASSERT((me->hs_elem == empty_hashset_items) == (me->hs_size == 0));
			DeeHashSet_LockEndWrite(me);
			Dee_Decref(item_key);
			return 1;
		}
		DeeHashSet_LockRead(me);

		/* Check if the set was modified. */
		if (me->hs_elem != vector ||
		    me->hs_mask != mask ||
		    item->hsi_key != item_key)
			goto restart;
	}
	DeeHashSet_LockEndRead(me);
	return 0;
err:
	return -1;
}



PUBLIC WUNUSED NONNULL((1, 2)) DREF /*String*/ DeeObject *DCALL
DeeHashSet_UnifyString(DeeObject *__restrict self,
                       char const *__restrict search_item,
                       size_t search_item_length) {
	HashSet *me                      = (HashSet *)self;
	DREF DeeStringObject *result = NULL;
	size_t mask;
	struct hashset_item *vector;
	struct hashset_item *first_dummy;
	dhash_t i, perturb, hash;
	hash = Dee_HashPtr(search_item, search_item_length);
again_lock:
	DeeHashSet_LockRead(me);
again:
	first_dummy = NULL;
	vector      = me->hs_elem;
	mask        = me->hs_mask;
	perturb = i = hash & mask;
	for (;; DeeHashSet_HashNx(i, perturb)) {
		DREF DeeObject *existing_key;
		struct hashset_item *item = &vector[i & mask];
		if (!item->hsi_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (!DeeString_Check(item->hsi_key)) {
			if (item->hsi_key == dummy)
				first_dummy = item;
			continue;
		}
		if (item->hsi_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_EqualsBuf(item->hsi_key, search_item, search_item_length))
			continue; /* Differing strings. */
		existing_key = item->hsi_key;
		Dee_Incref(existing_key);
		DeeHashSet_LockEndRead(me);
		Dee_XDecref(result);
		return existing_key; /* Already exists. */
	}
	if likely(!result) {
		/* Create the actual string that's going to get stored. */
		result = (DREF DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject, s_str) +
		                                                     (search_item_length + 1) * sizeof(char));
		if unlikely(!result) {
			DeeHashSet_LockEndRead(me);
			if (Dee_CollectMemory(offsetof(DeeStringObject, s_str) + (search_item_length + 1) * sizeof(char)))
				goto again_lock;
			return NULL;
		}
		DeeObject_Init(result, &DeeString_Type);
		result->s_data = NULL;
		result->s_hash = hash;
		result->s_len  = search_item_length;
		*(char *)mempcpyc(result->s_str, search_item, search_item_length, sizeof(char)) = '\0';
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeHashSet_LockUpgrade(me)) {
		DeeHashSet_LockEndWrite(me);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if (first_dummy && me->hs_size + 1 < me->hs_mask) {
		ASSERT(first_dummy != empty_hashset_items);
		ASSERT(!first_dummy->hsi_key ||
		       first_dummy->hsi_key == dummy);
		if (first_dummy->hsi_key)
			Dee_DecrefNokill(first_dummy->hsi_key);

		/* Fill in the target slot. */
		first_dummy->hsi_key  = (DREF DeeObject *)result;
		first_dummy->hsi_hash = hash;
		Dee_Incref(result);
		++me->hs_used;
		++me->hs_size;

		/* Try to keep the set vector big at least twice as big as the element count. */
		if (me->hs_size * 2 > me->hs_mask)
			hashset_rehash(me, 1);
		DeeHashSet_LockEndWrite(me);
		return (DREF DeeObject *)result; /* New item. */
	}

	/* Rehash the set and try again. */
	if (hashset_rehash(me, 1)) {
		DeeHashSet_LockDowngrade(me);
		goto again;
	}
	DeeHashSet_LockEndWrite(me);
	if (Dee_CollectMemory(1))
		goto again_lock;
	Dee_Decref(result);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeHashSet_InsertString(DeeObject *__restrict self,
                        char const *__restrict search_item,
                        size_t search_item_length) {
	HashSet *me                        = (HashSet *)self;
	DREF DeeStringObject *new_item = NULL;
	size_t mask;
	struct hashset_item *vector;
	struct hashset_item *first_dummy;
	dhash_t i, perturb, hash;
	hash = Dee_HashPtr(search_item, search_item_length);
again_lock:
	DeeHashSet_LockRead(me);
again:
	first_dummy = NULL;
	vector      = me->hs_elem;
	mask        = me->hs_mask;
	perturb = i = hash & mask;
	for (;; DeeHashSet_HashNx(i, perturb)) {
		struct hashset_item *item = &vector[i & mask];
		if (!item->hsi_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (!DeeString_Check(item->hsi_key)) {
			if (item->hsi_key == dummy)
				first_dummy = item;
			continue;
		}
		if (item->hsi_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_EqualsBuf(item->hsi_key, search_item, search_item_length))
			continue; /* Differing strings. */
		DeeHashSet_LockEndRead(me);
		Dee_XDecref(new_item);
		return 0; /* Already exists. */
	}
	if likely(!new_item) {
		/* Create the actual string that's going to get stored. */
		new_item = (DREF DeeStringObject *)DeeObject_TryMalloc(offsetof(DeeStringObject, s_str) +
		                                                       (search_item_length + 1) * sizeof(char));
		if unlikely(!new_item) {
			DeeHashSet_LockEndRead(me);
			if (Dee_CollectMemory(offsetof(DeeStringObject, s_str) + (search_item_length + 1) * sizeof(char)))
				goto again_lock;
			return -1;
		}
		DeeObject_Init(new_item, &DeeString_Type);
		new_item->s_data = NULL;
		new_item->s_hash = hash;
		new_item->s_len  = search_item_length;
		*(char *)mempcpyc(new_item->s_str, search_item, search_item_length, sizeof(char)) = '\0';
	}
#ifndef CONFIG_NO_THREADS
	if (!DeeHashSet_LockUpgrade(me)) {
		DeeHashSet_LockEndWrite(me);
		SCHED_YIELD();
		goto again_lock;
	}
#endif /* !CONFIG_NO_THREADS */
	if (first_dummy && me->hs_size + 1 < me->hs_mask) {
		ASSERT(first_dummy != empty_hashset_items);
		ASSERT(!first_dummy->hsi_key ||
		       first_dummy->hsi_key == dummy);
		if (first_dummy->hsi_key)
			Dee_DecrefNokill(first_dummy->hsi_key);

		/* Fill in the target slot. */
		first_dummy->hsi_key  = (DREF DeeObject *)new_item; /* Inherit reference. */
		first_dummy->hsi_hash = hash;
		++me->hs_used;
		++me->hs_size;

		/* Try to keep the set vector big at least twice as big as the element count. */
		if (me->hs_size * 2 > me->hs_mask)
			hashset_rehash(me, 1);
		DeeHashSet_LockEndWrite(me);
		return 1; /* New item. */
	}

	/* Rehash the set and try again. */
	if (hashset_rehash(me, 1)) {
		DeeHashSet_LockDowngrade(me);
		goto again;
	}
	DeeHashSet_LockEndWrite(me);
	if (Dee_CollectMemory(1))
		goto again_lock;
	Dee_Decref(new_item);
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeHashSet_RemoveString(DeeObject *__restrict self,
                        char const *__restrict search_item,
                        size_t search_item_length) {
	HashSet *me = (HashSet *)self;
	DeeObject *old_item;
	size_t mask;
	struct hashset_item *vector;
	dhash_t i, perturb;
	dhash_t hash = Dee_HashPtr(search_item, search_item_length);
#ifndef CONFIG_NO_THREADS
again_lock:
#endif /* !CONFIG_NO_THREADS */
	DeeHashSet_LockRead(me);
	vector  = me->hs_elem;
	mask    = me->hs_mask;
	perturb = i = hash & mask;
	for (;; DeeHashSet_HashNx(i, perturb)) {
		struct hashset_item *item = &vector[i & mask];
		if ((old_item = item->hsi_key) == NULL)
			break; /* Not found */
		if (item->hsi_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(old_item))
			continue; /* Not-a-string. */
		if (!DeeString_EqualsBuf(item->hsi_key, search_item, search_item_length))
			continue; /* Differing strings. */

		/* Found it! */
#ifndef CONFIG_NO_THREADS
		if (!DeeHashSet_LockUpgrade(me)) {
			DeeHashSet_LockEndWrite(me);
			SCHED_YIELD();
			goto again_lock;
		}
#endif /* !CONFIG_NO_THREADS */
		item->hsi_key = dummy;
		Dee_Incref(dummy);
		ASSERT(me->hs_used);
		if (--me->hs_used <= me->hs_size / 2)
			hashset_rehash(me, -1);
		ASSERT((me->hs_elem == empty_hashset_items) == (me->hs_mask == 0));
		ASSERT((me->hs_elem == empty_hashset_items) == (me->hs_used == 0));
		ASSERT((me->hs_elem == empty_hashset_items) == (me->hs_size == 0));
		DeeHashSet_LockEndRead(me);
		Dee_Decref(old_item);
		return 1;
	}
	DeeHashSet_LockEndRead(me);
	return 0;
}





/* @return:  1/true:  The object exists.
 * @return:  0/false: No such object exists.
 * @return: -1:       An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeHashSet_Contains(DeeObject *self, DeeObject *search_item) {
	HashSet *me = (HashSet *)self;
	size_t mask;
	struct hashset_item *vector;
	dhash_t i, perturb;
	int error;
	dhash_t hash = DeeObject_Hash(search_item);
	DeeHashSet_LockRead(me);
restart:
	vector  = me->hs_elem;
	mask    = me->hs_mask;
	perturb = i = hash & mask;
	for (;; DeeHashSet_HashNx(i, perturb)) {
		DREF DeeObject *item_key;
		struct hashset_item *item = &vector[i & mask];
		if (!item->hsi_key)
			break; /* Not found */
		if (item->hsi_hash != hash)
			continue; /* Non-matching hash */
		if (item->hsi_key == dummy)
			continue; /* Dummy key. */
		item_key = item->hsi_key;
		Dee_Incref(item_key);
		DeeHashSet_LockEndRead(me);
		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_CompareEq(search_item, item_key);
		Dee_Decref(item_key);
		if (error > 0)
			return 1; /* Found the item. */
		if unlikely(error < 0)
			goto err; /* Error in compare operator. */
		DeeHashSet_LockRead(me);
		/* Check if the set was modified. */
		if (me->hs_elem != vector ||
		    me->hs_mask != mask ||
		    item->hsi_key != item_key)
			goto restart;
	}
	DeeHashSet_LockEndRead(me);
	return 0;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) bool DCALL
DeeHashSet_ContainsString(DeeObject *__restrict self,
                          char const *__restrict search_item,
                          size_t search_item_length) {
	HashSet *me = (HashSet *)self;
	size_t mask;
	struct hashset_item *vector;
	dhash_t i, perturb;
	dhash_t hash = Dee_HashPtr(search_item, search_item_length);
	DeeHashSet_LockRead(me);
	vector  = me->hs_elem;
	mask    = me->hs_mask;
	perturb = i = hash & mask;
	for (;; DeeHashSet_HashNx(i, perturb)) {
		struct hashset_item *item = &vector[i & mask];
		if (!item->hsi_key)
			break; /* Not found */
		if (item->hsi_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->hsi_key))
			continue; /* Not-a-string. */
		if (DeeString_SIZE(item->hsi_key) != search_item_length)
			continue; /* Not-a-string. */
		if (!DeeString_EqualsBuf(item->hsi_key, search_item, search_item_length))
			continue; /* Differing strings. */
		DeeHashSet_LockEndRead(me);
		return true;
	}
	DeeHashSet_LockEndRead(me);
	return false;
}




PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_size(HashSet *__restrict self) {
	return DeeInt_NewSize(atomic_read(&self->hs_used));
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
hashset_nsi_getsize(HashSet *__restrict self) {
	return atomic_read(&self->hs_used);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
hashset_contains(HashSet *self, DeeObject *search_item) {
	int result = DeeHashSet_Contains((DeeObject *)self, search_item);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}


typedef struct {
	OBJECT_HEAD
	DREF HashSet        *si_set;  /* [1..1][const] The set that is being iterated. */
	struct hashset_item *si_next; /* [?..1][MAYBE(in(si_set->hs_elem))][atomic]
	                               *   The first candidate for the next item.
	                               *   NOTE: Before being dereferenced, this pointer is checked
	                               *         for being located inside the set's element vector.
	                               *         In the event that it is located at its end, `ITER_DONE'
	                               *         is returned, though in the event that it is located
	                               *         outside, an error is thrown (`err_changed_sequence()'). */
} HashSetIterator;
#define READ_ITEM(x) atomic_read(&(x)->si_next)

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashsetiterator_next(HashSetIterator *__restrict self) {
	DREF DeeObject *result;
	struct hashset_item *item, *end;
	HashSet *set = self->si_set;
	DeeHashSet_LockRead(set);
	end = set->hs_elem + (set->hs_mask + 1);
	for (;;) {
		struct hashset_item *old_item;
		item = atomic_read(&self->si_next);
		old_item = item;

		/* Validate that the pointer is still located in-bounds. */
		if (item >= end) {
			if unlikely(item > end)
				goto set_has_changed;
			goto iter_exhausted;
		}
		if unlikely(item < set->hs_elem)
			goto set_has_changed;

		/* Search for the next non-empty item. */
		while (item < end && (!item->hsi_key || item->hsi_key == dummy))
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->si_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->si_next, old_item, item + 1))
			break;
	}
	result = item->hsi_key;
	Dee_Incref(result);
	DeeHashSet_LockEndRead(set);
	return result;
set_has_changed:
	DeeHashSet_LockEndRead(set);
	err_changed_sequence((DeeObject *)set);
	return NULL;
iter_exhausted:
	DeeHashSet_LockEndRead(set);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hashsetiterator_ctor(HashSetIterator *__restrict self) {
	self->si_set = (HashSet *)DeeHashSet_New();
	if unlikely(!self->si_set)
		goto err;
	self->si_next = self->si_set->hs_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashsetiterator_copy(HashSetIterator *__restrict self,
                     HashSetIterator *__restrict other) {
	self->si_set = other->si_set;
	Dee_Incref(self->si_set);
	self->si_next = READ_ITEM(other);
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
hashsetiterator_fini(HashSetIterator *__restrict self) {
	Dee_Decref(self->si_set);
}

PRIVATE NONNULL((1, 2)) void DCALL
hashsetiterator_visit(HashSetIterator *__restrict self,
                      dvisit_t proc, void *arg) {
	Dee_Visit(self->si_set);
}

INTDEF DeeTypeObject HashSetIterator_Type;

PRIVATE WUNUSED NONNULL((1)) int DCALL
hashsetiterator_init(HashSetIterator *__restrict self,
                     size_t argc, DeeObject *const *argv) {
	HashSet *set;
	if (DeeArg_Unpack(argc, argv, "o:_HashSetIterator", &set))
		goto err;
	if (DeeObject_AssertType(set, &DeeHashSet_Type))
		goto err;
	self->si_set = set;
	Dee_Incref(set);
	self->si_next = atomic_read(&set->hs_elem);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hashsetiterator_bool(HashSetIterator *__restrict self) {
	struct hashset_item *item = READ_ITEM(self);
	HashSet *set              = self->si_set;
	/* Check if the iterator is in-bounds.
	 * NOTE: Since this is nothing but a shallow boolean check anyways, there
	 *       is no need to lock the Set since we're not dereferencing anything. */
	return (item >= set->hs_elem &&
	        item < set->hs_elem + (set->hs_mask + 1));
}

#define DEFINE_HASHSETITERATOR_COMPARE(name, op)                \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL       \
	name(HashSetIterator *self, HashSetIterator *other) {       \
		if (DeeObject_AssertType(other, &HashSetIterator_Type)) \
			goto err;                                           \
		return_bool(READ_ITEM(self) op READ_ITEM(other));       \
	err:                                                        \
		return NULL;                                            \
	}
DEFINE_HASHSETITERATOR_COMPARE(hashsetiterator_eq, ==)
DEFINE_HASHSETITERATOR_COMPARE(hashsetiterator_ne, !=)
DEFINE_HASHSETITERATOR_COMPARE(hashsetiterator_lo, <)
DEFINE_HASHSETITERATOR_COMPARE(hashsetiterator_le, <=)
DEFINE_HASHSETITERATOR_COMPARE(hashsetiterator_gr, >)
DEFINE_HASHSETITERATOR_COMPARE(hashsetiterator_ge, >=)
#undef DEFINE_HASHSETITERATOR_COMPARE

PRIVATE struct type_member tpconst hashsetiterator_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(HashSetIterator, si_set), "->?DHashSet"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF HashSet *DCALL
hseti_nii_getseq(HashSetIterator *__restrict self) {
	return_reference_(self->si_set);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
hseti_nii_getindex(HashSetIterator *__restrict self) {
	size_t mask;
	struct hashset_item *vector, *elem;
	DeeHashSet_LockRead(self->si_set);
	vector = self->si_set->hs_elem;
	mask   = self->si_set->hs_mask;
	DeeHashSet_LockEndRead(self->si_set);
	elem = READ_ITEM(self);
	if unlikely(elem < vector || elem > vector + mask)
		return (size_t)-2; /* Indeterminate (detached) */
	return (size_t)(elem - vector);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hseti_nii_setindex(HashSetIterator *__restrict self, size_t new_index) {
	size_t mask;
	struct hashset_item *vector;
	DeeHashSet_LockRead(self->si_set);
	vector = self->si_set->hs_elem;
	mask   = self->si_set->hs_mask;
	DeeHashSet_LockEndRead(self->si_set);
	if (new_index > mask + 1)
		new_index = mask + 1;
	atomic_write(&self->si_next, vector + new_index);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hseti_nii_rewind(HashSetIterator *__restrict self) {
	atomic_write(&self->si_next, atomic_read(&self->si_set->hs_elem));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hseti_nii_revert(HashSetIterator *__restrict self, size_t step) {
	size_t index, mask;
	struct hashset_item *vector, *elem;
again:
	DeeHashSet_LockRead(self->si_set);
	vector = self->si_set->hs_elem;
	mask   = self->si_set->hs_mask;
	DeeHashSet_LockEndRead(self->si_set);
	elem = READ_ITEM(self);
	if unlikely(elem < vector || elem > vector + mask)
		return 0; /* Indeterminate (detached) */
	index = (size_t)(elem - vector);
	if (step < index)
		vector = elem - step;
	if (!atomic_cmpxch_weak_or_write(&self->si_next, elem, vector))
		goto again;
	return elem == vector ? 1 : 2;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hseti_nii_advance(HashSetIterator *__restrict self, size_t step) {
	size_t index, mask;
	struct hashset_item *vector, *elem;
again:
	DeeHashSet_LockRead(self->si_set);
	vector = self->si_set->hs_elem;
	mask   = self->si_set->hs_mask;
	DeeHashSet_LockEndRead(self->si_set);
	elem = READ_ITEM(self);
	if unlikely(elem < vector || elem > vector + mask)
		return 0; /* Indeterminate (detached) */
	index = (size_t)(elem - vector) + step;
	if (index > mask + 1)
		index = mask + 1;
	vector += index;
	if (!atomic_cmpxch_weak_or_write(&self->si_next, elem, vector))
		goto again;
	return index == mask + 1 ? 1 : 2;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hseti_nii_prev(HashSetIterator *__restrict self) {
	size_t mask;
	struct hashset_item *vector, *elem;
again:
	DeeHashSet_LockRead(self->si_set);
	vector = self->si_set->hs_elem;
	mask   = self->si_set->hs_mask;
	DeeHashSet_LockEndRead(self->si_set);
	elem = READ_ITEM(self);
	if unlikely(elem <= vector || elem > vector + mask)
		return 1; /* Indeterminate (detached), or at start */
	if (!atomic_cmpxch_weak_or_write(&self->si_next, elem, vector))
		goto again;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hseti_nii_next(HashSetIterator *__restrict self) {
	size_t mask;
	struct hashset_item *vector, *elem;
again:
	DeeHashSet_LockRead(self->si_set);
	vector = self->si_set->hs_elem;
	mask   = self->si_set->hs_mask;
	DeeHashSet_LockEndRead(self->si_set);
	elem = READ_ITEM(self);
	if unlikely(elem < vector || elem >= vector + mask)
		return 1; /* Indeterminate (detached), or at end */
	vector = elem + 1;
	if (!atomic_cmpxch_weak_or_write(&self->si_next, elem, vector))
		goto again;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hseti_nii_hasprev(HashSetIterator *__restrict self) {
	struct hashset_item *item = READ_ITEM(self);
	HashSet *set              = self->si_set;
	/* Check if the iterator is in-bounds.
	 * NOTE: Since this is nothing but a shallow boolean check anyways, there
	 *       is no need to lock the Set since we're not dereferencing anything. */
	return (item > set->hs_elem &&
	        item <= set->hs_elem + (set->hs_mask + 1));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hseti_nii_peek(HashSetIterator *__restrict self) {
	DREF DeeObject *result;
	struct hashset_item *item, *end;
	HashSet *set = self->si_set;
	DeeHashSet_LockRead(set);
	end  = set->hs_elem + (set->hs_mask + 1);
	item = READ_ITEM(self);

	/* Validate that the pointer is still located in-bounds. */
	if (item >= end) {
		if unlikely(item > end)
			goto set_has_changed;
		goto iter_exhausted;
	}
	if unlikely(item < set->hs_elem)
		goto set_has_changed;

	/* Search for the next non-empty item. */
	while (item < end && (!item->hsi_key || item->hsi_key == dummy))
		++item;
	if (item == end)
		goto iter_exhausted;
	result = item->hsi_key;
	Dee_Incref(result);
	DeeHashSet_LockEndRead(set);
	return result;
set_has_changed:
	DeeHashSet_LockEndRead(set);
	err_changed_sequence((DeeObject *)set);
	return NULL;
iter_exhausted:
	DeeHashSet_LockEndRead(set);
	return ITER_DONE;
}


PRIVATE struct type_nii tpconst hashsetiterator_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&hseti_nii_getseq,
			/* .nii_getindex = */ (dfunptr_t)&hseti_nii_getindex,
			/* .nii_setindex = */ (dfunptr_t)&hseti_nii_setindex,
			/* .nii_rewind   = */ (dfunptr_t)&hseti_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)&hseti_nii_revert,
			/* .nii_advance  = */ (dfunptr_t)&hseti_nii_advance,
			/* .nii_prev     = */ (dfunptr_t)&hseti_nii_prev,
			/* .nii_next     = */ (dfunptr_t)&hseti_nii_next,
			/* .nii_hasprev  = */ (dfunptr_t)&hseti_nii_hasprev,
			/* .nii_peek     = */ (dfunptr_t)&hseti_nii_peek
		}
	}
};

PRIVATE struct type_cmp hashsetiterator_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&hashsetiterator_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&hashsetiterator_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&hashsetiterator_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&hashsetiterator_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&hashsetiterator_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&hashsetiterator_ge,
	/* .tp_nii  = */ &hashsetiterator_nii
};

INTERN DeeTypeObject HashSetIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_HashSetIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&hashsetiterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&hashsetiterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)&hashsetiterator_init,
				TYPE_FIXED_ALLOCATOR(HashSetIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&hashsetiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&hashsetiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&hashsetiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &hashsetiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&hashsetiterator_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ hashsetiterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};



PRIVATE WUNUSED NONNULL((1)) DREF HashSetIterator *DCALL
hashset_iter(HashSet *__restrict self) {
	DREF HashSetIterator *result;
	result = DeeObject_MALLOC(HashSetIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &HashSetIterator_Type);
	result->si_set = self;
	Dee_Incref(self);
	result->si_next = atomic_read(&self->hs_elem);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_repr(HashSet *__restrict self) {
	dssize_t error;
	struct unicode_printer p;
	struct hashset_item *iter, *end;
	struct hashset_item *vector;
	size_t mask;
	bool is_first;
again:
	unicode_printer_init(&p);
	if (UNICODE_PRINTER_PRINT(&p, "HashSet({ ") < 0)
		goto err;
	is_first = true;
	DeeHashSet_LockRead(self);
	vector = self->hs_elem;
	mask   = self->hs_mask;
	end    = (iter = vector) + (mask + 1);
	for (; iter < end; ++iter) {
		DREF DeeObject *key;
		key = iter->hsi_key;
		if (key == NULL || key == dummy)
			continue;
		Dee_Incref(key);
		DeeHashSet_LockEndRead(self);
		/* Print this item. */
		error = unicode_printer_printf(&p, "%s%r", is_first ? "" : ", ", key);
		Dee_Decref(key);
		if unlikely(error < 0)
			goto err;
		is_first = false;
		DeeHashSet_LockRead(self);
		if unlikely(self->hs_elem != vector ||
		            self->hs_mask != mask)
			goto restart;
	}
	DeeHashSet_LockEndRead(self);
	if ((is_first ? UNICODE_PRINTER_PRINT(&p, "})")
	              : UNICODE_PRINTER_PRINT(&p, " })")) < 0)
		goto err;
	return unicode_printer_pack(&p);
restart:
	DeeHashSet_LockEndRead(self);
	unicode_printer_fini(&p);
	goto again;
err:
	unicode_printer_fini(&p);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
hashset_printrepr(HashSet *__restrict self,
                  dformatprinter printer, void *arg) {
	dssize_t temp, result;
	struct hashset_item *iter, *end;
	struct hashset_item *vector;
	size_t mask;
	bool is_first;
	result = DeeFormat_PRINT(printer, arg, "HashSet({ ");
	if unlikely(result < 0)
		goto done;
	is_first = true;
	DeeHashSet_LockRead(self);
	vector = self->hs_elem;
	mask   = self->hs_mask;
	end    = (iter = vector) + (mask + 1);
	for (; iter < end; ++iter) {
		DREF DeeObject *key;
		key = iter->hsi_key;
		if (key == NULL || key == dummy)
			continue;
		Dee_Incref(key);
		DeeHashSet_LockEndRead(self);
		/* Print this item. */
		if (!is_first) {
			temp = DeeFormat_PRINT(printer, arg, ", ");
			if unlikely(temp < 0) {
				Dee_Decref(key);
				goto err;
			}
			result += temp;
		}
		temp = DeeObject_PrintRepr(key, printer, arg);
		Dee_Decref(key);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		is_first = false;
		DeeHashSet_LockRead(self);
		if unlikely(self->hs_elem != vector ||
		            self->hs_mask != mask) {
			DeeHashSet_LockEndRead(self);
			temp = DeeFormat_PRINT(printer, arg, ", <HashSet changed while being iterated>");
			if (temp < 0)
				goto err;
			result += temp;
			goto stop_after_changed;
		}
	}
	DeeHashSet_LockEndRead(self);
stop_after_changed:
	temp = is_first ? DeeFormat_PRINT(printer, arg, "})")
	                : DeeFormat_PRINT(printer, arg, " })");
	if (temp < 0)
		goto err;
	result += temp;
done:
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
hashset_hash(HashSet *__restrict self) {
	dhash_t result;
	struct hashset_item *iter, *end;
	struct hashset_item *vector;
	size_t mask;
again:
	result = DEE_HASHOF_EMPTY_SEQUENCE;
	DeeHashSet_LockRead(self);
	vector = self->hs_elem;
	mask   = self->hs_mask;
	end    = (iter = vector) + (mask + 1);
	for (; iter < end; ++iter) {
		DREF DeeObject *key;
		key = iter->hsi_key;
		if (key == NULL || key == dummy)
			continue;
		Dee_Incref(key);
		DeeHashSet_LockEndRead(self);
		result ^= DeeObject_Hash(key);
		Dee_Decref(key);
		DeeHashSet_LockRead(self);
		if unlikely(self->hs_elem != vector ||
		            self->hs_mask != mask) {
			DeeHashSet_LockEndRead(self);
			goto again;
		}
	}
	DeeHashSet_LockEndRead(self);
	return result;
}

PRIVATE struct type_nsi tpconst hashset_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SET,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE | TYPE_SEQX_FRESIZABLE,
	{
		/* .nsi_setlike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&hashset_nsi_getsize,
			/* .nsi_insert     = */ (dfunptr_t)&DeeHashSet_Insert,
			/* .nsi_remove     = */ (dfunptr_t)&DeeHashSet_Remove,
		}
	}
};

PRIVATE struct type_cmp hashset_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&hashset_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &hashset_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &hashset_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &hashset_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &hashset_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &hashset_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &hashset_ge,
};

PRIVATE struct type_seq hashset_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&hashset_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&hashset_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&hashset_contains,
	/* .tp_get       = */ NULL,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &hashset_nsi
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
hashset_bool(HashSet *__restrict self) {
	return atomic_read(&self->hs_used) != 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hashset_init(HashSet *__restrict self,
         size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	int error;
	if unlikely(DeeArg_Unpack(argc, argv, "o:HashSet", &seq))
		goto err;
	/* TODO: Support for initialization from `_RoSet' */
	/* TODO: Optimization for fast-sequence types. */
	if unlikely((seq = DeeObject_IterSelf(seq)) == NULL)
		goto err;
	error = hashset_init_iterator(self, seq);
	Dee_Decref(seq);
	return error;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_pop(HashSet *self, size_t argc, DeeObject *const *argv) {
	size_t i;
	DREF DeeObject *result;
	if (DeeArg_Unpack(argc, argv, ":pop"))
		goto err;
	DeeHashSet_LockWrite(self);
	for (i = 0; i <= self->hs_mask; ++i) {
		struct hashset_item *item = &self->hs_elem[i];
		if ((result = item->hsi_key) == NULL)
			continue; /* Unused slot. */
		if (result == dummy)
			continue; /* Deleted slot. */
		item->hsi_key = dummy;
		Dee_Incref(dummy);
		ASSERT(self->hs_used);
		if (--self->hs_used <= self->hs_size / 2)
			hashset_rehash(self, -1);
		ASSERT((self->hs_elem == empty_hashset_items) == (self->hs_mask == 0));
		ASSERT((self->hs_elem == empty_hashset_items) == (self->hs_used == 0));
		ASSERT((self->hs_elem == empty_hashset_items) == (self->hs_size == 0));
		DeeHashSet_LockEndWrite(self);
		return result;
	}
	DeeHashSet_LockEndWrite(self);
	/* HashSet is already empty. */
	err_empty_sequence((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_doclear(HashSet *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	hashset_clear(self);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_insert(HashSet *self, size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	int result;
	if (DeeArg_Unpack(argc, argv, "o:insert", &item))
		goto err;
	result = DeeHashSet_Insert((DeeObject *)self, item);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_unify(HashSet *self, size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	if (DeeArg_Unpack(argc, argv, "o:unify", &item))
		goto err;
	return DeeHashSet_Unify((DeeObject *)self, item);
err:
	return NULL;
}

#if (__SIZEOF_INT__ >= __SIZEOF_POINTER__ || \
     (defined(__i386__) || defined(__x86_64__)))
#define insert_callback DeeHashSet_Insert
#else /* ... */
PRIVATE dssize_t DCALL
insert_callback(HashSet *__restrict self, DeeObject *item) {
	return DeeHashSet_Insert((DeeObject *)self, item);
}
#endif /* !... */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_update(HashSet *self, size_t argc, DeeObject *const *argv) {
	DeeObject *items;
	dssize_t result;
	if (DeeArg_Unpack(argc, argv, "o:update", &items))
		goto err;
	result = DeeObject_Foreach(items, (Dee_foreach_t)&insert_callback, self);
	if unlikely(result < 0)
		goto err;
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_remove(HashSet *self, size_t argc, DeeObject *const *argv) {
	DeeObject *item;
	int result;
	if (DeeArg_Unpack(argc, argv, "o:remove", &item))
		goto err;
	result = DeeHashSet_Remove((DeeObject *)self, item);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_sizeof(HashSet *self) {
	return DeeInt_NewSize(sizeof(HashSet) +
	                      ((self->hs_mask + 1) *
	                       sizeof(struct hashset_item)));
}



PRIVATE struct type_method tpconst hashset_methods[] = {
	TYPE_METHOD(STR_pop, &hashset_pop,
	            "->\n"
	            "#tValueError{The set is empty}"
	            "Pop a random item from the set and return it"),
	TYPE_METHOD(STR_clear, &hashset_doclear,
	            "()\n"
	            "Clear all items from the set"),
	TYPE_METHOD("popitem", &hashset_pop,
	            "->\n"
	            "#tValueError{The set is empty}"
	            "Pop a random item from the set and return it (alias for ?#pop)"),
	TYPE_METHOD("unify", &hashset_unify,
	            "(ob)->\n"
	            "Insert @ob into the set if it wasn't inserted before, "
	            /**/ "and re-return it, or the pre-existing instance"),
	TYPE_METHOD(STR_insert, &hashset_insert,
	            "(ob)->?Dbool\n"
	            "Returns ?t if the object wasn't apart of the set before"),
	TYPE_METHOD("update", &hashset_update,
	            "(items:?S?O)->?Dint\n"
	            "Insert all items from @items into @this set, and return the number of inserted items"),
	TYPE_METHOD("insertall", &hashset_update,
	            "(items:?S?O)->?Dint\n"
	            "Alias for ?#update"),
	TYPE_METHOD(STR_remove, &hashset_remove,
	            "(ob)->?Dbool\n"
	            "Returns ?t if the object was removed from the set"),
	/* TODO: HashSet.byhash(template:?O)->?DSequence */
	/* Alternative function names. */
	TYPE_METHOD("add", &hashset_insert,
	            "(ob)->?Dbool\n"
	            "Deprecated alias for ?#insert"),
	TYPE_METHOD("discard", &hashset_remove,
	            "(ob)->?Dbool\n"
	            "Deprecated alias for ?#remove"),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	/* Old function names. */
	TYPE_METHOD("insert_all", &hashset_update,
	            "(ob)->?Dbool\n"
	            "Deprecated alias for ?#update"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_METHOD_END
};


#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE DEFINE_FLOAT(float_1_point_0, 1.0);
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deprecated_d100_get_maxloadfactor(DeeObject *__restrict UNUSED(self)) {
	return_reference_((DeeObject *)&float_1_point_0);
}

INTERN WUNUSED NONNULL((1)) int DCALL
deprecated_d100_del_maxloadfactor(DeeObject *__restrict UNUSED(self)) {
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
deprecated_d100_set_maxloadfactor(DeeObject *UNUSED(self),
                                  DeeObject *UNUSED(value)) {
	return 0;
}
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

INTDEF struct type_getset tpconst hashset_getsets[];
INTERN_TPCONST struct type_getset tpconst hashset_getsets[] = {
	TYPE_GETTER(STR_frozen, &DeeRoSet_FromSequence,
	            "->?Ert:RoSet\n"
	            "Returns a read-only (frozen) copy of @this HashSet"),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_GETSET("max_load_factor",
	            &deprecated_d100_get_maxloadfactor,
	            &deprecated_d100_del_maxloadfactor,
	            &deprecated_d100_set_maxloadfactor,
	            "->?Dfloat\n"
	            "Deprecated. Always returns ${1.0}, with del/set being ignored"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_GETTER("__sizeof__", &hashset_sizeof, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst hashset_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &HashSetIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &DeeRoSet_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_gc tpconst hashset_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&hashset_clear
};

PUBLIC DeeTypeObject DeeHashSet_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_HashSet),
	/* .tp_doc      = */ DOC("A mutable set-like container that uses hashing to detect/prevent duplicates\n"
	                         "\n"

	                         "()\n"
	                         "Create an empty HashSet\n"
	                         "\n"

	                         "(items:?S?O)\n"
	                         "Create a new HashSet populated with elements from @items\n"
	                         "\n"

	                         "copy->\n"
	                         "Returns a shallow copy of @this HashSet\n"
	                         "\n"

	                         "deepcopy->\n"
	                         "Returns a deep copy of @this HashSet\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if @this HashSet is non-empty\n"
	                         "\n"

	                         "contains->\n"
	                         "Returns ?t if @item is apart of @this HashSet\n"
	                         "\n"

	                         "#->\n"
	                         "Returns the number of items apart of @this HashSet\n"
	                         "\n"

	                         "iter->\n"
	                         "Returns an iterator for enumerating all items "
	                         /**/ "in @this HashSet, following a random order"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(HashSet),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&hashset_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&hashset_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&hashset_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&hashset_init,
				TYPE_FIXED_ALLOCATOR_GC(HashSet)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&hashset_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&hashset_deepload
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&hashset_repr,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&hashset_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&hashset_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&hashset_visit,
	/* .tp_gc            = */ &hashset_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &hashset_cmp,
	/* .tp_seq           = */ &hashset_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ hashset_methods,
	/* .tp_getsets       = */ hashset_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ hashset_class_members
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_HASHSET_C */
