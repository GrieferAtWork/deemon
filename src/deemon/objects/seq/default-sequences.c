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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SEQUENCES_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SEQUENCES_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack*, UNPuSIZ */
#include <deemon/bool.h>               /* Dee_True */
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>           /* DeeRT_Err* */
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/gc.h>                 /* DeeGCObject_MALLOC, DeeGC_TRACK */
#include <deemon/method-hints.h>       /* DeeObject_InvokeMethodHint, Dee_seq_enumerate_index_t, Dee_seq_enumerate_t, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/module.h>             /* DeeModule_CallExternStringf */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_ERR, Dee_BOUND_MISSING, Dee_Decref, Dee_Incref, Dee_TYPE, Dee_XDecref, Dee_foreach_t, Dee_ssize_t, Dee_visit_t, ITER_DONE, ITER_ISOK, OBJECT_HEAD_INIT, return_reference_ */
#include <deemon/operator-hints.h>     /* DeeNO_iter_next_t, DeeType_RequireSupportedNativeOperator */
#include <deemon/seq.h>                /* DeeSeqRange_Clamp, DeeSeqRange_Clamp_n, DeeSeq_*, Dee_seq_range */
#include <deemon/serial.h>             /* DeeSerial*, Dee_seraddr_t */
#include <deemon/thread.h>             /* DeeThread_CheckInterrupt */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_Type, METHOD_FNOREFESCAPE, OPERATOR_*, STRUCT_*, TF_NONE, TP_FFINAL, TP_FNORMAL, TYPE_MEMBER*, TYPE_METHOD_END, type_* */
#include <deemon/util/lock.h>          /* Dee_atomic_lock_init */

#include <hybrid/overflow.h> /* OVERFLOW_UADD, OVERFLOW_USUB */

#include "../../runtime/method-hint-defaults.h"
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "default-iterators.h"
#include "default-sequences.h"

#include <stddef.h> /* NULL, offsetof, size_t */

DECL_BEGIN

/************************************************************************/
/* DefaultSequence_WithSizeAndGetItemIndex_Type                         */
/* DefaultSequence_WithSizeAndGetItemIndexFast_Type                     */
/* DefaultSequence_WithSizeAndTryGetItemIndex_Type                      */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultSequence_WithSizeAndGetItemIndex, dssgi_seq) == offsetof(ProxyObject, po_obj));
#define ds_sgi_fini   generic_proxy__fini
#define ds_sgif_fini  generic_proxy__fini
#define ds_stgi_fini  generic_proxy__fini
#define ds_sgi_visit  generic_proxy__visit
#define ds_sgif_visit generic_proxy__visit
#define ds_stgi_visit generic_proxy__visit

#define ds_sgif_copy ds_sgi_copy
#define ds_stgi_copy ds_sgi_copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_sgi_copy(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
            DefaultSequence_WithSizeAndGetItemIndex *__restrict other) {
	Dee_Incref(other->dssgi_seq);
	self->dssgi_seq              = other->dssgi_seq;
	self->dssgi_tp_getitem_index = other->dssgi_tp_getitem_index;
	self->dssgi_start            = other->dssgi_start;
	self->dssgi_end              = other->dssgi_end;
	return 0;
}

#define ds_sgif_deepcopy ds_sgi_deepcopy
#define ds_stgi_deepcopy ds_sgi_deepcopy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_sgi_deepcopy(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                DefaultSequence_WithSizeAndGetItemIndex *__restrict other) {
	self->dssgi_seq = DeeObject_DeepCopy(other->dssgi_seq);
	if unlikely(!self->dssgi_seq)
		goto err;
	self->dssgi_tp_getitem_index = other->dssgi_tp_getitem_index;
	self->dssgi_start            = other->dssgi_start;
	self->dssgi_end              = other->dssgi_end;
	return 0;
err:
	return -1;
}

