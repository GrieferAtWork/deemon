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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_CONCAT_C
#define GUARD_DEEMON_OBJECTS_SEQ_CONCAT_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/gc.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/serial.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>        /* atomic_read */
#include <deemon/util/lock.h>          /* Dee_atomic_rwlock_init */

#include <hybrid/overflow.h> /* OVERFLOW_UADD */

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../gc_inspect.h"
#include "concat.h"

#include <stddef.h> /* NULL, offsetof, size_t */

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) int DCALL
catiterator_ctor(CatIterator *__restrict self) {
	self->cti_cat = (DREF Cat *)DeeSeq_Concat(Dee_EmptySeq, Dee_EmptySeq);
	if unlikely(!self->cti_cat)
		goto err;
	self->cti_curr = DeeObject_Iter(Dee_EmptySeq);
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
	iterator = other->cti_curr;
	Dee_Incref(iterator);
	self->cti_pseq = other->cti_pseq;
	CatIterator_LockEndRead(other);
	if unlikely((iterator = DeeObject_CopyInherited(iterator)) == NULL)
		goto err;
	self->cti_curr = iterator;
	self->cti_cat  = other->cti_cat;
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
	iterator = other->cti_curr;
	Dee_Incref(iterator);
	sequence_index = other->cti_pseq - DeeTuple_ELEM(other->cti_cat);
	CatIterator_LockEndRead(other);
	if unlikely((iterator = DeeObject_DeepCopyInherited(iterator)) == NULL)
		goto err;
	self->cti_curr = iterator;
	self->cti_cat  = (DREF Cat *)DeeObject_DeepCopy((DeeObject *)other->cti_cat);
	if unlikely(!self->cti_cat)
		goto err_iterator;
	self->cti_pseq = DeeTuple_ELEM(self->cti_cat) + sequence_index;
	Dee_atomic_rwlock_init(&self->cti_lock);
	return 0;
err_iterator:
	Dee_Decref(iterator);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
catiterator_init(CatIterator *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack1(err, argc, argv, "_SeqConcatIterator", &self->cti_cat);
	if (DeeObject_AssertTypeExact(self->cti_cat, &SeqConcat_Type))
		goto err;
	self->cti_pseq = DeeTuple_ELEM(self->cti_cat);
	self->cti_curr = DeeObject_InvokeMethodHint(seq_operator_iter, self->cti_pseq[0]);
	if unlikely(!self->cti_curr)
		goto err;
	Dee_Incref(self->cti_cat);
	Dee_atomic_rwlock_init(&self->cti_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
catiterator_serialize(CatIterator *__restrict self,
                      DeeSerial *__restrict writer,
                      Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(CatIterator, field))
	DREF DeeObject *in__cti_curr;
	DeeObject *const *in__cti_pseq;
	Dee_atomic_rwlock_init(&DeeSerial_Addr2Mem(writer, addr, CatIterator)->cti_lock);
	if (DeeSerial_PutObject(writer, ADDROF(cti_cat), self->cti_cat))
		goto err;
	CatIterator_LockRead(self);
	in__cti_curr = self->cti_curr;
	in__cti_pseq = self->cti_pseq;
	Dee_Incref(in__cti_curr);
	CatIterator_LockEndRead(self);
	if (DeeSerial_PutObjectInherited(writer, ADDROF(cti_curr), in__cti_curr))
		goto err;
	return DeeSerial_PutPointer(writer, ADDROF(cti_pseq), in__cti_pseq);
err:
	return -1;
#undef ADDROF
}

PRIVATE NONNULL((1)) void DCALL
catiterator_fini(CatIterator *__restrict self) {
	Dee_Decref(self->cti_curr);
	Dee_Decref(self->cti_cat);
}

PRIVATE NONNULL((1, 2)) void DCALL
catiterator_visit(CatIterator *__restrict self, Dee_visit_t proc, void *arg) {
	CatIterator_LockRead(self);
	Dee_Visit(self->cti_curr);
	CatIterator_LockEndRead(self);
	Dee_Visit(self->cti_cat);
}

PRIVATE NONNULL((1)) void DCALL
catiterator_clear(CatIterator *__restrict self) {
	DREF DeeObject *iter;
	CatIterator_LockWrite(self);
	iter = self->cti_curr; /* Inherit reference */
	self->cti_curr = DeeNone_NewRef();
	CatIterator_LockEndWrite(self);
	Dee_Decref(iter);
}

PRIVATE struct type_gc tpconst catiterator_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&catiterator_clear
};

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
	result = DeeObject_BoolInherited(curr);
	if (result != 0)
		goto done;
	iterpos = atomic_read(&self->cti_pseq);

	/* Check if one of the upcoming sequences is non-empty. */
	catend = DeeTuple_END(self->cti_cat);
	for (; iterpos < catend; ++iterpos) {
		result = DeeObject_InvokeMethodHint(seq_operator_bool, *iterpos);
		if (result != 0)
			break;
	}
done:
	return result;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
catiterator_compare(CatIterator *self, CatIterator *other) {
	DREF DeeObject *my_curr, *ot_curr;
	DeeObject *const *my_pseq, *const *ot_pseq;
	int result;
	if (DeeObject_AssertTypeExact(other, &SeqConcatIterator_Type))
		goto err;
	if (self == other)
		return 0;
	CatIterator_LockRead2(self, other);
	my_pseq = self->cti_pseq;
	ot_pseq = other->cti_pseq;
	if (my_pseq != ot_pseq) {
		CatIterator_LockEndRead(other);
		CatIterator_LockEndRead(self);
		return Dee_CompareNe(my_pseq, ot_pseq);
	}
	my_curr = self->cti_curr;
	Dee_Incref(my_curr);
	ot_curr = other->cti_curr;
	Dee_Incref(ot_curr);
	CatIterator_LockEndRead(other);
	CatIterator_LockEndRead(self);
	result = DeeObject_Compare(my_curr, ot_curr);
	Dee_Decref(ot_curr);
	Dee_Decref(my_curr);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp catiterator_cmp = {
	/* .tp_hash       = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&catiterator_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
catiterator_next(CatIterator *__restrict self) {
	DREF DeeObject *iter, *result;
again:
	CatIterator_LockRead(self);
again_locked:
	iter = self->cti_curr;
	Dee_Incref(iter);
	CatIterator_LockEndRead(self);
do_iter:
	result = DeeObject_IterNext(iter);
	Dee_Decref(iter);
	if (!ITER_ISOK(result)) {
		DeeObject *const *p_next;
		if unlikely(!result)
			goto err;
		CatIterator_LockWrite(self);

		/* Check if the iterator has changed. */
		if (self->cti_curr != iter) {
			CatIterator_LockDowngrade(self);
			goto again_locked;
		}

		/* Load the next sequence. */
		p_next = self->cti_pseq + 1;
		ASSERT(p_next > DeeTuple_ELEM(self->cti_cat));
		ASSERT(p_next <= DeeTuple_ELEM(self->cti_cat) + DeeTuple_SIZE(self->cti_cat));
		if unlikely(p_next == (DeeTuple_ELEM(self->cti_cat) +
		                      DeeTuple_SIZE(self->cti_cat))) {
			/* Fully exhausted. */
			CatIterator_LockEndWrite(self);
			return ITER_DONE;
		}
		CatIterator_LockEndWrite(self);

		/* Create an iterator for this sequence. */
		iter = DeeObject_InvokeMethodHint(seq_operator_iter, *p_next);
		if unlikely(!iter)
			goto err;
		CatIterator_LockWrite(self);
		COMPILER_READ_BARRIER();

		/* Check if the sequence was changed by someone else. */
		if (self->cti_pseq != p_next - 1) {
			CatIterator_LockEndWrite(self);
			Dee_Decref(iter);
			goto again;
		}

		/* Update the current sequence pointer. */
		self->cti_pseq = p_next;

		/* Store our new iterator, replacing the previous one. */
		result = self->cti_curr;
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
	if (DeeGC_ReferredBy(value, Dee_AsObject(self)))
		return err_reference_loop(Dee_AsObject(self), value);
	Dee_Incref(value);
	CatIterator_LockWrite(self);
	oldval       = self->cti_curr;
	self->cti_curr = value;
	CatIterator_LockEndRead(self);
	Dee_Decref(oldval);
	return 0;
}

PRIVATE struct type_getset tpconst catiterator_getsets[] = {
	TYPE_GETTER_AB_F(STR_seq, &catiterator_seq_get, METHOD_FNOREFESCAPE, "->?Ert:SeqConcat"),
	TYPE_GETSET_AB_F("__curr__", &catiterator_curr_get, NULL, &catiterator_curr_set, METHOD_FNOREFESCAPE, "->?DIterator"),
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
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ CatIterator,
			/* tp_ctor:        */ &catiterator_ctor,
			/* tp_copy_ctor:   */ &catiterator_copy,
			/* tp_deep_ctor:   */ &catiterator_deep,
			/* tp_any_ctor:    */ &catiterator_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &catiterator_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&catiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&catiterator_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&catiterator_visit,
	/* .tp_gc            = */ &catiterator_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &catiterator_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&catiterator_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ catiterator_getsets,
	/* .tp_members       = */ catiterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


INTDEF NONNULL((1)) void DCALL tuple_fini(DeeTupleObject *__restrict self);
INTDEF NONNULL((1, 2)) void DCALL tuple_visit(DeeTupleObject *__restrict self, Dee_visit_t proc, void *arg);
INTERN WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL tuple_serialize(DeeTupleObject *__restrict self, DeeSerial *__restrict writer);
#define cat_serialize tuple_serialize
#define cat_fini      tuple_fini
#define cat_visit     tuple_visit

PRIVATE WUNUSED NONNULL((1)) DREF CatIterator *DCALL
cat_iter(Cat *__restrict self) {
	DREF CatIterator *result;
	result = DeeGCObject_MALLOC(CatIterator);
	if unlikely(!result)
		goto err;
	ASSERT(DeeTuple_SIZE(self) != 0);
	result->cti_curr = DeeObject_InvokeMethodHint(seq_operator_iter, DeeTuple_GET(self, 0));
	if unlikely(!result->cti_curr)
		goto err_r;
	result->cti_pseq = DeeTuple_ELEM(self);
	result->cti_cat  = self;
	Dee_Incref(self);
	Dee_atomic_rwlock_init(&result->cti_lock);
	DeeObject_Init(result, &SeqConcatIterator_Type);
	return DeeGC_TRACK(CatIterator, result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cat_get_sequences(Cat *__restrict self) {
	return DeeRefVector_NewReadonly(Dee_AsObject(self),
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
		inner_frozen = DeeObject_InvokeMethodHint(seq_frozen, self->t_elem[i]);
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
		DeeTuple_FreeUninitialized(result);
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
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst cat_getsets[] = {
	TYPE_GETTER_AB_F("__sequences__", &cat_get_sequences, METHOD_FNOREFESCAPE, "->?S?DSequence"),
	TYPE_GETTER_AB(STR_frozen, &cat_get_frozen, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst cat_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqConcatIterator_Type),
	TYPE_MEMBER_CONST(STR_Frozen, &SeqConcat_Type),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cat_size(Cat *__restrict self) {
	size_t i, result = 0;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		size_t new_result;
		DeeObject *item = DeeTuple_GET(self, i);
		size_t temp = DeeObject_InvokeMethodHint(seq_operator_size, item);
		if unlikely(temp == (size_t)-1)
			return (size_t)-1;
		if (OVERFLOW_UADD(result, temp, &new_result)) {
			DeeRT_ErrIntegerOverflowUAdd(result, temp);
			return (size_t)-1;
		}
		result = new_result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
cat_size_fast(Cat *__restrict self) {
	size_t i, result = 0;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		size_t temp = DeeObject_SizeFast(DeeTuple_GET(self, i));
		if unlikely(temp == (size_t)-1)
			return (size_t)-1;
		if (OVERFLOW_UADD(result, temp, &result))
			return (size_t)-1;
		result += temp;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
cat_contains(Cat *self, DeeObject *search_item) {
	size_t i;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		int error = DeeObject_InvokeMethodHint(seq_contains, DeeTuple_GET(self, i), search_item);
		if (error != 0) {
			if unlikely(error < 0)
				goto err;
			return_true;
		}
	}
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cat_getitem_index(Cat *__restrict self, size_t index) {
	size_t i, temp, sub_index = index, total_length = 0;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		temp = DeeObject_InvokeMethodHint(seq_operator_size, DeeTuple_GET(self, i));
		if unlikely(temp == (size_t)-1)
			goto err;
		if (sub_index >= temp) {
			sub_index -= temp;
			total_length += temp;
			continue;
		}
		return DeeObject_InvokeMethodHint(seq_operator_getitem_index,
		                                  DeeTuple_GET(self, i),
		                                  sub_index);
	}
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, total_length);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cat_delitem_index(Cat *__restrict self, size_t index) {
	size_t i, temp, sub_index = index, total_length = 0;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		temp = DeeObject_InvokeMethodHint(seq_operator_size, DeeTuple_GET(self, i));
		if unlikely(temp == (size_t)-1)
			goto err;
		if (sub_index >= temp) {
			sub_index -= temp;
			total_length += temp;
			continue;
		}
		return DeeObject_InvokeMethodHint(seq_operator_delitem_index,
		                                  DeeTuple_GET(self, i), sub_index);
	}
	return DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, total_length);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
cat_setitem_index(Cat *self, size_t index, DeeObject *value) {
	size_t i, temp, sub_index = index, total_length = 0;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		temp = DeeObject_InvokeMethodHint(seq_operator_size, DeeTuple_GET(self, i));
		if unlikely(temp == (size_t)-1)
			goto err;
		if (sub_index >= temp) {
			sub_index -= temp;
			total_length += temp;
			continue;
		}
		return DeeObject_InvokeMethodHint(seq_operator_setitem_index,
		                                  DeeTuple_GET(self, i),
		                                  sub_index, value);
	}
	return DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, total_length);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cat_bounditem_index(Cat *self, size_t index) {
	size_t i, temp, sub_index = index;
	for (i = 0; i < DeeTuple_SIZE(self); ++i) {
		temp = DeeObject_InvokeMethodHint(seq_operator_size, DeeTuple_GET(self, i));
		if unlikely(temp == (size_t)-1)
			goto err;
		if (sub_index >= temp) {
			sub_index -= temp;
			continue;
		}
		return DeeObject_InvokeMethodHint(seq_operator_bounditem_index,
		                                  DeeTuple_GET(self, i), sub_index);
	}
	return Dee_BOUND_MISSING;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cat_foreach(Cat *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	for (i = 0; i < self->t_size; ++i) {
		temp = DeeObject_InvokeMethodHint(seq_operator_foreach,
		                                  DeeTuple_GET(self, i),
		                                  proc, arg);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE struct type_seq cat_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cat_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&cat_contains,
	/* .tp_getitem            = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__delitem__with__delitem_index),
	/* .tp_setitem            = */ DEFIMPL(&default__setitem__with__setitem_index),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&cat_foreach,
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem          = */ DEFIMPL(&default__bounditem__with__bounditem_index),
	/* .tp_hasitem            = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&cat_size,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&cat_size_fast,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&cat_getitem_index,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&cat_delitem_index,
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&cat_setitem_index,
	/* .tp_bounditem_index    = */ (int (DCALL *)(DeeObject *, size_t))&cat_bounditem_index,
	/* .tp_hasitem_index      = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
};


#if defined(CONFIG_NO_CACHES) || defined(CONFIG_NO_TUPLE_CACHES) || defined(CONFIG_EXPERIMENTAL_MMAP_DEC)
#define cat_tp_free_PTR NULL
#else /* CONFIG_NO_CACHES || CONFIG_NO_TUPLE_CACHES || CONFIG_EXPERIMENTAL_MMAP_DEC */
INTDEF NONNULL((1)) void DCALL tuple_tp_free(void *__restrict ob);
#define cat_tp_free_PTR &tuple_tp_free
#endif /* !CONFIG_NO_CACHES && !CONFIG_NO_TUPLE_CACHES && !CONFIG_EXPERIMENTAL_MMAP_DEC */

PRIVATE WUNUSED NONNULL((1)) int DCALL
cat_bool(Cat *__restrict self) {
	size_t i;
	for (i = 0; i < self->t_size; ++i) {
		int temp = DeeObject_Bool(self->t_elem[i]);
		if (temp != 0)
			return temp;
	}
	return 0;
}




INTDEF WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
tuple_deepcopy(DeeTupleObject *__restrict self);

PRIVATE WUNUSED NONNULL((1)) DREF Cat *DCALL
cat_deepcopy(Cat *__restrict self) {
	DREF Cat *result;
	result = (DREF Cat *)tuple_deepcopy((DeeTupleObject *)self);
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
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &cat_deepcopy,
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &cat_serialize,
			/* tp_free:        */ cat_tp_free_PTR
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&cat_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&cat_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&cat_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__26B2EC529683DE3C),
	/* .tp_seq           = */ &cat_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ cat_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ cat_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
			result = DeeTuple_NewUninitialized(lhs_size + rhs_size);
			if unlikely(!result)
				goto err;
			dst = Dee_Movprefv(DeeTuple_ELEM(result),
			                   DeeTuple_ELEM(self),
			                   lhs_size);
			Dee_Movrefv(dst, DeeTuple_ELEM(other), rhs_size);
		} else {
			DREF DeeObject **dst;
			result = DeeTuple_NewUninitialized(lhs_size + 1);
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
		result = DeeTuple_NewUninitialized(1 + rhs_size);
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
	return Dee_AsObject(result);
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_CONCAT_C */
