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
#include <deemon/tuple.h>

#include <hybrid/atomic.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

typedef DeeRoDictObject Dict;

typedef struct {
	OBJECT_HEAD
	Dict               *di_dict; /* [1..1][const] The Dict being iterated. */
	struct rodict_item *di_next; /* [?..1][in(di_dict->rd_elem)][atomic]
	                              * The first candidate for the next item. */
} DictIterator;

#ifdef CONFIG_NO_THREADS
#define READ_ITEM(x)            ((x)->di_next)
#else /* CONFIG_NO_THREADS */
#define READ_ITEM(x) ATOMIC_READ((x)->di_next)
#endif /* !CONFIG_NO_THREADS */

INTERN int DCALL
rodictiterator_ctor(DictIterator *__restrict self) {
	self->di_dict = (Dict *)DeeRoDict_New();
	if unlikely(!self->di_dict)
		return -1;
	self->di_next = self->di_dict->rd_elem;
	return 0;
}

INTERN int DCALL
rodictiterator_init(DictIterator *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
	Dict *Dict;
	if (DeeArg_Unpack(argc, argv, "o:_RoDictIterator", &Dict) ||
	    DeeObject_AssertTypeExact((DeeObject *)Dict, &DeeRoDict_Type))
		return -1;
	self->di_dict = Dict;
	Dee_Incref(Dict);
	self->di_next = Dict->rd_elem;
	return 0;
}

INTERN int DCALL
rodictiterator_copy(DictIterator *__restrict self,
                    DictIterator *__restrict other) {
	self->di_dict = other->di_dict;
	Dee_Incref(self->di_dict);
	self->di_next = READ_ITEM(other);
	return 0;
}

PRIVATE void DCALL
rodictiterator_fini(DictIterator *__restrict self) {
	Dee_Decref(self->di_dict);
}

PRIVATE void DCALL
rodictiterator_visit(DictIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->di_dict);
}

PRIVATE int DCALL
rodictiterator_bool(DictIterator *__restrict self) {
	struct rodict_item *item = READ_ITEM(self);
	Dict *Dict               = self->di_dict;
	for (;; ++item) {
		/* Check if the iterator is in-bounds. */
		if (item > Dict->rd_elem + Dict->rd_mask)
			return 0;
		if (item->di_key)
			break;
	}
	return 1;
}

PRIVATE DREF DeeObject *DCALL
rodictiterator_next_item(DictIterator *__restrict self) {
	struct rodict_item *item, *end;
	end = self->di_dict->rd_elem + self->di_dict->rd_mask + 1;
#ifndef CONFIG_NO_THREADS
	for (;;)
#endif /* !CONFIG_NO_THREADS */
	{
#ifdef CONFIG_NO_THREADS
		item = ATOMIC_READ(self->di_next);
#else /* CONFIG_NO_THREADS */
		struct rodict_item *old_item;
		old_item = item = ATOMIC_READ(self->di_next);
#endif /* !CONFIG_NO_THREADS */
		if (item >= end)
			goto iter_exhausted;
		while (item != end && !item->di_key)
			++item;
		if (item == end) {
#ifdef CONFIG_NO_THREADS
			self->di_next = item;
#else /* CONFIG_NO_THREADS */
			if (!ATOMIC_CMPXCH(self->di_next, old_item, item))
				continue;
#endif /* !CONFIG_NO_THREADS */
			goto iter_exhausted;
		}
#ifdef CONFIG_NO_THREADS
		self->di_next = item + 1;
#else /* CONFIG_NO_THREADS */
		if (ATOMIC_CMPXCH(self->di_next, old_item, item + 1))
			break;
#endif /* !CONFIG_NO_THREADS */
	}
	return DeeTuple_Pack(2, item->di_key, item->di_value);
iter_exhausted:
	return ITER_DONE;
}

