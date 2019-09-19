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
#ifndef GUARD_DEEMON_OBJECTS_ROSET_C
#define GUARD_DEEMON_OBJECTS_ROSET_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/roset.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/thread.h>

#include <hybrid/atomic.h>

#include "../runtime/runtime_error.h"

DECL_BEGIN

typedef DeeRoSetObject Set;

typedef struct {
	OBJECT_HEAD
	Set               *si_set;  /* [1..1][const] The set being iterated. */
	struct roset_item *si_next; /* [?..1][in(si_set->rs_elem)][atomic]
	                             * The first candidate for the next item. */
} SetIterator;

INTDEF DeeTypeObject RoSetIterator_Type;

#ifdef CONFIG_NO_THREADS
#define READ_ITEM(x)            ((x)->si_next)
#else /* CONFIG_NO_THREADS */
#define READ_ITEM(x) ATOMIC_READ((x)->si_next)
#endif /* !CONFIG_NO_THREADS */

INTERN int DCALL
rosetiterator_ctor(SetIterator *__restrict self) {
	self->si_set = (Set *)DeeRoSet_New();
	if unlikely(!self->si_set)
		return -1;
	self->si_next = self->si_set->rs_elem;
	return 0;
}

INTERN int DCALL
rosetiterator_init(SetIterator *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
	Set *set;
	if (DeeArg_Unpack(argc, argv, "o:_RoSetIterator", &set) ||
	    DeeObject_AssertTypeExact((DeeObject *)set, &DeeRoSet_Type))
		return -1;
	self->si_set = set;
	Dee_Incref(set);
	self->si_next = set->rs_elem;
	return 0;
}

INTERN int DCALL
rosetiterator_copy(SetIterator *__restrict self,
                   SetIterator *__restrict other) {
	self->si_set = other->si_set;
	Dee_Incref(self->si_set);
	self->si_next = READ_ITEM(other);
	return 0;
}

PRIVATE void DCALL
rosetiterator_fini(SetIterator *__restrict self) {
	Dee_Decref(self->si_set);
}

PRIVATE void DCALL
rosetiterator_visit(SetIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->si_set);
}

PRIVATE int DCALL
rosetiterator_bool(SetIterator *__restrict self) {
	struct roset_item *item = READ_ITEM(self);
	Set *set                = self->si_set;
	for (;; ++item) {
		/* Check if the iterator is in-bounds. */
		if (item > set->rs_elem + set->rs_mask)
			return 0;
		if (item->si_key)
			break;
	}
	return 1;
}

PRIVATE DREF DeeObject *DCALL
rosetiterator_next(SetIterator *__restrict self) {
	struct roset_item *item, *end;
	end = self->si_set->rs_elem + self->si_set->rs_mask + 1;
#ifndef CONFIG_NO_THREADS
	for (;;)
#endif /* !CONFIG_NO_THREADS */
	{
#ifdef CONFIG_NO_THREADS
		item = ATOMIC_READ(self->si_next);
#else /* CONFIG_NO_THREADS */
		struct roset_item *old_item;
		old_item = item = ATOMIC_READ(self->si_next);
#endif /* !CONFIG_NO_THREADS */
		if (item >= end)
			goto iter_exhausted;
		while (item != end && !item->si_key)
			++item;
		if (item == end) {
#ifdef CONFIG_NO_THREADS
			self->si_next = item;
#else /* CONFIG_NO_THREADS */
			if (!ATOMIC_CMPXCH(self->si_next, old_item, item))
				continue;
#endif /* !CONFIG_NO_THREADS */
			goto iter_exhausted;
		}
#ifdef CONFIG_NO_THREADS
		self->si_next = item + 1;
#else /* CONFIG_NO_THREADS */
		if (ATOMIC_CMPXCH(self->si_next, old_item, item + 1))
			break;
#endif /* !CONFIG_NO_THREADS */
	}
	return_reference_(item->si_key);
iter_exhausted:
	return ITER_DONE;
}

