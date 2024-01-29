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

PRIVATE WUNUSED NONNULL((1)) int DCALL
rodictiterator_ctor(RoDictIterator *__restrict self) {
	self->rodi_dict = DeeRoDict_New();
	if unlikely(!self->rodi_dict)
		goto err;
	self->rodi_next = self->rodi_dict->rd_elem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
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
		if (item->rdi_key)
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
		while (item < end && !item->rdi_key)
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item + 1))
			break;
	}
	return DeeTuple_Pack(2, item->rdi_key, item->rdi_value);
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
		while (item < end && !item->rdi_key)
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item + 1))
			break;
	}
	return_reference_(item->rdi_key);
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
		while (item < end && !item->rdi_key)
			++item;
		if (item == end) {
			if (!atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item))
				continue;
			goto iter_exhausted;
		}
		if (atomic_cmpxch_weak_or_write(&self->rodi_next, old_item, item + 1))
			break;
	}
	return_reference_(item->rdi_value);
iter_exhausted:
	return ITER_DONE;
}

INTDEF DeeTypeObject RoDictIterator_Type;
#define DEFINE_RODICTITERATOR_COMPARE(name, op)                \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL      \
	name(RoDictIterator *self, RoDictIterator *other) {        \
		if (DeeObject_AssertType(other, &RoDictIterator_Type)) \
			goto err;                                          \
		return_bool(READ_ITEM(self) op READ_ITEM(other));      \
	err:                                                       \
		return NULL;                                           \
	}
DEFINE_RODICTITERATOR_COMPARE(rodictiterator_eq, ==)
DEFINE_RODICTITERATOR_COMPARE(rodictiterator_ne, !=)
DEFINE_RODICTITERATOR_COMPARE(rodictiterator_lo, <)
DEFINE_RODICTITERATOR_COMPARE(rodictiterator_le, <=)
DEFINE_RODICTITERATOR_COMPARE(rodictiterator_gr, >)
DEFINE_RODICTITERATOR_COMPARE(rodictiterator_ge, >=)
#undef DEFINE_RODICTITERATOR_COMPARE

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






#define RODICT_ALLOC(mask)  ((DREF RoDict *)DeeObject_Calloc(SIZEOF_RODICT(mask)))
#define SIZEOF_RODICT(mask) (offsetof(RoDict, rd_elem) + (((mask) + 1) * sizeof(struct rodict_item)))
#define RODICT_INITIAL_MASK 0x03

PRIVATE WUNUSED NONNULL((1)) DREF RoDict *DCALL
DeeRoDict_Rehash(/*inherit(on_success)*/ DREF RoDict *__restrict self, size_t new_mask) {
	DREF RoDict *result;
	size_t i;
	result = RODICT_ALLOC(new_mask);
	if unlikely(!result)
		goto done;
	ASSERT(self->ob_refcnt == 1);
	ASSERT(self->ob_type == &DeeRoDict_Type);
	result->ob_refcnt = 1;
	result->ob_type = &DeeRoDict_Type;
	result->rd_size = self->rd_size;
	result->rd_mask = new_mask;
	for (i = 0; i <= self->rd_mask; ++i) {
		size_t j, perturb;
		struct rodict_item *item;
		if (!self->rd_elem[i].rdi_key)
			continue;
		perturb = j = self->rd_elem[i].rdi_hash & new_mask;
		for (;; RODICT_HASHNX(j, perturb)) {
			item = &result->rd_elem[j & new_mask];
			if (!item->rdi_key)
				break;
		}
		/* Copy the old item into the new slot. */
		memcpy(item, &self->rd_elem[i], sizeof(struct rodict_item));
	}
	DeeObject_Free(self);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeRoDict_DoInsert(DREF RoDict *__restrict self,
                   DeeObject *key, DeeObject *value) {
	size_t i, perturb, hash;
	struct rodict_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & self->rd_mask;
	for (;; RODICT_HASHNX(i, perturb)) {
		int error;
		item = &self->rd_elem[i & self->rd_mask];
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;

		/* Same hash. -> Check if it's also the same key. */
		error = DeeObject_CompareEq(key, item->rdi_key);
		if unlikely(error < 0)
			goto err;
		if (!error)
			continue; /* Not the same key. */

		/* It _is_ the same key! (override it...) */
		--self->rd_size;
		Dee_Decref(item->rdi_key);
		Dee_Decref(item->rdi_value);
		break;
	}

	/* Fill in the item. */
	++self->rd_size;
	item->rdi_hash  = hash;
	item->rdi_key   = key;
	item->rdi_value = value;
	Dee_Incref(key);
	Dee_Incref(value);
	return 0;
err:
	return -1;
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
		size_t new_mask = (me->rd_mask << 1) | 1;
		me = DeeRoDict_Rehash(me, new_mask);
		if unlikely(!me)
			goto err;
		*p_self = me;
	}

	/* Insert the new key/value-pair into the RoDict. */
	return DeeRoDict_DoInsert(me, key, value);
err:
	return -1;
}