PRIVATE DREF DeeObject *DCALL
rodictiterator_next_key(DictIterator *__restrict self) {
	struct rodict_item *item, *end;
	end = self->di_dict->rd_elem + self->di_dict->rd_mask + 1;
#ifndef CONFIG_NO_THREADS
	for (;;)
#endif /* !CONFIG_NO_THREADS */
	{
#ifdef CONFIG_NO_THREADS
		item = ATOMIC_READ(self->di_next);
#else /* CONFIG_NO_THREADS */
		struct rodict_item *old_item;
		old_item = item = ATOMIC_READ(self->di_next);
#endif /* !CONFIG_NO_THREADS */
		if (item >= end)
			goto iter_exhausted;
		while (item != end && !item->di_key)
			++item;
		if (item == end) {
#ifdef CONFIG_NO_THREADS
			self->di_next = item;
#else /* CONFIG_NO_THREADS */
			if (!ATOMIC_CMPXCH(self->di_next, old_item, item))
				continue;
#endif /* !CONFIG_NO_THREADS */
			goto iter_exhausted;
		}
#ifdef CONFIG_NO_THREADS
		self->di_next = item + 1;
#else /* CONFIG_NO_THREADS */
		if (ATOMIC_CMPXCH(self->di_next, old_item, item + 1))
			break;
#endif /* !CONFIG_NO_THREADS */
	}
	return_reference_(item->di_key);
iter_exhausted:
	return ITER_DONE;
}

PRIVATE DREF DeeObject *DCALL
rodictiterator_next_value(DictIterator *__restrict self) {
	struct rodict_item *item, *end;
	end = self->di_dict->rd_elem + self->di_dict->rd_mask + 1;
#ifndef CONFIG_NO_THREADS
	for (;;)
#endif /* !CONFIG_NO_THREADS */
	{
#ifdef CONFIG_NO_THREADS
		item = ATOMIC_READ(self->di_next);
#else /* CONFIG_NO_THREADS */
		struct rodict_item *old_item;
		old_item = item = ATOMIC_READ(self->di_next);
#endif /* !CONFIG_NO_THREADS */
		if (item >= end)
			goto iter_exhausted;
		while (item != end && !item->di_key)
			++item;
		if (item == end) {
#ifdef CONFIG_NO_THREADS
			self->di_next = item;
#else /* CONFIG_NO_THREADS */
			if (!ATOMIC_CMPXCH(self->di_next, old_item, item))
				continue;
#endif /* !CONFIG_NO_THREADS */
			goto iter_exhausted;
		}
#ifdef CONFIG_NO_THREADS
		self->di_next = item + 1;
#else /* CONFIG_NO_THREADS */
		if (ATOMIC_CMPXCH(self->di_next, old_item, item + 1))
			break;
#endif /* !CONFIG_NO_THREADS */
	}
	return_reference_(item->di_value);
iter_exhausted:
	return ITER_DONE;
}

