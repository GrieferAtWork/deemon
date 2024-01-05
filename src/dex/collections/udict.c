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
#ifndef GUARD_DEX_COLLECTIONS_UDICT_C
#define GUARD_DEX_COLLECTIONS_UDICT_C 1
#define DEE_SOURCE

#include "libcollections.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/dict.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

DECL_BEGIN

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

/* Dummy key object. */
#define dummy (&DeeDict_Dummy)

INTDEF WUNUSED NONNULL((1)) int DCALL uset_bool(USet *__restrict self);
STATIC_ASSERT(offsetof(USet, us_used) == offsetof(UDict, ud_used));
#define udict_bool uset_bool

#define READ_ITEM(x) atomic_read(&(x)->udi_next)

STATIC_ASSERT(offsetof(UDictIterator, udi_dict) == offsetof(USetIterator, usi_set));
STATIC_ASSERT(offsetof(UDictIterator, udi_next) == offsetof(USetIterator, usi_next));
INTDEF struct type_cmp usetiterator_cmp;
INTDEF WUNUSED NONNULL((1, 2)) int DCALL usetiterator_copy(USetIterator *__restrict self, USetIterator *__restrict other);
INTDEF NONNULL((1)) void DCALL usetiterator_fini(USetIterator *__restrict self);
INTDEF NONNULL((1, 2)) void DCALL usetiterator_visit(USetIterator *__restrict self, dvisit_t proc, void *arg);
#define udictiterator_cmp   usetiterator_cmp
#define udictiterator_copy  usetiterator_copy
#define udictiterator_fini  usetiterator_fini
#define udictiterator_visit usetiterator_visit

STATIC_ASSERT(offsetof(UDict, ud_elem) == offsetof(USet, us_elem));
STATIC_ASSERT(offsetof(UDict, ud_mask) == offsetof(USet, us_mask));
INTDEF WUNUSED NONNULL((1)) int DCALL usetiterator_bool(USetIterator *__restrict self);
#define udictiterator_bool usetiterator_bool

PRIVATE WUNUSED NONNULL((1)) int DCALL
udictiterator_ctor(UDictIterator *__restrict self) {
	self->udi_dict = (DREF UDict *)DeeObject_NewDefault(&UDict_Type);
	if unlikely(!self->udi_dict)
		goto err;
	self->udi_next = self->udi_dict->ud_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
udictiterator_init(UDictIterator *__restrict self,
                   size_t argc, DeeObject *const *argv) {
	UDict *dict;
	if (DeeArg_Unpack(argc, argv, "o:_UniqueDictIterator", &dict))
		goto err;
	if (DeeObject_AssertType(dict, &UDict_Type))
		goto err;
	self->udi_dict = dict;
	Dee_Incref(dict);
	self->udi_next = dict->ud_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udictiterator_next(UDictIterator *__restrict self) {
	DREF DeeObject *result, *key, *value;
	struct udict_item *item, *end;
	UDict *dict = self->udi_dict;
	UDict_LockRead(dict);
	end = dict->ud_elem + (dict->ud_mask + 1);
	for (;;) {
		struct udict_item *old_item;
		item     = atomic_read(&self->udi_next);
		old_item = item;

		/* Validate that the pointer is still located in-bounds. */
		if (item >= end) {
			if unlikely(item > end)
				goto dict_has_changed;
			goto iter_exhausted;
		}
		if unlikely(item < dict->ud_elem)
			goto dict_has_changed;
		/* Search for the next non-empty item. */
		while (item < end && (!item->di_key || item->di_key == dummy))
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->udi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->udi_next, old_item, item + 1))
			break;
	}
	key   = item->di_key;
	value = item->di_value;
	Dee_Incref(key);
	Dee_Incref(value);
	UDict_LockEndRead(dict);
	result = (DREF DeeObject *)DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err_key_value;
	DeeTuple_SET(result, 0, key);   /* Inherit reference */
	DeeTuple_SET(result, 1, value); /* Inherit reference */
	return result;
err_key_value:
	Dee_Decref(key);
	Dee_Decref(value);
	goto err;
dict_has_changed:
	UDict_LockEndRead(dict);
	err_changed_sequence((DeeObject *)dict);
err:
	return NULL;
iter_exhausted:
	UDict_LockEndRead(dict);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udictiterator_nextkey(UDictIterator *__restrict self) {
	DREF DeeObject *result;
	struct udict_item *item, *end;
	UDict *dict = self->udi_dict;
	UDict_LockRead(dict);
	end = dict->ud_elem + (dict->ud_mask + 1);
	for (;;) {
		struct udict_item *old_item;
		item     = atomic_read(&self->udi_next);
		old_item = item;

		/* Validate that the pointer is still located in-bounds. */
		if (item >= end) {
			if unlikely(item > end)
				goto dict_has_changed;
			goto iter_exhausted;
		}
		if unlikely(item < dict->ud_elem)
			goto dict_has_changed;
		/* Search for the next non-empty item. */
		while (item < end && (!item->di_key || item->di_key == dummy))
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->udi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->udi_next, old_item, item + 1))
			break;
	}
	result = item->di_key;
	Dee_Incref(result);
	UDict_LockEndRead(dict);
	return result;
dict_has_changed:
	UDict_LockEndRead(dict);
	err_changed_sequence((DeeObject *)dict);
	return NULL;
iter_exhausted:
	UDict_LockEndRead(dict);
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udictiterator_nextvalue(UDictIterator *__restrict self) {
	DREF DeeObject *result;
	struct udict_item *item, *end;
	UDict *dict = self->udi_dict;
	UDict_LockRead(dict);
	end = dict->ud_elem + (dict->ud_mask + 1);
	for (;;) {
		struct udict_item *old_item;
		item     = atomic_read(&self->udi_next);
		old_item = item;

		/* Validate that the pointer is still located in-bounds. */
		if (item >= end) {
			if unlikely(item > end)
				goto dict_has_changed;
			goto iter_exhausted;
		}
		if unlikely(item < dict->ud_elem)
			goto dict_has_changed;

		/* Search for the next non-empty item. */
		while (item < end && (!item->di_key || item->di_key == dummy))
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->udi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->udi_next, old_item, item + 1))
			break;
	}
	result = item->di_value;
	Dee_Incref(result);
	UDict_LockEndRead(dict);
	return result;
dict_has_changed:
	UDict_LockEndRead(dict);
	err_changed_sequence((DeeObject *)dict);
	return NULL;
iter_exhausted:
	UDict_LockEndRead(dict);
	return ITER_DONE;
}





PRIVATE struct type_member tpconst udictiterator_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(UDictIterator, udi_dict), "->?GUniqueDict"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject UDictIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_UniqueDictIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&udictiterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&udictiterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)&udictiterator_init,
				TYPE_FIXED_ALLOCATOR(USetIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&udictiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&udictiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&udictiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &udictiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&udictiterator_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ udictiterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};





PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
udict_setitem(UDict *self, DeeObject *key, DeeObject *value);

