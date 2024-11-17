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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_FILTER_C
#define GUARD_DEEMON_OBJECTS_SEQ_FILTER_C 1

#include "filter.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>

#include <hybrid/limitcore.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"

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
	self->f_seq = Dee_EmptySeq;
	self->f_fun = Dee_None;
	Dee_Incref(Dee_EmptySeq);
	Dee_Incref(Dee_None);
	return 0;
}

#define filter_copy  generic_proxy2_copy_alias12
#define filter_deep  generic_proxy2_deepcopy
#define filter_init  generic_proxy2_init
#define filter_fini  generic_proxy2_fini
#define filter_visit generic_proxy2_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
filteriterator_ctor(FilterIterator *__restrict self) {
	self->fi_iter = DeeObject_Iter(Dee_EmptySeq);
	if unlikely(!self->fi_iter)
		goto err;
	self->fi_func = Dee_None;
	Dee_Incref(Dee_None);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
filteriterator_init(FilterIterator *__restrict self,
                    size_t argc, DeeObject *const *argv) {
	Filter *filter;
	if (DeeArg_Unpack(argc, argv, "o:_SeqFilterIterator", &filter))
		goto err;
	if (DeeObject_AssertTypeExact(filter, &SeqFilter_Type))
		goto err;
	self->fi_iter = DeeObject_Iter(filter->f_seq);
	if unlikely(!self->fi_iter)
		goto err;
	self->fi_func = filter->f_fun;
	Dee_Incref(filter->f_fun);
	return 0;
err:
	return -1;
}

#define filteriterator_copy  generic_proxy2_copy_recursive1_alias2
#define filteriterator_deep  generic_proxy2_deepcopy
#define filteriterator_fini  generic_proxy2_fini
#define filteriterator_visit generic_proxy2_visit

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
		goto again;
	}
done:
	return result;
err_r:
	Dee_Decref(result);
	return NULL;
}

#define filteriterator_hash generic_proxy2_hash_recursive_ordered
STATIC_ASSERT(offsetof(FilterIterator, fi_iter) == offsetof(ProxyObject, po_obj));
#define filteriterator_trycompare_eq generic_proxy_trycompare_eq_recursive
#define filteriterator_compare_eq    generic_proxy_compare_eq_recursive
#define filteriterator_compare       generic_proxy_compare_recursive

