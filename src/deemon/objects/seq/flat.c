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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_FLAT_C
#define GUARD_DEEMON_OBJECTS_SEQ_FLAT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/gc.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/overflow.h>

/**/
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "default-api.h"

/**/
#include "flat.h"

#undef SSIZE_MIN
#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MIN __SSIZE_MIN__
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* From "../tuple.c" */
tuple_mh_foreach_reverse(DeeTupleObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* From "../tuple.c" */
tuple_mh_enumerate_index_reverse(DeeTupleObject *__restrict self, Dee_enumerate_index_t proc,
                                 void *arg, size_t start, size_t end);

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_InvokeForeachReverse(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t result;
	DREF DeeTupleObject *astuple;
	Dee_mh_seq_foreach_reverse_t op;
	op = DeeType_TryRequireSeqForeachReverse(Dee_TYPE(self));
	if (op != NULL)
		return (*op)(self, cb, arg);
	astuple = (DREF DeeTupleObject *)DeeTuple_FromSequence(self);
	if unlikely(!astuple)
		goto err;
	result = tuple_mh_foreach_reverse(astuple, cb, arg);
	Dee_Decref_likely(astuple);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_InvokeEnumerateIndexReverse(DeeObject *__restrict self,
                                   Dee_enumerate_index_t cb, void *arg,
                                   size_t start, size_t end) {
	Dee_ssize_t result;
	DREF DeeTupleObject *astuple;
	Dee_mh_seq_enumerate_index_reverse_t op;
	op = DeeType_TryRequireSeqEnumerateIndexReverse(Dee_TYPE(self));
	if (op != NULL)
		return (*op)(self, cb, arg, start, end);
	astuple = (DREF DeeTupleObject *)DeeTuple_FromSequence(self);
	if unlikely(!astuple)
		goto err;
	result = tuple_mh_enumerate_index_reverse(astuple, cb, arg, start, end);
	Dee_Decref_likely(astuple);
	return result;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sfi_inititer_withseq(SeqFlatIterator *__restrict self, DeeObject *seq) {
	DREF DeeObject *firstseq;
	self->sfi_baseiter = DeeObject_Iter(seq);
	if unlikely(!self->sfi_baseiter)
		goto err;
	firstseq = DeeObject_IterNext(self->sfi_baseiter);
	if unlikely(!firstseq)
		goto err_baseiter;
	if unlikely(firstseq == ITER_DONE) {
		/* Special case: "{}.flatten" */
		Dee_Incref(Dee_EmptyIterator);
		self->sfi_curriter = Dee_EmptyIterator;
	} else {
		self->sfi_curriter = DeeObject_Iter(seq);
		if unlikely(!self->sfi_curriter)
			goto err_baseiter_firstseq;
	}
	Dee_Decref_unlikely(firstseq);
	Dee_atomic_lock_init(&self->sfi_currlock);
	return 0;
err_baseiter_firstseq:
	Dee_Decref(firstseq);
err_baseiter:
	Dee_Decref(self->sfi_baseiter);
err:
	return -1;
}




/************************************************************************/
/* SeqFlat                                                              */
/************************************************************************/
STATIC_ASSERT(offsetof(SeqFlat, sf_seq) == offsetof(ProxyObject, po_obj));
#define sf_copy  generic_proxy_copy_alias
#define sf_deep  generic_proxy_deepcopy
#define sf_init  generic_proxy_init
#define sf_fini  generic_proxy_fini
#define sf_visit generic_proxy_visit

#define sf_foreachseq(self, cb, arg)         DeeSeq_OperatorForeach((self)->sf_seq, cb, arg)
#define sf_foreachseq_reverse(self, cb, arg) DeeSeq_InvokeForeachReverse((self)->sf_seq, cb, arg)

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_ctor(SeqFlat *__restrict self) {
	Dee_Incref(Dee_EmptySeq);
	self->sf_seq = Dee_EmptySeq;
	return 0;
}

#define SF_BOOL_FOREACH_YES SSIZE_MIN
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_bool_foreach_cb(void *UNUSED(arg), DeeObject *item) {
	Dee_ssize_t result = DeeSeq_OperatorBool(item);
	if (result > 0)
		result = SF_BOOL_FOREACH_YES;
	ASSERT(result == 0 || result == -1 || result == SF_BOOL_FOREACH_YES);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_bool(SeqFlat *__restrict self) {
	Dee_ssize_t result = sf_foreachseq(self, &sf_bool_foreach_cb, NULL);
	ASSERT(result == 0 || result == -1 || result == SF_BOOL_FOREACH_YES);
	if (result == SF_BOOL_FOREACH_YES)
		result = 1;
	return (int)result;
}

#define sf_trygetfirstseq(self) DeeSeq_InvokeTryGetFirst((self)->sf_seq)
#define sf_trygetlastseq(self)  DeeSeq_InvokeTryGetLast((self)->sf_seq)
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sf_getfirstseq(SeqFlat *__restrict self) {
	return DeeSeq_InvokeGetFirst(self->sf_seq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_delfirstseq(SeqFlat *__restrict self) {
	return DeeSeq_InvokeDelFirst(self->sf_seq);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sf_setfirstseq(SeqFlat *self, DeeObject *value) {
	return DeeSeq_InvokeSetFirst(self->sf_seq, value);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sf_getlastseq(SeqFlat *__restrict self) {
	return DeeSeq_InvokeGetLast(self->sf_seq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_dellastseq(SeqFlat *__restrict self) {
	return DeeSeq_InvokeDelLast(self->sf_seq);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sf_setlastseq(SeqFlat *self, DeeObject *value) {
	return DeeSeq_InvokeSetLast(self->sf_seq, value);
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqFlatIterator *DCALL
sf_iter(SeqFlat *__restrict self) {
	DREF SeqFlatIterator *result;
	result = DeeGCObject_MALLOC(SeqFlatIterator);
	if unlikely(!result)
		goto err;
	if unlikely(sfi_inititer_withseq(result, self->sf_seq))
		goto err_r;
	DeeObject_Init(result, &SeqFlatIterator_Type);
	return (DREF SeqFlatIterator *)DeeGC_Track((DREF DeeObject *)result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

#define SF_CONTAINS_FOREACH_YES SSIZE_MIN
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_contains_foreach_cb(void *arg, DeeObject *item) {
	Dee_ssize_t result;
	DREF DeeObject *contains_ob;
	contains_ob = DeeSeq_OperatorContains(item, (DeeObject *)arg);
	if unlikely(!contains_ob)
		goto err;
	result = DeeObject_BoolInherited(contains_ob);
	if (result > 0)
		result = SF_CONTAINS_FOREACH_YES;
	ASSERT(result == 0 || result == -1 || result == SF_CONTAINS_FOREACH_YES);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sf_contains(SeqFlat *self, DeeObject *item) {
	Dee_ssize_t result = sf_foreachseq(self, &sf_contains_foreach_cb, item);
	ASSERT(result == 0 || result == -1 || result == SF_CONTAINS_FOREACH_YES);
	if (result == SF_CONTAINS_FOREACH_YES)
		return_true;
	if (result == 0)
		return_false;
	return NULL;
}

struct sf_foreach_data {
	Dee_foreach_t sffd_proc; /* [1..1] Underlying proc */
	void         *sffd_arg;  /* [?..?] Cookie for `sffd_proc' */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_foreach_cb(void *arg, DeeObject *subseq) {
	struct sf_foreach_data *data;
	data = (struct sf_foreach_data *)arg;
	return DeeSeq_OperatorForeach(subseq, data->sffd_proc, data->sffd_arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_foreach(SeqFlat *__restrict self, Dee_foreach_t proc, void *arg) {
	struct sf_foreach_data data;
	data.sffd_proc = proc;
	data.sffd_arg  = arg;
	return sf_foreachseq(self, &sf_foreach_cb, &data);
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_mh_foreach_reverse_cb(void *arg, DeeObject *subseq) {
	struct sf_foreach_data *data;
	data = (struct sf_foreach_data *)arg;
	return DeeSeq_InvokeForeachReverse(subseq, data->sffd_proc, data->sffd_arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_foreach_reverse(SeqFlat *__restrict self, Dee_foreach_t proc, void *arg) {
	struct sf_foreach_data data;
	data.sffd_proc = proc;
	data.sffd_arg  = arg;
	return sf_foreachseq_reverse(self, &sf_mh_foreach_reverse_cb, &data);
}

struct sf_foreach_pair_data {
	Dee_foreach_pair_t sffpd_proc; /* [1..1] Underlying proc */
	void              *sffpd_arg;  /* [?..?] Cookie for `sffpd_proc' */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_foreach_pair_cb(void *arg, DeeObject *subseq) {
	struct sf_foreach_pair_data *data;
	data = (struct sf_foreach_pair_data *)arg;
	return DeeSeq_OperatorForeachPair(subseq, data->sffpd_proc, data->sffpd_arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_foreach_pair(SeqFlat *__restrict self, Dee_foreach_pair_t proc, void *arg) {
	struct sf_foreach_pair_data data;
	data.sffpd_proc = proc;
	data.sffpd_arg  = arg;
	return sf_foreachseq(self, &sf_foreach_pair_cb, &data);
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_size_foreach_cb(void *arg, DeeObject *subseq) {
	size_t subseq_size = DeeSeq_OperatorSize(subseq);
	if unlikely(subseq_size == (size_t)-1)
		goto err;
	if (OVERFLOW_UADD(*(size_t *)arg, subseq_size, (size_t *)arg))
		goto err_overflow;
	return 0;
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
sf_size(SeqFlat *__restrict self) {
	size_t result = 0;
	if unlikely(sf_foreachseq(self, &sf_size_foreach_cb, &result))
		goto err;
	if unlikely(result == (size_t)-1)
		goto err_overflow;
	return result;
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return (size_t)-1;
}


struct sf_enumerate_index_data {
	Dee_enumerate_index_t sfeid_proc;   /* [1..1] Underlying proc */
	void                 *sfeid_arg;    /* [?..?] Cookie for `sfeid_proc' */
	Dee_ssize_t           sfeid_result; /* Nested enumeration result */
	size_t                sfeid_index;  /* Next index */
	size_t                sfeid_end;    /* End index (stop enumeration when `sfeid_index') */
	size_t                sfeid_skip;   /* Number of indices that still need to be skipped */
};

#define SF_ENUMERATE_INDEX_FOREACH_DONE SSIZE_MIN
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_enumerate_index_foreach_inner_cb(void *arg, DeeObject *elem) {
	Dee_ssize_t status;
	struct sf_enumerate_index_data *data;
	data = (struct sf_enumerate_index_data *)arg;
	ASSERT(data->sfeid_skip == 0);
	status = (*data->sfeid_proc)(data->sfeid_arg, data->sfeid_index, elem);
	if unlikely(status < 0) {
		data->sfeid_result = status;
		return SF_ENUMERATE_INDEX_FOREACH_DONE;
	}
	data->sfeid_result += status;
	++data->sfeid_index;
	if (data->sfeid_index >= data->sfeid_end)
		return SF_ENUMERATE_INDEX_FOREACH_DONE;
	return 0;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_enumerate_index_foreach_inner_with_skip_cb(void *arg, DeeObject *elem) {
	struct sf_enumerate_index_data *data;
	data = (struct sf_enumerate_index_data *)arg;
	if (data->sfeid_skip) {
		--data->sfeid_skip;
		return 0;
	}
	return sf_enumerate_index_foreach_inner_cb(data, elem);
}

PRIVATE WUNUSED Dee_ssize_t DCALL
sf_enumerate_index_enumerate_inner_cb(void *arg, size_t UNUSED(index), DeeObject *elem) {
	if unlikely(!elem)
		return 0;
	return sf_enumerate_index_foreach_inner_cb(arg, elem);
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_enumerate_index_foreach_cb(void *arg, DeeObject *subseq) {
	size_t subseq_fastsize, subseq_skip;
	struct sf_enumerate_index_data *data;
	data = (struct sf_enumerate_index_data *)arg;
	if (!data->sfeid_skip)
		return DeeSeq_OperatorForeach(subseq, &sf_enumerate_index_foreach_inner_cb, data);
	subseq_fastsize = DeeSeq_OperatorSizeFast(subseq);
	if (subseq_fastsize == (size_t)-1)
		return DeeSeq_OperatorForeach(subseq, &sf_enumerate_index_foreach_inner_with_skip_cb, data);
	if (data->sfeid_skip >= subseq_fastsize) {
		data->sfeid_skip -= subseq_fastsize;
		return 0;
	}
	subseq_skip = data->sfeid_skip ;
	data->sfeid_skip = 0;
	return DeeSeq_OperatorEnumerateIndex(subseq, &sf_enumerate_index_enumerate_inner_cb,
	                                     data, subseq_skip, (size_t)-1);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_enumerate_index(SeqFlat *__restrict self, Dee_enumerate_index_t proc,
                   void *arg, size_t start, size_t end) {
	Dee_ssize_t status;
	struct sf_enumerate_index_data data;
	data.sfeid_proc   = proc;
	data.sfeid_arg    = arg;
	data.sfeid_result = 0;
	data.sfeid_index  = start;
	data.sfeid_skip   = start;
	data.sfeid_end    = end;
	status = sf_foreachseq(self, &sf_enumerate_index_foreach_cb, &data);
	if (status == 0 || status == SF_ENUMERATE_INDEX_FOREACH_DONE)
		return data.sfeid_result;
	ASSERT(status < 0);
	return status;
}

struct sf_enumerate_index_reverse_data {
	Dee_enumerate_index_t sfeird_proc;   /* [1..1] Underlying proc */
	void                 *sfeird_arg;    /* [?..?] Cookie for `sfeird_proc' */
	Dee_ssize_t           sfeird_result; /* Nested enumeration result */
	size_t                sfeird_index;  /* Next index */
	size_t                sfeird_start;  /* Start index (stop enumeration when `sfeird_index') */
	size_t                sfeird_skip;   /* Number of indices that still need to be skipped */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_enumerate_index_foreach_reverse_inner_cb(void *arg, DeeObject *elem) {
	Dee_ssize_t status;
	struct sf_enumerate_index_reverse_data *data;
	data = (struct sf_enumerate_index_reverse_data *)arg;
	ASSERT(data->sfeird_skip == 0);
	ASSERT(data->sfeird_index > 0);
	--data->sfeird_index;
	status = (*data->sfeird_proc)(data->sfeird_arg, data->sfeird_index, elem);
	if unlikely(status < 0) {
		data->sfeird_result = status;
		return SF_ENUMERATE_INDEX_FOREACH_DONE;
	}
	data->sfeird_result += status;
	if (data->sfeird_index <= data->sfeird_start)
		return SF_ENUMERATE_INDEX_FOREACH_DONE;
	return 0;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_enumerate_index_foreach_reverse_inner_with_skip_cb(void *arg, DeeObject *elem) {
	struct sf_enumerate_index_reverse_data *data;
	data = (struct sf_enumerate_index_reverse_data *)arg;
	if (data->sfeird_skip) {
		--data->sfeird_skip;
		return 0;
	}
	return sf_enumerate_index_foreach_reverse_inner_cb(data, elem);
}

PRIVATE WUNUSED Dee_ssize_t DCALL
sf_enumerate_index_enumerate_reverse_inner_cb(void *arg, size_t UNUSED(index), DeeObject *elem) {
	if unlikely(!elem)
		return 0;
	return sf_enumerate_index_foreach_reverse_inner_cb(arg, elem);
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_enumerate_index_foreach_reverse_cb(void *arg, DeeObject *subseq) {
	size_t subseq_size, subseq_end;
	Dee_mh_seq_enumerate_index_reverse_t ei_reverse_op;
	struct sf_enumerate_index_reverse_data *data;
	data = (struct sf_enumerate_index_reverse_data *)arg;
	if (!data->sfeird_skip)
		return DeeSeq_InvokeForeachReverse(subseq, &sf_enumerate_index_foreach_reverse_inner_cb, data);
	subseq_size = DeeSeq_OperatorSize(subseq);
	if (subseq_size == (size_t)-1)
		goto err;
	if (data->sfeird_skip >= subseq_size) {
		data->sfeird_skip -= subseq_size;
		return 0;
	}
	ei_reverse_op = DeeType_TryRequireSeqEnumerateIndexReverse(Dee_TYPE(subseq));
	if (!ei_reverse_op) {
		Dee_mh_seq_foreach_reverse_t fe_reverse_op;
		fe_reverse_op = DeeType_TryRequireSeqForeachReverse(Dee_TYPE(subseq));
		if (fe_reverse_op != NULL)
			return (*fe_reverse_op)(subseq, &sf_enumerate_index_foreach_reverse_inner_with_skip_cb, data);
	}
	subseq_end = subseq_size - data->sfeird_skip;
	data->sfeird_skip = 0;
	return DeeSeq_InvokeEnumerateIndexReverse(subseq, &sf_enumerate_index_enumerate_reverse_inner_cb,
	                                          data, 0, subseq_end);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_enumerate_index_reverse(SeqFlat *__restrict self, Dee_enumerate_index_t proc,
                              void *arg, size_t start, size_t end) {
	size_t fullsize;
	Dee_ssize_t status;
	struct sf_enumerate_index_reverse_data data;
	fullsize = sf_size(self);
	if unlikely(fullsize == (size_t)-1)
		goto err;
	if (end > fullsize)
		end = fullsize;
	data.sfeird_proc   = proc;
	data.sfeird_arg    = arg;
	data.sfeird_result = 0;
	data.sfeird_index  = fullsize;
	data.sfeird_start  = start;
	data.sfeird_skip   = fullsize - end;
	status = sf_foreachseq_reverse(self, &sf_enumerate_index_foreach_reverse_cb, &data);
	if (status == 0 || status == SF_ENUMERATE_INDEX_FOREACH_DONE)
		return data.sfeird_result;
	ASSERT(status < 0);
	return status;
err:
	return -1;
}




#define SF_TRYGETITEM_INDEX_FOUND SSIZE_MIN
PRIVATE WUNUSED Dee_ssize_t DCALL
sf_trygetitem_index_cb(void *arg, size_t UNUSED(index), DeeObject *item) {
	ASSERT(item);
	Dee_Incref(item);
	*(DREF DeeObject **)arg = item;
	return SF_TRYGETITEM_INDEX_FOUND;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sf_trygetitem_index(SeqFlat *__restrict self, size_t index) {
	size_t end_index;
	Dee_ssize_t status;
	DREF DeeObject *result;
	if (OVERFLOW_UADD(index, 1, &end_index))
		goto oob;
#ifndef NDEBUG
	result = NULL;
#endif /* !NDEBUG */
	status = sf_enumerate_index(self, &sf_trygetitem_index_cb, &result, index, index + 1);
	ASSERT(status == 0 || status == -1 || status == SF_TRYGETITEM_INDEX_FOUND);
	if likely(status == SF_TRYGETITEM_INDEX_FOUND) {
#ifndef NDEBUG
		ASSERT(result != NULL);
#endif /* !NDEBUG */
		return result;
	}
	if unlikely(status < 0)
		goto err;
oob:
	return ITER_DONE;
err:
	return NULL;
}


struct sf_interact_withitem_data {
	WUNUSED_T NONNULL_T((1)) int (DCALL *sfiwid_interact)(DeeObject *__restrict subseq, size_t subseq_index, DeeObject *cookie);
	DeeObject                           *sfiwid_cookie;
	size_t                               sfiwid_index;  /* # of elements that still need to be skipped */
	size_t                               sfiwid_count;  /* Total # of elements from sub-sequences */
};

#define SF_INTERACT_WITHITEM_DONE SSIZE_MIN
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_interact_withitem_cb(void *arg, DeeObject *subseq) {
	size_t subseq_size;
	struct sf_interact_withitem_data *data;
	data = (struct sf_interact_withitem_data *)arg;
	subseq_size = DeeSeq_OperatorSize(subseq);
	if unlikely(subseq_size == (size_t)-1)
		goto err;
	if (data->sfiwid_index >= subseq_size) {
		data->sfiwid_index -= subseq_size;
		data->sfiwid_count += subseq_size;
		return 0;
	}
	if unlikely((*data->sfiwid_interact)(subseq, data->sfiwid_index, data->sfiwid_cookie))
		goto err;
	return SF_INTERACT_WITHITEM_DONE;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
sf_interact_withitem(SeqFlat *__restrict self, size_t index,
                     WUNUSED_T NONNULL_T((1)) int (DCALL *interact)(DeeObject *__restrict subseq,
                                                                    size_t subseq_index,
                                                                    DeeObject *cookie),
                     DeeObject *cookie) {
	Dee_ssize_t status;
	struct sf_interact_withitem_data data;
	data.sfiwid_interact = interact;
	data.sfiwid_cookie   = cookie;
	data.sfiwid_index    = index;
	data.sfiwid_count    = 0;
	status = sf_foreachseq(self, &sf_interact_withitem_cb, &data);
	if likely(status == SF_INTERACT_WITHITEM_DONE)
		return 0;
	if unlikely(status < 0)
		goto err;
	/* Item not found -> index must be out-of-bounds */
	err_index_out_of_bounds((DeeObject *)self, index, data.sfiwid_count);
err:
	return -1;
}


#define sf_setitem_index_cb DeeSeq_OperatorSetItemIndex
PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_delitem_index_cb(DeeObject *__restrict subseq, size_t index,
                    DeeObject *UNUSED(cookie)) {
	return DeeSeq_OperatorDelItemIndex(subseq, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_delitem_index(SeqFlat *__restrict self, size_t index) {
	return sf_interact_withitem(self, index, &sf_delitem_index_cb, NULL);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
sf_setitem_index(SeqFlat *__restrict self, size_t index, DeeObject *value) {
	return sf_interact_withitem(self, index, &sf_setitem_index_cb, value);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_xchitem_index_cb(DeeObject *__restrict subseq, size_t index,
                    DeeObject *cookie) {
	DREF DeeObject *old_value;
	DeeObject *new_value = *(DeeObject **)cookie;
	old_value = DeeSeq_InvokeXchItemIndex(subseq, index, new_value);
	if unlikely(!old_value)
		goto err;
	*(DREF DeeObject **)cookie = old_value;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
sf_mh_xchitem_index(SeqFlat *__restrict self, size_t index, DeeObject *value) {
	DREF DeeObject *result = value;
	if unlikely(sf_interact_withitem(self, index, &sf_xchitem_index_cb,
	                                 (DeeObject *)&result))
		goto err;
	return result;
err:
	return NULL;
}



PRIVATE struct type_seq sf_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sf_iter,
	/* .tp_sizeob             = */ NULL,
	/* .tp_contains           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sf_contains,
	/* .tp_getitem            = */ NULL,
	/* .tp_delitem            = */ NULL,
	/* .tp_setitem            = */ NULL,
	/* .tp_getrange           = */ NULL,
	/* .tp_delrange           = */ NULL,
	/* .tp_setrange           = */ NULL,
	/* .tp_nsi                = */ NULL,
	/* .tp_foreach            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&sf_foreach,
	/* .tp_foreach_pair       = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&sf_foreach_pair,
	/* .tp_enumerate          = */ NULL,
	/* .tp_enumerate_index    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_index_t, void *, size_t, size_t))&sf_enumerate_index,
	/* .tp_iterkeys           = */ NULL,
	/* .tp_bounditem          = */ NULL,
	/* .tp_hasitem            = */ NULL,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&sf_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ &DeeObject_DefaultGetItemIndexWithTryGetItemIndex,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&sf_delitem_index,
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&sf_setitem_index,
	/* .tp_bounditem_index    = */ NULL,
	/* .tp_hasitem_index      = */ NULL,
	/* .tp_getrange_index     = */ NULL,
	/* .tp_delrange_index     = */ NULL,
	/* .tp_setrange_index     = */ NULL,
	/* .tp_getrange_index_n   = */ NULL,
	/* .tp_delrange_index_n   = */ NULL,
	/* .tp_setrange_index_n   = */ NULL,
	/* .tp_trygetitem         = */ NULL,
	/* .tp_trygetitem_index   = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&sf_trygetitem_index,
};

PRIVATE struct type_getset tpconst sf_getsets[] = {
	//TODO:TYPE_GETSET_F_NODOC(STR_first, &sf_getfirst, &sf_delfirst, &sf_setfirst, METHOD_FNOREFESCAPE),
	//TODO:TYPE_GETSET_F_NODOC(STR_last, &sf_getlast, &sf_dellast, &sf_setlast, METHOD_FNOREFESCAPE),
	TYPE_GETSET_F("__firstseq__", &sf_getfirstseq, &sf_delfirstseq, &sf_setfirstseq, METHOD_FNOREFESCAPE, "->?DSequence"),
	TYPE_GETSET_F("__lastseq__", &sf_getlastseq, &sf_dellastseq, &sf_setlastseq, METHOD_FNOREFESCAPE, "->?DSequence"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_clear_foreach_cb(void *UNUSED(arg), DeeObject *subseq) {
	return DeeSeq_InvokeClear(subseq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_mh_clear(SeqFlat *__restrict self) {
	return (int)sf_foreachseq(self, &sf_mh_clear_foreach_cb, NULL);
}


//TODO:PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
//TODO:sf_mh_find(DeeObject *self, DeeObject *item,
//TODO:           size_t start, size_t end) {
//TODO:}
//TODO:
//TODO:PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
//TODO:sf_mh_find_with_key(DeeObject *self, DeeObject *item,
//TODO:                    size_t start, size_t end, DeeObject *key) {
//TODO:}


PRIVATE struct type_method_hint tpconst sf_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &sf_mh_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &sf_mh_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_trygetfirst, &sf_mh_trygetfirst, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_trygetlast, &sf_mh_trygetlast, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_any, &sf_mh_any, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_any_with_key, &sf_mh_any_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_any_with_range, &sf_mh_any_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_any_with_range_and_key, &sf_mh_any_with_range_and_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_all, &sf_mh_all, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_all_with_key, &sf_mh_all_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_all_with_range, &sf_mh_all_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_all_with_range_and_key, &sf_mh_all_with_range_and_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_parity, &sf_mh_parity, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_parity_with_key, &sf_mh_parity_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_parity_with_range, &sf_mh_parity_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_parity_with_range_and_key, &sf_mh_parity_with_range_and_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_min, &sf_mh_min, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_min_with_key, &sf_mh_min_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_min_with_range, &sf_mh_min_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_min_with_range_and_key, &sf_mh_min_with_range_and_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_max, &sf_mh_max, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_max_with_key, &sf_mh_max_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_max_with_range, &sf_mh_max_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_max_with_range_and_key, &sf_mh_max_with_range_and_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_count, &sf_mh_count, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_count_with_key, &sf_mh_count_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_count_with_range, &sf_mh_count_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_count_with_range_and_key, &sf_mh_count_with_range_and_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_contains, &sf_mh_contains, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_contains_with_key, &sf_mh_contains_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_contains_with_range, &sf_mh_contains_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_contains_with_range_and_key, &sf_mh_contains_with_range_and_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_locate, &sf_mh_locate, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_locate_with_key, &sf_mh_locate_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_locate_with_range, &sf_mh_locate_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_locate_with_range_and_key, &sf_mh_locate_with_range_and_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_rlocate, &sf_mh_rlocate, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_rlocate_with_key, &sf_mh_rlocate_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_rlocate_with_range, &sf_mh_rlocate_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_rlocate_with_range_and_key, &sf_mh_rlocate_with_range_and_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_startswith, &sf_mh_startswith, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_startswith_with_key, &sf_mh_startswith_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_startswith_with_range, &sf_mh_startswith_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_startswith_with_range_and_key, &sf_mh_startswith_with_range_and_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_endswith, &sf_mh_endswith, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_endswith_with_key, &sf_mh_endswith_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_endswith_with_range, &sf_mh_endswith_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_endswith_with_range_and_key, &sf_mh_endswith_with_range_and_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_find, &sf_mh_find, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_find_with_key, &sf_mh_find_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_rfind, &sf_mh_rfind, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_rfind_with_key, &sf_mh_rfind_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_erase, &sf_mh_erase, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_insert, &sf_mh_insert, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_insertall, &sf_mh_insertall, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_pushfront, &sf_mh_pushfront, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_append, &sf_mh_append, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_extend, &sf_mh_extend, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_xchitem_index, &sf_mh_xchitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_clear, &sf_mh_clear, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_pop, &sf_mh_pop, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_remove, &sf_mh_remove, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_rremove, &sf_mh_rremove, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_removeall, &sf_mh_removeall, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_removeif, &sf_mh_removeif, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_resize, &sf_mh_resize, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_fill, &sf_mh_fill, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_reverse, &sf_mh_reverse, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_reversed, &sf_mh_reversed, METHOD_FNOREFESCAPE),
	/* TODO: TYPE_METHOD_HINT_F(seq_sum, &sf_mh_sum, METHOD_FNOREFESCAPE), */
	/* TODO: TYPE_METHOD_HINT_F(seq_sum_with_range, &sf_mh_sum_with_range, METHOD_FNOREFESCAPE), */
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_method tpconst sf_methods[] = {
	//TODO:TYPE_METHOD_HINTREF(seq_any),
	//TODO:TYPE_METHOD_HINTREF(seq_all),
	//TODO:TYPE_METHOD_HINTREF(seq_parity),
	//TODO:TYPE_METHOD_HINTREF(seq_min),
	//TODO:TYPE_METHOD_HINTREF(seq_max),
	//TODO:TYPE_METHOD_HINTREF(seq_count),
	//TODO:TYPE_METHOD_HINTREF(seq_contains),
	//TODO:TYPE_METHOD_HINTREF(seq_locate),
	//TODO:TYPE_METHOD_HINTREF(seq_rlocate),
	//TODO:TYPE_METHOD_HINTREF(seq_startswith),
	//TODO:TYPE_METHOD_HINTREF(seq_endswith),
	//TODO:TYPE_METHOD_HINTREF(seq_find),
	//TODO:TYPE_METHOD_HINTREF(seq_rfind),
	//TODO:TYPE_METHOD_HINTREF(seq_erase),
	//TODO:TYPE_METHOD_HINTREF(seq_insert),
	//TODO:TYPE_METHOD_HINTREF(seq_insertall),
	//TODO:TYPE_METHOD_HINTREF(seq_pushfront),
	//TODO:TYPE_METHOD_HINTREF(seq_append),
	//TODO:TYPE_METHOD_HINTREF(seq_extend),
	TYPE_METHOD_HINTREF(seq_xchitem),
	TYPE_METHOD_HINTREF(seq_clear),
	//TODO:TYPE_METHOD_HINTREF(seq_pop),
	//TODO:TYPE_METHOD_HINTREF(seq_remove),
	//TODO:TYPE_METHOD_HINTREF(seq_rremove),
	//TODO:TYPE_METHOD_HINTREF(seq_removeall),
	//TODO:TYPE_METHOD_HINTREF(seq_removeif),
	//TODO:TYPE_METHOD_HINTREF(seq_resize),
	//TODO:TYPE_METHOD_HINTREF(seq_fill),
	//TODO:TYPE_METHOD_HINTREF(seq_reverse),
	//TODO:TYPE_METHOD_HINTREF(seq_reversed),
	/* TODO: TYPE_METHOD_HINTREF(seq_sum),*/
	TYPE_METHOD_END
};

PRIVATE struct type_member tpconst sf_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(SeqFlat, sf_seq), "->?S?S?O"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst sf_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqFlatIterator_Type),
	TYPE_MEMBER_END
};


INTERN DeeTypeObject SeqFlat_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqFlat",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?S?S?O)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&sf_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&sf_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&sf_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&sf_init,
				TYPE_FIXED_ALLOCATOR(SeqFlat)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sf_bool /* TODO: __seq__.some.operator bool() */
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &sf_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ sf_methods,
	/* .tp_getsets       = */ sf_getsets,
	/* .tp_members       = */ sf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ sf_class_members,
	/* .tp_method_hints  = */ sf_method_hints,
};





/************************************************************************/
/* SeqFlatIterator                                                      */
/************************************************************************/
STATIC_ASSERT(offsetof(SeqFlatIterator, sfi_baseiter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqFlatIterator, sfi_baseiter) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(SeqFlatIterator, sfi_curriter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqFlatIterator, sfi_curriter) == offsetof(ProxyObject2, po_obj2));
#define sfi_fini generic_proxy2_fini

PRIVATE NONNULL((1)) int DCALL
sfi_ctor(SeqFlatIterator *__restrict self) {
	Dee_Incref_n(Dee_EmptyIterator, 2);
	self->sfi_baseiter = Dee_EmptyIterator;
	self->sfi_curriter = Dee_EmptyIterator;
	Dee_atomic_lock_init(&self->sfi_currlock);
	return 0;
}

PRIVATE NONNULL((1, 2)) int DCALL
sfi_copy(SeqFlatIterator *__restrict self,
         SeqFlatIterator *__restrict other) {
	DREF DeeObject *other_curriter;
	SeqFlatIterator_LockAcquire(self);
	other_curriter = other->sfi_curriter;
	Dee_Incref(other_curriter);
	SeqFlatIterator_LockRelease(self);
	self->sfi_curriter = DeeObject_Copy(other_curriter);
	Dee_Decref(other_curriter);
	if unlikely(!self->sfi_curriter)
		goto err;
	Dee_Incref(other->sfi_baseiter);
	self->sfi_baseiter = other->sfi_baseiter;
	Dee_atomic_lock_init(&self->sfi_currlock);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) int DCALL
sfi_deep(SeqFlatIterator *__restrict self,
         SeqFlatIterator *__restrict other) {
	DREF DeeObject *other_curriter;
	SeqFlatIterator_LockAcquire(self);
	other_curriter = other->sfi_curriter;
	Dee_Incref(other_curriter);
	SeqFlatIterator_LockRelease(self);
	self->sfi_curriter = DeeObject_DeepCopy(other_curriter);
	Dee_Decref(other_curriter);
	if unlikely(!self->sfi_curriter)
		goto err;
	self->sfi_baseiter = DeeObject_DeepCopy(other->sfi_baseiter);
	if unlikely(!self->sfi_baseiter)
		goto err_curriter;
	Dee_atomic_lock_init(&self->sfi_currlock);
	return 0;
err_curriter:
	Dee_Decref(self->sfi_curriter);
err:
	return -1;
}

PRIVATE NONNULL((1)) int DCALL
sfi_init(SeqFlatIterator *__restrict self,
         size_t argc, DeeObject *const *argv) {
	SeqFlat *flat;
	if (DeeArg_Unpack(argc, argv, "o:_SeqFlatIterator", &flat))
		goto err;
	if (DeeObject_AssertTypeExact(flat, &SeqFlat_Type))
		goto err;
	return sfi_inititer_withseq(self, flat->sf_seq);
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) void DCALL
sfi_visit(SeqFlatIterator *__restrict self,
          dvisit_t proc, void *arg) {
	Dee_Visit(self->sfi_baseiter);
	SeqFlatIterator_LockAcquire(self);
	Dee_Visit(self->sfi_curriter);
	SeqFlatIterator_LockRelease(self);
}

PRIVATE NONNULL((1)) void DCALL
sfi_clear(SeqFlatIterator *__restrict self) {
	DREF DeeObject *old_curriter;
	Dee_Incref(Dee_EmptyIterator);
	SeqFlatIterator_LockAcquire(self);
	old_curriter = self->sfi_curriter;
	self->sfi_curriter = Dee_EmptyIterator;
	SeqFlatIterator_LockRelease(self);
	Dee_Decref(old_curriter);
}

PRIVATE struct type_gc sfi_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&sfi_clear
};

PRIVATE NONNULL((1)) int DCALL
sfi_bool(SeqFlatIterator *__restrict self) {
	int result;
	for (;;) {
		DREF DeeObject *curriter;
		SeqFlatIterator_LockAcquire(self);
		curriter = self->sfi_curriter;
		Dee_Incref(curriter);
		SeqFlatIterator_LockRelease(self);
		result = DeeObject_BoolInherited(curriter);
		if (result != 0)
			break; /* non-empty, or error */
		result = DeeObject_Bool(self->sfi_baseiter);
		if (result != 0)
			break; /* non-empty, or error */
		if (curriter == atomic_read(&self->sfi_curriter))
			break; /* EOF has actually been reached. */
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sfi_next(SeqFlatIterator *__restrict self) {
	for (;;) {
		DREF DeeObject *result;
		DREF DeeObject *curriter;
		DREF DeeObject *new_curriter;
		SeqFlatIterator_LockAcquire(self);
		curriter = self->sfi_curriter;
		Dee_Incref(curriter);
		SeqFlatIterator_LockRelease(self);
		result = DeeObject_IterNext(curriter);
		Dee_Decref_unlikely(curriter);
		if (result != ITER_DONE)
			return result; /* Error or success */

		/* Current iterator has been exhausted -> load the next one */
		new_curriter = DeeObject_IterNext(self->sfi_baseiter);
		if (!ITER_ISOK(new_curriter))
			return new_curriter; /* Error or ITER_DONE */
		SeqFlatIterator_LockAcquire(self);
		curriter = self->sfi_curriter;     /* Inherit reference */
		self->sfi_curriter = new_curriter; /* Inherit reference */
		SeqFlatIterator_LockRelease(self);
		Dee_Decref(curriter);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	__builtin_unreachable();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sfi_nextpair(SeqFlatIterator *__restrict self,
             DREF DeeObject *key_and_value[2]) {
	for (;;) {
		int result;
		DREF DeeObject *curriter;
		DREF DeeObject *new_curriter;
		SeqFlatIterator_LockAcquire(self);
		curriter = self->sfi_curriter;
		Dee_Incref(curriter);
		SeqFlatIterator_LockRelease(self);
		result = DeeObject_IterNextPair(curriter, key_and_value);
		Dee_Decref_unlikely(curriter);
		if (result <= 0)
			return result; /* Error or success */

		/* Current iterator has been exhausted -> load the next one */
		new_curriter = DeeObject_IterNext(self->sfi_baseiter);
		if (!ITER_ISOK(new_curriter)) {
			/* Error or ITER_DONE */
			if unlikely(!new_curriter)
				goto err;
			return 1;
		}
		SeqFlatIterator_LockAcquire(self);
		curriter = self->sfi_curriter;     /* Inherit reference */
		self->sfi_curriter = new_curriter; /* Inherit reference */
		SeqFlatIterator_LockRelease(self);
		Dee_Decref(curriter);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	__builtin_unreachable();
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sfi_nextkey(SeqFlatIterator *__restrict self) {
	for (;;) {
		DREF DeeObject *result;
		DREF DeeObject *curriter;
		DREF DeeObject *new_curriter;
		SeqFlatIterator_LockAcquire(self);
		curriter = self->sfi_curriter;
		Dee_Incref(curriter);
		SeqFlatIterator_LockRelease(self);
		result = DeeObject_IterNextKey(curriter);
		Dee_Decref_unlikely(curriter);
		if (result != ITER_DONE)
			return result; /* Error or success */

		/* Current iterator has been exhausted -> load the next one */
		new_curriter = DeeObject_IterNext(self->sfi_baseiter);
		if (!ITER_ISOK(new_curriter))
			return new_curriter; /* Error or ITER_DONE */
		SeqFlatIterator_LockAcquire(self);
		curriter = self->sfi_curriter;     /* Inherit reference */
		self->sfi_curriter = new_curriter; /* Inherit reference */
		SeqFlatIterator_LockRelease(self);
		Dee_Decref(curriter);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	__builtin_unreachable();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sfi_nextvalue(SeqFlatIterator *__restrict self) {
	for (;;) {
		DREF DeeObject *result;
		DREF DeeObject *curriter;
		DREF DeeObject *new_curriter;
		SeqFlatIterator_LockAcquire(self);
		curriter = self->sfi_curriter;
		Dee_Incref(curriter);
		SeqFlatIterator_LockRelease(self);
		result = DeeObject_IterNextValue(curriter);
		Dee_Decref_unlikely(curriter);
		if (result != ITER_DONE)
			return result; /* Error or success */

		/* Current iterator has been exhausted -> load the next one */
		new_curriter = DeeObject_IterNext(self->sfi_baseiter);
		if (!ITER_ISOK(new_curriter))
			return new_curriter; /* Error or ITER_DONE */
		SeqFlatIterator_LockAcquire(self);
		curriter = self->sfi_curriter;     /* Inherit reference */
		self->sfi_curriter = new_curriter; /* Inherit reference */
		SeqFlatIterator_LockRelease(self);
		Dee_Decref(curriter);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	__builtin_unreachable();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
sfi_advance(SeqFlatIterator *__restrict self, size_t step) {
	size_t result = 0;
	for (;;) {
		size_t part;
		DREF DeeObject *curriter;
		DREF DeeObject *new_curriter;
		SeqFlatIterator_LockAcquire(self);
		curriter = self->sfi_curriter;
		Dee_Incref(curriter);
		SeqFlatIterator_LockRelease(self);
		part = DeeObject_IterAdvance(curriter, step);
		Dee_Decref_unlikely(curriter);
		if unlikely(part == (size_t)-1)
			goto err;
		ASSERT(part <= step);
		result += part;
		step -= part;
		if (!step)
			break;

		/* Current iterator has been exhausted -> load the next one */
		new_curriter = DeeObject_IterNext(self->sfi_baseiter);
		if (!ITER_ISOK(new_curriter)) {
			/* Error or ITER_DONE */
			if unlikely(!new_curriter)
				goto err;
			break;
		}
		SeqFlatIterator_LockAcquire(self);
		curriter = self->sfi_curriter;     /* Inherit reference */
		self->sfi_curriter = new_curriter; /* Inherit reference */
		SeqFlatIterator_LockRelease(self);
		Dee_Decref(curriter);
		if (DeeThread_CheckInterrupt())
			goto err;
	}
	return result;
err:
	return (size_t)-1;
}

PRIVATE struct type_iterator sfi_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&sfi_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sfi_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sfi_nextvalue,
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&sfi_advance,
};

PRIVATE struct type_member sfi_members[] = {
	TYPE_MEMBER_FIELD_DOC("__baseiter__", STRUCT_OBJECT,
	                      offsetof(SeqFlatIterator, sfi_baseiter),
	                      "->?DIterator"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sfi_get_curriter(SeqFlatIterator *__restrict self) {
	DREF DeeObject *result;
	SeqFlatIterator_LockAcquire(self);
	result = self->sfi_curriter;
	Dee_Incref(result);
	SeqFlatIterator_LockRelease(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sfi_set_curriter(SeqFlatIterator *self, DeeObject *value) {
	DREF DeeObject *old_curriter;
	Dee_Incref(value);
	SeqFlatIterator_LockAcquire(self);
	old_curriter = self->sfi_curriter;
	self->sfi_curriter = value;
	SeqFlatIterator_LockRelease(self);
	Dee_Decref(old_curriter);
	return 0;
}

PRIVATE struct type_getset sfi_getsets[] = {
	TYPE_GETSET("__curriter__", &sfi_get_curriter, NULL, &sfi_set_curriter, "->?DIterator"),
	TYPE_GETSET_END
};

INTERN DeeTypeObject SeqFlatIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqFlatIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(flat:?Ert:SeqFlat)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&sfi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&sfi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&sfi_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&sfi_init,
				TYPE_FIXED_ALLOCATOR_GC(SeqFlatIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sfi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sfi_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sfi_visit,
	/* .tp_gc            = */ &sfi_gc,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sfi_next,
	/* .tp_iterator      = */ &sfi_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ sfi_getsets,
	/* .tp_members       = */ sfi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSeq_Flat(DeeObject *__restrict self) {
	DREF SeqFlat *result;
	result = DeeObject_MALLOC(SeqFlat);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->sf_seq = self;
	DeeObject_Init(result, &SeqFlat_Type);
	return (DREF DeeObject *)result;
err:
	return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_FLAT_C */