INTDEF DeeTypeObject RoDictIterator_Type;
#define DEFINE_ITERATOR_COMPARE(name, op)                                   \
	PRIVATE DREF DeeObject *DCALL                                           \
	name(DictIterator *__restrict self,                                     \
	     DictIterator *__restrict other) {                                  \
		if (DeeObject_AssertType((DeeObject *)other, &RoDictIterator_Type)) \
			goto err;                                                       \
		return_bool(READ_ITEM(self) op READ_ITEM(other));                   \
	err:                                                                    \
		return NULL;                                                        \
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


PRIVATE struct type_member rodict_iterator_members[] = {
	TYPE_MEMBER_FIELD("seq", STRUCT_OBJECT, offsetof(DictIterator, di_dict)),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject RoDictIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoDictIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (int (DCALL *)(DeeObject *__restrict))&rodictiterator_ctor,
				/* .tp_copy_ctor = */ (int (DCALL *)(DeeObject *, DeeObject *))&rodictiterator_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (int (DCALL *)(size_t, DeeObject **__restrict))&rodictiterator_init,
				TYPE_FIXED_ALLOCATOR(DictIterator)
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






PUBLIC DREF DeeObject *DCALL
DeeRoDict_FromSequence(DeeObject *__restrict self) {
	DREF DeeObject *result;
	size_t length_hint;
	/* Optimization: Since rodicts are immutable, re-return if the
	 *               given sequence already is a read-only Dict. */
	if (DeeRoDict_CheckExact(self))
		return_reference_(self);
	/* TODO: if (DeeDict_CheckExact(self)) ... */
	/* Construct a read-only Dict from an iterator. */
	self = DeeObject_IterSelf(self);
	if unlikely(!self)
		return NULL;
	length_hint = DeeFastSeq_GetSize(self);
	result = (likely(length_hint != DEE_FASTSEQ_NOTFAST))
	         ? DeeRoDict_FromIteratorWithHint(self, length_hint)
	         : DeeRoDict_FromIterator(self);
	Dee_Decref(self);
	return result;
}

#define RODICT_ALLOC(mask)  ((DREF Dict *)DeeObject_Calloc(SIZEOF_RODICT(mask)))
#define SIZEOF_RODICT(mask) (offsetof(Dict, rd_elem) + (((mask) + 1) * sizeof(struct rodict_item)))
#define RODICT_INITIAL_MASK 0x03

PRIVATE DREF Dict *DCALL
rehash(DREF Dict *__restrict self, size_t old_mask, size_t new_mask) {
	DREF Dict *result;
	size_t i;
	result = RODICT_ALLOC(new_mask);
	if unlikely(!result)
		goto done;
	for (i = 0; i < old_mask; ++i) {
		size_t j, perturb;
		struct rodict_item *item;
		if (!self->rd_elem[i].di_key)
			continue;
		perturb = j = self->rd_elem[i].di_hash & new_mask;
		for (;; j = RODICT_HASHNX(j, perturb), RODICT_HASHPT(perturb)) {
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
insert(DREF Dict *__restrict self, size_t mask,
       size_t *__restrict pelemcount,
       /*inherit(always)*/ DREF DeeObject *__restrict key,
       /*inherit(always)*/ DREF DeeObject *__restrict value) {
	size_t i, perturb, hash;
	struct rodict_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & mask;
	for (;; i = RODICT_HASHNX(i, perturb), RODICT_HASHPT(perturb)) {
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
		--*pelemcount;
		Dee_Decref(item->di_key);
		Dee_Decref(item->di_value);
		break;
	}
	/* Fill in the item. */
	++*pelemcount;
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

PUBLIC DREF DeeObject *DCALL DeeRoDict_New(void) {
	DREF Dict *result;
	result = RODICT_ALLOC(RODICT_INITIAL_MASK);
	if unlikely(!result)
		goto done;
	result->rd_mask = RODICT_INITIAL_MASK;
	result->rd_size = 0;
	DeeObject_Init(result, &DeeRoDict_Type);
done:
	return (DREF DeeObject *)result;
}

PUBLIC DREF DeeObject *DCALL
DeeRoDict_NewWithHint(size_t num_items) {
	DREF Dict *result;
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
	return (DREF DeeObject *)result;
}

PUBLIC int DCALL
DeeRoDict_Insert(DREF DeeObject **__restrict pself,
                 DeeObject *__restrict key,
                 DeeObject *__restrict value) {
	Dict *me = (Dict *)*pself;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeRoDict_Type);
	ASSERT(!DeeObject_IsShared(me));
	if unlikely(me->rd_size * 2 > me->rd_mask) {
		size_t old_size = me->rd_size;
		size_t new_mask = (me->rd_mask << 1) | 1;
		me              = rehash(me, me->rd_mask, new_mask);
		if unlikely(!me)
			goto err;
		me->rd_mask = new_mask;
		me->rd_size = old_size; /* `rd_size' is not saved by `rehash()' */
	}
	/* Insert the new key/value-pair into the Dict. */
	Dee_Incref(key);
	Dee_Incref(value);
	if (insert(me, me->rd_mask, &me->rd_size, key, value))
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE DREF DeeObject *DCALL
DeeRoDict_FromIterator_impl(DeeObject *__restrict self, size_t mask) {
	DREF Dict *result, *new_result;
	DREF DeeObject *elem;
	size_t elem_count = 0;
	/* Construct a read-only Dict from an iterator. */
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
		/* Check if we must re-hash the resulting Dict. */
		if (elem_count * 2 > mask) {
			size_t new_mask = (mask << 1) | 1;
			new_result      = rehash(result, mask, new_mask);
			if unlikely(!new_result) {
				Dee_Decref(key_and_value[1]);
				Dee_Decref(key_and_value[0]);
				goto err_r;
			}
			mask = new_mask;
		}
		/* Insert the key-value pair into the resulting Dict. */
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

PUBLIC DREF DeeObject *DCALL
DeeRoDict_FromIteratorWithHint(DeeObject *__restrict self,
                               size_t num_items) {
	size_t mask = RODICT_INITIAL_MASK;
	while (mask <= num_items)
		mask = (mask << 1) | 1;
	mask = (mask << 1) | 1;
	return DeeRoDict_FromIterator_impl(self, mask);
}

PUBLIC DREF DeeObject *DCALL
DeeRoDict_FromIterator(DeeObject *__restrict self) {
	return DeeRoDict_FromIterator_impl(self, RODICT_INITIAL_MASK);
}


PRIVATE DREF DictIterator *DCALL
rodict_iter(Dict *__restrict self) {
	DREF DictIterator *result;
	result = DeeObject_MALLOC(DictIterator);
	if unlikely(!result)
		goto done;
	result->di_dict = self;
	result->di_next = self->rd_elem;
	Dee_Incref(self);
	DeeObject_Init(result, &RoDictIterator_Type);
done:
	return result;
}

PRIVATE DREF DeeObject *DCALL
rodict_size(Dict *__restrict self) {
	return DeeInt_NewSize(self->rd_size);
}

PRIVATE DREF DeeObject *DCALL
rodict_contains(Dict *__restrict self,
                DeeObject *__restrict key) {
	size_t i, perturb, hash;
	struct rodict_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & self->rd_mask;
	for (;; i = RODICT_HASHNX(i, perturb), RODICT_HASHPT(perturb)) {
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

PRIVATE DREF DeeObject *DCALL
rodict_getitem(Dict *__restrict self,
               DeeObject *__restrict key) {
	size_t i, perturb, hash;
	struct rodict_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & self->rd_mask;
	for (;; i = RODICT_HASHNX(i, perturb), RODICT_HASHPT(perturb)) {
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

INTERN DREF DeeObject *DCALL
DeeRoDict_GetItemDef(DeeObject *__restrict self,
                     DeeObject *__restrict key,
                     DeeObject *__restrict def) {
	size_t i, perturb, hash;
	struct rodict_item *item;
	Dict *me = (Dict *)self;
	hash     = DeeObject_Hash(key);
	perturb = i = hash & me->rd_mask;
	for (;; i = RODICT_HASHNX(i, perturb), RODICT_HASHPT(perturb)) {
		int error;
		item = &me->rd_elem[i & me->rd_mask];
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

INTERN DREF DeeObject *DCALL
DeeRoDict_GetItemString(DeeObject *__restrict self,
                        char const *__restrict key,
                        dhash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	Dict *me = (Dict *)self;
	perturb = i = hash & me->rd_mask;
	for (;; i = RODICT_HASHNX(i, perturb), RODICT_HASHPT(perturb)) {
		item = &me->rd_elem[i & me->rd_mask];
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		if (!DeeString_Check(item->di_key))
			continue;
		if (strcmp(key, DeeString_STR(item->di_key)) != 0)
			continue;
		/* Found it! */
		return_reference_(item->di_value);
	}
	err_unknown_key_str(self, key);
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeRoDict_GetItemStringLen(DeeObject *__restrict self,
                           char const *__restrict key,
                           size_t keylen,
                           dhash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	Dict *me = (Dict *)self;
	perturb = i = hash & me->rd_mask;
	for (;; i = RODICT_HASHNX(i, perturb), RODICT_HASHPT(perturb)) {
		item = &me->rd_elem[i & me->rd_mask];
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		if (!DeeString_Check(item->di_key))
			continue;
		if (DeeString_SIZE(item->di_key) != keylen)
			continue;
		if (memcmp(DeeString_STR(item->di_key), key, keylen * sizeof(char)) != 0)
			continue;
		/* Found it! */
		return_reference_(item->di_value);
	}
	err_unknown_key_str(self, key);
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeRoDict_GetItemStringDef(DeeObject *__restrict self,
                           char const *__restrict key,
                           dhash_t hash,
                           DeeObject *__restrict def) {
	size_t i, perturb;
	struct rodict_item *item;
	Dict *me = (Dict *)self;
	perturb = i = hash & me->rd_mask;
	for (;; i = RODICT_HASHNX(i, perturb), RODICT_HASHPT(perturb)) {
		item = &me->rd_elem[i & me->rd_mask];
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		if (!DeeString_Check(item->di_key))
			continue;
		if (strcmp(key, DeeString_STR(item->di_key)) != 0)
			continue;
		/* Found it! */
		return_reference_(item->di_value);
	}
	return_reference_(def);
}

INTERN DREF DeeObject *DCALL
DeeRoDict_GetItemStringLenDef(DeeObject *__restrict self,
                              char const *__restrict key,
                              size_t keylen,
                              dhash_t hash,
                              DeeObject *__restrict def) {
	size_t i, perturb;
	struct rodict_item *item;
	Dict *me = (Dict *)self;
	perturb = i = hash & me->rd_mask;
	for (;; i = RODICT_HASHNX(i, perturb), RODICT_HASHPT(perturb)) {
		item = &me->rd_elem[i & me->rd_mask];
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		if (!DeeString_Check(item->di_key))
			continue;
		if (DeeString_SIZE(item->di_key) != keylen)
			continue;
		if (memcmp(DeeString_STR(item->di_key), key, keylen * sizeof(char)) != 0)
			continue;
		/* Found it! */
		return_reference_(item->di_value);
	}
	return_reference_(def);
}

INTERN bool DCALL
DeeRoDict_HasItemString(DeeObject *__restrict self,
                        char const *__restrict key,
                        dhash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	Dict *me = (Dict *)self;
	perturb = i = hash & me->rd_mask;
	for (;; i = RODICT_HASHNX(i, perturb), RODICT_HASHPT(perturb)) {
		item = &me->rd_elem[i & me->rd_mask];
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		if (!DeeString_Check(item->di_key))
			continue;
		if (strcmp(key, DeeString_STR(item->di_key)) != 0)
			continue;
		/* Found it! */
		return true;
	}
	return false;
}

INTERN bool DCALL
DeeRoDict_HasItemStringLen(DeeObject *__restrict self,
                           char const *__restrict key,
                           size_t keylen,
                           dhash_t hash) {
	size_t i, perturb;
	struct rodict_item *item;
	Dict *me = (Dict *)self;
	perturb = i = hash & me->rd_mask;
	for (;; i = RODICT_HASHNX(i, perturb), RODICT_HASHPT(perturb)) {
		item = &me->rd_elem[i & me->rd_mask];
		if (!item->di_key)
			break;
		if (item->di_hash != hash)
			continue;
		if (!DeeString_Check(item->di_key))
			continue;
		if (DeeString_SIZE(item->di_key) != keylen)
			continue;
		if (memcmp(DeeString_STR(item->di_key), key, keylen * sizeof(char)) != 0)
			continue;
		/* Found it! */
		return true;
	}
	return false;
}


PRIVATE void DCALL
rodict_fini(Dict *__restrict self) {
	size_t i;
	for (i = 0; i <= self->rd_mask; ++i) {
		if (!self->rd_elem[i].di_key)
			continue;
		Dee_Decref(self->rd_elem[i].di_key);
		Dee_Decref(self->rd_elem[i].di_value);
	}
}

PRIVATE void DCALL
rodict_visit(Dict *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->rd_mask; ++i) {
		if (!self->rd_elem[i].di_key)
			continue;
		Dee_Visit(self->rd_elem[i].di_key);
		Dee_Visit(self->rd_elem[i].di_value);
	}
}

PRIVATE int DCALL
rodict_bool(Dict *__restrict self) {
	return self->rd_size != 0;
}

PRIVATE DREF DeeObject *DCALL
rodict_repr(Dict *__restrict self) {
	struct unicode_printer p = UNICODE_PRINTER_INIT;
	bool is_first            = true;
	size_t i;
	if (UNICODE_PRINTER_PRINT(&p, "{ ") < 0)
		goto err;
	for (i = 0; i <= self->rd_mask; ++i) {
		if (!self->rd_elem[i].di_key)
			continue;
		if unlikely(unicode_printer_printf(&p, "%s%r: %r",
		                                   is_first ? "" : ", ",
		                                   self->rd_elem[i].di_key,
		                                   self->rd_elem[i].di_value) < 0)
			goto err;
		is_first = false;
	}
	if unlikely((is_first ? unicode_printer_putascii(&p, '}')
	                      : UNICODE_PRINTER_PRINT(&p, " }")) < 0)
		goto err;
	return unicode_printer_pack(&p);
err:
	unicode_printer_fini(&p);
	return NULL;
}

PRIVATE size_t DCALL
rodict_nsi_getsize(Dict *__restrict self) {
	ASSERT(self->rd_size != (size_t)-2);
	return self->rd_size;
}


PRIVATE struct type_nsi rodict_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_MAP,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_maplike = */ {
			/* .nsi_getsize    = */ (void *)&rodict_nsi_getsize,
			/* .nsi_nextkey    = */ (void *)&rodictiterator_next_key,
			/* .nsi_nextvalue  = */ (void *)&rodictiterator_next_value,
			/* .nsi_getdefault = */ (void *)&DeeRoDict_GetItemDef
		}
	}
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

PRIVATE DREF DeeObject *DCALL
rodict_get(Dict *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	DeeObject *key, *def = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:get", &key, &def))
		goto err;
	return DeeRoDict_GetItemDef((DeeObject *)self, key, def);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
rodict_sizeof(Dict *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":__sizeof__"))
		goto err;
	return DeeInt_NewSize(offsetof(Dict, rd_elem) +
	                      ((self->rd_mask + 1) *
	                       sizeof(struct rodict_item)));
err:
	return NULL;
}

PRIVATE struct type_method rodict_methods[] = {
	{ DeeString_STR(&str_get),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **__restrict))&rodict_get,
	  DOC("(key,def=!N)\n"
	      "@return The value associated with @key or @def when @key has no value associated") },
	{ "__sizeof__",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **__restrict))&rodict_sizeof,
	  DOC("->?Dint") },
	{ NULL }
};

PRIVATE struct type_getset rodict_getsets[] = {
	{ "frozen", &DeeObject_NewRef, NULL, NULL, DOC("->?.") },
	{ NULL }
};

PRIVATE struct type_member rodict_members[] = {
	TYPE_MEMBER_FIELD("__mask__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(Dict, rd_mask)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(Dict, rd_size)),
	TYPE_MEMBER_END
};


PRIVATE struct type_member rodict_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &RoDictIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &DeeRoDict_Type),
	TYPE_MEMBER_END
};

PRIVATE DREF Dict *DCALL rodict_ctor(void) {
	DREF Dict *result;
	result = RODICT_ALLOC(1);
	if unlikely(!result)
		goto done;
	result->rd_mask = 1;
	result->rd_size = 0;
	DeeObject_Init(result, &DeeRoDict_Type);
done:
	return result;
}

PRIVATE DREF Dict *DCALL
rodict_deepcopy(Dict *__restrict self) {
	DREF Dict *result;
	size_t i;
	int temp;
	result = (DREF Dict *)DeeRoDict_NewWithHint(self->rd_size);
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
		/* Insert the copied key & value into the new Dict. */
		temp = DeeRoDict_Insert((DREF DeeObject **)&result, key_copy, value_copy);
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

PRIVATE DREF Dict *DCALL
rodict_init(size_t argc, DeeObject **__restrict argv) {
	DeeObject *seq;
	if (DeeArg_Unpack(argc, argv, "o:_RoDict", &seq))
		return NULL;
	return (DREF Dict *)DeeRoDict_FromSequence(seq);
}




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
				/* .tp_ctor      = */ (void *)&rodict_ctor,
				/* .tp_copy_ctor = */ (void *)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (void *)&rodict_deepcopy,
				/* .tp_any_ctor  = */ (void *)&rodict_init,
				/* .tp_free      = */ NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rodict_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rodict_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rodict_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rodict_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
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