#define ds_sgif_serialize ds_sgi_serialize
#define ds_stgi_serialize ds_sgi_serialize
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_sgi_serialize(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                 DeeSerial *__restrict writer, Dee_seraddr_t addr) {
	DefaultSequence_WithSizeAndGetItemIndex *out;
#define ADDROF(field) (addr + offsetof(DefaultSequence_WithSizeAndGetItemIndex, field))
	if (DeeSerial_PutObject(writer, ADDROF(dssgi_seq), self->dssgi_seq))
		goto err;
	if (DeeSerial_PutFuncPtr(writer, ADDROF(dssgi_tp_getitem_index), self->dssgi_tp_getitem_index))
		goto err;
	out = DeeSerial_Addr2Mem(writer, addr, DefaultSequence_WithSizeAndGetItemIndex);
	out->dssgi_start = self->dssgi_start;
	out->dssgi_end   = self->dssgi_end;
	return 0;
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ds_sgi_init(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
            size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_SeqWithSizeAndGetItemIndex",
	                  &self->dssgi_seq, &self->dssgi_start, &self->dssgi_end))
		goto err;
	seqtyp = Dee_TYPE(self->dssgi_seq);
	self->dssgi_tp_getitem_index = DeeType_RequireSupportedNativeOperator(seqtyp, getitem_index);
	if unlikely(!self->dssgi_tp_getitem_index)
		goto err_no_getitem;
	Dee_Incref(self->dssgi_seq);
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ds_sgif_init(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_SeqWithSizeAndGetItemIndexFast",
	                  &self->dssgi_seq, &self->dssgi_start, &self->dssgi_end))
		goto err;
	seqtyp = Dee_TYPE(self->dssgi_seq);
	if unlikely(!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem_index_fast)
		goto err_no_getitem;
	self->dssgi_tp_getitem_index = seqtyp->tp_seq->tp_getitem_index_fast;
	Dee_Incref(self->dssgi_seq);
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ds_stgi_init(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_SeqWithSizeAndTryGetItemIndex",
	                  &self->dssgi_seq, &self->dssgi_start, &self->dssgi_end))
		goto err;
	seqtyp = Dee_TYPE(self->dssgi_seq);
	self->dssgi_tp_getitem_index = DeeType_RequireSupportedNativeOperator(seqtyp, trygetitem_index);
	if unlikely(!self->dssgi_tp_getitem_index)
		goto err_no_getitem;
	Dee_Incref(self->dssgi_seq);
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
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

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeAndGetItemIndex *DCALL
ds_stgi_iter(DefaultSequence_WithSizeAndGetItemIndex *__restrict self) {
	DREF DefaultIterator_WithSizeAndGetItemIndex *result;
	result = DeeObject_MALLOC(DefaultIterator_WithSizeAndGetItemIndex);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dssgi_seq);
	result->disgi_seq              = self->dssgi_seq;
	result->disgi_tp_getitem_index = self->dssgi_tp_getitem_index;
	result->disgi_index            = self->dssgi_start;
	result->disgi_end              = self->dssgi_end;
	DeeObject_Init(result, &DefaultIterator_WithSizeAndTryGetItemIndex_Type);
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
ds_sgi_mg_seq_foreach_reverse(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                              Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i = self->dssgi_end;
	while (i > self->dssgi_start) {
		DREF DeeObject *elem;
		--i;
		elem = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, i);
		if (!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem))
				continue;
			if (DeeError_Catch(&DeeError_IndexError))
				continue;
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
ds_sgi_mh_seq_enumerate_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                              Dee_seq_enumerate_index_t proc, void *arg,
                              size_t start, size_t end) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	if (OVERFLOW_UADD(start, self->dssgi_start, &start))
		return 0;
	if (OVERFLOW_UADD(end, self->dssgi_start, &end))
		end = (size_t)-1;
	if (end > self->dssgi_end)
		end = self->dssgi_end;
	for (i = start; i < end; ++i) {
		DREF DeeObject *elem;
		elem = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, i);
		if (!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				temp = (*proc)(arg, i - self->dssgi_start, NULL);
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				return -1;
			}
		} else {
			temp = (*proc)(arg, i - self->dssgi_start, elem);
			Dee_Decref(elem);
		}
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_sgi_mh_seq_enumerate_index_reverse(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                                      Dee_seq_enumerate_index_t proc, void *arg,
                                      size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	if (OVERFLOW_UADD(start, self->dssgi_start, &start))
		return 0;
	if (OVERFLOW_UADD(end, self->dssgi_start, &end))
		end = (size_t)-1;
	if (end > self->dssgi_end)
		end = self->dssgi_end;
	while (end > start) {
		DREF DeeObject *elem;
		elem = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, --end);
		if (!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				temp = (*proc)(arg, end - self->dssgi_start, NULL);
			} else if (DeeError_Catch(&DeeError_IndexError)) {
				temp = 0;
			} else {
				return -1;
			}
		} else {
			temp = (*proc)(arg, end - self->dssgi_start, elem);
			Dee_Decref(elem);
		}
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_sgif_mh_seq_foreach_reverse(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                               Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i = self->dssgi_end;
	while (i > self->dssgi_start) {
		DREF DeeObject *elem;
		--i;
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_sgif_mh_seq_enumerate_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                               Dee_seq_enumerate_index_t proc, void *arg,
                               size_t start, size_t end) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	if (OVERFLOW_UADD(start, self->dssgi_start, &start))
		return 0;
	if (OVERFLOW_UADD(end, self->dssgi_start, &end))
		end = (size_t)-1;
	if (end > self->dssgi_end)
		end = self->dssgi_end;
	for (i = start; i < end; ++i) {
		DREF DeeObject *elem;
		elem = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, i);
		temp = (*proc)(arg, i - self->dssgi_start, elem);
		Dee_XDecref(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_sgif_mh_seq_enumerate_index_reverse(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                                       Dee_seq_enumerate_index_t proc, void *arg,
                                       size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	if (OVERFLOW_UADD(start, self->dssgi_start, &start))
		return 0;
	if (OVERFLOW_UADD(end, self->dssgi_start, &end))
		end = (size_t)-1;
	if (end > self->dssgi_end)
		end = self->dssgi_end;
	while (end > start) {
		DREF DeeObject *elem;
		elem = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, --end);
		temp = (*proc)(arg, end - self->dssgi_start, elem);
		Dee_XDecref(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_stgi_foreach(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                Dee_foreach_t proc, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	for (i = self->dssgi_start; i < self->dssgi_end; ++i) {
		DREF DeeObject *elem;
		elem = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, i);
		if (!ITER_ISOK(elem)) {
			if (elem == ITER_DONE)
				continue; /* Unbound item. */
			goto err;
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
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_stgi_mh_seq_foreach_reverse(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                               Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i = self->dssgi_end;
	while (i > self->dssgi_start) {
		DREF DeeObject *elem;
		--i;
		elem = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, i);
		if (!ITER_ISOK(elem)) {
			if (elem == ITER_DONE)
				continue; /* Unbound item. */
			goto err;
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
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_stgi_mh_seq_enumerate_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                               Dee_seq_enumerate_index_t proc, void *arg,
                               size_t start, size_t end) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	if (OVERFLOW_UADD(start, self->dssgi_start, &start))
		return 0;
	if (OVERFLOW_UADD(end, self->dssgi_start, &end))
		end = (size_t)-1;
	if (end > self->dssgi_end)
		end = self->dssgi_end;
	for (i = start; i < end; ++i) {
		DREF DeeObject *elem;
		elem = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, i);
		if (ITER_ISOK(elem)) {
			temp = (*proc)(arg, i - self->dssgi_start, elem);
			Dee_Decref(elem);
		} else {
			if (!elem)
				goto err;
			temp = (*proc)(arg, i - self->dssgi_start, NULL);
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
ds_stgi_mh_seq_enumerate_index_reverse(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                                       Dee_seq_enumerate_index_t proc, void *arg,
                                       size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	if (OVERFLOW_UADD(start, self->dssgi_start, &start))
		return 0;
	if (OVERFLOW_UADD(end, self->dssgi_start, &end))
		end = (size_t)-1;
	if (end > self->dssgi_end)
		end = self->dssgi_end;
	while (end > start) {
		DREF DeeObject *elem;
		elem = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, --end);
		if (ITER_ISOK(elem)) {
			temp = (*proc)(arg, end - self->dssgi_start, elem);
			Dee_Decref(elem);
		} else {
			if (!elem)
				goto err;
			temp = (*proc)(arg, end - self->dssgi_start, NULL);
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

#define ds_sgif_size ds_sgi_size
#define ds_stgi_size ds_sgi_size
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
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, ds_sgi_size(self));
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_sgif_getitem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	DREF DeeObject *result;
	size_t used_index;
	if (OVERFLOW_UADD(index, self->dssgi_start, &used_index))
		goto err_obb;
	if (used_index >= self->dssgi_end)
		goto err_obb;
	result = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, used_index);
	if unlikely(result == NULL)
		DeeRT_ErrUnboundIndex(self->dssgi_seq, used_index);
	return result;
err_obb:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, ds_sgif_size(self));
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_stgi_getitem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	DREF DeeObject *result;
	size_t used_index;
	if (OVERFLOW_UADD(index, self->dssgi_start, &used_index))
		goto err_obb;
	if (used_index >= self->dssgi_end)
		goto err_obb;
	result = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, used_index);
	if unlikely(result == ITER_DONE) {
		DeeRT_ErrUnboundIndex(self->dssgi_seq, used_index);
		result = NULL;
	}
	return result;
err_obb:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, ds_stgi_size(self));
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_sgi_trygetitem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	DREF DeeObject *result;
	size_t used_index;
	if unlikely(OVERFLOW_UADD(index, self->dssgi_start, &used_index))
		goto err_obb;
	if unlikely(used_index >= self->dssgi_end)
		goto err_obb;
	result = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, used_index);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_IndexError) ||
		    DeeError_Catch(&DeeError_UnboundItem))
			result = ITER_DONE;
	}
	return result;
err_obb:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_sgif_trygetitem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	DREF DeeObject *result;
	size_t used_index;
	if (OVERFLOW_UADD(index, self->dssgi_start, &used_index))
		goto err_obb;
	if (used_index >= self->dssgi_end)
		goto err_obb;
	result = (*self->dssgi_tp_getitem_index)(self->dssgi_seq, used_index);
	if unlikely(result == NULL)
		result = ITER_DONE;
	return result;
err_obb:
	return ITER_DONE;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_stgi_trygetitem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	size_t used_index;
	if (OVERFLOW_UADD(index, self->dssgi_start, &used_index))
		goto err_obb;
	if (used_index >= self->dssgi_end)
		goto err_obb;
	return (*self->dssgi_tp_getitem_index)(self->dssgi_seq, used_index);
err_obb:
	return ITER_DONE;
}

