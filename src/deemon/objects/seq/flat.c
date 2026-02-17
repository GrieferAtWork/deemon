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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_FLAT_C
#define GUARD_DEEMON_OBJECTS_SEQ_FLAT_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/arg.h>                /* DeeArg_Unpack1 */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error-rt.h>           /* DeeRT_Err* */
#include <deemon/gc.h>                 /* DeeGCObject_FREE, DeeGCObject_MALLOC, DeeGC_TRACK */
#include <deemon/method-hints.h>       /* DeeMH_seq_enumerate_index_reverse_t, DeeMH_seq_foreach_reverse_t, DeeObject_InvokeMethodHint, DeeObject_RequireMethodHint, Dee_seq_enumerate_index_t, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_COMPARE_ERR, Dee_Decref*, Dee_Incref, Dee_Incref_n, Dee_XDecref, Dee_foreach_pair_t, Dee_foreach_t, Dee_ssize_t, Dee_visit_t, ITER_DONE, ITER_ISOK, OBJECT_HEAD_INIT */
#include <deemon/seq.h>                /* DeeIterator_NewEmpty, DeeIterator_Type, DeeSeq_NewEmpty, DeeSeq_Type, Dee_EmptyIterator */
#include <deemon/serial.h>             /* DeeSerial*, Dee_seraddr_t */
#include <deemon/thread.h>             /* DeeThread_CheckInterrupt */
#include <deemon/tuple.h>              /* DeeTupleObject, DeeTuple_FromSequence */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, Dee_Visit, METHOD_FNOREFESCAPE, STRUCT_OBJECT_AB, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/lock.h>          /* Dee_atomic_lock_init */

#include <hybrid/limitcore.h> /* __SSIZE_MAX__, __SSIZE_MIN__ */
#include <hybrid/overflow.h>  /* OVERFLOW_UADD */

#include "../../runtime/method-hint-defaults.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "flat.h"

#include <stddef.h> /* NULL, offsetof, size_t */

#undef SSIZE_MIN
#undef SSIZE_MAX
#define SSIZE_MIN __SSIZE_MIN__
#define SSIZE_MAX __SSIZE_MAX__

#ifndef CONFIG_TINY_DEEMON
#define WANT_sf_mh_seq_any
#define WANT_sf_mh_seq_contains
#define WANT_sf_mh_seq_all
#define WANT_sf_mh_seq_parity
#define WANT_sf_mh_seq_min
#define WANT_sf_mh_seq_max
#define WANT_sf_mh_seq_count
#define WANT_sf_mh_seq_find
#endif /* !CONFIG_TINY_DEEMON */

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
		self->sfi_curriter = DeeObject_Iter(firstseq);
		if unlikely(!self->sfi_curriter)
			goto err_baseiter_firstseq;
		Dee_Decref_unlikely(firstseq);
	}
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
#define sf_copy      generic_proxy__copy_alias
#define sf_init      generic_proxy__init
#define sf_serialize generic_proxy__serialize
#define sf_fini      generic_proxy__fini
#define sf_visit     generic_proxy__visit

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
	return DeeGC_TRACK(SeqFlatIterator, result);
err_r:
	DeeGCObject_FREE(result);
