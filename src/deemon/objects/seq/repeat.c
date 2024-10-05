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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_REPEAT_C
#define GUARD_DEEMON_OBJECTS_SEQ_REPEAT_C 1

#include "repeat.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/util/atomic.h>

#include <hybrid/limitcore.h>
#include <hybrid/overflow.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../gc_inspect.h"
#include "../seq_functions.h"

#undef SSIZE_MAX
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

#define REPEATITER_READ_NUM(x) atomic_read(&(x)->rpi_num)

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatiter_ctor(RepeatIterator *__restrict self) {
	self->rpi_rep = (DREF Repeat *)DeeSeq_Repeat(Dee_EmptySeq, 1);
	if unlikely(!self->rpi_rep)
		goto err;
	self->rpi_iter = DeeObject_Iter(Dee_EmptySeq);
	if unlikely(!self->rpi_iter) { Dee_Decref(self->rpi_rep); }
	Dee_atomic_rwlock_init(&self->rpi_lock);
	self->rpi_num = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_copy(RepeatIterator *__restrict self,
                RepeatIterator *__restrict other) {
	DREF DeeObject *copy;
	RepeatIterator_LockRead(other);
	self->rpi_num  = other->rpi_num;
	self->rpi_iter = other->rpi_iter;
	Dee_Incref(self->rpi_iter);
	RepeatIterator_LockEndRead(other);
	copy = DeeObject_Copy(self->rpi_iter);
	Dee_Decref(self->rpi_iter);
	if unlikely(!copy)
		goto err;
	self->rpi_iter = copy;
	self->rpi_rep  = other->rpi_rep;
	Dee_Incref(self->rpi_rep);
	Dee_atomic_rwlock_init(&self->rpi_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_deep(RepeatIterator *__restrict self,
                RepeatIterator *__restrict other) {
	DREF DeeObject *copy;
	RepeatIterator_LockRead(other);
	self->rpi_num  = other->rpi_num;
	self->rpi_iter = other->rpi_iter;
	Dee_Incref(self->rpi_iter);
	RepeatIterator_LockEndRead(other);
	copy = DeeObject_DeepCopy(self->rpi_iter);
	Dee_Decref(self->rpi_iter);
	if unlikely(!copy)
		goto err;
	self->rpi_iter = copy;
	self->rpi_rep  = (DREF Repeat *)DeeObject_DeepCopy((DeeObject *)other->rpi_rep);
	if unlikely(!self->rpi_rep)
		goto err_iter;
	Dee_atomic_rwlock_init(&self->rpi_lock);
	return 0;
err_iter:
	Dee_Decref(self->rpi_iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatiter_init(RepeatIterator *__restrict self,
                size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqRepeatIterator", &self->rpi_rep))
		goto err;
	if (DeeObject_AssertTypeExact(self->rpi_rep, &SeqRepeat_Type))
		goto err;
	self->rpi_iter = DeeObject_Iter(self->rpi_rep->rp_seq);
	if unlikely(!self->rpi_iter)
		goto err;
	Dee_Incref(self->rpi_rep);
	Dee_atomic_rwlock_init(&self->rpi_lock);
	self->rpi_num = self->rpi_rep->rp_num - 1;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
repeatiter_fini(RepeatIterator *__restrict self) {
	Dee_Decref(self->rpi_rep);
	Dee_Decref(self->rpi_iter);
}

PRIVATE NONNULL((1, 2)) void DCALL
repeatiter_visit(RepeatIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->rpi_rep);
	Dee_Visit(self->rpi_iter);
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
repeatiter_hash(RepeatIterator *self) {
	Dee_hash_t result;
	DREF DeeObject *my_iter;
	result = REPEATITER_READ_NUM(self);
	RepeatIterator_LockRead(self);
	my_iter = self->rpi_iter;
	Dee_Incref(my_iter);
	RepeatIterator_LockEndRead(self);
	result = Dee_HashCombine(result, DeeObject_Hash(my_iter));
	Dee_Decref(my_iter);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_compare(RepeatIterator *self, RepeatIterator *other) {
	int result;
	size_t my_len, ot_len;
	DREF DeeObject *my_iter, *ot_iter;
	if (DeeObject_AssertTypeExact(other, &SeqRepeatIterator_Type))
		goto err;
	my_len = REPEATITER_READ_NUM(self);
	ot_len = REPEATITER_READ_NUM(other);
	if (my_len != ot_len)
		return Dee_CompareNe(my_len, ot_len);
	RepeatIterator_LockRead(self);
	my_iter = self->rpi_iter;
	Dee_Incref(my_iter);
	RepeatIterator_LockEndRead(self);
	RepeatIterator_LockRead(other);
	ot_iter = other->rpi_iter;
	Dee_Incref(ot_iter);
	RepeatIterator_LockEndRead(other);
	result = DeeObject_Compare(my_iter, ot_iter);
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_compare_eq(RepeatIterator *self, RepeatIterator *other) {
	int result;
	size_t my_len, ot_len;
	DREF DeeObject *my_iter, *ot_iter;
	if (DeeObject_AssertTypeExact(other, &SeqRepeatIterator_Type))
		goto err;
	my_len = REPEATITER_READ_NUM(self);
	ot_len = REPEATITER_READ_NUM(other);
	if (my_len != ot_len)
		return 1;
	RepeatIterator_LockRead(self);
	my_iter = self->rpi_iter;
	Dee_Incref(my_iter);
	RepeatIterator_LockEndRead(self);
	RepeatIterator_LockRead(other);
	ot_iter = other->rpi_iter;
	Dee_Incref(ot_iter);
	RepeatIterator_LockEndRead(other);
	result = DeeObject_CompareEq(my_iter, ot_iter);
	Dee_Decref(ot_iter);
	Dee_Decref(my_iter);
	return result;
err:
	return Dee_COMPARE_ERR;
}


PRIVATE struct type_cmp repeatiter_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&repeatiter_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&repeatiter_compare_eq,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&repeatiter_compare,
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatiter_next(RepeatIterator *__restrict self) {
	DREF DeeObject *result, *iter;
	RepeatIterator_LockRead(self);
	iter = self->rpi_iter;
	Dee_Incref(iter);
	RepeatIterator_LockEndRead(self);
again_iter:
	result = DeeObject_IterNext(iter);
	Dee_Decref(iter);
	if (result != ITER_DONE) {
done:
		return result;
	}
	if (!REPEATITER_READ_NUM(self))
		goto done;
	/* Create a new iterator for the next loop. */
	iter = DeeObject_Iter(self->rpi_rep->rp_seq);
	if unlikely(!iter)
		goto err;
	COMPILER_READ_BARRIER();
	RepeatIterator_LockWrite(self);
	if unlikely(!self->rpi_num) {
		RepeatIterator_LockEndWrite(self);
		Dee_Decref(iter);
		ASSERT(result == ITER_DONE);
		goto done;
	}
	result = self->rpi_iter;
	Dee_Incref(iter);
	self->rpi_iter = iter;
	--self->rpi_num; /* Decrement the number of remaining loops. */
	RepeatIterator_LockEndWrite(self);
	Dee_Decref(result); /* Drop the old iterator. */
	goto again_iter;    /* Read more items. */
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatiter_get_iter(RepeatIterator *__restrict self) {
	DREF DeeObject *result;
	RepeatIterator_LockRead(self);
	result = self->rpi_iter;
	Dee_Incref(result);
	RepeatIterator_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_set_iter(RepeatIterator *__restrict self,
                    DeeObject *__restrict value) {
	DREF DeeObject *oldvalue;
	if (DeeGC_ReferredBy(value, (DeeObject *)self))
		return err_reference_loop((DeeObject *)self, value);
	Dee_Incref(value);
	RepeatIterator_LockWrite(self);
	oldvalue      = self->rpi_iter;
	self->rpi_iter = value;
	RepeatIterator_LockEndWrite(self);
	Dee_Decref(oldvalue);
	return 0;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatiter_get_num(RepeatIterator *__restrict self) {
	size_t result;
	RepeatIterator_LockRead(self);
	result = self->rpi_num;
	RepeatIterator_LockEndRead(self);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_set_num(RepeatIterator *__restrict self,
                   DeeObject *__restrict value) {
	size_t newvalue;
	if (DeeObject_AsSize(value, &newvalue))
		goto err;
	RepeatIterator_LockWrite(self);
	self->rpi_num = newvalue;
	RepeatIterator_LockEndWrite(self);
	return 0;
err:
	return -1;
}


PRIVATE struct type_getset tpconst repeatiter_getsets[] = {
	TYPE_GETSET_F("__iter__", &repeatiter_get_iter, NULL, &repeatiter_set_iter, METHOD_FNOREFESCAPE, "->?DIterator"),
	TYPE_GETSET_F("__num__", &repeatiter_get_num, NULL, &repeatiter_set_num, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst repeatiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(RepeatIterator, rpi_rep), "->?Ert:SeqRepeat"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqRepeatIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRepeatIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqRepeat)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&repeatiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&repeatiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&repeatiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&repeatiter_init,
				TYPE_FIXED_ALLOCATOR(RepeatIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&repeatiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&repeatiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &repeatiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeatiter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ repeatiter_getsets,
	/* .tp_members       = */ repeatiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_ctor(Repeat *__restrict self) {
	self->rp_num = 1;
	self->rp_seq = Dee_EmptySeq;
	Dee_Incref(Dee_EmptySeq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeat_copy(Repeat *__restrict self,
            Repeat *__restrict other) {
	self->rp_num = other->rp_num;
	self->rp_seq = other->rp_seq;
	Dee_Incref(self->rp_seq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeat_deep(Repeat *__restrict self,
            Repeat *__restrict other) {
	self->rp_seq = DeeObject_DeepCopy(other->rp_seq);
	if unlikely(!self->rp_seq)
		goto err;
	self->rp_num = other->rp_num;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_init(Repeat *__restrict self,
            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":_SeqRepeat", &self->rp_seq, &self->rp_num))
		goto err;
	if unlikely(!self->rp_num) {
		self->rp_seq = Dee_EmptySeq;
		self->rp_num = 1;
	}
	Dee_Incref(self->rp_seq);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
repeat_fini(Repeat *__restrict self) {
	Dee_Decref(self->rp_seq);
}

PRIVATE NONNULL((1, 2)) void DCALL
repeat_visit(Repeat *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->rp_seq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_bool(Repeat *__restrict self) {
	return DeeObject_Bool(self->rp_seq);
}

PRIVATE WUNUSED NONNULL((1)) DREF RepeatIterator *DCALL
repeat_iter(Repeat *__restrict self) {
	DREF RepeatIterator *result;
	result = DeeObject_MALLOC(RepeatIterator);
	if unlikely(!result)
		goto done;
	result->rpi_iter = DeeObject_Iter(self->rp_seq);
	if unlikely(!result->rpi_iter)
		goto err_r;
	result->rpi_rep = self;
	result->rpi_num = self->rp_num - 1;
	Dee_atomic_rwlock_init(&result->rpi_lock);
	Dee_Incref(self);
	DeeObject_Init(result, &SeqRepeatIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeat_contains(Repeat *self, DeeObject *item) {
	return DeeObject_Contains(self->rp_seq, item);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeat_getitem(Repeat *self, DeeObject *index_ob) {
	size_t index;
	size_t seq_size;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	seq_size = DeeObject_Size(self->rp_seq);
	if unlikely(seq_size == (size_t)-1)
		goto err;
	if unlikely(index >= seq_size * self->rp_num) {
		err_index_out_of_bounds((DeeObject *)self, index,
		                        seq_size * self->rp_num);
		goto err;
	}
	index %= seq_size;
	return DeeObject_GetItemIndex(self->rp_seq, index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeat_size(Repeat *__restrict self) {
	size_t base_size;
	size_t result;
	base_size = DeeObject_Size(self->rp_seq);
	if unlikely(base_size == (size_t)-1)
		return (size_t)-1;
	if (OVERFLOW_UMUL(base_size, self->rp_num, &result)) {
		err_integer_overflow_i(sizeof(size_t) * 8, true);
		return (size_t)-1;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeat_size_fast(Repeat *__restrict self) {
	size_t base_size;
	size_t result;
	base_size = DeeFastSeq_GetSize_deprecated(self->rp_seq);
	if unlikely(base_size == (size_t)-1)
		return (size_t)-1;
	if (OVERFLOW_UMUL(base_size, self->rp_num, &result))
		result = (size_t)-1;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeat_getitem_index(Repeat *__restrict self, size_t index) {
	size_t seq_size;
	seq_size = DeeObject_Size(self->rp_seq);
	if unlikely(seq_size == (size_t)-1)
		goto err;
	if unlikely(index >= seq_size * self->rp_num) {
		err_index_out_of_bounds((DeeObject *)self, index,
		                        seq_size * self->rp_num);
		goto err;
	}
	index %= seq_size;
	return DeeObject_GetItemIndex(self->rp_seq, index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeat_foreach(Repeat *__restrict self, Dee_foreach_t proc, void *arg) {
	DeeTypeObject *tp_seq;
	Dee_ssize_t temp, result;
	tp_seq = Dee_TYPE(self->rp_seq);
	if likely((tp_seq->tp_seq && tp_seq->tp_seq->tp_foreach) ||
	          DeeType_InheritIter(tp_seq)) {
		size_t i = 0;
		result = 0;
		do {
			temp = (*tp_seq->tp_seq->tp_foreach)(self->rp_seq, proc, arg);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		} while (++i < self->rp_num);
		return result;
	}
	return err_unimplemented_operator(tp_seq, OPERATOR_ITER);
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeat_foreach_pair(Repeat *__restrict self, Dee_foreach_pair_t proc, void *arg) {
	DeeTypeObject *tp_seq;
	Dee_ssize_t temp, result;
	tp_seq = Dee_TYPE(self->rp_seq);
	if likely((tp_seq->tp_seq && tp_seq->tp_seq->tp_foreach_pair) ||
	          DeeType_InheritIter(tp_seq)) {
		size_t i = 0;
		result = 0;
		do {
			temp = (*tp_seq->tp_seq->tp_foreach_pair)(self->rp_seq, proc, arg);
			if unlikely(temp < 0)
				goto err_temp;
			result += temp;
		} while (++i < self->rp_num);
		return result;
	}
	return err_unimplemented_operator(tp_seq, OPERATOR_ITER);
err_temp:
	return temp;
}



PRIVATE struct type_nsi tpconst repeat_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&repeat_size,
			/* .nsi_getsize_fast = */ (dfunptr_t)&repeat_size_fast,
			/* .nsi_getitem      = */ (dfunptr_t)&repeat_getitem_index,
		}
	}
};


PRIVATE struct type_seq repeat_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeat_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeat_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeat_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ &repeat_nsi,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&repeat_foreach,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&repeat_foreach_pair,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&repeat_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&repeat_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&repeat_getitem_index,
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
	/* .tp_trygetitem                 = */ NULL,
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

PRIVATE WUNUSED NONNULL((1)) DREF Repeat *DCALL
repeat_get_frozen(Repeat *__restrict self) {
	DREF DeeObject *inner_frozen;
	DREF Repeat *result;
	inner_frozen = DeeObject_GetAttr(self->rp_seq, (DeeObject *)&str_frozen);
	if unlikely(!inner_frozen)
		goto err;
	if (inner_frozen == self->rp_seq) {
		Dee_DecrefNokill(inner_frozen);
		return_reference_(self);
	}
	result = DeeObject_MALLOC(Repeat);
	if unlikely(!result)
		goto err_inner;
	result->rp_seq = inner_frozen; /* Inherit reference */
	result->rp_num = self->rp_num;
	DeeObject_Init(result, &SeqRepeat_Type);
	return result;
err_inner:
	Dee_Decref(inner_frozen);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst repeat_getsets[] = {
	TYPE_GETTER(STR_frozen, &repeat_get_frozen, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst repeat_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(Repeat, rp_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__num__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(Repeat, rp_num)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst repeat_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqRepeatIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &SeqRepeat_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject SeqRepeat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqRepeat",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,num:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&repeat_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&repeat_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&repeat_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&repeat_init,
				TYPE_FIXED_ALLOCATOR(Repeat)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&repeat_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&repeat_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&repeat_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &repeat_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ repeat_getsets,
	/* .tp_members       = */ repeat_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ repeat_class_members
};






#define REPEATITEMPITER_READ_NUM(x) atomic_read(&(x)->rii_num)

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitemiter_ctor(RepeatItemIterator *__restrict self) {
	self->rii_rep = (DREF RepeatItem *)DeeSeq_RepeatItem(Dee_None, 0);
	if unlikely(!self->rii_rep)
		goto err;
	self->rii_obj = Dee_None;
	self->rii_num = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitemiter_copy(RepeatItemIterator *__restrict self,
                    RepeatItemIterator *__restrict other) {
	self->rii_num = REPEATITEMPITER_READ_NUM(other);
	self->rii_obj = other->rii_obj;
	self->rii_rep = other->rii_rep;
	Dee_Incref(self->rii_rep);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitemiter_deep(RepeatItemIterator *__restrict self,
                    RepeatItemIterator *__restrict other) {
	self->rii_num = REPEATITEMPITER_READ_NUM(other);
	self->rii_rep = (DREF RepeatItem *)DeeObject_DeepCopy((DeeObject *)other->rii_rep);
	if unlikely(!self->rii_rep)
		goto err;
	self->rii_obj = self->rii_rep->rpit_obj;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitemiter_init(RepeatItemIterator *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqItemRepeatIterator", &self->rii_rep))
		goto err;
	if (DeeObject_AssertTypeExact(self->rii_rep, &SeqItemRepeat_Type))
		goto err;
	self->rii_obj = self->rii_rep->rpit_obj;
	self->rii_num = self->rii_rep->rpit_num;
	Dee_Incref(self->rii_rep);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
repeatitemiter_fini(RepeatItemIterator *__restrict self) {
	Dee_Decref(self->rii_rep);
}

PRIVATE NONNULL((1, 2)) void DCALL
repeatitemiter_visit(RepeatItemIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->rii_rep);
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
repeatitemiter_hash(RepeatItemIterator *self) {
	return Dee_HashCombine(DeeObject_HashGeneric(self->rii_obj),
	                       REPEATITEMPITER_READ_NUM(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitemiter_compare(RepeatItemIterator *lhs,
                       RepeatItemIterator *rhs) {
	if (DeeObject_AssertTypeExact(rhs, &SeqItemRepeatIterator_Type))
		goto err;
	Dee_return_compare_if_ne(lhs->rii_obj, rhs->rii_obj);
	Dee_return_compareT(size_t, REPEATITEMPITER_READ_NUM(lhs),
	                    /*   */ REPEATITEMPITER_READ_NUM(rhs));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp repeatitemiter_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&repeatitemiter_hash,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&repeatitemiter_compare,
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitemiter_next(RepeatItemIterator *__restrict self) {
	size_t count;
	do {
		count = atomic_read(&self->rii_num);
		if (count == 0)
			return ITER_DONE;
	} while (!atomic_cmpxch_weak_or_write(&self->ob_refcnt, count, count - 1));
	return_reference_(self->rii_obj);
}

PRIVATE struct type_member tpconst repeatitemiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(RepeatItemIterator, rii_rep), "->?Ert:SeqItemRepeat"),
	TYPE_MEMBER_FIELD("__obj__", STRUCT_OBJECT, offsetof(RepeatItemIterator, rii_obj)),
	TYPE_MEMBER_FIELD("__num__", STRUCT_SIZE_T, offsetof(RepeatItemIterator, rii_num)),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqItemRepeatIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqItemRepeatIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqItemRepeat)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&repeatitemiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&repeatitemiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&repeatitemiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&repeatitemiter_init,
				TYPE_FIXED_ALLOCATOR(RepeatItemIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&repeatitemiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&repeatitemiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &repeatitemiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeatitemiter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ repeatitemiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_ctor(RepeatItem *__restrict self) {
	self->rpit_num = 1;
	self->rpit_obj = Dee_None;
	Dee_Incref(Dee_None);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitem_copy(RepeatItem *__restrict self,
                RepeatItem *__restrict other) {
	self->rpit_num = other->rpit_num;
	self->rpit_obj = other->rpit_obj;
	Dee_Incref(self->rpit_obj);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitem_deep(RepeatItem *__restrict self,
                RepeatItem *__restrict other) {
	self->rpit_num = other->rpit_num;
	self->rpit_obj = DeeObject_DeepCopy(other->rpit_obj);
	if unlikely(!self->rpit_obj)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_init(RepeatItem *__restrict self,
                size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":_SeqItemRepeat",
	                  &self->rpit_obj, &self->rpit_num))
		goto err;
	Dee_Incref(self->rpit_obj);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(Repeat, rp_seq) == offsetof(RepeatItem, rpit_obj));
#define repeatitem_fini  repeat_fini
#define repeatitem_visit repeat_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_bool(RepeatItem *__restrict self) {
	return likely(self->rpit_num) ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF RepeatItemIterator *DCALL
repeatitem_iter(RepeatItem *__restrict self) {
	DREF RepeatItemIterator *result;
	result = DeeObject_MALLOC(RepeatItemIterator);
	if unlikely(!result)
		goto done;
	result->rii_rep = self;
	result->rii_obj = self->rpit_obj;
	result->rii_num = self->rpit_num;
	Dee_Incref(self);
	DeeObject_Init(result, &SeqItemRepeatIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeatitem_contains(RepeatItem *self,
                    DeeObject *item) {
	return DeeObject_CmpEq(self->rpit_obj, item);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeatitem_size(RepeatItem *__restrict self) {
	if unlikely(self->rpit_num == (size_t)-1)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return self->rpit_num;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeatitem_size_fast(RepeatItem *__restrict self) {
	return self->rpit_num;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitem_getitem_index(RepeatItem *__restrict self, size_t index) {
	if unlikely(index >= self->rpit_num)
		goto err_bounds;
	return_reference_(self->rpit_obj);
err_bounds:
	err_index_out_of_bounds((DeeObject *)self, index, self->rpit_num);
/*err:*/
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
repeatitem_getitem_index_fast(RepeatItem *__restrict self, size_t index) {
	(void)index;
	return_reference_(self->rpit_obj);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitem_getrange_index(RepeatItem *__restrict self,
                          Dee_ssize_t i_begin,
                          Dee_ssize_t i_end) {
	struct Dee_seq_range range;
	DeeSeqRange_Clamp(&range, i_begin, i_end, self->rpit_num);
	return DeeSeq_RepeatItem(self->rpit_obj, range.sr_end - range.sr_start);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitem_getrange_index_n(RepeatItem *__restrict self,
                            Dee_ssize_t i_begin) {
	size_t start = DeeSeqRange_Clamp_n(i_begin, self->rpit_num);
	return DeeSeq_RepeatItem(self->rpit_obj, self->rpit_num - start);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeatitem_foreach(RepeatItem *self, Dee_foreach_t proc, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	for (i = 0; i < self->rpit_num; ++i) {
		temp = (*proc)(arg, self->rpit_obj);
		if unlikely(temp < 0)
			return temp;
		result += temp;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
repeatitem_foreach_pair(RepeatItem *self, Dee_foreach_pair_t proc, void *arg) {
	DREF DeeObject *pair[2];
	Dee_ssize_t temp, result = 0;
	if likely(self->rpit_num) {
		size_t i = 0;
		if (DeeObject_Unpack(self->rpit_obj, 2, pair))
			goto err;
		do {
			temp = (*proc)(arg, pair[0], pair[1]);
			if unlikely(temp < 0)
				goto err_pair_temp;
			result += temp;
		} while (++i < self->rpit_num);
		Dee_Decrefv(pair, 2);
	}
	return result;
err_pair_temp:
	Dee_Decrefv(pair, 2);
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeatitem_asvector(RepeatItem *self, /*out*/ DREF DeeObject **dst, size_t dst_length) {
	if unlikely(self->rpit_num == (size_t)-1) {
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	} else {
		if likely(dst_length >= self->rpit_num)
			Dee_Setrefv(dst, self->rpit_obj, self->rpit_num);
	}
	return self->rpit_num;
}


PRIVATE struct type_nsi tpconst repeatitem_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&repeatitem_size,
			/* .nsi_getsize_fast = */ (dfunptr_t)&repeatitem_size_fast,
			/* .nsi_getitem      = */ (dfunptr_t)&repeatitem_getitem_index,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)&repeatitem_getitem_index_fast,
			/* .nsi_getrange     = */ (dfunptr_t)&repeatitem_getrange_index,
			/* .nsi_getrange_n   = */ (dfunptr_t)&repeatitem_getrange_index_n,
		}
	}
};

PRIVATE struct type_seq repeatitem_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeatitem_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatitem_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ &repeatitem_nsi,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&repeatitem_foreach,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&repeatitem_foreach_pair,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&repeatitem_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&repeatitem_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&repeatitem_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&repeatitem_getitem_index_fast,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&repeatitem_getrange_index,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&repeatitem_getrange_index_n,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
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
	/* .tp_asvector                   = */ (size_t (DCALL *)(DeeObject *, DREF DeeObject **, size_t))&repeatitem_asvector,
};

PRIVATE struct type_member tpconst repeatitem_members[] = {
	TYPE_MEMBER_FIELD("__obj__", STRUCT_OBJECT, offsetof(RepeatItem, rpit_obj)),
	TYPE_MEMBER_FIELD("__num__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(RepeatItem, rpit_num)),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst repeatitem_getsets[] = {
	TYPE_GETTER(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst repeatitem_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqItemRepeatIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &SeqItemRepeat_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqItemRepeat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqItemRepeat",
	/* .tp_doc      = */ DOC("()\n"
	                         "(obj,num:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&repeatitem_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&repeatitem_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&repeatitem_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&repeatitem_init,
				TYPE_FIXED_ALLOCATOR(RepeatItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&repeatitem_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&repeatitem_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&repeatitem_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &repeatitem_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ repeatitem_getsets,
	/* .tp_members       = */ repeatitem_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ repeatitem_class_members
};



/* Construct new repetition-proxy-sequence objects. */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Repeat(DeeObject *__restrict self, size_t count) {
	DREF Repeat *result;
	if (!count || DeeFastSeq_GetSize_deprecated(self) == 0)
		return_reference_(Dee_EmptySeq);
	result = DeeObject_MALLOC(Repeat);
	if unlikely(!result)
		goto done;
	Dee_Incref(self);
	result->rp_seq = self;
	result->rp_num = count;
	DeeObject_Init(result, &SeqRepeat_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_RepeatItem(DeeObject *__restrict item, size_t count) {
	DREF RepeatItem *result;
	if unlikely(!count)
		return_empty_seq;
	result = DeeObject_MALLOC(RepeatItem);
	if unlikely(!result)
		goto done;
	Dee_Incref(item);
	result->rpit_obj = item;
	result->rpit_num = count;
	DeeObject_Init(result, &SeqItemRepeat_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_RepeatItemForever(DeeObject *__restrict item) {
	/* TODO: Dedicated sequence-proxy type */
	return DeeSeq_RepeatItem(item, (size_t)-1);
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_REPEAT_C */
