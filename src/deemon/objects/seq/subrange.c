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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SUBRANGE_C
#define GUARD_DEEMON_OBJECTS_SEQ_SUBRANGE_C 1

#include "subrange.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/thread.h>

#include <hybrid/minmax.h>
#include <hybrid/overflow.h>

#ifndef CONFIG_NO_THREADS
#include <hybrid/atomic.h>
#endif /* !CONFIG_NO_THREADS */

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) int DCALL
subrangeiterator_init(SubRangeIterator *__restrict self,
                      size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o|IuIu:_SubRangeIterator",
	                  &self->sr_iter,
	                  &self->sr_start,
	                  &self->sr_size))
		goto err;
	Dee_Incref(self->sr_iter);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
subrangeiterator_fini(SubRangeIterator *__restrict self) {
	Dee_Decref(self->sr_iter);
}

PRIVATE NONNULL((1, 2)) void DCALL
subrangeiterator_visit(SubRangeIterator *__restrict self,
                       dvisit_t proc, void *arg) {
	Dee_Visit(self->sr_iter);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
subrangeiterator_next(SubRangeIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	if (!self->sr_size)
		return ITER_DONE;
	--self->sr_size;
#else /* CONFIG_NO_THREADS */
	size_t oldval;
	/* Consume one from the max-iteration size. */
	do {
		oldval = ATOMIC_READ(self->sr_size);
		if (!oldval)
			return ITER_DONE;
	} while (!ATOMIC_CMPXCH(self->sr_size, oldval, oldval - 1));
#endif /* !CONFIG_NO_THREADS */
	return DeeObject_IterNext(self->sr_iter);
}

INTDEF DeeTypeObject SeqSubRangeIterator_Type;

#define DEFINE_COMPARE(name, op)                                                      \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL                             \
	name(SubRangeIterator *self, SubRangeIterator *other) {                           \
		if (DeeObject_AssertTypeExact((DeeObject *)other, &SeqSubRangeIterator_Type)) \
			return NULL;                                                              \
		return_bool(READ_SIZE(other) op READ_SIZE(self));                             \
	}
DEFINE_COMPARE(subrangeiterator_eq, ==)
DEFINE_COMPARE(subrangeiterator_ne, !=)
DEFINE_COMPARE(subrangeiterator_lo, <)
DEFINE_COMPARE(subrangeiterator_le, <=)
DEFINE_COMPARE(subrangeiterator_gr, >)
DEFINE_COMPARE(subrangeiterator_ge, >=)
#undef DEFINE_COMPARE

PRIVATE struct type_cmp subrangeiterator_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&subrangeiterator_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&subrangeiterator_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&subrangeiterator_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&subrangeiterator_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&subrangeiterator_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&subrangeiterator_ge
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
subrangeiterator_seq_get(SubRangeIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *base_seq;
	size_t end;
	base_seq = DeeObject_GetAttr(self->sr_iter, &str_seq);
	if unlikely(!base_seq)
		goto err;
	if (OVERFLOW_UADD(self->sr_start, self->sr_size, &end)) {
		result = DeeSeq_GetRangeN(base_seq, self->sr_start);
	} else {
		result = DeeSeq_GetRange(base_seq, self->sr_start, end);
	}
	Dee_Decref(base_seq);
	return result;
err:
	return NULL;
}

PRIVATE struct type_getset subrangeiterator_getsets[] = {
	{ DeeString_STR(&str_seq),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&subrangeiterator_seq_get,
	  NULL, NULL,
	  DOC("->?X2?Ert:SeqSubRange?Ert:SeqSubRangeN") },
	{ NULL }
};

PRIVATE struct type_member subrangeiterator_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(SubRangeIterator, sr_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_SIZE_T, offsetof(SubRangeIterator, sr_start)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_SIZE_T, offsetof(SubRangeIterator, sr_size)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
subrangeiterator_ctor(SubRangeIterator *__restrict self) {
	self->sr_start = 0;
	self->sr_size  = 0;
	self->sr_iter  = Dee_None;
	Dee_Incref(Dee_None);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
subrangeiterator_copy(SubRangeIterator *__restrict self,
                      SubRangeIterator *__restrict other) {
	self->sr_start = other->sr_start;
	self->sr_size  = READ_SIZE(other);
	self->sr_iter  = DeeObject_Copy(other->sr_iter);
	if unlikely(!self->sr_iter)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
subrangeiterator_deep(SubRangeIterator *__restrict self,
                      SubRangeIterator *__restrict other) {
	self->sr_start = other->sr_start;
	self->sr_size  = READ_SIZE(other);
	self->sr_iter  = DeeObject_DeepCopy(other->sr_iter);
	if unlikely(!self->sr_iter)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
subrangeiterator_bool(SubRangeIterator *__restrict self) {
	if (!READ_SIZE(self))
		return 0;
	return DeeObject_Bool(self->sr_iter);
}

INTERN DeeTypeObject SeqSubRangeIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqSubRangeIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&subrangeiterator_ctor,
				/* .tp_copy_ctor = */ (void *)&subrangeiterator_copy,
				/* .tp_deep_ctor = */ (void *)&subrangeiterator_deep,
				/* .tp_any_ctor  = */ (void *)&subrangeiterator_init,
				TYPE_FIXED_ALLOCATOR(SubRangeIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&subrangeiterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&subrangeiterator_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&subrangeiterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &subrangeiterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&subrangeiterator_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ subrangeiterator_getsets,
	/* .tp_members       = */ subrangeiterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};




PRIVATE NONNULL((1)) void DCALL
subrange_fini(SubRange *__restrict self) {
	Dee_Decref(self->sr_seq);
}

PRIVATE NONNULL((1, 2)) void DCALL
subrange_visit(SubRange *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->sr_seq);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
subrange_size(SubRange *__restrict self) {
	size_t inner_size = DeeObject_Size(self->sr_seq);
	if unlikely(inner_size == (size_t)-1)
		goto err;
	if (self->sr_start >= inner_size)
		return_reference_(&DeeInt_Zero);
	inner_size -= self->sr_start;
	if (inner_size > self->sr_size)
		inner_size = self->sr_size;
	return DeeInt_NewSize(inner_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
subrange_iter(SubRange *__restrict self) {
	DREF SubRangeIterator *result;
	DREF DeeObject *iterator;
	size_t begin_index;
	iterator = DeeObject_IterSelf(self->sr_seq);
	if unlikely(!iterator)
		goto err;
	result = DeeObject_MALLOC(SubRangeIterator);
	if unlikely(!result)
		goto err_iterator;
	begin_index      = self->sr_start;
	result->sr_start = begin_index;
	result->sr_size  = self->sr_size;
	result->sr_iter  = iterator; /* Inherit reference. */
	/* Discard leading items. */
	while (begin_index--) {
		DREF DeeObject *discard;
		discard = DeeObject_IterNext(iterator);
		if unlikely(!ITER_ISOK(discard)) {
			if (!discard)
				goto err_iterator_r;
			/* Empty iterator (the base iterator was exhausted during the discard-phase) */
			result->sr_size = 0;
			break;
		}
		Dee_Decref(discard);
		if (DeeThread_CheckInterrupt())
			goto err_iterator_r;
	}
	DeeObject_Init(result, &SeqSubRangeIterator_Type);
	return (DREF DeeObject *)result;
err_iterator_r:
	DeeObject_FREE(result);
err_iterator:
	Dee_Decref(iterator);
err:
	return NULL;
}

PRIVATE struct type_member subrange_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(SubRange, sr_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(SubRange, sr_start)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(SubRange, sr_size)),
	TYPE_MEMBER_END
};


PRIVATE struct type_member subrange_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SeqSubRangeIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
subrange_getitem(SubRange *self,
                 DeeObject *index_ob) {
	size_t index;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	return DeeObject_GetItemIndex(self->sr_seq, self->sr_start + index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
subrange_nsi_getsize(SubRange *__restrict self) {
	size_t inner_size = DeeObject_Size(self->sr_seq);
	if unlikely(inner_size != (size_t)-1) {
		if (self->sr_start >= inner_size)
			return 0;
		inner_size -= self->sr_start;
		if (inner_size > self->sr_size)
			inner_size = self->sr_size;
	}
	return inner_size;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
subrange_nsi_getsize_fast(SubRange *__restrict self) {
	size_t inner_size = DeeFastSeq_GetSize(self->sr_seq);
	if unlikely(inner_size != (size_t)-1) {
		if (self->sr_start >= inner_size)
			return 0;
		inner_size -= self->sr_start;
		if (inner_size > self->sr_size)
			inner_size = self->sr_size;
	}
	return inner_size;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
subrange_nsi_getitem(SubRange *__restrict self, size_t index) {
	return DeeObject_GetItemIndex(self->sr_seq,
	                              self->sr_start + index);
}

PRIVATE size_t DCALL
subrange_nsi_find(SubRange *__restrict self, size_t start, size_t end,
                  DeeObject *__restrict keyed_search_item,
                  DeeObject *key) {
	if (start >= self->sr_size)
		return (size_t)-1;
	if (end > self->sr_size)
		end = self->sr_size;
	return DeeSeq_Find(self->sr_seq,
	                   start + self->sr_start,
	                   end + self->sr_start,
	                   keyed_search_item,
	                   key);
}

PRIVATE size_t DCALL
subrange_nsi_rfind(SubRange *__restrict self, size_t start, size_t end,
                   DeeObject *__restrict keyed_search_item, DeeObject *key) {
	if (start >= self->sr_size)
		return (size_t)-1;
	if (end > self->sr_size)
		end = self->sr_size;
	return DeeSeq_RFind(self->sr_seq,
	                    start + self->sr_start,
	                    end + self->sr_start,
	                    keyed_search_item,
	                    key);
}


PRIVATE struct type_nsi subrange_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&subrange_nsi_getsize,
			/* .nsi_getsize_fast = */ (void *)&subrange_nsi_getsize_fast,
			/* .nsi_getitem      = */ (void *)&subrange_nsi_getitem,
			/* .nsi_delitem      = */ (void *)NULL,
			/* .nsi_setitem      = */ (void *)NULL,
			/* .nsi_getitem_fast = */ (void *)NULL,
			/* .nsi_getrange     = */ (void *)NULL,
			/* .nsi_getrange_n   = */ (void *)NULL,
			/* .nsi_setrange     = */ (void *)NULL,
			/* .nsi_setrange_n   = */ (void *)NULL,
			/* .nsi_find         = */ (void *)&subrange_nsi_find,
			/* .nsi_rfind        = */ (void *)&subrange_nsi_rfind,
			/* .nsi_xch          = */ (void *)NULL,
			/* .nsi_insert       = */ (void *)NULL,
			/* .nsi_insertall    = */ (void *)NULL,
			/* .nsi_insertvec    = */ (void *)NULL,
			/* .nsi_pop          = */ (void *)NULL,
			/* .nsi_erase        = */ (void *)NULL,
			/* .nsi_remove       = */ (void *)NULL,
			/* .nsi_rremove      = */ (void *)NULL,
			/* .nsi_removeall    = */ (void *)NULL,
			/* .nsi_removeif     = */ (void *)NULL
		}
	}
};

PRIVATE struct type_seq subrange_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&subrange_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&subrange_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&subrange_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &subrange_nsi
};


PRIVATE WUNUSED NONNULL((1)) int DCALL
subrange_ctor(SubRange *__restrict self) {
	self->sr_seq   = Dee_EmptySeq;
	self->sr_start = 0;
	self->sr_size  = 0;
	Dee_Incref(Dee_EmptySeq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
subrange_copy(SubRange *__restrict self,
              SubRange *__restrict other) {
	self->sr_seq   = other->sr_seq;
	self->sr_start = other->sr_start;
	self->sr_size  = other->sr_size;
	Dee_Incref(self->sr_seq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
subrange_deep(SubRange *__restrict self,
              SubRange *__restrict other) {
	self->sr_seq = DeeObject_DeepCopy(other->sr_seq);
	if unlikely(!self->sr_seq)
		goto err;
	self->sr_start = other->sr_start;
	self->sr_size  = other->sr_size;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
subrange_init(SubRange *__restrict self, size_t argc,
              DeeObject *const *argv) {
	size_t end     = (size_t)-1;
	self->sr_start = 0;
	if (DeeArg_Unpack(argc, argv, "o|IuIu:_SeqSubRange",
	                  &self->sr_seq, &self->sr_start, &end))
		goto err;
	if (end < self->sr_start)
		end = self->sr_start;
	self->sr_size = (size_t)(end - self->sr_start);
	Dee_Incref(self->sr_seq);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
subrange_bool(SubRange *__restrict self) {
	size_t seqsize;
	if unlikely(!self->sr_size)
		return 0;
	seqsize = DeeObject_Size(self->sr_seq);
	if unlikely(seqsize == (size_t)-1)
		goto err;
	return self->sr_start < seqsize;
err:
	return -1;
}


INTERN DeeTypeObject SeqSubRange_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqSubRange",
	/* .tp_doc      = */ DOC("(seq:?DSequence,start=!0,end=!-1)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&subrange_ctor,
				/* .tp_copy_ctor = */ (void *)&subrange_copy,
				/* .tp_deep_ctor = */ (void *)&subrange_deep,
				/* .tp_any_ctor  = */ (void *)&subrange_init,
				TYPE_FIXED_ALLOCATOR(SubRange)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&subrange_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&subrange_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&subrange_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &subrange_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ subrange_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ subrange_class_members
};

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_GetRange(DeeObject *__restrict self,
                size_t begin, size_t end) {
	DREF SubRange *result;
	if unlikely(begin >= end)
		return_reference_(Dee_EmptySeq);
	/* Create a sub-range sequence. */
	result = DeeObject_MALLOC(SubRange);
	if unlikely(!result)
		goto done;
	if (DeeObject_InstanceOfExact(self, &SeqSubRange_Type)) {
		SubRange *me = (SubRange *)self;
		/* Special handling for recursion. */
		Dee_Incref(me->sr_seq);
		result->sr_seq   = me->sr_seq;
		result->sr_start = begin + me->sr_start;
		result->sr_size  = MIN((size_t)(end - begin), me->sr_size);
	} else {
		Dee_Incref(self);
		result->sr_seq   = self;
		result->sr_start = begin;
		result->sr_size  = (size_t)(end - begin);
	}
	DeeObject_Init(result, &SeqSubRange_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_GetRangeN(DeeObject *__restrict self,
                 size_t begin) {
	DREF SubRangeN *result;
	if (!begin)
		return_reference_(self);
	/* Create a sub-range sequence. */
	result = DeeObject_MALLOC(SubRangeN);
	if unlikely(!result)
		goto done;
	if (DeeObject_InstanceOfExact(self, &SeqSubRangeN_Type)) {
		SubRangeN *me = (SubRangeN *)self;
		/* Special handling for recursion. */
		Dee_Incref(me->sr_seq);
		result->sr_seq   = me->sr_seq;
		result->sr_start = begin + me->sr_start;
	} else {
		Dee_Incref(self);
		result->sr_seq   = self;
		result->sr_start = begin;
	}
	DeeObject_Init(result, &SeqSubRangeN_Type);
done:
	return (DREF DeeObject *)result;
}




STATIC_ASSERT(COMPILER_OFFSETOF(SubRangeN, sr_seq) ==
              COMPILER_OFFSETOF(SubRange, sr_seq));
#define subrangen_fini  subrange_fini
#define subrangen_visit subrange_visit


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
subrangen_iter(SubRangeN *__restrict self) {
	DREF DeeObject *result, *elem;
	size_t offset;
	result = DeeObject_IterSelf(self->sr_seq);
	if unlikely(!result)
		goto done;
	offset = self->sr_start;
	while (offset--) {
		elem = DeeObject_IterNext(result);
		if (!ITER_ISOK(elem)) {
			if (!elem)
				goto err;
			break; /* End of sequence. */
		}
		Dee_Decref(elem);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
done:
	return result;
err:
	Dee_Decref(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
subrangen_nsi_getsize(SubRangeN *__restrict self) {
	size_t result;
	result = DeeObject_Size(self->sr_seq);
	if likely(result != (size_t)-1) {
		if (result <= self->sr_start)
			result = 0;
		else {
			result -= self->sr_start;
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
subrangen_nsi_getsize_fast(SubRangeN *__restrict self) {
	size_t result;
	result = DeeFastSeq_GetSize(self->sr_seq);
	if likely(result != (size_t)-1) {
		if (result <= self->sr_start)
			result = 0;
		else {
			result -= self->sr_start;
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
subrangen_size(SubRangeN *__restrict self) {
	size_t result = subrangen_nsi_getsize(self);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
subrangen_getitem(SubRangeN *self,
                  DeeObject *index_ob) {
	size_t index;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	return DeeObject_GetItemIndex(self->sr_seq,
	                              self->sr_start + index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
subrangen_nsi_getitem(SubRangeN *__restrict self, size_t index) {
	return DeeObject_GetItemIndex(self->sr_seq,
	                              self->sr_start + index);
}

PRIVATE size_t DCALL
subrangen_nsi_find(SubRangeN *__restrict self, size_t start, size_t end,
                   DeeObject *__restrict keyed_search_item, DeeObject *key) {
	return DeeSeq_Find(self->sr_seq,
	                   start + self->sr_start,
	                   end + self->sr_start,
	                   keyed_search_item,
	                   key);
}

PRIVATE size_t DCALL
subrangen_nsi_rfind(SubRangeN *__restrict self, size_t start, size_t end,
                    DeeObject *__restrict keyed_search_item, DeeObject *key) {
	return DeeSeq_RFind(self->sr_seq,
	                    start + self->sr_start,
	                    end + self->sr_start,
	                    keyed_search_item,
	                    key);
}

PRIVATE struct type_nsi subrangen_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&subrangen_nsi_getsize,
			/* .nsi_getsize_fast = */ (void *)&subrangen_nsi_getsize_fast,
			/* .nsi_getitem      = */ (void *)&subrangen_nsi_getitem,
			/* .nsi_delitem      = */ (void *)NULL,
			/* .nsi_setitem      = */ (void *)NULL,
			/* .nsi_getitem_fast = */ (void *)NULL,
			/* .nsi_getrange     = */ (void *)NULL,
			/* .nsi_getrange_n   = */ (void *)NULL,
			/* .nsi_setrange     = */ (void *)NULL,
			/* .nsi_setrange_n   = */ (void *)NULL,
			/* .nsi_find         = */ (void *)&subrangen_nsi_find,
			/* .nsi_rfind        = */ (void *)&subrangen_nsi_rfind,
			/* .nsi_xch          = */ (void *)NULL,
			/* .nsi_insert       = */ (void *)NULL,
			/* .nsi_insertall    = */ (void *)NULL,
			/* .nsi_insertvec    = */ (void *)NULL,
			/* .nsi_pop          = */ (void *)NULL,
			/* .nsi_erase        = */ (void *)NULL,
			/* .nsi_remove       = */ (void *)NULL,
			/* .nsi_rremove      = */ (void *)NULL,
			/* .nsi_removeall    = */ (void *)NULL,
			/* .nsi_removeif     = */ (void *)NULL
		}
	}
};

PRIVATE struct type_seq subrangen_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&subrangen_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&subrangen_size,
	/* .tp_contains  = */ NULL,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&subrangen_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &subrangen_nsi
};

PRIVATE struct type_member subrangen_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(SubRangeN, sr_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_SIZE_T, offsetof(SubRangeN, sr_start)),
	TYPE_MEMBER_END
};


PRIVATE struct type_member subrangen_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &DeeIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
subrangen_ctor(SubRangeN *__restrict self) {
	self->sr_seq   = Dee_EmptySeq;
	self->sr_start = 0;
	Dee_Incref(Dee_EmptySeq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
subrangen_copy(SubRangeN *__restrict self,
               SubRangeN *__restrict other) {
	self->sr_seq   = other->sr_seq;
	self->sr_start = other->sr_start;
	Dee_Incref(self->sr_seq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
subrangen_deep(SubRangeN *__restrict self,
               SubRangeN *__restrict other) {
	self->sr_seq = DeeObject_DeepCopy(other->sr_seq);
	if unlikely(!self->sr_seq)
		goto err;
	self->sr_start = other->sr_start;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
subrangen_init(SubRangeN *__restrict self, size_t argc,
               DeeObject *const *argv) {
	self->sr_start = 0;
	if (DeeArg_Unpack(argc, argv, "o|Iu:_SeqSubRangeN",
	                  &self->sr_seq, &self->sr_start))
		goto err;
	Dee_Incref(self->sr_seq);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
subrangen_bool(SubRangeN *__restrict self) {
	size_t seqsize = DeeObject_Size(self->sr_seq);
	if unlikely(seqsize == (size_t)-1)
		goto err;
	return self->sr_start < seqsize;
err:
	return -1;
}


INTERN DeeTypeObject SeqSubRangeN_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqSubRangeN",
	/* .tp_doc      = */ DOC("(seq:?DSequence,start=!0)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&subrangen_ctor,
				/* .tp_copy_ctor = */ (void *)&subrangen_copy,
				/* .tp_deep_ctor = */ (void *)&subrangen_deep,
				/* .tp_any_ctor  = */ (void *)&subrangen_init,
				TYPE_FIXED_ALLOCATOR(SubRangeN)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&subrangen_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&subrangen_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&subrangen_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &subrangen_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ subrangen_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ subrangen_class_members
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SUBRANGE_C */
