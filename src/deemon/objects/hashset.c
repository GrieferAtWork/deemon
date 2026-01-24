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
#ifndef GUARD_DEEMON_OBJECTS_HASHSET_C
#define GUARD_DEEMON_OBJECTS_HASHSET_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/dict.h>
#include <deemon/error-rt.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/hashset.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/none-operator.h>
#include <deemon/object.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/serial.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/system-features.h>    /* memcpyc(), ... */
#include <deemon/util/atomic.h>        /* atomic_* */
#include <deemon/util/lock.h>          /* Dee_atomic_rwlock_init */

#include <hybrid/sched/yield.h> /* SCHED_YIELD */
#include <hybrid/typecore.h>    /* __SIZEOF_INT__, __SIZEOF_SIZE_T__ */

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "generic-proxy.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */

DECL_BEGIN

typedef DeeHashSetObject HashSet;

PUBLIC_CONST struct Dee_hashset_item const DeeHashSet_EmptyItems[1] = {
	{ NULL, 0 }
};

#define empty_hashset_items ((struct Dee_hashset_item *)DeeHashSet_EmptyItems)
#define dummy               (&DeeDict_Dummy)


PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
hashset_insert_remainder_with_duplicates(HashSet *self, size_t num_items,
                                         /*inherit(on_success)*/ DREF DeeObject **items) {
	size_t key_i = 1;
	size_t extra_duplicates_c = 0;
	size_t extra_duplicates_a = 0;
	DREF DeeObject **extra_duplicates_v = NULL;
next_keyitem:
	while (key_i < num_items) {
		DREF DeeObject *key = items[key_i++];
		Dee_hash_t i, perturb, hash = DeeObject_Hash(key);
		perturb = i = hash & self->hs_mask;
		for (;; DeeHashSet_HashNx(i, perturb)) {
			struct hashset_item *item = &self->hs_elem[i & self->hs_mask];
			if (item->hsi_key) { /* Already in use */
				int temp;
				if likely(item->hsi_hash != hash)
					continue;
				temp = DeeObject_TryCompareEq(item->hsi_key, key);
				if (Dee_COMPARE_ISERR(temp))
					goto err;
				if likely(Dee_COMPARE_ISNE(temp))
					continue;

				/* Another duplicate key. */
				ASSERT(extra_duplicates_c <= extra_duplicates_a);
				if (extra_duplicates_c >= extra_duplicates_a) {
					size_t min_alloc = extra_duplicates_c + 1;
					size_t new_alloc = extra_duplicates_a * 2;
					DREF DeeObject **newvec;
					if (new_alloc < 4)
						new_alloc = 4;
					if (new_alloc < min_alloc)
						new_alloc = min_alloc;
					newvec = (DREF DeeObject **)Dee_TryReallocc(extra_duplicates_v, new_alloc, sizeof(DREF DeeObject *));
					if unlikely(!newvec) {
						new_alloc = min_alloc;
						newvec = (DREF DeeObject **)Dee_Reallocc(extra_duplicates_v, new_alloc, sizeof(DREF DeeObject *));
						if unlikely(!newvec)
							goto err;
					}
					extra_duplicates_v = newvec;
					extra_duplicates_a = new_alloc;
				}
				extra_duplicates_v[extra_duplicates_c] = key;
				++extra_duplicates_c;
				--self->hs_used;
				--self->hs_size;
				goto next_keyitem;
			}
			item->hsi_hash = hash;
			item->hsi_key  = key; /* Inherit reference. */
			break;
		}
	}
	Dee_Decref_unlikely(items[0]);
#ifndef __OPTIMIZE_SIZE__
	if (extra_duplicates_c)
#endif /* !__OPTIMIZE_SIZE__ */
	{
		Dee_Decrefv_unlikely(extra_duplicates_v, extra_duplicates_c);
		Dee_Free(extra_duplicates_v);
	}
	return 0;
err:
	Dee_Free(extra_duplicates_v);
	return -1;
}


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
		goto err;
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
		mask = (min_mask << 1) | 1;
		result->hs_elem = (struct hashset_item *)Dee_TryCallocc(mask + 1, sizeof(struct hashset_item));
		if unlikely(!result->hs_elem) {
			/* Try one level less if that failed. */
			mask = min_mask;
			result->hs_elem = (struct hashset_item *)Dee_Callocc(mask + 1, sizeof(struct hashset_item));
			if unlikely(!result->hs_elem)
				goto err_r;
		}

		/* Without any dummy items, these are identical. */
		result->hs_mask = mask;
		result->hs_used = num_items;
		result->hs_size = num_items;
		while (num_items--) {
			DREF DeeObject *key = *items++;
			Dee_hash_t i, perturb, hash = DeeObject_Hash(key);
			perturb = i = hash & mask;
			for (;; DeeHashSet_HashNx(i, perturb)) {
				struct hashset_item *item = &result->hs_elem[i & mask];
				if (item->hsi_key) { /* Already in use */
					int temp;
					if likely(item->hsi_hash != hash)
						continue;
					temp = DeeObject_TryCompareEq(item->hsi_key, key);
					if (Dee_COMPARE_ISERR(temp))
						goto err_r_elem;
					if likely(Dee_COMPARE_ISNE(temp))
						continue;

					/* Duplicate key. */
					--items;
					++num_items;
					if unlikely(hashset_insert_remainder_with_duplicates(result, num_items, items))
						goto err_r_elem;
					goto done_populate_result;
				}
				item->hsi_hash = hash;
				item->hsi_key  = key; /* Inherit reference. */
				break;
			}
		}