#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define DeeRoDict_InsertSequence_foreach (*(Dee_foreach_pair_t)&DeeRoDict_Insert)
#else /* __SIZEOF_SIZE_T__ == __SIZEOF_INT__ */
PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
DeeRoDict_InsertSequence_foreach(void *arg, DeeObject *key, DeeObject *value) {
	return DeeRoDict_Insert((RoDict **)arg, key, value);
}
#endif /* __SIZEOF_SIZE_T__ != __SIZEOF_INT__ */

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_FromSequence(DeeObject *__restrict sequence) {
	DREF RoDict *result;
	size_t length_hint, mask;

	/* Optimization: Since rodicts are immutable, re-return if the
	 *               given sequence already is a read-only RoDict. */
	if (DeeRoDict_CheckExact(sequence))
		return_reference_(sequence);

	/* TODO: Optimization: if (DeeDict_CheckExact(sequence)) ...
	 * (fix the dict's hash-vector to not contain dummies,
	 * then copy as-is) */

	/* Construct a read-only RoDict from a generic sequence. */
	mask        = RODICT_INITIAL_MASK;
	length_hint = DeeFastSeq_GetSize(sequence);
	if (length_hint != DEE_FASTSEQ_NOTFAST) {
		while (mask <= length_hint)
			mask = (mask << 1) | 1;
		mask = (mask << 1) | 1;
	}
	result = RODICT_ALLOC(mask);
	if likely(result) {
		/*result->rd_size = 0;*/
		result->rd_mask = mask;
		DeeObject_Init(result, &DeeRoDict_Type);
		if unlikely(DeeObject_ForeachPair(sequence, &DeeRoDict_InsertSequence_foreach, &result))
			goto err_r;
	}
	return (DREF DeeObject *)result;
err_r:
	Dee_DecrefDokill(result);
/*err:*/
	return NULL;
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
rodict_contains(RoDict *self, DeeObject *key) {
	size_t i, perturb, hash;
	struct rodict_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & self->rd_mask;
	for (;; RODICT_HASHNX(i, perturb)) {
		int error;
		item = &self->rd_elem[i & self->rd_mask];
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		error = DeeObject_CompareEq(key, item->rdi_key);
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
rodict_getitem(RoDict *self, DeeObject *key) {
	size_t i, perturb, hash;
	struct rodict_item *item;
	hash    = DeeObject_Hash(key);
	perturb = i = hash & self->rd_mask;
	for (;; RODICT_HASHNX(i, perturb)) {
		int error;
		item = &self->rd_elem[i & self->rd_mask];
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		error = DeeObject_CompareEq(key, item->rdi_key);
		if unlikely(error < 0)
			goto err;
		if (!error)
			continue; /* Non-equal keys. */
		/* Found it! */
		return_reference_(item->rdi_value);
	}
	err_unknown_key((DeeObject *)self, key);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeRoDict_GetItemDef(DeeObject *self, DeeObject *key, DeeObject *def) {
	RoDict *me = (RoDict *)self;
	size_t i, perturb, hash;
	struct rodict_item *item;
	hash     = DeeObject_Hash(key);
	perturb = i = RODICT_HASHST(me, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		int error;
		item = RODICT_HASHIT(me, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		error = DeeObject_CompareEq(key, item->rdi_key);
		if unlikely(error < 0)
			goto err;
		if (!error)
			continue; /* Non-equal keys. */
		/* Found it! */
		return_reference_(item->rdi_value);
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
	RoDict *me = (RoDict *)self;
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = RODICT_HASHST(me, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		item = RODICT_HASHIT(me, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (strcmp(DeeString_STR(item->rdi_key), key) == 0)
			return_reference_(item->rdi_value); /* Found it! */
	}
	err_unknown_key_str(self, key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeRoDict_GetItemStringLen(DeeObject *__restrict self,
                           char const *__restrict key,
                           size_t keylen,
                           dhash_t hash) {
	RoDict *me = (RoDict *)self;
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = RODICT_HASHST(me, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		item = RODICT_HASHIT(me, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (DeeString_EqualsBuf(item->rdi_key, key, keylen))
			return_reference_(item->rdi_value); /* Found it! */
	}
	err_unknown_key_str(self, key);
	return NULL;
}

INTERN WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
DeeRoDict_GetItemStringDef(DeeObject *self,
                           char const *__restrict key,
                           dhash_t hash,
                           DeeObject *def) {
	RoDict *me = (RoDict *)self;
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = RODICT_HASHST(me, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		item = RODICT_HASHIT(me, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (strcmp(DeeString_STR(item->rdi_key), key) == 0)
			return_reference_(item->rdi_value); /* Found it! */
	}
	return_reference_(def);
}

INTERN WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeRoDict_GetItemStringLenDef(DeeObject *self,
                              char const *__restrict key,
                              size_t keylen,
                              dhash_t hash,
                              DeeObject *def) {
	RoDict *me = (RoDict *)self;
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = RODICT_HASHST(me, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		item = RODICT_HASHIT(me, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (DeeString_EqualsBuf(item->rdi_key, key, keylen))
			return_reference_(item->rdi_value); /* Found it! */
	}
	return_reference_(def);
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeRoDict_HasItemString(DeeObject *__restrict self,
                        char const *__restrict key,
                        dhash_t hash) {
	RoDict *me = (RoDict *)self;
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = RODICT_HASHST(me, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		item = RODICT_HASHIT(me, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (strcmp(DeeString_STR(item->rdi_key), key) == 0)
			return true; /* Found it! */
	}
	return false;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeRoDict_HasItemStringLen(DeeObject *__restrict self,
                           char const *__restrict key,
                           size_t keylen,
                           dhash_t hash) {
	RoDict *me = (RoDict *)self;
	size_t i, perturb;
	struct rodict_item *item;
	perturb = i = RODICT_HASHST(me, hash);
	for (;; RODICT_HASHNX(i, perturb)) {
		item = RODICT_HASHIT(me, i);
		if (!item->rdi_key)
			break;
		if (item->rdi_hash != hash)
			continue;
		if (!DeeString_Check(item->rdi_key))
			continue;
		if (DeeString_EqualsBuf(item->rdi_key, key, keylen))
			return true; /* Found it! */
	}
	return false;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRoDict_ByHash(DeeObject *__restrict self, Dee_hash_t hash) {
	RoDict *me = (RoDict *)self;
	DREF DeeObject *result;
	DREF DeeTupleObject *match;
	dhash_t i, perturb;
	match = NULL;
	perturb = i = DeeRoDict_HashSt(me, hash);
	for (;; DeeRoDict_HashNx(i, perturb)) {
		struct rodict_item *item;
		item = DeeRoDict_HashIt(me, i);
		if (!item->rdi_key)
			break; /* Not found */
		if (item->rdi_hash != hash)
			continue; /* Non-matching hash */
		if unlikely(match) {
			/* There are multiple matches for `hash'. */
			Dee_Decref(match);
			/* XXX: RoDict-specific optimizations? */
			return DeeMap_HashFilter((DeeObject *)me, hash);
		}
		match = DeeTuple_NewUninitialized(2);
		if unlikely(!match)
			goto err;
		DeeTuple_SET(match, 0, item->rdi_key);
		DeeTuple_SET(match, 1, item->rdi_value);
		Dee_Incref(item->rdi_key);
		Dee_Incref(item->rdi_value);
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
		if (!self->rd_elem[i].rdi_key)
			continue;
		Dee_Decref(self->rd_elem[i].rdi_key);
		Dee_Decref(self->rd_elem[i].rdi_value);
	}
}

PRIVATE NONNULL((1, 2)) void DCALL
rodict_visit(RoDict *__restrict self, dvisit_t proc, void *arg) {
	size_t i;
	for (i = 0; i <= self->rd_mask; ++i) {
		if (!self->rd_elem[i].rdi_key)
			continue;
		Dee_Visit(self->rd_elem[i].rdi_key);
		Dee_Visit(self->rd_elem[i].rdi_value);
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
	result = DeeFormat_PRINT(printer, arg, "Dict.Frozen({ ");
	if unlikely(result < 0)
		goto done;
	for (i = 0; i <= self->rd_mask; ++i) {
		if (!self->rd_elem[i].rdi_key)
			continue;
		if (!is_first)
			DO(err, DeeFormat_PRINT(printer, arg, ", "));
		DO(err, DeeFormat_Printf(printer, arg, "%r: %r",
		                         self->rd_elem[i].rdi_key,
		                         self->rd_elem[i].rdi_value));
		is_first = false;
	}
	DO(err, is_first ? DeeFormat_PRINT(printer, arg, "})")
	                 : DeeFormat_PRINT(printer, arg, " })"));
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
		if (!self->rd_elem[i].rdi_key)
			continue;
		/* Note that we still combine the hashes for the key and value,
		 * thus not only mirroring the behavior of hash of the item (that
		 * is the tuple `(key, value)', including the order between the
		 * key and value within the hash, so that swapping the key and
		 * value would produce a different hash) */
		result ^= Dee_HashCombine(DeeObject_Hash(self->rd_elem[i].rdi_key),
		                          DeeObject_Hash(self->rd_elem[i].rdi_value));
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
	TYPE_METHOD_F(STR_get, &rodict_get, TYPE_METHOD_FNOREFESCAPE, DOC_GET(map_get_doc)),
	TYPE_KWMETHOD("byhash", &rodict_byhash, DOC_GET(map_byhash_doc)),
	TYPE_METHOD_END
};

PRIVATE struct type_getset tpconst rodict_getsets[] = {
	TYPE_GETTER(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_F("__sizeof__", &rodict_sizeof, TYPE_GETSET_FNOREFESCAPE, "->?Dint"),
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
		if (!self->rd_elem[i].rdi_key)
			continue;
		key_copy = DeeObject_DeepCopy(self->rd_elem[i].rdi_key);
		if unlikely(!key_copy)
			goto err;
		value_copy = DeeObject_DeepCopy(self->rd_elem[i].rdi_value);
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