#define ds_sgif_delitem_index ds_sgi_delitem_index
#define ds_stgi_delitem_index ds_sgi_delitem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
ds_sgi_delitem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	size_t used_index;
	if (OVERFLOW_UADD(index, self->dssgi_start, &used_index))
		goto err_obb;
	if (used_index >= self->dssgi_end)
		goto err_obb;
	return DeeObject_InvokeMethodHint(seq_operator_delitem_index, self->dssgi_seq, used_index);
err_obb:
	return DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, ds_sgi_size(self));
}

#define ds_sgif_setitem_index ds_sgi_setitem_index
#define ds_stgi_setitem_index ds_sgi_setitem_index
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
ds_sgi_setitem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
                     size_t index, DeeObject *value) {
	size_t used_index;
	if (OVERFLOW_UADD(index, self->dssgi_start, &used_index))
		goto err_obb;
	if (used_index >= self->dssgi_end)
		goto err_obb;
	return DeeObject_InvokeMethodHint(seq_operator_setitem_index, self->dssgi_seq, used_index, value);
err_obb:
	return DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, ds_sgi_size(self));
}


/* // Can't be used because our end offset may be greater than `Sequence.length(dssgi_seq)'
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_sgif_getitem_index_fast(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	ASSERT(index < (self->dssgi_end - self->dssgi_start));
	return (*self->dssgi_tp_getitem_index)(self->dssgi_seq, self->dssgi_start + index);
}
*/

#define ds_sgif_bounditem_index ds_sgi_bounditem_index
#define ds_stgi_bounditem_index ds_sgi_bounditem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
ds_sgi_bounditem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	size_t used_index;
	if (OVERFLOW_UADD(index, self->dssgi_start, &used_index))
		goto err_obb;
	if (used_index >= self->dssgi_end)
		goto err_obb;
	return DeeObject_InvokeMethodHint(seq_operator_bounditem_index, self->dssgi_seq, used_index);
err_obb:
	return Dee_BOUND_MISSING; /* Item doesn't exist */
}

#define ds_sgif_hasitem_index ds_sgi_hasitem_index
#define ds_stgi_hasitem_index ds_sgi_hasitem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
ds_sgi_hasitem_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, size_t index) {
	size_t used_index;
	if (OVERFLOW_UADD(index, self->dssgi_start, &used_index))
		goto err_obb;
	if (used_index >= self->dssgi_end)
		goto err_obb;
	return DeeObject_InvokeMethodHint(seq_operator_hasitem_index, self->dssgi_seq, used_index);
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
ds_stgi_getrange_index(DefaultSequence_WithSizeAndGetItemIndex *__restrict self,
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
	DeeObject_Init(result, &DefaultSequence_WithSizeAndTryGetItemIndex_Type);
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

PRIVATE WUNUSED NONNULL((1)) DREF DefaultSequence_WithSizeAndGetItemIndex *DCALL
ds_stgi_getrange_index_n(DefaultSequence_WithSizeAndGetItemIndex *__restrict self, Dee_ssize_t start) {
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
	DeeObject_Init(result, &DefaultSequence_WithSizeAndTryGetItemIndex_Type);
	return result;
err:
	return NULL;
}

#define ds_sgif_members ds_sgi_members
#define ds_stgi_members ds_sgi_members
PRIVATE struct type_member tpconst ds_sgi_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(DefaultSequence_WithSizeAndGetItemIndex, dssgi_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DefaultSequence_WithSizeAndGetItemIndex, dssgi_start)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DefaultSequence_WithSizeAndGetItemIndex, dssgi_end)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst ds_sgi_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeAndGetItemIndex_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst ds_sgif_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeAndGetItemIndexFast_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst ds_stgi_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeAndTryGetItemIndex_Type),
	TYPE_MEMBER_END
};


#define ds_sgif_methods ds_sgi_methods
#define ds_stgi_methods ds_sgi_methods
PRIVATE struct type_method tpconst ds_sgi_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst ds_sgi_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_enumerate_index, &ds_sgi_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &ds_sgi_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &ds_sgi_mg_seq_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_method_hint tpconst ds_sgif_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_enumerate_index, &ds_sgif_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &ds_sgif_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &ds_sgif_mh_seq_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_method_hint tpconst ds_stgi_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_enumerate_index, &ds_stgi_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &ds_stgi_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &ds_stgi_mh_seq_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};


PRIVATE struct type_seq ds_sgi_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_sgi_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__delitem__with__delitem_index),
	/* .tp_setitem                    = */ DEFIMPL(&default__setitem__with__setitem_index),
	/* .tp_getrange                   = */ DEFIMPL(&default__getrange__with__getrange_index__and__getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ds_sgi_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__bounditem_index),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_sgi_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_sgi_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_sgi_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&ds_sgi_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&ds_sgi_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&ds_sgi_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&ds_sgi_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&ds_sgi_getrange_index,
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&ds_sgi_getrange_index_n,
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_sgi_trygetitem_index,
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

PRIVATE struct type_seq ds_sgif_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_sgif_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__delitem__with__delitem_index),
	/* .tp_setitem                    = */ DEFIMPL(&default__setitem__with__setitem_index),
	/* .tp_getrange                   = */ DEFIMPL(&default__getrange__with__getrange_index__and__getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ds_sgif_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__bounditem_index),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_sgif_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_sgif_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_sgif_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL, /*(DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_sgif_getitem_index_fast,*/
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&ds_sgif_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&ds_sgif_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&ds_sgif_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&ds_sgif_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&ds_sgif_getrange_index,
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&ds_sgif_getrange_index_n,
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_sgif_trygetitem_index,
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

PRIVATE struct type_seq ds_stgi_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_stgi_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__delitem__with__delitem_index),
	/* .tp_setitem                    = */ DEFIMPL(&default__setitem__with__setitem_index),
	/* .tp_getrange                   = */ DEFIMPL(&default__getrange__with__getrange_index__and__getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ds_stgi_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__bounditem_index),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_stgi_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_stgi_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_stgi_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&ds_stgi_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&ds_stgi_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&ds_stgi_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&ds_stgi_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&ds_stgi_getrange_index,
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&ds_stgi_getrange_index_n,
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_stgi_trygetitem_index,
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

