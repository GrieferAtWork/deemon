/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
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

#include <hybrid/atomic.h>
#include <hybrid/overflow.h>

#include "../../runtime/runtime_error.h"
#include "../gc_inspect.h"

DECL_BEGIN

#ifndef CONFIG_NO_THREADS
#define REPEATITER_READ_NUM(x) ATOMIC_READ((x)->ri_num)
#else /* !CONFIG_NO_THREADS */
#define REPEATITER_READ_NUM(x)            ((x)->ri_num)
#endif /* !CONFIG_NO_THREADS */


PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatiter_ctor(RepeatIterator *__restrict self) {
	self->ri_rep = (DREF Repeat *)DeeSeq_Repeat(Dee_EmptySeq, 1);
	if unlikely(!self->ri_rep)
		goto err;
	self->ri_iter = DeeObject_IterSelf(Dee_EmptySeq);
	if unlikely(!self->ri_iter) { Dee_Decref(self->ri_rep); }
	rwlock_init(&self->ri_lock);
	self->ri_num = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_copy(RepeatIterator *__restrict self,
                RepeatIterator *__restrict other) {
	DREF DeeObject *copy;
	rwlock_read(&other->ri_lock);
	self->ri_num  = other->ri_num;
	self->ri_iter = other->ri_iter;
	Dee_Incref(self->ri_iter);
	rwlock_endread(&other->ri_lock);
	copy = DeeObject_Copy(self->ri_iter);
	Dee_Decref(self->ri_iter);
	if unlikely(!copy)
		goto err;
	self->ri_iter = copy;
	self->ri_rep  = other->ri_rep;
	Dee_Incref(self->ri_rep);
	rwlock_init(&self->ri_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_deep(RepeatIterator *__restrict self,
                RepeatIterator *__restrict other) {
	DREF DeeObject *copy;
	rwlock_read(&other->ri_lock);
	self->ri_num  = other->ri_num;
	self->ri_iter = other->ri_iter;
	Dee_Incref(self->ri_iter);
	rwlock_endread(&other->ri_lock);
	copy = DeeObject_DeepCopy(self->ri_iter);
	Dee_Decref(self->ri_iter);
	if unlikely(!copy)
		goto err;
	self->ri_iter = copy;
	self->ri_rep  = (DREF Repeat *)DeeObject_DeepCopy((DeeObject *)other->ri_rep);
	if unlikely(!self->ri_rep)
		goto err_iter;
	rwlock_init(&self->ri_lock);
	return 0;
err_iter:
	Dee_Decref(self->ri_iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatiter_init(RepeatIterator *__restrict self,
                size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqRepeatIterator", &self->ri_rep))
		goto err;
	if (DeeObject_AssertTypeExact(self->ri_rep, &SeqRepeat_Type))
		goto err;
	self->ri_iter = DeeObject_IterSelf(self->ri_rep->r_seq);
	if unlikely(!self->ri_iter)
		goto err;
	Dee_Incref(self->ri_rep);
	rwlock_init(&self->ri_lock);
	self->ri_num = self->ri_rep->r_num - 1;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
repeatiter_fini(RepeatIterator *__restrict self) {
	Dee_Decref(self->ri_rep);
	Dee_Decref(self->ri_iter);
}

PRIVATE NONNULL((1, 2)) void DCALL
repeatiter_visit(RepeatIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ri_rep);
	Dee_Visit(self->ri_iter);
}


#define DEFINE_REPEATITER_CMP(name, check_diffnum, if_sameiter, compare_iter)       \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                           \
	name(RepeatIterator *self, RepeatIterator *other) {                             \
		DREF DeeObject *my_iter, *ot_iter;                                          \
		DREF DeeObject *result;                                                     \
		if (DeeObject_AssertTypeExact(other, &SeqRepeatIterator_Type)) \
			return NULL;                                                            \
		check_diffnum;                                                              \
		rwlock_read(&self->ri_lock);                                                \
		my_iter = self->ri_iter;                                                    \
		Dee_Incref(my_iter);                                                        \
		rwlock_endread(&self->ri_lock);                                             \
		rwlock_read(&other->ri_lock);                                               \
		ot_iter = other->ri_iter;                                                   \
		Dee_Incref(ot_iter);                                                        \
		rwlock_endread(&other->ri_lock);                                            \
		if (my_iter == ot_iter)                                                     \
			result = if_sameiter, Dee_Incref(if_sameiter);                          \
		else {                                                                      \
			result = compare_iter(my_iter, ot_iter);                                \
		}                                                                           \
		Dee_Decref(ot_iter);                                                        \
		Dee_Decref(my_iter);                                                        \
		return result;                                                              \
	}

DEFINE_REPEATITER_CMP(
repeatiter_eq,
{
	if (REPEATITER_READ_NUM(self) != REPEATITER_READ_NUM(other))
		return_false;
},
Dee_True, DeeObject_CompareEqObject)

DEFINE_REPEATITER_CMP(
repeatiter_ne,
{
	if (REPEATITER_READ_NUM(self) != REPEATITER_READ_NUM(other))
		return_true;
},
Dee_False, DeeObject_CompareNeObject)

DEFINE_REPEATITER_CMP(
repeatiter_lo, {
	size_t my_len = REPEATITER_READ_NUM(self);
	size_t ot_len = REPEATITER_READ_NUM(other);
	if (my_len != ot_len)
		return_bool_(ot_len < my_len);
},
Dee_False, DeeObject_CompareLoObject)

DEFINE_REPEATITER_CMP(
repeatiter_le, {
	size_t my_len = REPEATITER_READ_NUM(self);
	size_t ot_len = REPEATITER_READ_NUM(other);
	if (my_len != ot_len)
		return_bool_(ot_len < my_len);
},
Dee_True, DeeObject_CompareLeObject)

DEFINE_REPEATITER_CMP(
repeatiter_gr, {
	size_t my_len = REPEATITER_READ_NUM(self);
	size_t ot_len = REPEATITER_READ_NUM(other);
	if (my_len != ot_len)
		return_bool_(ot_len > my_len);
},
Dee_False, DeeObject_CompareGrObject)

DEFINE_REPEATITER_CMP(
repeatiter_ge, {
	size_t my_len = REPEATITER_READ_NUM(self);
	size_t ot_len = REPEATITER_READ_NUM(other);
	if (my_len != ot_len)
		return_bool_(ot_len > my_len);
},
Dee_True, DeeObject_CompareGeObject)
#undef DEFINE_REPEATITER_CMP


PRIVATE struct type_cmp repeatiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatiter_ge
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatiter_next(RepeatIterator *__restrict self) {
	DREF DeeObject *result, *iter;
	rwlock_read(&self->ri_lock);
	iter = self->ri_iter;
	Dee_Incref(iter);
	rwlock_endread(&self->ri_lock);
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
	iter = DeeObject_IterSelf(self->ri_rep->r_seq);
	if unlikely(!iter)
		goto err;
	COMPILER_READ_BARRIER();
	rwlock_write(&self->ri_lock);
	if unlikely(!self->ri_num) {
		rwlock_endwrite(&self->ri_lock);
		Dee_Decref(iter);
		ASSERT(result == ITER_DONE);
		goto done;
	}
	result = self->ri_iter;
	Dee_Incref(iter);
	self->ri_iter = iter;
	--self->ri_num; /* Decrement the number of remaining loops. */
	rwlock_endwrite(&self->ri_lock);
	Dee_Decref(result); /* Drop the old iterator. */
	goto again_iter;    /* Read more items. */
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatiter_get_iter(RepeatIterator *__restrict self) {
	DREF DeeObject *result;
	rwlock_read(&self->ri_lock);
	result = self->ri_iter;
	Dee_Incref(result);
	rwlock_endread(&self->ri_lock);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_set_iter(RepeatIterator *__restrict self,
                    DeeObject *__restrict value) {
	DREF DeeObject *oldvalue;
	if (DeeGC_ReferredBy(value, (DeeObject *)self))
		return err_reference_loop((DeeObject *)self, value);
	Dee_Incref(value);
	rwlock_write(&self->ri_lock);
	oldvalue      = self->ri_iter;
	self->ri_iter = value;
	rwlock_endwrite(&self->ri_lock);
	Dee_Decref(oldvalue);
	return 0;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatiter_get_num(RepeatIterator *__restrict self) {
	size_t result;
	rwlock_read(&self->ri_lock);
	result = self->ri_num;
	rwlock_endread(&self->ri_lock);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatiter_set_num(RepeatIterator *__restrict self,
                   DeeObject *__restrict value) {
	size_t newvalue;
	if (DeeObject_AsSize(value, &newvalue))
		goto err;
	rwlock_write(&self->ri_lock);
	self->ri_num = newvalue;
	rwlock_endwrite(&self->ri_lock);
	return 0;
err:
	return -1;
}


PRIVATE struct type_getset tpconst repeatiter_getsets[] = {
	{ "__iter__",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeatiter_get_iter, NULL,
	  (int (DCALL *)(DeeObject *, DeeObject *))&repeatiter_set_iter,
	  DOC("->?DIterator") },
	{ "__num__",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeatiter_get_num, NULL,
	  (int (DCALL *)(DeeObject *, DeeObject *))&repeatiter_set_num,
	  DOC("->?Dint") },
	{ NULL }
};

PRIVATE struct type_member tpconst repeatiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(RepeatIterator, ri_rep), "->?Ert:SeqRepeat"),
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
	self->r_num = 1;
	self->r_seq = Dee_EmptySeq;
	Dee_Incref(Dee_EmptySeq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeat_copy(Repeat *__restrict self,
            Repeat *__restrict other) {
	self->r_num = other->r_num;
	self->r_seq = other->r_seq;
	Dee_Incref(self->r_seq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeat_deep(Repeat *__restrict self,
            Repeat *__restrict other) {
	self->r_seq = DeeObject_DeepCopy(other->r_seq);
	if unlikely(!self->r_seq)
		goto err;
	self->r_num = other->r_num;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_init(Repeat *__restrict self,
            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "oIu:_SeqRepeat", &self->r_seq, &self->r_num))
		goto err;
	if unlikely(!self->r_num) {
		self->r_seq = Dee_EmptySeq;
		self->r_num = 1;
	}
	Dee_Incref(self->r_seq);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
repeat_fini(Repeat *__restrict self) {
	Dee_Decref(self->r_seq);
}

PRIVATE NONNULL((1, 2)) void DCALL
repeat_visit(Repeat *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->r_seq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeat_bool(Repeat *__restrict self) {
	return DeeObject_Bool(self->r_seq);
}

PRIVATE WUNUSED NONNULL((1)) DREF RepeatIterator *DCALL
repeat_iter(Repeat *__restrict self) {
	DREF RepeatIterator *result;
	result = DeeObject_MALLOC(RepeatIterator);
	if unlikely(!result)
		goto done;
	result->ri_iter = DeeObject_IterSelf(self->r_seq);
	if unlikely(!result->ri_iter)
		goto err_r;
	result->ri_rep = self;
	result->ri_num = self->r_num - 1;
	rwlock_init(&result->ri_lock);
	Dee_Incref(self);
	DeeObject_Init(result, &SeqRepeatIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeat_size(Repeat *__restrict self) {
	size_t base_size;
	size_t result;
	base_size = DeeObject_Size(self->r_seq);
	if unlikely(base_size == (size_t)-1)
		goto err;
	if (OVERFLOW_UMUL(base_size, self->r_num, &result))
		goto err_overflow;
	return DeeInt_NewSize(result);
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeat_contains(Repeat *self,
                DeeObject *item) {
	return DeeObject_ContainsObject(self->r_seq, item);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeat_get(Repeat *self,
           DeeObject *index_ob) {
	size_t index;
	size_t seq_size;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	seq_size = DeeObject_Size(self->r_seq);
	if unlikely(seq_size == (size_t)-1)
		goto err;
	if unlikely(index >= seq_size * self->r_num) {
		err_index_out_of_bounds((DeeObject *)self, index,
		                        seq_size * self->r_num);
		goto err;
	}
	index %= seq_size;
	return DeeObject_GetItemIndex(self->r_seq, index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeat_nsi_getsize(Repeat *__restrict self) {
	size_t base_size;
	size_t result;
	base_size = DeeObject_Size(self->r_seq);
	if unlikely(base_size == (size_t)-1)
		return (size_t)-1;
	if (OVERFLOW_UMUL(base_size, self->r_num, &result)) {
		err_integer_overflow_i(sizeof(size_t) * 8, true);
		return (size_t)-1;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeat_nsi_getsize_fast(Repeat *__restrict self) {
	size_t base_size;
	size_t result;
	base_size = DeeFastSeq_GetSize(self->r_seq);
	if unlikely(base_size == (size_t)-1)
		return (size_t)-1;
	if (OVERFLOW_UMUL(base_size, self->r_num, &result))
		result = (size_t)-1;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeat_nsi_getitem(Repeat *__restrict self, size_t index) {
	size_t seq_size;
	seq_size = DeeObject_Size(self->r_seq);
	if unlikely(seq_size == (size_t)-1)
		goto err;
	if unlikely(index >= seq_size * self->r_num) {
		err_index_out_of_bounds((DeeObject *)self, index,
		                        seq_size * self->r_num);
		goto err;
	}
	index %= seq_size;
	return DeeObject_GetItemIndex(self->r_seq, index);
err:
	return NULL;
}

PRIVATE size_t DCALL
repeat_nsi_find(Repeat *__restrict self, size_t start, size_t end,
                DeeObject *__restrict keyed_search_item, DeeObject *key) {
	return DeeSeq_Find(self->r_seq, start, end, keyed_search_item, key);
}

PRIVATE size_t DCALL
repeat_nsi_rfind(Repeat *__restrict self, size_t start, size_t end,
                 DeeObject *__restrict keyed_search_item, DeeObject *key) {
	size_t result;
	result = DeeSeq_RFind(self->r_seq, start, end, keyed_search_item, key);
	if (result != (size_t)-1 && result != (size_t)-2) {
		size_t inner_size = DeeObject_Size(self->r_seq);
		size_t addend;
		if unlikely(inner_size == (size_t)-1)
			return (size_t)-2;
		if (self->r_num > 1 && inner_size != 0) {
			if (OVERFLOW_UMUL(self->r_num - 1, inner_size, &addend)) {
				err_integer_overflow_i(sizeof(size_t) * 8, true);
				return (size_t)-2;
			}
			if unlikely(result == (size_t)-2)
				err_integer_overflow_i(sizeof(size_t) * 8, true);
		}
	}
	return result;
}



PRIVATE struct type_nsi tpconst repeat_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&repeat_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)&repeat_nsi_getsize_fast,
			/* .nsi_getitem      = */ (dfunptr_t)&repeat_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)&repeat_nsi_find,
			/* .nsi_rfind        = */ (dfunptr_t)&repeat_nsi_rfind,
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


PRIVATE struct type_seq repeat_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeat_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeat_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeat_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeat_get,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_getrange  = */ NULL,
	/* .tp_delrange  = */ NULL,
	/* .tp_setrange  = */ NULL,
	/* .tp_nsi       = */ &repeat_nsi
};

PRIVATE struct type_member tpconst repeat_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(Repeat, r_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__num__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(Repeat, r_num)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst repeat_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SeqRepeatIterator_Type),
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
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ repeat_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ repeat_class_members
};












#ifndef CONFIG_NO_THREADS
#define REPEATITEMPITER_READ_NUM(x) ATOMIC_READ((x)->rii_num)
#else /* !CONFIG_NO_THREADS */
#define REPEATITEMPITER_READ_NUM(x)            ((x)->rii_num)
#endif /* CONFIG_NO_THREADS */


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
	self->rii_obj = self->rii_rep->ri_obj;
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
	self->rii_obj = self->rii_rep->ri_obj;
	self->rii_num = self->rii_rep->ri_num;
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeatitemiter_eq(RepeatItemIterator *self,
                  RepeatItemIterator *other) {
	if (DeeObject_AssertTypeExact(other, &SeqItemRepeatIterator_Type))
		goto err;
	if (REPEATITEMPITER_READ_NUM(self) != REPEATITEMPITER_READ_NUM(other))
		return_false;
	return DeeObject_CompareEqObject(self->rii_obj, other->rii_obj);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeatitemiter_ne(RepeatItemIterator *self,
                  RepeatItemIterator *other) {
	if (DeeObject_AssertTypeExact(other, &SeqItemRepeatIterator_Type))
		goto err;
	if (REPEATITEMPITER_READ_NUM(self) != REPEATITEMPITER_READ_NUM(other))
		return_true;
	return DeeObject_CompareNeObject(self->rii_obj, other->rii_obj);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeatitemiter_lo(RepeatItemIterator *self,
                  RepeatItemIterator *other) {
	size_t my_num, ot_num;
	if (DeeObject_AssertTypeExact(other, &SeqItemRepeatIterator_Type))
		goto err;
	my_num = REPEATITEMPITER_READ_NUM(self);
	ot_num = REPEATITEMPITER_READ_NUM(other);
	if (my_num != ot_num)
		return_bool_(my_num < ot_num);
	return DeeObject_CompareLoObject(self->rii_obj, other->rii_obj);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeatitemiter_le(RepeatItemIterator *self,
                  RepeatItemIterator *other) {
	size_t my_num, ot_num;
	if (DeeObject_AssertTypeExact(other, &SeqItemRepeatIterator_Type))
		goto err;
	my_num = REPEATITEMPITER_READ_NUM(self);
	ot_num = REPEATITEMPITER_READ_NUM(other);
	if (my_num != ot_num)
		return_bool_(my_num < ot_num);
	return DeeObject_CompareLeObject(self->rii_obj, other->rii_obj);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeatitemiter_gr(RepeatItemIterator *self,
                  RepeatItemIterator *other) {
	size_t my_num, ot_num;
	if (DeeObject_AssertTypeExact(other, &SeqItemRepeatIterator_Type))
		goto err;
	my_num = REPEATITEMPITER_READ_NUM(self);
	ot_num = REPEATITEMPITER_READ_NUM(other);
	if (my_num != ot_num)
		return_bool_(my_num > ot_num);
	return DeeObject_CompareGrObject(self->rii_obj, other->rii_obj);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeatitemiter_ge(RepeatItemIterator *self,
                  RepeatItemIterator *other) {
	size_t my_num, ot_num;
	if (DeeObject_AssertTypeExact(other, &SeqItemRepeatIterator_Type))
		goto err;
	my_num = REPEATITEMPITER_READ_NUM(self);
	ot_num = REPEATITEMPITER_READ_NUM(other);
	if (my_num != ot_num)
		return_bool_(my_num > ot_num);
	return DeeObject_CompareGeObject(self->rii_obj, other->rii_obj);
err:
	return NULL;
}


PRIVATE struct type_cmp repeatitemiter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatitemiter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatitemiter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatitemiter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatitemiter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatitemiter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatitemiter_ge
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitemiter_next(RepeatItemIterator *__restrict self) {
	size_t count;
#ifndef CONFIG_NO_THREADS
	do {
		if ((count = ATOMIC_READ(self->rii_num)) == 0)
			return ITER_DONE;
	} while (!ATOMIC_CMPXCH_WEAK(self->ob_refcnt, count, count - 1));
#else /* !CONFIG_NO_THREADS */
	if (!self->rii_num)
		return ITER_DONE;
	--self->rii_num;
#endif /* CONFIG_NO_THREADS */
	return_reference_(self->rii_obj);
}

PRIVATE struct type_member tpconst repeatitemiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(RepeatItemIterator, rii_rep), "->?Ert:SeqItemRepeat"),
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
	self->ri_num = 1;
	self->ri_obj = Dee_None;
	Dee_Incref(Dee_None);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitem_copy(RepeatItem *__restrict self,
                RepeatItem *__restrict other) {
	self->ri_num = other->ri_num;
	self->ri_obj = other->ri_obj;
	Dee_Incref(self->ri_obj);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
repeatitem_deep(RepeatItem *__restrict self,
                RepeatItem *__restrict other) {
	self->ri_num = other->ri_num;
	self->ri_obj = DeeObject_DeepCopy(other->ri_obj);
	if unlikely(!self->ri_obj)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_init(RepeatItem *__restrict self,
                size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "oIu:_SeqItemRepeat", &self->ri_obj, &self->ri_num))
		goto err;
	if unlikely(!self->ri_num)
		self->ri_obj = Dee_None;
	Dee_Incref(self->ri_obj);
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(Repeat, r_seq) == offsetof(RepeatItem, ri_obj));
#define repeatitem_fini  repeat_fini
#define repeatitem_visit repeat_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
repeatitem_bool(RepeatItem *__restrict self) {
	return self->ri_num ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF RepeatItemIterator *DCALL
repeatitem_iter(RepeatItem *__restrict self) {
	DREF RepeatItemIterator *result;
	result = DeeObject_MALLOC(RepeatItemIterator);
	if unlikely(!result)
		goto done;
	result->rii_rep = self;
	result->rii_obj = self->ri_obj;
	result->rii_num = self->ri_num;
	Dee_Incref(self);
	DeeObject_Init(result, &SeqItemRepeatIterator_Type);
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitem_size(RepeatItem *__restrict self) {
	return DeeInt_NewSize(self->ri_num);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeatitem_contains(RepeatItem *self,
                    DeeObject *item) {
	return DeeObject_CompareEqObject(self->ri_obj, item);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
repeatitem_get(RepeatItem *self,
               DeeObject *index_ob) {
	dssize_t index;
	if (DeeObject_AsSSize(index_ob, &index))
		goto err;
	if (index < 0)
		index += self->ri_num;
	if unlikely((size_t)index >= self->ri_num) {
		err_index_out_of_bounds((DeeObject *)self,
		                        (size_t)index,
		                        self->ri_num);
	}
	return_reference_(self->ri_obj);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
repeatitem_getrange(RepeatItem *self,
                    DeeObject *start_ob,
                    DeeObject *end_ob) {
	dssize_t start, end;
	if (DeeObject_AsSSize(start_ob, &start))
		goto err;
	if (DeeNone_Check(end_ob))
		end = self->ri_num;
	else if (DeeObject_AsSSize(end_ob, &end))
		goto err;
	if unlikely(start < 0)
		start += self->ri_num;
	if unlikely(end < 0)
		end += self->ri_num;
	if unlikely((size_t)start >= self->ri_num ||
		         (size_t)start >= (size_t)end)
	return_reference_(Dee_EmptySeq);
	if unlikely((size_t)end > self->ri_num)
		end = (dssize_t)self->ri_num;
	end -= start;
	ASSERT(end != 0);
	return DeeSeq_RepeatItem(self->ri_obj, (size_t)end);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeatitem_nsi_getsize(RepeatItem *__restrict self) {
	if unlikely(self->ri_num == (size_t)-1)
		err_integer_overflow_i(sizeof(size_t) * 8, true);
	return self->ri_num;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
repeatitem_nsi_getsize_fast(RepeatItem *__restrict self) {
	return self->ri_num;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitem_nsi_getitem(RepeatItem *__restrict self, size_t index) {
	if unlikely(index >= self->ri_num) {
		err_index_out_of_bounds((DeeObject *)self, index, self->ri_num);
		return NULL;
	}
	return_reference_(self->ri_obj);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
repeatitem_nsi_getitem_fast(RepeatItem *__restrict self, size_t UNUSED(index)) {
	return_reference_(self->ri_obj);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitem_nsi_getrange(RepeatItem *__restrict self,
                        dssize_t start,
                        dssize_t end) {
	if unlikely(start < 0)
		start += self->ri_num;
	if unlikely(end < 0)
		end += self->ri_num;
	if unlikely((size_t)start >= self->ri_num ||
		         (size_t)start >= (size_t)end)
	return_reference_(Dee_EmptySeq);
	if unlikely((size_t)end > self->ri_num)
		end = (dssize_t)self->ri_num;
	end -= start;
	ASSERT(end != 0);
	return DeeSeq_RepeatItem(self->ri_obj, (size_t)end);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
repeatitem_nsi_getrange_n(RepeatItem *__restrict self,
                          dssize_t start) {
	if unlikely(start < 0)
		start += self->ri_num;
	if unlikely((size_t)start >= self->ri_num)
		return_reference_(Dee_EmptySeq);
	ASSERT(self->ri_num != 0);
	return DeeSeq_RepeatItem(self->ri_obj,
	                         self->ri_num - (size_t)start);
}

PRIVATE size_t DCALL
repeatitem_nsi_find(RepeatItem *__restrict self,
                    size_t start, size_t end,
                    DeeObject *__restrict keyed_search_item,
                    DeeObject *key) {
	int error;
	if (start >= self->ri_num || start >= end)
		return (size_t)-1;
	error = DeeObject_CompareKeyEq(self->ri_obj, keyed_search_item, key);
	if unlikely(error < 0)
		return (size_t)-2;
	if (!error)
		return (size_t)-1;
	return start;
}

PRIVATE size_t DCALL
repeatitem_nsi_rfind(RepeatItem *__restrict self,
                     size_t start, size_t end,
                     DeeObject *__restrict keyed_search_item,
                     DeeObject *key) {
	size_t result;
	result = repeatitem_nsi_find(self, start, end, keyed_search_item, key);
	if (result != (size_t)-1 && result != (size_t)-2) {
		if (end > self->ri_num)
			end = self->ri_num;
		result = end - 1;
	}
	return result;
}


PRIVATE struct type_nsi tpconst repeatitem_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&repeatitem_nsi_getsize,
			/* .nsi_getsize_fast = */ (dfunptr_t)&repeatitem_nsi_getsize_fast,
			/* .nsi_getitem      = */ (dfunptr_t)&repeatitem_nsi_getitem,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)&repeatitem_nsi_getitem_fast,
			/* .nsi_getrange     = */ (dfunptr_t)&repeatitem_nsi_getrange,
			/* .nsi_getrange_n   = */ (dfunptr_t)&repeatitem_nsi_getrange_n,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)&repeatitem_nsi_find,
			/* .nsi_rfind        = */ (dfunptr_t)&repeatitem_nsi_rfind,
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

PRIVATE struct type_seq repeatitem_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeatitem_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&repeatitem_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatitem_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&repeatitem_get,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_getrange  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&repeatitem_getrange,
	/* .tp_delrange  = */ NULL,
	/* .tp_setrange  = */ NULL,
	/* .tp_nsi       = */ &repeatitem_nsi
};

PRIVATE struct type_member tpconst repeatitem_members[] = {
	TYPE_MEMBER_FIELD("__obj__", STRUCT_OBJECT, offsetof(RepeatItem, ri_obj)),
	TYPE_MEMBER_FIELD("__num__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(RepeatItem, ri_num)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst repeatitem_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SeqItemRepeatIterator_Type),
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
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ repeatitem_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ repeatitem_class_members
};



/* Construct new repetition-proxy-sequence objects. */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Repeat(DeeObject *__restrict self, size_t count) {
	DREF Repeat *result;
	if (!count || DeeFastSeq_GetSize(self) == 0)
		return_reference_(Dee_EmptySeq);
	result = DeeObject_MALLOC(Repeat);
	if unlikely(!result)
		goto done;
	Dee_Incref(self);
	result->r_seq = self;
	result->r_num = count;
	DeeObject_Init(result, &SeqRepeat_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_RepeatItem(DeeObject *__restrict item, size_t count) {
	DREF RepeatItem *result;
	if (!count)
		return_reference_(Dee_EmptySeq);
	result = DeeObject_MALLOC(RepeatItem);
	if unlikely(!result)
		goto done;
	Dee_Incref(item);
	result->ri_obj = item;
	result->ri_num = count;
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