INTDEF DeeTypeObject RoSetIterator_Type;
#define DEFINE_ITERATOR_COMPARE(name, op)                                  \
	PRIVATE DREF DeeObject *DCALL                                          \
	name(SetIterator *__restrict self,                                     \
	     SetIterator *__restrict other) {                                  \
		if (DeeObject_AssertType((DeeObject *)other, &RoSetIterator_Type)) \
			goto err;                                                      \
		return_bool(READ_ITEM(self) op READ_ITEM(other));                  \
	err:                                                                   \
		return NULL;                                                       \
	}
DEFINE_ITERATOR_COMPARE(rosetiterator_eq, ==)
DEFINE_ITERATOR_COMPARE(rosetiterator_ne, !=)
DEFINE_ITERATOR_COMPARE(rosetiterator_lo, <)
DEFINE_ITERATOR_COMPARE(rosetiterator_le, <=)
DEFINE_ITERATOR_COMPARE(rosetiterator_gr, >)
DEFINE_ITERATOR_COMPARE(rosetiterator_ge, >=)
#undef DEFINE_ITERATOR_COMPARE

PRIVATE struct type_cmp rosetiterator_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rosetiterator_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rosetiterator_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rosetiterator_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rosetiterator_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rosetiterator_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rosetiterator_ge
};


