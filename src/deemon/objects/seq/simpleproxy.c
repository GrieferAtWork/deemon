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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_C
#define GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "default-api.h"

/**/
#include "simpleproxy.h"

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_ctor(SeqSimpleProxy *__restrict self) {
	self->sp_seq = Dee_EmptySeq;
	Dee_Incref(Dee_EmptySeq);
	return 0;
}

STATIC_ASSERT(offsetof(SeqSimpleProxy, sp_seq) == offsetof(ProxyObject, po_obj));
#define proxy_copy            generic_proxy_copy_alias
#define proxy_deep            generic_proxy_deepcopy
#define proxy_init            generic_proxy_init
#define proxy_fini            generic_proxy_fini
#define proxy_visit           generic_proxy_visit
#define proxy_bool            generic_proxy_bool
#define proxy_sizeob          generic_proxy_sizeob
#define proxy_size            generic_proxy_size
#define proxy_size_fast       generic_proxy_size_fast
#define proxy_iterkeys        generic_proxy_iterkeys
#define proxy_hasitem         generic_proxy_hasitem
#define proxy_bounditem       generic_proxy_bounditem
#define proxy_hasitem_index   generic_proxy_hasitem_index
#define proxy_bounditem_index generic_proxy_bounditem_index

PRIVATE WUNUSED NONNULL((1)) DREF SeqSimpleProxy *DCALL
proxy_get_frozen(SeqSimpleProxy *__restrict self) {
	DREF DeeObject *inner_frozen;
	DREF SeqSimpleProxy *result;
	inner_frozen = DeeObject_GetAttr(self->sp_seq, (DeeObject *)&str_frozen);
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
	result->si_iter = DeeSeq_OperatorIter(self->sp_seq);
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
	result->si_iter = DeeSeq_OperatorIter(self->sp_seq);
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
	result->si_iter = DeeSeq_OperatorIter(self->sp_seq);
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
	result = elem = DeeSeq_OperatorGetItem(self->sp_seq, index);
	if likely(elem) {
		result = DeeInt_NewUIntptr(DeeObject_Id(elem));
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
types_getitem(SeqSimpleProxy *__restrict self, DeeObject *index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeSeq_OperatorGetItem(self->sp_seq, index);
	if likely(elem) {
		result = (DREF DeeObject *)Dee_TYPE(elem);
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
classes_getitem(SeqSimpleProxy *__restrict self, DeeObject *index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeSeq_OperatorGetItem(self->sp_seq, index);
	if likely(elem) {
		result = (DREF DeeObject *)DeeObject_Class(elem);
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ids_getitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeSeq_OperatorGetItemIndex(self->sp_seq, index);
	if likely(elem) {
		result = DeeInt_NewUIntptr(DeeObject_Id(elem));
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
types_getitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeSeq_OperatorGetItemIndex(self->sp_seq, index);
	if likely(elem) {
		result = (DREF DeeObject *)Dee_TYPE(elem);
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
classes_getitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeSeq_OperatorGetItemIndex(self->sp_seq, index);
	if likely(elem) {
		result = (DREF DeeObject *)DeeObject_Class(elem);
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ids_trygetitem(SeqSimpleProxy *__restrict self, DeeObject *index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeSeq_OperatorTryGetItem(self->sp_seq, index);
	if likely(ITER_ISOK(elem)) {
		result = DeeInt_NewUIntptr(DeeObject_Id(elem));
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
types_trygetitem(SeqSimpleProxy *__restrict self, DeeObject *index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeSeq_OperatorTryGetItem(self->sp_seq, index);
	if likely(ITER_ISOK(elem)) {
		result = (DREF DeeObject *)Dee_TYPE(elem);
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
classes_trygetitem(SeqSimpleProxy *__restrict self, DeeObject *index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeSeq_OperatorTryGetItem(self->sp_seq, index);
	if likely(ITER_ISOK(elem)) {
		result = (DREF DeeObject *)DeeObject_Class(elem);
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ids_trygetitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeSeq_OperatorTryGetItemIndex(self->sp_seq, index);
	if likely(ITER_ISOK(elem)) {
		result = DeeInt_NewUIntptr(DeeObject_Id(elem));
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
types_trygetitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeSeq_OperatorTryGetItemIndex(self->sp_seq, index);
	if likely(ITER_ISOK(elem)) {
		result = (DREF DeeObject *)Dee_TYPE(elem);
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
classes_trygetitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeSeq_OperatorTryGetItemIndex(self->sp_seq, index);
	if likely(ITER_ISOK(elem)) {
		result = (DREF DeeObject *)DeeObject_Class(elem);
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
	status = DeeSeq_OperatorForeach(self->sp_seq, &ids_contains_foreach_cb, (void *)id_value);
	if unlikely(status == -1)
		goto err;
	return_bool_(status == 0);
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
	status = DeeSeq_OperatorForeach(self->sp_seq, &types_contains_foreach_cb, typ);
	if unlikely(status == -1)
		goto err;
	return_bool_(status == 0);
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
	status = DeeSeq_OperatorForeach(self->sp_seq, &classes_contains_foreach_cb, typ);
	if unlikely(status == -1)
		goto err;
	return_bool_(status == 0);
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
	return DeeSeq_OperatorForeach(self->sp_seq, &ids_foreach_cb, &data);
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
	return DeeSeq_OperatorForeach(self->sp_seq, &types_foreach_cb, &data);
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
	return DeeSeq_OperatorForeach(self->sp_seq, &classes_foreach_cb, &data);
}

struct proxy_enumerate_data {
	Dee_seq_enumerate_t ped_proc; /* [1..1] Underlying callback. */
	void           *ped_arg;  /* [?..?] Cookie for `ped_proc' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ids_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
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
ids_enumerate(SeqSimpleProxy *self, Dee_seq_enumerate_t proc, void *arg) {
	struct proxy_enumerate_data data;
	data.ped_proc = proc;
	data.ped_arg  = arg;
	return DeeSeq_OperatorEnumerate(self->sp_seq, &ids_enumerate_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
types_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct proxy_enumerate_data *data;
	data = (struct proxy_enumerate_data *)arg;
	return (*data->ped_proc)(data->ped_arg, key, value ? (DeeObject *)Dee_TYPE(value) : NULL);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
types_enumerate(SeqSimpleProxy *self, Dee_seq_enumerate_t proc, void *arg) {
	struct proxy_enumerate_data data;
	data.ped_proc = proc;
	data.ped_arg  = arg;
	return DeeSeq_OperatorEnumerate(self->sp_seq, &types_enumerate_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
classes_enumerate_cb(void *arg, DeeObject *key, DeeObject *value) {
	struct proxy_enumerate_data *data;
	data = (struct proxy_enumerate_data *)arg;
	return (*data->ped_proc)(data->ped_arg, key, value ? (DeeObject *)DeeObject_Class(value) : NULL);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
classes_enumerate(SeqSimpleProxy *self, Dee_seq_enumerate_t proc, void *arg) {
	struct proxy_enumerate_data data;
	data.ped_proc = proc;
	data.ped_arg  = arg;
	return DeeSeq_OperatorEnumerate(self->sp_seq, &classes_enumerate_cb, &data);
}


struct proxy_enumerate_index_data {
	Dee_seq_enumerate_index_t peid_proc; /* [1..1] Underlying callback. */
	void                 *peid_arg;  /* [?..?] Cookie for `peid_proc' */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
ids_enumerate_index_cb(void *arg, size_t key, DeeObject *value) {
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
ids_enumerate_index(SeqSimpleProxy *self, Dee_seq_enumerate_index_t proc,
                    void *arg, size_t start, size_t end) {
	struct proxy_enumerate_index_data data;
	data.peid_proc = proc;
	data.peid_arg  = arg;
	return DeeSeq_OperatorEnumerateIndex(self->sp_seq, &ids_enumerate_index_cb, &data, start, end);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
types_enumerate_index_cb(void *arg, size_t key, DeeObject *value) {
	struct proxy_enumerate_index_data *data;
	data = (struct proxy_enumerate_index_data *)arg;
	return (*data->peid_proc)(data->peid_arg, key, value ? (DeeObject *)Dee_TYPE(value) : NULL);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
types_enumerate_index(SeqSimpleProxy *self, Dee_seq_enumerate_index_t proc,
                      void *arg, size_t start, size_t end) {
	struct proxy_enumerate_index_data data;
	data.peid_proc = proc;
	data.peid_arg  = arg;
	return DeeSeq_OperatorEnumerateIndex(self->sp_seq, &types_enumerate_index_cb, &data, start, end);
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
classes_enumerate_index_cb(void *arg, size_t key, DeeObject *value) {
	struct proxy_enumerate_index_data *data;
	data = (struct proxy_enumerate_index_data *)arg;
	return (*data->peid_proc)(data->peid_arg, key, value ? (DeeObject *)DeeObject_Class(value) : NULL);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
classes_enumerate_index(SeqSimpleProxy *self, Dee_seq_enumerate_index_t proc,
                        void *arg, size_t start, size_t end) {
	struct proxy_enumerate_index_data data;
	data.peid_proc = proc;
	data.peid_arg  = arg;
	return DeeSeq_OperatorEnumerateIndex(self->sp_seq, &classes_enumerate_index_cb, &data, start, end);
}


PRIVATE struct type_seq ids_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ids_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ids_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ids_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ids_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_t, void *))&ids_enumerate,
	/* .tp_enumerate_index            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_index_t, void *, size_t, size_t))&ids_enumerate_index,
	/* .tp_iterkeys                   = */ (DREF DeeObject *(DCALL *)(DeeObject *))&proxy_iterkeys,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&proxy_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&proxy_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ids_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&proxy_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&proxy_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ids_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ids_trygetitem_index,
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

PRIVATE struct type_seq types_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&types_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&types_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&types_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&types_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_t, void *))&types_enumerate,
	/* .tp_enumerate_index            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_index_t, void *, size_t, size_t))&types_enumerate_index,
	/* .tp_iterkeys                   = */ (DREF DeeObject *(DCALL *)(DeeObject *))&proxy_iterkeys,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&proxy_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&proxy_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&types_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&proxy_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&proxy_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&types_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&types_trygetitem_index,
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

PRIVATE struct type_seq classes_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&classes_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&proxy_sizeob,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&classes_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&classes_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&classes_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_t, void *))&classes_enumerate,
	/* .tp_enumerate_index            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_seq_enumerate_index_t, void *, size_t, size_t))&classes_enumerate_index,
	/* .tp_iterkeys                   = */ (DREF DeeObject *(DCALL *)(DeeObject *))&proxy_iterkeys,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&proxy_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&proxy_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&classes_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&proxy_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&proxy_hasitem_index,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&classes_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&classes_trygetitem_index,
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


PRIVATE struct type_member tpconst ids_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqIdsIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &SeqIds_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst types_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqTypesIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &SeqTypes_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst classes_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqClassesIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &SeqClasses_Type),
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(SeqSimpleProxy)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&proxy_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &ids_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_getsets,
	/* .tp_members       = */ proxy_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ ids_class_members
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(SeqSimpleProxy)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&proxy_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &types_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_getsets,
	/* .tp_members       = */ proxy_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ types_class_members
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&proxy_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&proxy_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&proxy_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&proxy_init,
				TYPE_FIXED_ALLOCATOR(SeqSimpleProxy)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&proxy_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&proxy_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&proxy_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &classes_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ proxy_getsets,
	/* .tp_members       = */ proxy_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ classes_class_members
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
	if (DeeArg_Unpack(argc, argv,
	                  self->ob_type == &SeqIdsIterator_Type
	                  ? "o:_SeqIdsIterator"
	                  : self->ob_type == &SeqTypesIterator_Type
	                    ? "o:_SeqTypesIterator"
	                    : "o:_SeqClassesIterator",
	                  &seq))
		goto err;
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
#define iter_copy          generic_proxy_copy_recursive
#define iter_deep          generic_proxy_deepcopy
#define iter_fini          generic_proxy_fini
#define iter_visit         generic_proxy_visit
#define iter_bool          generic_proxy_bool
#define iter_hash          generic_proxy_hash_recursive
#define iter_compare       generic_proxy_compare_recursive
#define iter_compare_eq    generic_proxy_compare_eq_recursive
#define iter_trycompare_eq generic_proxy_trycompare_eq_recursive
#define iter_cmp           generic_proxy_cmp_recursive

PRIVATE struct type_member tpconst iter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(SeqSimpleProxyIterator, si_iter), "->?DIterator"),
	TYPE_MEMBER_END
};




PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
idsiter_next(SeqSimpleProxyIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *elem;
	result = elem = DeeObject_IterNext(self->si_iter);
	if (ITER_ISOK(elem)) {
		result = DeeInt_NewUIntptr(DeeObject_Id(elem));
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
typesiter_next(SeqSimpleProxyIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *elem;
	result = elem = DeeObject_IterNext(self->si_iter);
	if (ITER_ISOK(elem)) {
		result = (DREF DeeObject *)Dee_TYPE(elem);
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
classesiter_next(SeqSimpleProxyIterator *__restrict self) {
	DREF DeeObject *result;
	DREF DeeObject *elem;
	result = elem = DeeObject_IterNext(self->si_iter);
	if (ITER_ISOK(elem)) {
		result = (DREF DeeObject *)DeeObject_Class(elem);
		Dee_Incref(result);
		Dee_Decref(elem);
	}
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
idsiter_getseq(SeqSimpleProxyIterator *__restrict self) {
	DREF DeeObject *baseseq, *result;
	result = baseseq = DeeObject_GetAttr(self->si_iter, (DeeObject *)&str_seq);
	if (ITER_ISOK(baseseq)) {
		result = SeqIds_New(baseseq);
		Dee_Decref(baseseq);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
typesiter_getseq(SeqSimpleProxyIterator *__restrict self) {
	DREF DeeObject *baseseq, *result;
	result = baseseq = DeeObject_GetAttr(self->si_iter, (DeeObject *)&str_seq);
	if (ITER_ISOK(baseseq)) {
		result = SeqTypes_New(baseseq);
		Dee_Decref(baseseq);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
classesiter_getseq(SeqSimpleProxyIterator *__restrict self) {
	DREF DeeObject *baseseq, *result;
	result = baseseq = DeeObject_GetAttr(self->si_iter, (DeeObject *)&str_seq);
	if (ITER_ISOK(baseseq)) {
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&iter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&iter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&iter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&iter_init,
				TYPE_FIXED_ALLOCATOR(SeqSimpleProxyIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&iter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&iter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&iter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &iter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&idsiter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ idsiter_getsets,
	/* .tp_members       = */ iter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&iter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&iter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&iter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&iter_init,
				TYPE_FIXED_ALLOCATOR(SeqSimpleProxyIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&iter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&iter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&iter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &iter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&typesiter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ typesiter_getsets,
	/* .tp_members       = */ iter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&iter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&iter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&iter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&iter_init,
				TYPE_FIXED_ALLOCATOR(SeqSimpleProxyIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&iter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&iter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&iter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &iter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&classesiter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ classesiter_getsets,
	/* .tp_members       = */ iter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
	return (DREF DeeObject *)result;
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
	return (DREF DeeObject *)result;
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
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_C */
