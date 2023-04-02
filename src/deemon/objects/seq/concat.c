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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_CONCAT_C
#define GUARD_DEEMON_OBJECTS_SEQ_CONCAT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/overflow.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../gc_inspect.h"
#include "svec.h"

DECL_BEGIN

typedef DeeTupleObject Tuple;
typedef DeeTupleObject Cat;

INTDEF DeeTypeObject SeqConcat_Type;
INTDEF DeeTypeObject SeqConcatIterator_Type;
#define SeqConcat_Check(ob) DeeObject_InstanceOfExact(ob, &SeqConcat_Type)

typedef struct {
	OBJECT_HEAD
	DREF DeeObject     *cti_curr; /* [1..1][lock(cti_lock)] The current iterator. */
	DeeObject   *const *cti_pseq; /* [1..1][1..1][lock(cti_lock)][in(cti_cat)] The current sequence. */
	DREF Cat           *cti_cat;  /* [1..1][const] The underly sequence cat. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t cti_lock; /* Lock for this iterator. */
#endif /* !CONFIG_NO_THREADS */
} CatIterator;

#define CatIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->cti_lock)
#define CatIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->cti_lock)
#define CatIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->cti_lock)
#define CatIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->cti_lock)
#define CatIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->cti_lock)
#define CatIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->cti_lock)
#define CatIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->cti_lock)
#define CatIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->cti_lock)
#define CatIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->cti_lock)
#define CatIterator_LockRead2(a, b)      Dee_atomic_rwlock_read_2(&(a)->cti_lock, &(b)->cti_lock)
#define CatIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->cti_lock)
#define CatIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->cti_lock)
#define CatIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->cti_lock)
#define CatIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->cti_lock)
#define CatIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->cti_lock)
#define CatIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->cti_lock)
#define CatIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->cti_lock)



