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
#ifndef GUARD_DEEMON_OBJECTS_RODICT_C
#define GUARD_DEEMON_OBJECTS_RODICT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>
#include <deemon/system-features.h> /* bcmpc(), ... */
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0

typedef DeeRoDictObject RoDict;

typedef struct {
	OBJECT_HEAD
	RoDict             *rodi_dict; /* [1..1][const] The RoDict being iterated. */
	struct rodict_item *rodi_next; /* [?..1][in(rodi_dict->rd_elem)][atomic]
	                                * The first candidate for the next item. */
} RoDictIterator;
#define READ_ITEM(x) atomic_read(&(x)->rodi_next)

INTERN WUNUSED NONNULL((1)) int DCALL
rodictiterator_ctor(RoDictIterator *__restrict self) {
	self->rodi_dict = DeeRoDict_New();
	if unlikely(!self->rodi_dict)
		goto err;
	self->rodi_next = self->rodi_dict->rd_elem;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL
rodictiterator_init(RoDictIterator *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	RoDict *RoDict;
	if (DeeArg_Unpack(argc, argv, "o:_RoDictIterator", &RoDict))
		goto err;
	if (DeeObject_AssertTypeExact(RoDict, &DeeRoDict_Type))
		goto err;
	self->rodi_dict = RoDict;
	Dee_Incref(RoDict);
	self->rodi_next = RoDict->rd_elem;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
rodictiterator_copy(RoDictIterator *__restrict self,
                    RoDictIterator *__restrict other) {
	self->rodi_dict = other->rodi_dict;
	Dee_Incref(self->rodi_dict);
	self->rodi_next = READ_ITEM(other);
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
rodictiterator_fini(RoDictIterator *__restrict self) {
	Dee_Decref(self->rodi_dict);
}

PRIVATE NONNULL((1, 2)) void DCALL
rodictiterator_visit(RoDictIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->rodi_dict);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rodictiterator_bool(RoDictIterator *__restrict self) {
	struct rodict_item *item = READ_ITEM(self);
	RoDict *RoDict               = self->rodi_dict;
	for (;; ++item) {
		/* Check if the iterator is in-bounds. */
		if (item > RoDict->rd_elem + RoDict->rd_mask)
			return 0;
		if (item->di_key)
			break;
	}
	return 1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodictiterator_next_item(RoDictIterator *__restrict self) {
	struct rodict_item *item, *end;
	end = self->rodi_dict->rd_elem + self->rodi_dict->rd_mask + 1;
	for (;;) {
		struct rodict_item *old_item;
		item     = atomic_read(&self->rodi_next);
		old_item = item;
		if (item >= end)
			goto iter_exhausted;
		while (item < end && !item->di_key)
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item + 1))
			break;
	}
	return DeeTuple_Pack(2, item->di_key, item->di_value);
iter_exhausted:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodictiterator_next_key(RoDictIterator *__restrict self) {
	struct rodict_item *item, *end;
	end = self->rodi_dict->rd_elem + self->rodi_dict->rd_mask + 1;
	for (;;) {
		struct rodict_item *old_item;
		item     = atomic_read(&self->rodi_next);
		old_item = item;
		if (item >= end)
			goto iter_exhausted;
		while (item < end && !item->di_key)
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item + 1))
			break;
	}
	return_reference_(item->di_key);
iter_exhausted:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodictiterator_next_value(RoDictIterator *__restrict self) {
	struct rodict_item *item, *end;
	end = self->rodi_dict->rd_elem + self->rodi_dict->rd_mask + 1;
	for (;;) {
		struct rodict_item *old_item;
		item     = atomic_read(&self->rodi_next);
		old_item = item;
		if (item >= end)
			goto iter_exhausted;
		while (item < end && !item->di_key)
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item + 1))
			break;
	}
	return_reference_(item->di_value);
iter_exhausted:
	return ITER_DONE;
}

INTDEF DeeTypeObject RoDictIterator_Type;
#define DEFINE_ITERATOR_COMPARE(name, op)                      \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL      \
	name(RoDictIterator *self, RoDictIterator *other) {            \
		if (DeeObject_AssertType(other, &RoDictIterator_Type)) \
			goto err;                                          \
		return_bool(READ_ITEM(self) op READ_ITEM(other));      \
	err:                                                       \
		return NULL;                                           \
	}
