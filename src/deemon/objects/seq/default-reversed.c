/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_REVERSED_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_REVERSED_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/object.h>
#include <deemon/seq.h>

/**/
#include "default-reversed.h"

/**/
#include "../../runtime/runtime_error.h"

DECL_BEGIN

#define rs_giif_copy rs_gii_copy
#define rs_tgii_copy rs_gii_copy
PRIVATE WUNUSED NONNULL((1)) int DCALL
rs_gii_copy(DefaultReversed_WithGetItemIndex *__restrict self,
            DefaultReversed_WithGetItemIndex *__restrict other) {
	Dee_Incref(self->drwgii_seq);
	self->drwgii_tp_getitem_index = other->drwgii_tp_getitem_index;
	self->drwgii_seq              = other->drwgii_seq;
	self->drwgii_max              = other->drwgii_max;
	self->drwgii_size             = other->drwgii_size;
	return 0;
}

#define rs_giif_deepcopy rs_gii_deepcopy
#define rs_tgii_deepcopy rs_gii_deepcopy
PRIVATE WUNUSED NONNULL((1)) int DCALL
rs_gii_deepcopy(DefaultReversed_WithGetItemIndex *__restrict self,
                DefaultReversed_WithGetItemIndex *__restrict other) {
	self->drwgii_seq = DeeObject_DeepCopy(other->drwgii_seq);
	if unlikely(!self->drwgii_seq)
		goto err;
	self->drwgii_tp_getitem_index = other->drwgii_tp_getitem_index;
	self->drwgii_max              = other->drwgii_max;
	self->drwgii_size             = other->drwgii_size;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
verify_max_and_size(DefaultReversed_WithGetItemIndex *__restrict self) {
	if unlikely(self->drwgii_max == (size_t)-1 ||
	            self->drwgii_size > (self->drwgii_max + 1)) {
		return DeeError_Throwf(&DeeError_ValueError,
		                       "Bad max=%" PRFuSIZ ", size=%" PRFuSIZ " values",
		                       self->drwgii_max, self->drwgii_size);
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rs_gii_init(DefaultReversed_WithGetItemIndex *__restrict self,
            size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_SeqReversedWithGetItemIndex",
	                  &self->drwgii_seq, &self->drwgii_max, &self->drwgii_size))
		goto err;
	if unlikely(verify_max_and_size(self))
		goto err;
	seqtyp = Dee_TYPE(self->drwgii_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem_index) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	self->drwgii_tp_getitem_index = seqtyp->tp_seq->tp_getitem_index;
	Dee_Incref(self->drwgii_seq);
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rs_giif_init(DefaultReversed_WithGetItemIndex *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_SeqReversedWithGetItemIndexFast",
	                  &self->drwgii_seq, &self->drwgii_max, &self->drwgii_size))
		goto err;
	if unlikely(verify_max_and_size(self))
		goto err;
	seqtyp = Dee_TYPE(self->drwgii_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem_index_fast) &&
	    (!DeeType_InheritGetItem(seqtyp) || !seqtyp->tp_seq->tp_getitem_index_fast))
		goto err_no_getitem;
	self->drwgii_tp_getitem_index = seqtyp->tp_seq->tp_getitem_index_fast;
	Dee_Incref(self->drwgii_seq);
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rs_tgii_init(DefaultReversed_WithGetItemIndex *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_SeqReversedWithTryGetItemIndex",
	                  &self->drwgii_seq, &self->drwgii_max, &self->drwgii_size))
		goto err;
	if unlikely(verify_max_and_size(self))
		goto err;
	seqtyp = Dee_TYPE(self->drwgii_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_trygetitem_index) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	self->drwgii_tp_getitem_index = seqtyp->tp_seq->tp_trygetitem_index;
	Dee_Incref(self->drwgii_seq);
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