INTERN DeeTypeObject DefaultSequence_WithSizeAndGetItemIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqWithSizeAndGetItemIndex",
	/* .tp_doc      = */ DOC("(objWithGetItem,start:?Dint,end:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultSequence_WithSizeAndGetItemIndex,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &ds_sgi_copy,
			/* tp_deep_ctor:   */ &ds_sgi_deepcopy,
			/* tp_any_ctor:    */ &ds_sgi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &ds_sgi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_sgi_fini,
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ds_sgi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__247219960F1E745D),
	/* .tp_seq           = */ &ds_sgi_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ ds_sgi_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_sgi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ds_sgi_class_members,
	/* .tp_method_hints  = */ ds_sgi_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject DefaultSequence_WithSizeAndGetItemIndexFast_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqWithSizeAndGetItemIndexFast",
	/* .tp_doc      = */ DOC("(objWithGetItemIndexFast,start:?Dint,end:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultSequence_WithSizeAndGetItemIndex,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &ds_sgif_copy,
			/* tp_deep_ctor:   */ &ds_sgif_deepcopy,
			/* tp_any_ctor:    */ &ds_sgif_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &ds_sgif_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_sgif_fini,
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ds_sgif_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__247219960F1E745D),
	/* .tp_seq           = */ &ds_sgif_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ ds_sgif_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_sgif_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ds_sgif_class_members,
	/* .tp_method_hints  = */ ds_sgif_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject DefaultSequence_WithSizeAndTryGetItemIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqWithSizeAndTryGetItemIndex",
	/* .tp_doc      = */ DOC("(objWithTryGetItem,start:?Dint,end:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultSequence_WithSizeAndGetItemIndex,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &ds_stgi_copy,
			/* tp_deep_ctor:   */ &ds_stgi_deepcopy,
			/* tp_any_ctor:    */ &ds_stgi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &ds_stgi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_stgi_fini,
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ds_stgi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__247219960F1E745D),
	/* .tp_seq           = */ &ds_stgi_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ ds_stgi_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_stgi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ds_stgi_class_members,
	/* .tp_method_hints  = */ ds_stgi_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};
























/************************************************************************/
/* DefaultSequence_WithSizeObAndGetItem_Type                            */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultSequence_WithSizeObAndGetItem, dssg_seq) == offsetof(ProxyObject3, po_obj1) ||
              offsetof(DefaultSequence_WithSizeObAndGetItem, dssg_seq) == offsetof(ProxyObject3, po_obj2) ||
              offsetof(DefaultSequence_WithSizeObAndGetItem, dssg_seq) == offsetof(ProxyObject3, po_obj3));
STATIC_ASSERT(offsetof(DefaultSequence_WithSizeObAndGetItem, dssg_start) == offsetof(ProxyObject3, po_obj1) ||
              offsetof(DefaultSequence_WithSizeObAndGetItem, dssg_start) == offsetof(ProxyObject3, po_obj2) ||
              offsetof(DefaultSequence_WithSizeObAndGetItem, dssg_start) == offsetof(ProxyObject3, po_obj3));
STATIC_ASSERT(offsetof(DefaultSequence_WithSizeObAndGetItem, dssg_end) == offsetof(ProxyObject3, po_obj1) ||
              offsetof(DefaultSequence_WithSizeObAndGetItem, dssg_end) == offsetof(ProxyObject3, po_obj2) ||
              offsetof(DefaultSequence_WithSizeObAndGetItem, dssg_end) == offsetof(ProxyObject3, po_obj3));
