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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SEQUENCES_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SEQUENCES_C 1

#include <deemon/api.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/gc.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/thread.h>

#include <hybrid/overflow.h>
/**/

#include "default-iterators.h"
#include "default-sequences.h"
/**/

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

DECL_BEGIN

/************************************************************************/
/* DefaultSequence_WithSizeAndGetItemIndex_Type                         */
/* DefaultSequence_WithSizeAndGetItemIndexFast_Type                     */
/************************************************************************/

#define ds_sgif_fini  ds_sgi_fini
#define ds_sgif_visit ds_sgi_visit

PRIVATE NONNULL((1)) void DCALL
ds_sgi_fini(DefaultSequence_WithSizeAndGetItemIndex *__restrict self) {
	Dee_Decref(self->dssgi_seq);
}

PRIVATE NONNULL((1, 2)) void DCALL
ds_sgi_visit(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
             dvisit_t proc, void *arg) {
	Dee_Visit(self->dssgi_seq);
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
ds_sgi_iter(DefaultSequence_WithSizeAndGetItemIndex *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dssgi_seq);
	result->disgi_seq              = self->dssgi_seq;
	result->disgi_tp_getitem_index = self->dssgi_tp_getitem_index;
	result->disgi_index            = self->dssgi_start;
	result->disgi_end              = self->dssgi_end;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndex_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
ds_sgif_iter(DefaultSequence_WithSizeAndGetItemIndex *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dssgi_seq);
	result->disgi_seq              = self->dssgi_seq;
	result->disgi_tp_getitem_index = self->dssgi_tp_getitem_index;
	result->disgi_index            = self->dssgi_start;
	result->disgi_end              = self->dssgi_end;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItemIndexFast_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_sgi_foreach(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
               Dee_foreach_t proc, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	for (i = self->dssgi_start; i < self->dssgi_end; ++i) {
		DREF DeeObject *elem;
		elem = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, i);
		if (!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			if (DeeError_Catch(&DeeError_IndexError))
				break;
			return -1;
		}
		temp = (*proc)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_sgif_foreach(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                Dee_foreach_t proc, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	for (i = self->dssgi_start; i < self->dssgi_end; ++i) {
		DREF DeeObject *elem;
		elem = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, i);
		if (!elem)
			continue; /* Unbound item. */
		temp = (*proc)(arg, elem);
		Dee_Decref(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

#define ds_sgif_size ds_sgi_size
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
ds_sgi_size(DefaultSequence_WithSizeAndGetItemIndex *__restrict self) {
	return self->dssgi_end - self->dssgi_start;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_sgi_getitem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	size_t used_index;
	if unlikely(OVERFLOW_UADD(index, self->dssgi_start, &used_index))
		goto err_obb;
	if unlikely(used_index >= self->dssgi_end)
		goto err_obb;
	return (*self->dssgi_tp_getitem_index)(self->dssgi_seq, used_index);
err_obb:
	err_index_out_of_bounds((DeeObject *)self, index, ds_sgi_size(self));
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_sgif_getitem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	size_t used_index;
	struct type_seq *tp_seq = Dee_TYPE(self->dssgi_seq)->tp_seq;
	ASSERT(tp_seq);
	ASSERT(tp_seq->tp_getitem_index_fast);
	ASSERT(tp_seq->tp_getitem_index_fast == self->dssgi_tp_getitem_index);
	ASSERTF(tp_seq->tp_getitem_index, "Always present because `tp_getitem_index_fast' is, too");
	if (OVERFLOW_UADD(index, self->dssgi_start, &used_index))
		goto err_obb;
	if (used_index >= self->dssgi_end)
		goto err_obb;
	return (*tp_seq->tp_getitem_index)(self->dssgi_seq, used_index);
err_obb:
	err_index_out_of_bounds((DeeObject *)self, index, ds_sgi_size(self));
	return NULL;
}

#define ds_sgif_delitem_index ds_sgi_delitem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
ds_sgi_delitem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	struct type_seq *tp_seq = Dee_TYPE(self->dssgi_seq)->tp_seq;
	ASSERT(tp_seq);
	ASSERT(tp_seq->tp_getitem_index);
	ASSERT(tp_seq->tp_getitem_index == self->dssgi_tp_getitem_index ||
	       tp_seq->tp_getitem_index_fast == self->dssgi_tp_getitem_index);
	if (tp_seq->tp_delitem_index) {
		size_t used_index;
		if (OVERFLOW_UADD(index, self->dssgi_start, &used_index))
			goto err_obb;
		if (used_index >= self->dssgi_end)
			goto err_obb;
		return (*tp_seq->tp_delitem_index)(self->dssgi_seq, used_index);
	}
	return err_unimplemented_operator(Dee_TYPE(self->dssgi_seq), OPERATOR_DELITEM);
err_obb:
	return err_index_out_of_bounds((DeeObject *)self, index, ds_sgi_size(self));
}

#define ds_sgif_setitem_index ds_sgi_setitem_index
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
ds_sgi_setitem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                     size_t index, DeeObject *value) {
	struct type_seq *tp_seq = Dee_TYPE(self->dssgi_seq)->tp_seq;
	ASSERT(tp_seq);
	ASSERT(tp_seq->tp_getitem_index);
	ASSERT(tp_seq->tp_getitem_index == self->dssgi_tp_getitem_index ||
	       tp_seq->tp_getitem_index_fast == self->dssgi_tp_getitem_index);
	if (tp_seq->tp_setitem_index) {
		size_t used_index;
		if (OVERFLOW_UADD(index, self->dssgi_start, &used_index))
			goto err_obb;
		if (used_index >= self->dssgi_end)
			goto err_obb;
		return (*tp_seq->tp_setitem_index)(self->dssgi_seq, used_index, value);
	}
	return err_unimplemented_operator(Dee_TYPE(self->dssgi_seq), OPERATOR_SETITEM);
err_obb:
	return err_index_out_of_bounds((DeeObject *)self, index, ds_sgi_size(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_sgif_getitem_index_fast(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	ASSERT(index < (self->dssgi_end - self->dssgi_start));
	return (*self->dssgi_tp_getitem_index)(self->dssgi_seq, self->dssgi_start + index);
}

#define ds_sgif_bounditem_index ds_sgi_bounditem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
ds_sgi_bounditem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	size_t used_index;
	struct type_seq *tp_seq = Dee_TYPE(self->dssgi_seq)->tp_seq;
	ASSERT(tp_seq);
	ASSERT(tp_seq->tp_getitem_index);
	ASSERT(tp_seq->tp_getitem_index == self->dssgi_tp_getitem_index ||
	       tp_seq->tp_getitem_index_fast == self->dssgi_tp_getitem_index);
	ASSERTF(tp_seq->tp_bounditem_index, "Always present when `tp_getitem_index' is present");
	if (OVERFLOW_UADD(index, self->dssgi_start, &used_index))
		goto err_obb;
	if (used_index >= self->dssgi_end)
		goto err_obb;
	return (*tp_seq->tp_bounditem_index)(self->dssgi_seq, used_index);
err_obb:
	return -2; /* Item doesn't exist */
}

#define ds_sgif_hasitem_index ds_sgi_hasitem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
ds_sgi_hasitem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	size_t used_index;
	struct type_seq *tp_seq = Dee_TYPE(self->dssgi_seq)->tp_seq;
	ASSERT(tp_seq);
	ASSERT(tp_seq->tp_getitem_index);
	ASSERT(tp_seq->tp_getitem_index == self->dssgi_tp_getitem_index ||
	       tp_seq->tp_getitem_index_fast == self->dssgi_tp_getitem_index);
	ASSERTF(tp_seq->tp_hasitem_index, "Always present when `tp_getitem_index' is present");
	if (OVERFLOW_UADD(index, self->dssgi_start, &used_index))
		goto err_obb;
	if (used_index >= self->dssgi_end)
		goto err_obb;
	return (*tp_seq->tp_hasitem_index)(self->dssgi_seq, used_index);
err_obb:
	return 0; /* Item doesn't exist */
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultSequence_WithSizeAndGetItemIndex *DCALL
ds_sgi_getrange_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                      Dee_ssize_t start, Dee_ssize_t end) {
	struct Dee_seq_range range;
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	size_t size = self->dssgi_end - self->dssgi_start;
	DeeSeqRange_Clamp(&range, start, end, size);
	if (range.sr_start <= 0 && range.sr_end >= size)
		return_reference_(self);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dssgi_seq);
	result->dssgi_seq              = self->dssgi_seq;
	result->dssgi_tp_getitem_index = self->dssgi_tp_getitem_index;
	result->dssgi_start            = self->dssgi_start + range.sr_start;
	result->dssgi_end              = self->dssgi_start + range.sr_end;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndex_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultSequence_WithSizeAndGetItemIndex *DCALL
ds_sgif_getrange_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                       Dee_ssize_t start, Dee_ssize_t end) {
	struct Dee_seq_range range;
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	size_t size = self->dssgi_end - self->dssgi_start;
	DeeSeqRange_Clamp(&range, start, end, size);
	if (range.sr_start <= 0 && range.sr_end >= size)
		return_reference_(self);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dssgi_seq);
	result->dssgi_seq              = self->dssgi_seq;
	result->dssgi_tp_getitem_index = self->dssgi_tp_getitem_index;
	result->dssgi_start            = self->dssgi_start + range.sr_start;
	result->dssgi_end              = self->dssgi_start + range.sr_end;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndexFast_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultSequence_WithSizeAndGetItemIndex *DCALL
ds_sgi_getrange_index_n(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, Dee_ssize_t start) {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	size_t used_start, size;
	size       = self->dssgi_end - self->dssgi_start;
	used_start = DeeSeqRange_Clamp_n(start, size);
	if (used_start <= 0)
		return_reference_(self);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dssgi_seq);
	result->dssgi_seq              = self->dssgi_seq;
	result->dssgi_tp_getitem_index = self->dssgi_tp_getitem_index;
	result->dssgi_start            = self->dssgi_start + used_start;
	result->dssgi_end              = self->dssgi_end;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndex_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultSequence_WithSizeAndGetItemIndex *DCALL
ds_sgif_getrange_index_n(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, Dee_ssize_t start) {
	DREF DefaultSequence_WithSizeAndGetItemIndex *result;
	size_t used_start, size;
	size       = self->dssgi_end - self->dssgi_start;
	used_start = DeeSeqRange_Clamp_n(start, size);
	if (used_start <= 0)
		return_reference_(self);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dssgi_seq);
	result->dssgi_seq              = self->dssgi_seq;
	result->dssgi_tp_getitem_index = self->dssgi_tp_getitem_index;
	result->dssgi_start            = self->dssgi_start + used_start;
	result->dssgi_end              = self->dssgi_end;
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItemIndexFast_Type);
	return result;
err:
	return NULL;
}