PRIVATE NONNULL((1)) void DCALL
udict_fini(UDict *__restrict self) {
	weakref_support_fini(self);
	ASSERT((self->ud_elem == empty_dict_items) == (self->ud_mask == 0));
	ASSERT((self->ud_elem == empty_dict_items) == (self->ud_size == 0));
	ASSERT(self->ud_used <= self->ud_size);
	if (self->ud_elem != empty_dict_items) {
		struct udict_item *iter, *end;
		end = (iter = self->ud_elem) + (self->ud_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Decref(iter->di_key);
			Dee_XDecref(iter->di_value);
		}
		Dee_Free(self->ud_elem);
	}
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
udict_insert_iterator(UDict *self, DeeObject *iterator) {
	DREF DeeObject *elem;
	DREF DeeObject *key_and_value[2];
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		/* Unpack the yielded element again. */
		if unlikely(DeeObject_Unpack(elem, 2, key_and_value))
			goto err_elem;
		Dee_Decref(elem);
		if unlikely(udict_setitem(self, key_and_value[0], key_and_value[1]))
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
udict_init_iterator(UDict *self, DeeObject *iterator) {
	self->ud_mask = 0;
	self->ud_size = 0;
	self->ud_used = 0;
	self->ud_elem = (struct udict_item *)empty_dict_items;
	Dee_atomic_rwlock_init(&self->ud_lock);
	weakref_support_init(self);
	if unlikely(udict_insert_iterator(self, iterator)) {
		udict_fini(self);
		goto err;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
udict_init_sequence(UDict *__restrict self,
                    DeeObject *__restrict sequence) {
	int error;
	DREF DeeObject *iterator;

	/* TODO: Optimizations for `DeeDict_Type' */
	/* TODO: Optimizations for `DeeRoDict_Type' */
	/* TODO: Optimizations for `URoDict_Type' */

	iterator = DeeObject_IterSelf(sequence);
	if unlikely(!iterator)
		goto err;
	error = udict_init_iterator(self, iterator);
	Dee_Decref(iterator);
	return error;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
udict_ctor(UDict *__restrict self) {
	self->ud_mask = 0;
	self->ud_size = 0;
	self->ud_used = 0;
	self->ud_elem = (struct udict_item *)empty_dict_items;
	Dee_atomic_rwlock_init(&self->ud_lock);
	weakref_support_init(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
udict_copy(UDict *__restrict self,
           UDict *__restrict other) {
	struct udict_item *iter, *end;
	Dee_atomic_rwlock_init(&self->ud_lock);
again:
	UDict_LockRead(other);
	self->ud_mask = other->ud_mask;
	self->ud_used = other->ud_used;
	self->ud_size = other->ud_size;
	if ((self->ud_elem = other->ud_elem) != empty_dict_items) {
		self->ud_elem = (struct udict_item *)Dee_TryMallocc(other->ud_mask + 1,
		                                                   sizeof(struct udict_item));
		if unlikely(!self->ud_elem) {
			UDict_LockEndRead(other);
			if (Dee_CollectMemory((other->ud_mask + 1) *
			                      sizeof(struct udict_item)))
				goto again;
			goto err;
		}
		memcpyc(self->ud_elem, other->ud_elem,
		        self->ud_mask + 1,
		        sizeof(struct udict_item));
		end = (iter = self->ud_elem) + (self->ud_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->di_key)
				continue;
			Dee_Incref(iter->di_key);
			Dee_XIncref(iter->di_value);
		}
	}
	UDict_LockEndRead(other);
	weakref_support_init(self);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
udict_deepload(UDict *__restrict self) {
	typedef struct {
		DREF DeeObject *e_key;   /* [0..1][lock(:ud_lock)] Dictionary item key. */
		DREF DeeObject *e_value; /* [1..1|if(di_key == dummy, 0..0)][valid_if(di_key)][lock(:ud_lock)] Dictionary item value. */
	} Entry;
	/* #1 Allocate 2 new element-vector of the same size as `self'
	 *    One of them has a length `ud_mask+1', the other `ud_used'
	 * #2 Copy all key/value pairs from `self' into the ud_used-one (create references)
	 *    NOTE: Skip NULL/dummy entries in the Dict vector.
	 * #3 Go through the vector and create deep copies of all keys and items.
	 *    For every key, hash it and insert it into the 2nd vector from before.
	 * #4 Clear and free the 1st vector.
	 * #5 Assign the 2nd vector to the Dict, extracting the old one at the same time.
	 * #6 Clear and free the old vector. */
	Entry *new_items, *items = NULL;
	size_t i, hash_i, item_count, old_item_count = 0;
	struct udict_item *new_map, *old_map;
	size_t new_mask;
	for (;;) {
		UDict_LockRead(self);
		/* Optimization: if the Dict is empty, then there's nothing to copy! */
		if (self->ud_elem == empty_dict_items) {
			UDict_LockEndRead(self);
			return 0;
		}
		item_count = self->ud_used;
		if (item_count <= old_item_count)
			break;
		UDict_LockEndRead(self);
		new_items = (Entry *)Dee_Reallocc(items, item_count, sizeof(Entry));
		if unlikely(!new_items)
			goto err_items;
		old_item_count = item_count;
		items          = new_items;
	}
	/* Copy all used items. */
	for (i = 0, hash_i = 0; i < item_count; ++hash_i) {
		ASSERT(hash_i <= self->ud_mask);
		if (self->ud_elem[hash_i].di_key == NULL)
			continue;
		if (self->ud_elem[hash_i].di_key == dummy)
			continue;
		items[i].e_key   = self->ud_elem[hash_i].di_key;
		items[i].e_value = self->ud_elem[hash_i].di_value;
		Dee_Incref(items[i].e_key);
		Dee_Incref(items[i].e_value);
		++i;
	}
	UDict_LockEndRead(self);
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
	new_map = (struct udict_item *)Dee_Callocc(new_mask + 1, sizeof(struct udict_item));
	if unlikely(!new_map)
		goto err_items_v;
	/* Insert all the copied items into the new map. */
	for (i = 0; i < item_count; ++i) {
		dhash_t j, perturb, hash;
		hash    = UHASH(items[i].e_key);
		perturb = j = hash & new_mask;
		for (;; DeeDict_HashNx(j, perturb)) {
			struct udict_item *item = &new_map[j & new_mask];
			if (item->di_key) {
				/* Check if deepcopy caused one of the elements to get duplicated. */
				if unlikely(USAME(item->di_key, items[i].e_key)) {
					Dee_Decref(items[i].e_key);
					Dee_Decref(items[i].e_value);
					--item_count;
					memmovedownc(&items[i],
					             &items[i + 1],
					             item_count - i,
					             sizeof(struct udict_item));
					break;
				}
				/* Slot already in use */
				continue;
			}
			item->di_key   = items[i].e_key;   /* Inherit reference. */
			item->di_value = items[i].e_value; /* Inherit reference. */
			break;
		}
	}
	UDict_LockWrite(self);
	i            = self->ud_mask + 1;
	self->ud_mask = new_mask;
	self->ud_used = item_count;
	self->ud_size = item_count;
	old_map      = self->ud_elem;
	self->ud_elem = new_map;
	UDict_LockEndWrite(self);
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


PRIVATE WUNUSED NONNULL((1)) int DCALL
udict_init(UDict *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	if unlikely(DeeArg_Unpack(argc, argv, "o:UniqueDict", &seq))
		goto err;
	return udict_init_sequence(self, seq);
err:
	return -1;
}


PRIVATE NONNULL((1)) void DCALL
udict_clear(UDict *__restrict self) {
	struct udict_item *elem;
	size_t mask;
	UDict_LockWrite(self);
	ASSERT((self->ud_elem == empty_dict_items) == (self->ud_mask == 0));
	ASSERT((self->ud_elem == empty_dict_items) == (self->ud_size == 0));
	ASSERT(self->ud_used <= self->ud_size);
	/* Extract the vector and mask. */
	elem         = self->ud_elem;
	mask         = self->ud_mask;
	self->ud_elem = (struct udict_item *)empty_dict_items;
	self->ud_mask = 0;
	self->ud_used = 0;
	self->ud_size = 0;
	UDict_LockEndWrite(self);
	/* Destroy the vector. */
	if (elem != empty_dict_items) {
		struct udict_item *iter, *end;
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
udict_visit(UDict *__restrict self, dvisit_t proc, void *arg) {
	UDict_LockRead(self);
	ASSERT((self->ud_elem == empty_dict_items) == (self->ud_mask == 0));
	ASSERT((self->ud_elem == empty_dict_items) == (self->ud_size == 0));
	ASSERT(self->ud_used <= self->ud_size);
	if (self->ud_elem != empty_dict_items) {
		struct udict_item *iter, *end;
		end = (iter = self->ud_elem) + (self->ud_mask + 1);
		for (; iter < end; ++iter) {
			if (!iter->di_key)
				continue;
			/* Visit all keys and associated values. */
			Dee_Visit(iter->di_key);
			Dee_XVisit(iter->di_value);
		}
	}
	UDict_LockEndRead(self);
}

/* Resize the hash size by a factor of 2 and re-insert all elements.
 * When `sizedir > 0', increase the hash; When `sizedir < 0', decrease it.
 * During this process all dummy items are discarded.
 * @return: true:  Successfully rehashed the Dict.
 * @return: false: Not enough memory. - The caller should collect some and try again. */
PRIVATE NONNULL((1)) bool DCALL
udict_rehash(UDict *__restrict self, int sizedir) {
	struct udict_item *new_vector, *iter, *end;
	size_t new_mask = self->ud_mask;
	if (sizedir > 0) {
		new_mask = (new_mask << 1) | 1;
		if unlikely(new_mask == 1)
			new_mask = 16 - 1; /* Start out bigger than 2. */
	} else if (sizedir < 0) {
		if unlikely(!self->ud_used) {
			ASSERT(!self->ud_used);
			/* Special case: delete the vector. */
			if (self->ud_size) {
				ASSERT(self->ud_elem != empty_dict_items);
				/* Must discard dummy items. */
				end = (iter = self->ud_elem) + (self->ud_mask + 1);
				for (; iter < end; ++iter) {
					ASSERT(iter->di_key == NULL ||
					       iter->di_key == dummy);
					if (iter->di_key == dummy)
						Dee_DecrefNokill(dummy);
				}
			}
			if (self->ud_elem != empty_dict_items)
				Dee_Free(self->ud_elem);
			self->ud_elem = (struct udict_item *)empty_dict_items;
			self->ud_mask = 0;
			self->ud_size = 0;
			return true;
		}
		new_mask = (new_mask >> 1);
		if (self->ud_used >= new_mask)
			return true;
	}
	ASSERT(self->ud_used < new_mask);
	ASSERT(self->ud_used <= self->ud_size);
	new_vector = (struct udict_item *)Dee_TryCallocc(new_mask + 1, sizeof(struct udict_item));
	if unlikely(!new_vector)
		return false;
	ASSERT((self->ud_elem == empty_dict_items) == (self->ud_mask == 0));
	ASSERT((self->ud_elem == empty_dict_items) == (self->ud_size == 0));
	if (self->ud_elem != empty_dict_items) {
		/* Re-insert all existing items into the new Dict vector. */
		end = (iter = self->ud_elem) + (self->ud_mask + 1);
		for (; iter < end; ++iter) {
			struct udict_item *item;
			dhash_t i, perturb;

			/* Skip dummy keys. */
			if (!iter->di_key || iter->di_key == dummy)
				continue;
			perturb = i = UHASH(iter->di_key) & new_mask;
			for (;; UDict_HashNx(i, perturb)) {
				item = &new_vector[i & new_mask];
				if (!item->di_key)
					break; /* Empty slot found. */
			}

			/* Transfer this object. */
			item->di_key   = iter->di_key;
			item->di_value = iter->di_value;
		}
		Dee_Free(self->ud_elem);

		/* With all dummy items gone, the size now equals what is actually used. */
		self->ud_size = self->ud_used;
	}
	ASSERT(self->ud_size == self->ud_used);
	self->ud_mask = new_mask;
	self->ud_elem = new_vector;
	return true;
}

/* This one's basically your hasitem operator. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
udict_contains(UDict *self, DeeObject *key) {
	dhash_t i, perturb;
	dhash_t hash = UHASH(key);
	UDict_LockRead(self);
	perturb = i = hash & self->ud_mask;
	for (;; UDict_HashNx(i, perturb)) {
		struct udict_item *item;
		item = &self->ud_elem[i & self->ud_mask];
		if (USAME(item->di_key, key)) {
			UDict_LockEndRead(self);
			return_true; /* Found the item. */
		}
		if (!item->di_key)
			break;
	}
	UDict_LockEndRead(self);
	return_false;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
udict_getitem(UDict *self, DeeObject *key) {
	dhash_t i, perturb;
	dhash_t hash = UHASH(key);
	UDict_LockRead(self);
	perturb = i = hash & self->ud_mask;
	for (;; UDict_HashNx(i, perturb)) {
		struct udict_item *item;
		item = &self->ud_elem[i & self->ud_mask];
		if (USAME(item->di_key, key)) {
			/* Found the item. */
			DREF DeeObject *result;
			result = item->di_value;
			Dee_Incref(result);
			UDict_LockEndRead(self);
			return result;
		}
		if (!item->di_key)
			break;
	}
	UDict_LockEndRead(self);
	err_unknown_key((DeeObject *)self, key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
UDict_GetItemDef(UDict *self, DeeObject *key, DeeObject *def) {
	dhash_t i, perturb;
	dhash_t hash = UHASH(key);
	UDict_LockRead(self);
	perturb = i = hash & self->ud_mask;
	for (;; UDict_HashNx(i, perturb)) {
		struct udict_item *item;
		item = &self->ud_elem[i & self->ud_mask];
		if (USAME(item->di_key, key)) {
			/* Found the item. */
			DREF DeeObject *result;
			result = item->di_value;
			Dee_Incref(result);
			UDict_LockEndRead(self);
			return result;
		}
		if (!item->di_key)
			break;
	}
	UDict_LockEndRead(self);
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
udict_popitem(UDict *self, DeeObject *key, DeeObject *def) {
	dhash_t i, perturb;
	dhash_t hash = UHASH(key);
	UDict_LockWrite(self);
	perturb = i = hash & self->ud_mask;
	for (;; UDict_HashNx(i, perturb)) {
		struct udict_item *item;
		item = &self->ud_elem[i & self->ud_mask];
		if (USAME(item->di_key, key)) {
			/* Found the item. */
			DREF DeeObject *result;
			result = item->di_value;
			Dee_Incref(dummy);
			item->di_key   = dummy;
			item->di_value = NULL;
			ASSERT(self->ud_used);
			if (--self->ud_used <= self->ud_size / 3)
				udict_rehash(self, -1);
			UDict_LockEndWrite(self);
			Dee_DecrefNokill(key);
			return result;
		}
		if (!item->di_key)
			break;
	}
	UDict_LockEndWrite(self);
	if (def)
		return_reference_(def);
	err_unknown_key((DeeObject *)self, key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
udict_delitem(UDict *self, DeeObject *key) {
	DREF DeeObject *pop_item;
	pop_item = udict_popitem(self, key, NULL);
	Dee_XDecref(pop_item);
	return likely(pop_item) ? 0 : -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
udict_setitem(UDict *self, DeeObject *key, DeeObject *value) {
	struct udict_item *first_dummy;
	dhash_t i, perturb, hash = UHASH(key);
again:
	UDict_LockWrite(self);
again_locked:
	first_dummy = NULL;
	perturb = i = hash & self->ud_mask;
	for (;; UDict_HashNx(i, perturb)) {
		struct udict_item *item;
		item = &self->ud_elem[i & self->ud_mask];
		if (!item->di_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->di_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (USAME(item->di_key, key)) {
			DREF DeeObject *oldval;
			oldval = item->di_value;
			Dee_Incref(value);
			item->di_value = value;
			UDict_LockEndWrite(self);
			Dee_Decref(oldval);
			return 0;
		}
	}
	if (first_dummy && self->ud_size + 1 < self->ud_mask) {
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);
		if (first_dummy->di_key)
			Dee_DecrefNokill(first_dummy->di_key);
		/* Fill in the target slot. */
		first_dummy->di_key   = key;
		first_dummy->di_value = value;
		Dee_Incref(key);
		Dee_Incref(value);
		++self->ud_used;
		++self->ud_size;
		/* Try to keep the Dict vector big at least twice as big as the element count. */
		if (self->ud_size * 2 > self->ud_mask)
			udict_rehash(self, 1);
		UDict_LockEndWrite(self);
		return 0;
	}
	/* Rehash the Dict and try again. */
	if (udict_rehash(self, 1))
		goto again_locked;
	UDict_LockEndWrite(self);
	if (Dee_CollectMemory(1))
		goto again;
	return -1;
}


#define SETITEM_SETOLD 0 /* if_exists: *p_old_value = GET_OLD_VALUE(); SET_OLD_ITEM(); return 1;
                          * else:      return 0; */
#define SETITEM_SETNEW 1 /* if_exists: *p_old_value = GET_OLD_VALUE(); return 1;
                          * else:      ADD_NEW_ITEM(); return 0; */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
udict_setitem_ex(UDict *self,
                 DeeObject *key,
                 DeeObject *value,
                 unsigned int mode,
                 DREF DeeObject **p_old_value) {
	struct udict_item *first_dummy;
	dhash_t i, perturb, hash = UHASH(key);
again:
	UDict_LockWrite(self);
again_locked:
	first_dummy = NULL;
	perturb = i = hash & self->ud_mask;
	for (;; UDict_HashNx(i, perturb)) {
		struct udict_item *item;
		item = &self->ud_elem[i & self->ud_mask];
		if (!item->di_key) {
			if (!first_dummy)
				first_dummy = item;
			break; /* Not found */
		}
		if (item->di_key == dummy) {
			first_dummy = item;
			continue;
		}
		if (USAME(item->di_key, key)) {
			DREF DeeObject *item_value;
			/* Found an existing key. */
			if (mode == SETITEM_SETOLD) {
				item_value = item->di_value;
				Dee_Incref(value);
				item->di_value = value;
				UDict_LockEndWrite(self);
				if (p_old_value) {
					*p_old_value = item_value; /* Inherit reference */
				} else {
					Dee_Decref(item_value);
				}
			} else {
				if (p_old_value) {
					item_value = item->di_value;
					Dee_Incref(item_value);
					*p_old_value = item_value;
				}
				UDict_LockEndWrite(self);
			}
			return 1;
		}
	}
	if (mode == SETITEM_SETOLD) {
		UDict_LockEndWrite(self);
		return 0;
	}
	if (first_dummy && self->ud_size + 1 < self->ud_mask) {
		ASSERT(first_dummy != empty_dict_items);
		ASSERT(!first_dummy->di_key ||
		       first_dummy->di_key == dummy);
		if (first_dummy->di_key)
			Dee_DecrefNokill(first_dummy->di_key);
		/* Fill in the target slot. */
		first_dummy->di_key   = key;
		first_dummy->di_value = value;
		Dee_Incref(key);
		Dee_Incref(value);
		++self->ud_used;
		++self->ud_size;
		/* Try to keep the Dict vector big at least twice as big as the element count. */
		if (self->ud_size * 2 > self->ud_mask)
			udict_rehash(self, 1);
		UDict_LockEndWrite(self);
		return 0;
	}
	/* Rehash the Dict and try again. */
	if (udict_rehash(self, 1))
		goto again_locked;
	UDict_LockEndWrite(self);
	if (Dee_CollectMemory(1))
		goto again;
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
udict_nsi_getsize(UDict *__restrict self) {
	return atomic_read(&self->ud_used);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
udict_nsi_setdefault(UDict *self, DeeObject *key, DeeObject *defl) {
	DeeObject *old_value;
	int error;
	error = udict_setitem_ex(self, key, defl, SETITEM_SETNEW, &old_value);
	if unlikely(error < 0)
		goto err;
	if (error == 1)
		return old_value;
	return_reference_(defl);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
udict_nsi_updateold(UDict *self, DeeObject *key,
                    DeeObject *value, DREF DeeObject **p_oldvalue) {
	return udict_setitem_ex(self, key, value, SETITEM_SETOLD, p_oldvalue);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
udict_nsi_insertnew(UDict *self, DeeObject *key,
                    DeeObject *value, DREF DeeObject **p_oldvalue) {
	int error;
	error = udict_setitem_ex(self, key, value, SETITEM_SETNEW, p_oldvalue);
	if unlikely(error < 0)
		goto err;
	return !error;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_size(UDict *__restrict self) {
	return DeeInt_NewSize(atomic_read(&self->ud_used));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_iter(UDict *__restrict self) {
	DREF UDictIterator *result;
	result = DeeObject_MALLOC(UDictIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &UDictIterator_Type);
	result->udi_dict = self;
	Dee_Incref(self);
	result->udi_next = atomic_read(&self->ud_elem);
done:
	return (DREF DeeObject *)result;
}






PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_clearfun(UDict *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	udict_clear(self);
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_get(UDict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *def = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:get", &key, &def))
		goto err;
	return UDict_GetItemDef(self, key, def);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_pop(UDict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *def = NULL;
	if (DeeArg_Unpack(argc, argv, "o|o:pop", &key, &def))
		goto err;
	return udict_popitem(self, key, def);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_popsomething(UDict *self, size_t argc, DeeObject *const *argv) {
	DREF DeeTupleObject *result;
	struct udict_item *iter;
	if (DeeArg_Unpack(argc, argv, ":popitem"))
		goto err;
	/* Allocate a tuple which we're going to fill with some key-value pair. */
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err;
	UDict_LockWrite(self);
	if unlikely(!self->ud_used) {
		UDict_LockEndWrite(self);
		DeeTuple_FreeUninitialized(result);
		err_empty_sequence((DeeObject *)self);
		goto err;
	}
	iter = self->ud_elem;
	while (!iter->di_key || iter->di_key == dummy) {
		ASSERT(iter != self->ud_elem + self->ud_mask);
		++iter;
	}
	DeeTuple_SET(result, 0, iter->di_key);   /* Inherit reference. */
	DeeTuple_SET(result, 1, iter->di_value); /* Inherit reference. */
	Dee_Incref(dummy);
	iter->di_key   = dummy;
	iter->di_value = NULL;
	ASSERT(self->ud_used);
	if (--self->ud_used <= self->ud_size / 3)
		udict_rehash(self, -1);
	UDict_LockEndWrite(self);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_setdefault(UDict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *value = Dee_None, *old_value;
	int error;
	if (DeeArg_Unpack(argc, argv, "o|o:setdefault", &key, &value))
		goto err;
	error = udict_setitem_ex(self, key, value, SETITEM_SETNEW, &old_value);
	if unlikely(error < 0)
		goto err;
	if (error == 1)
		return old_value;
	return_reference_(value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_setold(UDict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *value;
	int error;
	if (DeeArg_Unpack(argc, argv, "oo:setold", &key, &value))
		goto err;
	error = udict_setitem_ex(self, key, value, SETITEM_SETOLD, NULL);
	if unlikely(error < 0)
		goto err;
	return_bool_(error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_setnew(UDict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *value;
	int error;
	if (DeeArg_Unpack(argc, argv, "oo:setnew", &key, &value))
		goto err;
	error = udict_setitem_ex(self, key, value, SETITEM_SETNEW, NULL);
	if unlikely(error < 0)
		goto err;
	return_bool_(!error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_setold_ex(UDict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *value, *old_value, *result;
	int error;
	if (DeeArg_Unpack(argc, argv, "oo:setold_ex", &key, &value))
		goto err;
	error = udict_setitem_ex(self, key, value, SETITEM_SETOLD, &old_value);
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
udict_setnew_ex(UDict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *value, *old_value, *result;
	int error;
	if (DeeArg_Unpack(argc, argv, "oo:setnew_ex", &key, &value))
		goto err;
	error = udict_setitem_ex(self, key, value, SETITEM_SETNEW, &old_value);
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
udict_update(UDict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *items, *iterator;
	int error;
	if (DeeArg_Unpack(argc, argv, "o:update", &items))
		goto err;
	iterator = DeeObject_IterSelf(items);
	if unlikely(!iterator)
		goto err;
	error = udict_insert_iterator(self, iterator);
	Dee_Decref(iterator);
	if unlikely(error)
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_sizeof(UDict *self) {
	return DeeInt_NewSize(sizeof(UDict) +
	                      ((self->ud_mask + 1) *
	                       sizeof(struct udict_item)));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_repr(UDict *__restrict self) {
	struct unicode_printer p;
	dssize_t error;
	struct udict_item *iter, *end;
	bool is_first;
	struct udict_item *vector;
	size_t mask;
again:
	unicode_printer_init(&p);
	if (UNICODE_PRINTER_PRINT(&p, "collections.UniqueDict({ ") < 0)
		goto err;
	UDict_LockRead(self);
	is_first = true;
	vector   = self->ud_elem;
	mask     = self->ud_mask;
	end      = (iter = vector) + (mask + 1);
	for (; iter < end; ++iter) {
		DREF DeeObject *key, *value;
		if (iter->di_key == NULL ||
		    iter->di_key == dummy)
			continue;
		key   = iter->di_key;
		value = iter->di_value;
		Dee_Incref(key);
		Dee_Incref(value);
		UDict_LockEndRead(self);
		/* Print this key/value pair. */
		error = unicode_printer_printf(&p, "%s%r: %r", is_first ? "" : ", ", key, value);
		Dee_Decref(value);
		Dee_Decref(key);
		if unlikely(error < 0)
			goto err;
		is_first = false;
		UDict_LockRead(self);
		if (self->ud_elem != vector ||
		    self->ud_mask != mask)
			goto restart;
	}
	UDict_LockEndRead(self);
	if unlikely((is_first ? UNICODE_PRINTER_PRINT(&p, "})")
	                      : UNICODE_PRINTER_PRINT(&p, " })")) < 0)
		goto err;
	return unicode_printer_pack(&p);
restart:
	UDict_LockEndRead(self);
	unicode_printer_fini(&p);
	goto again;
err:
	unicode_printer_fini(&p);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
udict_printrepr(UDict *__restrict self,
                dformatprinter printer, void *arg) {
	dssize_t temp, result;
	struct udict_item *iter, *end;
	struct udict_item *vector;
	size_t mask;
	bool is_first;
	result = DeeFormat_PRINT(printer, arg, "collections.UDict({ ");
	if unlikely(result < 0)
		goto done;
	is_first = true;
	UDict_LockRead(self);
	vector = self->ud_elem;
	mask   = self->ud_mask;
	end    = (iter = vector) + (mask + 1);
	for (; iter < end; ++iter) {
		DREF DeeObject *key, *value;
		key = iter->di_key;
		if (key == NULL || key == dummy)
			continue;
		value = iter->di_value;
		Dee_Incref(key);
		Dee_Incref(value);
		UDict_LockEndRead(self);
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
		UDict_LockRead(self);
		if unlikely(self->ud_elem != vector ||
		            self->ud_mask != mask) {
			UDict_LockEndRead(self);
			temp = DeeFormat_PRINT(printer, arg, ", <UDict changed while being iterated>");
			if (temp < 0)
				goto err;
			result += temp;
			goto stop_after_changed;
		}
	}
	UDict_LockEndRead(self);
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



PRIVATE struct type_getset tpconst udict_getsets[] = {
	TYPE_GETTER_F("frozen", &URoDict_FromUDict, TYPE_GETSET_FNOREFESCAPE,
	              "->?AFrozen?.\n"
	              "Returns a read-only (frozen) copy of @this dict"),
	TYPE_GETTER_F("__sizeof__", &udict_sizeof, TYPE_GETSET_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst udict_members[] = {
	TYPE_MEMBER_FIELD("__mask__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(UDict, ud_mask)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(UDict, ud_size)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst udict_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &UDictIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &URoDict_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_method tpconst udict_methods[] = {
	TYPE_METHOD_F("get", &udict_get, TYPE_METHOD_FNOREFESCAPE,
	              "(key,def=!N)->\n"
	              "#r{The value associated with @key or @def when @key has no value associated}"),
	TYPE_METHOD_F("pop", &udict_pop, TYPE_METHOD_FNOREFESCAPE,
	              "(key)->\n"
	              "(key,def)->\n"
	              "#tKeyError{No @def was given and @key was not found}"
	              "Delete @key from @this and return its previously assigned value or @def when @key had no item associated"),
	TYPE_METHOD_F("clear", &udict_clearfun, TYPE_METHOD_FNOREFESCAPE,
	              "()\n"
	              "Clear all values from @this ?."),
	TYPE_METHOD_F("popitem", &udict_popsomething, TYPE_METHOD_FNOREFESCAPE,
	              "->?T2?O?O\n"
	              "#r{A random pair key-value pair that has been removed}"
	              "#tValueError{@this ?. was empty}"),
	TYPE_METHOD_F("setdefault", &udict_setdefault, TYPE_METHOD_FNOREFESCAPE,
	              "(key,def=!N)->\n"
	              "#r{The object currently assigned to @key}"
	              "Lookup @key in @this ?. and return its value if found. Otherwise, assign @def to @key and return it instead"),
	TYPE_METHOD_F("setold", &udict_setold, TYPE_METHOD_FNOREFESCAPE,
	              "(key,value)->?Dbool\n"
	              "#r{Indicative of @value having been assigned to @key}"
	              "Assign @value to @key, only succeeding when @key already existed to begin with"),
	TYPE_METHOD_F("setnew", &udict_setnew, TYPE_METHOD_FNOREFESCAPE,
	              "(key,value)->?Dbool\n"
	              "#r{Indicative of @value having been assigned to @key}"
	              "Assign @value to @key, only succeeding when @key didn't exist before"),
	TYPE_METHOD_F("setold_ex", &udict_setold_ex, TYPE_METHOD_FNOREFESCAPE,
	              "(key,value)->?T2?Dbool?O\n"
	              "#r{A pair of values (new-value-was-assigned, old-value-or-none)}"
	              "Same as #setold but also return the previously assigned object"),
	TYPE_METHOD_F("setnew_ex", &udict_setnew_ex, TYPE_METHOD_FNOREFESCAPE,
	              "(key,value)->?T2?Dbool?O\n"
	              "#r{A pair of values (new-value-was-assigned, old-value-or-none)}"
	              "Same as #setnew but return the previously assigned object on failure"),
	TYPE_METHOD_F("update", &udict_update, TYPE_METHOD_FNOREFESCAPE,
	              "(items:?S?T2?O?O)\n"
	              "Iterate @items and unpack each element into 2 others, using them as "
	              /**/ "key and value to insert into @this ?."),
	TYPE_METHOD_END
};

PRIVATE struct type_gc tpconst udict_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&udict_clear
};

PRIVATE struct type_nsi tpconst udict_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE | TYPE_SEQX_FRESIZABLE,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&udict_nsi_getsize,
			/* .nsi_nextkey    = */ (dfunptr_t)&udictiterator_nextkey,
			/* .nsi_nextvalue  = */ (dfunptr_t)&udictiterator_nextvalue,
			/* .nsi_getdefault = */ (dfunptr_t)&UDict_GetItemDef,
			/* .nsi_setdefault = */ (dfunptr_t)&udict_nsi_setdefault,
			/* .nsi_updateold  = */ (dfunptr_t)&udict_nsi_updateold,
			/* .nsi_insertnew  = */ (dfunptr_t)&udict_nsi_insertnew
		}
	}
};

PRIVATE struct type_seq udict_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&udict_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&udict_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&udict_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&udict_getitem,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&udict_delitem,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&udict_setitem,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &udict_nsi
};


INTERN DeeTypeObject UDict_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "UniqueDict",
	/* .tp_doc      = */ DOC("A mutable mapping-like container that uses ?Aid?O "
	                         /**/ "and ${x === y} to detect/prevent duplicates\n"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(UDict),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&udict_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&udict_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&udict_copy,
				/* .tp_any_ctor  = */ (dfunptr_t)&udict_init,
				TYPE_FIXED_ALLOCATOR_GC(UDict)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&udict_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&udict_deepload
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&udict_repr,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&udict_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&udict_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&udict_visit,
	/* .tp_gc            = */ &udict_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &udict_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ udict_methods,
	/* .tp_getsets       = */ udict_getsets,
	/* .tp_members       = */ udict_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ udict_class_members
};











#undef READ_ITEM
#define READ_ITEM(x) atomic_read(&(x)->urdi_next)

STATIC_ASSERT(offsetof(UDictIterator, udi_dict) == offsetof(URoDictIterator, urdi_dict));
STATIC_ASSERT(offsetof(UDictIterator, udi_next) == offsetof(URoDictIterator, urdi_next));
#define urodictiterator_copy  udictiterator_copy
#define urodictiterator_fini  udictiterator_fini
#define urodictiterator_visit udictiterator_visit
#define urodictiterator_cmp   udictiterator_cmp

PRIVATE WUNUSED NONNULL((1)) int DCALL
urodictiterator_bool(URoDictIterator *__restrict self) {
	struct udict_item *item = READ_ITEM(self);
	URoDict *dict           = self->urdi_dict;
	/* Check if the iterator is in-bounds.
	 * NOTE: Since this is nothing but a shallow boolean check anyways, there
	 *       is no need to lock the Dict since we're not dereferencing anything. */
	return (item >= dict->urd_elem &&
	        item < dict->urd_elem + (dict->urd_mask + 1));
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
urodictiterator_ctor(URoDictIterator *__restrict self) {
	self->urdi_dict = (DREF URoDict *)DeeObject_NewDefault(&URoDict_Type);
	if unlikely(!self->urdi_dict)
		goto err;
	self->urdi_next = self->urdi_dict->urd_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
urodictiterator_init(URoDictIterator *__restrict self,
                     size_t argc, DeeObject *const *argv) {
	URoDict *dict;
	if (DeeArg_Unpack(argc, argv, "o:_UniqueRoDictIterator", &dict))
		goto err;
	if (DeeObject_AssertTypeExact(dict, &URoDict_Type))
		goto err;
	self->urdi_dict = dict;
	Dee_Incref(dict);
	self->urdi_next = dict->urd_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) struct udict_item *DCALL
urodictiterator_nextitem(URoDictIterator *__restrict self) {
	struct udict_item *item, *end;
	end = self->urdi_dict->urd_elem + self->urdi_dict->urd_mask + 1;
	for (;;) {
		struct udict_item *old_item;
		item     = atomic_read(&self->urdi_next);
		old_item = item;
		if (item >= end)
			goto iter_exhausted;
		while (item < end && !item->di_key)
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->urdi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->urdi_next, old_item, item + 1))
			break;
	}
	return item;
iter_exhausted:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
urodictiterator_next(URoDictIterator *__restrict self) {
	struct udict_item *item;
	item = urodictiterator_nextitem(self);
	if (!item)
		return ITER_DONE;
	return DeeTuple_NewVector(2, (DREF DeeObject **)item);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
urodictiterator_nextkey(URoDictIterator *__restrict self) {
	struct udict_item *item;
	item = urodictiterator_nextitem(self);
	if (!item)
		return ITER_DONE;
	return_reference_(item->di_key);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
urodictiterator_nextvalue(URoDictIterator *__restrict self) {
	struct udict_item *item;
	item = urodictiterator_nextitem(self);
	if (!item)
		return ITER_DONE;
	return_reference_(item->di_value);
}


PRIVATE struct type_member tpconst urodictiterator_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(URoDictIterator, urdi_dict), "->?AFrozen?GUniqueDict"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject URoDictIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_UniqueRoDictIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&urodictiterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&urodictiterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)&urodictiterator_init,
				TYPE_FIXED_ALLOCATOR(URoSetIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&urodictiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&urodictiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&urodictiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &urodictiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&urodictiterator_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ urodictiterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};




STATIC_ASSERT(offsetof(URoDict, urd_size) == offsetof(UDict, ud_used));
#define urodict_bool udict_bool

#define URODICT_ALLOC(mask)  ((DREF URoDict *)DeeObject_Calloc(SIZEOF_URODICT(mask)))
#define SIZEOF_URODICT(mask) (offsetof(URoDict, urd_elem) + (((mask) + 1) * sizeof(struct udict_item)))
#define URODICT_INITIAL_MASK 0x03

INTERN WUNUSED DREF URoDict *DCALL URoDict_New(void) {
	DREF URoDict *result;
	result = (DREF URoDict *)DeeObject_Malloc(SIZEOF_URODICT(0));
	if unlikely(!result)
		goto done;
	result->urd_mask           = 0;
	result->urd_size           = 0;
	result->urd_elem[0].di_key = NULL;
	DeeObject_Init(result, &URoDict_Type);
done:
	return result;
}

PRIVATE WUNUSED DREF URoDict *DCALL
URoDict_NewWithHint(size_t num_items) {
	DREF URoDict *result;
	size_t mask = URODICT_INITIAL_MASK;
	while (mask <= num_items)
		mask = (mask << 1) | 1;
	mask   = (mask << 1) | 1;
	result = URODICT_ALLOC(mask);
	if unlikely(!result)
		goto done;
	result->urd_mask = mask;
	result->urd_size = 0;
	DeeObject_Init(result, &URoDict_Type);
done:
	return result;
}

PRIVATE WUNUSED DREF URoDict *DCALL
urodict_rehash(DREF URoDict *__restrict self,
               size_t old_mask, size_t new_mask) {
	DREF URoDict *result;
	size_t i;
	result = URODICT_ALLOC(new_mask);
	if unlikely(!result)
		goto done;
	for (i = 0; i <= old_mask; ++i) {
		size_t j, perturb;
		struct udict_item *item;
		if (!self->urd_elem[i].di_key)
			continue;
		perturb = j = UHASH(self->urd_elem[i].di_key) & new_mask;
		for (;; URoDict_HashNx(j, perturb)) {
			item = &result->urd_elem[j & new_mask];
			if (!item->di_key)
				break;
		}

		/* Copy the old item into the new slot. */
		memcpy(item, &self->urd_elem[i], sizeof(struct udict_item));
	}
	DeeObject_Free(self);
done:
	return result;
}

PRIVATE void DCALL
urodict_insert(DREF URoDict *__restrict self, size_t mask,
               size_t *__restrict p_elemcount,
               /*inherit(always)*/ DREF DeeObject *__restrict key,
               /*inherit(always)*/ DREF DeeObject *__restrict value) {
	size_t i, perturb, hash;
	struct udict_item *item;
	hash    = UHASH(key);
	perturb = i = hash & mask;
	for (;; URoDict_HashNx(i, perturb)) {
		item = &self->urd_elem[i & mask];
		if (!item->di_key)
			break;
		if (!USAME(item->di_key, key))
			continue;

		/* It _is_ the same key! (override it...) */
		--*p_elemcount;
		Dee_Decref(item->di_key);
		Dee_Decref(item->di_value);
		break;
	}

	/* Fill in the item. */
	++*p_elemcount;
	item->di_key   = key;   /* Inherit reference. */
	item->di_value = value; /* Inherit reference. */
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
URoDict_Insert(DREF URoDict **__restrict p_self,
               DeeObject *key, DeeObject *value) {
	URoDict *self = *p_self;
	if unlikely(self->urd_size * 2 > self->urd_mask) {
		size_t old_size = self->urd_size;
		size_t new_mask = (self->urd_mask << 1) | 1;
		self = urodict_rehash(self, self->urd_mask, new_mask);
		if unlikely(!self)
			goto err;
		self->urd_mask = new_mask;
		self->urd_size = old_size; /* `urd_size' is not saved by `rehash()' */
	}

	/* Insert the new key/value-pair into the Dict. */
	Dee_Incref(key);
	Dee_Incref(value);
	urodict_insert(self, self->urd_mask, &self->urd_size, key, value);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF URoDict *DCALL
URoDict_FromIterator_impl(DeeObject *__restrict self, size_t mask) {
	DREF URoDict *result, *new_result;
	DREF DeeObject *elem;
	size_t elem_count = 0;

	/* Construct a read-only Dict from an iterator. */
	result = URODICT_ALLOC(mask);
	if unlikely(!result)
		goto done;
	while (ITER_ISOK(elem = DeeObject_IterNext(self))) {
		int error;
		DREF DeeObject *key_and_value[2];
		error = DeeObject_Unpack(elem, 2, key_and_value);
		Dee_Decref(elem);
		if unlikely(error)
			goto err_r;

		/* Check if we must re-hash the resulting Dict. */
		if (elem_count * 2 > mask) {
			size_t new_mask = (mask << 1) | 1;
			new_result      = urodict_rehash(result, mask, new_mask);
			if unlikely(!new_result) {
				Dee_Decref(key_and_value[1]);
				Dee_Decref(key_and_value[0]);
				goto err_r;
			}
			result = new_result;
			mask   = new_mask;
		}

		/* Insert the key-value pair into the resulting Dict. */
		urodict_insert(result, mask, &elem_count, key_and_value[0], key_and_value[1]);
		if (DeeThread_CheckInterrupt())
			goto err_r;
	}
	if unlikely(!elem)
		goto err_r;
	/* Fill in control members and setup the resulting object. */
	result->urd_size = elem_count;
	result->urd_mask = mask;
	DeeObject_Init(result, &URoDict_Type);
done:
	return result;
err_r:
	for (elem_count = 0; elem_count <= mask; ++elem_count) {
		if (!result->urd_elem[elem_count].di_key)
			continue;
		Dee_Decref(result->urd_elem[elem_count].di_key);
		Dee_Decref(result->urd_elem[elem_count].di_value);
	}
	DeeObject_Free(result);
	return NULL;
}



INTERN WUNUSED NONNULL((1)) DREF URoDict *DCALL
URoDict_FromIterator(DeeObject *__restrict iterator) {
	return URoDict_FromIterator_impl(iterator, URODICT_INITIAL_MASK);
}

INTERN WUNUSED NONNULL((1)) DREF URoDict *DCALL
URoDict_FromSequence(DeeObject *__restrict sequence) {
	DREF URoDict *result;
	DREF DeeObject *iterator;
	DeeTypeObject *seqtype = Dee_TYPE(sequence);
	if (seqtype == &URoDict_Type)
		return_reference_((DREF URoDict *)sequence);

	/* TODO: Optimizations for `DeeDict_Type' */
	/* TODO: Optimizations for `DeeRoDict_Type' */
	/* TODO: Optimizations for `UDict_Type' */

	iterator = DeeObject_IterSelf(sequence);
	if unlikely(!iterator)
		goto err;
	result = URoDict_FromIterator(iterator);
	Dee_Decref(iterator);
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF URoDict *DCALL
URoDict_FromUDict(UDict *__restrict self) {
	/* TODO */
	return URoDict_FromSequence((DeeObject *)self);
}

PRIVATE WUNUSED NONNULL((1)) DREF URoDict *DCALL
urodict_deepcopy(URoDict *__restrict self) {
	DREF URoDict *result;
	size_t i;
	int temp;
	result = (DREF URoDict *)URoDict_NewWithHint(self->urd_size);
	if unlikely(!result)
		goto done;
	for (i = 0; i <= self->urd_mask; ++i) {
		DREF DeeObject *key_copy, *value_copy;
		/* Deep-copy the key & value */
		if (!self->urd_elem[i].di_key)
			continue;
		key_copy = DeeObject_DeepCopy(self->urd_elem[i].di_key);
		if unlikely(!key_copy)
			goto err;
		value_copy = DeeObject_DeepCopy(self->urd_elem[i].di_value);
		if unlikely(!value_copy) {
			Dee_Decref(key_copy);
			goto err;
		}
		/* Insert the copied key & value into the new Dict. */
		temp = URoDict_Insert(&result, key_copy, value_copy);
		Dee_Decref(value_copy);
		Dee_Decref(key_copy);
		if unlikely(temp)
			goto err;
	}
done:
	return result;
err:
	Dee_Clear(result);
	goto done;
}


PRIVATE WUNUSED DREF URoDict *DCALL
urodict_init(size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	if unlikely(DeeArg_Unpack(argc, argv, "o:_UniqueRoDict", &seq))
		goto err;
	return URoDict_FromSequence(seq);
err:
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
urodict_fini(URoDict *__restrict self) {
	size_t i;
	for (i = 0; i <= self->urd_mask; ++i) {
		if (!self->urd_elem[i].di_key)
			continue;
		Dee_Decref(self->urd_elem[i].di_key);
		Dee_Decref(self->urd_elem[i].di_value);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
urodict_visit(URoDict *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->urd_mask; ++i) {
		if (!self->urd_elem[i].di_key)
			continue;
		Dee_Visit(self->urd_elem[i].di_key);
		Dee_Visit(self->urd_elem[i].di_value);
	}
}


PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
urodict_printrepr(URoDict *__restrict self,
                  dformatprinter printer, void *arg) {
	dssize_t temp, result;
	bool is_first = true;
	size_t i;
	result = DeeFormat_PRINT(printer, arg, "collections.UniqueDict.Frozen({ ");
	if unlikely(result < 0)
		goto done;
	for (i = 0; i <= self->urd_mask; ++i) {
		if (!self->urd_elem[i].di_key)
			continue;
		if (!is_first)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_Printf(printer, arg, "%r: %r",
		                         self->urd_elem[i].di_key,
		                         self->urd_elem[i].di_value));
		is_first = false;
	}
	DO(err, is_first ? DeeFormat_PRINT(printer, arg, "})")
	                 : DeeFormat_PRINT(printer, arg, " })"));
done:
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
urodict_nsi_getsize(URoDict *__restrict self) {
	ASSERT(self->urd_size != (size_t)-2);
	return self->urd_size;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
URoDict_GetItemDef(URoDict *self, DeeObject *key, DeeObject *def) {
	size_t i, perturb, hash;
	struct udict_item *item;
	hash    = UHASH(key);
	perturb = i = URoDict_HashSt(self, hash);
	for (;; URoDict_HashNx(i, perturb)) {
		item = URoDict_HashIt(self, i);
		if (!item->di_key)
			break;
		if (!USAME(item->di_key, key))
			continue;
		/* Found it! */
		return_reference_(item->di_value);
	}
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
}

PRIVATE WUNUSED NONNULL((1)) DREF URoDictIterator *DCALL
urodict_iter(URoDict *__restrict self) {
	DREF URoDictIterator *result;
	result = DeeObject_MALLOC(URoDictIterator);
	if unlikely(!result)
		goto done;
	result->urdi_dict = self;
	result->urdi_next = self->urd_elem;
	Dee_Incref(self);
	DeeObject_Init(result, &URoDictIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
urodict_size(URoDict *__restrict self) {
	return DeeInt_NewSize(self->urd_size);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
urodict_contains(URoDict *self, DeeObject *key) {
	size_t i, perturb, hash;
	struct udict_item *item;
	hash    = UHASH(key);
	perturb = i = hash & self->urd_mask;
	for (;; URoDict_HashNx(i, perturb)) {
		item = &self->urd_elem[i & self->urd_mask];
		if (!item->di_key)
			break;
		if (USAME(item->di_key, key))
			return_true; /* Found it! */
	}
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
urodict_getitem(URoDict *self, DeeObject *key) {
	size_t i, perturb, hash;
	struct udict_item *item;
	hash    = UHASH(key);
	perturb = i = hash & self->urd_mask;
	for (;; URoDict_HashNx(i, perturb)) {
		item = &self->urd_elem[i & self->urd_mask];
		if (!item->di_key)
			break;
		if (USAME(item->di_key, key))
			return_reference_(item->di_value); /* Found it! */
	}
	err_unknown_key((DeeObject *)self, key);
	return NULL;
}



PRIVATE struct type_nsi tpconst urodict_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&urodict_nsi_getsize,
			/* .nsi_nextkey    = */ (dfunptr_t)&urodictiterator_nextkey,
			/* .nsi_nextvalue  = */ (dfunptr_t)&urodictiterator_nextvalue,
			/* .nsi_getdefault = */ (dfunptr_t)&URoDict_GetItemDef
		}
	}
};


PRIVATE struct type_seq urodict_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&urodict_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&urodict_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&urodict_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&urodict_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &urodict_nsi
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
urodict_get(URoDict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *def = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:get", &key, &def))
		goto err;
	return URoDict_GetItemDef(self, key, def);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
urodict_sizeof(URoDict *self) {
	return DeeInt_NewSize(offsetof(URoDict, urd_elem) +
	                      ((self->urd_mask + 1) *
	                       sizeof(struct udict_item)));
}


PRIVATE struct type_method tpconst urodict_methods[] = {
	TYPE_METHOD_F("get", &urodict_get, TYPE_METHOD_FNOREFESCAPE,
	              "(key,def=!N)->\n"
	              "#r{The value associated with @key or @def when @key has no value associated}"),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst urodict_getsets[] = {
	TYPE_GETTER("frozen", &DeeObject_NewRef, "->?."),
	TYPE_GETTER_F("__sizeof__", &urodict_sizeof, TYPE_GETSET_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst urodict_members[] = {
	TYPE_MEMBER_FIELD("__mask__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(URoDict, urd_mask)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(URoDict, urd_size)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst urodict_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &URoDictIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &URoDict_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject URoDict_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_UniqueRoDict",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&URoDict_New,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&urodict_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&urodict_init
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&urodict_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&urodict_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&urodict_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&urodict_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &urodict_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ urodict_methods,
	/* .tp_getsets       = */ urodict_getsets,
	/* .tp_members       = */ urodict_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ urodict_class_members
};


DECL_END

#endif /* !GUARD_DEX_COLLECTIONS_UDICT_C */
