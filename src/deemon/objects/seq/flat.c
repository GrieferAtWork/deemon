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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_FLAT_C
#define GUARD_DEEMON_OBJECTS_SEQ_FLAT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/gc.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/limitcore.h>
#include <hybrid/overflow.h>

/**/
#include "../../runtime/method-hint-defaults.h"
#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "flat.h"
/**/

#include <stddef.h> /* size_t */

#undef SSIZE_MIN
#undef SSIZE_MAX
#define SSIZE_MIN __SSIZE_MIN__
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* From "../tuple.c" */
tuple_mh_foreach_reverse(DeeTupleObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL /* From "../tuple.c" */
tuple_mh_enumerate_index_reverse(DeeTupleObject *__restrict self, Dee_seq_enumerate_index_t proc,
                                 void *arg, size_t start, size_t end);

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeSeq_InvokeForeachReverse(DeeObject *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_ssize_t result;
	DREF DeeTupleObject *astuple;
	DeeMH_seq_foreach_reverse_t op;
	op = DeeObject_RequireMethodHint(self, seq_foreach_reverse);
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
                                   Dee_seq_enumerate_index_t cb, void *arg,
                                   size_t start, size_t end) {
	Dee_ssize_t result;
	DREF DeeTupleObject *astuple;
	DeeMH_seq_enumerate_index_reverse_t op;
	op = DeeObject_RequireMethodHint(self, seq_enumerate_index_reverse);
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
		self->sfi_curriter = DeeIterator_NewEmpty();
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
#define sf_copy  generic_proxy__copy_alias
#define sf_deep  generic_proxy__deepcopy
#define sf_init  generic_proxy__init
#define sf_fini  generic_proxy__fini
#define sf_visit generic_proxy__visit

#define sf_foreachseq(self, cb, arg)         DeeObject_InvokeMethodHint(seq_operator_foreach, (self)->sf_seq, cb, arg)
#define sf_foreachseq_reverse(self, cb, arg) DeeSeq_InvokeForeachReverse((self)->sf_seq, cb, arg)

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_ctor(SeqFlat *__restrict self) {
	self->sf_seq = DeeSeq_NewEmpty();
	return 0;
}

#define SF_BOOL_FOREACH_YES SSIZE_MIN
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_bool_foreach_cb(void *UNUSED(arg), DeeObject *item) {
	Dee_ssize_t result = DeeObject_InvokeMethodHint(seq_operator_bool, item);
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

#define sf_trygetfirstseq(self) DeeObject_InvokeMethodHint(seq_trygetfirst, (self)->sf_seq)
#define sf_trygetlastseq(self)  DeeObject_InvokeMethodHint(seq_trygetlast, (self)->sf_seq)
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sf_getfirstseq(SeqFlat *__restrict self) {
	return DeeObject_InvokeMethodHint(seq_getfirst, self->sf_seq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_delfirstseq(SeqFlat *__restrict self) {
	return DeeObject_InvokeMethodHint(seq_delfirst, self->sf_seq);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sf_setfirstseq(SeqFlat *self, DeeObject *value) {
	return DeeObject_InvokeMethodHint(seq_setfirst, self->sf_seq, value);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sf_getlastseq(SeqFlat *__restrict self) {
	return DeeObject_InvokeMethodHint(seq_getlast, self->sf_seq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_dellastseq(SeqFlat *__restrict self) {
	return DeeObject_InvokeMethodHint(seq_dellast, self->sf_seq);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sf_setlastseq(SeqFlat *self, DeeObject *value) {
	return DeeObject_InvokeMethodHint(seq_setlast, self->sf_seq, value);
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
	contains_ob = DeeObject_InvokeMethodHint(seq_operator_contains, item, (DeeObject *)arg);
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
	return DeeObject_InvokeMethodHint(seq_operator_foreach, subseq, data->sffd_proc, data->sffd_arg);
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
	return DeeObject_InvokeMethodHint(seq_operator_foreach_pair, subseq,
	                                  data->sffpd_proc, data->sffpd_arg);
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
	size_t sum, *p_sum = (size_t *)arg;
	size_t subseq_size = DeeObject_InvokeMethodHint(seq_operator_size, subseq);
	if unlikely(subseq_size == (size_t)-1)
		goto err;
	if (OVERFLOW_UADD(*p_sum, subseq_size, &sum))
		goto err_overflow;
	*p_sum = sum;
	return 0;
err_overflow:
	DeeRT_ErrIntegerOverflowUAdd(*p_sum, subseq_size);
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
	DeeRT_ErrIntegerOverflowU(result, (size_t)-2);
err:
	return (size_t)-1;
}


struct sf_enumerate_index_data {
	Dee_seq_enumerate_index_t sfeid_proc;   /* [1..1] Underlying proc */
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
		return DeeObject_InvokeMethodHint(seq_operator_foreach, subseq, &sf_enumerate_index_foreach_inner_cb, data);
	subseq_fastsize = DeeObject_SizeFast(subseq);
	if (subseq_fastsize == (size_t)-1)
		return DeeObject_InvokeMethodHint(seq_operator_foreach, subseq, &sf_enumerate_index_foreach_inner_with_skip_cb, data);
	if (data->sfeid_skip >= subseq_fastsize) {
		data->sfeid_skip -= subseq_fastsize;
		return 0;
	}
	subseq_skip = data->sfeid_skip ;
	data->sfeid_skip = 0;
	return DeeObject_InvokeMethodHint(seq_enumerate_index, subseq,
	                                  &sf_enumerate_index_enumerate_inner_cb,
	                                  data, subseq_skip, (size_t)-1);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_enumerate_index(SeqFlat *__restrict self,
                          Dee_seq_enumerate_index_t proc,
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
	Dee_seq_enumerate_index_t sfeird_proc;   /* [1..1] Underlying proc */
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
	DeeMH_seq_enumerate_index_reverse_t ei_reverse_op;
	struct sf_enumerate_index_reverse_data *data;
	data = (struct sf_enumerate_index_reverse_data *)arg;
	if (!data->sfeird_skip)
		return DeeSeq_InvokeForeachReverse(subseq, &sf_enumerate_index_foreach_reverse_inner_cb, data);
	subseq_size = DeeObject_InvokeMethodHint(seq_operator_size, subseq);
	if (subseq_size == (size_t)-1)
		goto err;
	if (data->sfeird_skip >= subseq_size) {
		data->sfeird_skip -= subseq_size;
		return 0;
	}
	ei_reverse_op = DeeObject_RequireMethodHint(subseq, seq_enumerate_index_reverse);
	if (!ei_reverse_op) {
		DeeMH_seq_foreach_reverse_t fe_reverse_op;
		fe_reverse_op = DeeObject_RequireMethodHint(subseq, seq_foreach_reverse);
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
sf_mh_enumerate_index_reverse(SeqFlat *__restrict self, Dee_seq_enumerate_index_t proc,
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
	status = sf_mh_seq_enumerate_index(self, &sf_trygetitem_index_cb, &result, index, index + 1);
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sf_getitem_index(SeqFlat *__restrict self, size_t index) {
	size_t end_index;
	Dee_ssize_t status;
	DREF DeeObject *result;
	struct sf_enumerate_index_data data;
	if (OVERFLOW_UADD(index, 1, &end_index)) {
		size_t size = sf_size(self);
		if unlikely(size == (size_t)-1)
			goto err;
		DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, size);
		goto err;
	}
#ifndef NDEBUG
	result = NULL;
#endif /* !NDEBUG */

	//status = sf_mh_seq_enumerate_index(self, &sf_trygetitem_index_cb, &result, index, index + 1);
	data.sfeid_proc   = &sf_trygetitem_index_cb;
	data.sfeid_arg    = &result;
	data.sfeid_result = 0;
	data.sfeid_index  = index;
	data.sfeid_skip   = index;
	data.sfeid_end    = end_index;
	status = sf_foreachseq(self, &sf_enumerate_index_foreach_cb, &data);
	if (status == 0 || status == SF_ENUMERATE_INDEX_FOREACH_DONE) {
		status = data.sfeid_result;
	} else {
		ASSERT(status < 0);
	}
	ASSERT(status == 0 || status == -1 || status == SF_TRYGETITEM_INDEX_FOUND);
	if likely(status == SF_TRYGETITEM_INDEX_FOUND) {
#ifndef NDEBUG
		ASSERT(result != NULL);
#endif /* !NDEBUG */
		return result;
	}
	if unlikely(status < 0)
		goto err;
	DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, data.sfeid_index);
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
	subseq_size = DeeObject_InvokeMethodHint(seq_operator_size, subseq);
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
	DeeRT_ErrIndexOutOfBounds((DeeObject *)self, index, data.sfiwid_count);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_delitem_index_cb(DeeObject *__restrict subseq, size_t index,
                    DeeObject *UNUSED(cookie)) {
	return DeeObject_InvokeMethodHint(seq_operator_delitem_index, subseq, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_delitem_index(SeqFlat *__restrict self, size_t index) {
	return sf_interact_withitem(self, index, &sf_delitem_index_cb, NULL);
}

#define sf_setitem_index_cb default__seq_operator_setitem_index

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
sf_setitem_index(SeqFlat *__restrict self, size_t index, DeeObject *value) {
	return sf_interact_withitem(self, index, &sf_setitem_index_cb, value);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_xchitem_index_cb(DeeObject *__restrict subseq, size_t index,
                    DeeObject *cookie) {
	DREF DeeObject *old_value;
	DeeObject *new_value = *(DeeObject **)cookie;
	old_value = DeeObject_InvokeMethodHint(seq_xchitem_index, subseq, index, new_value);
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
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sf_contains,
	/* .tp_getitem            = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__delitem__with__delitem_index),
	/* .tp_setitem            = */ DEFIMPL(&default__setitem__with__setitem_index),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&sf_foreach,
	/* .tp_foreach_pair       = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&sf_foreach_pair,
	/* .tp_bounditem          = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__hasitem__with__bounditem),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&sf_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&sf_getitem_index,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&sf_delitem_index,
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&sf_setitem_index,
	/* .tp_bounditem_index    = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem         = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
	/* .tp_trygetitem_index   = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&sf_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__getitem_string_hash),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__bounditem_string_hash),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__bounditem_string_len_hash),
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
	return DeeObject_InvokeMethodHint(seq_clear, subseq);
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
	TYPE_METHOD_HINT_F(seq_enumerate_index, &sf_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
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
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	//TODO:TYPE_METHOD_HINTREF(Sequence_any),
	//TODO:TYPE_METHOD_HINTREF(Sequence_all),
	//TODO:TYPE_METHOD_HINTREF(Sequence_parity),
	//TODO:TYPE_METHOD_HINTREF(Sequence_min),
	//TODO:TYPE_METHOD_HINTREF(Sequence_max),
	//TODO:TYPE_METHOD_HINTREF(Sequence_count),
	//TODO:TYPE_METHOD_HINTREF(Sequence_contains),
	//TODO:TYPE_METHOD_HINTREF(Sequence_locate),
	//TODO:TYPE_METHOD_HINTREF(Sequence_rlocate),
	//TODO:TYPE_METHOD_HINTREF(Sequence_startswith),
	//TODO:TYPE_METHOD_HINTREF(Sequence_endswith),
	//TODO:TYPE_METHOD_HINTREF(Sequence_find),
	//TODO:TYPE_METHOD_HINTREF(Sequence_rfind),
	//TODO:TYPE_METHOD_HINTREF(Sequence_erase),
	//TODO:TYPE_METHOD_HINTREF(Sequence_insert),
	//TODO:TYPE_METHOD_HINTREF(Sequence_insertall),
	//TODO:TYPE_METHOD_HINTREF(Sequence_pushfront),
	//TODO:TYPE_METHOD_HINTREF(Sequence_append),
	//TODO:TYPE_METHOD_HINTREF(Sequence_extend),
	TYPE_METHOD_HINTREF(Sequence_xchitem),
	TYPE_METHOD_HINTREF(Sequence_clear),
	//TODO:TYPE_METHOD_HINTREF(Sequence_pop),
	//TODO:TYPE_METHOD_HINTREF(Sequence_remove),
	//TODO:TYPE_METHOD_HINTREF(Sequence_rremove),
	//TODO:TYPE_METHOD_HINTREF(Sequence_removeall),
	//TODO:TYPE_METHOD_HINTREF(Sequence_removeif),
	//TODO:TYPE_METHOD_HINTREF(Sequence_resize),
	//TODO:TYPE_METHOD_HINTREF(Sequence_fill),
	//TODO:TYPE_METHOD_HINTREF(Sequence_reverse),
	//TODO:TYPE_METHOD_HINTREF(Sequence_reversed),
	/* TODO: TYPE_METHOD_HINTREF(Sequence_sum),*/
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
				/* .tp_ctor      = */ (Dee_funptr_t)&sf_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&sf_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&sf_deep,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&sf_init,
				TYPE_FIXED_ALLOCATOR(SeqFlat)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sf_bool /* TODO: __seq__.some.operator bool() */,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__3C4D336761465F8A),
	/* .tp_seq           = */ &sf_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ sf_methods,
	/* .tp_getsets       = */ sf_getsets,
	/* .tp_members       = */ sf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ sf_class_members,
	/* .tp_method_hints  = */ sf_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};





/************************************************************************/
/* SeqFlatIterator                                                      */
/************************************************************************/
STATIC_ASSERT(offsetof(SeqFlatIterator, sfi_baseiter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqFlatIterator, sfi_baseiter) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(SeqFlatIterator, sfi_curriter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqFlatIterator, sfi_curriter) == offsetof(ProxyObject2, po_obj2));
#define sfi_fini generic_proxy2__fini

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
	_DeeArg_Unpack1(err, argc, argv, "_SeqFlatIterator", &flat);
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
	SeqFlatIterator_LockAcquire(self);
	old_curriter       = self->sfi_curriter;
	self->sfi_curriter = DeeIterator_NewEmpty();
	SeqFlatIterator_LockRelease(self);
	Dee_Decref(old_curriter);
}

PRIVATE struct type_gc tpconst sfi_gc = {
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
		DREF DeeObject *new_currseq;
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
		new_currseq = DeeObject_IterNext(self->sfi_baseiter);
		if (!ITER_ISOK(new_currseq))
			return new_currseq; /* Error or ITER_DONE */
		new_curriter = DeeObject_Iter(new_currseq);
		Dee_Decref(new_currseq);
		if unlikely(!new_curriter)
			goto err;
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
	TYPE_GETSET_AB("__curriter__", &sfi_get_curriter, NULL, &sfi_set_curriter, "->?DIterator"),
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
				/* .tp_ctor      = */ (Dee_funptr_t)&sfi_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&sfi_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&sfi_deep,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&sfi_init,
				TYPE_FIXED_ALLOCATOR_GC(SeqFlatIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sfi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sfi_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sfi_visit,
	/* .tp_gc            = */ &sfi_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__439AAF00B07ABA02),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sfi_next,
	/* .tp_iterator      = */ &sfi_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ sfi_getsets,
	/* .tp_members       = */ sfi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
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