#define ds_sgif_members ds_sgi_members
PRIVATE struct type_member tpconst ds_sgi_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(DefaultSequence_WithSizeAndGetItemIndex, dssgi_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DefaultSequence_WithSizeAndGetItemIndex, dssgi_start)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DefaultSequence_WithSizeAndGetItemIndex, dssgi_end)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member ds_sgi_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeAndGetItemIndex_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member ds_sgif_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeAndGetItemIndexFast_Type),
	TYPE_MEMBER_END
};


PRIVATE struct type_seq ds_sgi_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_sgi_iter,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ NULL,
	/* .tp_foreach            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ds_sgi_foreach,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_sgi_size,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_sgi_getitem_index,
	/* .tp_delitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&ds_sgi_delitem_index,
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&ds_sgi_setitem_index,
	/* .tp_bounditem_index    = */ (int (DCALL *)(DeeObject *, size_t))&ds_sgi_bounditem_index,
	/* .tp_hasitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&ds_sgi_hasitem_index,
	/* .tp_getrange_index     = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&ds_sgi_getrange_index,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&ds_sgi_getrange_index_n,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_sgi_size,
	/* .tp_getitem_index_fast = */ NULL,
};

PRIVATE struct type_seq ds_sgif_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_sgif_iter,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ NULL,
	/* .tp_foreach            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ds_sgif_foreach,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_sgif_size,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_sgif_getitem_index,
	/* .tp_delitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&ds_sgif_delitem_index,
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&ds_sgif_setitem_index,
	/* .tp_bounditem_index    = */ (int (DCALL *)(DeeObject *, size_t))&ds_sgif_bounditem_index,
	/* .tp_hasitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&ds_sgif_hasitem_index,
	/* .tp_getrange_index     = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&ds_sgif_getrange_index,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&ds_sgif_getrange_index_n,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_sgif_size,
	/* .tp_getitem_index_fast = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_sgif_getitem_index_fast,
};

INTERN DeeTypeObject DefaultSequence_WithSizeAndGetItemIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqWithSizeAndGetItemIndex",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DefaultSequence_WithSizeAndGetItemIndex)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_sgi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ds_sgi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &ds_sgi_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_sgi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ds_sgi_class_members
};

INTERN DeeTypeObject DefaultSequence_WithSizeAndGetItemIndexFast_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqWithSizeAndGetItemIndexFast",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DefaultSequence_WithSizeAndGetItemIndex)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_sgif_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ds_sgif_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &ds_sgif_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_sgif_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ds_sgif_class_members
};
























/************************************************************************/
/* DefaultSequence_WithSizeAndGetItem_Type                              */
/* DefaultSequence_WithTSizeAndGetItem_Type                             */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultSequence_WithSizeAndGetItem, dssg_seq) == offsetof(DefaultSequence_WithTSizeAndGetItem, dstsg_seq));
STATIC_ASSERT(offsetof(DefaultSequence_WithSizeAndGetItem, dssg_start) == offsetof(DefaultSequence_WithTSizeAndGetItem, dstsg_start));
STATIC_ASSERT(offsetof(DefaultSequence_WithSizeAndGetItem, dssg_end) == offsetof(DefaultSequence_WithTSizeAndGetItem, dstsg_end));