err:
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
sf_mh_seq_foreach_reverse_cb(void *arg, DeeObject *subseq) {
	struct sf_foreach_data *data;
	data = (struct sf_foreach_data *)arg;
	return DeeSeq_InvokeForeachReverse(subseq, data->sffd_proc, data->sffd_arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_foreach_reverse(SeqFlat *__restrict self, Dee_foreach_t proc, void *arg) {
	struct sf_foreach_data data;
	data.sffd_proc = proc;
	data.sffd_arg  = arg;
	return sf_foreachseq_reverse(self, &sf_mh_seq_foreach_reverse_cb, &data);
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
	/* TODO: FIXME: This is wrong -- don't use `seq_operator_foreach' here
	 *       -- that breaks when flattened sequences contain unbound items! */
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
	void                     *sfeird_arg;    /* [?..?] Cookie for `sfeird_proc' */
	Dee_ssize_t               sfeird_result; /* Nested enumeration result */
	size_t                    sfeird_index;  /* Next index */
	size_t                    sfeird_start;  /* Start index (stop enumeration when `sfeird_index') */
	size_t                    sfeird_skip;   /* Number of indices that still need to be skipped */
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
sf_mh_seq_enumerate_index_reverse_cb(void *arg, DeeObject *subseq) {
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
		/* TODO: FIXME: This is wrong -- don't use `seq_foreach_reverse' here
		 *       -- that breaks when flattened sequences contain unbound items! */
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
sf_mh_seq_enumerate_index_reverse(SeqFlat *__restrict self, Dee_seq_enumerate_index_t proc,
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
	status = sf_foreachseq_reverse(self, &sf_mh_seq_enumerate_index_reverse_cb, &data);
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
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, size);
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
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, data.sfeid_index);
err:
	return NULL;
}

typedef WUNUSED_T NONNULL_T((1)) int
(DCALL *sf_interact_withitem_cb_t)(DeeObject *__restrict subseq,
                                   size_t subseq_index, void *arg);

struct sf_interact_withitem_data {
	sf_interact_withitem_cb_t sfiwid_interact; /* [1..1] Interaction callback */
	void                     *sfiwid_arg;      /* [?..?] Interaction cookie */
	size_t                    sfiwid_index;    /* # of elements that still need to be skipped */
	size_t                    sfiwid_count;    /* Total # of elements from sub-sequences */
};

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
	if unlikely((*data->sfiwid_interact)(subseq, data->sfiwid_index, data->sfiwid_arg))
		goto err;
	return -2;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
sf_interact_withitem(SeqFlat *__restrict self, size_t index,
                     sf_interact_withitem_cb_t interact, void *arg) {
	Dee_ssize_t status;
	struct sf_interact_withitem_data data;
	data.sfiwid_interact = interact;
	data.sfiwid_arg      = arg;
	data.sfiwid_index    = index;
	data.sfiwid_count    = 0;
	status = sf_foreachseq(self, &sf_interact_withitem_cb, &data);
	ASSERT(status == 0 || status == -1 || status == -2);
	if likely(status == -2)
		return 0;
	if unlikely(status == 0) {
		/* Item not found -> index must be out-of-bounds */
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, data.sfiwid_count);
	}
	return -1;
}

struct sf_interact_withposition_data {
	sf_interact_withitem_cb_t sfiwpd_interact; /* [1..1] Interaction callback */
	void                     *sfiwpd_arg;      /* [?..?] Interaction cookie */
	size_t                    sfiwpd_index;    /* # of elements that still need to be skipped */
	size_t                    sfiwpd_count;    /* Total # of elements from sub-sequences */
	DREF DeeObject           *sfiwpd_lastseq;  /* [0..1] Last-enumerated sequence */
	size_t                    sfiwpd_lastidx;  /* [valid_if(sfiwpd_lastseq)] position in `sfiwpd_lastseq' */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_interact_withposition_cb(void *arg, DeeObject *subseq) {
	size_t subseq_size;
	struct sf_interact_withposition_data *data;
	data = (struct sf_interact_withposition_data *)arg;
	subseq_size = DeeObject_InvokeMethodHint(seq_operator_size, subseq);
	if unlikely(subseq_size == (size_t)-1)
		goto err;
	if (data->sfiwpd_index > subseq_size) {
		data->sfiwpd_lastidx = data->sfiwpd_index;
		Dee_XDecref(data->sfiwpd_lastseq);
		data->sfiwpd_lastseq = subseq;
		Dee_Incref(subseq);
		data->sfiwpd_index -= subseq_size;
		data->sfiwpd_count += subseq_size;
		return 0;
	}
	Dee_XDecref(data->sfiwpd_lastseq);
	if unlikely((*data->sfiwpd_interact)(subseq, data->sfiwpd_index, data->sfiwpd_arg))
		goto err;
	return -2;
err:
	return -1;
}

/* Interact with a given position (as needed to implement "insert") */
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
sf_interact_withposition(SeqFlat *__restrict self, size_t position,
                         sf_interact_withitem_cb_t interact, void *arg) {
	Dee_ssize_t status;
	struct sf_interact_withposition_data data;
	data.sfiwpd_interact = interact;
	data.sfiwpd_arg      = arg;
	data.sfiwpd_index    = position;
	data.sfiwpd_count    = 0;
	data.sfiwpd_lastseq  = NULL;
	status = sf_foreachseq(self, &sf_interact_withposition_cb, &data);
	ASSERT(status == 0 || status == -1 || status == -2);
	if likely(status == -2)
		return 0;
	if (status == 0) {
		int result;
		if unlikely(!data.sfiwpd_lastseq)
			return DeeRT_ErrEmptySequence(self);
		ASSERT(interact == data.sfiwpd_interact);
		ASSERT(arg == data.sfiwpd_arg);
		result = (*interact)(data.sfiwpd_lastseq, data.sfiwpd_lastidx, arg);
		Dee_Decref_unlikely(data.sfiwpd_lastseq);
		return result;
	}
	return -1;
}


/* @param: subseq:       The matching sub-sequence that containing. The
 *                       matched sub-range from the given [start,end) is
 *                       `[range_start,+=(subseq_end-subseq_start))'
 * @param: subseq_start: Start offset into `subseq' to interact with
 * @param: subseq_end:   End offset into `subseq' to interact with
 * @param: range_start:  Starting index in flattened super-seq matching
 *                       up with `subseq:subseq_start' */
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t
(DCALL *sf_interact_range_cb_t)(void *arg, DeeObject *subseq,
                                size_t subseq_start, size_t subseq_end,
                                size_t range_start);

struct sf_interact_range_foreach_data {
	sf_interact_range_cb_t sfirfd_cb;    /* [1..1] Interaction callback */
	void                  *sfirfd_arg;   /* [?..?] Cookie for `sfirfd_cb' */
	size_t                 sfirfd_start; /* # of elements from sub-sequences that must still be skipped */
	size_t                 sfirfd_range; /* Starting offset in super-sequence where next interaction range starts. */
	size_t                 sfirfd_count; /* # of elements that must still be enumerated. */
	Dee_ssize_t            sfirfd_sum;   /* Sum of return values of `sfirfd_cb', or negative for fast-fail */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
sf_interact_range_foreach_cb(void *arg, DeeObject *elem) {
	size_t subseq_end;
	struct sf_interact_range_foreach_data *data;
	data = (struct sf_interact_range_foreach_data *)arg;
	subseq_end = DeeObject_InvokeMethodHint(seq_operator_size, elem);
	if unlikely(subseq_end == (size_t)-1)
		goto err;
	if (data->sfirfd_start >= subseq_end) {
		/* Fully skip this sub-sequence */
		data->sfirfd_start -= subseq_end;
	} else {
		Dee_ssize_t temp;
		size_t subseq_start = data->sfirfd_start;
		size_t subseq_size = subseq_end - subseq_start;
		data->sfirfd_start = 0;
		if (subseq_size > data->sfirfd_count) {
			subseq_size = data->sfirfd_count;
			subseq_end = subseq_start + subseq_size;
		}

		/* Enumerate sub-sequence */
		temp = (*data->sfirfd_cb)(data->sfirfd_arg, elem, subseq_start,
		                          subseq_end, data->sfirfd_range);
		if unlikely(temp < 0) {
			data->sfirfd_sum = temp;
			return -2; /* Stop enumeration */
		}
		data->sfirfd_sum += temp;
		if (subseq_size >= data->sfirfd_count)
			return -2; /* Last sub-sequence was enumerated -> stop */
		data->sfirfd_range += subseq_size;
		data->sfirfd_count -= subseq_size;
	}
	return 0;
err:
	return -1;
}

/* Interact with the specified range within "self", invoking "interact" on
 * every sub-sequence containing at least 1 item from the given [start,end)
 * range.
 *
 * @param: start:    Start offset into `self' of sub-range to interact with
 * @param: end:      End offset into `self' of sub-range to interact with
 * @param: interact: Callback invoked on each matching sub-range.
 * @param: arg:      Cookie for `interact'
 * @return: >= 0: Sum of invocations of `iteract'
 * @return: < 0:  First negative return value of `iteract'
 * @return: -1:   An error was thrown, or `interact' returned `-1' */
PRIVATE WUNUSED NONNULL((1, 4)) Dee_ssize_t DCALL
sf_interact_withrange(SeqFlat *__restrict self, size_t start, size_t end,
                      sf_interact_range_cb_t interact, void *arg) {
	Dee_ssize_t result;
	struct sf_interact_range_foreach_data data;
	if (start >= end)
		return 0; /* Nothing to interact with */
	data.sfirfd_cb    = interact;
	data.sfirfd_arg   = arg;
	data.sfirfd_start = start;
	data.sfirfd_range = start;
	data.sfirfd_count = end - start;
	data.sfirfd_sum   = 0;
	result = DeeObject_InvokeMethodHint(seq_operator_foreach, self->sf_seq,
	                                    &sf_interact_range_foreach_cb, &data);
	if (result == 0 || result == -2)
		result = data.sfirfd_sum;
	return result;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_delitem_index_cb(DeeObject *__restrict subseq, size_t index, void *arg) {
	(void)arg;
	return DeeObject_InvokeMethodHint(seq_operator_delitem_index, subseq, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_delitem_index(SeqFlat *__restrict self, size_t index) {
	return sf_interact_withitem(self, index, &sf_delitem_index_cb, NULL);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
sf_setitem_index(SeqFlat *__restrict self, size_t index, DeeObject *value) {
	sf_interact_withitem_cb_t cb = (sf_interact_withitem_cb_t)&default__seq_operator_setitem_index;
	return sf_interact_withitem(self, index, cb, value);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_mh_seq_xchitem_index_cb(DeeObject *__restrict subseq, size_t index, void *cookie) {
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
sf_mh_seq_xchitem_index(SeqFlat *__restrict self, size_t index, DeeObject *value) {
	DREF DeeObject *result = value;
	if unlikely(sf_interact_withitem(self, index, &sf_mh_seq_xchitem_index_cb, &result))
		goto err;
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_clear_foreach_cb(void *UNUSED(arg), DeeObject *subseq) {
	return (Dee_ssize_t)DeeObject_InvokeMethodHint(seq_clear, subseq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_mh_seq_clear(SeqFlat *__restrict self) {
	return (int)sf_foreachseq(self, &sf_mh_seq_clear_foreach_cb, NULL);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_erase_cb(void *arg, DeeObject *subseq, size_t subseq_start,
                   size_t subseq_end, size_t range_start) {
	(void)arg;
	(void)range_start;
	return (Dee_ssize_t)DeeObject_InvokeMethodHint(seq_erase, subseq,
	                                               subseq_start,
	                                               subseq_end - subseq_start);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_mh_seq_erase(SeqFlat *__restrict self, size_t index, size_t count) {
	size_t end;
	if (OVERFLOW_UADD(index, count, &end))
		end = (size_t)-1;
	return (int)sf_interact_withrange(self, index, end, &sf_mh_seq_erase_cb, NULL);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_mh_seq_insert_cb(DeeObject *__restrict subseq, size_t subseq_index, void *arg) {
	return DeeObject_InvokeMethodHint(seq_insert, subseq, subseq_index, (DeeObject *)arg);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
sf_mh_seq_insert(SeqFlat *self, size_t index, DeeObject *item) {
	return sf_interact_withposition(self, index, &sf_mh_seq_insert_cb, item);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_mh_seq_insertall_cb(DeeObject *__restrict subseq, size_t subseq_index, void *arg) {
	return DeeObject_InvokeMethodHint(seq_insertall, subseq, subseq_index, (DeeObject *)arg);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
sf_mh_seq_insertall(SeqFlat *self, size_t index, DeeObject *items) {
	return sf_interact_withposition(self, index, &sf_mh_seq_insertall_cb, items);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sf_mh_seq_pushfront(SeqFlat *self, DeeObject *item) {
	int result;
	DREF DeeObject *seq = sf_getfirstseq(self);
	if unlikely(!seq)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_pushfront, seq, item);
	Dee_Decref_unlikely(seq);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sf_mh_seq_append(SeqFlat *self, DeeObject *item) {
	int result;
	DREF DeeObject *seq = sf_getlastseq(self);
	if unlikely(!seq)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_append, seq, item);
	Dee_Decref_unlikely(seq);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sf_mh_seq_extend(SeqFlat *self, DeeObject *item) {
	int result;
	DREF DeeObject *seq = sf_getlastseq(self);
	if unlikely(!seq)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_extend, seq, item);
	Dee_Decref_unlikely(seq);
	return result;
err:
	return -1;
}



#ifdef WANT_sf_mh_seq_any
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_any_cb(void *UNUSED(arg), DeeObject *subseq) {
	int result = DeeObject_InvokeMethodHint(seq_any, subseq);
	if (result > 0)
		result = -2;
	return (Dee_ssize_t)result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_mh_seq_any(SeqFlat *__restrict self) {
	int result = (int)sf_foreachseq(self, &sf_mh_seq_any_cb, NULL);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == -2)
		result = 1;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_any_with_key_cb(void *arg, DeeObject *subseq) {
	int result = DeeObject_InvokeMethodHint(seq_any_with_key, subseq, (DeeObject *)arg);
	if (result > 0)
		result = -2;
	return (Dee_ssize_t)result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sf_mh_seq_any_with_key(SeqFlat *self, DeeObject *key) {
	int result = (int)sf_foreachseq(self, &sf_mh_seq_any_with_key_cb, key);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == -2)
		result = 1;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_any_with_range_cb(void *UNUSED(arg), DeeObject *subseq,
                            size_t subseq_start, size_t subseq_end,
                            size_t UNUSED(range_start)) {
	int result = DeeObject_InvokeMethodHint(seq_any_with_range, subseq, subseq_start, subseq_end);
	if (result > 0)
		result = -2;
	return (Dee_ssize_t)result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_mh_seq_any_with_range(SeqFlat *__restrict self, size_t start, size_t end) {
	int result = (int)sf_interact_withrange(self, start, end, &sf_mh_seq_any_with_range_cb, NULL);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == -2)
		result = 1;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_any_with_range_and_key_cb(void *arg, DeeObject *subseq,
                                    size_t subseq_start, size_t subseq_end,
                                    size_t UNUSED(range_start)) {
	int result = DeeObject_InvokeMethodHint(seq_any_with_range_and_key, subseq,
	                                        subseq_start, subseq_end, (DeeObject *)arg);
	if (result > 0)
		result = -2;
	return (Dee_ssize_t)result;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
sf_mh_seq_any_with_range_and_key(SeqFlat *self, size_t start, size_t end, DeeObject *key) {
	int result = (int)sf_interact_withrange(self, start, end, &sf_mh_seq_any_with_range_and_key_cb, key);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == -2)
		result = 1;
	return result;
}
#endif /* WANT_sf_mh_seq_any */


#ifdef WANT_sf_mh_seq_all
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_all_cb(void *UNUSED(arg), DeeObject *subseq) {
	int result = DeeObject_InvokeMethodHint(seq_all, subseq);
	if (result > 0) {
		result = 0; /* Still all */
	} else if (result == 0) {
		result = -2; /* Not all */
	}
	return (Dee_ssize_t)result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_mh_seq_all(SeqFlat *__restrict self) {
	int result = (int)sf_foreachseq(self, &sf_mh_seq_all_cb, NULL);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == 0) {
		result = 1; /* Still all */
	} else if (result == -2) {
		result = 0; /* Not all */
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_all_with_key_cb(void *arg, DeeObject *subseq) {
	int result = DeeObject_InvokeMethodHint(seq_all_with_key, subseq, (DeeObject *)arg);
	if (result > 0) {
		result = 0; /* Still all */
	} else if (result == 0) {
		result = -2; /* Not all */
	}
	return (Dee_ssize_t)result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sf_mh_seq_all_with_key(SeqFlat *self, DeeObject *key) {
	int result = (int)sf_foreachseq(self, &sf_mh_seq_all_with_key_cb, key);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == 0) {
		result = 1; /* Still all */
	} else if (result == -2) {
		result = 0; /* Not all */
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_all_with_range_cb(void *UNUSED(arg), DeeObject *subseq,
                            size_t subseq_start, size_t subseq_end,
                            size_t UNUSED(range_start)) {
	int result = DeeObject_InvokeMethodHint(seq_all_with_range, subseq, subseq_start, subseq_end);
	if (result > 0) {
		result = 0; /* Still all */
	} else if (result == 0) {
		result = -2; /* Not all */
	}
	return (Dee_ssize_t)result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_mh_seq_all_with_range(SeqFlat *__restrict self, size_t start, size_t end) {
	int result = (int)sf_interact_withrange(self, start, end, &sf_mh_seq_all_with_range_cb, NULL);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == 0) {
		result = 1; /* Still all */
	} else if (result == -2) {
		result = 0; /* Not all */
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_all_with_range_and_key_cb(void *arg, DeeObject *subseq,
                                    size_t subseq_start, size_t subseq_end,
                                    size_t UNUSED(range_start)) {
	int result = DeeObject_InvokeMethodHint(seq_all_with_range_and_key, subseq,
	                                        subseq_start, subseq_end, (DeeObject *)arg);
	if (result > 0) {
		result = 0; /* Still all */
	} else if (result == 0) {
		result = -2; /* Not all */
	}
	return (Dee_ssize_t)result;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
sf_mh_seq_all_with_range_and_key(SeqFlat *self, size_t start, size_t end, DeeObject *key) {
	int result = (int)sf_interact_withrange(self, start, end, &sf_mh_seq_all_with_range_and_key_cb, key);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == 0) {
		result = 1; /* Still all */
	} else if (result == -2) {
		result = 0; /* Not all */
	}
	return result;
}
#endif /* WANT_sf_mh_seq_all */


#ifdef WANT_sf_mh_seq_parity
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_parity_cb(void *UNUSED(arg), DeeObject *subseq) {
	return (Dee_ssize_t)DeeObject_InvokeMethodHint(seq_parity, subseq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_mh_seq_parity(SeqFlat *__restrict self) {
	Dee_ssize_t result = sf_foreachseq(self, &sf_mh_seq_parity_cb, NULL);
	if (result >= 0)
		result &= 1;
	return (int)result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_parity_with_key_cb(void *arg, DeeObject *subseq) {
	return DeeObject_InvokeMethodHint(seq_parity_with_key, subseq, (DeeObject *)arg);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sf_mh_seq_parity_with_key(SeqFlat *self, DeeObject *key) {
	Dee_ssize_t result = sf_foreachseq(self, &sf_mh_seq_parity_with_key_cb, key);
	if (result >= 0)
		result &= 1;
	return (int)result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_parity_with_range_cb(void *UNUSED(arg), DeeObject *subseq,
                               size_t subseq_start, size_t subseq_end,
                               size_t UNUSED(range_start)) {
	return DeeObject_InvokeMethodHint(seq_parity_with_range, subseq, subseq_start, subseq_end);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sf_mh_seq_parity_with_range(SeqFlat *__restrict self, size_t start, size_t end) {
	Dee_ssize_t result = sf_interact_withrange(self, start, end, &sf_mh_seq_parity_with_range_cb, NULL);
	if (result >= 0)
		result &= 1;
	return (int)result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_parity_with_range_and_key_cb(void *arg, DeeObject *subseq,
                                       size_t subseq_start, size_t subseq_end,
                                       size_t UNUSED(range_start)) {
	return DeeObject_InvokeMethodHint(seq_parity_with_range_and_key, subseq,
	                                  subseq_start, subseq_end, (DeeObject *)arg);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
sf_mh_seq_parity_with_range_and_key(SeqFlat *self, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t result = sf_interact_withrange(self, start, end, &sf_mh_seq_parity_with_range_and_key_cb, key);
	if (result >= 0)
		result &= 1;
	return (int)result;
}
#endif /* WANT_sf_mh_seq_parity */


#ifdef WANT_sf_mh_seq_min
PRIVATE DeeObject sf_mh_seq_min_dummy = { OBJECT_HEAD_INIT(&DeeObject_Type) };

struct sf_mh_seq_min_data {
	DREF DeeObject *sfmhsmd_minval; /* [0..1] Lowest value encountered thus far. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_min_cb(void *arg, DeeObject *subseq) {
	DREF DeeObject *minval;
	struct sf_mh_seq_min_data *data = (struct sf_mh_seq_min_data *)arg;
	minval = DeeObject_InvokeMethodHint(seq_min, subseq, &sf_mh_seq_min_dummy);
	if unlikely(!minval)
		goto err;
	if (minval == &sf_mh_seq_min_dummy) {
		Dee_DecrefNokill(&sf_mh_seq_min_dummy);
	} else if (!data->sfmhsmd_minval) {
		data->sfmhsmd_minval = minval; /* Inherit reference */
	} else {
		int cmp = DeeObject_CmpLoAsBool(minval, data->sfmhsmd_minval);
		if unlikely(cmp < 0)
			goto err_minval;
		if (cmp) {
			DREF DeeObject *temp;
			temp = data->sfmhsmd_minval;
			data->sfmhsmd_minval = minval;
			minval = temp;
		}
		Dee_Decref(minval);
	}
	return 0;
err_minval:
	Dee_Decref(minval);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sf_mh_seq_min(SeqFlat *self, DeeObject *def) {
	struct sf_mh_seq_min_data data;
	data.sfmhsmd_minval = NULL;
	if unlikely(sf_foreachseq(self, &sf_mh_seq_min_cb, &data))
		goto err_data;
	if (data.sfmhsmd_minval == NULL) {
		data.sfmhsmd_minval = def;
		Dee_Incref(def);
	}
	return data.sfmhsmd_minval;
err_data:
	Dee_XDecref(data.sfmhsmd_minval);
	return NULL;
}


struct sf_mh_seq_min_with_key_data {
	DREF DeeObject *sfmhsmwkd_minval; /* [0..1] Lowest value encountered thus far */
	DeeObject      *sfmhsmwkd_key;    /* [1..1] Comparison key */
};

#ifndef DeeObject_CmpLoAsBoolWithKey_DEFINED
#define DeeObject_CmpLoAsBoolWithKey_DEFINED
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_CmpLoAsBoolWithKey(DeeObject *lhs, DeeObject *rhs, DeeObject *key) {
	int result;
	lhs = DeeObject_Call(key, 1, (DeeObject **)&lhs);
	if unlikely(!lhs)
		goto err;
	rhs = DeeObject_Call(key, 1, (DeeObject **)&rhs);
	if unlikely(!rhs)
		goto err_lhs;
	result = DeeObject_CmpLoAsBool(lhs, rhs);
	Dee_Decref(rhs);
	Dee_Decref(lhs);
	return result;
err_lhs:
	Dee_Decref(lhs);
err:
	return -1;
}
#endif /* !DeeObject_CmpLoAsBoolWithKey_DEFINED */

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_min_with_key_cb(void *arg, DeeObject *subseq) {
	DREF DeeObject *minval;
	struct sf_mh_seq_min_with_key_data *data = (struct sf_mh_seq_min_with_key_data *)arg;
	minval = DeeObject_InvokeMethodHint(seq_min_with_key, subseq, &sf_mh_seq_min_dummy, data->sfmhsmwkd_key);
	if unlikely(!minval)
		goto err;
	if (minval == &sf_mh_seq_min_dummy) {
		Dee_DecrefNokill(&sf_mh_seq_min_dummy);
	} else if (!data->sfmhsmwkd_minval) {
		data->sfmhsmwkd_minval = minval; /* Inherit reference */
	} else {
		int cmp = DeeObject_CmpLoAsBoolWithKey(minval, data->sfmhsmwkd_minval, data->sfmhsmwkd_key);
		if unlikely(cmp < 0)
			goto err_minval;
		if (cmp) {
			DREF DeeObject *temp;
			temp = data->sfmhsmwkd_minval;
			data->sfmhsmwkd_minval = minval;
			minval = temp;
		}
		Dee_Decref(minval);
	}
	return 0;
err_minval:
	Dee_Decref(minval);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
sf_mh_seq_min_with_key(SeqFlat *self, DeeObject *def, DeeObject *key) {
	struct sf_mh_seq_min_with_key_data data;
	data.sfmhsmwkd_minval = NULL;
	data.sfmhsmwkd_key    = key;
	if unlikely(sf_foreachseq(self, &sf_mh_seq_min_with_key_cb, &data))
		goto err_data;
	if (data.sfmhsmwkd_minval == NULL) {
		data.sfmhsmwkd_minval = def;
		Dee_Incref(def);
	}
	return data.sfmhsmwkd_minval;
err_data:
	Dee_XDecref(data.sfmhsmwkd_minval);
	return NULL;
}


struct sf_mh_seq_min_with_range_data {
	DREF DeeObject *sfmhsmwrd_minval; /* [0..1] Lowest value encountered thus far. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_min_with_range_cb(void *arg, DeeObject *subseq,
                            size_t subseq_start, size_t subseq_end,
                            size_t UNUSED(range_start)) {
	DREF DeeObject *minval;
	struct sf_mh_seq_min_with_range_data *data = (struct sf_mh_seq_min_with_range_data *)arg;
	minval = DeeObject_InvokeMethodHint(seq_min_with_range, subseq,
	                                    subseq_start, subseq_end,
	                                    &sf_mh_seq_min_dummy);
	if unlikely(!minval)
		goto err;
	if (minval == &sf_mh_seq_min_dummy) {
		Dee_DecrefNokill(&sf_mh_seq_min_dummy);
	} else if (!data->sfmhsmwrd_minval) {
		data->sfmhsmwrd_minval = minval; /* Inherit reference */
	} else {
		int cmp = DeeObject_CmpLoAsBool(minval, data->sfmhsmwrd_minval);
		if unlikely(cmp < 0)
			goto err_minval;
		if (cmp) {
			DREF DeeObject *temp;
			temp = data->sfmhsmwrd_minval;
			data->sfmhsmwrd_minval = minval;
			minval = temp;
		}
		Dee_Decref(minval);
	}
	return 0;
err_minval:
	Dee_Decref(minval);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
sf_mh_seq_min_with_range(SeqFlat *self, size_t start, size_t end, DeeObject *def) {
	struct sf_mh_seq_min_with_range_data data;
	data.sfmhsmwrd_minval = NULL;
	if unlikely(sf_interact_withrange(self, start, end, &sf_mh_seq_min_with_range_cb, &data))
		goto err_data;
	if (data.sfmhsmwrd_minval == NULL) {
		data.sfmhsmwrd_minval = def;
		Dee_Incref(def);
	}
	return data.sfmhsmwrd_minval;
err_data:
	Dee_XDecref(data.sfmhsmwrd_minval);
	return NULL;
}

struct sf_mh_seq_min_with_range_and_key_data {
	DREF DeeObject *sfmhsmwrakd_minval; /* [0..1] Lowest value encountered thus far. */
	DeeObject      *sfmhsmwkd_key;      /* [1..1] Comparison key */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_min_with_range_and_key_cb(void *arg, DeeObject *subseq,
                                    size_t subseq_start, size_t subseq_end,
                                    size_t UNUSED(range_start)) {
	DREF DeeObject *minval;
	struct sf_mh_seq_min_with_range_and_key_data *data = (struct sf_mh_seq_min_with_range_and_key_data *)arg;
	minval = DeeObject_InvokeMethodHint(seq_min_with_range_and_key, subseq,
	                                    subseq_start, subseq_end,
	                                    &sf_mh_seq_min_dummy,
	                                    data->sfmhsmwkd_key);
	if unlikely(!minval)
		goto err;
	if (minval == &sf_mh_seq_min_dummy) {
		Dee_DecrefNokill(&sf_mh_seq_min_dummy);
	} else if (!data->sfmhsmwrakd_minval) {
		data->sfmhsmwrakd_minval = minval; /* Inherit reference */
	} else {
		int cmp = DeeObject_CmpLoAsBoolWithKey(minval, data->sfmhsmwrakd_minval, data->sfmhsmwkd_key);
		if unlikely(cmp < 0)
			goto err_minval;
		if (cmp) {
			DREF DeeObject *temp;
			temp = data->sfmhsmwrakd_minval;
			data->sfmhsmwrakd_minval = minval;
			minval = temp;
		}
		Dee_Decref(minval);
	}
	return 0;
err_minval:
	Dee_Decref(minval);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4, 4)) DREF DeeObject *DCALL
sf_mh_seq_min_with_range_and_key(SeqFlat *self, size_t start, size_t end, DeeObject *def, DeeObject *key) {
	struct sf_mh_seq_min_with_range_and_key_data data;
	data.sfmhsmwrakd_minval = NULL;
	data.sfmhsmwkd_key      = key;
	if unlikely(sf_interact_withrange(self, start, end, &sf_mh_seq_min_with_range_and_key_cb, &data))
		goto err_data;
	if (data.sfmhsmwrakd_minval == NULL) {
		data.sfmhsmwrakd_minval = def;
		Dee_Incref(def);
	}
	return data.sfmhsmwrakd_minval;
err_data:
	Dee_XDecref(data.sfmhsmwrakd_minval);
	return NULL;
}
#endif /* WANT_sf_mh_seq_min */


#ifdef WANT_sf_mh_seq_max
#ifdef WANT_sf_mh_seq_min
#define sf_mh_seq_max_dummy sf_mh_seq_min_dummy
#else /* WANT_sf_mh_seq_min */
PRIVATE DeeObject sf_mh_seq_max_dummy = { OBJECT_HEAD_INIT(&DeeObject_Type) };
#endif /* !WANT_sf_mh_seq_min */

struct sf_mh_seq_max_data {
	DREF DeeObject *sfmhsmd_maxval; /* [0..1] Greatest value encountered thus far. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_max_cb(void *arg, DeeObject *subseq) {
	DREF DeeObject *maxval;
	struct sf_mh_seq_max_data *data = (struct sf_mh_seq_max_data *)arg;
	maxval = DeeObject_InvokeMethodHint(seq_max, subseq, &sf_mh_seq_max_dummy);
	if unlikely(!maxval)
		goto err;
	if (maxval == &sf_mh_seq_max_dummy) {
		Dee_DecrefNokill(&sf_mh_seq_max_dummy);
	} else if (!data->sfmhsmd_maxval) {
		data->sfmhsmd_maxval = maxval; /* Inherit reference */
	} else {
		int cmp = DeeObject_CmpGrAsBool(maxval, data->sfmhsmd_maxval);
		if unlikely(cmp < 0)
			goto err_maxval;
		if (cmp) {
			DREF DeeObject *temp;
			temp = data->sfmhsmd_maxval;
			data->sfmhsmd_maxval = maxval;
			maxval = temp;
		}
		Dee_Decref(maxval);
	}
	return 0;
err_maxval:
	Dee_Decref(maxval);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sf_mh_seq_max(SeqFlat *self, DeeObject *def) {
	struct sf_mh_seq_max_data data;
	data.sfmhsmd_maxval = NULL;
	if unlikely(sf_foreachseq(self, &sf_mh_seq_max_cb, &data))
		goto err_data;
	if (data.sfmhsmd_maxval == NULL) {
		data.sfmhsmd_maxval = def;
		Dee_Incref(def);
	}
	return data.sfmhsmd_maxval;
err_data:
	Dee_XDecref(data.sfmhsmd_maxval);
	return NULL;
}


struct sf_mh_seq_max_with_key_data {
	DREF DeeObject *sfmhsmwkd_maxval; /* [0..1] Greatest value encountered thus far */
	DeeObject      *sfmhsmwkd_key;    /* [1..1] Comparison key */
};

#ifndef DeeObject_CmpGrAsBoolWithKey_DEFINED
#define DeeObject_CmpGrAsBoolWithKey_DEFINED
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeObject_CmpGrAsBoolWithKey(DeeObject *lhs, DeeObject *rhs, DeeObject *key) {
	int result;
	lhs = DeeObject_Call(key, 1, (DeeObject **)&lhs);
	if unlikely(!lhs)
		goto err;
	rhs = DeeObject_Call(key, 1, (DeeObject **)&rhs);
	if unlikely(!rhs)
		goto err_lhs;
	result = DeeObject_CmpGrAsBool(lhs, rhs);
	Dee_Decref(rhs);
	Dee_Decref(lhs);
	return result;
err_lhs:
	Dee_Decref(lhs);
err:
	return -1;
}
#endif /* !DeeObject_CmpGrAsBoolWithKey_DEFINED */

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_max_with_key_cb(void *arg, DeeObject *subseq) {
	DREF DeeObject *maxval;
	struct sf_mh_seq_max_with_key_data *data = (struct sf_mh_seq_max_with_key_data *)arg;
	maxval = DeeObject_InvokeMethodHint(seq_max_with_key, subseq, &sf_mh_seq_max_dummy, data->sfmhsmwkd_key);
	if unlikely(!maxval)
		goto err;
	if (maxval == &sf_mh_seq_max_dummy) {
		Dee_DecrefNokill(&sf_mh_seq_max_dummy);
	} else if (!data->sfmhsmwkd_maxval) {
		data->sfmhsmwkd_maxval = maxval; /* Inherit reference */
	} else {
		int cmp = DeeObject_CmpGrAsBoolWithKey(maxval, data->sfmhsmwkd_maxval, data->sfmhsmwkd_key);
		if unlikely(cmp < 0)
			goto err_maxval;
		if (cmp) {
			DREF DeeObject *temp;
			temp = data->sfmhsmwkd_maxval;
			data->sfmhsmwkd_maxval = maxval;
			maxval = temp;
		}
		Dee_Decref(maxval);
	}
	return 0;
err_maxval:
	Dee_Decref(maxval);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
sf_mh_seq_max_with_key(SeqFlat *self, DeeObject *def, DeeObject *key) {
	struct sf_mh_seq_max_with_key_data data;
	data.sfmhsmwkd_maxval = NULL;
	data.sfmhsmwkd_key    = key;
	if unlikely(sf_foreachseq(self, &sf_mh_seq_max_with_key_cb, &data))
		goto err_data;
	if (data.sfmhsmwkd_maxval == NULL) {
		data.sfmhsmwkd_maxval = def;
		Dee_Incref(def);
	}
	return data.sfmhsmwkd_maxval;
err_data:
	Dee_XDecref(data.sfmhsmwkd_maxval);
	return NULL;
}


struct sf_mh_seq_max_with_range_data {
	DREF DeeObject *sfmhsmwrd_maxval; /* [0..1] Greatest value encountered thus far. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_max_with_range_cb(void *arg, DeeObject *subseq,
                            size_t subseq_start, size_t subseq_end,
                            size_t UNUSED(range_start)) {
	DREF DeeObject *maxval;
	struct sf_mh_seq_max_with_range_data *data = (struct sf_mh_seq_max_with_range_data *)arg;
	maxval = DeeObject_InvokeMethodHint(seq_max_with_range, subseq,
	                                    subseq_start, subseq_end,
	                                    &sf_mh_seq_max_dummy);
	if unlikely(!maxval)
		goto err;
	if (maxval == &sf_mh_seq_max_dummy) {
		Dee_DecrefNokill(&sf_mh_seq_max_dummy);
	} else if (!data->sfmhsmwrd_maxval) {
		data->sfmhsmwrd_maxval = maxval; /* Inherit reference */
	} else {
		int cmp = DeeObject_CmpGrAsBool(maxval, data->sfmhsmwrd_maxval);
		if unlikely(cmp < 0)
			goto err_maxval;
		if (cmp) {
			DREF DeeObject *temp;
			temp = data->sfmhsmwrd_maxval;
			data->sfmhsmwrd_maxval = maxval;
			maxval = temp;
		}
		Dee_Decref(maxval);
	}
	return 0;
err_maxval:
	Dee_Decref(maxval);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
sf_mh_seq_max_with_range(SeqFlat *self, size_t start, size_t end, DeeObject *def) {
	struct sf_mh_seq_max_with_range_data data;
	data.sfmhsmwrd_maxval = NULL;
	if unlikely(sf_interact_withrange(self, start, end, &sf_mh_seq_max_with_range_cb, &data))
		goto err_data;
	if (data.sfmhsmwrd_maxval == NULL) {
		data.sfmhsmwrd_maxval = def;
		Dee_Incref(def);
	}
	return data.sfmhsmwrd_maxval;
err_data:
	Dee_XDecref(data.sfmhsmwrd_maxval);
	return NULL;
}

struct sf_mh_seq_max_with_range_and_key_data {
	DREF DeeObject *sfmhsmwrakd_maxval; /* [0..1] Greatest value encountered thus far. */
	DeeObject      *sfmhsmwkd_key;      /* [1..1] Comparison key */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_max_with_range_and_key_cb(void *arg, DeeObject *subseq,
                                    size_t subseq_start, size_t subseq_end,
                                    size_t UNUSED(range_start)) {
	DREF DeeObject *maxval;
	struct sf_mh_seq_max_with_range_and_key_data *data = (struct sf_mh_seq_max_with_range_and_key_data *)arg;
	maxval = DeeObject_InvokeMethodHint(seq_max_with_range_and_key, subseq,
	                                    subseq_start, subseq_end,
	                                    &sf_mh_seq_max_dummy,
	                                    data->sfmhsmwkd_key);
	if unlikely(!maxval)
		goto err;
	if (maxval == &sf_mh_seq_max_dummy) {
		Dee_DecrefNokill(&sf_mh_seq_max_dummy);
	} else if (!data->sfmhsmwrakd_maxval) {
		data->sfmhsmwrakd_maxval = maxval; /* Inherit reference */
	} else {
		int cmp = DeeObject_CmpGrAsBoolWithKey(maxval, data->sfmhsmwrakd_maxval, data->sfmhsmwkd_key);
		if unlikely(cmp < 0)
			goto err_maxval;
		if (cmp) {
			DREF DeeObject *temp;
			temp = data->sfmhsmwrakd_maxval;
			data->sfmhsmwrakd_maxval = maxval;
			maxval = temp;
		}
		Dee_Decref(maxval);
	}
	return 0;
err_maxval:
	Dee_Decref(maxval);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4, 4)) DREF DeeObject *DCALL
sf_mh_seq_max_with_range_and_key(SeqFlat *self, size_t start, size_t end, DeeObject *def, DeeObject *key) {
	struct sf_mh_seq_max_with_range_and_key_data data;
	data.sfmhsmwrakd_maxval = NULL;
	data.sfmhsmwkd_key      = key;
	if unlikely(sf_interact_withrange(self, start, end, &sf_mh_seq_max_with_range_and_key_cb, &data))
		goto err_data;
	if (data.sfmhsmwrakd_maxval == NULL) {
		data.sfmhsmwrakd_maxval = def;
		Dee_Incref(def);
	}
	return data.sfmhsmwrakd_maxval;
err_data:
	Dee_XDecref(data.sfmhsmwrakd_maxval);
	return NULL;
}
#endif /* WANT_sf_mh_seq_max */


#ifdef WANT_sf_mh_seq_count
struct sf_mh_seq_count_data {
	DeeObject *sfmhscd_item;  /* [1..1] Item to count */
	size_t     sfmhscd_count; /* Count total */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_count_cb(void *arg, DeeObject *subseq) {
	size_t count, total;
	struct sf_mh_seq_count_data *data;
	data  = (struct sf_mh_seq_count_data *)arg;
	count = DeeObject_InvokeMethodHint(seq_count, subseq, data->sfmhscd_item);
	if unlikely(count == (size_t)-1)
		goto err;
	if (OVERFLOW_UADD(data->sfmhscd_count, count, &total))
		goto err_overflow;
	data->sfmhscd_count = total;
	return 0;
err_overflow:
	DeeRT_ErrIntegerOverflowUAdd(data->sfmhscd_count, count);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
sf_mh_seq_count(SeqFlat *self, DeeObject *item) {
	struct sf_mh_seq_count_data data;
	data.sfmhscd_item  = item;
	data.sfmhscd_count = 0;
	if unlikely(sf_foreachseq(self, &sf_mh_seq_count_cb, &data))
		data.sfmhscd_count = (size_t)-1;
	return data.sfmhscd_count;
}


struct sf_mh_seq_count_with_key_data {
	DeeObject *sfmhscwkd_item;  /* [1..1] Item to count */
	DeeObject *sfmhscwkd_key;   /* [1..1] Key mapper */
	size_t     sfmhscwkd_count; /* Count total */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_count_with_key_cb(void *arg, DeeObject *subseq) {
	size_t count, total;
	struct sf_mh_seq_count_with_key_data *data;
	data  = (struct sf_mh_seq_count_with_key_data *)arg;
	count = DeeObject_InvokeMethodHint(seq_count_with_key, subseq,
	                                   data->sfmhscwkd_item,
	                                   data->sfmhscwkd_key);
	if unlikely(count == (size_t)-1)
		goto err;
	if (OVERFLOW_UADD(data->sfmhscwkd_count, count, &total))
		goto err_overflow;
	data->sfmhscwkd_count = total;
	return 0;
err_overflow:
	DeeRT_ErrIntegerOverflowUAdd(data->sfmhscwkd_count, count);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) size_t DCALL
sf_mh_seq_count_with_key(SeqFlat *self, DeeObject *item, DeeObject *key) {
	struct sf_mh_seq_count_with_key_data data;
	data.sfmhscwkd_item  = item;
	data.sfmhscwkd_key   = key;
	data.sfmhscwkd_count = 0;
	if unlikely(sf_foreachseq(self, &sf_mh_seq_count_with_key_cb, &data))
		data.sfmhscwkd_count = (size_t)-1;
	return data.sfmhscwkd_count;
}


struct sf_mh_seq_count_with_range_data {
	DeeObject *sfmhscwrd_item;  /* [1..1] Item to count */
	size_t     sfmhscwrd_count; /* Count total */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_count_with_range_cb(void *arg, DeeObject *subseq,
                              size_t subseq_start, size_t subseq_end,
                              size_t UNUSED(range_start)) {
	size_t count, total;
	struct sf_mh_seq_count_with_range_data *data;
	data  = (struct sf_mh_seq_count_with_range_data *)arg;
	count = DeeObject_InvokeMethodHint(seq_count_with_range, subseq,
	                                   data->sfmhscwrd_item,
	                                   subseq_start, subseq_end);
	if unlikely(count == (size_t)-1)
		goto err;
	if (OVERFLOW_UADD(data->sfmhscwrd_count, count, &total))
		goto err_overflow;
	data->sfmhscwrd_count = total;
	return 0;
err_overflow:
	DeeRT_ErrIntegerOverflowUAdd(data->sfmhscwrd_count, count);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
sf_mh_seq_count_with_range(SeqFlat *self, DeeObject *item, size_t start, size_t end) {
	struct sf_mh_seq_count_with_range_data data;
	data.sfmhscwrd_item  = item;
	data.sfmhscwrd_count = 0;
	if unlikely(sf_interact_withrange(self, start, end, &sf_mh_seq_count_with_range_cb, &data))
		data.sfmhscwrd_count = (size_t)-1;
	return data.sfmhscwrd_count;
}

struct sf_mh_seq_count_with_range_and_key_data {
	DeeObject *sfmhscwrd_item;  /* [1..1] Item to count */
	DeeObject *sfmhscwrd_key;   /* [1..1] Key mapper */
	size_t     sfmhscwrd_count; /* Count total */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
sf_mh_seq_count_with_range_and_key_cb(void *arg, DeeObject *subseq,
                              size_t subseq_start, size_t subseq_end,
                              size_t UNUSED(range_start)) {
	size_t count, total;
	struct sf_mh_seq_count_with_range_and_key_data *data;
	data  = (struct sf_mh_seq_count_with_range_and_key_data *)arg;
	count = DeeObject_InvokeMethodHint(seq_count_with_range_and_key, subseq,
	                                   data->sfmhscwrd_item,
	                                   subseq_start, subseq_end,
	                                   data->sfmhscwrd_key);
	if unlikely(count == (size_t)-1)
		goto err;
	if (OVERFLOW_UADD(data->sfmhscwrd_count, count, &total))
		goto err_overflow;
	data->sfmhscwrd_count = total;
	return 0;
err_overflow:
	DeeRT_ErrIntegerOverflowUAdd(data->sfmhscwrd_count, count);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
sf_mh_seq_count_with_range_and_key(SeqFlat *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	struct sf_mh_seq_count_with_range_and_key_data data;
	data.sfmhscwrd_item  = item;
	data.sfmhscwrd_key   = key;
	data.sfmhscwrd_count = 0;
	if unlikely(sf_interact_withrange(self, start, end, &sf_mh_seq_count_with_range_and_key_cb, &data))
		data.sfmhscwrd_count = (size_t)-1;
	return data.sfmhscwrd_count;
}
#endif /* WANT_sf_mh_seq_count */


#ifdef WANT_sf_mh_seq_contains
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_contains_cb(void *arg, DeeObject *subseq) {
	int result = DeeObject_InvokeMethodHint(seq_contains, subseq, (DeeObject *)arg);
	if (result > 0)
		result = -2;
	return (Dee_ssize_t)result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sf_mh_seq_contains(SeqFlat *self, DeeObject *item) {
	int result = (int)sf_foreachseq(self, &sf_mh_seq_contains_cb, item);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == -2)
		result = 1;
	return result;
}

struct sf_mh_seq_contains_with_key_data {
	DeeObject *sfmhscwk_item; /* [1..1] Item to find */
	DeeObject *sfmhscwk_key;  /* [1..1] Key mapper */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_contains_with_key_cb(void *arg, DeeObject *subseq) {
	struct sf_mh_seq_contains_with_key_data *data = (struct sf_mh_seq_contains_with_key_data *)arg;
	int result = DeeObject_InvokeMethodHint(seq_contains_with_key, subseq, data->sfmhscwk_item, data->sfmhscwk_key);
	if (result > 0)
		result = -2;
	return (Dee_ssize_t)result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
sf_mh_seq_contains_with_key(SeqFlat *self, DeeObject *item, DeeObject *key) {
	int result;
	struct sf_mh_seq_contains_with_key_data data;
	data.sfmhscwk_item = item;
	data.sfmhscwk_key  = key;
	result = (int)sf_foreachseq(self, &sf_mh_seq_contains_with_key_cb, &data);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == -2)
		result = 1;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_contains_with_range_cb(void *arg, DeeObject *subseq,
                                 size_t subseq_start, size_t subseq_end,
                                 size_t UNUSED(range_start)) {
	int result = DeeObject_InvokeMethodHint(seq_contains_with_range,
	                                        subseq, (DeeObject *)arg,
	                                        subseq_start, subseq_end);
	if (result > 0)
		result = -2;
	return (Dee_ssize_t)result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
sf_mh_seq_contains_with_range(SeqFlat *self, DeeObject *item, size_t start, size_t end) {
	int result = (int)sf_interact_withrange(self, start, end, &sf_mh_seq_contains_with_range_cb, item);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == -2)
		result = 1;
	return result;
}

struct sf_mh_seq_contains_with_range_and_key_data {
	DeeObject *sfmhscwrak_item; /* [1..1] Item to find */
	DeeObject *sfmhscwrak_key;  /* [1..1] Key mapper */
};
PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_contains_with_range_and_key_cb(void *arg, DeeObject *subseq,
                                         size_t subseq_start, size_t subseq_end,
                                         size_t UNUSED(range_start)) {
	struct sf_mh_seq_contains_with_range_and_key_data *data = (struct sf_mh_seq_contains_with_range_and_key_data *)arg;
	int result = DeeObject_InvokeMethodHint(seq_contains_with_range_and_key,
	                                        subseq, data->sfmhscwrak_item,
	                                        subseq_start, subseq_end,
	                                        data->sfmhscwrak_key);
	if (result > 0)
		result = -2;
	return (Dee_ssize_t)result;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
sf_mh_seq_contains_with_range_and_key(SeqFlat *self, DeeObject *item,
                                      size_t start, size_t end, DeeObject *key) {
	int result;
	struct sf_mh_seq_contains_with_range_and_key_data data;
	data.sfmhscwrak_item = item;
	data.sfmhscwrak_key  = key;
	result = (int)sf_interact_withrange(self, start, end, &sf_mh_seq_contains_with_range_and_key_cb, &data);
	ASSERT(result == 0 || result == -1 || result == -2);
	if (result == -2)
		result = 1;
	return result;
}
#endif /* WANT_sf_mh_seq_contains */


#ifdef WANT_sf_mh_seq_find
union sf_mh_seq_find_data {
	struct {
		DeeObject *s_item;    /* [1..1] Item to find */
	}          sfmhfd_search; /* [valid_if(NOT_RETURNED)] Item to find */
	size_t     sfmhfd_index;  /* [valid_if(return == -2)] Discovered index */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_find_cb(void *arg, DeeObject *subseq,
                  size_t subseq_start, size_t subseq_end,
                  size_t range_start) {
	union sf_mh_seq_find_data *data = (union sf_mh_seq_find_data *)arg;
	size_t index = DeeObject_InvokeMethodHint(seq_find, subseq,
	                                          data->sfmhfd_search.s_item,
	                                          subseq_start, subseq_end);
	if (index != (size_t)-1) {
		if (index == (size_t)Dee_COMPARE_ERR)
			return -1;
		data->sfmhfd_index = range_start + index;
		return -2;
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
sf_mh_seq_find(SeqFlat *self, DeeObject *item, size_t start, size_t end) {
	Dee_ssize_t status;
	union sf_mh_seq_find_data data;
	data.sfmhfd_search.s_item = item;
	status = sf_interact_withrange(self, start, end, &sf_mh_seq_find_cb, &data);
	ASSERT(status == 0 || status == -1 || status == -2);
	if (status == -2)
		return data.sfmhfd_index;
	if (status == -1)
		return (size_t)Dee_COMPARE_ERR;
	return (size_t)-1;
}

union sf_mh_seq_find_with_key_data {
	struct {
		DeeObject *s_item;  /* [1..1] Item to find */
		DeeObject *s_key;   /* [1..1] Find key */
	}          sfmhfwkd_search; /* [valid_if(NOT_RETURNED)] Item to find */
	size_t     sfmhfwkd_index;  /* [valid_if(return == -2)] Discovered index */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
sf_mh_seq_find_with_key_cb(void *arg, DeeObject *subseq,
                           size_t subseq_start, size_t subseq_end,
                           size_t range_start) {
	union sf_mh_seq_find_with_key_data *data = (union sf_mh_seq_find_with_key_data *)arg;
	size_t index = DeeObject_InvokeMethodHint(seq_find_with_key, subseq,
	                                          data->sfmhfwkd_search.s_item,
	                                          subseq_start, subseq_end,
	                                          data->sfmhfwkd_search.s_key);
	if (index != (size_t)-1) {
		if (index == (size_t)Dee_COMPARE_ERR)
			return -1;
		data->sfmhfwkd_index = range_start + index;
		return -2;
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
sf_mh_seq_find_with_key(SeqFlat *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	Dee_ssize_t status;
	union sf_mh_seq_find_with_key_data data;
	data.sfmhfwkd_search.s_item = item;
	data.sfmhfwkd_search.s_key  = key;
	status = sf_interact_withrange(self, start, end, &sf_mh_seq_find_with_key_cb, &data);
	ASSERT(status == 0 || status == -1 || status == -2);
	if (status == -2)
		return data.sfmhfwkd_index;
	if (status == -1)
		return (size_t)Dee_COMPARE_ERR;
	return (size_t)-1;
}
#endif /* WANT_sf_mh_seq_find */


PRIVATE struct type_getset tpconst sf_getsets[] = {
	//TODO:TYPE_GETSET_F_NODOC(STR_first, &sf_getfirst, &sf_delfirst, &sf_setfirst, METHOD_FNOREFESCAPE),
	//TODO:TYPE_GETSET_F_NODOC(STR_last, &sf_getlast, &sf_dellast, &sf_setlast, METHOD_FNOREFESCAPE),
	TYPE_GETSET_F("__firstseq__", &sf_getfirstseq, &sf_delfirstseq, &sf_setfirstseq, METHOD_FNOREFESCAPE, "->?DSequence"),
	TYPE_GETSET_F("__lastseq__", &sf_getlastseq, &sf_dellastseq, &sf_setlastseq, METHOD_FNOREFESCAPE, "->?DSequence"),
	TYPE_GETSET_END
};

PRIVATE struct type_method tpconst sf_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
#ifdef WANT_sf_mh_seq_any
	TYPE_METHOD_HINTREF(Sequence_any),
#endif /* WANT_sf_mh_seq_any */
#ifdef WANT_sf_mh_seq_all
	TYPE_METHOD_HINTREF(Sequence_all),
#endif /* WANT_sf_mh_seq_all */
#ifdef WANT_sf_mh_seq_parity
	TYPE_METHOD_HINTREF(Sequence_parity),
#endif /* WANT_sf_mh_seq_parity */
#ifdef WANT_sf_mh_seq_min
	TYPE_METHOD_HINTREF(Sequence_min),
#endif /* WANT_sf_mh_seq_min */
#ifdef WANT_sf_mh_seq_max
	TYPE_METHOD_HINTREF(Sequence_max),
#endif /* WANT_sf_mh_seq_max */
#ifdef WANT_sf_mh_seq_count
	TYPE_METHOD_HINTREF(Sequence_count),
#endif /* WANT_sf_mh_seq_count */
#ifdef WANT_sf_mh_seq_contains
	TYPE_METHOD_HINTREF(Sequence_contains),
#endif /* WANT_sf_mh_seq_contains */
	//TODO:TYPE_METHOD_HINTREF(Sequence_locate),
	//TODO:TYPE_METHOD_HINTREF(Sequence_rlocate),
	//TODO:TYPE_METHOD_HINTREF(Sequence_startswith),
	//TODO:TYPE_METHOD_HINTREF(Sequence_endswith),
#ifdef WANT_sf_mh_seq_find
	TYPE_METHOD_HINTREF(Sequence_find),
#endif /* WANT_sf_mh_seq_find */
	//TODO:TYPE_METHOD_HINTREF(Sequence_rfind),
	TYPE_METHOD_HINTREF(Sequence_erase),
	TYPE_METHOD_HINTREF(Sequence_insert),
	TYPE_METHOD_HINTREF(Sequence_insertall),
	TYPE_METHOD_HINTREF(Sequence_pushfront),
	TYPE_METHOD_HINTREF(Sequence_append),
	TYPE_METHOD_HINTREF(Sequence_extend),
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

PRIVATE struct type_method_hint tpconst sf_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_enumerate_index, &sf_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &sf_mh_seq_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &sf_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_trygetfirst, &sf_mh_seq_trygetfirst, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_trygetlast, &sf_mh_seq_trygetlast, METHOD_FNOREFESCAPE),
#ifdef WANT_sf_mh_seq_any
	TYPE_METHOD_HINT_F(seq_any, &sf_mh_seq_any, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_key, &sf_mh_seq_any_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_range, &sf_mh_seq_any_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_any_with_range_and_key, &sf_mh_seq_any_with_range_and_key, METHOD_FNOREFESCAPE),
#endif /* WANT_sf_mh_seq_any */
#ifdef WANT_sf_mh_seq_all
	TYPE_METHOD_HINT_F(seq_all, &sf_mh_seq_all, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_key, &sf_mh_seq_all_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_range, &sf_mh_seq_all_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_all_with_range_and_key, &sf_mh_seq_all_with_range_and_key, METHOD_FNOREFESCAPE),
#endif /* WANT_sf_mh_seq_all */
#ifdef WANT_sf_mh_seq_parity
	TYPE_METHOD_HINT_F(seq_parity, &sf_mh_seq_parity, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_key, &sf_mh_seq_parity_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_range, &sf_mh_seq_parity_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_parity_with_range_and_key, &sf_mh_seq_parity_with_range_and_key, METHOD_FNOREFESCAPE),
#endif /* WANT_sf_mh_seq_parity */
#ifdef WANT_sf_mh_seq_min
	TYPE_METHOD_HINT_F(seq_min, &sf_mh_seq_min, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_min_with_key, &sf_mh_seq_min_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_min_with_range, &sf_mh_seq_min_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_min_with_range_and_key, &sf_mh_seq_min_with_range_and_key, METHOD_FNOREFESCAPE),
#endif /* WANT_sf_mh_seq_min */
#ifdef WANT_sf_mh_seq_max
	TYPE_METHOD_HINT_F(seq_max, &sf_mh_seq_max, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max_with_key, &sf_mh_seq_max_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max_with_range, &sf_mh_seq_max_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_max_with_range_and_key, &sf_mh_seq_max_with_range_and_key, METHOD_FNOREFESCAPE),
#endif /* WANT_sf_mh_seq_max */
#ifdef WANT_sf_mh_seq_count
	TYPE_METHOD_HINT_F(seq_count, &sf_mh_seq_count, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_key, &sf_mh_seq_count_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_range, &sf_mh_seq_count_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_count_with_range_and_key, &sf_mh_seq_count_with_range_and_key, METHOD_FNOREFESCAPE),
#endif /* WANT_sf_mh_seq_count */
#ifdef WANT_sf_mh_seq_contains
	TYPE_METHOD_HINT_F(seq_contains, &sf_mh_seq_contains, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_key, &sf_mh_seq_contains_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_range, &sf_mh_seq_contains_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_range_and_key, &sf_mh_seq_contains_with_range_and_key, METHOD_FNOREFESCAPE),
#endif /* WANT_sf_mh_seq_contains */
	//TODO:TYPE_METHOD_HINT_F(seq_locate, &sf_mh_seq_locate, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_locate_with_key, &sf_mh_seq_locate_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_locate_with_range, &sf_mh_seq_locate_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_locate_with_range_and_key, &sf_mh_seq_locate_with_range_and_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_rlocate, &sf_mh_seq_rlocate, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_rlocate_with_key, &sf_mh_seq_rlocate_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_rlocate_with_range, &sf_mh_seq_rlocate_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_rlocate_with_range_and_key, &sf_mh_seq_rlocate_with_range_and_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_startswith, &sf_mh_seq_startswith, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_startswith_with_key, &sf_mh_seq_startswith_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_startswith_with_range, &sf_mh_seq_startswith_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_startswith_with_range_and_key, &sf_mh_seq_startswith_with_range_and_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_endswith, &sf_mh_seq_endswith, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_endswith_with_key, &sf_mh_seq_endswith_with_key, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_endswith_with_range, &sf_mh_seq_endswith_with_range, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_endswith_with_range_and_key, &sf_mh_seq_endswith_with_range_and_key, METHOD_FNOREFESCAPE),
#ifdef WANT_sf_mh_seq_find
	TYPE_METHOD_HINT_F(seq_find, &sf_mh_seq_find, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find_with_key, &sf_mh_seq_find_with_key, METHOD_FNOREFESCAPE),
#endif /* WANT_sf_mh_seq_find */
	//TODO:TYPE_METHOD_HINT_F(seq_rfind, &sf_mh_seq_rfind, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_rfind_with_key, &sf_mh_seq_rfind_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_erase, &sf_mh_seq_erase, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_insert, &sf_mh_seq_insert, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_insertall, &sf_mh_seq_insertall, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_pushfront, &sf_mh_seq_pushfront, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_append, &sf_mh_seq_append, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_extend, &sf_mh_seq_extend, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_xchitem_index, &sf_mh_seq_xchitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_clear, &sf_mh_seq_clear, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_pop, &sf_mh_seq_pop, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_remove, &sf_mh_seq_remove, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_rremove, &sf_mh_seq_rremove, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_removeall, &sf_mh_seq_removeall, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_removeif, &sf_mh_seq_removeif, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_resize, &sf_mh_seq_resize, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_fill, &sf_mh_seq_fill, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_reverse, &sf_mh_seq_reverse, METHOD_FNOREFESCAPE),
	//TODO:TYPE_METHOD_HINT_F(seq_reversed, &sf_mh_seq_reversed, METHOD_FNOREFESCAPE),
	/* TODO: TYPE_METHOD_HINT_F(seq_sum, &sf_mh_seq_sum, METHOD_FNOREFESCAPE), */
	/* TODO: TYPE_METHOD_HINT_F(seq_sum_with_range, &sf_mh_seq_sum_with_range, METHOD_FNOREFESCAPE), */
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_member tpconst sf_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT_AB, offsetof(SeqFlat, sf_seq),
	                      "->?S?S?O\n"
	                      "The underlying sequence that is being flattened"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst sf_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqFlatIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_seq sf_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sf_iter,
	/* .tp_sizeob             = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains           = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem            = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem            = */ DEFIMPL(&default__delitem__with__delitem_index),
	/* .tp_setitem            = */ DEFIMPL(&default__setitem__with__setitem_index),
	/* .tp_getrange           = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange           = */ DEFIMPL(&default__seq_operator_delrange__with__seq_operator_delrange_index__and__seq_operator_delrange_index_n),
	/* .tp_setrange           = */ DEFIMPL(&default__seq_operator_setrange__with__seq_operator_setrange_index__and__seq_operator_setrange_index_n),
	/* .tp_foreach            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&sf_foreach,
	/* .tp_foreach_pair       = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&sf_foreach_pair,
	/* .tp_bounditem          = */ DEFIMPL(&default__bounditem__with__getitem),
	/* .tp_hasitem            = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&sf_size,
	/* .tp_size_fast          = */ NULL,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&sf_getitem_index,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&sf_delitem_index,
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&sf_setitem_index,
	/* .tp_bounditem_index    = */ DEFIMPL(&default__bounditem_index__with__getitem_index),
	/* .tp_hasitem_index      = */ DEFIMPL(&default__hasitem_index__with__bounditem_index),
	/* .tp_getrange_index     = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index     = */ DEFIMPL(&default__seq_operator_delrange_index__with__seq_operator_size__and__seq_erase),
	/* .tp_setrange_index     = */ DEFIMPL(&default__seq_operator_setrange_index__with__seq_operator_size__and__seq_erase__and__seq_insertall),
	/* .tp_getrange_index_n   = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index_n   = */ DEFIMPL(&default__seq_operator_delrange_index_n__with__seq_operator_size__and__seq_operator_delrange_index),
	/* .tp_setrange_index_n   = */ DEFIMPL(&default__seq_operator_setrange_index_n__with__seq_operator_size__and__seq_operator_setrange_index),
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqFlat,
			/* tp_ctor:        */ &sf_ctor,
			/* tp_copy_ctor:   */ &sf_copy,
			/* tp_any_ctor:    */ &sf_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &sf_serialize
		),
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__22D95991F3D69B20),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__3C4D336761465F8A),
	/* .tp_seq           = */ &sf_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
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
	SeqFlatIterator_LockAcquire(other);
	other_curriter = other->sfi_curriter;
	Dee_Incref(other_curriter);
	SeqFlatIterator_LockRelease(other);
	other_curriter = DeeObject_CopyInherited(other_curriter);
	if unlikely(!other_curriter)
		goto err;
	self->sfi_curriter = other_curriter;
	self->sfi_baseiter = DeeObject_Copy(other->sfi_baseiter);
	if unlikely(!self->sfi_baseiter)
		goto err_other_curriter;
	Dee_atomic_lock_init(&self->sfi_currlock);
	return 0;
err_other_curriter:
	ASSERT(self->sfi_curriter == other_curriter);
	Dee_Decref(other_curriter);
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) int DCALL
sfi_serialize(SeqFlatIterator *__restrict self,
              DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(SeqFlatIterator, field))
	DREF DeeObject *self__sfi_curriter;
	Dee_atomic_lock_init(&DeeSerial_Addr2Mem(writer, addr, SeqFlatIterator)->sfi_currlock);
	SeqFlatIterator_LockAcquire(self);
	self__sfi_curriter = self->sfi_curriter;
	Dee_Incref(self__sfi_curriter);
	SeqFlatIterator_LockRelease(self);
	if (DeeSerial_PutObjectInherited(writer, ADDROF(sfi_curriter), self__sfi_curriter))
		goto err;
	return DeeSerial_PutObject(writer, ADDROF(sfi_baseiter), self->sfi_baseiter);
err:
	return -1;
#undef ADDROF
}

PRIVATE NONNULL((1)) int DCALL
sfi_init(SeqFlatIterator *__restrict self,
         size_t argc, DeeObject *const *argv) {
	SeqFlat *flat;
	DeeArg_Unpack1(err, argc, argv, "_SeqFlatIterator", &flat);
	if (DeeObject_AssertTypeExact(flat, &SeqFlat_Type))
		goto err;
	return sfi_inititer_withseq(self, flat->sf_seq);
err:
	return -1;
}

PRIVATE NONNULL((1, 2)) void DCALL
sfi_visit(SeqFlatIterator *__restrict self,
          Dee_visit_t proc, void *arg) {
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
		DREF DeeObject *new_currseq;
		DREF DeeObject *new_curriter;
		SeqFlatIterator_LockAcquire(self);
		curriter = self->sfi_curriter;
		Dee_Incref(curriter);
		SeqFlatIterator_LockRelease(self);
		result = DeeObject_BoolInherited(curriter);
		if (result != 0)
			break; /* non-empty, or error */

		/* Current iterator has been exhausted -> load the next one */
		new_currseq = DeeObject_IterNext(self->sfi_baseiter);
		if (!ITER_ISOK(new_currseq))
			return new_currseq == ITER_DONE ? 0 : -1;  /* Error or ITER_DONE */
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
	return result;
err:
	return -1;
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
		part = DeeObject_InvokeMethodHint(iter_advance, curriter, step);
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
};

PRIVATE struct type_method tpconst sfi_methods[] = {
	TYPE_METHOD_HINTREF(Iterator_advance),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst sfi_method_hints[] = {
	TYPE_METHOD_HINT(iter_advance, &sfi_advance),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_member sfi_members[] = {
	TYPE_MEMBER_FIELD_DOC("__baseiter__", STRUCT_OBJECT_AB,
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

PRIVATE WUNUSED NONNULL((1)) DREF SeqFlat *DCALL
sfi_mh_iter_getseq(SeqFlatIterator *__restrict self) {
	DREF DeeObject *baseseq;
	baseseq = DeeObject_InvokeMethodHint(iter_getseq, self->sfi_baseiter);
	if unlikely(!baseseq)
		goto err_nest;
	return SeqFlat_NewInherited(baseseq);
err_nest:
	/* TODO: Nest current error within `err_iter_unsupportedf(self, "seq")' */
	return NULL;
}

PRIVATE struct type_getset sfi_getsets[] = {
	TYPE_GETTER_AB(STR_seq, &sfi_mh_iter_getseq, "->?Ert:SeqFlat"),
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ SeqFlatIterator,
			/* tp_ctor:        */ &sfi_ctor,
			/* tp_copy_ctor:   */ &sfi_copy,
			/* tp_any_ctor:    */ &sfi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &sfi_serialize
		),
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&sfi_visit,
	/* .tp_gc            = */ &sfi_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__439AAF00B07ABA02),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sfi_next,
	/* .tp_iterator      = */ &sfi_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ sfi_methods,
	/* .tp_getsets       = */ sfi_getsets,
	/* .tp_members       = */ sfi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ sfi_method_hints,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
SeqFlat_Of(DeeObject *__restrict self) {
	return Dee_AsObject(SeqFlat_New(self));
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_FLAT_C */