PRIVATE struct type_cmp filteriterator_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&filteriterator_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *self, DeeObject *))&filteriterator_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *self, DeeObject *))&filteriterator_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *self, DeeObject *))&filteriterator_trycompare_eq,
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
				/* .tp_ctor      = */ (dfunptr_t)&filteriterator_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&filteriterator_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&filteriterator_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&filteriterator_init,
				TYPE_FIXED_ALLOCATOR(FilterIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&filteriterator_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&filteriterator_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &filteriterator_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filteriterator_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ filteriterator_getsets,
	/* .tp_members       = */ filteriterator_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED NONNULL((1)) DREF FilterIterator *DCALL
filter_iter(Filter *__restrict self) {
	DREF FilterIterator *result;
	result = DeeObject_MALLOC(FilterIterator);
	if unlikely(!result)
		goto done;
	result->fi_iter = DeeObject_Iter(self->f_seq);
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

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
filter_foreach(Filter *self, Dee_foreach_t proc, void *arg) {
	struct filter_foreach_data data;
	data.ffd_fun  = self->f_fun;
	data.ffd_proc = proc;
	data.ffd_arg  = arg;
	return DeeObject_Foreach(self->f_seq, &filter_foreach_cb, &data);
}

struct filter_as_unbound_enumerate_data {
	DeeObject      *faued_fun;  /* [1..1] Function used for filtering. */
	Dee_enumerate_t faued_proc; /* [1..1] Underlying callback. */
	void           *faued_arg;  /* [?..?] Cookie for `pfd_proc' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
filter_as_unbound_enumerate_cb(void *arg, DeeObject *index, DeeObject *value) {
	struct filter_as_unbound_enumerate_data *data;
	data = (struct filter_as_unbound_enumerate_data *)arg;
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
filter_as_unbound_enumerate(Filter *self, Dee_enumerate_t proc, void *arg) {
	struct filter_as_unbound_enumerate_data data;
	data.faued_fun  = self->f_fun;
	data.faued_proc = proc;
	data.faued_arg  = arg;
	return DeeObject_Enumerate(self->f_seq, &filter_as_unbound_enumerate_cb, &data);
}

struct filter_as_unbound_enumerate_index_data {
	DeeObject            *faueid_fun;  /* [1..1] Function used for filtering. */
	Dee_enumerate_index_t faueid_proc; /* [1..1] Underlying callback. */
	void                 *faueid_arg;  /* [?..?] Cookie for `pfd_proc' */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
filter_as_unbound_enumerate_index_cb(void *arg, size_t index, DeeObject *value) {
	struct filter_as_unbound_enumerate_index_data *data;
	data = (struct filter_as_unbound_enumerate_index_data *)arg;
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
filter_as_unbound_enumerate_index(Filter *self, Dee_enumerate_index_t proc,
                                  void *arg, size_t start, size_t end) {
	struct filter_as_unbound_enumerate_index_data data;
	data.faueid_fun  = self->f_fun;
	data.faueid_proc = proc;
	data.faueid_arg  = arg;
	return DeeObject_EnumerateIndex(self->f_seq, &filter_as_unbound_enumerate_index_cb, &data, start, end);
}

#define filter_enumerate_index_cb_MAGIC_EARLY_STOP \
	(__SSIZE_MIN__ + 99) /* Shhht. We don't talk about this one... */
struct filter_enumerate_index_data {
	DeeObject            *feid_fun;   /* [1..1] Function used for filtering. */
	Dee_enumerate_index_t feid_proc;  /* [1..1] Underlying callback. */
	void                 *feid_arg;   /* [?..?] Cookie for `pfd_proc' */
	size_t                feid_index; /* Next index */
	size_t                feid_start; /* Start index */
	size_t                feid_end;   /* End index */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
filter_enumerate_index_cb(void *arg, DeeObject *value) {
	int pred_bool;
	struct filter_enumerate_index_data *data;
	size_t index;
	data = (struct filter_enumerate_index_data *)arg;
	if unlikely(data->feid_index >= data->feid_end)
		return filter_enumerate_index_cb_MAGIC_EARLY_STOP;
	pred_bool = invoke_filter(data->feid_fun, value);
	if (pred_bool <= 0)
		return pred_bool; /* Error or ignored */
	index = data->feid_index++;
	if (index < data->feid_start)
		return 0;
	return (*data->feid_proc)(data->feid_arg, index, value);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
filter_enumerate_index(Filter *self, Dee_enumerate_index_t proc,
                       void *arg, size_t start, size_t end) {
	Dee_ssize_t result;
	struct filter_enumerate_index_data data;
	data.feid_fun   = self->f_fun;
	data.feid_proc  = proc;
	data.feid_arg   = arg;
	data.feid_start = start;
	data.feid_end   = end;
	result = DeeObject_Foreach(self->f_seq, &filter_enumerate_index_cb, &data);
	if unlikely(result == filter_enumerate_index_cb_MAGIC_EARLY_STOP)
		result = 0;
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
filter_size_cb(void *arg, DeeObject *value) {
	return invoke_filter((DeeObject *)arg, value);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL filter_size(Filter *self) {
	return (size_t)DeeObject_Foreach(self->f_seq, &filter_size_cb, self->f_fun);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
filter_bool_cb(void *arg, DeeObject *value) {
	int result = invoke_filter((DeeObject *)arg, value);
	if (result > 1)
		result = -2;
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
filter_bool(Filter *self) {
	Dee_ssize_t foreach_status;
	foreach_status = DeeObject_Foreach(self->f_seq, &filter_bool_cb, self->f_fun);
	if (foreach_status == -2)
		return 1;
	return (int)foreach_status;
}

STATIC_ASSERT(offsetof(Filter, f_seq) == offsetof(ProxyObject, po_obj));
#define filter_as_unbound_bool generic_proxy_bool

struct filter_getitem_index_data {
	DeeObject      *fgid_fun;    /* [1..1] Function used for filtering. */
	DREF DeeObject *fgid_result; /* [?..1] Index value, or undefined if not yet discovered. */
	size_t          fgid_skip;   /* Number of items still needing to be skipped. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
filter_getitem_index_cb(void *arg, DeeObject *value) {
	int pred_bool;
	struct filter_getitem_index_data *data;
	data = (struct filter_getitem_index_data *)arg;
	pred_bool = invoke_filter(data->fgid_fun, value);
	if (pred_bool <= 0)
		return pred_bool; /* Error or ignored */
	if (data->fgid_skip) {
		--data->fgid_skip;
		return 0;
	}
	Dee_Incref(value);
	data->fgid_result = value;
	return -2;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
filter_getitem_index(Filter *self, size_t index) {
	Dee_ssize_t foreach_status;
	struct filter_getitem_index_data data;
	data.fgid_fun  = self->f_fun;
	data.fgid_skip = index;
	foreach_status = DeeObject_Foreach(self->f_seq, &filter_getitem_index_cb, &data);
	if likely(foreach_status == -2)
		return data.fgid_result;
	if unlikely(foreach_status == -1)
		goto err;
	/* Index out-of-bounds */
	err_index_out_of_bounds((DeeObject *)self, index,
	                        index - data.fgid_skip);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
filter_bounditem_index(Filter *self, size_t index) {
	size_t size = filter_size(self);
	if unlikely(size == (size_t)-1)
		goto err;
	return index < size ? 1 : -2;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
filter_hasitem_index(Filter *self, size_t index) {
	size_t size = filter_size(self);
	if unlikely(size == (size_t)-1)
		goto err;
	return index < size ? 1 : 0;
err:
	return -1;
}


STATIC_ASSERT(offsetof(Filter, f_seq) == offsetof(ProxyObject, po_obj));
#define filter_as_unbound_sizeob    generic_proxy_sizeob
#define filter_as_unbound_size      generic_proxy_size
#define filter_as_unbound_size_fast generic_proxy_size_fast

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
filter_contains(Filter *self, DeeObject *elem) {
	int result = DeeObject_ContainsAsBool(self->f_seq, elem);
	if unlikely(result < 0)
		goto err;
	if (!result)
		goto nope;
	result = invoke_filter(self->f_fun, elem);
	if unlikely(result < 0)
		goto err;
	if (!result)
		goto nope;
	return_true;
nope:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
filter_as_unbound_getitem(Filter *self, DeeObject *index) {
	int temp;
	DREF DeeObject *result;
	result = DeeObject_GetItem(self->f_seq, index);
	if unlikely(!result)
		goto err;
	temp = invoke_filter(self->f_fun, result);
	if unlikely(temp < 0)
		goto err_r;
	if unlikely(!temp) {
		err_unbound_key((DeeObject *)self, index);
		goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
filter_as_unbound_getitem_index(Filter *self, size_t index) {
	int temp;
	DREF DeeObject *result;
	result = DeeObject_GetItemIndex(self->f_seq, index);
	if unlikely(!result)
		goto err;
	temp = invoke_filter(self->f_fun, result);
	if unlikely(temp < 0)
		goto err_r;
	if unlikely(!temp) {
		err_unbound_index((DeeObject *)self, index);
		goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
filter_as_unbound_getitem_string_hash(Filter *self, char const *key, Dee_hash_t hash) {
	int temp;
	DREF DeeObject *result;
	result = DeeObject_GetItemStringHash(self->f_seq, key, hash);
	if unlikely(!result)
		goto err;
	temp = invoke_filter(self->f_fun, result);
	if unlikely(temp < 0)
		goto err_r;
	if unlikely(!temp) {
		err_unbound_key_str((DeeObject *)self, key);
		goto err_r;
	}
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
filter_as_unbound_getitem_string_len_hash(Filter *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int temp;
	DREF DeeObject *result;
	result = DeeObject_GetItemStringLenHash(self->f_seq, key, keylen, hash);
	if unlikely(!result)
		goto err;
	temp = invoke_filter(self->f_fun, result);
	if unlikely(temp < 0)
		goto err_r;
	if unlikely(!temp) {
		err_unbound_key_str_len((DeeObject *)self, key, keylen);
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
			return 0;
		if (DeeError_Catch(&DeeError_IndexError))
			return -2;
		goto err;
	}
	temp = invoke_filter(self->f_fun, itemvalue);
	Dee_Decref(itemvalue);
	if unlikely(temp < 0)
		goto err;
	if unlikely(!temp)
		return 0;
	return 1;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
filter_as_unbound_bounditem(Filter *self, DeeObject *index) {
	DREF DeeObject *result;
	result = DeeObject_GetItem(self->f_seq, index);
	return filter_bounditem_handle_itemvalue(self, result);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
filter_as_unbound_bounditem_index(Filter *self, size_t index) {
	DREF DeeObject *result;
	result = DeeObject_GetItemIndex(self->f_seq, index);
	return filter_bounditem_handle_itemvalue(self, result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
filter_as_unbound_bounditem_string_hash(Filter *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_GetItemStringHash(self->f_seq, key, hash);
	return filter_bounditem_handle_itemvalue(self, result);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
filter_as_unbound_bounditem_string_len_hash(Filter *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_GetItemStringLenHash(self->f_seq, key, keylen, hash);
	return filter_bounditem_handle_itemvalue(self, result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
filter_as_unbound_trygetitem(Filter *self, DeeObject *index) {
	int temp;
	DREF DeeObject *result;
	result = DeeObject_TryGetItem(self->f_seq, index);
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
filter_as_unbound_trygetitem_index(Filter *self, size_t index) {
	int temp;
	DREF DeeObject *result;
	result = DeeObject_TryGetItemIndex(self->f_seq, index);
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
filter_as_unbound_trygetitem_string_hash(Filter *self, char const *key, Dee_hash_t hash) {
	int temp;
	DREF DeeObject *result;
	result = DeeObject_TryGetItemStringHash(self->f_seq, key, hash);
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
filter_as_unbound_trygetitem_string_len_hash(Filter *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int temp;
	DREF DeeObject *result;
	result = DeeObject_TryGetItemStringLenHash(self->f_seq, key, keylen, hash);
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
filter_as_unbound_getrange(Filter *self, DeeObject *start, DeeObject *end) {
	DREF Filter *result;
	DREF DeeObject *base;
	base = DeeObject_GetRange(self->f_seq, start, end);
	if unlikely(!base)
		goto err;
	result = (DREF Filter *)DeeSeq_Filter(base, self->f_fun);
	Dee_Decref(base);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Filter *DCALL
filter_as_unbound_getrange_index(Filter *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF Filter *result;
	DREF DeeObject *base;
	base = DeeObject_GetRangeIndex(self->f_seq, start, end);
	if unlikely(!base)
		goto err;
	result = (DREF Filter *)DeeSeq_Filter(base, self->f_fun);
	Dee_Decref(base);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Filter *DCALL
filter_as_unbound_getrange_index_n(Filter *self, Dee_ssize_t start) {
	DREF Filter *result;
	DREF DeeObject *base;
	base = DeeObject_GetRangeIndexN(self->f_seq, start);
	if unlikely(!base)
		goto err;
	result = (DREF Filter *)DeeSeq_Filter(base, self->f_fun);
	Dee_Decref(base);
	return result;
err:
	return NULL;
}

STATIC_ASSERT(offsetof(Filter, f_seq) == offsetof(ProxyObject, po_obj));
#define filter_as_unbound_hasitem                 generic_proxy_hasitem
#define filter_as_unbound_hasitem_index           generic_proxy_hasitem_index
#define filter_as_unbound_hasitem_string_hash     generic_proxy_hasitem_string_hash
#define filter_as_unbound_hasitem_string_len_hash generic_proxy_hasitem_string_len_hash

PRIVATE struct type_seq filter_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filter_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&filter_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&filter_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_index_t, void *, size_t, size_t))&filter_enumerate_index,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&filter_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&filter_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&filter_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&filter_hasitem_index,
};

PRIVATE struct type_seq filter_as_unbound_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filter_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filter_as_unbound_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&filter_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&filter_as_unbound_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&filter_as_unbound_getrange,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&filter_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_t, void *))&filter_as_unbound_enumerate,
	/* .tp_enumerate_index            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_index_t, void *, size_t, size_t))&filter_as_unbound_enumerate_index,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&filter_as_unbound_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&filter_as_unbound_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&filter_as_unbound_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&filter_as_unbound_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&filter_as_unbound_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&filter_as_unbound_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&filter_as_unbound_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&filter_as_unbound_getrange_index,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&filter_as_unbound_getrange_index_n,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&filter_as_unbound_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&filter_as_unbound_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&filter_as_unbound_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&filter_as_unbound_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&filter_as_unbound_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&filter_as_unbound_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&filter_as_unbound_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&filter_as_unbound_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&filter_as_unbound_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&filter_as_unbound_hasitem_string_len_hash,
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
				/* .tp_ctor      = */ (dfunptr_t)&filter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&filter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&filter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&filter_init,
				TYPE_FIXED_ALLOCATOR(Filter)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&filter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&filter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&filter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &filter_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ filter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ filter_class_members
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
				/* .tp_ctor      = */ (dfunptr_t)&filter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&filter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&filter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&filter_init,
				TYPE_FIXED_ALLOCATOR(Filter)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&filter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&filter_as_unbound_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&filter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &filter_as_unbound_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ filter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ filter_class_members
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