#define ds_tsg_fini ds_sg_fini
PRIVATE NONNULL((1)) void DCALL
ds_sg_fini(DefaultSequence_WithSizeAndGetItem *__restrict self) {
	Dee_Decref(self->dssg_seq);
	Dee_Decref(self->dssg_start);
	Dee_Decref(self->dssg_end);
}

#define ds_tsg_visit ds_sg_visit
PRIVATE NONNULL((1, 2)) void DCALL
ds_sg_visit(DefaultSequence_WithSizeAndGetItem *__restrict self,
            dvisit_t proc, void *arg) {
	Dee_Visit(self->dssg_seq);
	Dee_Visit(self->dssg_start);
	Dee_Visit(self->dssg_end);
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItem *DCALL
ds_sg_iter(DefaultSequence_WithSizeAndGetItem *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItem *result;
	result = DeeGCObject_MALLOC(DefaultIterator_WithSizeAndGetItem);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dssg_seq);
	Dee_Incref(self->dssg_start);
	Dee_Incref(self->dssg_end);
	result->disg_seq        = self->dssg_seq;
	result->disg_tp_getitem = self->dssg_tp_getitem;
	result->disg_index      = self->dssg_start;
	result->disg_end        = self->dssg_end;
	Dee_atomic_lock_init(&result->disg_lock);
	DeeObject_Init(result, &DefaultIterator_WithSizeAndGetItem_Type);
	return (DREF DefaultIterator_WithSizeAndGetItem *)DeeGC_Track((DREF DeeObject *)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithTSizeAndGetItem *DCALL
ds_tsg_iter(DefaultSequence_WithTSizeAndGetItem *__restrict self) {
	DREF DefaultIterator_WithTSizeAndGetItem *result;
	result = DeeGCObject_MALLOC(DefaultIterator_WithTSizeAndGetItem);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dstsg_seq);
	Dee_Incref(self->dstsg_start);
	Dee_Incref(self->dstsg_end);
	result->ditsg_seq         = self->dstsg_seq;
	result->ditsg_tp_tgetitem = self->dstsg_tp_tgetitem;
	result->ditsg_index       = self->dstsg_start;
	result->ditsg_end         = self->dstsg_end;
	result->ditsg_tp_seq      = self->dstsg_tp_seq;
	Dee_atomic_lock_init(&result->ditsg_lock);
	DeeObject_Init(result, &DefaultIterator_WithTSizeAndGetItem_Type);
	return (DREF DefaultIterator_WithTSizeAndGetItem *)DeeGC_Track((DREF DeeObject *)result);
err:
	return NULL;
}

#define ds_tsg_sizeob ds_sg_sizeob
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_sg_sizeob(DefaultSequence_WithSizeAndGetItem *__restrict self) {
	return DeeObject_Sub(self->dssg_end, self->dssg_start);
}

#define ds_tsg_mapindex(self, index) ds_sg_mapindex((DefaultSequence_WithSizeAndGetItem *)(self), index)
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ds_sg_mapindex(DefaultSequence_WithSizeAndGetItem *self, DeeObject *index) {
	int temp;
	DREF DeeObject *used_index;
	used_index = DeeObject_Add(self->dssg_start, index);
	if unlikely(!used_index)
		goto err;
	temp = DeeObject_CompareGe(used_index, self->dssg_end);
	if unlikely(temp < 0)
		goto err_used_index;
	return used_index;
err_used_index:
	Dee_Decref(used_index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ds_sg_getitem(DefaultSequence_WithSizeAndGetItem *self, DeeObject *index) {
	DREF DeeObject *result;
	DREF DeeObject *used_index = ds_sg_mapindex(self, index);
	if unlikely(!used_index)
		goto err;
	result = (*self->dssg_tp_getitem)(self->dssg_seq, used_index);
	Dee_Decref(used_index);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ds_tsg_getitem(DefaultSequence_WithTSizeAndGetItem *self, DeeObject *index) {
	DREF DeeObject *result;
	DREF DeeObject *used_index = ds_tsg_mapindex(self, index);
	if unlikely(!used_index)
		goto err;
	result = (*self->dstsg_tp_tgetitem)(self->dstsg_tp_seq, self->dstsg_seq, used_index);
	Dee_Decref(used_index);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_sg_delitem(DefaultSequence_WithSizeAndGetItem *self, DeeObject *index) {
	struct type_seq *tp_seq = Dee_TYPE(self->dssg_seq)->tp_seq;
	ASSERT(tp_seq);
	ASSERT(tp_seq->tp_getitem);
	ASSERT(tp_seq->tp_getitem == self->dssg_tp_getitem);
	if (tp_seq->tp_delitem) {
		int result;
		DREF DeeObject *used_index = ds_sg_mapindex(self, index);
		if unlikely(!used_index)
			goto err;
		result = (*tp_seq->tp_delitem)(self->dssg_seq, used_index);
		Dee_Decref(used_index);
		return result;
	}
	return err_unimplemented_operator(Dee_TYPE(self->dssg_seq), OPERATOR_DELITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_tsg_delitem(DefaultSequence_WithTSizeAndGetItem *self, DeeObject *index) {
	struct type_seq *tp_seq = self->dstsg_tp_seq->tp_seq;
	ASSERT(tp_seq);
	ASSERT(tp_seq->tp_getitem);
	if (tp_seq->tp_delitem) {
		int result;
		DREF DeeObject *used_index = ds_tsg_mapindex(self, index);
		if unlikely(!used_index)
			goto err;
		result = DeeType_invoke_seq_tp_delitem(self->dstsg_tp_seq,
		                                       tp_seq->tp_delitem,
		                                       self->dstsg_seq,
		                                       used_index);
		Dee_Decref(used_index);
		return result;
	}
	return err_unimplemented_operator(self->dstsg_tp_seq, OPERATOR_DELITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
ds_sg_setitem(DefaultSequence_WithSizeAndGetItem *self,
              DeeObject *index, DeeObject *value) {
	struct type_seq *tp_seq = Dee_TYPE(self->dssg_seq)->tp_seq;
	ASSERT(tp_seq);
	ASSERT(tp_seq->tp_getitem);
	ASSERT(tp_seq->tp_getitem == self->dssg_tp_getitem);
	if (tp_seq->tp_setitem) {
		int result;
		DREF DeeObject *used_index = ds_sg_mapindex(self, index);
		if unlikely(!used_index)
			goto err;
		result = (*tp_seq->tp_setitem)(self->dssg_seq, used_index, value);
		Dee_Decref(used_index);
		return result;
	}
	return err_unimplemented_operator(Dee_TYPE(self->dssg_seq), OPERATOR_SETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
ds_tsg_setitem(DefaultSequence_WithTSizeAndGetItem *self,
               DeeObject *index, DeeObject *value) {
	struct type_seq *tp_seq = self->dstsg_tp_seq->tp_seq;
	ASSERT(tp_seq);
	ASSERT(tp_seq->tp_getitem);
	if (tp_seq->tp_setitem) {
		int result;
		DREF DeeObject *used_index = ds_tsg_mapindex(self, index);
		if unlikely(!used_index)
			goto err;
		result = DeeType_invoke_seq_tp_setitem(self->dstsg_tp_seq,
		                                       tp_seq->tp_setitem,
		                                       self->dstsg_seq,
		                                       used_index, value);
		Dee_Decref(used_index);
		return result;
	}
	return err_unimplemented_operator(self->dstsg_tp_seq, OPERATOR_SETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DefaultSequence_WithSizeAndGetItem *DCALL
ds_sg_getrange(DefaultSequence_WithSizeAndGetItem *self, DeeObject *start, DeeObject *end) {
	int temp;
	DREF DefaultSequence_WithSizeAndGetItem *result;
	DREF DeeObject *clamed_start_end_and_pair[2];
	DREF DeeObject *clamed_start_and_end;
	DREF DeeObject *sizeob = ds_sg_sizeob(self);
	if unlikely(!sizeob)
		goto err;

	/* Make a call to "util.clamprange()" to do the range-fixup. */
	clamed_start_and_end = DeeModule_CallExternStringf("util", "clamprange", "ooo", start, end, sizeob);
	Dee_Decref(sizeob);
	if unlikely(!clamed_start_and_end)
		goto err;
	temp = DeeObject_Unpack(clamed_start_and_end, 2, clamed_start_end_and_pair);
	Dee_Decref(clamed_start_and_end);
	if unlikely(temp)
		goto err;
	Dee_Incref(self->dssg_seq);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeAndGetItem);
	if unlikely(!result)
		goto err_clamed_start_end_and_pair;
	result->dssg_seq        = self->dssg_seq;
	result->dssg_tp_getitem = self->dssg_tp_getitem;
	result->dssg_start      = clamed_start_end_and_pair[0]; /* Inherit reference */
	result->dssg_end        = clamed_start_end_and_pair[1]; /* Inherit reference */
	DeeObject_Init(result, &DefaultSequence_WithSizeAndGetItem_Type);
	return result;
err_clamed_start_end_and_pair:
	Dee_Decref(clamed_start_end_and_pair[1]);
	Dee_Decref(clamed_start_end_and_pair[0]);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DefaultSequence_WithTSizeAndGetItem *DCALL
ds_tsg_getrange(DefaultSequence_WithTSizeAndGetItem *self, DeeObject *start, DeeObject *end) {
	int temp;
	DREF DefaultSequence_WithTSizeAndGetItem *result;
	DREF DeeObject *clamed_start_end_and_pair[2];
	DREF DeeObject *clamed_start_and_end;
	DREF DeeObject *sizeob = ds_tsg_sizeob((DefaultSequence_WithSizeAndGetItem *)self);
	if unlikely(!sizeob)
		goto err;

	/* Make a call to "util.clamprange()" to do the range-fixup. */
	clamed_start_and_end = DeeModule_CallExternStringf("util", "clamprange", "ooo", start, end, sizeob);
	Dee_Decref(sizeob);
	if unlikely(!clamed_start_and_end)
		goto err;
	temp = DeeObject_Unpack(clamed_start_and_end, 2, clamed_start_end_and_pair);
	Dee_Decref(clamed_start_and_end);
	if unlikely(temp)
		goto err;
	Dee_Incref(self->dstsg_seq);
	result = DeeObject_MALLOC(DefaultSequence_WithTSizeAndGetItem);
	if unlikely(!result)
		goto err_clamed_start_end_and_pair;
	result->dstsg_seq         = self->dstsg_seq;
	result->dstsg_tp_tgetitem = self->dstsg_tp_tgetitem;
	result->dstsg_start       = clamed_start_end_and_pair[0]; /* Inherit reference */
	result->dstsg_end         = clamed_start_end_and_pair[1]; /* Inherit reference */
	result->dstsg_tp_seq      = self->dstsg_tp_seq;
	DeeObject_Init(result, &DefaultSequence_WithTSizeAndGetItem_Type);
	return result;
err_clamed_start_end_and_pair:
	Dee_Decref(clamed_start_end_and_pair[1]);
	Dee_Decref(clamed_start_end_and_pair[0]);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_sg_foreach(DefaultSequence_WithSizeAndGetItem *self, Dee_foreach_t proc, void *arg) {
	DREF DeeObject *index;
	Dee_ssize_t temp, result = 0;
	int error;
	error = DeeObject_CompareGe(self->dssg_start, self->dssg_end);
	if unlikely(error < 0)
		goto err;
	if (!error) {
		index = self->dssg_start;
		Dee_Incref(index);
		for (;;) {
			DREF DeeObject *elem;
			elem = (*self->dssg_tp_getitem)(self->dssg_seq, index);
			if unlikely(!elem) {
				if (DeeError_Catch(&DeeError_UnboundItem))
					continue;
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err_index;
			}
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_index_temp;
			result += temp;
			if (DeeThread_CheckInterrupt())
				goto err_index;
			error = DeeObject_CompareGe(self->dssg_start, self->dssg_end);
			if unlikely(error < 0)
				goto err_index;
			if (!error)
				break;
			if unlikely(DeeObject_Inc(&index))
				goto err_index;
		}
		Dee_Decref(index);
	}
	return result;
err_index_temp:
	Dee_Decref(index);
	return temp;
err_index:
	Dee_Decref(index);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_tsg_foreach(DefaultSequence_WithTSizeAndGetItem *self, Dee_foreach_t proc, void *arg) {
	DREF DeeObject *index;
	Dee_ssize_t temp, result = 0;
	int error;
	error = DeeObject_CompareGe(self->dstsg_start, self->dstsg_end);
	if unlikely(error < 0)
		goto err;
	if (!error) {
		index = self->dstsg_start;
		Dee_Incref(index);
		for (;;) {
			DREF DeeObject *elem;
			elem = (*self->dstsg_tp_tgetitem)(self->dstsg_tp_seq, self->dstsg_seq, index);
			if unlikely(!elem) {
				if (DeeError_Catch(&DeeError_UnboundItem))
					continue;
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err_index;
			}
			temp = (*proc)(arg, elem);
			Dee_Decref(elem);
			if unlikely(temp < 0)
				goto err_index_temp;
			result += temp;
			if (DeeThread_CheckInterrupt())
				goto err_index;
			error = DeeObject_CompareGe(self->dstsg_start, self->dstsg_end);
			if unlikely(error < 0)
				goto err_index;
			if (!error)
				break;
			if unlikely(DeeObject_Inc(&index))
				goto err_index;
		}
		Dee_Decref(index);
	}
	return result;
err_index_temp:
	Dee_Decref(index);
	return temp;
err_index:
	Dee_Decref(index);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_sg_bounditem(DefaultSequence_WithSizeAndGetItem *self, DeeObject *index) {
	int result;
	DREF DeeObject *used_index;
	struct type_seq *tp_seq = Dee_TYPE(self->dssg_seq)->tp_seq;
	ASSERT(tp_seq);
	ASSERT(tp_seq->tp_getitem);
	ASSERT(tp_seq->tp_getitem == self->dssg_tp_getitem);
	ASSERTF(tp_seq->tp_bounditem, "Always present when `tp_getitem' is present");
	used_index = ds_sg_mapindex(self, index);
	if unlikely(!used_index)
		goto err;
	result = (*tp_seq->tp_bounditem)(self->dssg_seq, used_index);
	Dee_Decref(used_index);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_tsg_bounditem(DefaultSequence_WithTSizeAndGetItem *self, DeeObject *index) {
	int result;
	DREF DeeObject *used_index;
	struct type_seq *tp_seq = self->dstsg_tp_seq->tp_seq;
	ASSERT(tp_seq);
	ASSERT(tp_seq->tp_getitem);
	ASSERTF(tp_seq->tp_bounditem, "Always present when `tp_getitem' is present");
	used_index = ds_tsg_mapindex(self, index);
	if unlikely(!used_index)
		goto err;
	result = DeeType_invoke_seq_tp_bounditem(self->dstsg_tp_seq,
	                                         tp_seq->tp_bounditem,
	                                         self->dstsg_seq,
	                                         used_index);
	Dee_Decref(used_index);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_sg_hasitem(DefaultSequence_WithSizeAndGetItem *self, DeeObject *index) {
	int result;
	DREF DeeObject *used_index;
	struct type_seq *tp_seq = Dee_TYPE(self->dssg_seq)->tp_seq;
	ASSERT(tp_seq);
	ASSERT(tp_seq->tp_getitem);
	ASSERT(tp_seq->tp_getitem == self->dssg_tp_getitem);
	ASSERTF(tp_seq->tp_hasitem, "Always present when `tp_getitem' is present");
	used_index = ds_sg_mapindex(self, index);
	if unlikely(!used_index)
		goto err;
	result = (*tp_seq->tp_hasitem)(self->dssg_seq, used_index);
	Dee_Decref(used_index);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_tsg_hasitem(DefaultSequence_WithTSizeAndGetItem *self, DeeObject *index) {
	int result;
	DREF DeeObject *used_index;
	struct type_seq *tp_seq = self->dstsg_tp_seq->tp_seq;
	ASSERT(tp_seq);
	ASSERT(tp_seq->tp_getitem);
	ASSERTF(tp_seq->tp_hasitem, "Always present when `tp_getitem' is present");
	used_index = ds_tsg_mapindex(self, index);
	if unlikely(!used_index)
		goto err;
	result = DeeType_invoke_seq_tp_hasitem(self->dstsg_tp_seq,
	                                       tp_seq->tp_hasitem,
	                                       self->dstsg_seq,
	                                       used_index);
	Dee_Decref(used_index);
	return result;
err:
	return -1;
}


PRIVATE struct type_seq ds_sg_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_sg_iter,
	/* .tp_sizeob             = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_sg_sizeob,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ds_sg_getitem,
	/* .tp_delitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&ds_sg_delitem,
	/* .tp_setitem            = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&ds_sg_setitem,
	/* .tp_getrange           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&ds_sg_getrange,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ NULL,
	/* .tp_foreach            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ds_sg_foreach,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ (int (DCALL *)(DeeObject *, DeeObject *))&ds_sg_bounditem,
	/* .tp_hasitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&ds_sg_hasitem,
	/* .tp_size               = */ NULL,
	/* .tp_getitem_index      = */ NULL,
	/* .tp_delitem_index      = */ NULL,
	/* .tp_setitem_index      = */ NULL,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ NULL,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ NULL,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index_fast = */ NULL,
};

PRIVATE struct type_seq ds_tsg_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_tsg_iter,
	/* .tp_sizeob             = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_tsg_sizeob,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ds_tsg_getitem,
	/* .tp_delitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&ds_tsg_delitem,
	/* .tp_setitem            = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&ds_tsg_setitem,
	/* .tp_getrange           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&ds_tsg_getrange,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ NULL,
	/* .tp_foreach            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ds_tsg_foreach,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ (int (DCALL *)(DeeObject *, DeeObject *))&ds_tsg_bounditem,
	/* .tp_hasitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&ds_tsg_hasitem,
	/* .tp_size               = */ NULL,
	/* .tp_getitem_index      = */ NULL,
	/* .tp_delitem_index      = */ NULL,
	/* .tp_setitem_index      = */ NULL,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ NULL,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ NULL,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index_fast = */ NULL,
};

PRIVATE struct type_member tpconst ds_tsg_members[] = {
	TYPE_MEMBER_FIELD_DOC("__tpseq__", STRUCT_OBJECT, offsetof(DefaultSequence_WithTSizeAndGetItem, dstsg_tp_seq), "->?DType"),
#define ds_sg_members (ds_tsg_members + 1)
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(DefaultSequence_WithTSizeAndGetItem, dstsg_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_OBJECT, offsetof(DefaultSequence_WithTSizeAndGetItem, dstsg_start)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_OBJECT, offsetof(DefaultSequence_WithTSizeAndGetItem, dstsg_end)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member ds_sg_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeAndGetItem_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member ds_tsg_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithTSizeAndGetItem_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DefaultSequence_WithSizeAndGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqWithSizeAndGetItem",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DefaultSequence_WithSizeAndGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_sg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ds_sg_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &ds_sg_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_sg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ds_sg_class_members
};

INTERN DeeTypeObject DefaultSequence_WithTSizeAndGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqWithTSizeAndGetItem",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DefaultSequence_WithTSizeAndGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_tsg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ds_tsg_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &ds_tsg_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_tsg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ds_tsg_class_members
};
























/************************************************************************/
/* DefaultSequence_WithIter_Type                                        */
/* DefaultSequence_WithTIter_Type                                       */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultSequence_WithIter, dsi_seq) == offsetof(DefaultSequence_WithSizeAndGetItemIndex, dssgi_seq));
STATIC_ASSERT(offsetof(DefaultSequence_WithIter, dsi_seq) == offsetof(DefaultSequence_WithTIter, dsti_seq));
STATIC_ASSERT(offsetof(DefaultSequence_WithIter, dsi_start) == offsetof(DefaultSequence_WithTIter, dsti_start));
STATIC_ASSERT(offsetof(DefaultSequence_WithIter, dsi_limit) == offsetof(DefaultSequence_WithTIter, dsti_limit));

#define ds_i_fini   ds_sgi_fini
#define ds_ti_fini  ds_sgi_fini
#define ds_i_visit  ds_sgi_visit
#define ds_ti_visit ds_sgi_visit

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_i_iter(DefaultSequence_WithIter *__restrict self) {
	DREF DefaultIterator_WithNextAndLimit *result;
	DREF DeeObject *iter;
	size_t iter_status;
#if defined(__OPTIMIZE__) && !defined(__OPTIMIZE_SIZE__)
	if (self->dsi_start == 0 && self->dsi_limit == (size_t)-1)
		return (*self->dsi_tp_iter)(self->dsi_seq); /* So the compiler can fold the call-frame. */
#endif /* __OPTIMIZE__ && !__OPTIMIZE_SIZE__ */
	iter = (*self->dsi_tp_iter)(self->dsi_seq);
	if unlikely(!iter)
		goto err;
	iter_status = DeeObject_IterAdvance(iter, self->dsi_start);
	if (iter_status != self->dsi_start) {
		if unlikely(iter_status == (size_t)-1)
			goto err_iter;
		return iter; /* Exhausted iterator... */
	}
	if unlikely(!Dee_TYPE(iter)->tp_iter_next &&
	            !DeeType_InheritIterNext(Dee_TYPE(iter)))
		goto err_iter_no_iter_next;
	result = DeeObject_MALLOC(DefaultIterator_WithNextAndLimit);
	if unlikely(!result)
		goto err;
	result->dinl_iter    = iter; /* Inherit reference */
	result->dinl_tp_next = Dee_TYPE(iter)->tp_iter_next;
	result->dinl_limit   = self->dsi_limit;
	DeeObject_Init(result, &DefaultIterator_WithNextAndLimit_Type);
	return (DREF DeeObject *)result;
err_iter_no_iter_next:
	err_unimplemented_operator(Dee_TYPE(iter), OPERATOR_ITERNEXT);
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_ti_iter(DefaultSequence_WithTIter *__restrict self) {
	DREF DefaultIterator_WithNextAndLimit *result;
	DREF DeeObject *iter;
	size_t iter_status;
#if defined(__OPTIMIZE__) && !defined(__OPTIMIZE_SIZE__)
	if (self->dsti_start == 0 && self->dsti_limit == (size_t)-1)
		return (*self->dsti_tp_titer)(self->dsti_tp_seq, self->dsti_seq); /* So the compiler can fold the call-frame. */
#endif /* __OPTIMIZE__ && !__OPTIMIZE_SIZE__ */
	iter = (*self->dsti_tp_titer)(self->dsti_tp_seq, self->dsti_seq);
	if unlikely(!iter)
		goto err;
	iter_status = DeeObject_IterAdvance(iter, self->dsti_start);
	if (iter_status != self->dsti_start) {
		if unlikely(iter_status == (size_t)-1)
			goto err_iter;
		return iter; /* Exhausted iterator... */
	}
	if unlikely(!Dee_TYPE(iter)->tp_iter_next &&
	            !DeeType_InheritIterNext(Dee_TYPE(iter)))
		goto err_iter_no_iter_next;
	result = DeeObject_MALLOC(DefaultIterator_WithNextAndLimit);
	if unlikely(!result)
		goto err;
	result->dinl_iter    = iter; /* Inherit reference */
	result->dinl_tp_next = Dee_TYPE(iter)->tp_iter_next;
	result->dinl_limit   = self->dsti_limit;
	DeeObject_Init(result, &DefaultIterator_WithNextAndLimit_Type);
	return (DREF DeeObject *)result;
err_iter_no_iter_next:
	err_unimplemented_operator(Dee_TYPE(iter), OPERATOR_ITERNEXT);
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
ds_i_size(DefaultSequence_WithIter *__restrict self) {
	DREF DeeObject *iter;
	size_t result = 0;
	if likely(self->dsi_limit > 0) {
		size_t iter_status;
		iter = (*self->dsi_tp_iter)(self->dsi_seq);
		if unlikely(!iter)
			goto err;
		iter_status = DeeObject_IterAdvance(iter, self->dsi_start);
		if (iter_status != self->dsi_start) {
			if unlikely(iter_status == (size_t)-1)
				goto err_iter;
			/* Exhausted iterator... */
		} else {
			result = DeeObject_IterAdvance(iter, self->dsi_limit);
		}
		Dee_Decref(iter);
	}
	return result;
err_iter:
	Dee_Decref(iter);
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
ds_ti_size(DefaultSequence_WithTIter *__restrict self) {
	DREF DeeObject *iter;
	size_t result = 0;
	if likely(self->dsti_limit > 0) {
		size_t iter_status;
		iter = (*self->dsti_tp_titer)(self->dsti_tp_seq, self->dsti_seq);
		if unlikely(!iter)
			goto err;
		iter_status = DeeObject_IterAdvance(iter, self->dsti_start);
		if (iter_status != self->dsti_start) {
			if unlikely(iter_status == (size_t)-1)
				goto err_iter;
			/* Exhausted iterator... */
		} else {
			result = DeeObject_IterAdvance(iter, self->dsti_limit);
		}
		Dee_Decref(iter);
	}
	return result;
err_iter:
	Dee_Decref(iter);
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_i_getitem_index(DefaultSequence_WithIter *__restrict self, size_t index) {
	size_t iter_status;
	DREF DeeObject *result, *iter;
	if unlikely(index >= self->dsi_limit)
		goto err_obb;
	if unlikely(OVERFLOW_UADD(index, self->dsi_start, &index))
		goto err_overflow;
	iter = (*self->dsi_tp_iter)(self->dsi_seq);
	if unlikely(!iter)
		goto err;
	iter_status = DeeObject_IterAdvance(iter, index);
	if (iter_status != index) {
		if unlikely(iter_status == (size_t)-1)
			goto err_iter;
		if (OVERFLOW_USUB(iter_status, self->dsi_start, &iter_status))
			iter_status = 0;
		err_index_out_of_bounds((DeeObject *)self, index, iter_status);
		goto err_iter;
	}
	result = DeeObject_IterNext(iter);
	Dee_Decref(iter);
	if unlikely(result == ITER_DONE) {
		index -= self->dsi_start;
		err_index_out_of_bounds((DeeObject *)self, index, index);
		goto err;
	}
	return result;
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
	goto err;
err_obb:
	err_index_out_of_bounds((DeeObject *)self, index, self->dsi_limit);
	goto err;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_ti_getitem_index(DefaultSequence_WithTIter *__restrict self, size_t index) {
	size_t iter_status;
	DREF DeeObject *result, *iter;
	if unlikely(index >= self->dsti_limit)
		goto err_obb;
	if unlikely(OVERFLOW_UADD(index, self->dsti_start, &index))
		goto err_overflow;
	iter = (*self->dsti_tp_titer)(self->dsti_tp_seq, self->dsti_seq);
	if unlikely(!iter)
		goto err;
	iter_status = DeeObject_IterAdvance(iter, index);
	if (iter_status != index) {
		if unlikely(iter_status == (size_t)-1)
			goto err_iter;
		if (OVERFLOW_USUB(iter_status, self->dsti_start, &iter_status))
			iter_status = 0;
		err_index_out_of_bounds((DeeObject *)self, index, iter_status);
		goto err_iter;
	}
	result = DeeObject_IterNext(iter);
	Dee_Decref(iter);
	if unlikely(result == ITER_DONE) {
		index -= self->dsti_start;
		err_index_out_of_bounds((DeeObject *)self, index, index);
		goto err;
	}
	return result;
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
	goto err;
err_obb:
	err_index_out_of_bounds((DeeObject *)self, index, self->dsti_limit);
	goto err;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_i_getrange_index(DefaultSequence_WithIter *__restrict self,
                    Dee_ssize_t start, Dee_ssize_t end) {
	DREF DefaultSequence_WithIter *result;
	struct Dee_seq_range range;
	if (start >= 0 && end >= 0) {
		range.sr_start = (size_t)start;
		range.sr_end   = (size_t)end;
	} else {
		size_t size = ds_i_size(self);
		if unlikely(size == (size_t)-1)
			goto err;
		DeeSeqRange_Clamp(&range, start, end, size);
	}
	if (range.sr_start >= range.sr_end)
		return_empty_seq;
	result = DeeObject_MALLOC(DefaultSequence_WithIter);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dsi_seq);
	result->dsi_seq     = self->dsi_seq;
	result->dsi_tp_iter = self->dsi_tp_iter;
	result->dsi_start   = self->dsi_start + range.sr_start;
	result->dsi_limit   = range.sr_end - range.sr_start;
	DeeObject_Init(result, &DefaultSequence_WithIter_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_ti_getrange_index(DefaultSequence_WithTIter *__restrict self,
                     Dee_ssize_t start, Dee_ssize_t end) {
	DREF DefaultSequence_WithTIter *result;
	struct Dee_seq_range range;
	if (start >= 0 && end >= 0) {
		range.sr_start = (size_t)start;
		range.sr_end   = (size_t)end;
	} else {
		size_t size = ds_ti_size(self);
		if unlikely(size == (size_t)-1)
			goto err;
		DeeSeqRange_Clamp(&range, start, end, size);
	}
	if (range.sr_start >= range.sr_end)
		return_empty_seq;
	result = DeeObject_MALLOC(DefaultSequence_WithTIter);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dsti_seq);
	result->dsti_seq      = self->dsti_seq;
	result->dsti_tp_titer = self->dsti_tp_titer;
	result->dsti_start    = self->dsti_start + range.sr_start;
	result->dsti_limit    = range.sr_end - range.sr_start;
	result->dsti_tp_seq   = self->dsti_tp_seq;
	DeeObject_Init(result, &DefaultSequence_WithTIter_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultSequence_WithIter *DCALL
ds_i_getrange_index_n(DefaultSequence_WithIter *__restrict self,
                      Dee_ssize_t start) {
	DREF DefaultSequence_WithIter *result;
	size_t used_start;
	if (start >= 0) {
		used_start = (size_t)start;
	} else {
		size_t size = ds_i_size(self);
		if unlikely(size == (size_t)-1)
			goto err;
		used_start = DeeSeqRange_Clamp_n(start, size);
	}
	if (used_start == 0)
		return_reference_(self);
	result = DeeObject_MALLOC(DefaultSequence_WithIter);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dsi_seq);
	result->dsi_seq     = self->dsi_seq;
	result->dsi_tp_iter = self->dsi_tp_iter;
	result->dsi_start   = self->dsi_start + start;
	result->dsi_limit   = self->dsi_limit - start;
	if (self->dsi_limit == (size_t)-1)
		result->dsi_limit = (size_t)-1;
	DeeObject_Init(result, &DefaultSequence_WithIter_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultSequence_WithTIter *DCALL
ds_ti_getrange_index_n(DefaultSequence_WithTIter *__restrict self,
                       Dee_ssize_t start) {
	DREF DefaultSequence_WithTIter *result;
	size_t used_start;
	if (start >= 0) {
		used_start = (size_t)start;
	} else {
		size_t size = ds_ti_size(self);
		if unlikely(size == (size_t)-1)
			goto err;
		used_start = DeeSeqRange_Clamp_n(start, size);
	}
	if (used_start == 0)
		return_reference_(self);
	result = DeeObject_MALLOC(DefaultSequence_WithTIter);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dsti_seq);
	result->dsti_seq      = self->dsti_seq;
	result->dsti_tp_titer = self->dsti_tp_titer;
	result->dsti_start    = self->dsti_start + start;
	result->dsti_limit    = self->dsti_limit - start;
	if (self->dsti_limit == (size_t)-1)
		result->dsti_limit = (size_t)-1;
	DeeObject_Init(result, &DefaultSequence_WithTIter_Type);
	return result;
err:
	return NULL;
}

PRIVATE struct type_seq ds_i_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_i_iter,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ NULL,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_i_size,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&ds_i_getitem_index,
	/* .tp_delitem_index      = */ NULL,
	/* .tp_setitem_index      = */ NULL,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&ds_i_getrange_index,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&ds_i_getrange_index_n,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index_fast = */ NULL
};

PRIVATE struct type_seq ds_ti_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_ti_iter,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ NULL,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ NULL,
	/* .tp_foreach            = */ NULL,
	/* .tp_foreach_pair       = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_ti_size,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&ds_ti_getitem_index,
	/* .tp_delitem_index      = */ NULL,
	/* .tp_setitem_index      = */ NULL,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&ds_ti_getrange_index,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&ds_ti_getrange_index_n,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index_fast = */ NULL
};

PRIVATE struct type_member tpconst ds_ti_members[] = {
	TYPE_MEMBER_FIELD_DOC("__tpseq__", STRUCT_OBJECT, offsetof(DefaultSequence_WithTIter, dsti_tp_seq), "->?DType"),
#define ds_i_members (ds_ti_members + 1)
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(DefaultSequence_WithIter, dsi_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_OBJECT, offsetof(DefaultSequence_WithIter, dsi_start)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_OBJECT, offsetof(DefaultSequence_WithIter, dsi_limit)),
	TYPE_MEMBER_END
};

#if 1 /* Not always true, but is generally the case. */
#define ds_ti_class_members ds_i_class_members
PRIVATE struct type_member ds_i_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithNextAndLimit_Type),
	TYPE_MEMBER_END
};
#else
#define ds_ti_class_members NULL
#define ds_i_class_members  NULL
#endif

INTERN DeeTypeObject DefaultSequence_WithIter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqWithIter",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DefaultSequence_WithIter)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_i_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ds_i_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &ds_i_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_i_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ds_i_class_members,
};

INTERN DeeTypeObject DefaultSequence_WithTIter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqWithTIter",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DefaultSequence_WithTIter)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_ti_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ds_ti_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &ds_ti_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_ti_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ds_ti_class_members,
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SEQUENCES_C */
