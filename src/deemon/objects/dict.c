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
#include <deemon/system-features.h> /* bcmpc(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include <hybrid/sched/yield.h>

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "seq/hashfilter.h"

DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

/* A dummy object used by Dict and HashSet to refer to deleted
 * keys that are still apart of the hash chain.
 * DO NOT EXPOSE THIS OBJECT TO USER-CODE! */
PUBLIC DeeObject DeeDict_Dummy = {
	OBJECT_HEAD_INIT(&DeeObject_Type)
};
#define dummy (&DeeDict_Dummy)


typedef DeeDictObject Dict;

#define empty_dict_items ((struct Dee_dict_item *)DeeDict_EmptyItems)
PUBLIC_CONST struct Dee_dict_item const DeeDict_EmptyItems[1] = {
	{ NULL, NULL, 0 }
};

PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
dict_insert_remainder_with_duplicates(Dict *self, size_t num_keyitems,
                                      /*inhert(on_success)*/ DREF DeeObject **key_items) {
	size_t keyitem_i = 1;
	size_t extra_duplicates_c = 0;
	size_t extra_duplicates_a = 0;
	size_t *extra_duplicates_v = NULL;
next_keyitem:
	while (keyitem_i < num_keyitems) {
		DREF DeeObject *key   = key_items[keyitem_i * 2 + 0];
		DREF DeeObject *value = key_items[keyitem_i * 2 + 1];
		Dee_hash_t i, perturb, hash;
		++keyitem_i;
		hash = DeeObject_Hash(key);
		perturb = i = hash & self->d_mask;
		for (;; DeeDict_HashNx(i, perturb)) {
			struct dict_item *item = &self->d_elem[i & self->d_mask];
			if (item->di_key) { /* Already in use */
				int temp;
				if likely(item->di_hash != hash)
					continue;
				temp = DeeObject_TryCompareEq(item->di_key, key);
				if unlikely(temp == Dee_COMPARE_ERR)
					goto err;
				if likely(temp != 0)
					continue;

				/* Another duplicate key. */
				ASSERT(extra_duplicates_c <= extra_duplicates_a);
				if (extra_duplicates_c >= extra_duplicates_a) {
					size_t min_alloc = extra_duplicates_c + 1;
					size_t new_alloc = extra_duplicates_a * 2;
					size_t *newvec;
					if (new_alloc < 4)
						new_alloc = 4;
					if (new_alloc < min_alloc)
						new_alloc = min_alloc;
					newvec = (size_t *)Dee_TryReallocc(extra_duplicates_v, new_alloc, sizeof(size_t));
					if unlikely(!newvec) {
						new_alloc = min_alloc;
						newvec = (size_t *)Dee_Reallocc(extra_duplicates_v, new_alloc, sizeof(size_t));
						if unlikely(!newvec)
							goto err;
					}
					extra_duplicates_v = newvec;
					extra_duplicates_a = new_alloc;
				}
				extra_duplicates_v[extra_duplicates_c] = keyitem_i;
				++extra_duplicates_c;
				--self->d_used;
				--self->d_size;
				goto next_keyitem;
			}
			item->di_hash  = hash;
			item->di_key   = key;   /* Inherit reference. */
			item->di_value = value; /* Inherit reference. */
			break;
		}
	}
	Dee_Decref_unlikely(key_items[0]);
	Dee_Decref_unlikely(key_items[1]);
#ifndef __OPTIMIZE_SIZE__
	if (extra_duplicates_c)
#endif /* !__OPTIMIZE_SIZE__ */
	{
		for (keyitem_i = 0; keyitem_i < extra_duplicates_c; ++keyitem_i) {
			size_t index = extra_duplicates_v[keyitem_i];
			Dee_Decref_unlikely(key_items[index * 2 + 0]);
			Dee_Decref_unlikely(key_items[index * 2 + 1]);
		}
		Dee_Free(extra_duplicates_v);
	}
	return 0;
err:
	Dee_Free(extra_duplicates_v);
	return -1;
}