DEFINE_ITERATOR_COMPARE(rodictiterator_eq, ==)
DEFINE_ITERATOR_COMPARE(rodictiterator_ne, !=)
DEFINE_ITERATOR_COMPARE(rodictiterator_lo, <)
DEFINE_ITERATOR_COMPARE(rodictiterator_le, <=)
DEFINE_ITERATOR_COMPARE(rodictiterator_gr, >)
DEFINE_ITERATOR_COMPARE(rodictiterator_ge, >=)
#undef DEFINE_ITERATOR_COMPARE

PRIVATE struct type_cmp rodictiterator_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodictiterator_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodictiterator_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodictiterator_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodictiterator_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodictiterator_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodictiterator_ge
};


PRIVATE struct type_member tpconst rodict_iterator_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(RoDictIterator, rodi_dict), "->?Ert:RoDict"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject RoDictIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoDictIterator",
	/* .tp_doc      = */ DOC("next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rodictiterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&rodictiterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)&rodictiterator_init,
				TYPE_FIXED_ALLOCATOR(RoDictIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rodictiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rodictiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rodictiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &rodictiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rodictiterator_next_item,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rodict_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};






PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_FromSequence(DeeObject *__restrict self) {
	DREF DeeObject *result;
	size_t length_hint;
	/* Optimization: Since rodicts are immutable, re-return if the
	 *               given sequence already is a read-only RoDict. */
	if (DeeRoDict_CheckExact(self))
		return_reference_(self);
	/* TODO: Optimization: if (DeeDict_CheckExact(self)) ... */
	/* Construct a read-only RoDict from an iterator. */
	self = DeeObject_IterSelf(self);
	if unlikely(!self)
		goto err;
	length_hint = DeeFastSeq_GetSize(self);
	result = likely(length_hint != DEE_FASTSEQ_NOTFAST)
	         ? DeeRoDict_FromIteratorWithHint(self, length_hint)
	         : DeeRoDict_FromIterator(self);
	Dee_Decref(self);
	return result;
err:
	return NULL;
}

#define RODICT_ALLOC(mask)  ((DREF RoDict *)DeeObject_Calloc(SIZEOF_RODICT(mask)))
#define SIZEOF_RODICT(mask) (offsetof(RoDict, rd_elem) + (((mask) + 1) * sizeof(struct rodict_item)))
#define RODICT_INITIAL_MASK 0x03

PRIVATE WUNUSED DREF RoDict *DCALL
rehash(DREF RoDict *__restrict self, size_t old_mask, size_t new_mask) {
	DREF RoDict *result;
	size_t i;
	result = RODICT_ALLOC(new_mask);
	if unlikely(!result)
		goto done;
	for (i = 0; i <= old_mask; ++i) {
		size_t j, perturb;
		struct rodict_item *item;
		if (!self->rd_elem[i].di_key)
			continue;
		perturb = j = self->rd_elem[i].di_hash & new_mask;
		for (;; RODICT_HASHNX(j, perturb)) {
			item = &result->rd_elem[j & new_mask];
			if (!item->di_key)
				break;
		}
		/* Copy the old item into the new slot. */
		memcpy(item, &self->rd_elem[i], sizeof(struct rodict_item));
	}
	DeeObject_Free(self);
done:
	return result;
}

/* NOTE: _Always_ inherits references to `key' and `value' */
PRIVATE int DCALL
insert(DREF RoDict *__restrict self, size_t mask,
       size_t *__restrict p_elemcount,
       /*inherit(always)*/ DREF DeeObject *__restrict key,
       /*inherit(always)*/ DREF DeeObject *__restrict value) {
	size_t i, perturb, hash;
	struct rodict_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & mask;
	for (;; RODICT_HASHNX(i, perturb)) {
		int error;
		item = &self->rd_elem[i & mask];
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;

		/* Same hash. -> Check if it's also the same key. */
		error = DeeObject_CompareEq(key, item->di_key);
		if unlikely(error < 0)
			goto err;
		if (!error)
			continue; /* Not the same key. */

		/* It _is_ the same key! (override it...) */
		--*p_elemcount;
		Dee_Decref(item->di_key);
		Dee_Decref(item->di_value);
		break;
	}

	/* Fill in the item. */
	++*p_elemcount;
	item->di_hash  = hash;
	item->di_key   = key;   /* Inherit reference. */
	item->di_value = value; /* Inherit reference. */
	return 0;
err:
	/* Always inherit references (even upon error) */
	Dee_Decref(value);
	Dee_Decref(key);
	return -1;
}

PUBLIC WUNUSED DREF RoDict *DCALL
DeeRoDict_New(void) {
	DREF RoDict *result;
	result = RODICT_ALLOC(RODICT_INITIAL_MASK);
	if unlikely(!result)
		goto done;
	result->rd_mask = RODICT_INITIAL_MASK;
	result->rd_size = 0;
	DeeObject_Init(result, &DeeRoDict_Type);
done:
	return result;
}

PUBLIC WUNUSED DREF RoDict *DCALL
DeeRoDict_NewWithHint(size_t num_items) {
	DREF RoDict *result;
	size_t mask = RODICT_INITIAL_MASK;
	while (mask <= num_items)
		mask = (mask << 1) | 1;
	mask   = (mask << 1) | 1;
	result = RODICT_ALLOC(mask);
	if unlikely(!result)
		goto done;
	result->rd_mask = mask;
	result->rd_size = 0;
	DeeObject_Init(result, &DeeRoDict_Type);
done:
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeRoDict_Insert(/*in|out*/ DREF RoDict **__restrict p_self,
                 DeeObject *key, DeeObject *value) {
	DREF RoDict *me = *p_self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeRoDict_Type);
	ASSERT(!DeeObject_IsShared(me));
	ASSERT(key != (DeeObject *)me);
	ASSERT(value != (DeeObject *)me);
	if unlikely(me->rd_size * 2 > me->rd_mask) {
		size_t old_size = me->rd_size;
		size_t new_mask = (me->rd_mask << 1) | 1;
		me              = rehash(me, me->rd_mask, new_mask);
		if unlikely(!me)
			goto err;
		me->rd_mask = new_mask;
		me->rd_size = old_size; /* `rd_size' is not saved by `rehash()' */
		*p_self = me;
	}

	/* Insert the new key/value-pair into the RoDict. */
	Dee_Incref(key);
	Dee_Incref(value);
	if (insert(me, me->rd_mask, &me->rd_size, key, value))
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_FromIterator_impl(DeeObject *__restrict self, size_t mask) {
	DREF RoDict *result;
	DREF DeeObject *elem;
	size_t elem_count = 0;
	/* Construct a read-only RoDict from an iterator. */
	result = RODICT_ALLOC(mask);
	if unlikely(!result)
		goto done;
	while (ITER_ISOK(elem = DeeObject_IterNext(self))) {
		int error;
		DREF DeeObject *key_and_value[2];
		error = DeeObject_Unpack(elem, 2, key_and_value);
		Dee_Decref(elem);
		if unlikely(error)
			goto err_r;
		/* Check if we must re-hash the resulting RoDict. */
		if (elem_count * 2 > mask) {
			DREF RoDict *new_result;
			size_t new_mask = (mask << 1) | 1;
			new_result      = rehash(result, mask, new_mask);
			if unlikely(!new_result) {
				Dee_Decref(key_and_value[1]);
				Dee_Decref(key_and_value[0]);
				goto err_r;
			}
			mask = new_mask;
			result = new_result;
		}
		/* Insert the key-value pair into the resulting RoDict. */
		if unlikely(insert(result, mask, &elem_count, key_and_value[0], key_and_value[1]))
			goto err_r;
		if (DeeThread_CheckInterrupt())
			goto err_r;
	}
	if unlikely(!elem)
		goto err_r;
	/* Fill in control members and setup the resulting object. */
	result->rd_size = elem_count;
	result->rd_mask = mask;
	DeeObject_Init(result, &DeeRoDict_Type);
done:
	return (DREF DeeObject *)result;
err_r:
	for (elem_count = 0; elem_count <= mask; ++elem_count) {
		if (!result->rd_elem[elem_count].di_key)
			continue;
		Dee_Decref(result->rd_elem[elem_count].di_key);
		Dee_Decref(result->rd_elem[elem_count].di_value);
	}
	DeeObject_Free(result);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_FromIteratorWithHint(DeeObject *__restrict self,
                               size_t num_items) {
	size_t mask = RODICT_INITIAL_MASK;
	while (mask <= num_items)
		mask = (mask << 1) | 1;
	mask = (mask << 1) | 1;
	return DeeRoDict_FromIterator_impl(self, mask);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_FromIterator(DeeObject *__restrict self) {
	return DeeRoDict_FromIterator_impl(self, RODICT_INITIAL_MASK);
}


PRIVATE WUNUSED NONNULL((1)) DREF RoDictIterator *DCALL
rodict_iter(RoDict *__restrict self) {
	DREF RoDictIterator *result;
	result = DeeObject_MALLOC(RoDictIterator);
	if unlikely(!result)
		goto done;
	result->rodi_dict = self;
	result->rodi_next = self->rd_elem;
	Dee_Incref(self);
	DeeObject_Init(result, &RoDictIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_size(RoDict *__restrict self) {
	return DeeInt_NewSize(self->rd_size);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_contains(RoDict *self,
                DeeObject *key) {
	size_t i, perturb, hash;
	struct rodict_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & self->rd_mask;
	for (;; RODICT_HASHNX(i, perturb)) {
		int error;
		item = &self->rd_elem[i & self->rd_mask];
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		error = DeeObject_CompareEq(key, item->di_key);
		if unlikely(error < 0)
			goto err;
		if (!error)
			continue; /* Non-equal keys. */
		/* Found it! */
		return_true;
	}
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_getitem(RoDict *self,
               DeeObject *key) {
	size_t i, perturb, hash;
	struct rodict_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & self->rd_mask;
	for (;; RODICT_HASHNX(i, perturb)) {
		int error;
		item = &self->rd_elem[i & self->rd_mask];
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		error = DeeObject_CompareEq(key, item->di_key);
		if unlikely(error < 0)
			goto err;
		if (!error)
			continue; /* Non-equal keys. */
		/* Found it! */
		return_reference_(item->di_value);
	}
	err_unknown_key((DeeObject *)self, key);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeRoDict_GetItemDef(DeeObject *self, DeeObject *key, DeeObject *def) {
	size_t i, perturb, hash;
	struct rodict_item *item;
	RoDict *me = (RoDict *)self;
	hash     = DeeObject_Hash(key);
	perturb = i = RODICT_HASHST(me, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		int error;
		item = RODICT_HASHIT(me, i);
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		error = DeeObject_CompareEq(key, item->di_key);
		if unlikely(error < 0)
			goto err;
		if (!error)
			continue; /* Non-equal keys. */
		/* Found it! */
		return_reference_(item->di_value);
	}
	if (def != ITER_DONE)
		Dee_Incref(def);
	return def;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeRoDict_GetItemString(DeeObject *__restrict self,
                        char const *__restrict key,
                        dhash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	RoDict *me = (RoDict *)self;
	perturb = i = RODICT_HASHST(me, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		item = RODICT_HASHIT(me, i);
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		if (!DeeString_Check(item->di_key))
			continue;
		if (strcmp(DeeString_STR(item->di_key), key) == 0)
			return_reference_(item->di_value); /* Found it! */
	}
	err_unknown_key_str(self, key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeRoDict_GetItemStringLen(DeeObject *__restrict self,
                           char const *__restrict key,
                           size_t keylen,
                           dhash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	RoDict *me = (RoDict *)self;
	perturb = i = RODICT_HASHST(me, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		item = RODICT_HASHIT(me, i);
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		if (!DeeString_Check(item->di_key))
			continue;
		if (DeeString_EqualsBuf(item->di_key, key, keylen))
			return_reference_(item->di_value); /* Found it! */
	}
	err_unknown_key_str(self, key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
DeeRoDict_GetItemStringDef(DeeObject *self,
                           char const *__restrict key,
                           dhash_t hash,
                           DeeObject *def) {
	size_t i, perturb;
	struct rodict_item *item;
	RoDict *me = (RoDict *)self;
	perturb = i = RODICT_HASHST(me, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		item = RODICT_HASHIT(me, i);
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		if (!DeeString_Check(item->di_key))
			continue;
		if (strcmp(DeeString_STR(item->di_key), key) == 0)
			return_reference_(item->di_value); /* Found it! */
	}
	return_reference_(def);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeRoDict_GetItemStringLenDef(DeeObject *self,
                              char const *__restrict key,
                              size_t keylen,
                              dhash_t hash,
                              DeeObject *def) {
	size_t i, perturb;
	struct rodict_item *item;
	RoDict *me = (RoDict *)self;
	perturb = i = RODICT_HASHST(me, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		item = RODICT_HASHIT(me, i);
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		if (!DeeString_Check(item->di_key))
			continue;
		if (DeeString_EqualsBuf(item->di_key, key, keylen))
			return_reference_(item->di_value); /* Found it! */
	}
	return_reference_(def);
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeRoDict_HasItemString(DeeObject *__restrict self,
                        char const *__restrict key,
                        dhash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	RoDict *me = (RoDict *)self;
	perturb = i = RODICT_HASHST(me, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		item = RODICT_HASHIT(me, i);
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		if (!DeeString_Check(item->di_key))
			continue;
		if (strcmp(DeeString_STR(item->di_key), key) == 0)
			return true; /* Found it! */
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeRoDict_HasItemStringLen(DeeObject *__restrict self,
                           char const *__restrict key,
                           size_t keylen,
                           dhash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	RoDict *me = (RoDict *)self;
	perturb = i = RODICT_HASHST(me, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		item = RODICT_HASHIT(me, i);
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		if (!DeeString_Check(item->di_key))
			continue;
		if (DeeString_EqualsBuf(item->di_key, key, keylen))
			return true; /* Found it! */
	}
	return false;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_ByHash(DeeObject *__restrict self, Dee_hash_t hash) {
	DREF DeeObject *result;
	DREF DeeTupleObject *match;
	dhash_t i, perturb;
	match = NULL;
	perturb = i = DeeRoDict_HashSt(self, hash);
	for (;; DeeRoDict_HashNx(i, perturb)) {
		struct rodict_item *item;
		item = DeeRoDict_HashIt(self, i);
		if (!item->di_key)
			break; /* Not found */
		if (item->di_hash != hash)
			continue; /* Non-matching hash */
		if unlikely(match) {
			/* There are multiple matches for `hash'. */
			Dee_Decref(match);
			/* XXX: RoDict-specific optimizations? */
			return DeeMap_HashFilter(self, hash);
		}
		match = DeeTuple_NewUninitialized(2);
		if unlikely(!match)
			goto err;
		DeeTuple_SET(match, 0, item->di_key);
		DeeTuple_SET(match, 1, item->di_value);
		Dee_Incref(item->di_key);
		Dee_Incref(item->di_value);
	}
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


PRIVATE NONNULL((1)) void DCALL
rodict_fini(RoDict *__restrict self) {
	size_t i;
	for (i = 0; i <= self->rd_mask; ++i) {
		if (!self->rd_elem[i].di_key)
			continue;
		Dee_Decref(self->rd_elem[i].di_key);
		Dee_Decref(self->rd_elem[i].di_value);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
rodict_visit(RoDict *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->rd_mask; ++i) {
		if (!self->rd_elem[i].di_key)
			continue;
		Dee_Visit(self->rd_elem[i].di_key);
		Dee_Visit(self->rd_elem[i].di_value);
	}
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rodict_bool(RoDict *__restrict self) {
	return self->rd_size != 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
rodict_printrepr(RoDict *__restrict self,
                 dformatprinter printer, void *arg) {
	dssize_t temp, result;
	bool is_first = true;
	size_t i;
	result = DeeFormat_PRINT(printer, arg, "{ ");
	if unlikely(result < 0)
		goto done;
	for (i = 0; i <= self->rd_mask; ++i) {
		if (!self->rd_elem[i].di_key)
			continue;
		if (!is_first)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_Printf(printer, arg, "%r: %r",
		                         self->rd_elem[i].di_key,
		                         self->rd_elem[i].di_value));
		is_first = false;
	}
	if (is_first) {
		DO(err, DeeFormat_PRINT(printer, arg, "}"));
	} else {
		DO(err, DeeFormat_PRINT(printer, arg, " }"));
	}
done:
	return result;
err:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
rodict_hash(RoDict *__restrict self) {
	size_t i;
	dhash_t result = DEE_HASHOF_EMPTY_SEQUENCE;
	for (i = 0; i <= self->rd_mask; ++i) {
		if (!self->rd_elem[i].di_key)
			continue;
		/* Note that we still combine the hashes for the key and value,
		 * thus not only mirroring the behavior of hash of the item (that
		 * is the tuple `(key, value)', including the order between the
		 * key and value within the hash, so that swapping the key and
		 * value would produce a different hash) */
		result ^= Dee_HashCombine(DeeObject_Hash(self->rd_elem[i].di_key),
		                          DeeObject_Hash(self->rd_elem[i].di_value));
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rodict_nsi_getsize(RoDict *__restrict self) {
	ASSERT(self->rd_size != (size_t)-2);
	return self->rd_size;
}


PRIVATE struct type_nsi tpconst rodict_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (dfunptr_t)&rodict_nsi_getsize,
			/* .nsi_nextkey    = */ (dfunptr_t)&rodictiterator_next_key,
			/* .nsi_nextvalue  = */ (dfunptr_t)&rodictiterator_next_value,
			/* .nsi_getdefault = */ (dfunptr_t)&DeeRoDict_GetItemDef
		}
	}
};


PRIVATE struct type_cmp rodict_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&rodict_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &rodict_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &rodict_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &rodict_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &rodict_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &rodict_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))NULL, // TODO: &rodict_ge,
};

PRIVATE struct type_seq rodict_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rodict_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rodict_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodict_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodict_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &rodict_nsi
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_get(RoDict *self, size_t argc, DeeObject *const *argv) {
	DeeObject *key, *def = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:get", &key, &def))
		goto err;
	return DeeRoDict_GetItemDef((DeeObject *)self, key, def);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_sizeof(RoDict *self) {
	return DeeInt_NewSize(offsetof(RoDict, rd_elem) +
	                      ((self->rd_mask + 1) *
	                       sizeof(struct rodict_item)));
}

INTDEF struct keyword seq_byhash_kwlist[];

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_byhash(RoDict *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DeeObject *template_;
	if (DeeArg_UnpackKw(argc, argv, kw, seq_byhash_kwlist, "o:byhash", &template_))
		goto err;
	return DeeRoDict_ByHash((DeeObject *)self,
	                        DeeObject_Hash(template_));
err:
	return NULL;
}


DOC_REF(map_get_doc);
DOC_REF(map_byhash_doc);

PRIVATE struct type_method tpconst rodict_methods[] = {
	TYPE_METHOD(STR_get, &rodict_get, DOC_GET(map_get_doc)),
	TYPE_KWMETHOD("byhash", &rodict_byhash, DOC_GET(map_byhash_doc)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst rodict_getsets[] = {
	TYPE_GETTER(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETTER("__sizeof__", &rodict_sizeof, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst rodict_members[] = {
	TYPE_MEMBER_FIELD("__mask__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(RoDict, rd_mask)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(RoDict, rd_size)),
	TYPE_MEMBER_END
};


PRIVATE struct type_member tpconst rodict_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &RoDictIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &DeeRoDict_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED DREF RoDict *DCALL rodict_ctor(void) {
	DREF RoDict *result;
	result = RODICT_ALLOC(0);
	if unlikely(!result)
		goto done;
	result->rd_mask = 0;
	result->rd_size = 0;
	DeeObject_Init(result, &DeeRoDict_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF RoDict *DCALL
rodict_deepcopy(RoDict *__restrict self) {
	DREF RoDict *result;
	size_t i;
	int temp;
	result = (DREF RoDict *)DeeRoDict_NewWithHint(self->rd_size);
	if unlikely(!result)
		goto done;
	for (i = 0; i <= self->rd_mask; ++i) {
		DREF DeeObject *key_copy, *value_copy;
		/* Deep-copy the key & value */
		if (!self->rd_elem[i].di_key)
			continue;
		key_copy = DeeObject_DeepCopy(self->rd_elem[i].di_key);
		if unlikely(!key_copy)
			goto err;
		value_copy = DeeObject_DeepCopy(self->rd_elem[i].di_value);
		if unlikely(!value_copy) {
			Dee_Decref(key_copy);
			goto err;
		}
		/* Insert the copied key & value into the new RoDict. */
		temp = DeeRoDict_Insert(&result, key_copy, value_copy);
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

PRIVATE WUNUSED DREF RoDict *DCALL
rodict_init(size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	if (DeeArg_Unpack(argc, argv, "o:_RoDict", &seq))
		goto err;
	return (DREF RoDict *)DeeRoDict_FromSequence(seq);
err:
	return NULL;
}




/* The main `_RoDict' container class. */
PUBLIC DeeTypeObject DeeRoDict_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoDict",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rodict_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&rodict_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&rodict_init,
				/* .tp_free      = */ (dfunptr_t)NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rodict_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&rodict_bool,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&rodict_printrepr
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rodict_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &rodict_cmp,
	/* .tp_seq           = */ &rodict_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rodict_methods,
	/* .tp_getsets       = */ rodict_getsets,
	/* .tp_members       = */ rodict_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rodict_class_members
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_RODICT_C */
