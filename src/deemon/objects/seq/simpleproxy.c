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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_C
#define GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/int.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/seq.h>

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "simpleproxy.h"

#include <stddef.h> /* size_t, offsetof */

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_ctor(SeqSimpleProxy *__restrict self) {
	self->sp_seq = DeeSeq_NewEmpty();
	return 0;
}

STATIC_ASSERT(offsetof(SeqSimpleProxy, sp_seq) == offsetof(ProxyObject, po_obj));
#define proxy_copy            generic_proxy__copy_alias
#define proxy_deep            generic_proxy__deepcopy
#define proxy_init            generic_proxy__init
#define proxy_serialize       generic_proxy__serialize
#define proxy_fini            generic_proxy__fini
#define proxy_visit           generic_proxy__visit
#define proxy_size_fast       generic_proxy__size_fast
#define proxy_bool            generic_proxy__seq_operator_bool
#define proxy_sizeob          generic_proxy__seq_operator_sizeob
#define proxy_size            generic_proxy__seq_operator_size
#define proxy_hasitem         generic_proxy__seq_operator_hasitem
#define proxy_bounditem       generic_proxy__seq_operator_bounditem
#define proxy_hasitem_index   generic_proxy__seq_operator_hasitem_index
#define proxy_bounditem_index generic_proxy__seq_operator_bounditem_index

