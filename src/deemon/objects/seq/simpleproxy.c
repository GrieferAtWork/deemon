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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_C
#define GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_C 1

#include "simpleproxy.h"

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

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_ctor(SeqSimpleProxy *__restrict self) {
	self->sp_seq = Dee_EmptySeq;
	Dee_Incref(Dee_EmptySeq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_copy(SeqSimpleProxy *__restrict self,
           SeqSimpleProxy *__restrict other) {
	self->sp_seq = other->sp_seq;
	Dee_Incref(self->sp_seq);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
proxy_deep(SeqSimpleProxy *__restrict self,
           SeqSimpleProxy *__restrict other) {
	self->sp_seq = DeeObject_DeepCopy(other->sp_seq);
	if unlikely(!self->sp_seq)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_init(SeqSimpleProxy *__restrict self,
           size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv,
	                  self->ob_type == &SeqIds_Type
	                  ? "o:_SeqIds"
	                  : self->ob_type == &SeqTypes_Type
	                    ? "o:_SeqTypes"
	                    : "o:_SeqClasses",
	                  &self->sp_seq))
		goto err;
	Dee_Incref(self->sp_seq);
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
proxy_fini(SeqSimpleProxy *__restrict self) {
	Dee_Decref(self->sp_seq);
}

PRIVATE NONNULL((1, 2)) void DCALL
proxy_visit(SeqSimpleProxy *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->sp_seq);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
proxy_bool(SeqSimpleProxy *__restrict self) {
	return DeeObject_Bool(self->sp_seq);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
proxy_size(SeqSimpleProxy *__restrict self) {
	return DeeObject_Size(self->sp_seq);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
proxy_size_fast(SeqSimpleProxy *__restrict self) {
	return DeeObject_SizeFast(self->sp_seq);
}

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
	result->si_iter = DeeObject_Iter(self->sp_seq);
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
	result->si_iter = DeeObject_Iter(self->sp_seq);
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
	result->si_iter = DeeObject_Iter(self->sp_seq);
	if unlikely(!result->si_iter)
		goto err_r;
	DeeObject_Init(result, &SeqClassesIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ids_getitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeObject_GetItemIndex(self->sp_seq, index);
	if likely(elem) {
		result = DeeInt_NewUIntptr(DeeObject_Id(elem));
		Dee_Decref(elem);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
types_getitem_index(SeqSimpleProxy *__restrict self, size_t index) {
	DREF DeeObject *result, *elem;
	result = elem = DeeObject_GetItemIndex(self->sp_seq, index);
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
	result = elem = DeeObject_GetItemIndex(self->sp_seq, index);
	if likely(elem) {
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
	status = DeeObject_Foreach(self->sp_seq, &ids_contains_foreach_cb, (void *)id_value);
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
	status = DeeObject_Foreach(self->sp_seq, &types_contains_foreach_cb, typ);
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
	status = DeeObject_Foreach(self->sp_seq, &classes_contains_foreach_cb, typ);
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
	return DeeObject_Foreach(self->sp_seq, &ids_foreach_cb, &data);
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
	return DeeObject_Foreach(self->sp_seq, &types_foreach_cb, &data);
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
	return DeeObject_Foreach(self->sp_seq, &classes_foreach_cb, &data);
}


PRIVATE struct type_nsi tpconst ids_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&proxy_size,
			/* .nsi_getsize_fast = */ (dfunptr_t)NULL,
			/* .nsi_getitem      = */ (dfunptr_t)&ids_getitem_index,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_delrange     = */ (dfunptr_t)NULL,
			/* .nsi_delrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)NULL, /* TODO */
			/* .nsi_rfind        = */ (dfunptr_t)NULL, /* TODO */
			/* .nsi_xch          = */ (dfunptr_t)NULL,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)NULL,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)NULL,
			/* .nsi_rremove      = */ (dfunptr_t)NULL,
			/* .nsi_removeall    = */ (dfunptr_t)NULL,
			/* .nsi_removeif     = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_nsi tpconst types_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&proxy_size,
			/* .nsi_getsize_fast = */ (dfunptr_t)NULL,
			/* .nsi_getitem      = */ (dfunptr_t)&types_getitem_index,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_delrange     = */ (dfunptr_t)NULL,
			/* .nsi_delrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)NULL, /* TODO */
			/* .nsi_rfind        = */ (dfunptr_t)NULL, /* TODO */
			/* .nsi_xch          = */ (dfunptr_t)NULL,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)NULL,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)NULL,
			/* .nsi_rremove      = */ (dfunptr_t)NULL,
			/* .nsi_removeall    = */ (dfunptr_t)NULL,
			/* .nsi_removeif     = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_nsi tpconst classes_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&proxy_size,
			/* .nsi_getsize_fast = */ (dfunptr_t)NULL,
			/* .nsi_getitem      = */ (dfunptr_t)&classes_getitem_index,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)NULL,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_delrange     = */ (dfunptr_t)NULL,
			/* .nsi_delrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)NULL, /* TODO */
			/* .nsi_rfind        = */ (dfunptr_t)NULL, /* TODO */
			/* .nsi_xch          = */ (dfunptr_t)NULL,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)NULL,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)NULL,
			/* .nsi_rremove      = */ (dfunptr_t)NULL,
			/* .nsi_removeall    = */ (dfunptr_t)NULL,
			/* .nsi_removeif     = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_seq ids_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ids_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ids_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ &ids_nsi,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ids_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ids_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
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
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&types_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ &types_nsi,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&types_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&types_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
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
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&classes_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ &classes_nsi,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&classes_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&proxy_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&classes_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
iter_copy(SeqSimpleProxyIterator *__restrict self,
          SeqSimpleProxyIterator *__restrict other) {
	self->si_iter = DeeObject_Copy(other->si_iter);
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

STATIC_ASSERT(offsetof(SeqSimpleProxy, sp_seq) ==
              offsetof(SeqSimpleProxyIterator, si_iter));
#define iter_deep proxy_deep
#define iter_fini proxy_fini
#define iter_bool proxy_bool
#define iter_visit proxy_visit

#define DEFINE_ITER_COMPARE(name, func)                       \
	PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL     \
	name(SeqSimpleProxyIterator *self,                        \
	     SeqSimpleProxyIterator *other) {                     \
		if (DeeObject_AssertTypeExact(other, Dee_TYPE(self))) \
			goto err;                                         \
		return func(self->si_iter, other->si_iter);           \
	err:                                                      \
		return NULL;                                          \
	}
DEFINE_ITER_COMPARE(iter_eq, DeeObject_CompareEqObject)
DEFINE_ITER_COMPARE(iter_ne, DeeObject_CompareNeObject)
DEFINE_ITER_COMPARE(iter_lo, DeeObject_CompareLoObject)
DEFINE_ITER_COMPARE(iter_le, DeeObject_CompareLeObject)
DEFINE_ITER_COMPARE(iter_gr, DeeObject_CompareGrObject)
DEFINE_ITER_COMPARE(iter_ge, DeeObject_CompareGeObject)
#undef DEFINE_ITER_COMPARE

PRIVATE struct type_cmp iter_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&iter_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&iter_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&iter_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&iter_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&iter_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&iter_ge
};

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
