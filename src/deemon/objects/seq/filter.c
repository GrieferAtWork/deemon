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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_FILTER_C
#define GUARD_DEEMON_OBJECTS_SEQ_FILTER_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/thread.h>

#include <hybrid/limitcore.h>
/**/

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "filter.h"
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

LOCAL WUNUSED NONNULL((1, 2)) int DCALL
invoke_filter(DeeObject *filter, DeeObject *elem) {
	DREF DeeObject *filter_result;
	filter_result = DeeObject_Call(filter, 1, &elem);
	if unlikely(!filter_result)
		goto err;
	return DeeObject_BoolInherited(filter_result);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
filter_ctor(Filter *__restrict self) {
	self->f_seq = DeeSeq_NewEmpty();
	self->f_fun = DeeNone_NewRef();
	return 0;
}

#define filter_copy  generic_proxy2__copy_alias12
#define filter_deep  generic_proxy2__deepcopy
#define filter_init  generic_proxy2__init
#define filter_fini  generic_proxy2__fini
#define filter_visit generic_proxy2__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
filteriterator_ctor(FilterIterator *__restrict self) {
	self->fi_iter = DeeObject_Iter(Dee_EmptySeq);
	if unlikely(!self->fi_iter)
		goto err;
	self->fi_func = DeeNone_NewRef();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
filteriterator_init(FilterIterator *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	Filter *filter;
	DeeArg_Unpack1(err, argc, argv, "_SeqFilterIterator", &filter);
	if (DeeObject_AssertTypeExact(filter, &SeqFilter_Type))
		goto err;
	self->fi_iter = DeeObject_InvokeMethodHint(seq_operator_iter, filter->f_seq);
	if unlikely(!self->fi_iter)
		goto err;
	self->fi_func = filter->f_fun;
	Dee_Incref(filter->f_fun);
	return 0;
err:
	return -1;
}

#define filteriterator_copy  generic_proxy2__copy_recursive1_alias2
#define filteriterator_deep  generic_proxy2__deepcopy
#define filteriterator_fini  generic_proxy2__fini
#define filteriterator_visit generic_proxy2__visit

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
filteriterator_next(FilterIterator *__restrict self) {
	DREF DeeObject *result;
	int pred_bool;
again:
	result = DeeObject_IterNext(self->fi_iter);
	if unlikely(!ITER_ISOK(result))
		goto done;
	pred_bool = invoke_filter(self->fi_func, result);
	if unlikely(pred_bool < 0)
		goto err_r;
	if (!pred_bool) {
		Dee_Decref(result);
		if (DeeThread_CheckInterrupt())
			goto err;
		goto again;
	}
done:
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

#define filteriterator_hash generic_proxy2__hash_recursive_ordered
STATIC_ASSERT(offsetof(FilterIterator, fi_iter) == offsetof(ProxyObject, po_obj));
#define filteriterator_trycompare_eq generic_proxy__trycompare_eq_recursive
#define filteriterator_compare_eq    generic_proxy__compare_eq_recursive
#define filteriterator_compare       generic_proxy__compare_recursive

PRIVATE struct type_cmp filteriterator_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&filteriterator_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *self, DeeObject *))&filteriterator_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *self, DeeObject *))&filteriterator_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *self, DeeObject *))&filteriterator_trycompare_eq,
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


PRIVATE WUNUSED NONNULL((1)) DREF Filter *DCALL
filteriterator_seq_get(FilterIterator *__restrict self) {
	DREF Filter *result;
	DREF DeeObject *base_seq;
	base_seq = DeeObject_GetAttr(self->fi_iter, (DeeObject *)&str_seq);
	if unlikely(!base_seq)
		goto err;
	result = DeeObject_MALLOC(Filter);
	if unlikely(!result)
		goto err_base_seq;
	result->f_seq = base_seq; /* Inherit reference. */
	result->f_fun = self->fi_func;
	Dee_Incref(result->f_fun);
	DeeObject_Init(result, &SeqFilter_Type);
	return result;
err_base_seq:
	Dee_Decref(base_seq);
err:
	return NULL;
}