PRIVATE WUNUSED NONNULL((1)) DREF SeqSimpleProxy *DCALL
proxy_get_frozen(SeqSimpleProxy *__restrict self) {
	DREF DeeObject *inner_frozen;
	DREF SeqSimpleProxy *result;
	inner_frozen = DeeObject_GetAttr(self->sp_seq, Dee_AsObject(&str_frozen));
	if unlikely(!inner_frozen)
		goto err;
	if (inner_frozen == self->sp_seq) {
		Dee_DecrefNokill(inner_frozen);
		return_reference_(self);
	}
	result = DeeObject_MALLOC(SeqSimpleProxy);
	if unlikely(!result)
		goto err_inner;
	result->sp_seq = inner_frozen; /* Inherit reference */
	DeeObject_Init(result, Dee_TYPE(self));
	return result;
err_inner:
	Dee_Decref(inner_frozen);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst proxy_getsets[] = {
	TYPE_GETTER(STR_frozen, &proxy_get_frozen, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst proxy_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(SeqSimpleProxy, sp_seq), "->?DSequence"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF SeqSimpleProxyIterator *DCALL
ids_iter(SeqSimpleProxy *__restrict self) {
	DREF SeqSimpleProxyIterator *result;
	result = DeeObject_MALLOC(SeqSimpleProxyIterator);
	if unlikely(!result)
		goto done;
	result->si_iter = DeeObject_InvokeMethodHint(seq_operator_iter, self->sp_seq);
	if unlikely(!result->si_iter)
		goto err_r;
	DeeObject_Init(result, &SeqIdsIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqSimpleProxyIterator *DCALL
types_iter(SeqSimpleProxy *__restrict self) {
	DREF SeqSimpleProxyIterator *result;
	result = DeeObject_MALLOC(SeqSimpleProxyIterator);
	if unlikely(!result)
		goto done;
	result->si_iter = DeeObject_InvokeMethodHint(seq_operator_iter, self->sp_seq);
	if unlikely(!result->si_iter)
		goto err_r;
	DeeObject_Init(result, &SeqTypesIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF SeqSimpleProxyIterator *DCALL
classes_iter(SeqSimpleProxy *__restrict self) {
	DREF SeqSimpleProxyIterator *result;
	result = DeeObject_MALLOC(SeqSimpleProxyIterator);
	if unlikely(!result)
		goto done;
	result->si_iter = DeeObject_InvokeMethodHint(seq_operator_iter, self->sp_seq);
	if unlikely(!result->si_iter)
		goto err_r;
	DeeObject_Init(result, &SeqClassesIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ids_getitem(SeqSimpleProxy *__restrict self, DeeObject *index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeObject_InvokeMethodHint(seq_operator_getitem, self->sp_seq, index);
	if likely(elem) {
		result = DeeInt_NewUIntptr(DeeObject_Id(elem));
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
types_getitem(SeqSimpleProxy *__restrict self, DeeObject *index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeObject_InvokeMethodHint(seq_operator_getitem, self->sp_seq, index);
	if likely(elem) {
		result = Dee_AsObject(Dee_TYPE(elem));
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
classes_getitem(SeqSimpleProxy *__restrict self, DeeObject *index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeObject_InvokeMethodHint(seq_operator_getitem, self->sp_seq, index);
	if likely(elem) {
		result = Dee_AsObject(DeeObject_Class(elem));
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ids_getitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeObject_InvokeMethodHint(seq_operator_getitem_index, self->sp_seq, index);
	if likely(elem) {
		result = DeeInt_NewUIntptr(DeeObject_Id(elem));
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
types_getitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeObject_InvokeMethodHint(seq_operator_getitem_index, self->sp_seq, index);
	if likely(elem) {
		result = Dee_AsObject(Dee_TYPE(elem));
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
classes_getitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeObject_InvokeMethodHint(seq_operator_getitem_index, self->sp_seq, index);
	if likely(elem) {
		result = Dee_AsObject(DeeObject_Class(elem));
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ids_trygetitem(SeqSimpleProxy *__restrict self, DeeObject *index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeObject_InvokeMethodHint(seq_operator_trygetitem, self->sp_seq, index);
	if likely(ITER_ISOK(elem)) {
		result = DeeInt_NewUIntptr(DeeObject_Id(elem));
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
types_trygetitem(SeqSimpleProxy *__restrict self, DeeObject *index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeObject_InvokeMethodHint(seq_operator_trygetitem, self->sp_seq, index);
	if likely(ITER_ISOK(elem)) {
		result = Dee_AsObject(Dee_TYPE(elem));
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
classes_trygetitem(SeqSimpleProxy *__restrict self, DeeObject *index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeObject_InvokeMethodHint(seq_operator_trygetitem, self->sp_seq, index);
	if likely(ITER_ISOK(elem)) {
		result = Dee_AsObject(DeeObject_Class(elem));
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ids_trygetitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeObject_InvokeMethodHint(seq_operator_trygetitem_index, self->sp_seq, index);
	if likely(ITER_ISOK(elem)) {
		result = DeeInt_NewUIntptr(DeeObject_Id(elem));
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
types_trygetitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeObject_InvokeMethodHint(seq_operator_trygetitem_index, self->sp_seq, index);
	if likely(ITER_ISOK(elem)) {
		result = Dee_AsObject(Dee_TYPE(elem));
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
classes_trygetitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeObject_InvokeMethodHint(seq_operator_trygetitem_index, self->sp_seq, index);
	if likely(ITER_ISOK(elem)) {
		result = Dee_AsObject(DeeObject_Class(elem));
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
ids_contains_foreach_cb(void *arg, DeeObject *elem) {
	return DeeObject_Id(elem) == (uintptr_t)arg ? -2 : 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ids_contains(SeqSimpleProxy *self, DeeObject *id_obj) {
	Dee_ssize_t status;
	uintptr_t id_value;
	if (DeeObject_AsUIntptr(id_obj, &id_value))
		goto err;
	status = DeeObject_InvokeMethodHint(seq_operator_foreach, self->sp_seq, &ids_contains_foreach_cb, (void *)id_value);
	if unlikely(status == -1)
		goto err;
	return_bool(status == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
types_contains_foreach_cb(void *arg, DeeObject *elem) {
	return Dee_TYPE(elem) == (DeeTypeObject *)arg ? -2 : 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
types_contains(SeqSimpleProxy *self,
               DeeTypeObject *typ) {
	Dee_ssize_t status;
	if unlikely(!DeeType_Check(typ))
		return_false;
	status = DeeObject_InvokeMethodHint(seq_operator_foreach, self->sp_seq, &types_contains_foreach_cb, typ);
	if unlikely(status == -1)
		goto err;
	return_bool(status == 0);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
classes_contains_foreach_cb(void *arg, DeeObject *elem) {
	return DeeObject_Class(elem) == (DeeTypeObject *)arg ? -2 : 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
classes_contains(SeqSimpleProxy *self,
                 DeeTypeObject *typ) {
	Dee_ssize_t status;
	if unlikely(!DeeType_Check(typ))
		return_false;
	status = DeeObject_InvokeMethodHint(seq_operator_foreach, self->sp_seq, &classes_contains_foreach_cb, typ);
	if unlikely(status == -1)
		goto err;
	return_bool(status == 0);
err:
	return NULL;
}

struct proxy_foreach_data {
	Dee_foreach_t pfd_proc; /* [1..1] Underlying callback. */
	void         *pfd_arg;  /* [?..?] Cookie for `pfd_proc' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ids_foreach_cb(void *arg, DeeObject *elem) {
	Dee_ssize_t result;
	struct proxy_foreach_data *data;
	data = (struct proxy_foreach_data *)arg;
	elem = DeeInt_NewUIntptr(DeeObject_Id(elem));
	if unlikely(!elem)
		goto err;
	result = (*data->pfd_proc)(data->pfd_arg, elem);
	Dee_Decref(elem);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ids_foreach(SeqSimpleProxy *self, Dee_foreach_t proc, void *arg) {
	struct proxy_foreach_data data;
	data.pfd_proc = proc;
	data.pfd_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_operator_foreach, self->sp_seq, &ids_foreach_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
types_foreach_cb(void *arg, DeeObject *elem) {
	struct proxy_foreach_data *data;
	data = (struct proxy_foreach_data *)arg;
	return (*data->pfd_proc)(data->pfd_arg, (DeeObject *)Dee_TYPE(elem));
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
types_foreach(SeqSimpleProxy *self, Dee_foreach_t proc, void *arg) {
	struct proxy_foreach_data data;
	data.pfd_proc = proc;
	data.pfd_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_operator_foreach, self->sp_seq, &types_foreach_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
classes_foreach_cb(void *arg, DeeObject *elem) {
	struct proxy_foreach_data *data;
	data = (struct proxy_foreach_data *)arg;
	return (*data->pfd_proc)(data->pfd_arg, (DeeObject *)DeeObject_Class(elem));
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
classes_foreach(SeqSimpleProxy *self, Dee_foreach_t proc, void *arg) {
	struct proxy_foreach_data data;
	data.pfd_proc = proc;
	data.pfd_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_operator_foreach, self->sp_seq, &classes_foreach_cb, &data);
}

struct proxy_enumerate_data {
	Dee_seq_enumerate_t ped_proc; /* [1..1] Underlying callback. */
	void               *ped_arg;  /* [?..?] Cookie for `ped_proc' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ids_mh_seq_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	Dee_ssize_t result;
	struct proxy_enumerate_data *data;
	data = (struct proxy_enumerate_data *)arg;
	if (value) {
		value = DeeInt_NewUIntptr(DeeObject_Id(value));
		if unlikely(!value)
			goto err;
		result = (*data->ped_proc)(data->ped_arg, key, value);
		Dee_Decref(value);
	} else {
		result = (*data->ped_proc)(data->ped_arg, key, NULL);
	}
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ids_mh_seq_enumerate(SeqSimpleProxy *self, Dee_seq_enumerate_t proc, void *arg) {
	struct proxy_enumerate_data data;
	data.ped_proc = proc;
	data.ped_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_enumerate, self->sp_seq, &ids_mh_seq_enumerate_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
types_mh_seq_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct proxy_enumerate_data *data;
	data = (struct proxy_enumerate_data *)arg;
	return (*data->ped_proc)(data->ped_arg, key, value ? (DeeObject *)Dee_TYPE(value) : NULL);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
types_mh_seq_enumerate(SeqSimpleProxy *self, Dee_seq_enumerate_t proc, void *arg) {
	struct proxy_enumerate_data data;
	data.ped_proc = proc;
	data.ped_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_enumerate, self->sp_seq, &types_mh_seq_enumerate_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
classes_mh_seq_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct proxy_enumerate_data *data;
	data = (struct proxy_enumerate_data *)arg;
	return (*data->ped_proc)(data->ped_arg, key, value ? (DeeObject *)DeeObject_Class(value) : NULL);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
classes_mh_seq_enumerate(SeqSimpleProxy *self, Dee_seq_enumerate_t proc, void *arg) {
	struct proxy_enumerate_data data;
	data.ped_proc = proc;
	data.ped_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_enumerate, self->sp_seq, &classes_mh_seq_enumerate_cb, &data);
}


struct proxy_enumerate_index_data {
	Dee_seq_enumerate_index_t peid_proc; /* [1..1] Underlying callback. */
	void                     *peid_arg;  /* [?..?] Cookie for `peid_proc' */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
ids_mh_seq_enumerate_index_cb(void *arg, size_t key, DeeObject *value) {
	Dee_ssize_t result;
	struct proxy_enumerate_index_data *data;
	data = (struct proxy_enumerate_index_data *)arg;
	if (value) {
		value = DeeInt_NewUIntptr(DeeObject_Id(value));
		if unlikely(!value)
			goto err;
		result = (*data->peid_proc)(data->peid_arg, key, value);
		Dee_Decref(value);
	} else {
		result = (*data->peid_proc)(data->peid_arg, key, NULL);
	}
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ids_mh_seq_enumerate_index(SeqSimpleProxy *self, Dee_seq_enumerate_index_t proc,
                           void *arg, size_t start, size_t end) {
	struct proxy_enumerate_index_data data;
	data.peid_proc = proc;
	data.peid_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_enumerate_index, self->sp_seq,
	                                  &ids_mh_seq_enumerate_index_cb,
	                                  &data, start, end);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
types_mh_seq_enumerate_index_cb(void *arg, size_t key, DeeObject *value) {
	struct proxy_enumerate_index_data *data;
	data = (struct proxy_enumerate_index_data *)arg;
	return (*data->peid_proc)(data->peid_arg, key, value ? (DeeObject *)Dee_TYPE(value) : NULL);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
types_mh_seq_enumerate_index(SeqSimpleProxy *self, Dee_seq_enumerate_index_t proc,
                             void *arg, size_t start, size_t end) {
	struct proxy_enumerate_index_data data;
	data.peid_proc = proc;
	data.peid_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_enumerate_index, self->sp_seq,
	                                  &types_mh_seq_enumerate_index_cb,
	                                  &data, start, end);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
classes_mh_seq_enumerate_index_cb(void *arg, size_t key, DeeObject *value) {
	struct proxy_enumerate_index_data *data;
	data = (struct proxy_enumerate_index_data *)arg;
	return (*data->peid_proc)(data->peid_arg, key, value ? (DeeObject *)DeeObject_Class(value) : NULL);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
classes_mh_seq_enumerate_index(SeqSimpleProxy *self, Dee_seq_enumerate_index_t proc,
                               void *arg, size_t start, size_t end) {
	struct proxy_enumerate_index_data data;
	data.peid_proc = proc;
	data.peid_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_enumerate_index, self->sp_seq,
	                                  &classes_mh_seq_enumerate_index_cb,
	                                  &data, start, end);
}


PRIVATE struct type_seq ids_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ids_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ids_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ids_getitem,
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ids_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&proxy_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&proxy_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ids_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&proxy_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&proxy_hasitem_index,
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ids_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ids_trygetitem_index,
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

PRIVATE struct type_seq types_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&types_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&types_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&types_getitem,
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&types_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&proxy_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&proxy_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&types_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&proxy_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&proxy_hasitem_index,
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&types_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&types_trygetitem_index,
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

PRIVATE struct type_seq classes_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&classes_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&classes_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&classes_getitem,
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&classes_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&proxy_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&proxy_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&classes_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&proxy_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&proxy_hasitem_index,
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&classes_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&classes_trygetitem_index,
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

PRIVATE struct type_method tpconst proxy_methods[] = {
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst ids_method_hints[] = {
	TYPE_METHOD_HINT(seq_enumerate, &ids_mh_seq_enumerate),
	TYPE_METHOD_HINT(seq_enumerate_index, &ids_mh_seq_enumerate_index),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_method_hint tpconst types_method_hints[] = {
	TYPE_METHOD_HINT(seq_enumerate, &types_mh_seq_enumerate),
	TYPE_METHOD_HINT(seq_enumerate_index, &types_mh_seq_enumerate_index),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_method_hint tpconst classes_method_hints[] = {
	TYPE_METHOD_HINT(seq_enumerate, &classes_mh_seq_enumerate),
	TYPE_METHOD_HINT(seq_enumerate_index, &classes_mh_seq_enumerate_index),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_member tpconst ids_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqIdsIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeInt_Type),
	TYPE_MEMBER_CONST(STR_Frozen, &SeqIds_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst types_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqTypesIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeType_Type),
	TYPE_MEMBER_CONST(STR_Frozen, &SeqTypes_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst classes_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqClassesIterator_Type),
	TYPE_MEMBER_CONST(STR_ItemType, &DeeType_Type),
	TYPE_MEMBER_CONST(STR_Frozen, &SeqClasses_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqIds_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqIds",
	/* .tp_doc      = */ DOC("(seq?:?DSequence)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqSimpleProxy,
			/* tp_ctor:        */ &proxy_ctor,
			/* tp_copy_ctor:   */ &proxy_copy,
			/* tp_deep_ctor:   */ &proxy_deep,
			/* tp_any_ctor:    */ &proxy_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &proxy_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&proxy_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&proxy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__247219960F1E745D),
	/* .tp_seq           = */ &ids_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ proxy_methods,
	/* .tp_getsets       = */ NULL, /* Don't use "proxy_getsets" here -- freezing the underlying sequence may change object IDs */
	/* .tp_members       = */ proxy_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ids_class_members,
	/* .tp_method_hints  = */ ids_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject SeqTypes_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqTypes",
	/* .tp_doc      = */ DOC("(seq?:?DSequence)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqSimpleProxy,
			/* tp_ctor:        */ &proxy_ctor,
			/* tp_copy_ctor:   */ &proxy_copy,
			/* tp_deep_ctor:   */ &proxy_deep,
			/* tp_any_ctor:    */ &proxy_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &proxy_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&proxy_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&proxy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__247219960F1E745D),
	/* .tp_seq           = */ &types_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ proxy_methods,
	/* .tp_getsets       = */ proxy_getsets,
	/* .tp_members       = */ proxy_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ types_class_members,
	/* .tp_method_hints  = */ types_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject SeqClasses_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqClasses",
	/* .tp_doc      = */ DOC("(seq?:?DSequence)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqSimpleProxy,
			/* tp_ctor:        */ &proxy_ctor,
			/* tp_copy_ctor:   */ &proxy_copy,
			/* tp_deep_ctor:   */ &proxy_deep,
			/* tp_any_ctor:    */ &proxy_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &proxy_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&proxy_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&proxy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__247219960F1E745D),
	/* .tp_seq           = */ &classes_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ proxy_methods,
	/* .tp_getsets       = */ proxy_getsets,
	/* .tp_members       = */ proxy_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ classes_class_members,
	/* .tp_method_hints  = */ classes_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};



PRIVATE WUNUSED NONNULL((1)) int DCALL
iter_ctor(SeqSimpleProxyIterator *__restrict self) {
	self->si_iter = DeeObject_Iter(Dee_EmptySeq);
	if unlikely(!self->si_iter)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
iter_init(SeqSimpleProxyIterator *__restrict self,
          size_t argc, DeeObject *const *argv) {
	SeqSimpleProxy *seq;
	DeeTypeObject *tp;
	if unlikely(argc != 1)
		return DeeArg_BadArgc1(DeeType_GetName(Dee_TYPE(self)), argc);
	seq = (SeqSimpleProxy *)argv[0];
	tp = (self->ob_type == &SeqIdsIterator_Type)
	     ? &SeqIds_Type
	     : (self->ob_type == &SeqTypesIterator_Type)
	       ? &SeqTypes_Type
	       : &SeqClasses_Type;
	if (DeeObject_AssertTypeExact(seq, tp))
		goto err;
	self->si_iter = DeeObject_Iter(seq->sp_seq);
	if unlikely(!self->si_iter)
		goto err;
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(SeqSimpleProxyIterator, si_iter) == offsetof(ProxyObject, po_obj));
#define iter_copy          generic_proxy__copy_recursive
#define iter_deep          generic_proxy__deepcopy
#define iter_serialize     generic_proxy__serialize
#define iter_fini          generic_proxy__fini
#define iter_visit         generic_proxy__visit
#define iter_bool          generic_proxy__bool
#define iter_hash          generic_proxy__hash_recursive
#define iter_compare       generic_proxy__compare_recursive
#define iter_compare_eq    generic_proxy__compare_eq_recursive
#define iter_trycompare_eq generic_proxy__trycompare_eq_recursive
#define iter_cmp           generic_proxy__cmp_recursive

PRIVATE struct type_member tpconst iter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(SeqSimpleProxyIterator, si_iter), "->?DIterator"),
	TYPE_MEMBER_END
};




PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
idsiter_next(SeqSimpleProxyIterator *__restrict self) {
	DREF DeeObject *result = DeeObject_IterNext(self->si_iter);
	if (ITER_ISOK(result)) {
		DREF DeeObject *elem = result;
		result = DeeInt_NewUIntptr(DeeObject_Id(elem));
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
typesiter_next(SeqSimpleProxyIterator *__restrict self) {
	DREF DeeObject *result = DeeObject_IterNext(self->si_iter);
	if (ITER_ISOK(result)) {
		DREF DeeObject *elem = result;
		result = Dee_AsObject(Dee_TYPE(elem));
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
classesiter_next(SeqSimpleProxyIterator *__restrict self) {
	DREF DeeObject *result = DeeObject_IterNext(self->si_iter);
	if (ITER_ISOK(result)) {
		DREF DeeObject *elem = result;
		result = Dee_AsObject(DeeObject_Class(elem));
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
idsiter_getseq(SeqSimpleProxyIterator *__restrict self) {
	DREF DeeObject *result = DeeObject_GetAttr(self->si_iter, Dee_AsObject(&str_seq));
	if (ITER_ISOK(result)) {
		DREF DeeObject *baseseq = result;
		result = SeqIds_New(baseseq);
		Dee_Decref(baseseq);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
typesiter_getseq(SeqSimpleProxyIterator *__restrict self) {
	DREF DeeObject *result = DeeObject_GetAttr(self->si_iter, Dee_AsObject(&str_seq));
	if (ITER_ISOK(result)) {
		DREF DeeObject *baseseq = result;
		result = SeqTypes_New(baseseq);
		Dee_Decref(baseseq);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
classesiter_getseq(SeqSimpleProxyIterator *__restrict self) {
	DREF DeeObject *result = DeeObject_GetAttr(self->si_iter, Dee_AsObject(&str_seq));
	if (ITER_ISOK(result)) {
		DREF DeeObject *baseseq = result;
		result = SeqClasses_New(baseseq);
		Dee_Decref(baseseq);
	}
	return result;
}


PRIVATE struct type_getset tpconst idsiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &idsiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:SeqIds"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst typesiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &typesiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:SeqTypes"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst classesiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &classesiter_getseq, METHOD_FNOREFESCAPE, "->?Ert:SeqClasses"),
	TYPE_GETSET_END
};



INTERN DeeTypeObject SeqIdsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqIdsIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqIds)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqSimpleProxyIterator,
			/* tp_ctor:        */ &iter_ctor,
			/* tp_copy_ctor:   */ &iter_copy,
			/* tp_deep_ctor:   */ &iter_deep,
			/* tp_any_ctor:    */ &iter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &iter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&iter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&iter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&iter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &iter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&idsiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ idsiter_getsets,
	/* .tp_members       = */ iter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject SeqTypesIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqTypesIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqTypes)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqSimpleProxyIterator,
			/* tp_ctor:        */ &iter_ctor,
			/* tp_copy_ctor:   */ &iter_copy,
			/* tp_deep_ctor:   */ &iter_deep,
			/* tp_any_ctor:    */ &iter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &iter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&iter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&iter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&iter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &iter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&typesiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ typesiter_getsets,
	/* .tp_members       = */ iter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject SeqClassesIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqClassesIterator",
	/* .tp_doc      = */ DOC("(seq?:?Ert:SeqClasses)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqSimpleProxyIterator,
			/* tp_ctor:        */ &iter_ctor,
			/* tp_copy_ctor:   */ &iter_copy,
			/* tp_deep_ctor:   */ &iter_deep,
			/* tp_any_ctor:    */ &iter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &iter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&iter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&iter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&iter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &iter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&classesiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ classesiter_getsets,
	/* .tp_members       = */ iter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};



INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
SeqIds_New(DeeObject *__restrict seq) {
	DREF SeqSimpleProxy *result;
	result = DeeObject_MALLOC(SeqSimpleProxy);
	if unlikely(!result)
		goto done;
	Dee_Incref(seq);
	result->sp_seq = seq;
	DeeObject_Init(result, &SeqIds_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
SeqTypes_New(DeeObject *__restrict seq) {
	DREF SeqSimpleProxy *result;
	result = DeeObject_MALLOC(SeqSimpleProxy);
	if unlikely(!result)
		goto done;
	Dee_Incref(seq);
	result->sp_seq = seq;
	DeeObject_Init(result, &SeqTypes_Type);
done:
	return Dee_AsObject(result);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
SeqClasses_New(DeeObject *__restrict seq) {
	DREF SeqSimpleProxy *result;
	result = DeeObject_MALLOC(SeqSimpleProxy);
	if unlikely(!result)
		goto done;
	Dee_Incref(seq);
	result->sp_seq = seq;
	DeeObject_Init(result, &SeqClasses_Type);
done:
	return Dee_AsObject(result);
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_C */
