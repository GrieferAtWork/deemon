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
#ifndef GUARD_DEX_COLLECTIONS_UDICT_C
#define GUARD_DEX_COLLECTIONS_UDICT_C 1
#define DEE_SOURCE

#include "libcollections.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_*ALLOC*, DeeObject_Free, Dee_*alloc*, Dee_CollectMemory, Dee_CollectMemoryc, Dee_Free, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, _Dee_MallococBufsize */
#include <deemon/arg.h>             /* DeeArg_Unpack1 */
#include <deemon/bool.h>            /* Dee_True, return_false, return_true */
#include <deemon/dict.h>            /* DeeDict_Dummy */
#include <deemon/error-rt.h>        /* DeeRT_ErrEmptySequence, DeeRT_ErrUnknownKey */
#include <deemon/format.h>          /* DeeFormat_PRINT, DeeFormat_Printf */
#include <deemon/int.h>             /* DeeInt_NewSize */
#include <deemon/map.h>             /* DeeMapping_Type */
#include <deemon/method-hints.h>    /* TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/none.h>            /* Dee_None */
#include <deemon/object.h>
#include <deemon/seq.h>             /* DeeIterator_Type, DeeSeq_Unpack */
#include <deemon/string.h>          /* Dee_UNICODE_PRINTER_PRINT, Dee_unicode_printer* */
#include <deemon/system-features.h> /* memcpy*, memmovedownc */
#include <deemon/thread.h>          /* DeeThread_CheckInterrupt */
#include <deemon/tuple.h>           /* DeeTuple* */
#include <deemon/util/atomic.h>     /* atomic_cmpxch_weak_or_write, atomic_read */
#include <deemon/util/lock.h>       /* Dee_atomic_rwlock_init */

#include <hybrid/typecore.h> /* __SIZEOF_INT__, __SIZEOF_SIZE_T__ */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */

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
INTDEF NONNULL((1, 2)) void DCALL usetiterator_visit(USetIterator *__restrict self, Dee_visit_t proc, void *arg);
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
	DeeArg_Unpack1(err, argc, argv, "_UniqueDictIterator", &dict);
	if (DeeObject_AssertType(dict, &UDict_Type))
		goto err;
	self->udi_dict = dict;
	Dee_Incref(dict);
	self->udi_next = dict->ud_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