PRIVATE WUNUSED NONNULL((1)) int DCALL
catiterator_ctor(CatIterator *__restrict self) {
	self->cti_cat = (DREF Cat *)DeeSeq_Concat(Dee_EmptySeq, Dee_EmptySeq);
	if unlikely(!self->cti_cat)
		goto err;
	self->cti_curr = DeeObject_IterSelf(Dee_EmptySeq);
	if unlikely(!self->cti_curr)
		goto err_cat;
	self->cti_pseq = DeeTuple_ELEM(self->cti_cat);
	Dee_atomic_rwlock_init(&self->cti_lock);
	return 0;
err_cat:
	Dee_Decref(self->cti_cat);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
catiterator_copy(CatIterator *__restrict self,
                 CatIterator *__restrict other) {
	DREF DeeObject *iterator;
	CatIterator_LockRead(other);
	iterator       = other->cti_curr;
	self->cti_pseq = other->cti_pseq;
	Dee_Incref(iterator);
	CatIterator_LockEndRead(other);
	self->cti_curr = DeeObject_Copy(iterator);
	Dee_Decref(iterator);
	if unlikely(!self->cti_curr)
		goto err;
	self->cti_cat = other->cti_cat;
	Dee_Incref(self->cti_cat);
	Dee_atomic_rwlock_init(&self->cti_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
catiterator_deep(CatIterator *__restrict self,
                 CatIterator *__restrict other) {
	DREF DeeObject *iterator;
	size_t sequence_index;
	CatIterator_LockRead(other);
	iterator       = other->cti_curr;
	sequence_index = other->cti_pseq - DeeTuple_ELEM(other->cti_cat);
	Dee_Incref(iterator);
	CatIterator_LockEndRead(other);
	self->cti_curr = DeeObject_DeepCopy(iterator);
	Dee_Decref(iterator);
	if unlikely(!self->cti_curr)
		goto err;
	self->cti_cat = (DREF Cat *)DeeObject_DeepCopy((DeeObject *)other->cti_cat);
	if unlikely(!self->cti_cat)
		goto err_curr;
	self->cti_pseq = DeeTuple_ELEM(self->cti_cat) + sequence_index;
	Dee_atomic_rwlock_init(&self->cti_lock);
	return 0;
err_curr:
	Dee_Decref(self->cti_curr);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
catiterator_init(CatIterator *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqConcatIterator", &self->cti_cat))
		goto err;
	if (DeeObject_AssertTypeExact(self->cti_cat, &SeqConcat_Type))
		goto err;
	self->cti_pseq = DeeTuple_ELEM(self->cti_cat);
	self->cti_curr = DeeObject_IterSelf(self->cti_pseq[0]);
	if unlikely(!self->cti_curr)
		goto err;
	Dee_Incref(self->cti_cat);
	Dee_atomic_rwlock_init(&self->cti_lock);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
catiterator_fini(CatIterator *__restrict self) {
	Dee_Decref(self->cti_curr);
	Dee_Decref(self->cti_cat);
}

PRIVATE NONNULL((1, 2)) void DCALL
catiterator_visit(CatIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->cti_curr);
	Dee_Visit(self->cti_cat);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
catiterator_bool(CatIterator *__restrict self) {
	int result;
	DeeObject *const *iterpos;
	DeeObject *const *catend;
	DREF DeeObject *curr;
	CatIterator_LockRead(self);
	curr = self->cti_curr;
	Dee_Incref(curr);
	CatIterator_LockEndRead(self);

	/* Check if the current iterator has remaining elements. */
	result = DeeObject_Bool(curr);
	Dee_Decref(curr);
	if (result != 0)
		goto done;
	iterpos = atomic_read(&self->cti_pseq);

	/* Check if one of the upcoming sequences is non-empty. */
	catend = DeeTuple_END(self->cti_cat);
	for (; iterpos < catend; ++iterpos) {
		result = DeeObject_Bool(*iterpos);
		if (result != 0)
			break;
	}
done:
	return result;
}


#define DEFINE_CATITERATOR_COMPARE(name, if_equal, if_diffseq, compare_object) \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                      \
	name(CatIterator *self, CatIterator *other) {                              \
		DREF DeeObject *result;                                                \
		DREF DeeObject *my_curr, *ot_curr;                                     \
		DREF DeeObject **my_pseq, **ot_pseq;                                   \
		if (DeeObject_AssertTypeExact(other, &SeqConcatIterator_Type))         \
			return NULL;                                                       \
		if (self == other)                                                     \
			if_equal;                                                          \
		CatIterator_LockRead2(self, other);                                    \
		my_pseq = (DREF DeeObject **)self->cti_pseq;                           \
		ot_pseq = (DREF DeeObject **)other->cti_pseq;                          \
		if (my_pseq != ot_pseq) {                                              \
			CatIterator_LockEndRead(other);                                    \
			CatIterator_LockEndRead(self);                                     \
			if_diffseq;                                                        \
		}                                                                      \
		my_curr = self->cti_curr;                                              \
		Dee_Incref(my_curr);                                                   \
		ot_curr = other->cti_curr;                                             \
		Dee_Incref(ot_curr);                                                   \
		CatIterator_LockEndRead(other);                                        \
		CatIterator_LockEndRead(self);                                         \
		result = compare_object(my_curr, ot_curr);                             \
		Dee_Decref(ot_curr);                                                   \
		Dee_Decref(my_curr);                                                   \
		return result;                                                         \
	}
DEFINE_CATITERATOR_COMPARE(catiterator_eq, return_true, return_false, DeeObject_CompareEqObject)
DEFINE_CATITERATOR_COMPARE(catiterator_ne, return_false, return_true, DeeObject_CompareNeObject)
DEFINE_CATITERATOR_COMPARE(catiterator_lo, return_false, return_bool_(my_pseq < ot_pseq), DeeObject_CompareLoObject)
DEFINE_CATITERATOR_COMPARE(catiterator_le, return_true, return_bool_(my_pseq < ot_pseq), DeeObject_CompareLeObject)
DEFINE_CATITERATOR_COMPARE(catiterator_gr, return_false, return_bool_(my_pseq > ot_pseq), DeeObject_CompareGrObject)
DEFINE_CATITERATOR_COMPARE(catiterator_ge, return_true, return_bool_(my_pseq > ot_pseq), DeeObject_CompareGeObject)
#undef DEFINE_CATITERATOR_COMPARE

PRIVATE struct type_cmp catiterator_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&catiterator_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&catiterator_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&catiterator_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&catiterator_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&catiterator_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&catiterator_ge
};



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
catiterator_next(CatIterator *__restrict self) {
	DREF DeeObject *iter, *result;
again_locked:
	CatIterator_LockRead(self);
again:
	iter = self->cti_curr;
	Dee_Incref(iter);
	CatIterator_LockEndRead(self);
do_iter:
	result = DeeObject_IterNext(iter);
	Dee_Decref(iter);
	if (!ITER_ISOK(result)) {
		DeeObject *const *pnext;
		if unlikely(!result)
			goto err;
		CatIterator_LockWrite(self);

		/* Check if the iterator has changed. */
		if (self->cti_curr != iter) {
			CatIterator_LockDowngrade(self);
			goto again;
		}

		/* Load the next sequence. */
		pnext = self->cti_pseq + 1;
		ASSERT(pnext > DeeTuple_ELEM(self->cti_cat));
		ASSERT(pnext <= DeeTuple_ELEM(self->cti_cat) + DeeTuple_SIZE(self->cti_cat));
		if unlikely(pnext == (DeeTuple_ELEM(self->cti_cat) +
		                      DeeTuple_SIZE(self->cti_cat))) {
			/* Fully exhausted. */
			CatIterator_LockEndWrite(self);
			return ITER_DONE;
		}
		CatIterator_LockEndWrite(self);

		/* Create an iterator for this sequence. */
		iter = DeeObject_IterSelf(*pnext);
		if unlikely(!iter)
			goto err;
		CatIterator_LockWrite(self);
		COMPILER_READ_BARRIER();

		/* Check if the sequence was changed by someone else. */
		if (self->cti_pseq != pnext - 1) {
			CatIterator_LockEndWrite(self);
			Dee_Decref(iter);
			goto again_locked;
		}

		/* Update the current sequence pointer. */
		self->cti_pseq = pnext;

		/* Store our new iterator, replacing the previous one. */
		result       = self->cti_curr;
		self->cti_curr = iter;
		Dee_Incref(iter); /* The reference now stored in `self->cti_curr' */
		CatIterator_LockEndWrite(self);

		/* Drop the old iterator. */
		Dee_Decref(result);
		goto do_iter;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
catiterator_seq_get(CatIterator *__restrict self) {
	DREF DeeObject *result;
	CatIterator_LockRead(self);
	result = *self->cti_pseq;
	Dee_Incref(result);
	CatIterator_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
catiterator_curr_get(CatIterator *__restrict self) {
	DREF DeeObject *result;
	CatIterator_LockRead(self);
	result = self->cti_curr;
	Dee_Incref(result);
	CatIterator_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
catiterator_curr_set(CatIterator *__restrict self,
                     DeeObject *__restrict value) {
	DREF DeeObject *oldval;
	if (DeeGC_ReferredBy(value, (DeeObject *)self))
		return err_reference_loop((DeeObject *)self, value);
	Dee_Incref(value);
	CatIterator_LockWrite(self);
	oldval       = self->cti_curr;
	self->cti_curr = value;
	CatIterator_LockEndRead(self);
	Dee_Decref(oldval);
	return 0;
}

PRIVATE struct type_getset tpconst catiterator_getsets[] = {
	TYPE_GETTER(STR_seq, &catiterator_seq_get, "->?Ert:SeqConcat"),
	TYPE_GETSET("__curr__", &catiterator_curr_get, NULL, &catiterator_curr_set, "->?DIterator"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst catiterator_members[] = {
	TYPE_MEMBER_FIELD_DOC("__sequences__", STRUCT_OBJECT, offsetof(CatIterator, cti_cat), "->?S?DSequence"),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject SeqConcatIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqConcatIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqConcat)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&catiterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&catiterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&catiterator_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&catiterator_init,
				TYPE_FIXED_ALLOCATOR(CatIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&catiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&catiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&catiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &catiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&catiterator_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ catiterator_getsets,
	/* .tp_members       = */ catiterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


INTDEF NONNULL((1)) void DCALL tuple_fini(Tuple *__restrict self);
INTDEF NONNULL((1, 2)) void DCALL tuple_visit(Tuple *__restrict self, dvisit_t proc, void *arg);
#define cat_fini   tuple_fini
#define cat_visit  tuple_visit

PRIVATE WUNUSED NONNULL((1)) DREF CatIterator *DCALL
cat_iter(Cat *__restrict self) {
	DREF CatIterator *result;
	result = DeeObject_MALLOC(CatIterator);
	if unlikely(!result)
		goto done;
	ASSERT(DeeTuple_SIZE(self) != 0);
	result->cti_curr = DeeObject_IterSelf(DeeTuple_GET(self, 0));
	if unlikely(!result->cti_curr)
		goto err_r;
	result->cti_pseq = DeeTuple_ELEM(self);
	result->cti_cat  = self;
	Dee_Incref(self);
	Dee_atomic_rwlock_init(&result->cti_lock);
	DeeObject_Init(result, &SeqConcatIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cat_get_sequences(Cat *__restrict self) {
	return DeeRefVector_NewReadonly((DeeObject *)self,
	                                DeeTuple_SIZE(self),
	                                DeeTuple_ELEM(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF Cat *DCALL
cat_get_frozen(Cat *__restrict self) {
	size_t i;
	DREF Cat *result;

	/* Recursively freeze all referenced sequences. */
	result = (DREF Cat *)DeeTuple_NewUninitialized(self->t_size);
	if unlikely(!result)
		goto err;
	for (i = 0; i < self->t_size; ++i) {
		DREF DeeObject *inner_frozen;
		inner_frozen = DeeObject_GetAttr(self->t_elem[i], (DeeObject *)&str_frozen);
		if unlikely(!inner_frozen)
			goto err_r_i;
		result->t_elem[i] = inner_frozen;
	}

	/* Check for special case: the frozen variants of all inner sequences
	 * are identical to the non-frozen versions (by-id). In this case, we
	 * don't need to actually copy `self', but can simply re-return the
	 * original cat-object! */
	if (bcmpc(result->t_elem, self->t_elem,
	          result->t_size, sizeof(DREF DeeObject *)) == 0) {
		for (i = 0; i < result->t_size; ++i)
			Dee_DecrefNokill(result->t_elem[i]);
		DeeTuple_FreeUninitialized((DeeObject *)result);
		return_reference_(self);
	}

	/* Fix the resulting object type. */
	ASSERT(result->ob_type == &DeeTuple_Type);
	Dee_DecrefNokill(&DeeTuple_Type);
	Dee_Incref(&SeqConcat_Type);
	result->ob_type = &SeqConcat_Type;
	return result;
err_r_i:
	Dee_Decrefv(result->t_elem, i);
	DeeTuple_FreeUninitialized((DeeObject *)result);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst cat_getsets[] = {
	TYPE_GETTER("__sequences__", &cat_get_sequences, "->?S?DSequence"),
	TYPE_GETTER(STR_frozen, &cat_get_frozen, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst cat_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqConcatIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &SeqConcat_Type),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cat_nsi_getsize(Cat *__restrict self) {
	size_t i, result = 0;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		size_t temp = DeeObject_Size(DeeTuple_GET(self, i));
		if unlikely(temp == (size_t)-1)
			return (size_t)-1;
		if (OVERFLOW_UADD(result, temp, &result)) {
			err_integer_overflow_i(sizeof(size_t) * 8, true);
			return (size_t)-1;
		}
		result += temp;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cat_size(Cat *__restrict self) {
	size_t result = cat_nsi_getsize(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cat_contains(Cat *self,
             DeeObject *search_item) {
	size_t i;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		DREF DeeObject *result;
		int error;
		result = DeeObject_ContainsObject(DeeTuple_GET(self, i), search_item);
		if unlikely(!result)
			goto err;
		error = DeeObject_Bool(result);
		if (error != 0) {
			if unlikely(error < 0)
				Dee_Clear(result);
			return result;
		}
		Dee_Decref(result);
	}
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cat_nsi_getitem(Cat *__restrict self, size_t index) {
	size_t i, temp, sub_index = index, total_length = 0;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		temp = DeeObject_Size(DeeTuple_GET(self, i));
		if (sub_index >= temp) {
			sub_index -= temp;
			total_length += temp;
			continue;
		}
		return DeeObject_GetItemIndex(DeeTuple_GET(self, i), sub_index);
	}
	err_index_out_of_bounds((DeeObject *)self, index, total_length);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cat_getitem(Cat *self,
            DeeObject *index_ob) {
	size_t index;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	return cat_nsi_getitem(self, index);
err:
	return NULL;
}

PRIVATE size_t DCALL
cat_nsi_find(Cat *__restrict self,
             size_t start, size_t end,
             DeeObject *__restrict keyed_search_item,
             DeeObject *key) {
	size_t temp, i, offset = 0;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		temp = DeeSeq_Find(DeeTuple_GET(self, i), start, end, keyed_search_item, key);
		if unlikely(temp == (size_t)-2)
			goto err;
		if (temp != (size_t)-1) {
			if (OVERFLOW_UADD(offset, temp, &offset))
				goto index_overflow;
			if unlikely(offset == (size_t)-1 ||
				         offset == (size_t)-2)
			goto index_overflow;
			return offset;
		}
		temp = DeeObject_Size(DeeTuple_GET(self, i));
		if unlikely(temp == (size_t)-1)
			goto err;
		if (temp >= end)
			break;
		start = 0;
		end -= temp;
		offset += temp;
	}
	return (size_t)-1;
index_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return (size_t)-2;
}

PRIVATE size_t DCALL
cat_nsi_rfind(Cat *__restrict self,
              size_t start, size_t end,
              DeeObject *__restrict keyed_search_item,
              DeeObject *key) {
	size_t seq_min      = 0, seq_max;
	size_t start_offset = 0, temp;
	size_t *seq_lengths, i;
	size_t effective_length;
	if unlikely(end <= start)
		goto done;
	seq_max = DeeTuple_SIZE(self);
	if (start != 0) {
		/* Find the first sequence which `start' is apart of. */
		for (;;) {
			if (seq_min >= seq_max)
				goto done;
			temp = DeeObject_Size(DeeTuple_GET(self, seq_min));
			if unlikely(temp == (size_t)-1)
				goto err;
			if (temp > start)
				break;
			start -= temp;
			end -= temp;
			start_offset += temp;
			++seq_min;
		}
		/* The first used sequence contains `temp' items! */
		if (end <= temp) {
			/* All items that should be searched for are located within a single sub-sequence! */
			temp = DeeSeq_RFind(DeeTuple_GET(self, seq_min), start, end, keyed_search_item, key);
			goto check_final_temp_from_first;
		}
		seq_lengths = (size_t *)Dee_AMallocc(seq_max - seq_min, sizeof(size_t));
		if unlikely(!seq_lengths)
			goto err;
		seq_lengths[0]   = temp; /* Remember the length of the first sequence. */
		i                = seq_min + 1;
		effective_length = temp;
	} else {
		seq_lengths = (size_t *)Dee_AMallocc(seq_max, sizeof(size_t));
		if unlikely(!seq_lengths)
			goto err;
		i                = seq_min;
		effective_length = 0;
	}
	/* Figure out the lengths of all sequences we're supposed to search */
	for (; i < seq_max; ++i) {
		if (effective_length >= end) {
			seq_max = i + 1;
			break;
		}
		temp = DeeObject_Size(DeeTuple_GET(self, i));
		if unlikely(temp == (size_t)-1)
			goto err_seqlen;
		effective_length += temp;
		seq_lengths[i - seq_min] = effective_length;
	}
	ASSERT((seq_max - seq_min) >= 2);
	/* Search the last sequence. */
	if (effective_length >= end) {
		temp = DeeSeq_RFind(DeeTuple_GET(self, seq_max - 1),
		                    0, end - seq_lengths[(seq_max - seq_min) - 2],
		                    keyed_search_item, key);
	} else {
		temp = DeeSeq_RFind(DeeTuple_GET(self, seq_max - 1), 0, (size_t)-1, keyed_search_item, key);
	}
check_temp_for_errors:
	if unlikely(temp == (size_t)-2)
		goto err;
	if (temp != (size_t)-1) {
		Dee_AFree(seq_lengths);
		if (OVERFLOW_UADD(temp, start_offset, &temp))
			goto index_overflow;
		if ((seq_max - seq_min) >= 2) {
			start_offset = seq_lengths[(seq_max - seq_min) - 2];
			goto add_start_offset;
		}
		return temp;
	}
	/* Not apart of the last sequence. -> Search all full sequences before then. */
	--seq_max;
	if (seq_max > seq_min + 1) {
		temp = DeeSeq_RFind(DeeTuple_GET(self, seq_max - 1),
		                    0, (size_t)-1,
		                    keyed_search_item, key);
		goto check_temp_for_errors;
	}
	ASSERT(seq_min + 1 == seq_max);
	Dee_AFree(seq_lengths);
	/* Search the first sequence. */
	temp = DeeSeq_RFind(DeeTuple_GET(self, seq_min),
	                    start, (size_t)-1, keyed_search_item, key);
check_final_temp_from_first:
	if likely(temp != (size_t)-2) {
		if (temp != (size_t)-1) {
add_start_offset:
			if (OVERFLOW_UADD(temp, start_offset, &temp))
				goto index_overflow;
		}
	}
	return temp;
done:
	return (size_t)-1;
index_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
	goto err;
err_seqlen:
	Dee_AFree(seq_lengths);
err:
	return (size_t)-2;
}


PRIVATE struct type_nsi tpconst cat_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&cat_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)NULL,
			/* .nsi_getitem      = */ (dfunptr_t)&cat_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)&cat_nsi_find,
			/* .nsi_rfind        = */ (dfunptr_t)&cat_nsi_rfind,
			/* .nsi_xch          = */ (dfunptr_t)NULL,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)NULL,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)NULL,
			/* .nsi_rremove      = */ (dfunptr_t)NULL,
			/* .nsi_removeall    = */ (dfunptr_t)NULL,
			/* .nsi_removeif     = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_seq cat_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cat_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cat_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cat_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cat_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &cat_nsi
};


INTDEF NONNULL((1)) void DCALL tuple_tp_free(void *__restrict ob);
#define cat_tp_free  tuple_tp_free

PRIVATE WUNUSED NONNULL((1)) int DCALL
cat_bool(Cat *__restrict self) {
	size_t i;
	int temp;
	for (i = 0; i < self->t_size; ++i) {
		temp = DeeObject_Bool(self->t_elem[i]);
		if (temp != 0)
			return temp;
	}
	return 0;
}




INTDEF WUNUSED NONNULL((1)) DREF Tuple *DCALL
tuple_deepcopy(Tuple *__restrict self);

PRIVATE WUNUSED NONNULL((1)) DREF Cat *DCALL
cat_deepcopy(Cat *__restrict self) {
	DREF Cat *result;
	result = (DREF Cat *)tuple_deepcopy((Tuple *)self);
	if likely(result) {
		Dee_Incref(&SeqConcat_Type);
		result->ob_type = &SeqConcat_Type;
		Dee_DecrefNokill(&DeeTuple_Type);
	}
	return result;
}


INTERN DeeTypeObject SeqConcat_Type = {
	/* NOTE: `_SeqConcat' objects are never empty, else
	 *        we'd get an overlap with `Dee_EmptyTuple' */
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqConcat",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&cat_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_free      = */ (dfunptr_t)&cat_tp_free,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cat_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&cat_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&cat_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &cat_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ cat_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ cat_class_members
};

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_Concat(DeeObject *self, DeeObject *other) {
	DREF DeeTupleObject *result;
	/* Special handling for recursive cats. */
	if (SeqConcat_Check(self)) {
		size_t lhs_size;
		lhs_size = DeeTuple_SIZE(self);
		if (SeqConcat_Check(other)) {
			DREF DeeObject **dst;
			size_t rhs_size;
			rhs_size = DeeTuple_SIZE(other);
			result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(lhs_size +
			                                                          rhs_size);
			if unlikely(!result)
				goto err;
			dst = Dee_Movprefv(DeeTuple_ELEM(result),
			                   DeeTuple_ELEM(self),
			                   lhs_size);
			Dee_Movrefv(dst, DeeTuple_ELEM(other), rhs_size);
		} else {
			DREF DeeObject **dst;
			result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(lhs_size + 1);
			if unlikely(!result)
				goto err;
			dst = Dee_Movprefv(DeeTuple_ELEM(result), DeeTuple_ELEM(self), lhs_size);
			*dst = other;
			Dee_Incref(other);
		}
	} else if (SeqConcat_Check(other)) {
		DREF DeeObject **dst;
		size_t rhs_size;
		rhs_size = DeeTuple_SIZE(other);
		result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(1 + rhs_size);
		if unlikely(!result)
			goto err;
		dst    = DeeTuple_ELEM(result);
		*dst++ = self;
		Dee_Incref(self);
		Dee_Movrefv(dst, DeeTuple_ELEM(other), rhs_size);
	} else {
		result = (DREF DeeTupleObject *)DeeTuple_Pack(2, self, other);
		if unlikely(!result)
			goto err;
	}

	/* Fix the resulting object type. */
	ASSERT(result->ob_type == &DeeTuple_Type);
	Dee_DecrefNokill(&DeeTuple_Type);
	Dee_Incref(&SeqConcat_Type);
	result->ob_type = &SeqConcat_Type;
	return (DREF DeeObject *)result;
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_CONCAT_C */