/* Create a new Dict by inheriting a set of passed key-item pairs.
 * @param: key_items:    A vector containing `num_keyitems*2' elements,
 *                       even ones being keys and odd ones being items.
 * @param: num_keyitems: The number of key-item pairs passed.
 * WARNING: This function does _NOT_ inherit the passed vector, but _ONLY_ its elements! */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeDict_NewKeyItemsInherited(size_t num_keyitems,
                             /*inhert(on_success)*/ DREF DeeObject **key_items) {
	DREF Dict *result;
	/* Allocate the Dict object. */
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto err;
	if (!num_keyitems) {
		/* Special case: allocate an empty Dict. */
		result->d_mask = 0;
		result->d_size = 0;
		result->d_used = 0;
		result->d_elem = empty_dict_items;
	} else {
		size_t min_mask = 16 - 1, mask;

		/* Figure out how large the mask of the Dict is going to be. */
		while ((num_keyitems & min_mask) != num_keyitems)
			min_mask = (min_mask << 1) | 1;

		/* Prefer using a mask of one greater level to improve performance. */
		mask = (min_mask << 1) | 1;
		result->d_elem = (struct dict_item *)Dee_TryCallocc(mask + 1, sizeof(struct dict_item));
		if unlikely(!result->d_elem) {
			/* Try one level less if that failed. */
			mask = min_mask;
			result->d_elem = (struct dict_item *)Dee_Callocc(mask + 1, sizeof(struct dict_item));
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
			Dee_hash_t i, perturb, hash;
			hash = DeeObject_Hash(key);
			perturb = i = hash & mask;
			for (;; DeeDict_HashNx(i, perturb)) {
				struct dict_item *item = &result->d_elem[i & mask];
				if (item->di_key) { /* Already in use */
					int temp;
					if likely(item->di_hash != hash)
						continue;
					temp = DeeObject_TryCompareEq(item->di_key, key);
					if unlikely(temp == Dee_COMPARE_ERR)
						goto err_r_elem;
					if likely(temp != 0)
						continue;

					/* Duplicate key. */
					key_items -= 2;
					++num_keyitems;
					if unlikely(dict_insert_remainder_with_duplicates(result, num_keyitems, key_items))
						goto err_r_elem;
					goto done_populate_result;
				}
				item->di_hash  = hash;
				item->di_key   = key;   /* Inherit reference. */
				item->di_value = value; /* Inherit reference. */
				break;
			}
		}
done_populate_result:;
	}
	Dee_atomic_rwlock_init(&result->d_lock);

	/* Initialize and start tracking the new Dict. */
	weakref_support_init(result);
	DeeObject_Init(result, &DeeDict_Type);
	return DeeGC_Track((DeeObject *)result);
err_r_elem:
	Dee_Free(result->d_elem);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
dict_setitem(Dict *self, DeeObject *key, DeeObject *value);
PRIVATE NONNULL((1)) void DCALL dict_fini(Dict *__restrict self);

#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define dict_insert_sequence_foreach (*(Dee_foreach_pair_t)&dict_setitem)
#else /* __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
dict_insert_sequence_foreach(void *arg, DeeObject *key, DeeObject *value) {
	return dict_setitem((Dict *)arg, key, value);
}
#endif /* __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_copy(Dict *__restrict self, Dict *__restrict other);

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_init_sequence(Dict *__restrict self,
                   DeeObject *__restrict sequence) {
	DeeTypeObject *tp = Dee_TYPE(sequence);
	if (tp == &DeeDict_Type)
		return dict_copy(self, (Dict *)sequence);

	/* Optimizations for `_RoDict' */
	if (tp == &DeeRoDict_Type) {
		STATIC_ASSERT(sizeof(struct dict_item) == sizeof(struct rodict_item));
		STATIC_ASSERT(offsetof(struct dict_item, di_key) == offsetof(struct rodict_item, rdi_key));
		STATIC_ASSERT(offsetof(struct dict_item, di_value) == offsetof(struct rodict_item, rdi_value));
		STATIC_ASSERT(offsetof(struct dict_item, di_hash) == offsetof(struct rodict_item, rdi_hash));
		struct dict_item *iter, *end;
		DeeRoDictObject *src = (DeeRoDictObject *)sequence;
		Dee_atomic_rwlock_init(&self->d_lock);
		self->d_used = self->d_size = src->rd_size;
		if unlikely(!self->d_size) {
			self->d_mask = 0;
			self->d_elem = empty_dict_items;
		} else {
			self->d_mask = src->rd_mask;
			self->d_elem = (struct dict_item *)Dee_Mallocc(src->rd_mask + 1,
			                                               sizeof(struct dict_item));
			if unlikely(!self->d_elem)
				goto err;
			memcpyc(self->d_elem, src->rd_elem,
			        self->d_mask + 1,
			        sizeof(struct dict_item));
			end = (iter = self->d_elem) + (self->d_mask + 1);
			for (; iter < end; ++iter) {
				if (!iter->di_key)
					continue;
				Dee_Incref(iter->di_key);
				Dee_Incref(iter->di_value);
			}
		}
		weakref_support_init(self);
		return 0;
	}
	/* TODO: Optimizations for `_SharedMap' */
	/* TODO: Fast-sequence support */

	/* Fallback: enumerate the sequence pair-wise and insert into "self" */
	self->d_mask = 0;
	self->d_size = 0;
	self->d_used = 0;
	self->d_elem = empty_dict_items;
	Dee_atomic_rwlock_init(&self->d_lock);
	weakref_support_init(self);
	if unlikely(DeeObject_ForeachPair(sequence, &dict_insert_sequence_foreach, self))
		goto err_self;
	return 0;
err_self:
	dict_fini(self);
err:
	return -1;
}

PUBLIC WUNUSED DREF DeeObject *DCALL
DeeDict_NewWithHint(size_t num_keyitems) {
	DREF Dict *result;
	/* Allocate the Dict object. */
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto err;
	if (!num_keyitems) {
		/* Special case: allocate an empty Dict. */
return_empty_dict:
		result->d_mask = 0;
		result->d_elem = empty_dict_items;
	} else {
		size_t min_mask = 16 - 1, mask;

		/* Figure out how large the mask of the Dict is going to be. */
		while ((num_keyitems & min_mask) != num_keyitems)
			min_mask = (min_mask << 1) | 1;

		/* Prefer using a mask of one greater level to improve performance. */
		mask = (min_mask << 1) | 1;
		result->d_elem = (struct dict_item *)Dee_TryCallocc(mask + 1, sizeof(struct dict_item));
		if unlikely(!result->d_elem) {
			/* Try one level less if that failed. */
			mask = min_mask;
			result->d_elem = (struct dict_item *)Dee_TryCallocc(mask + 1, sizeof(struct dict_item));
			if unlikely(!result->d_elem)
				goto return_empty_dict;
		}

		/* Without any dummy items, these are identical. */
		result->d_mask = mask;
	}
	result->d_size = 0;
	result->d_used = 0;
	Dee_atomic_rwlock_init(&result->d_lock);
	weakref_support_init(result);
	DeeObject_Init(result, &DeeDict_Type);
	return DeeGC_Track((DeeObject *)result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDict_FromSequence(DeeObject *__restrict self) {
	DREF Dict *result;
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto err;
	if unlikely(dict_init_sequence(result, self))
		goto err_r;
	DeeObject_Init(result, &DeeDict_Type);
	return DeeGC_Track((DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDict_FromSequenceInherited(/*inherit(on_success)*/ DREF DeeObject *__restrict self) {
	DREF DeeObject *result;
	if (DeeDict_CheckExact(self)) {
		if (!DeeObject_IsShared(self))
			return self; /* Can re-use existing Dict object. */
	}
	result = DeeDict_FromSequence(self);
	if likely(result)
		Dee_Decref(self);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDict_FromRoDict(DeeObject *__restrict self) {
	DREF Dict *result;
	struct dict_item *iter, *end;
	DeeRoDictObject *src = (DeeRoDictObject *)self;
	ASSERT_OBJECT_TYPE_EXACT(src, &DeeRoDict_Type);
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto err;
	Dee_atomic_rwlock_init(&result->d_lock);
	result->d_mask = src->rd_mask;
	result->d_used = result->d_size = src->rd_size;
	if unlikely(!result->d_size) {
		result->d_elem = empty_dict_items;
	} else {
		result->d_elem = (struct dict_item *)Dee_Mallocc(src->rd_mask + 1,
		                                                 sizeof(struct dict_item));
		if unlikely(!result->d_elem)
			goto err_r;
		iter = (struct dict_item *)memcpyc(result->d_elem, src->rd_elem,
		                                   result->d_mask + 1,
		                                   sizeof(struct dict_item));
		end = iter + (result->d_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Incref(iter->di_key);
			Dee_Incref(iter->di_value);
		}
	}
	weakref_support_init(result);
	DeeObject_Init(result, &DeeDict_Type);
	return DeeGC_Track((DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_ctor(Dict *__restrict self) {
	self->d_mask = 0;
	self->d_size = 0;
	self->d_used = 0;
	self->d_elem = empty_dict_items;
	Dee_atomic_rwlock_init(&self->d_lock);
	weakref_support_init(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_copy(Dict *__restrict self,
          Dict *__restrict other) {
	struct dict_item *iter, *end;
	Dee_atomic_rwlock_init(&self->d_lock);
again:
	DeeDict_LockRead(other);
	self->d_mask = other->d_mask;
	self->d_used = other->d_used;
	self->d_size = other->d_size;
	if ((self->d_elem = other->d_elem) != empty_dict_items) {
		self->d_elem = (struct dict_item *)Dee_TryMallocc(other->d_mask + 1,
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
		for (; iter < end; ++iter) {
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
		new_items = (Entry *)Dee_Reallocc(items, item_count, sizeof(Entry));
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
	new_map = (struct dict_item *)Dee_Callocc(new_mask + 1, sizeof(struct dict_item));
	if unlikely(!new_map)
		goto err_items_v;

	/* Insert all the copied items into the new map. */
	for (i = 0; i < item_count; ++i) {
		Dee_hash_t j, perturb, hash;
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
					int error = DeeObject_TryCompareEq(item->di_key, items[i].e_key);
					if unlikely(error == Dee_COMPARE_ERR)
						goto err_items_v_new_map;
					if (error == 0)
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

PRIVATE NONNULL((1)) void DCALL
dict_fini(Dict *__restrict self) {
	weakref_support_fini(self);
	ASSERT((self->d_elem == empty_dict_items) == (self->d_mask == 0));
	ASSERT((self->d_elem == empty_dict_items) == (self->d_size == 0));
	ASSERT(self->d_used <= self->d_size);
	if (self->d_elem != empty_dict_items) {
		struct dict_item *iter, *end;
		end = (iter = self->d_elem) + (self->d_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Decref(iter->di_key);
			Dee_XDecref(iter->di_value);
		}
		Dee_Free(self->d_elem);
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_assign(Dict *self, DeeObject *other) {
	Dict temp;
	size_t old_mask;
	struct Dee_dict_item *old_elem;
	if unlikely(dict_init_sequence(&temp, other))
		goto err;
	DeeDict_LockWrite(self);
	old_mask = self->d_mask;
	old_elem = self->d_elem;
	self->d_mask = temp.d_mask;
	self->d_size = temp.d_size;
	self->d_used = temp.d_used;
	self->d_elem = temp.d_elem;
	DeeDict_LockEndWrite(self);
	if (old_elem != empty_dict_items) {
		struct dict_item *iter, *end;
		end = (iter = old_elem) + (old_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Decref(iter->di_key);
			Dee_XDecref(iter->di_value);
		}
		Dee_Free(old_elem);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_moveassign(Dict *self, Dict *other) {
	size_t old_mask;
	struct Dee_dict_item *old_elem;
	if unlikely(self == other)
		return 0;
	/* Steal everything from "other" and put it into "self" */
	DeeLock_Acquire2(DeeDict_LockWrite(self), DeeDict_LockTryWrite(self), DeeDict_LockEndWrite(self),
	                 DeeDict_LockWrite(other), DeeDict_LockTryWrite(other), DeeDict_LockEndWrite(other));
	old_mask = self->d_mask;
	old_elem = self->d_elem;
	self->d_mask = other->d_mask;
	self->d_size = other->d_size;
	self->d_used = other->d_used;
	self->d_elem = other->d_elem;
	other->d_mask = 0;
	other->d_size = 0;
	other->d_used = 0;
	other->d_elem = empty_dict_items;
	DeeDict_LockEndWrite(self);
	DeeDict_LockEndWrite(other);
	if (old_elem != empty_dict_items) {
		struct dict_item *iter, *end;
		end = (iter = old_elem) + (old_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Decref(iter->di_key);
			Dee_XDecref(iter->di_value);
		}
		Dee_Free(old_elem);
	}
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
dict_clear(Dict *__restrict self) {
	struct dict_item *elem;
	size_t mask;
	DeeDict_LockWrite(self);
	ASSERT((self->d_elem == empty_dict_items) == (self->d_mask == 0));
	ASSERT((self->d_elem == empty_dict_items) == (self->d_size == 0));
	ASSERT(self->d_used <= self->d_size);

	/* Extract the vector and mask. */
	elem         = self->d_elem;
	mask         = self->d_mask;
	self->d_elem = empty_dict_items;
	self->d_mask = 0;
	self->d_used = 0;
	self->d_size = 0;
	DeeDict_LockEndWrite(self);

	/* Destroy the vector. */
	if (elem != empty_dict_items) {
		struct dict_item *iter, *end;
		end = (iter = elem) + (mask + 1);
		for (; iter < end; ++iter) {
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
		for (; iter < end; ++iter) {
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
				for (; iter < end; ++iter) {
					ASSERT(iter->di_key == NULL ||
					       iter->di_key == dummy);
					if (iter->di_key == dummy)
						Dee_DecrefNokill(dummy);
				}
			}
			if (self->d_elem != empty_dict_items)
				Dee_Free(self->d_elem);
			self->d_elem = empty_dict_items;
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
	new_vector = (struct dict_item *)Dee_TryCallocc(new_mask + 1, sizeof(struct dict_item));
	if unlikely(!new_vector)
		return false;
	ASSERT((self->d_elem == empty_dict_items) == (self->d_mask == 0));
	ASSERT((self->d_elem == empty_dict_items) == (self->d_size == 0));
	if (self->d_elem != empty_dict_items) {
		/* Re-insert all existing items into the new Dict vector. */
		end = (iter = self->d_elem) + (self->d_mask + 1);
		for (; iter < end; ++iter) {
			struct dict_item *item;
			Dee_hash_t i, perturb;

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
DeeDict_GetItemStringHash(DeeObject *__restrict self,
                          char const *__restrict key,
                          Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	DREF DeeObject *result;
	Dee_hash_t i, perturb;
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (!strcmp(DeeString_STR(item->di_key), key)) {
			result = item->di_value;
			Dee_Incref(result);
			DeeDict_LockEndRead(me);
			return result;
		}
	}
	DeeDict_LockEndRead(me);
	err_unknown_key_str((DeeObject *)me, key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeDict_GetItemStringLenHash(DeeObject *__restrict self,
                             char const *__restrict key,
                             size_t keylen,
                             Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	DREF DeeObject *result;
	Dee_hash_t i, perturb;
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (DeeString_EqualsBuf(item->di_key, key, keylen)) {
			result = item->di_value;
			Dee_Incref(result);
			DeeDict_LockEndRead(me);
			return result;
		}
	}
	DeeDict_LockEndRead(me);
	err_unknown_key_str_len((DeeObject *)me, key, keylen);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_trygetitem_string_hash(DeeObject *self, char const *key, Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	DREF DeeObject *result;
	Dee_hash_t i, perturb;
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (strcmp(DeeString_STR(item->di_key), key) == 0) {
			result = item->di_value;
			Dee_Incref(result);
			DeeDict_LockEndRead(me);
			return result;
		}
	}
	DeeDict_LockEndRead(me);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_trygetitem_string_len_hash(DeeObject *self, char const *key,
                                size_t keylen, Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	DREF DeeObject *result;
	Dee_hash_t i, perturb;
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (DeeString_EqualsBuf(item->di_key, key, keylen)) {
			result = item->di_value;
			Dee_Incref(result);
			DeeDict_LockEndRead(me);
			return result;
		}
	}
	DeeDict_LockEndRead(me);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_hasitem_string_hash(DeeObject *__restrict self,
                         char const *__restrict key,
                         Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	Dee_hash_t i, perturb;
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (strcmp(DeeString_STR(item->di_key), key) == 0) {
			DeeDict_LockEndRead(me);
			return 1;
		}
	}
	DeeDict_LockEndRead(me);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_hasitem_string_len_hash(DeeObject *__restrict self,
                             char const *__restrict key,
                             size_t keylen, Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	Dee_hash_t i, perturb;
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (DeeString_EqualsBuf(item->di_key, key, keylen)) {
			DeeDict_LockEndRead(me);
			return 1;
		}
	}
	DeeDict_LockEndRead(me);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_bounditem_string_hash(DeeObject *__restrict self,
                           char const *__restrict key,
                           Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	Dee_hash_t i, perturb;
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (strcmp(DeeString_STR(item->di_key), key) == 0) {
			DeeDict_LockEndRead(me);
			return 1;
		}
	}
	DeeDict_LockEndRead(me);
	return -2;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_bounditem_string_len_hash(DeeObject *__restrict self,
                               char const *__restrict key,
                               size_t keylen, Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	Dee_hash_t i, perturb;
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (DeeString_EqualsBuf(item->di_key, key, keylen)) {
			DeeDict_LockEndRead(me);
			return 1;
		}
	}
	DeeDict_LockEndRead(me);
	return -2;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeDict_DelItemStringHash(DeeObject *__restrict self,
                          char const *__restrict key,
                          Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	DREF DeeObject *old_key, *old_value;
	Dee_hash_t i, perturb;
#ifndef CONFIG_NO_THREADS
again_lock:
#endif /* !CONFIG_NO_THREADS */
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
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
		DeeDict_LockEndWrite(me);
		Dee_Decref(old_value);
		Dee_Decref(old_key);
		return 0;
	}
	DeeDict_LockEndRead(me);
	return err_unknown_key_str((DeeObject *)me, key);
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeDict_DelItemStringLenHash(DeeObject *__restrict self,
                             char const *__restrict key,
                             size_t keylen,
                             Dee_hash_t hash) {
	Dict *me = (Dict *)self;
	DREF DeeObject *old_key, *old_value;
	Dee_hash_t i, perturb;
#ifndef CONFIG_NO_THREADS
again_lock:
#endif /* !CONFIG_NO_THREADS */
	DeeDict_LockRead(me);
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if (!DeeString_Check(item->di_key))
			continue; /* NOTE: This also captures `dummy' */
		if (!DeeString_EqualsBuf(item->di_key, key, keylen))
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
		DeeDict_LockEndWrite(me);
		Dee_Decref(old_value);
		Dee_Decref(old_key);
		return 0;
	}
	DeeDict_LockEndRead(me);
	return err_unknown_key_str_len((DeeObject *)me, key, keylen);
}

INTERN WUNUSED NONNULL((1, 2, 4)) int DCALL
DeeDict_SetItemStringHash(DeeObject *self,
                          char const *__restrict key,
                          Dee_hash_t hash,
                          DeeObject *value) {
	Dict *me = (Dict *)self;
	DREF DeeObject *old_value;
	struct dict_item *first_dummy;
	Dee_hash_t i, perturb;
again_lock:
	DeeDict_LockRead(me);
again:
	first_dummy = NULL;
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
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
		DeeDict_LockEndWrite(me);
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
		key_ob = (DREF DeeStringObject *)DeeObject_TryMallocc(offsetof(DeeStringObject, s_str),
		                                                      key_len + 1, sizeof(char));
		if unlikely(!key_ob)
			goto collect_memory;
		DeeObject_Init(key_ob, &DeeString_Type);
		key_ob->s_data = NULL;
		key_ob->s_hash = hash;
		key_ob->s_len  = key_len;
		memcpyc(key_ob->s_str, key, key_len + 1, sizeof(char));
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
		DeeDict_LockEndWrite(me);
		return 0;
	}

	/* Rehash the Dict and try again. */
	if (dict_rehash(me, 1)) {
		DeeDict_LockDowngrade(me);
		goto again;
	}
collect_memory:
	DeeDict_LockEndWrite(me);
	if (Dee_CollectMemory(1))
		goto again_lock;
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int DCALL
DeeDict_SetItemStringLenHash(DeeObject *self,
                             char const *__restrict key,
                             size_t keylen,
                             Dee_hash_t hash,
                             DeeObject *value) {
	Dict *me = (Dict *)self;
	DREF DeeObject *old_value;
	struct dict_item *first_dummy;
	Dee_hash_t i, perturb;
again_lock:
	DeeDict_LockRead(me);
again:
	first_dummy = NULL;
	perturb = i = DeeDict_HashSt(me, hash);
	for (;; DeeDict_HashNx(i, perturb)) {
		struct dict_item *item = DeeDict_HashIt(me, i);
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
		if (!DeeString_EqualsBuf(item->di_key, key, keylen))
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
		DeeDict_LockEndWrite(me);
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
		key_ob = (DREF DeeStringObject *)DeeObject_TryMallocc(offsetof(DeeStringObject, s_str),
		                                                      keylen + 1, sizeof(char));
		if unlikely(!key_ob)
			goto collect_memory;
		DeeObject_Init(key_ob, &DeeString_Type);
		key_ob->s_data = NULL;
		key_ob->s_hash = hash;
		key_ob->s_len  = keylen;
		*(char *)mempcpyc(key_ob->s_str, key, keylen, sizeof(char)) = '\0';
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
		DeeDict_LockEndWrite(me);
		return 0;
	}

	/* Rehash the Dict and try again. */
	if (dict_rehash(me, 1)) {
		DeeDict_LockDowngrade(me);
		goto again;
	}
collect_memory:
	DeeDict_LockEndWrite(me);
	if (Dee_CollectMemory(1))
		goto again_lock;
	return -1;
}

/* This one's basically your hasitem operator. */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_contains(Dict *self, DeeObject *key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	Dee_hash_t hash = DeeObject_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		int error;
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
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		if (error == 0)
			return_true; /* Found the item. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return_false;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_getitem(Dict *self, DeeObject *key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeObject_Hash(key);
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
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return item_value; /* Found the item. */
		Dee_Decref(item_value);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
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
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_getitem_index(Dict *self, size_t key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeInt_Size_Hash(key);
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
		error = DeeInt_Size_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return item_value; /* Found the item. */
		Dee_Decref(item_value);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key ||
		    item->di_value != item_value)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	err_unknown_key_int((DeeObject *)self, key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_trygetitem(Dict *self, DeeObject *key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeObject_Hash(key);
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
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return item_value; /* Found the item. */
		Dee_Decref(item_value);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key ||
		    item->di_value != item_value)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_trygetitem_index(Dict *self, size_t key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeInt_Size_Hash(key);
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
		error = DeeInt_Size_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return item_value; /* Found the item. */
		Dee_Decref(item_value);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key ||
		    item->di_value != item_value)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_bounditem(Dict *self, DeeObject *key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeObject_Hash(key);
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
		item_key   = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return 1; /* Found the item. */
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return -2;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_bounditem_index(Dict *self, size_t key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeInt_Size_Hash(key);
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
		item_key   = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeInt_Size_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return 1; /* Found the item. */
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return -2;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_hasitem(Dict *self, DeeObject *key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeObject_Hash(key);
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
		item_key   = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return 1; /* Found the item. */
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_hasitem_index(Dict *self, size_t key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash = DeeInt_Size_Hash(key);
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
		item_key   = item->di_key;
		Dee_Incref(item_key);
		DeeDict_LockEndRead(self);

		/* Invoke the compare operator outside of any lock. */
		error = DeeInt_Size_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return 1; /* Found the item. */
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		DeeDict_LockRead(self);

		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
dict_nsi_getdefault(DeeObject *self, DeeObject *key, DeeObject *def) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	int error;
	Dee_hash_t hash;
	Dict *me = (Dict *)self;
	hash = DeeObject_Hash(key);
	DeeDict_LockRead(me);
restart:
	vector  = me->d_elem;
	mask    = me->d_mask;
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
		DeeDict_LockEndRead(me);

		/* Invoke the compare operator outside of any lock. */
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if (error == 0)
			return item_value; /* Found the item. */
		Dee_Decref(item_value);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		DeeDict_LockRead(me);

		/* Check if the Dict was modified. */
		if (me->d_elem != vector ||
		    me->d_mask != mask ||
		    item->di_key != item_key ||
		    item->di_value != item_value)
			goto restart;
	}
	DeeDict_LockEndRead(me);
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDict_ByHash(DeeObject *__restrict self, Dee_hash_t hash, bool key_only) {
	DREF DeeObject *result;
	DREF DeeObject *match;
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
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
			match = (DREF DeeObject *)DeeTuple_NewUninitialized(2);
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
				DeeTuple_FreeUninitialized((DREF DeeTupleObject *)match);
				goto again;
			}
		}
	}
	DeeDict_LockEndRead(me);
	if (!match)
		return_empty_tuple;
	result = (DREF DeeObject *)DeeTuple_NewUninitialized(1);
	if unlikely(!result)
		goto err_match;
	DeeTuple_SET(result, 0, match); /* Inherit reference */
	return result;
err_match:
	Dee_Decref(match);
err:
	return NULL;
}


/* Returns "ITER_DONE" if "key" wasn't found. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_popitem(Dict *self, DeeObject *key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	Dee_hash_t hash = DeeObject_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		int error;
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
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		if (error == 0) {
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
	return ITER_DONE;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_delitem(Dict *self, DeeObject *key) {
	DREF DeeObject *pop_item;
	pop_item = dict_popitem(self, key);
	if (pop_item == ITER_DONE)
		return 0;
	if unlikely(!pop_item)
		goto err;
	Dee_Decref(pop_item);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_delitem_index(Dict *self, size_t key) {
	size_t mask;
	struct dict_item *vector;
	Dee_hash_t i, perturb;
	Dee_hash_t hash = DeeInt_Size_Hash(key);
	DeeDict_LockRead(self);
restart:
	vector  = self->d_elem;
	mask    = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		int error;
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
		error = DeeInt_Size_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		if (error == 0) {
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
			Dee_Decref(item_value);
			return 0;
		}
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
dict_setitem(Dict *self, DeeObject *key, DeeObject *value) {
	size_t mask;
	struct dict_item *vector;
	struct dict_item *first_dummy;
	Dee_hash_t i, perturb, hash = DeeObject_Hash(key);
again_lock:
	DeeDict_LockRead(self);
again:
	first_dummy = NULL;
	vector      = self->d_elem;
	mask        = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		int error;
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
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		if (error == 0) {
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
	if ((first_dummy != NULL) &&
	    (self->d_size + 1 < self->d_mask ||
	     first_dummy->di_key != NULL)) {
		bool wasdummy;
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);
		wasdummy = first_dummy->di_key != NULL;
		if (wasdummy)
			Dee_DecrefNokill(first_dummy->di_key);
		/* Fill in the target slot. */
		first_dummy->di_key   = key;
		first_dummy->di_hash  = hash;
		first_dummy->di_value = value;
		Dee_Incref(key);
		Dee_Incref(value);
		++self->d_used;
		if (!wasdummy) {
			++self->d_size;
			/* Try to keep the Dict vector big at least twice as big as the element count. */
			if (self->d_size * 2 > self->d_mask)
				dict_rehash(self, 1);
		}
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

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
dict_setitem_index(Dict *self, size_t key, DeeObject *value) {
	size_t mask;
	DREF DeeObject *keyob = NULL;
	struct dict_item *vector;
	struct dict_item *first_dummy;
	Dee_hash_t i, perturb, hash = DeeInt_Size_Hash(key);
again_lock:
	DeeDict_LockRead(self);
again:
	first_dummy = NULL;
	vector      = self->d_elem;
	mask        = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		int error;
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
		error = DeeInt_Size_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err_keyob; /* Error in compare operator. */
		if (error == 0) {
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
			Dee_Incref(value);
			item->di_value = value;
			DeeDict_LockEndWrite(self);
			Dee_Decref(item_value);
			if unlikely(keyob)
				Dee_Decref(keyob);
			return 0;
		}
		DeeDict_LockRead(self);
		/* Check if the Dict was modified. */
		if (self->d_elem != vector ||
		    self->d_mask != mask ||
		    item->di_key != item_key)
			goto again;
	}
	if (!keyob) {
		DeeDict_LockEndRead(self);
		keyob = DeeInt_NewSize(key);
		if unlikely(!keyob)
			goto err;
		DeeDict_LockWrite(self);
		goto again_lock;
	} else {
#ifndef CONFIG_NO_THREADS
		if (!DeeDict_LockUpgrade(self)) {
			DeeDict_LockEndWrite(self);
			SCHED_YIELD();
			goto again_lock;
		}
#endif /* !CONFIG_NO_THREADS */
	}
	if ((first_dummy != NULL) &&
	    (self->d_size + 1 < self->d_mask ||
	     first_dummy->di_key != NULL)) {
		bool wasdummy;
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);
		wasdummy = first_dummy->di_key != NULL;
		if (wasdummy)
			Dee_DecrefNokill(first_dummy->di_key);
		/* Fill in the target slot. */
		first_dummy->di_key   = keyob; /* Inherit reference */
		first_dummy->di_hash  = hash;
		first_dummy->di_value = value;
		Dee_Incref(value);
		++self->d_used;
		if (!wasdummy) {
			++self->d_size;
			/* Try to keep the Dict vector big at least twice as big as the element count. */
			if (self->d_size * 2 > self->d_mask)
				dict_rehash(self, 1);
		}
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
err_keyob:
	Dee_XDecref(keyob);
err:
	return -1;
}


#define SETITEM_SETOLD 0 /* if_exists: *p_old_value = GET_OLD_VALUE(); SET_OLD_ITEM(); return 1;
                          * else:      return 0; */
#define SETITEM_SETNEW 1 /* if_exists: *p_old_value = GET_OLD_VALUE(); return 1;
                          * else:      ADD_NEW_ITEM(); return 0; */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
dict_setitem_ex(Dict *self,
                DeeObject *key,
                DeeObject *value,
                unsigned int mode,
                DREF DeeObject **p_old_value) {
	size_t mask;
	struct dict_item *vector;
	struct dict_item *first_dummy;
	Dee_hash_t i, perturb, hash = DeeObject_Hash(key);
again_lock:
	DeeDict_LockRead(self);
again:
	first_dummy = NULL;
	vector      = self->d_elem;
	mask        = self->d_mask;
	perturb = i = hash & mask;
	for (;; DeeDict_HashNx(i, perturb)) {
		int error;
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
		error = DeeObject_TryCompareEq(key, item_key);
		Dee_Decref(item_key);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err; /* Error in compare operator. */
		if (error == 0) {
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
				if (p_old_value) {
					*p_old_value = item_value; /* Inherit reference */
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
				if (p_old_value) {
					item_value = item->di_value;
					Dee_Incref(item_value);
					*p_old_value = item_value;
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
#endif /* !CONFIG_NO_THREADS */
	if ((first_dummy != NULL) &&
	    (self->d_size + 1 < self->d_mask ||
	     first_dummy->di_key != NULL)) {
		bool wasdummy;
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);
		wasdummy = first_dummy->di_key != NULL;
		if (wasdummy)
			Dee_DecrefNokill(first_dummy->di_key);
		/* Fill in the target slot. */
		first_dummy->di_key   = key;
		first_dummy->di_hash  = hash;
		first_dummy->di_value = value;
		Dee_Incref(key);
		Dee_Incref(value);
		++self->d_used;
		if (!wasdummy) {
			++self->d_size;
			/* Try to keep the Dict vector big at least twice as big as the element count. */
			if (self->d_size * 2 > self->d_mask)
				dict_rehash(self, 1);
		}
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
dict_size(Dict *__restrict self) {
	return atomic_read(&self->d_used);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_sizeob(Dict *__restrict self) {
	size_t result = atomic_read(&self->d_used);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
dict_nsi_setdefault(Dict *self, DeeObject *key, DeeObject *defl) {
	DeeObject *old_value;
	int error;
	error = dict_setitem_ex(self, key, defl, SETITEM_SETNEW, &old_value);
	if unlikely(error < 0)
		goto err;
	if (error == 1)
		return old_value;
	return_reference_(defl);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
dict_nsi_updateold(Dict *self, DeeObject *key,
                   DeeObject *value, DREF DeeObject **p_oldvalue) {
	return dict_setitem_ex(self, key, value, SETITEM_SETOLD, p_oldvalue);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
dict_nsi_insertnew(Dict *self, DeeObject *key,
                   DeeObject *value, DREF DeeObject **p_oldvalue) {
	int error;
	error = dict_setitem_ex(self, key, value, SETITEM_SETNEW, p_oldvalue);
	if unlikely(error < 0)
		goto err;
	return !error;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dict_foreach(Dict *self, Dee_foreach_pair_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	DeeDict_LockRead(self);
	for (i = 0; i <= self->d_mask; ++i) {
		DREF DeeObject *key, *value;
		key = self->d_elem[i].di_key;
		if (!key || key == dummy)
			continue;
		value = self->d_elem[i].di_value;
		Dee_Incref(key);
		Dee_Incref(value);
		DeeDict_LockEndRead(self);
		temp = (*proc)(arg, key, value);
		Dee_Decref_unlikely(value);
		Dee_Decref_unlikely(key);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		DeeDict_LockRead(self);
	}
	DeeDict_LockEndRead(self);
	return result;
err:
	return temp;
}


PRIVATE struct type_nsi tpconst dict_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE | TYPE_SEQX_FRESIZABLE,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&dict_size,
			/* .nsi_nextkey    = */ (dfunptr_t)&dictiterator_next_key,
			/* .nsi_nextvalue  = */ (dfunptr_t)&dictiterator_next_value,
			/* .nsi_getdefault = */ (dfunptr_t)&dict_nsi_getdefault,
			/* .nsi_setdefault = */ (dfunptr_t)&dict_nsi_setdefault,
			/* .nsi_updateold  = */ (dfunptr_t)&dict_nsi_updateold,
			/* .nsi_insertnew  = */ (dfunptr_t)&dict_nsi_insertnew
		}
	}
};

PRIVATE struct type_seq dict_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dict_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dict_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&dict_setitem,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ &dict_nsi,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&dict_foreach,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&dict_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&dict_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&dict_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&dict_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&dict_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&dict_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&dict_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dict_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&dict_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeDict_GetItemStringHash,
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeDict_DelItemStringHash,
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&DeeDict_SetItemStringHash,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeDict_GetItemStringLenHash,
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeDict_DelItemStringLenHash,
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&DeeDict_SetItemStringLenHash,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_hasitem_string_len_hash,
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_repr(Dict *__restrict self) {
	dssize_t error;
	struct unicode_printer p;
	struct dict_item *iter, *end;
	struct dict_item *vector;
	size_t mask;
	bool is_first;
again:
	unicode_printer_init(&p);
	if (UNICODE_PRINTER_PRINT(&p, "Dict({ ") < 0)
		goto err;
	is_first = true;
	DeeDict_LockRead(self);
	vector = self->d_elem;
	mask   = self->d_mask;
	end    = (iter = vector) + (mask + 1);
	for (; iter < end; ++iter) {
		DREF DeeObject *key, *value;
		key = iter->di_key;
		if (key == NULL || key == dummy)
			continue;
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
		if unlikely(self->d_elem != vector ||
		            self->d_mask != mask)
			goto restart;
	}
	DeeDict_LockEndRead(self);
	if unlikely((is_first ? UNICODE_PRINTER_PRINT(&p, "})")
	                      : UNICODE_PRINTER_PRINT(&p, " })")) < 0)
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

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
dict_printrepr(Dict *__restrict self,
               dformatprinter printer, void *arg) {
	dssize_t temp, result;
	struct dict_item *iter, *end;
	struct dict_item *vector;
	size_t mask;
	bool is_first;
	result = DeeFormat_PRINT(printer, arg, "Dict({ ");
	if unlikely(result < 0)
		goto done;
	is_first = true;
	DeeDict_LockRead(self);
	vector = self->d_elem;
	mask   = self->d_mask;
	end    = (iter = vector) + (mask + 1);
	for (; iter < end; ++iter) {
		DREF DeeObject *key, *value;
		key = iter->di_key;
		if (key == NULL || key == dummy)
			continue;
		value = iter->di_value;
		Dee_Incref(key);
		Dee_Incref(value);
		DeeDict_LockEndRead(self);
		/* Print this key/value pair. */
		if (!is_first) {
			temp = DeeFormat_PRINT(printer, arg, ", ");
			if unlikely(temp < 0) {
				Dee_Decref(value);
				Dee_Decref(key);
				goto err;
			}
			result += temp;
		}
		temp = DeeFormat_Printf(printer, arg, "%r: %r", key, value);
		Dee_Decref(value);
		Dee_Decref(key);
		if unlikely(temp < 0)
			goto err;
		is_first = false;
		DeeDict_LockRead(self);
		if unlikely(self->d_elem != vector ||
		            self->d_mask != mask) {
			DeeDict_LockEndRead(self);
			temp = DeeFormat_PRINT(printer, arg, ", <Dict changed while being iterated>");
			if (temp < 0)
				goto err;
			result += temp;
			goto stop_after_changed;
		}
	}
	DeeDict_LockEndRead(self);
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_bool(Dict *__restrict self) {
	return atomic_read(&self->d_used) != 0;
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
	return dict_nsi_getdefault((DeeObject *)self, key, def);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_pop(Dict *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *key, *def = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:pop", &key, &def))
		goto err;
	result = dict_popitem(self, key);
	if (result == ITER_DONE) {
		result = def;
		if unlikely(!result) {
			err_unknown_key((DeeObject *)self, key);
		} else {
			Dee_Incref(result);
		}
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
dict_popsomething(Dict *self, size_t argc, DeeObject *const *argv) {
	DREF DeeTupleObject *result;
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
		Dee_Decref_unlikely(old_value);
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
	DeeObject *items;
	if (DeeArg_Unpack(argc, argv, "o:update", &items))
		goto err;
	if unlikely(DeeObject_ForeachPair(items, &dict_insert_sequence_foreach, self))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_sizeof(Dict *self) {
	return DeeInt_NewSize(sizeof(Dict) +
	                      ((self->d_mask + 1) *
	                       sizeof(struct dict_item)));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_byhash(Dict *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DeeObject *template_;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__template, "o:byhash", &template_))
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
 * XXX: This would need to be implemented in "DeeMapping_Type"; not here
 */

DOC_REF(map_get_doc);
DOC_REF(map_pop_doc);
DOC_REF(map_clear_doc);
DOC_REF(map_popitem_doc);
DOC_REF(map_setdefault_doc);
DOC_REF(map_setold_doc);
DOC_REF(map_setnew_doc);
DOC_REF(map_setold_ex_doc);
DOC_REF(map_setnew_ex_doc);
DOC_REF(map_update_doc);
DOC_REF(map_byhash_doc);

PRIVATE struct type_method tpconst dict_methods[] = {
	TYPE_METHOD_F(STR_get, &dict_get, METHOD_FNOREFESCAPE, DOC_GET(map_get_doc)),
	TYPE_METHOD_F(STR_pop, &dict_pop, METHOD_FNOREFESCAPE, DOC_GET(map_pop_doc)),
	TYPE_METHOD_F(STR_clear, &dict_doclear, METHOD_FNOREFESCAPE, DOC_GET(map_clear_doc)),
	TYPE_METHOD_F("popitem", &dict_popsomething, METHOD_FNOREFESCAPE, DOC_GET(map_popitem_doc)),
	TYPE_METHOD_F("setdefault", &dict_setdefault, METHOD_FNOREFESCAPE, DOC_GET(map_setdefault_doc)),
	TYPE_METHOD_F("setold", &dict_setold, METHOD_FNOREFESCAPE, DOC_GET(map_setold_doc)),
	TYPE_METHOD_F("setnew", &dict_setnew, METHOD_FNOREFESCAPE, DOC_GET(map_setnew_doc)),
	TYPE_METHOD_F("setold_ex", &dict_setold_ex, METHOD_FNOREFESCAPE, DOC_GET(map_setold_ex_doc)),
	TYPE_METHOD_F("setnew_ex", &dict_setnew_ex, METHOD_FNOREFESCAPE, DOC_GET(map_setnew_ex_doc)),
	TYPE_METHOD_F("update", &dict_update, METHOD_FNOREFESCAPE, DOC_GET(map_update_doc)),
	TYPE_KWMETHOD("byhash", &dict_byhash, DOC_GET(map_byhash_doc)),
	TYPE_METHOD_END
};

#ifndef CONFIG_NO_DEEMON_100_COMPAT
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deprecated_d100_get_maxloadfactor(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL
deprecated_d100_del_maxloadfactor(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL
deprecated_d100_set_maxloadfactor(DeeObject *self, DeeObject *value);
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

INTDEF struct type_getset tpconst dict_getsets[];
INTERN_TPCONST struct type_getset tpconst dict_getsets[] = {
	TYPE_GETTER_F("keys", &dict_keys,
	              METHOD_FCONSTCALL, /* CONSTCALL because this returns a proxy */
	              "->?AKeys?.\n"
	              "#r{A proxy sequence for viewing the keys of @this ?.}"),
	TYPE_GETTER_F("values", &dict_values,
	              METHOD_FCONSTCALL, /* CONSTCALL because this returns a proxy */
	              "->?AValues?.\n"
	              "#r{A proxy sequence for viewing the values of @this ?.}"),
	TYPE_GETTER_F("items", &dict_items,
	              METHOD_FCONSTCALL, /* CONSTCALL because this returns a proxy */
	              "->?AItems?.\n"
	              "#r{A proxy sequence for viewing the key-value pairs of @this ?.}"),
	TYPE_GETTER_F(STR_frozen, &DeeRoDict_FromSequence, METHOD_FNOREFESCAPE,
	              "->?Ert:RoDict\n"
	              "Returns a read-only (frozen) copy of @this ?."),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_GETSET_F("max_load_factor",
	              &deprecated_d100_get_maxloadfactor,
	              &deprecated_d100_del_maxloadfactor,
	              &deprecated_d100_set_maxloadfactor,
	              METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	              "->?Dfloat\n"
	              "Deprecated. Always returns ${1.0}, with del/set being ignored"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_GETTER_F("__sizeof__", &dict_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};



INTDEF DeeTypeObject DictIterator_Type;
PRIVATE struct type_member tpconst dict_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DictIterator_Type),
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

PRIVATE struct type_operator const dict_operators[] = {
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
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0033_DELITEM, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0034_SETITEM, METHOD_FNOREFESCAPE),
};

PUBLIC DeeTypeObject DeeDict_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Dict),
	/* .tp_doc      = */ DOC("The builtin mapping object for translating keys to items\n"
	                         "\n"

	                         "()\n"
	                         "Create a new, empty ?.\n"
	                         "\n"

	                         "(items:?S?T2?O?O)\n"
	                         "Create a new ?., using key-items pairs extracted from @items.\n"
	                         "Iterate @items and unpack each element into 2 others, using them "
	                         /**/ "as key and value to insert into @this ?."),
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
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_moveassign,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&dict_deepload
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_repr,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&dict_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&dict_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dict_visit,
	/* .tp_gc            = */ &dict_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &dict_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ dict_methods,
	/* .tp_getsets       = */ dict_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dict_class_members,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ dict_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(dict_operators)
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_DICT_C */