PRIVATE struct type_getset tpconst filteriterator_getsets[] = {
	TYPE_GETTER_F(STR_seq, &filteriterator_seq_get, METHOD_FNOREFESCAPE, "->?Ert:SeqFilter"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst filteriterator_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(FilterIterator, fi_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD_DOC("__filter__", STRUCT_OBJECT, offsetof(FilterIterator, fi_func), "->?DCallable"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqFilterIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqFilterIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqFilter)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&filteriterator_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&filteriterator_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&filteriterator_deep,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&filteriterator_init,
				TYPE_FIXED_ALLOCATOR(FilterIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&filteriterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&filteriterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &filteriterator_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filteriterator_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ filteriterator_getsets,
	/* .tp_members       = */ filteriterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


#define filterub_iter filter_iter
PRIVATE WUNUSED NONNULL((1)) DREF FilterIterator *DCALL
filter_iter(Filter *__restrict self) {
	DREF FilterIterator *result;
	result = DeeObject_MALLOC(FilterIterator);
	if unlikely(!result)
		goto done;
	result->fi_iter = DeeObject_InvokeMethodHint(seq_operator_iter, self->f_seq);
	if unlikely(!result->fi_iter)
		goto err_r;
	result->fi_func = self->f_fun;
	Dee_Incref(result->fi_func);
	DeeObject_Init(result, &SeqFilterIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

struct filter_foreach_data {
	DeeObject    *ffd_fun;  /* [1..1] Function used for filtering. */
	Dee_foreach_t ffd_proc; /* [1..1] Underlying callback. */
	void         *ffd_arg;  /* [?..?] Cookie for `pfd_proc' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
filter_foreach_cb(void *arg, DeeObject *elem) {
	int pred_bool;
	struct filter_foreach_data *data;
	data      = (struct filter_foreach_data *)arg;
	pred_bool = invoke_filter(data->ffd_fun, elem);
	if unlikely(pred_bool < 0)
		goto err;
	if (!pred_bool)
		return 0; /* Don't enumerate this one. */
	return (*data->ffd_proc)(data->ffd_arg, elem);
err:
	return -1;
}

#define filterub_foreach filter_foreach
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
filter_foreach(Filter *self, Dee_foreach_t proc, void *arg) {
	struct filter_foreach_data data;
	data.ffd_fun  = self->f_fun;
	data.ffd_proc = proc;
	data.ffd_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_operator_foreach, self->f_seq, &filter_foreach_cb, &data);
}

struct filterub_mh_seq_enumerate_data {
	DeeObject      *faued_fun;  /* [1..1] Function used for filtering. */
	Dee_seq_enumerate_t faued_proc; /* [1..1] Underlying callback. */
	void           *faued_arg;  /* [?..?] Cookie for `pfd_proc' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
filterub_mh_seq_enumerate_cb(void *arg, DeeObject *index, DeeObject *value) {
	struct filterub_mh_seq_enumerate_data *data;
	data = (struct filterub_mh_seq_enumerate_data *)arg;
	if (value) {
		int pred_bool = invoke_filter(data->faued_fun, value);
		if unlikely(pred_bool < 0)
			goto err;
		if (!pred_bool)
			value = NULL; /* Treat as unbound */
	}
	return (*data->faued_proc)(data->faued_arg, index, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
filterub_mh_seq_enumerate(Filter *self, Dee_seq_enumerate_t proc, void *arg) {
	struct filterub_mh_seq_enumerate_data data;
	data.faued_fun  = self->f_fun;
	data.faued_proc = proc;
	data.faued_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_enumerate, self->f_seq, &filterub_mh_seq_enumerate_cb, &data);
}

struct filterub_mh_seq_enumerate_index_data {
	DeeObject                *faueid_fun;  /* [1..1] Function used for filtering. */
	Dee_seq_enumerate_index_t faueid_proc; /* [1..1] Underlying callback. */
	void                     *faueid_arg;  /* [?..?] Cookie for `pfd_proc' */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
filterub_mh_seq_enumerate_index_cb(void *arg, size_t index, DeeObject *value) {
	struct filterub_mh_seq_enumerate_index_data *data;
	data = (struct filterub_mh_seq_enumerate_index_data *)arg;
	if (value) {
		int pred_bool = invoke_filter(data->faueid_fun, value);
		if unlikely(pred_bool < 0)
			goto err;
		if (!pred_bool)
			value = NULL; /* Treat as unbound */
	}
	return (*data->faueid_proc)(data->faueid_arg, index, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
filterub_mh_seq_enumerate_index(Filter *self, Dee_seq_enumerate_index_t proc,
                                void *arg, size_t start, size_t end) {
	struct filterub_mh_seq_enumerate_index_data data;
	data.faueid_fun  = self->f_fun;
	data.faueid_proc = proc;
	data.faueid_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_enumerate_index, self->f_seq, &filterub_mh_seq_enumerate_index_cb, &data, start, end);
}

#define filterub_contains filter_contains
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
filter_contains(Filter *self, DeeObject *elem) {
	int result = DeeObject_InvokeMethodHint(seq_contains, self->f_seq, elem);
	if (result <= 0) {
		if unlikely(result < 0)
			goto err;
		goto nope;
	}
	result = invoke_filter(self->f_fun, elem);
	if (result <= 0) {
		if unlikely(result < 0)
			goto err;
		goto nope;
	}
	return_true;
nope:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
filterub_getitem(Filter *self, DeeObject *index) {
	int temp;
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(seq_operator_getitem, self->f_seq, index);
	if unlikely(!result)
		goto err;
	temp = invoke_filter(self->f_fun, result);
	if unlikely(temp < 0)
		goto err_r;
	if unlikely(!temp) {
		DeeRT_ErrUnboundKey((DeeObject *)self, index);
		goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
filterub_getitem_index(Filter *self, size_t index) {
	int temp;
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(seq_operator_getitem_index, self->f_seq, index);
	if unlikely(!result)
		goto err;
	temp = invoke_filter(self->f_fun, result);
	if unlikely(temp < 0)
		goto err_r;
	if unlikely(!temp) {
		DeeRT_ErrUnboundIndex((DeeObject *)self, index);
		goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
filter_bounditem_handle_itemvalue(Filter *self, /*inherit(always)*/ DREF DeeObject *itemvalue) {
	int temp;
	if unlikely(!itemvalue) {
		if (DeeError_Catch(&DeeError_UnboundItem))
			return Dee_BOUND_NO;
		if (DeeError_Catch(&DeeError_IndexError))
			return Dee_BOUND_MISSING;
		goto err;
	}
	temp = invoke_filter(self->f_fun, itemvalue);
	Dee_Decref(itemvalue);
	if unlikely(temp < 0)
		goto err;
	if unlikely(!temp)
		return Dee_BOUND_NO;
	return Dee_BOUND_YES;
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
filterub_bounditem(Filter *self, DeeObject *index) {
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(seq_operator_getitem, self->f_seq, index);
	return filter_bounditem_handle_itemvalue(self, result);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
filterub_bounditem_index(Filter *self, size_t index) {
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(seq_operator_getitem_index, self->f_seq, index);
	return filter_bounditem_handle_itemvalue(self, result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
filterub_trygetitem(Filter *self, DeeObject *index) {
	int temp;
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(seq_operator_trygetitem, self->f_seq, index);
	if unlikely(!ITER_ISOK(result))
		return result;
	temp = invoke_filter(self->f_fun, result);
	if unlikely(temp < 0)
		goto err_r;
	if unlikely(!temp) {
		Dee_Decref(result);
		return ITER_DONE;
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
filterub_trygetitem_index(Filter *self, size_t index) {
	int temp;
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(seq_operator_trygetitem_index, self->f_seq, index);
	if unlikely(!ITER_ISOK(result))
		return result;
	temp = invoke_filter(self->f_fun, result);
	if unlikely(temp < 0)
		goto err_r;
	if unlikely(!temp) {
		Dee_Decref(result);
		return ITER_DONE;
	}
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF Filter *DCALL
filterub_getrange(Filter *self, DeeObject *start, DeeObject *end) {
	DREF Filter *result;
	DREF DeeObject *base;
	base = DeeObject_InvokeMethodHint(seq_operator_getrange, self->f_seq, start, end);
	if unlikely(!base)
		goto err;
	result = (DREF Filter *)DeeSeq_Filter(base, self->f_fun);
	Dee_Decref(base);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Filter *DCALL
filterub_getrange_index(Filter *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF Filter *result;
	DREF DeeObject *base;
	base = DeeObject_InvokeMethodHint(seq_operator_getrange_index, self->f_seq, start, end);
	if unlikely(!base)
		goto err;
	result = (DREF Filter *)DeeSeq_Filter(base, self->f_fun);
	Dee_Decref(base);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Filter *DCALL
filterub_getrange_index_n(Filter *self, Dee_ssize_t start) {
	DREF Filter *result;
	DREF DeeObject *base;
	base = DeeObject_InvokeMethodHint(seq_operator_getrange_index_n, self->f_seq, start);
	if unlikely(!base)
		goto err;
	result = (DREF Filter *)DeeSeq_Filter(base, self->f_fun);
	Dee_Decref(base);
	return result;
err:
	return NULL;
}

STATIC_ASSERT(offsetof(Filter, f_seq) == offsetof(ProxyObject, po_obj));
#define filterub_bool             generic_proxy__seq_operator_bool
#define filterub_sizeob           generic_proxy__seq_operator_sizeob
#define filterub_size             generic_proxy__seq_operator_size
#define filterub_size_fast        generic_proxy__size_fast
#define filterub_hasitem          generic_proxy__seq_operator_hasitem
#define filterub_hasitem_index    generic_proxy__seq_operator_hasitem_index
#define filterub_delitem          generic_proxy__seq_operator_delitem
#define filterub_delrange         generic_proxy__seq_operator_delrange
#define filterub_delitem_index    generic_proxy__seq_operator_delitem_index
#define filterub_delrange_index   generic_proxy__seq_operator_delrange_index
#define filterub_delrange_index_n generic_proxy__seq_operator_delrange_index_n

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
filter_verify_insert(void *arg, DeeObject *item) {
	Filter *self = (Filter *)arg;
	int result = invoke_filter(self->f_fun, item);
	if likely(result > 0)
		return 0;
	if (result == 0) {
		result = DeeError_Throwf(&DeeError_ValueError,
		                         "Cannot insert %k into filtered sequence: "
		                         /**/ "item is black-listed by filter");
	}
	ASSERT(result == -1);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
filter_verify_insert_all(Filter *self, DeeObject *items) {
	return DeeObject_InvokeMethodHint(seq_operator_foreach, items, &filter_verify_insert, self);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
filterub_setitem(Filter *self, DeeObject *key_or_index, DeeObject *value) {
	int result = (int)filter_verify_insert(self, value);
	if unlikely(result)
		return result;
	return generic_proxy__seq_operator_setitem((ProxyObject *)self, key_or_index, value);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
filterub_setitem_index(Filter *self, size_t index, DeeObject *value) {
	int result = (int)filter_verify_insert(self, value);
	if unlikely(result)
		return result;
	return generic_proxy__seq_operator_setitem_index((ProxyObject *)self, index, value);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
filterub_setrange(Filter *self, DeeObject *start, DeeObject *end, DeeObject *values) {
	int result = (int)filter_verify_insert_all(self, values);
	if unlikely(result)
		return result;
	return generic_proxy__seq_operator_setrange((ProxyObject *)self, start, end, values);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
filterub_setrange_index(Filter *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
	int result = (int)filter_verify_insert_all(self, values);
	if unlikely(result)
		return result;
	return generic_proxy__seq_operator_setrange_index((ProxyObject *)self, start, end, values);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
filterub_setrange_index_n(Filter *self, Dee_ssize_t start, DeeObject *values) {
	int result = (int)filter_verify_insert_all(self, values);
	if unlikely(result)
		return result;
	return generic_proxy__seq_operator_setrange_index_n((ProxyObject *)self, start, values);
}


PRIVATE struct type_seq filter_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filter_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&filter_contains,
	/* .tp_getitem                    = */ DEFIMPL(&default__getitem__with__getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&filter_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL(&default__bounditem__with__bounditem_index),
	/* .tp_hasitem                    = */ DEFIMPL(&default__hasitem__with__hasitem_index),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_foreach),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__trygetitem_index),
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

PRIVATE struct type_seq filterub_seq = {
	/* .tp_iter               = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filterub_iter,
	/* .tp_sizeob             = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filterub_sizeob,
	/* .tp_contains           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&filterub_contains,
	/* .tp_getitem            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&filterub_getitem,
	/* .tp_delitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&filterub_delitem,
	/* .tp_setitem            = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&filterub_setitem,
	/* .tp_getrange           = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&filterub_getrange,
	/* .tp_delrange           = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&filterub_delrange,
	/* .tp_setrange           = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&filterub_setrange,
	/* .tp_foreach            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&filterub_foreach,
	/* .tp_foreach_pair       = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem          = */ (int (DCALL *)(DeeObject *, DeeObject *))&filterub_bounditem,
	/* .tp_hasitem            = */ (int (DCALL *)(DeeObject *, DeeObject *))&filterub_hasitem,
	/* .tp_size               = */ (size_t (DCALL *)(DeeObject *__restrict))&filterub_size,
	/* .tp_size_fast          = */ (size_t (DCALL *)(DeeObject *__restrict))&filterub_size_fast,
	/* .tp_getitem_index      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&filterub_getitem_index,
	/* .tp_getitem_index_fast = */ NULL,
	/* .tp_delitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&filterub_delitem_index,
	/* .tp_setitem_index      = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&filterub_setitem_index,
	/* .tp_bounditem_index    = */ (int (DCALL *)(DeeObject *, size_t))&filterub_bounditem_index,
	/* .tp_hasitem_index      = */ (int (DCALL *)(DeeObject *, size_t))&filterub_hasitem_index,
	/* .tp_getrange_index     = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&filterub_getrange_index,
	/* .tp_delrange_index     = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&filterub_delrange_index,
	/* .tp_setrange_index     = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&filterub_setrange_index,
	/* .tp_getrange_index_n   = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&filterub_getrange_index_n,
	/* .tp_delrange_index_n   = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&filterub_delrange_index_n,
	/* .tp_setrange_index_n   = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&filterub_setrange_index_n,
	/* .tp_trygetitem         = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&filterub_trygetitem,
	/* .tp_trygetitem_index   = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&filterub_trygetitem_index,
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

PRIVATE struct type_member tpconst filter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(Filter, f_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__filter__", STRUCT_OBJECT, offsetof(Filter, f_fun), "->?DCallable"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst filter_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqFilterIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE DeeObject filter_locate_dummy = { OBJECT_HEAD_INIT(&DeeObject_Type) };

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
filter_trygetfirst(Filter *__restrict self) {
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(seq_locate, self->f_seq, self->f_fun, &filter_locate_dummy);
	if (result == &filter_locate_dummy) {
		Dee_DecrefNokill(&filter_locate_dummy);
		result = ITER_DONE;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
filter_trygetlast(Filter *__restrict self) {
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(seq_rlocate_with_range, self->f_seq, self->f_fun, 0, (size_t)-1, &filter_locate_dummy);
	if (result == &filter_locate_dummy) {
		Dee_DecrefNokill(&filter_locate_dummy);
		result = ITER_DONE;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
filter_getfirst(Filter *__restrict self) {
	DREF DeeObject *result = filter_trygetfirst(self);
	if unlikely(result == ITER_DONE)
		result = DeeRT_ErrUnboundAttr(self, &str_first);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
filter_getlast(Filter *__restrict self) {
	DREF DeeObject *result = filter_trygetlast(self);
	if unlikely(result == ITER_DONE)
		result = DeeRT_ErrUnboundAttr(self, &str_last);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
filter_bool(Filter *self) {
	DREF DeeObject *result = filter_trygetfirst(self);
	if (result == ITER_DONE)
		return 0;
	if unlikely(result == NULL)
		return -1;
	Dee_Decref(result);
	return 1;
}

#define filter_boundlast  filter_nonempty_asbound
#define filter_boundfirst filter_nonempty_asbound
PRIVATE WUNUSED NONNULL((1)) int DCALL
filter_nonempty_asbound(Filter *__restrict self) {
	int ok = filter_bool(self);
	if unlikely(ok < 0)
		goto err;
	return Dee_BOUND_FROMBOOL(ok);
err:
	return Dee_BOUND_ERR;
}


PRIVATE struct type_getset tpconst filter_getsets[] = {
	TYPE_GETTER_BOUND_NODOC(STR_first, &filter_getfirst, &filter_boundfirst),
	TYPE_GETTER_BOUND_NODOC(STR_last, &filter_getlast, &filter_boundlast),
	TYPE_GETSET_END
};

PRIVATE struct type_method tpconst filterub_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst filter_method_hints[] = {
	TYPE_METHOD_HINT(seq_trygetfirst, &filter_trygetfirst),
	TYPE_METHOD_HINT(seq_trygetlast, &filter_trygetlast),
	/* TODO: any() -> Sequence.any(__seq__, e -> __filter__(e) && e) */
	/* TODO: all() -> Sequence.all(__seq__, e -> !__filter__(e) || e) */
	/* TODO: parity() -> Sequence.parity(__seq__, e -> __filter__(e) && e) */
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_method_hint tpconst filterub_method_hints[] = {
	TYPE_METHOD_HINT(seq_enumerate, &filterub_mh_seq_enumerate),
	TYPE_METHOD_HINT(seq_enumerate_index, &filterub_mh_seq_enumerate_index),
	TYPE_METHOD_HINT_END
};

INTERN DeeTypeObject SeqFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqFilter",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,fun:?DCallable)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&filter_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&filter_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&filter_deep,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&filter_init,
				TYPE_FIXED_ALLOCATOR(Filter)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&filter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&filter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&filter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__26B2EC529683DE3C),
	/* .tp_seq           = */ &filter_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ filter_getsets,
	/* .tp_members       = */ filter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ filter_class_members,
	/* .tp_method_hints  = */ filter_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject SeqFilterAsUnbound_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqFilterAsUnbound",
	/* .tp_doc      = */ DOC("()\n"
	                         "(seq:?DSequence,fun:?DCallable)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)&filter_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&filter_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)&filter_deep,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&filter_init,
				TYPE_FIXED_ALLOCATOR(Filter)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&filter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&filterub_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&filter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__22D95991F3D69B20),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__247219960F1E745D),
	/* .tp_seq           = */ &filterub_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ filterub_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ filter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ filter_class_members,
	/* .tp_method_hints  = */ filterub_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_Filter(DeeObject *self, DeeObject *pred_keep) {
	DREF Filter *result;
	result = DeeObject_MALLOC(Filter);
	if unlikely(!result)
		goto done;
	Dee_Incref(self);
	Dee_Incref(pred_keep);
	result->f_seq = self;
	result->f_fun = pred_keep;
	DeeObject_Init(result, &SeqFilter_Type);
done:
	return (DREF DeeObject *)result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_FilterAsUnbound(DeeObject *self, DeeObject *pred_keep) {
	DREF Filter *result;
	result = DeeObject_MALLOC(Filter);
	if unlikely(!result)
		goto done;
	Dee_Incref(self);
	Dee_Incref(pred_keep);
	result->f_seq = self;
	result->f_fun = pred_keep;
	DeeObject_Init(result, &SeqFilterAsUnbound_Type);
done:
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_FILTER_C */
