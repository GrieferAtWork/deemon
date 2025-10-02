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

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>

/**/
#include "../../runtime/runtime_error.h"
#include "default-reversed.h"
/**/

#include <stddef.h> /* size_t */

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
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPxSIZ ":_SeqReversedWithGetItemIndex",
	                  &self->drwgii_seq, &self->drwgii_max, &self->drwgii_size))
		goto err;
	if unlikely(verify_max_and_size(self))
		goto err;
	seqtyp = Dee_TYPE(self->drwgii_seq);
	self->drwgii_tp_getitem_index = DeeType_RequireSupportedNativeOperator(seqtyp, getitem_index);
	if unlikely(!self->drwgii_tp_getitem_index)
		goto err_no_getitem;
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
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPxSIZ ":_SeqReversedWithGetItemIndexFast",
	                  &self->drwgii_seq, &self->drwgii_max, &self->drwgii_size))
		goto err;
	if unlikely(verify_max_and_size(self))
		goto err;
	seqtyp = Dee_TYPE(self->drwgii_seq);
	if unlikely(!seqtyp->tp_seq)
		goto err_no_getitem;
	self->drwgii_tp_getitem_index = seqtyp->tp_seq->tp_getitem_index_fast;
	if unlikely(!self->drwgii_tp_getitem_index)
		goto err_no_getitem;
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
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPxSIZ ":_SeqReversedWithTryGetItemIndex",
	                  &self->drwgii_seq, &self->drwgii_max, &self->drwgii_size))
		goto err;
	if unlikely(verify_max_and_size(self))
		goto err;
	seqtyp = Dee_TYPE(self->drwgii_seq);
	self->drwgii_tp_getitem_index = DeeType_RequireSupportedNativeOperator(seqtyp, trygetitem_index);
	if unlikely(!self->drwgii_tp_getitem_index)
		goto err_no_getitem;
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
rs_gii_mh_seq_enumerate_index(DefaultReversed_WithGetItemIndex *__restrict self,
                              Dee_seq_enumerate_index_t proc, void *arg,
                              size_t start, size_t end) {
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
rs_gii_mh_seq_enumerate_index_reverse(DefaultReversed_WithGetItemIndex *__restrict self,
                                      Dee_seq_enumerate_index_t proc, void *arg,
                                      size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	if (end > self->drwgii_size)
		end = self->drwgii_size;
	while (end > start) {
		size_t real_index = self->drwgii_max - (--end);
		DREF DeeObject *item;
		item = (*self->drwgii_tp_getitem_index)(self->drwgii_seq, real_index);
		if (item) {
			temp = (*proc)(arg, end, item);
			Dee_Decref(item);
		} else if (DeeError_Catch(&DeeError_IndexError) ||
		           DeeError_Catch(&DeeError_UnboundItem)) {
			temp = (*proc)(arg, end, NULL);
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
rs_giif_mh_seq_enumerate_index(DefaultReversed_WithGetItemIndex *__restrict self,
                               Dee_seq_enumerate_index_t proc, void *arg,
                               size_t start, size_t end) {
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
rs_giif_mh_seq_enumerate_index_reverse(DefaultReversed_WithGetItemIndex *__restrict self,
                                       Dee_seq_enumerate_index_t proc, void *arg,
                                       size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	if (end > self->drwgii_size)
		end = self->drwgii_size;
	while (end > start) {
		size_t real_index = self->drwgii_max - (--end);
		DREF DeeObject *item;
		item = (*self->drwgii_tp_getitem_index)(self->drwgii_seq, real_index);
		temp = (*proc)(arg, end, item);
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
rs_tgii_mh_seq_enumerate_index(DefaultReversed_WithGetItemIndex *__restrict self,
                               Dee_seq_enumerate_index_t proc, void *arg,
                               size_t start, size_t end) {
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rs_tgii_mh_seq_enumerate_index_reverse(DefaultReversed_WithGetItemIndex *__restrict self,
                                       Dee_seq_enumerate_index_t proc, void *arg,
                                       size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	if (end > self->drwgii_size)
		end = self->drwgii_size;
	while (end > start) {
		size_t real_index = self->drwgii_max - (--end);
		DREF DeeObject *item;
		item = (*self->drwgii_tp_getitem_index)(self->drwgii_seq, real_index);
		if (item == ITER_DONE) {
			temp = (*proc)(arg, end, NULL);
		} else if (item) {
			temp = (*proc)(arg, end, item);
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
	DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, self->drwgii_size);
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
		DeeRT_ErrUnboundIndex((DeeObject *)self, index);
	return result;
err_obb:
	DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, self->drwgii_size);
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
	DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, self->drwgii_size);
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



PRIVATE struct type_method tpconst rs_gii_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst rs_gii_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_enumerate_index, &rs_gii_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &rs_gii_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_seq rs_gii_seq = {
	/* .tp_iter                       = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__getrange__with__getrange_index__and__getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__seq_operator_foreach_pair__with__seq_operator_foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__bounditem_index),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&rs_gii_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&rs_gii_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rs_gii_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&rs_gii_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&rs_gii_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&rs_gii_getrange_index,
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&rs_gii_getrange_index_n,
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rs_gii_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};


#if 1
#define rs_giif_methods rs_gii_methods
#else
PRIVATE struct type_method tpconst rs_giif_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};
#endif

PRIVATE struct type_method_hint tpconst rs_giif_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_enumerate_index, &rs_giif_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &rs_giif_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_seq rs_giif_seq = {
	/* .tp_iter                       = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__getrange__with__getrange_index__and__getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__seq_operator_foreach_pair__with__seq_operator_foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__size__and__getitem_index_fast),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&rs_giif_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&rs_giif_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rs_giif_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rs_giif_getitem_index_fast,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&rs_giif_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&rs_giif_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&rs_giif_getrange_index,
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&rs_giif_getrange_index_n,
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rs_giif_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

#if 1
#define rs_tgii_methods rs_gii_methods
#else
PRIVATE struct type_method tpconst rs_tgii_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};
#endif

PRIVATE struct type_method_hint tpconst rs_tgii_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_enumerate_index, &rs_tgii_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &rs_tgii_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_seq rs_tgii_seq = {
	/* .tp_iter                       = */ DEFIMPL(&default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__getrange__with__getrange_index__and__getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__seq_operator_foreach_pair__with__seq_operator_foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__bounditem_index),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&rs_tgii_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&rs_tgii_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rs_tgii_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&rs_tgii_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&rs_tgii_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&rs_tgii_getrange_index,
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&rs_tgii_getrange_index_n,
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rs_tgii_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
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
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_size),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rs_gii_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__C2B62E6BCA44673D),
	/* .tp_seq           = */ &rs_gii_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rs_gii_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rs_gii_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ rs_gii_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_size),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rs_giif_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__7EA181D4706D1525),
	/* .tp_seq           = */ &rs_giif_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rs_giif_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rs_giif_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ rs_giif_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_size),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rs_tgii_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__C2B62E6BCA44673D),
	/* .tp_seq           = */ &rs_tgii_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rs_tgii_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rs_tgii_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ rs_tgii_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_REVERSED_C */