#define rs_giif_fini rs_gii_fini
#define rs_tgii_fini rs_gii_fini
PRIVATE NONNULL((1)) void DCALL
rs_gii_fini(DefaultReversed_WithGetItemIndex *__restrict self) {
	Dee_Decref(self->drwgii_seq);
}

#define rs_giif_visit rs_gii_visit
#define rs_tgii_visit rs_gii_visit
PRIVATE NONNULL((1, 2)) void DCALL
rs_gii_visit(DefaultReversed_WithGetItemIndex *__restrict self,
             Dee_visit_t proc, void *arg) {
	Dee_Visit(self->drwgii_seq);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rs_gii_enumerate_index(DefaultReversed_WithGetItemIndex *__restrict self,
                       Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	if (end > self->drwgii_size)
		end = self->drwgii_size;
	for (i = start; i < end; ++i) {
		size_t real_index = self->drwgii_max - i;
		DREF DeeObject *item;
		item = (*self->drwgii_tp_getitem_index)(self->drwgii_seq, real_index);
		if (item) {
			temp = (*proc)(arg, i, item);
			Dee_Decref(item);
		} else if (DeeError_Catch(&DeeError_IndexError) ||
		           DeeError_Catch(&DeeError_UnboundItem)) {
			temp = (*proc)(arg, i, NULL);
		} else {
			goto err;
		}
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rs_giif_enumerate_index(DefaultReversed_WithGetItemIndex *__restrict self,
                        Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	if (end > self->drwgii_size)
		end = self->drwgii_size;
	for (i = start; i < end; ++i) {
		size_t real_index = self->drwgii_max - i;
		DREF DeeObject *item;
		item = (*self->drwgii_tp_getitem_index)(self->drwgii_seq, real_index);
		temp = (*proc)(arg, i, item);
		Dee_XDecref(item);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rs_tgii_enumerate_index(DefaultReversed_WithGetItemIndex *__restrict self,
                        Dee_seq_enumerate_index_t proc, void *arg, size_t start, size_t end) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	if (end > self->drwgii_size)
		end = self->drwgii_size;
	for (i = start; i < end; ++i) {
		size_t real_index = self->drwgii_max - i;
		DREF DeeObject *item;
		item = (*self->drwgii_tp_getitem_index)(self->drwgii_seq, real_index);
		if (item == ITER_DONE) {
			temp = (*proc)(arg, i, NULL);
		} else if (item) {
			temp = (*proc)(arg, i, item);
			Dee_Decref(item);
		} else {
			goto err;
		}
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
err:
	return -1;
}

#define rs_gii_size_fast  rs_gii_size
#define rs_giif_size      rs_gii_size
#define rs_giif_size_fast rs_gii_size
#define rs_tgii_size      rs_gii_size
#define rs_tgii_size_fast rs_gii_size
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rs_gii_size(DefaultReversed_WithGetItemIndex *__restrict self) {
	return self->drwgii_size;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rs_gii_getitem_index(DefaultReversed_WithGetItemIndex *__restrict self, size_t index) {
	if unlikely(index >= self->drwgii_size)
		goto err_obb;
	return (*self->drwgii_tp_getitem_index)(self->drwgii_seq, self->drwgii_max - index);
err_obb:
	err_index_out_of_bounds((DeeObject *)self, index, self->drwgii_size);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rs_gii_trygetitem_index(DefaultReversed_WithGetItemIndex *__restrict self, size_t index) {
	if unlikely(index >= self->drwgii_size)
		return ITER_DONE;
	return DeeObject_TryGetItemIndex(self->drwgii_seq, self->drwgii_max - index);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rs_giif_getitem_index(DefaultReversed_WithGetItemIndex *__restrict self, size_t index) {
	DREF DeeObject *result;
	if unlikely(index >= self->drwgii_size)
		goto err_obb;
	result = (*self->drwgii_tp_getitem_index)(self->drwgii_seq, self->drwgii_max - index);
	if unlikely(!result)
		err_unbound_index((DeeObject *)self, index);
	return result;
err_obb:
	err_index_out_of_bounds((DeeObject *)self, index, self->drwgii_size);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rs_giif_getitem_index_fast(DefaultReversed_WithGetItemIndex *__restrict self, size_t index) {
	return (*self->drwgii_tp_getitem_index)(self->drwgii_seq, self->drwgii_max - index);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rs_giif_trygetitem_index(DefaultReversed_WithGetItemIndex *__restrict self, size_t index) {
	DREF DeeObject *result;
	if unlikely(index >= self->drwgii_size)
		return ITER_DONE;
	result = (*self->drwgii_tp_getitem_index)(self->drwgii_seq, self->drwgii_max - index);
	if unlikely(!result)
		result = ITER_DONE;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rs_tgii_getitem_index(DefaultReversed_WithGetItemIndex *__restrict self, size_t index) {
	if unlikely(index >= self->drwgii_size)
		goto err_obb;
	return DeeObject_GetItemIndex(self->drwgii_seq, self->drwgii_max - index);
err_obb:
	err_index_out_of_bounds((DeeObject *)self, index, self->drwgii_size);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rs_tgii_trygetitem_index(DefaultReversed_WithGetItemIndex *__restrict self, size_t index) {
	if unlikely(index >= self->drwgii_size)
		return ITER_DONE;
	return (*self->drwgii_tp_getitem_index)(self->drwgii_seq, self->drwgii_max - index);
}

#define rs_giif_bounditem_index rs_gii_bounditem_index
#define rs_tgii_bounditem_index rs_gii_bounditem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
rs_gii_bounditem_index(DefaultReversed_WithGetItemIndex *__restrict self, size_t index) {
	if unlikely(index >= self->drwgii_size)
		return Dee_BOUND_MISSING;
	return DeeObject_BoundItemIndex(self->drwgii_seq, self->drwgii_max - index);
}

#define rs_giif_hasitem_index rs_gii_hasitem_index
#define rs_tgii_hasitem_index rs_gii_hasitem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
rs_gii_hasitem_index(DefaultReversed_WithGetItemIndex *__restrict self, size_t index) {
	if unlikely(index >= self->drwgii_size)
		return 0;
	return DeeObject_HasItemIndex(self->drwgii_seq, self->drwgii_max - index);
}

#define rs_giif_getrange_index rs_gii_getrange_index
#define rs_tgii_getrange_index rs_gii_getrange_index
PRIVATE WUNUSED NONNULL((1)) DREF DefaultReversed_WithGetItemIndex *DCALL
rs_gii_getrange_index(DefaultReversed_WithGetItemIndex *__restrict self,
                      Dee_ssize_t start, Dee_ssize_t end) {
	struct Dee_seq_range range;
	DREF DefaultReversed_WithGetItemIndex *result;
	DeeSeqRange_Clamp(&range, start, end, self->drwgii_size);
	result = DeeObject_MALLOC(DefaultReversed_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	result->drwgii_tp_getitem_index = self->drwgii_tp_getitem_index;
	Dee_Incref(self->drwgii_seq);
	result->drwgii_seq  = self->drwgii_seq;
	result->drwgii_max  = self->drwgii_max - range.sr_start;
	result->drwgii_size = range.sr_end - range.sr_start;
	DeeObject_Init(result, Dee_TYPE(self));
	return result;
err:
	return NULL;
}

#define rs_giif_getrange_index_n rs_gii_getrange_index_n
#define rs_tgii_getrange_index_n rs_gii_getrange_index_n
PRIVATE WUNUSED NONNULL((1)) DREF DefaultReversed_WithGetItemIndex *DCALL
rs_gii_getrange_index_n(DefaultReversed_WithGetItemIndex *__restrict self,
                        Dee_ssize_t start) {
	DREF DefaultReversed_WithGetItemIndex *result;
	size_t used_start = DeeSeqRange_Clamp_n(start, self->drwgii_size);
	result = DeeObject_MALLOC(DefaultReversed_WithGetItemIndex);
	if unlikely(!result)
		goto err;
	result->drwgii_tp_getitem_index = self->drwgii_tp_getitem_index;
	Dee_Incref(self->drwgii_seq);
	result->drwgii_seq  = self->drwgii_seq;
	result->drwgii_max  = self->drwgii_max - used_start;
	result->drwgii_size = self->drwgii_size - used_start;
	DeeObject_Init(result, Dee_TYPE(self));
	return result;
err:
	return NULL;
}



PRIVATE struct type_seq rs_gii_seq = {
	/* .tp_iter                       = */ NULL,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_index_t, void *, size_t, size_t))&rs_gii_enumerate_index,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&rs_gii_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&rs_gii_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rs_gii_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&rs_gii_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&rs_gii_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&rs_gii_getrange_index,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&rs_gii_getrange_index_n,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rs_gii_trygetitem_index,
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


PRIVATE struct type_seq rs_giif_seq = {
	/* .tp_iter                       = */ NULL,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_index_t, void *, size_t, size_t))&rs_giif_enumerate_index,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&rs_giif_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&rs_giif_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rs_giif_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rs_giif_getitem_index_fast,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&rs_giif_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&rs_giif_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&rs_giif_getrange_index,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&rs_giif_getrange_index_n,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rs_giif_trygetitem_index,
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

PRIVATE struct type_seq rs_tgii_seq = {
	/* .tp_iter                       = */ NULL,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_index_t, void *, size_t, size_t))&rs_tgii_enumerate_index,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&rs_tgii_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&rs_tgii_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rs_tgii_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&rs_tgii_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&rs_tgii_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&rs_tgii_getrange_index,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&rs_tgii_getrange_index_n,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rs_tgii_trygetitem_index,
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


#define rs_giif_members rs_gii_members
#define rs_tgii_members rs_gii_members
PRIVATE struct type_member tpconst rs_gii_members[] = {
	TYPE_MEMBER_FIELD("__seq__", STRUCT_OBJECT, offsetof(DefaultReversed_WithGetItemIndex, drwgii_seq)),
	TYPE_MEMBER_FIELD("__max__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(DefaultReversed_WithGetItemIndex, drwgii_max)),
	TYPE_MEMBER_FIELD("__size__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(DefaultReversed_WithGetItemIndex, drwgii_size)),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject DefaultReversed_WithGetItemIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqReversedWithGetItemIndex",
	/* .tp_doc      = */ DOC("(objWithGetItem,max:?Dint,size:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&rs_gii_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&rs_gii_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&rs_gii_init,
				TYPE_FIXED_ALLOCATOR(DefaultReversed_WithGetItemIndex)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rs_gii_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rs_gii_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &rs_gii_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rs_gii_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};

INTERN DeeTypeObject DefaultReversed_WithGetItemIndexFast_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqReversedWithGetItemIndexFast",
	/* .tp_doc      = */ DOC("(objWithGetItemIndexFast,max:?Dint,size:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&rs_giif_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&rs_giif_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&rs_giif_init,
				TYPE_FIXED_ALLOCATOR(DefaultReversed_WithGetItemIndex)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rs_giif_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rs_giif_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &rs_giif_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rs_giif_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};

INTERN DeeTypeObject DefaultReversed_WithTryGetItemIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqReversedWithTryGetItemIndex",
	/* .tp_doc      = */ DOC("(objWithGetItem,max:?Dint,size:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&rs_tgii_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&rs_tgii_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&rs_tgii_init,
				TYPE_FIXED_ALLOCATOR(DefaultReversed_WithGetItemIndex)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rs_tgii_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rs_tgii_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &rs_tgii_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rs_tgii_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_REVERSED_C */