#define ds_sg_fini  generic_proxy3__fini
#define ds_sg_visit generic_proxy3__visit

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_sg_copy(DefaultSequence_WithSizeObAndGetItem *__restrict self,
           DefaultSequence_WithSizeObAndGetItem *__restrict other) {
	Dee_Incref(other->dssg_seq);
	Dee_Incref(other->dssg_start);
	Dee_Incref(other->dssg_end);
	self->dssg_seq        = other->dssg_seq;
	self->dssg_start      = other->dssg_start;
	self->dssg_end        = other->dssg_end;
	self->dssg_tp_getitem = other->dssg_tp_getitem;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_sg_deepcopy(DefaultSequence_WithSizeObAndGetItem *__restrict self,
               DefaultSequence_WithSizeObAndGetItem *__restrict other) {
	self->dssg_seq = DeeObject_DeepCopy(other->dssg_seq);
	if unlikely(!self->dssg_seq)
		goto err;
	self->dssg_start = DeeObject_DeepCopy(other->dssg_start);
	if unlikely(!self->dssg_start)
		goto err_seq;
	self->dssg_end = DeeObject_DeepCopy(other->dssg_end);
	if unlikely(!self->dssg_end)
		goto err_seq_start;
	self->dssg_tp_getitem = other->dssg_tp_getitem;
	return 0;
err_seq_start:
	Dee_Decref(self->dssg_start);
err_seq:
	Dee_Decref(self->dssg_seq);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_sg_serialize(DefaultSequence_WithSizeObAndGetItem *__restrict self,
                DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DefaultSequence_WithSizeObAndGetItem, field))
	int result = DeeSerial_PutObject(writer, ADDROF(dssg_seq), self->dssg_seq);
	if likely(result == 0)
		result = DeeSerial_PutObject(writer, ADDROF(dssg_start), self->dssg_start);
	if likely(result == 0)
		result = DeeSerial_PutObject(writer, ADDROF(dssg_end), self->dssg_end);
	if likely(result == 0)
		result = DeeSerial_PutFuncPtr(writer, ADDROF(dssg_tp_getitem), self->dssg_tp_getitem);
	return result;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ds_sg_init(DefaultSequence_WithSizeObAndGetItem *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	DeeArg_Unpack3(err, argc, argv, "_SeqWithSizeObAndGetItem",
	                &self->dssg_seq, &self->dssg_start, &self->dssg_end);
	seqtyp = Dee_TYPE(self->dssg_seq);
	self->dssg_tp_getitem = DeeType_RequireSupportedNativeOperator(seqtyp, getitem);
	if unlikely(!self->dssg_tp_getitem)
		goto err_no_getitem;
	Dee_Incref(self->dssg_seq);
	Dee_Incref(self->dssg_start);
	Dee_Incref(self->dssg_end);
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultIterator_WithSizeObAndGetItem *DCALL
ds_sg_iter(DefaultSequence_WithSizeObAndGetItem *__restrict self) {
	DREF DefaultIterator_WithSizeObAndGetItem *result;
	result = DeeGCObject_MALLOC(DefaultIterator_WithSizeObAndGetItem);
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
	DeeObject_Init(result, &DefaultIterator_WithSizeObAndGetItem_Type);
	return DeeGC_TRACK(DefaultIterator_WithSizeObAndGetItem, result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_sg_sizeob(DefaultSequence_WithSizeObAndGetItem *__restrict self) {
	return DeeObject_Sub(self->dssg_end, self->dssg_start);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ds_sg_mapindex(DefaultSequence_WithSizeObAndGetItem *self, DeeObject *index) {
	int temp;
	DREF DeeObject *used_index;
	used_index = DeeObject_Add(self->dssg_start, index);
	if unlikely(!used_index)
		goto err;
	temp = DeeObject_CmpGeAsBool(used_index, self->dssg_end);
	if unlikely(temp < 0)
		goto err_used_index;
	return used_index;
err_used_index:
	Dee_Decref(used_index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ds_sg_getitem(DefaultSequence_WithSizeObAndGetItem *self, DeeObject *index) {
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_sg_delitem(DefaultSequence_WithSizeObAndGetItem *self, DeeObject *index) {
	int result;
	DREF DeeObject *used_index = ds_sg_mapindex(self, index);
	if unlikely(!used_index)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_operator_delitem, self->dssg_seq, used_index);
	Dee_Decref(used_index);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
ds_sg_setitem(DefaultSequence_WithSizeObAndGetItem *self,
              DeeObject *index, DeeObject *value) {
	int result;
	DREF DeeObject *used_index = ds_sg_mapindex(self, index);
	if unlikely(!used_index)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_operator_setitem, self->dssg_seq, used_index, value);
	Dee_Decref(used_index);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DefaultSequence_WithSizeObAndGetItem *DCALL
ds_sg_getrange(DefaultSequence_WithSizeObAndGetItem *self, DeeObject *start, DeeObject *end) {
	int temp;
	DREF DefaultSequence_WithSizeObAndGetItem *result;
	DREF DeeObject *clamed_start_and_end_pair[2];
	DREF DeeObject *clamed_start_and_end;
	DREF DeeObject *sizeob = ds_sg_sizeob(self);
	if unlikely(!sizeob)
		goto err;

	/* Make a call to "util.clamprange()" to do the range-fixup. */
	clamed_start_and_end = DeeModule_CallExternStringf("util", "clamprange", "ooo", start, end, sizeob);
	Dee_Decref(sizeob);
	if unlikely(!clamed_start_and_end)
		goto err;
	temp = DeeSeq_Unpack(clamed_start_and_end, 2, clamed_start_and_end_pair);
	Dee_Decref(clamed_start_and_end);
	if unlikely(temp)
		goto err;
	Dee_Incref(self->dssg_seq);
	result = DeeObject_MALLOC(DefaultSequence_WithSizeObAndGetItem);
	if unlikely(!result)
		goto err_clamed_start_and_end_pair;
	result->dssg_seq        = self->dssg_seq;
	result->dssg_start      = clamed_start_and_end_pair[0]; /* Inherit reference */
	result->dssg_end        = clamed_start_and_end_pair[1]; /* Inherit reference */
	result->dssg_tp_getitem = self->dssg_tp_getitem;
	DeeObject_Init(result, &DefaultSequence_WithSizeObAndGetItem_Type);
	return result;
err_clamed_start_and_end_pair:
	Dee_Decref(clamed_start_and_end_pair[1]);
	Dee_Decref(clamed_start_and_end_pair[0]);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_sg_foreach(DefaultSequence_WithSizeObAndGetItem *self, Dee_foreach_t proc, void *arg) {
	DREF DeeObject *index;
	Dee_ssize_t temp, result = 0;
	int error;
	error = DeeObject_CmpGeAsBool(self->dssg_start, self->dssg_end);
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
			error = DeeObject_CmpGeAsBool(index, self->dssg_end);
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
ds_sg_mh_seq_foreach_reverse(DefaultSequence_WithSizeObAndGetItem *self,
                             Dee_foreach_t proc, void *arg) {
	DREF DeeObject *index;
	Dee_ssize_t temp, result = 0;
	int error;
	error = DeeObject_CmpLoAsBool(self->dssg_end, self->dssg_start);
	if unlikely(error < 0)
		goto err;
	if (!error) {
		index = self->dssg_end;
		Dee_Incref(index);
		for (;;) {
			DREF DeeObject *elem;
			if unlikely(DeeObject_Dec(&index))
				goto err_index;
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
			error = DeeObject_CmpLoAsBool(index, self->dssg_start);
			if unlikely(error < 0)
				goto err_index;
			if (!error)
				break;
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
ds_sg_mh_seq_enumerate(DefaultSequence_WithSizeObAndGetItem *self,
                       Dee_seq_enumerate_t proc, void *arg) {
	DREF DeeObject *index;
	Dee_ssize_t temp, result = 0;
	int error;
	error = DeeObject_CmpGeAsBool(self->dssg_start, self->dssg_end);
	if unlikely(error < 0)
		goto err;
	if (!error) {
		index = self->dssg_start;
		Dee_Incref(index);
		for (;;) {
			DREF DeeObject *elem;
			elem = (*self->dssg_tp_getitem)(self->dssg_seq, index);
			if unlikely(!elem) {
				if (DeeError_Catch(&DeeError_UnboundItem)) {
					temp = (*proc)(arg, index, NULL);
				} else {
					if (DeeError_Catch(&DeeError_IndexError))
						break;
					goto err_index;
				}
			} else {
				temp = (*proc)(arg, index, elem);
				Dee_Decref(elem);
			}
			if unlikely(temp < 0)
				goto err_index_temp;
			result += temp;
			if (DeeThread_CheckInterrupt())
				goto err_index;
			error = DeeObject_CmpGeAsBool(index, self->dssg_end);
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
ds_sg_mh_seq_enumerate_index(DefaultSequence_WithSizeObAndGetItem *self,
                             Dee_seq_enumerate_index_t proc,
                             void *arg, size_t start, size_t end) {
	DREF DeeObject *index, *endindex;
	Dee_ssize_t temp, result = 0;
	int error;
	index = self->dssg_start;
	if (start != 0) {
		index = DeeObject_AddSize(index, start);
		if unlikely(!index)
			goto err;
	} else {
		Dee_Incref(index);
	}
	endindex = self->dssg_end;
	if (end != (size_t)-1) {
		DREF DeeObject *wanted_end;
		wanted_end = DeeObject_AddSize(self->dssg_start, end);
		if unlikely(!wanted_end)
			goto err_index;
		/* if (endindex > wanted_end)
		 *     endindex = wanted_end; */
		error = DeeObject_CmpGrAsBool(endindex, wanted_end);
		if unlikely(error == 0) {
			Dee_Decref(wanted_end);
			Dee_Incref(endindex);
		} else {
			endindex = wanted_end;
			if unlikely(error < 0)
				goto err_index_endindex;
		}
	} else {
		Dee_Incref(endindex);
	}
	for (;;) {
		size_t index_value;
		DREF DeeObject *elem;
		error = DeeObject_CmpGeAsBool(index, endindex);
		if unlikely(error < 0)
			goto err_index_endindex;
		if (!error)
			break;
		if unlikely(DeeObject_AsSize(index, &index_value))
			goto err_index_endindex;
		elem = (*self->dssg_tp_getitem)(self->dssg_seq, index);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				temp = (*proc)(arg, index_value, NULL);
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err_index_endindex;
			}
		} else {
			temp = (*proc)(arg, index_value, elem);
			Dee_Decref(elem);
		}
		if unlikely(temp < 0)
			goto err_temp_index_endindex;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_index_endindex;
		if unlikely(DeeObject_Inc(&index))
			goto err_index_endindex;
	}
	Dee_Decref(endindex);
	Dee_Decref(index);
	return result;
err_temp_index_endindex:
	Dee_Decref(endindex);
/*err_temp_index:*/
	Dee_Decref(index);
	return temp;
err_index_endindex:
	Dee_Decref(endindex);
err_index:
	Dee_Decref(index);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_sg_mh_seq_enumerate_index_reverse(DefaultSequence_WithSizeObAndGetItem *self,
                                     Dee_seq_enumerate_index_t proc,
                                     void *arg, size_t start, size_t end) {
	DREF DeeObject *startindex, *endindex;
	Dee_ssize_t temp, result = 0;
	int error;
	startindex = self->dssg_start;
	if (start != 0) {
		startindex = DeeObject_AddSize(startindex, start);
		if unlikely(!startindex)
			goto err;
	} else {
		Dee_Incref(startindex);
	}
	endindex = self->dssg_end;
	if (end != (size_t)-1) {
		DREF DeeObject *wanted_end;
		wanted_end = DeeObject_AddSize(self->dssg_start, end);
		if unlikely(!wanted_end)
			goto err_startindex;
		/* if (endindex > wanted_end)
		 *     endindex = wanted_end; */
		error = DeeObject_CmpGrAsBool(endindex, wanted_end);
		if unlikely(error == 0) {
			Dee_Decref(wanted_end);
			Dee_Incref(endindex);
		} else {
			endindex = wanted_end;
			if unlikely(error < 0)
				goto err_startindex_endindex;
		}
	} else {
		Dee_Incref(endindex);
	}
	for (;;) {
		size_t index_value;
		DREF DeeObject *elem;
		error = DeeObject_CmpLoAsBool(endindex, startindex);
		if unlikely(error < 0)
			goto err_startindex_endindex;
		if (!error)
			break;
		if unlikely(DeeObject_Dec(&endindex))
			goto err_startindex_endindex;
		if unlikely(DeeObject_AsSize(endindex, &index_value))
			goto err_startindex_endindex;
		elem = (*self->dssg_tp_getitem)(self->dssg_seq, endindex);
		if unlikely(!elem) {
			if (DeeError_Catch(&DeeError_UnboundItem)) {
				temp = (*proc)(arg, index_value, NULL);
			} else {
				if (DeeError_Catch(&DeeError_IndexError))
					break;
				goto err_startindex_endindex;
			}
		} else {
			temp = (*proc)(arg, index_value, elem);
			Dee_Decref(elem);
		}
		if unlikely(temp < 0)
			goto err_temp_startindex_endindex;
		result += temp;
		if (DeeThread_CheckInterrupt())
			goto err_startindex_endindex;
	}
	Dee_Decref(endindex);
	Dee_Decref(startindex);
	return result;
err_temp_startindex_endindex:
	Dee_Decref(endindex);
/*err_temp_startindex:*/
	Dee_Decref(startindex);
	return temp;
err_startindex_endindex:
	Dee_Decref(endindex);
err_startindex:
	Dee_Decref(startindex);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_sg_bounditem(DefaultSequence_WithSizeObAndGetItem *self, DeeObject *index) {
	int result;
	DREF DeeObject *used_index = ds_sg_mapindex(self, index);
	if unlikely(!used_index)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_operator_bounditem, self->dssg_seq, used_index);
	Dee_Decref(used_index);
	return result;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_sg_hasitem(DefaultSequence_WithSizeObAndGetItem *self, DeeObject *index) {
	int result;
	DREF DeeObject *used_index;
	used_index = ds_sg_mapindex(self, index);
	if unlikely(!used_index)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_operator_hasitem, self->dssg_seq, used_index);
	Dee_Decref(used_index);
	return result;
err:
	return -1;
}

#if 1
#define ds_sg_methods ds_sgi_methods
#else
PRIVATE struct type_method tpconst ds_sg_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};
#endif

PRIVATE struct type_method_hint tpconst ds_sg_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_enumerate, &ds_sg_mh_seq_enumerate, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index, &ds_sg_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &ds_sg_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &ds_sg_mh_seq_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_seq ds_sg_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_sg_iter,
	/* .tp_sizeob             = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_sg_sizeob,
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ds_sg_getitem,
	/* .tp_delitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&ds_sg_delitem,
	/* .tp_setitem            = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&ds_sg_setitem,
	/* .tp_getrange           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&ds_sg_getrange,
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ds_sg_foreach,
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem          = */ (int (DCALL *)(DeeObject *, DeeObject *))&ds_sg_bounditem,
	/* .tp_hasitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&ds_sg_hasitem,
	/* .tp_size               = */ DEFIMPL(&default__size__with__sizeob),
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ DEFIMPL(&default__getitem_index__with__getitem),
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__delitem_index__with__delitem),
	/* .tp_setitem_index      = */ DEFIMPL(&default__setitem_index__with__setitem),
	/* .tp_bounditem_index    = */ DEFIMPL(&default__bounditem_index__with__bounditem),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__hasitem_index__with__hasitem),
	/* .tp_getrange_index     = */ DEFIMPL(&default__getrange_index__with__getrange),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__getrange_index_n__with__getrange),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__getitem),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_member tpconst ds_sg_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(DefaultSequence_WithSizeObAndGetItem, dssg_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD("__start__", STRUCT_OBJECT, offsetof(DefaultSequence_WithSizeObAndGetItem, dssg_start)),
	TYPE_MEMBER_FIELD("__end__", STRUCT_OBJECT, offsetof(DefaultSequence_WithSizeObAndGetItem, dssg_end)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst ds_sg_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithSizeObAndGetItem_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DefaultSequence_WithSizeObAndGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqWithSizeObAndGetItem",
	/* .tp_doc      = */ DOC("(objWithGetItem,start,end)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultSequence_WithSizeObAndGetItem,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &ds_sg_copy,
			/* tp_deep_ctor:   */ &ds_sg_deepcopy,
			/* tp_any_ctor:    */ &ds_sg_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &ds_sg_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_sg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_sizeob),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ds_sg_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__5819FE7E0C5EF426),
	/* .tp_seq           = */ &ds_sg_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ ds_sg_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_sg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ds_sg_class_members,
	/* .tp_method_hints  = */ ds_sg_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};























/************************************************************************/
/* DefaultSequence_WithIter_Type                                        */
/* DefaultSequence_WithIterAndLimit_Type                                */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultSequence_WithIter, dsi_seq) == offsetof(ProxyObject, po_obj));
#define ds_i_fini  generic_proxy__fini
#define ds_i_visit generic_proxy__visit

STATIC_ASSERT(offsetof(DefaultSequence_WithIterAndLimit, dsial_seq) == offsetof(ProxyObject, po_obj));
#define ds_ial_fini  generic_proxy__fini
#define ds_ial_visit generic_proxy__visit

STATIC_ASSERT(offsetof(DefaultSequence_WithIterAndLimit, dsial_seq) == offsetof(DefaultSequence_WithSizeAndGetItemIndex, dssgi_seq));
STATIC_ASSERT(offsetof(DefaultSequence_WithIterAndLimit, dsial_tp_iter) == offsetof(DefaultSequence_WithSizeAndGetItemIndex, dssgi_tp_getitem_index));
STATIC_ASSERT(offsetof(DefaultSequence_WithIterAndLimit, dsial_start) == offsetof(DefaultSequence_WithSizeAndGetItemIndex, dssgi_start) ||
              offsetof(DefaultSequence_WithIterAndLimit, dsial_start) == offsetof(DefaultSequence_WithSizeAndGetItemIndex, dssgi_end));
STATIC_ASSERT(offsetof(DefaultSequence_WithIterAndLimit, dsial_limit) == offsetof(DefaultSequence_WithSizeAndGetItemIndex, dssgi_start) ||
              offsetof(DefaultSequence_WithIterAndLimit, dsial_limit) == offsetof(DefaultSequence_WithSizeAndGetItemIndex, dssgi_end));
#define ds_ial_copy      ds_sgi_copy
#define ds_ial_deepcopy  ds_sgi_deepcopy
#define ds_ial_serialize ds_sgi_serialize

STATIC_ASSERT(offsetof(DefaultSequence_WithIter, dsi_seq) == offsetof(ProxyObjectWithPointer, po_obj));
STATIC_ASSERT(offsetof(DefaultSequence_WithIter, dsi_tp_iter) == offsetof(ProxyObjectWithPointer, po_ptr));
#define ds_i_serialize generic_proxy_with_funcpointer__serialize

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_i_copy(DefaultSequence_WithIter *__restrict self,
          DefaultSequence_WithIter *__restrict other) {
	Dee_Incref(other->dsi_seq);
	self->dsi_seq     = other->dsi_seq;
	self->dsi_tp_iter = other->dsi_tp_iter;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
ds_i_deepcopy(DefaultSequence_WithIter *__restrict self,
              DefaultSequence_WithIter *__restrict other) {
	self->dsi_seq = DeeObject_DeepCopy(other->dsi_seq);
	if unlikely(!self->dsi_seq)
		goto err;
	self->dsi_tp_iter = other->dsi_tp_iter;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ds_i_init(DefaultSequence_WithIter *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp;
	DeeArg_Unpack1(err, argc, argv, "_SeqWithIter", &self->dsi_seq);
	itertyp = Dee_TYPE(self->dsi_seq);
	self->dsi_tp_iter = DeeType_RequireSupportedNativeOperator(itertyp, iter);
	if unlikely(!self->dsi_tp_iter)
		goto err_no_getitem;
	Dee_Incref(self->dsi_seq);
	return 0;
err_no_getitem:
	err_unimplemented_operator(itertyp, OPERATOR_ITER);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ds_ial_init(DefaultSequence_WithIterAndLimit *__restrict self,
            size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_SeqWithIterAndLimit",
	                  &self->dsial_seq, &self->dsial_start, &self->dsial_limit))
		goto err;
	itertyp = Dee_TYPE(self->dsial_seq);
	self->dsial_tp_iter = DeeType_RequireSupportedNativeOperator(itertyp, iter);
	if unlikely(!self->dsial_tp_iter)
		goto err_no_getitem;
	Dee_Incref(self->dsial_seq);
	return 0;
err_no_getitem:
	err_unimplemented_operator(itertyp, OPERATOR_ITER);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_i_iter(DefaultSequence_WithIter *__restrict self) {
	return (*self->dsi_tp_iter)(self->dsi_seq);
}

#define ds_i_size default__seq_operator_size__with__seq_operator_foreach

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_i_getrange_index(DefaultSequence_WithIter *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DefaultSequence_WithIterAndLimit *result;
	struct Dee_seq_range range;
	if (start >= 0 && end >= 0) {
		range.sr_start = (size_t)start;
		range.sr_end   = (size_t)end;
	} else {
		size_t size = ds_i_size(Dee_AsObject(self));
		if unlikely(size == (size_t)-1)
			goto err;
		DeeSeqRange_Clamp(&range, start, end, size);
	}
	if (range.sr_start >= range.sr_end)
		goto empty_seq;
	result = DeeObject_MALLOC(DefaultSequence_WithIterAndLimit);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dsi_seq);
	result->dsial_seq     = self->dsi_seq; /* Fast-forward the underlying sequence */
	result->dsial_start   = range.sr_start;
	result->dsial_limit   = range.sr_end - range.sr_start;
	result->dsial_tp_iter = self->dsi_tp_iter; /* Fast-forward the underlying sequence */
	DeeObject_Init(result, &DefaultSequence_WithIterAndLimit_Type);
	return Dee_AsObject(result);
empty_seq:
	return DeeSeq_NewEmpty();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_i_getrange_index_n(DefaultSequence_WithIter *self, Dee_ssize_t start) {
	DREF DefaultSequence_WithIterAndLimit *result;
	size_t used_start;
	if (start >= 0) {
		used_start = (size_t)start;
	} else {
		size_t size = ds_i_size(Dee_AsObject(self));
		if unlikely(size == (size_t)-1)
			goto err;
		used_start = DeeSeqRange_Clamp_n(start, size);
	}
	result = DeeObject_MALLOC(DefaultSequence_WithIterAndLimit);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dsi_seq);
	result->dsial_seq     = self->dsi_seq; /* Fast-forward the underlying sequence */
	result->dsial_start   = used_start;
	result->dsial_limit   = (size_t)-1;
	result->dsial_tp_iter = self->dsi_tp_iter; /* Fast-forward the underlying sequence */
	DeeObject_Init(result, &DefaultSequence_WithIterAndLimit_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_ial_iter(DefaultSequence_WithIterAndLimit *__restrict self) {
	DeeNO_iter_next_t iter_next;
	DREF DefaultIterator_WithNextAndLimit *result;
	DREF DeeObject *iter;
	size_t iter_status;
#if defined(__OPTIMIZE__) && !defined(__OPTIMIZE_SIZE__)
	if (self->dsial_start == 0 && self->dsial_limit == (size_t)-1)
		return (*self->dsial_tp_iter)(self->dsial_seq); /* So the compiler can fold the call-frame. */
#endif /* __OPTIMIZE__ && !__OPTIMIZE_SIZE__ */
	iter = (*self->dsial_tp_iter)(self->dsial_seq);
	if unlikely(!iter)
		goto err;
	iter_status = DeeObject_IterAdvance(iter, self->dsial_start);
	if (iter_status != self->dsial_start) {
		if unlikely(iter_status == (size_t)-1)
			goto err_iter;
		return iter; /* Exhausted iterator... */
	}
	iter_next = DeeType_RequireSupportedNativeOperator(Dee_TYPE(iter), iter_next);
	if unlikely(!iter_next)
		goto err_iter_no_iter_next;
	result = DeeObject_MALLOC(DefaultIterator_WithNextAndLimit);
	if unlikely(!result)
		goto err;
	result->dinl_iter    = iter; /* Inherit reference */
	result->dinl_tp_next = iter_next;
	result->dinl_limit   = self->dsial_limit;
	DeeObject_Init(result, &DefaultIterator_WithNextAndLimit_Type);
	return Dee_AsObject(result);
err_iter_no_iter_next:
	err_unimplemented_operator(Dee_TYPE(iter), OPERATOR_ITERNEXT);
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
ds_ial_size(DefaultSequence_WithIterAndLimit *__restrict self) {
	DREF DeeObject *iter;
	size_t result = 0;
	if likely(self->dsial_limit > 0) {
		size_t iter_status;
		iter = (*self->dsial_tp_iter)(self->dsial_seq);
		if unlikely(!iter)
			goto err;
		iter_status = DeeObject_IterAdvance(iter, self->dsial_start);
		if (iter_status != self->dsial_start) {
			if unlikely(iter_status == (size_t)-1)
				goto err_iter;
			/* Exhausted iterator... */
		} else {
			result = DeeObject_IterAdvance(iter, self->dsial_limit);
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
ds_ial_getitem_index(DefaultSequence_WithIterAndLimit *__restrict self, size_t index) {
	size_t iter_status, abs_index;
	DREF DeeObject *result, *iter;
	if unlikely(index >= self->dsial_limit)
		goto err_obb;
	if unlikely(OVERFLOW_UADD(index, self->dsial_start, &abs_index))
		goto err_overflow;
	iter = (*self->dsial_tp_iter)(self->dsial_seq);
	if unlikely(!iter)
		goto err;
	iter_status = DeeObject_IterAdvance(iter, abs_index);
	if (iter_status != abs_index) {
		if unlikely(iter_status == (size_t)-1)
			goto err_iter;
		if (OVERFLOW_USUB(iter_status, self->dsial_start, &iter_status))
			iter_status = 0;
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), abs_index, iter_status);
		goto err_iter;
	}
	result = DeeObject_IterNext(iter);
	Dee_Decref(iter);
	if unlikely(result == ITER_DONE) {
		ASSERT(index == (abs_index - self->dsial_start));
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, index);
		goto err;
	}
	return result;
err_iter:
	Dee_Decref(iter);
err:
	return NULL;
err_overflow:
	DeeRT_ErrIntegerOverflowUAdd(index, self->dsial_start);
	goto err;
err_obb:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->dsial_limit);
	goto err;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_ial_getrange_index(DefaultSequence_WithIterAndLimit *__restrict self,
                      Dee_ssize_t start, Dee_ssize_t end) {
	DREF DefaultSequence_WithIterAndLimit *result;
	struct Dee_seq_range range;
	if (start >= 0 && end >= 0) {
		range.sr_start = (size_t)start;
		range.sr_end   = (size_t)end;
	} else {
		size_t size = ds_ial_size(self);
		if unlikely(size == (size_t)-1)
			goto err;
		DeeSeqRange_Clamp(&range, start, end, size);
	}
	if (range.sr_start >= range.sr_end)
		return DeeSeq_NewEmpty();
	result = DeeObject_MALLOC(DefaultSequence_WithIterAndLimit);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dsial_seq);
	result->dsial_seq     = self->dsial_seq;
	result->dsial_tp_iter = self->dsial_tp_iter;
	result->dsial_start   = self->dsial_start + range.sr_start;
	result->dsial_limit   = range.sr_end - range.sr_start;
	DeeObject_Init(result, &DefaultSequence_WithIterAndLimit_Type);
	return Dee_AsObject(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultSequence_WithIterAndLimit *DCALL
ds_ial_getrange_index_n(DefaultSequence_WithIterAndLimit *__restrict self,
                        Dee_ssize_t start) {
	DREF DefaultSequence_WithIterAndLimit *result;
	size_t used_start;
	if (start >= 0) {
		used_start = (size_t)start;
	} else {
		size_t size = ds_ial_size(self);
		if unlikely(size == (size_t)-1)
			goto err;
		used_start = DeeSeqRange_Clamp_n(start, size);
	}
	if (used_start == 0)
		return_reference_(self);
	result = DeeObject_MALLOC(DefaultSequence_WithIterAndLimit);
	if unlikely(!result)
		goto err;
	Dee_Incref(self->dsial_seq);
	result->dsial_seq     = self->dsial_seq;
	result->dsial_tp_iter = self->dsial_tp_iter;
	result->dsial_start   = self->dsial_start + start;
	result->dsial_limit   = self->dsial_limit - start;
	if (self->dsial_limit == (size_t)-1)
		result->dsial_limit = (size_t)-1;
	DeeObject_Init(result, &DefaultSequence_WithIterAndLimit_Type);
	return result;
err:
	return NULL;
}

PRIVATE struct type_seq ds_i_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_i_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__getrange__with__getrange_index__and__getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_i_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&ds_i_getrange_index,
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&ds_i_getrange_index_n,
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
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

PRIVATE struct type_seq ds_ial_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_ial_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem            = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange           = */ DEFIMPL(&default__getrange__with__getrange_index__and__getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem          = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_ial_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict, size_t))&ds_ial_getitem_index,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index      = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index    = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index     = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&ds_ial_getrange_index,
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&ds_ial_getrange_index_n,
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__getitem_string_hash),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
};

PRIVATE struct type_member tpconst ds_ial_members[] = {
	TYPE_MEMBER_FIELD("__start__", STRUCT_OBJECT, offsetof(DefaultSequence_WithIterAndLimit, dsial_start)),
	TYPE_MEMBER_FIELD("__limit__", STRUCT_OBJECT, offsetof(DefaultSequence_WithIterAndLimit, dsial_limit)),
#define ds_i_members (ds_ial_members + 2)
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(DefaultSequence_WithIterAndLimit, dsial_seq), "->?DSequence"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst ds_ial_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DefaultIterator_WithNextAndLimit_Type),
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DefaultSequence_WithIter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqWithIter",
	/* .tp_doc      = */ DOC("(objWithIter)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultSequence_WithIter,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &ds_i_copy,
			/* tp_deep_ctor:   */ &ds_i_deepcopy,
			/* tp_any_ctor:    */ &ds_i_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &ds_i_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_i_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_iter),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ds_i_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &ds_i_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_i_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject DefaultSequence_WithIterAndLimit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqWithIterAndLimit",
	/* .tp_doc      = */ DOC("(objWithIter,start:?Dint,limit:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultSequence_WithIterAndLimit,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &ds_ial_copy,
			/* tp_deep_ctor:   */ &ds_ial_deepcopy,
			/* tp_any_ctor:    */ &ds_ial_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &ds_ial_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_ial_fini,
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ds_ial_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__26B2EC529683DE3C),
	/* .tp_seq           = */ &ds_ial_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_ial_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ds_ial_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SEQUENCES_C */