udictiterator_nextpair(UDictIterator *__restrict self,
                       DREF DeeObject *key_and_value[2]) {
	struct udict_item *item, *end;
	UDict *dict = self->udi_dict;
	UDict_LockRead(dict);
	end = dict->ud_elem + (dict->ud_mask + 1);
	for (;;) {
		struct udict_item *old_item;
		item = atomic_read(&self->udi_next);
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
	key_and_value[0] = item->di_key;
	key_and_value[1] = item->di_value;
	Dee_Incref(key_and_value[0]);
	Dee_Incref(key_and_value[1]);
	UDict_LockEndRead(dict);
	return 0;
dict_has_changed:
	UDict_LockEndRead(dict);
	return err_changed_sequence((DeeObject *)dict);
iter_exhausted:
	UDict_LockEndRead(dict);
	return 1;
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

PRIVATE struct type_iterator udictiterator_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&udictiterator_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&udictiterator_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&udictiterator_nextvalue,
	/* .tp_advance   = */ NULL, // TODO: (size_t (DCALL *)(DeeObject *__restrict, size_t))&udictiterator_advance,
};



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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ USetIterator,
			/* tp_ctor:        */ &udictiterator_ctor,
			/* tp_copy_ctor:   */ &udictiterator_copy,
			/* tp_deep_ctor:   */ NULL, /* TODO */
			/* tp_any_ctor:    */ &udictiterator_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&udictiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&udictiterator_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&udictiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &udictiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ &udictiterator_iterator,
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
	Dee_weakref_support_fini(self);
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
		if unlikely(DeeSeq_Unpack(elem, 2, key_and_value))
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

#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define udict_setitem_as_foreach_PTR ((Dee_foreach_pair_t)(Dee_funptr_t)&udict_setitem)
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
#define udict_setitem_as_foreach_PTR (&udict_setitem_as_foreach)
PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
udict_setitem_as_foreach(void *arg, DeeObject *key, DeeObject *value) {
	return udict_setitem((UDict *)arg, key, value);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
udict_init_sequence(UDict *__restrict self,
                    DeeObject *__restrict sequence) {
	/* TODO: Optimizations for `DeeDict_Type' */
	/* TODO: Optimizations for `DeeRoDict_Type' */
	/* TODO: Optimizations for `URoDict_Type' */

	self->ud_mask = 0;
	self->ud_size = 0;
	self->ud_used = 0;
	self->ud_elem = (struct udict_item *)empty_dict_items;
	Dee_atomic_rwlock_init(&self->ud_lock);
	Dee_weakref_support_init(self);
	if (DeeObject_ForeachPair(sequence, udict_setitem_as_foreach_PTR, self))
		goto err_self;
	return 0;
err_self:
	udict_fini(self);
/*err:*/
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
udict_ctor(UDict *__restrict self) {
	self->ud_mask = 0;
	self->ud_size = 0;
	self->ud_used = 0;
	self->ud_elem = (struct udict_item *)empty_dict_items;
	Dee_atomic_rwlock_init(&self->ud_lock);
	Dee_weakref_support_init(self);
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
			if (Dee_CollectMemoryc(other->ud_mask + 1, sizeof(struct udict_item)))
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
	Dee_weakref_support_init(self);
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
		Dee_hash_t j, perturb, hash;
		hash    = UHASH(items[i].e_key);
		perturb = j = hash & new_mask;
		for (;; UDict_HashNx(j, perturb)) {
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
	DeeArg_Unpack1(err, argc, argv, "UniqueDict", &seq);
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
udict_visit(UDict *__restrict self, Dee_visit_t proc, void *arg) {
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
			Dee_hash_t i, perturb;

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
	Dee_hash_t i, perturb;
	Dee_hash_t hash = UHASH(key);
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
udict_bounditem(UDict *self, DeeObject *key) {
	Dee_hash_t i, perturb;
	Dee_hash_t hash = UHASH(key);
	UDict_LockRead(self);
	perturb = i = hash & self->ud_mask;
	for (;; UDict_HashNx(i, perturb)) {
		struct udict_item *item;
		item = &self->ud_elem[i & self->ud_mask];
		if (USAME(item->di_key, key)) {
			UDict_LockEndRead(self);
			return Dee_BOUND_YES; /* Found the item. */
		}
		if (!item->di_key)
			break;
	}
	UDict_LockEndRead(self);
	return Dee_BOUND_MISSING;
}

#ifdef CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS
#define udict_hasitem udict_bounditem
#else /* CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
udict_hasitem(UDict *self, DeeObject *key) {
	int bound = udict_bounditem(self, key);
	return Dee_BOUND_ASHAS_NOEXCEPT(bound);
}
#endif /* !CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
udict_getitem(UDict *self, DeeObject *key) {
	Dee_hash_t i, perturb;
	Dee_hash_t hash = UHASH(key);
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
	DeeRT_ErrUnknownKey(self, key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
udict_trygetitem(UDict *self, DeeObject *key) {
	Dee_hash_t i, perturb;
	Dee_hash_t hash = UHASH(key);
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
	return ITER_DONE;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
udict_mh_clear(UDict *__restrict self) {
	udict_clear(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
udict_mh_pop(UDict *self, DeeObject *key) {
	Dee_hash_t i, perturb;
	Dee_hash_t hash = UHASH(key);
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
	DeeRT_ErrUnknownKey(self, key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
udict_mh_pop_with_default(UDict *self, DeeObject *key, DeeObject *def) {
	Dee_hash_t i, perturb;
	Dee_hash_t hash = UHASH(key);
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
	return_reference_(def);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
udict_delitem(UDict *self, DeeObject *key) {
	DREF DeeObject *pop_item;
	pop_item = udict_mh_pop_with_default(self, key, Dee_None);
	if unlikely(!pop_item)
		goto err;
	Dee_Decref(pop_item);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
udict_setitem(UDict *self, DeeObject *key, DeeObject *value) {
	struct udict_item *first_dummy;
	Dee_hash_t i, perturb, hash = UHASH(key);
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


PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
udict_mh_setold_ex(UDict *self, DeeObject *key, DeeObject *value) {
	struct udict_item *first_dummy;
	Dee_hash_t i, perturb, hash = UHASH(key);
/*again:*/
	UDict_LockWrite(self);
/*again_locked:*/
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
			item_value = item->di_value;
			Dee_Incref(value);
			item->di_value = value;
			UDict_LockEndWrite(self);
			return item_value; /* Inherit reference */
		}
	}
	UDict_LockEndWrite(self);
	return ITER_DONE;
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
udict_mh_setnew_ex(UDict *self, DeeObject *key, DeeObject *value) {
	struct udict_item *first_dummy;
	Dee_hash_t i, perturb, hash = UHASH(key);
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
			item_value = item->di_value;
			Dee_Incref(item_value);
			UDict_LockEndWrite(self);
			return item_value;
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
		return ITER_DONE;
	}

	/* Rehash the Dict and try again. */
	if (udict_rehash(self, 1))
		goto again_locked;
	UDict_LockEndWrite(self);
	if (Dee_CollectMemory(1))
		goto again;
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
udict_size(UDict *__restrict self) {
	return atomic_read(&self->ud_used);
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
	return Dee_AsObject(result);
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_mh_popitem(UDict *__restrict self) {
	DREF DeeTupleObject *result;
	struct udict_item *iter;
	/* Allocate a tuple which we're going to fill with some key-value pair. */
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err;
	UDict_LockWrite(self);
	if unlikely(!self->ud_used) {
		UDict_LockEndWrite(self);
		DeeTuple_FreeUninitializedPair(result);
		DeeRT_ErrEmptySequence(self);
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
	return Dee_AsObject(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
udict_mh_update(UDict *self, DeeObject *items) {
	int error;
	DREF DeeObject *iterator;
	iterator = DeeObject_Iter(items);
	if unlikely(!iterator)
		goto err;
	error = udict_insert_iterator(self, iterator);
	Dee_Decref(iterator);
	return error;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_sizeof(UDict *self) {
	return DeeInt_NewSize(sizeof(UDict) +
	                      ((self->ud_mask + 1) *
	                       sizeof(struct udict_item)));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
udict_repr(UDict *__restrict self) {
	struct Dee_unicode_printer p;
	Dee_ssize_t error;
	struct udict_item *iter, *end;
	bool is_first;
	struct udict_item *vector;
	size_t mask;
again:
	Dee_unicode_printer_init(&p);
	if (Dee_UNICODE_PRINTER_PRINT(&p, "collections.UniqueDict({ ") < 0)
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
		error = Dee_unicode_printer_printf(&p, "%s%r: %r", is_first ? "" : ", ", key, value);
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
	if unlikely((is_first ? Dee_UNICODE_PRINTER_PRINT(&p, "})")
	                      : Dee_UNICODE_PRINTER_PRINT(&p, " })")) < 0)
		goto err;
	return Dee_unicode_printer_pack(&p);
restart:
	UDict_LockEndRead(self);
	Dee_unicode_printer_fini(&p);
	goto again;
err:
	Dee_unicode_printer_fini(&p);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
udict_printrepr(UDict *__restrict self,
                Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
udict_foreach(UDict *self, Dee_foreach_pair_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	UDict_LockRead(self);
	for (i = 0; i <= self->ud_mask; ++i) {
		DREF DeeObject *key, *value;
		key = self->ud_elem[i].di_key;
		if (!key || key == dummy)
			continue;
		value = self->ud_elem[i].di_value;
		Dee_Incref(key);
		Dee_Incref(value);
		UDict_LockEndRead(self);
		temp = (*proc)(arg, key, value);
		Dee_Decref_unlikely(value);
		Dee_Decref_unlikely(key);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		UDict_LockRead(self);
	}
	UDict_LockEndRead(self);
	return result;
err_temp:
	return temp;
}



PRIVATE struct type_getset tpconst udict_getsets[] = {
	TYPE_GETTER_AB_F("frozen", &URoDict_FromUDict, METHOD_FNOREFESCAPE,
	                 "->?AFrozen?.\n"
	                 "Returns a read-only (frozen) copy of @this dict"),
	TYPE_GETTER_AB("cached", &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB_F("__sizeof__", &udict_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
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
	TYPE_MEMBER_CONST("__map_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_END
};

PRIVATE struct type_method tpconst udict_methods[] = {
	TYPE_METHOD_HINTREF(Mapping_pop),
	TYPE_METHOD_HINTREF(Sequence_clear),
	TYPE_METHOD_HINTREF(Mapping_popitem),
	TYPE_METHOD_HINTREF(Mapping_update),
	TYPE_METHOD_HINTREF(Mapping_setold_ex),
	TYPE_METHOD_HINTREF(Mapping_setnew_ex),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst udict_methods_hints[] = {
	TYPE_METHOD_HINT_F(seq_clear, &udict_mh_clear, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_pop, &udict_mh_pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_pop_with_default, &udict_mh_pop_with_default, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_popitem, &udict_mh_popitem, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_update, &udict_mh_update, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setold_ex, &udict_mh_setold_ex, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setnew_ex, &udict_mh_setnew_ex, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_gc tpconst udict_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&udict_clear
};

PRIVATE struct type_seq udict_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&udict_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&udict_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&udict_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&udict_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&udict_setitem,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&udict_foreach,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&udict_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&udict_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&udict_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&udict_size,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&udict_trygetitem,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};


INTERN DeeTypeObject UDict_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "UniqueDict",
	/* .tp_doc      = */ DOC("A mutable mapping-like container that uses ?Aid?O "
	                         /**/ "and ${x === y} to detect/prevent duplicates\n"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC,
	/* .tp_weakrefs = */ Dee_WEAKREF_SUPPORT_ADDR(UDict),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ UDict,
			/* tp_ctor:        */ &udict_ctor,
			/* tp_copy_ctor:   */ &udict_copy,
			/* tp_deep_ctor:   */ &udict_copy,
			/* tp_any_ctor:    */ &udict_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
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
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&udict_printrepr
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&udict_visit,
	/* .tp_gc            = */ &udict_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &udict_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ udict_methods,
	/* .tp_getsets       = */ udict_getsets,
	/* .tp_members       = */ udict_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ udict_class_members,
	/* .tp_method_hints  = */ udict_methods_hints,
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
	DeeArg_Unpack1(err, argc, argv, "_UniqueRoDictIterator", &dict);
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
urodictiterator_nextpair(URoDictIterator *__restrict self,
                         DREF DeeObject *key_and_value[2]) {
	struct udict_item *item;
	item = urodictiterator_nextitem(self);
	if (!item)
		return 1;
	key_and_value[0] = item->di_key;
	key_and_value[1] = item->di_value;
	Dee_Incref(key_and_value[0]);
	Dee_Incref(key_and_value[1]);
	return 0;
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

PRIVATE struct type_iterator urodictiterator_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&urodictiterator_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&urodictiterator_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&urodictiterator_nextvalue,
	/* .tp_advance   = */ NULL, // TODO: (size_t (DCALL *)(DeeObject *__restrict, size_t))&urodictiterator_advance,
};

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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ URoSetIterator,
			/* tp_ctor:        */ &urodictiterator_ctor,
			/* tp_copy_ctor:   */ &urodictiterator_copy,
			/* tp_deep_ctor:   */ NULL, /* TODO */
			/* tp_any_ctor:    */ &urodictiterator_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&urodictiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&urodictiterator_bool
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&urodictiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &urodictiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ &urodictiterator_iterator,
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

#define URoDict_MALLOC(mask)   DeeObject_MALLOCC_SAFE(URoDict, urd_elem, (mask) + 1)
#define URoDict_ALLOC(mask)    DeeObject_CALLOCC_SAFE(URoDict, urd_elem, (mask) + 1)
#define URoDict_TRYALLOC(mask) DeeObject_TRYCALLOCC_SAFE(URoDict, urd_elem, (mask) + 1)
#define URODICT_INITIAL_MASK   0x03

INTERN WUNUSED DREF URoDict *DCALL URoDict_New(void) {
	DREF URoDict *result;
	result = URoDict_MALLOC(0);
	if unlikely(!result)
		goto done;
	result->urd_mask = 0;
	result->urd_size = 0;
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
	result = URoDict_ALLOC(mask);
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
	result = URoDict_ALLOC(new_mask);
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
	result->urd_size = self->urd_size;
	result->urd_mask = new_mask;
	DeeObject_Free(self);
done:
	return result;
}

PRIVATE NONNULL((1, 2, 3)) void DCALL
urodict_insert(DREF URoDict *__restrict self,
               /*inherit(always)*/ DREF DeeObject *__restrict key,
               /*inherit(always)*/ DREF DeeObject *__restrict value) {
	size_t i, perturb, hash;
	struct udict_item *item;
	hash    = UHASH(key);
	perturb = i = hash & self->urd_mask;
	for (;; URoDict_HashNx(i, perturb)) {
		item = &self->urd_elem[i & self->urd_mask];
		if (!item->di_key)
			break;
		if (!USAME(item->di_key, key))
			continue;

		/* It _is_ the same key! (override it...) */
		--self->urd_size;
		Dee_Decref(item->di_key);
		Dee_Decref(item->di_value);
		break;
	}

	/* Fill in the item. */
	++self->urd_size;
	item->di_key   = key;   /* Inherit reference. */
	item->di_value = value; /* Inherit reference. */
}

PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
URoDict_Insert(/*URoDict **p_self*/ void *arg, DeeObject *key, DeeObject *value) {
	URoDict **p_self = (URoDict **)arg;
	URoDict *self = *p_self;
	if unlikely(self->urd_size * 2 > self->urd_mask) {
		size_t new_mask = (self->urd_mask << 1) | 1;
		self = urodict_rehash(self, self->urd_mask, new_mask);
		if unlikely(!self)
			goto err;
	}

	/* Insert the new key/value-pair into the Dict. */
	Dee_Incref(key);
	Dee_Incref(value);
	urodict_insert(self, key, value);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF URoDict *DCALL
URoDict_FromSequence_fallback(DeeObject *__restrict self) {
	DREF URoDict *result;
	size_t initial_mask = URODICT_INITIAL_MASK;
	size_t sizehint = DeeObject_SizeFast(self);
	if (sizehint != (size_t)-1) {
		initial_mask = 1;
		sizehint <<= 1;
		while (initial_mask < sizehint)
			initial_mask = (initial_mask << 1) | 1;
	}

	/* Construct a read-only Dict from an iterator. */
	result = URoDict_TRYALLOC(initial_mask);
	if unlikely(!result) {
		initial_mask = 1;
		result = URoDict_ALLOC(initial_mask);
		if unlikely(!result)
			goto err;
	}
	result->urd_mask = initial_mask;
	if (DeeObject_ForeachPair(self, &URoDict_Insert, &result))
		goto err_r;
	DeeObject_Init(result, &URoDict_Type);
	return result;
	{
		size_t i;
err_r:
		for (i = 0; i <= result->urd_mask; ++i) {
			if (!result->urd_elem[i].di_key)
				continue;
			Dee_Decref(result->urd_elem[i].di_key);
			Dee_Decref(result->urd_elem[i].di_value);
		}
	}
	DeeObject_Free(result);
err:
	return NULL;
}



INTERN WUNUSED NONNULL((1)) DREF URoDict *DCALL
URoDict_FromSequence(DeeObject *__restrict sequence) {
	DeeTypeObject *seqtype = Dee_TYPE(sequence);
	if (seqtype == &URoDict_Type)
		return_reference_((DREF URoDict *)sequence);

	/* TODO: Optimizations for `DeeDict_Type' */
	/* TODO: Optimizations for `DeeRoDict_Type' */
	/* TODO: Optimizations for `UDict_Type' */

	return URoDict_FromSequence_fallback(sequence);
}

INTERN WUNUSED NONNULL((1)) DREF URoDict *DCALL
URoDict_FromUDict(UDict *__restrict self) {
	/* TODO */
	return URoDict_FromSequence(Dee_AsObject(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF URoDict *DCALL
urodict_deepcopy(URoDict *__restrict self) {
	DREF URoDict *result;
	size_t i;
	result = (DREF URoDict *)URoDict_NewWithHint(self->urd_size);
	if unlikely(!result)
		goto done;
	for (i = 0; i <= self->urd_mask; ++i) {
		Dee_ssize_t temp;
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
	DeeArg_Unpack1(err, argc, argv, "_UniqueRoDict", &seq);
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
urodict_visit(URoDict *__restrict self, Dee_visit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->urd_mask; ++i) {
		if (!self->urd_elem[i].di_key)
			continue;
		Dee_Visit(self->urd_elem[i].di_key);
		Dee_Visit(self->urd_elem[i].di_value);
	}
}


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
urodict_printrepr(URoDict *__restrict self,
                  Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
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
urodict_size(URoDict *__restrict self) {
	ASSERT(self->urd_size != (size_t)-2);
	return self->urd_size;
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
	DeeRT_ErrUnknownKey(self, key);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
urodict_bounditem(URoDict *self, DeeObject *key) {
	size_t i, perturb, hash;
	struct udict_item *item;
	hash    = UHASH(key);
	perturb = i = hash & self->urd_mask;
	for (;; URoDict_HashNx(i, perturb)) {
		item = &self->urd_elem[i & self->urd_mask];
		if (!item->di_key)
			break;
		if (USAME(item->di_key, key))
			return Dee_BOUND_YES; /* Found it! */
	}
	return Dee_BOUND_MISSING;
}

#ifdef CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS
#define urodict_hasitem urodict_bounditem
#else /* CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
urodict_hasitem(URoDict *self, DeeObject *key) {
	int bound = urodict_bounditem(self, key);
	return Dee_BOUND_ASHAS_NOEXCEPT(bound);
}
#endif /* !CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
urodict_trygetitem(URoDict *self, DeeObject *key) {
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
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
urodict_foreach(URoDict *self, Dee_foreach_pair_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	for (i = 0; i <= self->urd_mask; ++i) {
		DeeObject *key = self->urd_elem[i].di_key;
		if (!key)
			continue;
		temp = (*proc)(arg, key, self->urd_elem[i].di_value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}


PRIVATE struct type_seq urodict_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&urodict_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&urodict_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&urodict_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&urodict_foreach,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&urodict_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&urodict_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&urodict_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&urodict_size,
	/* .tp_getitem_index              = */ NULL,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&urodict_trygetitem,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
urodict_sizeof(URoDict *self) {
	size_t result;
	result = _Dee_MallococBufsize(offsetof(URoDict, urd_elem),
	                              self->urd_mask + 1,
	                              sizeof(struct udict_item));
	return DeeInt_NewSize(result);
}


PRIVATE struct type_getset tpconst urodict_getsets[] = {
	TYPE_GETTER_AB("frozen", &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB("cached", &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB_F("__sizeof__", &urodict_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
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
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &URoDict_New,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &urodict_deepcopy,
			/* tp_any_ctor:    */ &urodict_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */,
			/* tp_free:        */ NULL
		),
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
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&urodict_printrepr
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&urodict_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &urodict_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ urodict_getsets,
	/* .tp_members       = */ urodict_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ urodict_class_members
};


DECL_END

#endif /* !GUARD_DEX_COLLECTIONS_UDICT_C */