PRIVATE struct type_member roset_iterator_members[] = {
	TYPE_MEMBER_FIELD("seq", STRUCT_OBJECT, offsetof(SetIterator, si_set)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject RoSetIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoSetIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (int (DCALL *)(DeeObject *__restrict))&rosetiterator_ctor,
				/* .tp_copy_ctor = */ (int (DCALL *)(DeeObject *, DeeObject *))&rosetiterator_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ (int (DCALL *)(size_t, DeeObject **__restrict))&rosetiterator_init,
				TYPE_FIXED_ALLOCATOR(SetIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rosetiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rosetiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rosetiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &rosetiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rosetiterator_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ roset_iterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};





PUBLIC DREF DeeObject *DCALL
DeeRoSet_FromSequence(DeeObject *__restrict self) {
	DREF DeeObject *result;
	size_t length_hint;
	/* Optimization: Since rosets are immutable, re-return if the
	 *               given sequence already is a read-only set. */
	if (DeeRoSet_CheckExact(self))
		return_reference_(self);
	/* TODO: if (DeeHashSet_CheckExact(self)) ... */
	/* Construct a read-only set from an iterator. */
	self = DeeObject_IterSelf(self);
	if unlikely(!self)
		goto err;
	/* TODO: Use the fast-sequence interface directly! */
	length_hint = DeeFastSeq_GetSize(self);
	result = (likely(length_hint != DEE_FASTSEQ_NOTFAST))
	         ? DeeRoSet_FromIteratorWithHint(self, length_hint)
	         : DeeRoSet_FromIterator(self);
	Dee_Decref(self);
	return result;
err:
	return NULL;
}

#define ROSET_ALLOC(mask)  ((DREF Set *)DeeObject_Calloc(SIZEOF_ROSET(mask)))
#define SIZEOF_ROSET(mask) (offsetof(Set, rs_elem) + (((mask) + 1) * sizeof(struct roset_item)))
#define ROSET_INITIAL_MASK 0x03

PRIVATE DREF Set *DCALL
rehash(DREF Set *__restrict self, size_t old_mask, size_t new_mask) {
	DREF Set *result;
	size_t i;
	result = ROSET_ALLOC(new_mask);
	if unlikely(!result)
		goto done;
	for (i = 0; i <= old_mask; ++i) {
		size_t j, perturb;
		struct roset_item *item;
		if (!self->rs_elem[i].si_key)
			continue;
		perturb = j = self->rs_elem[i].si_hash & new_mask;
		for (;; j = ROSET_HASHNX(j, perturb), ROSET_HASHPT(perturb)) {
			item = &result->rs_elem[j & new_mask];
			if (!item->si_key)
				break;
		}
		/* Copy the old item into the new slot. */
		memcpy(item, &self->rs_elem[i], sizeof(struct roset_item));
	}
	DeeObject_Free(self);
done:
	return result;
}

/* NOTE: _Always_ inherits references to `key' */
PRIVATE int DCALL
insert(DREF Set *__restrict self, size_t mask,
       DREF DeeObject *__restrict key) {
	size_t i, perturb, hash;
	struct roset_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & mask;
	for (;; i = ROSET_HASHNX(i, perturb), ROSET_HASHPT(perturb)) {
		int error;
		item = &self->rs_elem[i & mask];
		if (!item->si_key)
			break;
		if (item->si_hash != hash)
			continue;
		/* Same hash. -> Check if it's also the same key. */
		error = DeeObject_CompareEq(key, item->si_key);
		if unlikely(error < 0)
			goto err;
		if (error)
			return 1; /* It _is_ the same key! */
	}
	/* Fill in the item. */
	item->si_hash = hash;
	item->si_key  = key; /* Inherit reference. */
	return 0;
err:
	/* Always inherit references (even upon error) */
	Dee_Decref(key);
	return -1;
}

PUBLIC DREF DeeObject *DCALL DeeRoSet_New(void) {
	DREF Set *result;
	result = ROSET_ALLOC(ROSET_INITIAL_MASK);
	if unlikely(!result)
		goto done;
	result->rs_mask = ROSET_INITIAL_MASK;
	result->rs_size = 0;
	DeeObject_Init(result, &DeeRoSet_Type);
done:
	return (DREF DeeObject *)result;
}

PUBLIC DREF DeeObject *DCALL
DeeRoSet_NewWithHint(size_t num_items) {
	DREF Set *result;
	size_t mask = ROSET_INITIAL_MASK;
	while (mask <= num_items)
		mask = (mask << 1) | 1;
	mask   = (mask << 1) | 1;
	result = ROSET_ALLOC(mask);
	if unlikely(!result)
		goto done;
	result->rs_mask = mask;
	result->rs_size = 0;
	DeeObject_Init(result, &DeeRoSet_Type);
done:
	return (DREF DeeObject *)result;
}

PUBLIC int DCALL
DeeRoSet_Insert(DREF DeeObject **__restrict pself,
                DeeObject *__restrict key) {
	int error;
	Set *me = (Set *)*pself;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeRoSet_Type);
	ASSERT(!DeeObject_IsShared(me));
	if unlikely(me->rs_size * 2 > me->rs_mask) {
		size_t old_size = me->rs_size;
		size_t new_mask = (me->rs_mask << 1) | 1;
		me              = rehash(me, me->rs_mask, new_mask);
		if unlikely(!me)
			goto err;
		me->rs_mask = new_mask;
		me->rs_size = old_size; /* `rs_size' is not saved by `rehash()' */
	}
	/* Insert the new key/value-pair into the set. */
	Dee_Incref(key);
	error = insert(me, me->rs_mask, key);
	if (error != 0) {
		if unlikely(error < 0)
			goto err;
	} else {
		++me->rs_size; /* Keep track of the number of inserted items. */
	}
	return error;
err:
	return -1;
}

PRIVATE DREF DeeObject *DCALL
DeeRoSet_FromIterator_impl(DeeObject *__restrict self, size_t mask) {
	DREF Set *result, *new_result;
	DREF DeeObject *elem;
	size_t elem_count = 0;
	int error;
	/* Construct a read-only set from an iterator. */
	result = ROSET_ALLOC(mask);
	if unlikely(!result)
		goto done;
	while (ITER_ISOK(elem = DeeObject_IterNext(self))) {
		/* Check if we must re-hash the resulting set. */
		if (elem_count * 2 > mask) {
			size_t new_mask = (mask << 1) | 1;
			new_result      = rehash(result, mask, new_mask);
			if unlikely(!new_result) {
				Dee_Decref(elem);
				goto err_r;
			}
			mask   = new_mask;
			result = new_result;
		}
		/* Insert the key-value pair into the resulting set. */
		error = insert(result, mask, elem);
		if unlikely(error != 0) {
			if unlikely(error < 0)
				goto err_r;
		} else {
			++elem_count;
		}
		if (DeeThread_CheckInterrupt())
			goto err_r;
	}
	if unlikely(!elem)
		goto err_r;
	/* Fill in control members and setup the resulting object. */
	result->rs_size = elem_count;
	result->rs_mask = mask;
	DeeObject_Init(result, &DeeRoSet_Type);
done:
	return (DREF DeeObject *)result;
err_r:
	for (elem_count = 0; elem_count <= mask; ++elem_count) {
		if (!result->rs_elem[elem_count].si_key)
			continue;
		Dee_Decref(result->rs_elem[elem_count].si_key);
	}
	DeeObject_Free(result);
	return NULL;
}

PUBLIC DREF DeeObject *DCALL
DeeRoSet_FromIteratorWithHint(DeeObject *__restrict self,
                              size_t num_items) {
	size_t mask = ROSET_INITIAL_MASK;
	while (mask <= num_items)
		mask = (mask << 1) | 1;
	mask = (mask << 1) | 1;
	return DeeRoSet_FromIterator_impl(self, mask);
}

PUBLIC DREF DeeObject *DCALL
DeeRoSet_FromIterator(DeeObject *__restrict self) {
	return DeeRoSet_FromIterator_impl(self, ROSET_INITIAL_MASK);
}


PRIVATE DREF SetIterator *DCALL
roset_iter(Set *__restrict self) {
	DREF SetIterator *result;
	result = DeeObject_MALLOC(SetIterator);
	if unlikely(!result)
		goto done;
	result->si_set  = self;
	result->si_next = self->rs_elem;
	Dee_Incref(self);
	DeeObject_Init(result, &RoSetIterator_Type);
done:
	return result;
}

PRIVATE DREF DeeObject *DCALL
roset_size(Set *__restrict self) {
	return DeeInt_NewSize(self->rs_size);
}

PRIVATE DREF DeeObject *DCALL
roset_contains(Set *__restrict self,
               DeeObject *__restrict key) {
	size_t i, perturb, hash;
	struct roset_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & self->rs_mask;
	for (;; i = ROSET_HASHNX(i, perturb), ROSET_HASHPT(perturb)) {
		int error;
		item = &self->rs_elem[i & self->rs_mask];
		if (!item->si_key)
			break;
		if (item->si_hash != hash)
			continue;
		error = DeeObject_CompareEq(key, item->si_key);
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

PUBLIC int DCALL
DeeRoSet_Contains(DeeObject *__restrict self,
                  DeeObject *__restrict key) {
	size_t i, perturb, hash;
	struct roset_item *item;
	Set *me = (Set *)self;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & me->rs_mask;
	for (;; i = ROSET_HASHNX(i, perturb), ROSET_HASHPT(perturb)) {
		int error;
		item = &me->rs_elem[i & me->rs_mask];
		if (!item->si_key)
			break;
		if (item->si_hash != hash)
			continue;
		error = DeeObject_CompareEq(key, item->si_key);
		if unlikely(error < 0)
			goto err;
		if (!error)
			continue; /* Non-equal keys. */
		/* Found it! */
		return 1;
	}
	return 0;
err:
	return -1;
}

PUBLIC bool DCALL
DeeRoSet_ContainsString(DeeObject *__restrict self,
                        char const *__restrict key,
                        size_t key_length) {
	size_t i, perturb, hash;
	struct roset_item *item;
	Set *me = (Set *)self;
	hash    = Dee_HashPtr(key, key_length);
	perturb = i = hash & me->rs_mask;
	for (;; i = ROSET_HASHNX(i, perturb), ROSET_HASHPT(perturb)) {
		item = &me->rs_elem[i & me->rs_mask];
		if (!item->si_key)
			break;
		if (item->si_hash != hash)
			continue;
		if (!DeeString_Check(item->si_key))
			continue;
		if (DeeString_SIZE(item->si_key) != key_length)
			continue;
		if (memcmp(key, DeeString_STR(item->si_key), key_length) != 0)
			continue;
		/* Found it! */
		return true;
	}
	return false;
}


PRIVATE void DCALL
roset_fini(Set *__restrict self) {
	size_t i;
	for (i = 0; i <= self->rs_mask; ++i) {
		if (!self->rs_elem[i].si_key)
			continue;
		Dee_Decref(self->rs_elem[i].si_key);
	}
}

PRIVATE void DCALL
roset_visit(Set *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->rs_mask; ++i) {
		if (!self->rs_elem[i].si_key)
			continue;
		Dee_Visit(self->rs_elem[i].si_key);
	}
}

PRIVATE int DCALL
roset_bool(Set *__restrict self) {
	return self->rs_size != 0;
}

PRIVATE DREF DeeObject *DCALL
roset_repr(Set *__restrict self) {
	struct unicode_printer p = UNICODE_PRINTER_INIT;
	bool is_first            = true;
	size_t i;
	if (UNICODE_PRINTER_PRINT(&p, "{ ") < 0)
		goto err;
	for (i = 0; i <= self->rs_mask; ++i) {
		if (!self->rs_elem[i].si_key)
			continue;
		if unlikely(unicode_printer_printf(&p, "%s%r",
			                                is_first ? "" : ", ",
			                                self->rs_elem[i].si_key) < 0)
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

PRIVATE struct type_seq roset_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&roset_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&roset_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&roset_contains
};

PRIVATE DREF DeeObject *DCALL
roset_sizeof(Set *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":__sizeof__"))
		goto err;
	return DeeInt_NewSize(offsetof(Set, rs_elem) +
	                      ((self->rs_mask + 1) *
	                       sizeof(struct roset_item)));
err:
	return NULL;
}

PRIVATE struct type_method roset_methods[] = {
	{ "__sizeof__",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **__restrict))&roset_sizeof,
	  DOC("->?Dint") },
	{ NULL }
};