done_populate_result:;
	}
	Dee_atomic_rwlock_init(&result->hs_lock);

	/* Initialize and start tracking the new set. */
	weakref_support_init(result);
	DeeObject_Init(result, &DeeHashSet_Type);
	return DeeGC_Track((DeeObject *)result);
err_r_elem:
	Dee_Free(result->hs_elem);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}


PRIVATE NONNULL((1)) void DCALL
hashset_fini(HashSet *__restrict self);

#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define hashset_insert_sequence_foreach_PTR ((Dee_foreach_t)&DeeHashSet_Insert)
#else /* __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
#define hashset_insert_sequence_foreach_PTR &hashset_insert_sequence_foreach
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
hashset_insert_sequence_foreach(void *arg, DeeObject *key) {
	return DeeHashSet_Insert((DeeObject *)arg, key);
}
#endif /* __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL hashset_copy(HashSet *__restrict self, HashSet *__restrict other);

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashset_init_sequence(HashSet *__restrict self,
                      DeeObject *__restrict sequence) {
	DeeTypeObject *tp = Dee_TYPE(sequence);
	if (tp == &DeeHashSet_Type)
		return hashset_copy(self, (HashSet *)sequence);

	/* Optimizations for `_RoSet' */
	if (tp == &DeeRoSet_Type) {
		STATIC_ASSERT(sizeof(struct hashset_item) == sizeof(struct roset_item));
		STATIC_ASSERT(offsetof(struct hashset_item, hsi_key) == offsetof(struct roset_item, rsi_key));
		STATIC_ASSERT(offsetof(struct hashset_item, hsi_hash) == offsetof(struct roset_item, rsi_hash));
		struct hashset_item *iter, *end;
		DeeRoSetObject *src = (DeeRoSetObject *)sequence;
		Dee_atomic_rwlock_init(&self->hs_lock);
		self->hs_used = self->hs_size = src->rs_size;
		if unlikely(!self->hs_size) {
			self->hs_mask = 0;
			self->hs_elem = (struct hashset_item *)empty_hashset_items;
		} else {
			self->hs_mask = src->rs_mask;
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

	/* Fallback: enumerate the sequence pair-wise and insert into "self" */
	self->hs_mask = 0;
	self->hs_size = 0;
	self->hs_used = 0;
	self->hs_elem = empty_hashset_items;
	Dee_atomic_rwlock_init(&self->hs_lock);
	weakref_support_init(self);
	if unlikely(DeeObject_Foreach(sequence, hashset_insert_sequence_foreach_PTR, self) < 0)
		goto err_self;
	return 0;
err_self:
	hashset_fini(self);
err:
	return -1;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeHashSet_NewWithHint(size_t num_items) {
	DREF HashSet *result;

	/* Allocate the set object. */
	result = DeeGCObject_MALLOC(HashSet);
	if unlikely(!result)
		goto err;
	if unlikely(!num_items) {
		/* Special case: allocate an empty set. */
return_empty_set:
		result->hs_mask = 0;
		result->hs_elem = empty_hashset_items;
	} else {
		size_t min_mask = 16 - 1, mask;

		/* Figure out how large the mask of the set is going to be. */
		while ((num_items & min_mask) != num_items)
			min_mask = (min_mask << 1) | 1;

		/* Prefer using a mask of one greater level to improve performance. */
		mask = (min_mask << 1) | 1;
		result->hs_elem = (struct hashset_item *)Dee_TryCallocc(mask + 1, sizeof(struct hashset_item));
		if unlikely(!result->hs_elem) {
			/* Try one level less if that failed. */
			mask = min_mask;
			result->hs_elem = (struct hashset_item *)Dee_TryCallocc(mask + 1, sizeof(struct hashset_item));
			if unlikely(!result->hs_elem)
				goto return_empty_set;
		}

		/* Without any dummy items, these are identical. */
		result->hs_mask = mask;
	}
	result->hs_used = 0;
	result->hs_size = 0;
	Dee_atomic_rwlock_init(&result->hs_lock);

	/* Initialize and start tracking the new set. */
	weakref_support_init(result);
	DeeObject_Init(result, &DeeHashSet_Type);
	return DeeGC_Track((DeeObject *)result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeHashSet_FromSequence(DeeObject *__restrict self) {
	DREF HashSet *result;
	result = DeeGCObject_MALLOC(HashSet);
	if unlikely(!result)
		goto err;
	if unlikely(hashset_init_sequence(result, self))
		goto err_r;
	DeeObject_Init(result, &DeeHashSet_Type);
	return DeeGC_Track((DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeHashSet_FromSequenceInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self) {
	DREF DeeObject *result;
	if (DeeHashSet_CheckExact(self)) {
		if (!DeeObject_IsShared(self))
			return self; /* Can re-use existing HashSet object. */
	}
	result = DeeHashSet_FromSequence(self);
	if likely(result)
		Dee_Decref(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeHashSet_FromRoSet(DeeObject *__restrict self) {
	struct hashset_item *iter, *end;
	DeeRoSetObject *src = (DeeRoSetObject *)self;
	DREF HashSet *result;
	ASSERT_OBJECT_TYPE_EXACT(src, &DeeRoSet_Type);
	result = DeeGCObject_MALLOC(HashSet);
	if unlikely(!result)
		goto err;
	result->hs_mask = src->rs_mask;
	result->hs_used = result->hs_size = src->rs_size;
	if unlikely(!result->hs_size) {
		result->hs_elem = (struct hashset_item *)empty_hashset_items;
	} else {
		result->hs_elem = (struct hashset_item *)Dee_Mallocc(src->rs_mask + 1,
		                                                     sizeof(struct hashset_item));
		if unlikely(!result->hs_elem)
			goto err_r;
		iter = (struct hashset_item *)memcpyc(result->hs_elem, src->rs_elem,
		                                      result->hs_mask + 1,
		                                      sizeof(struct hashset_item));
		end  = iter + (result->hs_mask + 1);
		for (; iter < end; ++iter)
			Dee_XIncref(iter->hsi_key);
	}
	Dee_atomic_rwlock_init(&result->hs_lock);
	weakref_support_init(result);
	DeeObject_Init(result, &DeeHashSet_Type);
	return DeeGC_Track((DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
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
			if (Dee_CollectMemoryc(other->hs_mask + 1, sizeof(struct hashset_item)))
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
		Dee_hash_t j, perturb, hash;
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
					int error = DeeObject_TryCompareEq(item->hsi_key, items[i]);
					if (Dee_COMPARE_ISERR(error))
						goto err_items_v_new_map;
					if (Dee_COMPARE_ISEQ(error))
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
hashset_visit(HashSet *__restrict self, Dee_visit_t proc, void *arg) {
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
			Dee_hash_t i, perturb;

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
	struct hashset_item *first_dummy;
	Dee_hash_t i, perturb, hash = DeeObject_Hash(search_item);
again_lock:
	DeeHashSet_LockRead(me);
again:
	first_dummy = NULL;
	vector      = me->hs_elem;
	mask        = me->hs_mask;
	perturb = i = hash & mask;
	for (;; DeeHashSet_HashNx(i, perturb)) {
		int error;
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
		error = DeeObject_TryCompareEq(search_item, item_key);
		Dee_Decref(item_key);
		if (Dee_COMPARE_ISERR(error))
			goto err; /* Error in compare operator. */
		if (Dee_COMPARE_ISEQ(error)) {
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
	struct hashset_item *first_dummy;
	Dee_hash_t i, perturb, hash = DeeObject_Hash(search_item);
again_lock:
	DeeHashSet_LockRead(me);
again:
	first_dummy = NULL;
	vector      = me->hs_elem;
	mask        = me->hs_mask;
	perturb = i = hash & mask;
	for (;; DeeHashSet_HashNx(i, perturb)) {
		int error;
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
		error = DeeObject_TryCompareEq(search_item, item_key);
		Dee_Decref(item_key);
		if (Dee_COMPARE_ISERR(error))
			goto err; /* Error in compare operator. */
		DeeHashSet_LockRead(me);

		/* Check if the set was modified. */
		if (me->hs_elem != vector ||
		    me->hs_mask != mask ||
		    item->hsi_key != item_key)
			goto again;
		if (Dee_COMPARE_ISEQ(error)) {
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
	Dee_hash_t i, perturb;
	Dee_hash_t hash = DeeObject_Hash(search_item);
	DeeHashSet_LockRead(me);
restart:
	vector  = me->hs_elem;
	mask    = me->hs_mask;
	perturb = i = hash & mask;
	for (;; DeeHashSet_HashNx(i, perturb)) {
		int error;
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
		error = DeeObject_TryCompareEq(search_item, item_key);
		Dee_Decref(item_key);
		if (Dee_COMPARE_ISERR(error))
			goto err; /* Error in compare operator. */
		if (Dee_COMPARE_ISEQ(error)) {
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



/* @return:  1/true:  The object exists.
 * @return:  0/false: No such object exists.
 * @return: -1:       An error occurred. */
PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
DeeHashSet_Contains(DeeObject *self, DeeObject *search_item) {
	HashSet *me = (HashSet *)self;
	size_t mask;
	struct hashset_item *vector;
	Dee_hash_t i, perturb;
	Dee_hash_t hash = DeeObject_Hash(search_item);
	DeeHashSet_LockRead(me);
restart:
	vector  = me->hs_elem;
	mask    = me->hs_mask;
	perturb = i = hash & mask;
	for (;; DeeHashSet_HashNx(i, perturb)) {
		int error;
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
		error = DeeObject_TryCompareEq(search_item, item_key);
		Dee_Decref(item_key);
		if (Dee_COMPARE_ISEQ_NO_ERR(error))
			return 1; /* Found the item. */
		if (Dee_COMPARE_ISERR(error))
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


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
hashset_size(HashSet *__restrict self) {
	return atomic_read(&self->hs_used);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
hashset_contains(HashSet *self, DeeObject *search_item) {
	int result = DeeHashSet_Contains(Dee_AsObject(self), search_item);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
hashset_foreach(HashSet *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	DeeHashSet_LockRead(self);
	for (i = 0; i <= self->hs_mask; ++i) {
		DREF DeeObject *key;
		key = self->hs_elem[i].hsi_key;
		if (!key || key == dummy)
			continue;
		Dee_Incref(key);
		DeeHashSet_LockEndRead(self);
		temp = (*proc)(arg, key);
		Dee_Decref_unlikely(key);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		DeeHashSet_LockRead(self);
	}
	DeeHashSet_LockEndRead(self);
	return result;
err:
	return temp;
}


typedef struct {
	PROXY_OBJECT_HEAD_EX(HashSet, hsi_set)  /* [1..1][const] The set that is being iterated. */
	struct hashset_item          *hsi_next; /* [?..1][MAYBE(in(hsi_set->hs_elem))][atomic]
	                                         * The first candidate for the next item.
	                                         * NOTE: Before being dereferenced, this pointer is checked
	                                         *       for being located inside the set's element vector.
	                                         *       In the event that it is located at its end, `ITER_DONE'
	                                         *       is returned, though in the event that it is located
	                                         *       outside, an error is thrown (`err_changed_sequence()'). */
} HashSetIterator;
#define READ_ITEM(x) atomic_read(&(x)->hsi_next)

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashsetiterator_next(HashSetIterator *__restrict self) {
	DREF DeeObject *result;
	struct hashset_item *item, *end;
	HashSet *set = self->hsi_set;
	DeeHashSet_LockRead(set);
	end = set->hs_elem + (set->hs_mask + 1);
	for (;;) {
		struct hashset_item *old_item;
		item = atomic_read(&self->hsi_next);
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
			if (!atomic_cmpxch_weak_or_write(&self->hsi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->hsi_next, old_item, item + 1))
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
	self->hsi_set = (HashSet *)DeeHashSet_New();
	if unlikely(!self->hsi_set)
		goto err;
	self->hsi_next = self->hsi_set->hs_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashsetiterator_copy(HashSetIterator *__restrict self,
                     HashSetIterator *__restrict other) {
	self->hsi_set = other->hsi_set;
	Dee_Incref(self->hsi_set);
	self->hsi_next = READ_ITEM(other);
	return 0;
}

STATIC_ASSERT(offsetof(HashSetIterator, hsi_set) == offsetof(ProxyObject, po_obj));
#define hashsetiterator_fini  generic_proxy__fini
#define hashsetiterator_visit generic_proxy__visit

INTDEF DeeTypeObject HashSetIterator_Type;

PRIVATE WUNUSED NONNULL((1)) int DCALL
hashsetiterator_init(HashSetIterator *__restrict self,
                     size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("_HashSetIterator", params: "
	DeeHashSetObject *set;
", docStringPrefix: "hashsetiterator");]]]*/
#define hashsetiterator__HashSetIterator_params "set:?DHashSet"
	struct {
		DeeHashSetObject *set;
	} args;
	DeeArg_Unpack1(err, argc, argv, "_HashSetIterator", &args.set);
/*[[[end]]]*/
	if (DeeObject_AssertType(args.set, &DeeHashSet_Type))
		goto err;
	self->hsi_set = args.set;
	Dee_Incref(args.set);
	self->hsi_next = atomic_read(&args.set->hs_elem);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(HashSetIterator, hsi_set) == offsetof(ProxyObjectWithPointer, po_obj));
STATIC_ASSERT(offsetof(HashSetIterator, hsi_next) == offsetof(ProxyObjectWithPointer, po_ptr));
#define hashsetiterator_serialize generic_proxy_with_pointer__serialize_atomic

PRIVATE WUNUSED NONNULL((1)) int DCALL
hashsetiterator_bool(HashSetIterator *__restrict self) {
	struct hashset_item *item = READ_ITEM(self);
	HashSet *set = self->hsi_set;
	/* Check if the iterator is in-bounds.
	 * NOTE: Since this is nothing but a shallow boolean check anyways, there
	 *       is no need to lock the Set since we're not dereferencing anything. */
	return (item >= set->hs_elem &&
	        item < set->hs_elem + (set->hs_mask + 1));
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
hashsetiterator_hash(HashSetIterator *self) {
	return Dee_HashPointer(READ_ITEM(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashsetiterator_compare(HashSetIterator *self, HashSetIterator *other) {
	if (DeeObject_AssertType(other, &HashSetIterator_Type))
		goto err;
	Dee_return_compareT(struct hashset_item *, READ_ITEM(self),
	                    /*                  */ READ_ITEM(other));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_member tpconst hashsetiterator_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(HashSetIterator, hsi_set), "->?DHashSet"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF HashSet *DCALL
hseti_nii_getseq(HashSetIterator *__restrict self) {
	return_reference_(self->hsi_set);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
hseti_nii_getindex(HashSetIterator *__restrict self) {
	size_t mask;
	struct hashset_item *vector, *elem;
	DeeHashSet_LockRead(self->hsi_set);
	vector = self->hsi_set->hs_elem;
	mask   = self->hsi_set->hs_mask;
	DeeHashSet_LockEndRead(self->hsi_set);
	elem = READ_ITEM(self);
	if unlikely(elem < vector || elem > vector + mask)
		return (size_t)-2; /* Indeterminate (detached) */
	return (size_t)(elem - vector);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hseti_nii_setindex(HashSetIterator *__restrict self, size_t new_index) {
	size_t mask;
	struct hashset_item *vector;
	DeeHashSet_LockRead(self->hsi_set);
	vector = self->hsi_set->hs_elem;
	mask   = self->hsi_set->hs_mask;
	DeeHashSet_LockEndRead(self->hsi_set);
	if (new_index > mask + 1)
		new_index = mask + 1;
	atomic_write(&self->hsi_next, vector + new_index);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hseti_nii_rewind(HashSetIterator *__restrict self) {
	atomic_write(&self->hsi_next, atomic_read(&self->hsi_set->hs_elem));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hseti_nii_revert(HashSetIterator *__restrict self, size_t step) {
	size_t index, mask;
	struct hashset_item *vector, *elem;
again:
	DeeHashSet_LockRead(self->hsi_set);
	vector = self->hsi_set->hs_elem;
	mask   = self->hsi_set->hs_mask;
	DeeHashSet_LockEndRead(self->hsi_set);
	elem = READ_ITEM(self);
	if unlikely(elem < vector || elem > vector + mask)
		return 0; /* Indeterminate (detached) */
	index = (size_t)(elem - vector);
	if (step < index)
		vector = elem - step;
	if (!atomic_cmpxch_weak_or_write(&self->hsi_next, elem, vector))
		goto again;
	return elem == vector ? 1 : 2;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hseti_nii_advance(HashSetIterator *__restrict self, size_t step) {
	size_t index, mask;
	struct hashset_item *vector, *elem;
again:
	DeeHashSet_LockRead(self->hsi_set);
	vector = self->hsi_set->hs_elem;
	mask   = self->hsi_set->hs_mask;
	DeeHashSet_LockEndRead(self->hsi_set);
	elem = READ_ITEM(self);
	if unlikely(elem < vector || elem > vector + mask)
		return 0; /* Indeterminate (detached) */
	index = (size_t)(elem - vector) + step;
	if (index > mask + 1)
		index = mask + 1;
	vector += index;
	if (!atomic_cmpxch_weak_or_write(&self->hsi_next, elem, vector))
		goto again;
	return index == mask + 1 ? 1 : 2;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hseti_nii_prev(HashSetIterator *__restrict self) {
	size_t mask;
	struct hashset_item *vector, *elem;
again:
	DeeHashSet_LockRead(self->hsi_set);
	vector = self->hsi_set->hs_elem;
	mask   = self->hsi_set->hs_mask;
	DeeHashSet_LockEndRead(self->hsi_set);
	elem = READ_ITEM(self);
	if unlikely(elem <= vector || elem > vector + mask)
		return 1; /* Indeterminate (detached), or at start */
	if (!atomic_cmpxch_weak_or_write(&self->hsi_next, elem, vector))
		goto again;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hseti_nii_next(HashSetIterator *__restrict self) {
	size_t mask;
	struct hashset_item *vector, *elem;
again:
	DeeHashSet_LockRead(self->hsi_set);
	vector = self->hsi_set->hs_elem;
	mask   = self->hsi_set->hs_mask;
	DeeHashSet_LockEndRead(self->hsi_set);
	elem = READ_ITEM(self);
	if unlikely(elem < vector || elem >= vector + mask)
		return 1; /* Indeterminate (detached), or at end */
	vector = elem + 1;
	if (!atomic_cmpxch_weak_or_write(&self->hsi_next, elem, vector))
		goto again;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hseti_nii_hasprev(HashSetIterator *__restrict self) {
	struct hashset_item *item = READ_ITEM(self);
	HashSet *set              = self->hsi_set;
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
	HashSet *set = self->hsi_set;
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
			/* .nii_getseq   = */ (Dee_funptr_t)&hseti_nii_getseq,
			/* .nii_getindex = */ (Dee_funptr_t)&hseti_nii_getindex,
			/* .nii_setindex = */ (Dee_funptr_t)&hseti_nii_setindex,
			/* .nii_rewind   = */ (Dee_funptr_t)&hseti_nii_rewind,
			/* .nii_revert   = */ (Dee_funptr_t)&hseti_nii_revert,
			/* .nii_advance  = */ (Dee_funptr_t)&hseti_nii_advance,
			/* .nii_prev     = */ (Dee_funptr_t)&hseti_nii_prev,
			/* .nii_next     = */ (Dee_funptr_t)&hseti_nii_next,
			/* .nii_hasprev  = */ (Dee_funptr_t)&hseti_nii_hasprev,
			/* .nii_peek     = */ (Dee_funptr_t)&hseti_nii_peek
		}
	}
};

PRIVATE struct type_cmp hashsetiterator_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&hashsetiterator_hash,
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&hashsetiterator_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
	/* .tp_nii           = */ &hashsetiterator_nii,
};

INTERN DeeTypeObject HashSetIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_HashSetIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(set:?DHashSet)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ HashSetIterator,
			/* tp_ctor:        */ &hashsetiterator_ctor,
			/* tp_copy_ctor:   */ &hashsetiterator_copy,
			/* tp_deep_ctor:   */ NULL,
			/* tp_any_ctor:    */ &hashsetiterator_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &hashsetiterator_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&hashsetiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&hashsetiterator_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&hashsetiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &hashsetiterator_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&hashsetiterator_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ hashsetiterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};



PRIVATE WUNUSED NONNULL((1)) DREF HashSetIterator *DCALL
hashset_iter(HashSet *__restrict self) {
	DREF HashSetIterator *result;
	result = DeeObject_MALLOC(HashSetIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &HashSetIterator_Type);
	result->hsi_set = self;
	Dee_Incref(self);
	result->hsi_next = atomic_read(&self->hs_elem);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_repr(HashSet *__restrict self) {
	Dee_ssize_t error;
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
hashset_printrepr(HashSet *__restrict self,
                  Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
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

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
hashset_asvector_nothrow(HashSet *self, size_t dst_length, /*out*/ DREF DeeObject **dst) {
	size_t result;
	DeeHashSet_LockRead(self);
	result = self->hs_used;
	if likely(dst_length >= result) {
		struct hashset_item *iter, *end;
		end = (iter = self->hs_elem) + (self->hs_mask + 1);
		for (; iter < end; ++iter) {
			DeeObject *key = iter->hsi_key;
			if (key == NULL || key == dummy)
				continue;
			Dee_Incref(key);
			*dst++ = key;
		}
	}
	DeeHashSet_LockEndRead(self);
	return result;
}

PRIVATE struct type_seq hashset_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&hashset_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&hashset_contains,
	/* .tp_getitem                    = */ DEFIMPL_UNSUPPORTED(&default__getitem__unsupported),
	/* .tp_delitem                    = */ DEFIMPL_UNSUPPORTED(&default__delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL_UNSUPPORTED(&default__setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&hashset_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL_UNSUPPORTED(&default__bounditem__unsupported),
	/* .tp_hasitem                    = */ DEFIMPL_UNSUPPORTED(&default__hasitem__unsupported),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&hashset_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&hashset_size,
	/* .tp_getitem_index              = */ DEFIMPL_UNSUPPORTED(&default__getitem_index__unsupported),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL_UNSUPPORTED(&default__delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL_UNSUPPORTED(&default__setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL_UNSUPPORTED(&default__bounditem_index__unsupported),
	/* .tp_hasitem_index              = */ DEFIMPL_UNSUPPORTED(&default__hasitem_index__unsupported),
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL_UNSUPPORTED(&default__trygetitem__unsupported),
	/* .tp_trygetitem_index           = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_index__unsupported),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_string_hash__unsupported),
	/* .tp_getitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__getitem_string_hash__unsupported),
	/* .tp_delitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ DEFIMPL_UNSUPPORTED(&default__bounditem_string_hash__unsupported),
	/* .tp_hasitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__hasitem_string_hash__unsupported),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_string_len_hash__unsupported),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__getitem_string_len_hash__unsupported),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL_UNSUPPORTED(&default__bounditem_string_len_hash__unsupported),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__hasitem_string_len_hash__unsupported),
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&hashset_asvector_nothrow,
	/* .tp_asvector_nothrow           = */ (size_t (DCALL *)(DeeObject *, size_t, DREF DeeObject **))&hashset_asvector_nothrow,
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
hashset_bool(HashSet *__restrict self) {
	return DeeHashSet_SIZE_ATOMIC(self) != 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hashset_init(HashSet *__restrict self,
             size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("HashSet", params: "
	DeeObject *seq: ?S?O;
", docStringPrefix: "hashset");]]]*/
#define hashset_HashSet_params "seq:?S?O"
	struct {
		DeeObject *seq;
	} args;
	DeeArg_Unpack1(err, argc, argv, "HashSet", &args.seq);
/*[[[end]]]*/
	return hashset_init_sequence(self, args.seq);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hashset_serialize(HashSet *__restrict self,
                  DeeSerial *__restrict writer,
                  Dee_seraddr_t addr) {
	HashSet *out;
	size_t i, sizeof_elem, out__hs_mask;
	size_t addrof_out__hs_elem;
	struct hashset_item *out__hs_elem;
	struct hashset_item *in__hs_elem;
again:
	out = DeeSerial_Addr2Mem(writer, addr, HashSet);
	weakref_support_init(out);
	Dee_atomic_rwlock_init(&out->hs_lock);
	DeeHashSet_LockRead(self);
	out->hs_mask = self->hs_mask;
	out->hs_size = self->hs_size;
	out->hs_used = self->hs_used;
	if (self->hs_elem == DeeHashSet_EmptyItems) {
		DeeHashSet_LockEndRead(self);
		return DeeSerial_PutStaticDeemon(writer, addr + offsetof(HashSet, hs_elem),
		                                     DeeHashSet_EmptyItems);
	}
	sizeof_elem = (out->hs_mask + 1) * sizeof(struct hashset_item);
	addrof_out__hs_elem = DeeSerial_TryMalloc(writer, sizeof_elem, NULL);
	if (!Dee_SERADDR_ISOK(addrof_out__hs_elem)) {
		DeeHashSet_LockEndRead(self);
		addrof_out__hs_elem = DeeSerial_Malloc(writer, sizeof_elem, NULL);
		if (!Dee_SERADDR_ISOK(addrof_out__hs_elem))
			goto err;
		DeeHashSet_LockRead(self);
		out = DeeSerial_Addr2Mem(writer, addr, HashSet);
		if unlikely(out->hs_mask != self->hs_mask) {
free_out_hs_elem__and__again:
			DeeHashSet_LockEndRead(self);
			DeeSerial_Free(writer, addrof_out__hs_elem, NULL);
			goto again;
		}
		if unlikely(out->hs_size != self->hs_size)
			goto free_out_hs_elem__and__again;
		if unlikely(out->hs_used != self->hs_used)
			goto free_out_hs_elem__and__again;
		if unlikely(self->hs_elem == DeeHashSet_EmptyItems)
			goto free_out_hs_elem__and__again;
	}
	out = DeeSerial_Addr2Mem(writer, addr, HashSet);
	out__hs_elem = DeeSerial_Addr2Mem(writer, addrof_out__hs_elem, struct hashset_item);
	in__hs_elem  = self->hs_elem;
	out__hs_mask = out->hs_mask;
	memcpyc(out__hs_elem, in__hs_elem, out__hs_mask + 1, sizeof(struct hashset_item));
	for (i = 0; i <= out__hs_mask; ++i)
		Dee_XIncref(out__hs_elem[i].hsi_key);
	DeeHashSet_LockEndRead(self);
	for (i = 0; i <= out__hs_mask; ++i) {
		if (DeeSerial_XInplacePutObject(writer,
		                                addrof_out__hs_elem +
		                                i * sizeof(struct hashset_item) +
		                               offsetof(struct hashset_item, hsi_key))) {
			out__hs_elem = DeeSerial_Addr2Mem(writer, addrof_out__hs_elem, struct hashset_item);
			while (++i <= out__hs_mask)
				Dee_XDecref(out__hs_elem[i].hsi_key);
			goto err;
		}
	}
	return DeeSerial_PutAddr(writer,
	                         addr + offsetof(HashSet, hs_elem),
	                         addrof_out__hs_elem);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_mh_pop(HashSet *self) {
	size_t i;
	DREF DeeObject *result;
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
	DeeRT_ErrEmptySequence(self);
/*err:*/
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
hashset_mh_pop_with_default(HashSet *self, DeeObject *def) {
	size_t i;
	DREF DeeObject *result;
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
	return_reference_(def);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
hashset_mh_clear(HashSet *self) {
	hashset_clear(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_sizeof(HashSet *self) {
	return DeeInt_NewSize(sizeof(HashSet) +
	                      ((self->hs_mask + 1) *
	                       sizeof(struct hashset_item)));
}



PRIVATE struct type_method tpconst hashset_methods[] = {
	/* TODO: HashSet.byhash(template:?O)->?DSequence */
	TYPE_METHOD_HINTREF(Set_insert),
	TYPE_METHOD_HINTREF(Set_remove),
	TYPE_METHOD_HINTREF(Set_unify),
	TYPE_METHOD_HINTREF(Set_pop),
	TYPE_METHOD_HINTREF(Sequence_clear),

	/* Alternative function names. */
	TYPE_METHOD(STR_popitem, &DeeMA_Set_pop,
	            "(def?)->\n"
	            "Deprecated alias for ?#pop"),
	TYPE_METHOD("add", &DeeMA_Set_insert,
	            "(ob)->?Dbool\n"
	            "Deprecated alias for ?#insert"),
	TYPE_METHOD("discard", &DeeMA_Set_remove,
	            "(ob)->?Dbool\n"
	            "Deprecated alias for ?#remove"),
	TYPE_METHOD("update", &DeeMA_Set_insertall,
	            "(items:?S?O)->?Dint\n"
	            "Deprecated alias for ?#insertall"),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_METHOD("insert_all", &DeeMA_Set_insertall,
	            "(ob)->?Dbool\n"
	            "Deprecated alias for ?#insertall"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

	TYPE_METHOD_END
};


PRIVATE struct type_method_hint tpconst hashset_method_hints[] = {
	TYPE_METHOD_HINT_F(set_insert, &DeeHashSet_Insert, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_remove, &DeeHashSet_Remove, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_unify, &DeeHashSet_Unify, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_pop, &hashset_mh_pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_pop_with_default, &hashset_mh_pop_with_default, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_clear, &hashset_mh_clear, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};


#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE DEFINE_FLOAT(float_1_point_0, 1.0);
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deprecated_d100_get_maxloadfactor(DeeObject *__restrict UNUSED(self)) {
	Dee_Incref(&float_1_point_0);
	return Dee_AsObject(&float_1_point_0);
}
#define deprecated_d100_del_maxloadfactor (*(int (DCALL *)(DeeObject *))&_DeeNone_reti0_1)
#define deprecated_d100_set_maxloadfactor (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

INTDEF struct type_getset tpconst hashset_getsets[];
INTERN_TPCONST struct type_getset tpconst hashset_getsets[] = {
	TYPE_GETTER_AB_F(STR_frozen, &DeeRoSet_FromSequence, METHOD_FNOREFESCAPE,
	                 "->?Ert:RoSet\n"
	                 "Returns a read-only (frozen) copy of @this HashSet"),
	TYPE_GETTER_AB(STR_cached, &DeeObject_NewRef, "->?."),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_GETSET_AB_F("max_load_factor",
	                 &deprecated_d100_get_maxloadfactor,
	                 &deprecated_d100_del_maxloadfactor,
	                 &deprecated_d100_set_maxloadfactor,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dfloat\n"
	                 "Deprecated. Always returns ${1.0}, with del/set being ignored"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_GETTER_AB_F("__sizeof__", &hashset_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst hashset_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &HashSetIterator_Type),
	TYPE_MEMBER_CONST(STR_Frozen, &DeeRoSet_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_gc tpconst hashset_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&hashset_clear
};

PRIVATE struct type_operator const hashset_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0004_ASSIGN, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0005_MOVEASSIGN, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FNOREFESCAPE),
};

PUBLIC DeeTypeObject DeeHashSet_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_HashSet),
	/* .tp_doc      = */ DOC("A mutable set-like container that uses hashing to detect/prevent duplicates\n"
	                         "\n"

	                         "()\n"
	                         "Create an empty HashSet\n"
	                         "\n"

	                         "(" hashset_HashSet_params ")\n"
	                         "Create a new HashSet populated with elements from @seq\n"
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ HashSet,
			/* tp_ctor:        */ &hashset_ctor,
			/* tp_copy_ctor:   */ &hashset_copy,
			/* tp_deep_ctor:   */ &hashset_copy,
			/* tp_any_ctor:    */ &hashset_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &hashset_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&hashset_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&hashset_deepload,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&hashset_repr,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&hashset_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&hashset_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&hashset_visit,
	/* .tp_gc            = */ &hashset_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__47C97A4265F9F31F),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__A5C53AFDF1233C5A), /* TODO: &hashset_cmp */
	/* .tp_seq           = */ &hashset_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ hashset_methods,
	/* .tp_getsets       = */ hashset_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ hashset_class_members,
	/* .tp_method_hints  = */ hashset_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ hashset_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(hashset_operators),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_HASHSET_C */