PRIVATE struct type_getset roset_getsets[] = {
	{ "frozen", &DeeObject_NewRef, NULL, NULL, DOC("->?.") },
	{ NULL }
};

PRIVATE struct type_member roset_members[] = {
	TYPE_MEMBER_FIELD("__mask__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(Set, rs_mask)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(Set, rs_size)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member roset_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &RoSetIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &DeeRoSet_Type),
	TYPE_MEMBER_END
};

PRIVATE DREF Set *DCALL roset_ctor(void) {
	DREF Set *result;
	result = ROSET_ALLOC(1);
	if unlikely(!result)
		goto done;
	result->rs_mask = 1;
	result->rs_size = 0;
	DeeObject_Init(result, &DeeRoSet_Type);
done:
	return result;
}

PRIVATE DREF Set *DCALL
roset_deepcopy(Set *__restrict self) {
	DREF Set *result;
	size_t i;
	int temp;
	result = (DREF Set *)DeeRoSet_NewWithHint(self->rs_size);
	if unlikely(!result)
		goto done;
	for (i = 0; i <= self->rs_mask; ++i) {
		DREF DeeObject *key_copy;
		/* Deep-copy the key & value */
		if (!self->rs_elem[i].si_key)
			continue;
		key_copy = DeeObject_DeepCopy(self->rs_elem[i].si_key);
		if unlikely(!key_copy)
			goto err;
		/* Insert the copied key & value into the new set. */
		temp = DeeRoSet_Insert((DREF DeeObject **)&result, key_copy);
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

PRIVATE DREF Set *DCALL
roset_init(size_t argc, DeeObject **__restrict argv) {
	DeeObject *seq;
	if (DeeArg_Unpack(argc, argv, "o:_RoSet", &seq))
		return NULL;
	return (DREF Set *)DeeRoSet_FromSequence(seq);
}



PUBLIC DeeTypeObject DeeRoSet_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoSet",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (DREF DeeObject *(DCALL *)(void))&roset_ctor,
				/* .tp_copy_ctor = */ &DeeObject_NewRef,
				/* .tp_deep_ctor = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&roset_deepcopy,
				/* .tp_any_ctor  = */ (DREF DeeObject *(DCALL *)(size_t, DeeObject **__restrict))&roset_init,
				/* .tp_free      = */ NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&roset_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&roset_repr,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&roset_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&roset_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &roset_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ roset_methods,
	/* .tp_getsets       = */ roset_getsets,
	/* .tp_members       = */ roset_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ roset_class_members
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ROSET_C */
