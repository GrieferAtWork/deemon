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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_MAPPED_C
#define GUARD_DEEMON_OBJECTS_SEQ_MAPPED_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>

#include <hybrid/minmax.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"

/**/
#include "mapped.h"

DECL_BEGIN

STATIC_ASSERT(offsetof(SeqMappedIterator, smi_iter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqMappedIterator, smi_iter) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(SeqMappedIterator, smi_mapper) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqMappedIterator, smi_mapper) == offsetof(ProxyObject2, po_obj2));
#define mappediter_fini  generic_proxy2_fini
#define mappediter_visit generic_proxy2_visit

STATIC_ASSERT(offsetof(SeqMappedIterator, smi_iter) == offsetof(ProxyObject, po_obj));
#define mappediter_bool generic_proxy_bool

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mappediter_next(SeqMappedIterator *__restrict self) {
	DREF DeeObject *result;
	result = DeeObject_IterNext(self->smi_iter);
	if (ITER_ISOK(result)) {
		DREF DeeObject *new_result;
		/* Invoke the mapper callback. */
		new_result = DeeObject_Call(self->smi_mapper, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mappediter_seq_get(SeqMappedIterator *__restrict self) {
	/* Forward access to this attribute to the pointed-to iterator. */
	DREF DeeObject *orig, *result;
	orig = DeeObject_GetAttr(self->smi_iter, (DeeObject *)&str_seq);
	if unlikely(!orig)
		goto err;
	result = DeeSeq_Map(orig, self->smi_mapper);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}

PRIVATE struct type_getset tpconst mappediter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &mappediter_seq_get, METHOD_FNOREFESCAPE, "->?Ert:SeqMapped"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst mappediter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(SeqMappedIterator, smi_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(SeqMappedIterator, smi_mapper), "->?DCallable"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
mappediter_hash(SeqMappedIterator *self) {
	return Dee_HashCombine(DeeObject_HashGeneric(self->smi_mapper),
	                       DeeObject_Hash(self->smi_iter));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mappediter_compare_eq(SeqMappedIterator *self, SeqMappedIterator *other) {
	if (DeeObject_AssertTypeExact(other, &SeqMappedIterator_Type))
		goto err;
	if (self->smi_mapper != other->smi_mapper)
		return 1;
	return DeeObject_CompareEq(self->smi_iter, other->smi_iter);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mappediter_compare(SeqMappedIterator *self, SeqMappedIterator *other) {
	if (DeeObject_AssertTypeExact(other, &SeqMappedIterator_Type))
		goto err;
	Dee_return_compare_if_ne(self->smi_mapper, other->smi_mapper);
	return DeeObject_Compare(self->smi_iter, other->smi_iter);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp mappediter_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&mappediter_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&mappediter_compare_eq,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&mappediter_compare,
};

STATIC_ASSERT(offsetof(SeqMappedIterator, smi_iter) == offsetof(ProxyObject2, po_obj1));
STATIC_ASSERT(offsetof(SeqMappedIterator, smi_mapper) == offsetof(ProxyObject2, po_obj2));
#define mappediter_copy generic_proxy2_copy_recursive1_alias2 /* copy "smi_iter", alias "smi_mapper" */
#define mappediter_deep generic_proxy2_deepcopy

PRIVATE WUNUSED NONNULL((1)) int DCALL
mappediter_ctor(SeqMappedIterator *__restrict self) {
	self->smi_iter = DeeObject_Iter(Dee_EmptySeq);
	if unlikely(!self->smi_iter)
		goto err;
	self->smi_mapper = Dee_None;
	Dee_Incref(Dee_None);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mappediter_init(SeqMappedIterator *__restrict self,
               size_t argc, DeeObject *const *argv) {
	SeqMapped *trans;
	if (DeeArg_Unpack(argc, argv, "o:_SeqMappedIterator", &trans))
		goto err;
	if (DeeObject_AssertTypeExact(trans, &SeqMapped_Type))
		goto err;
	self->smi_iter = DeeObject_Iter(trans->sm_seq);
	if unlikely(!self->smi_iter)
		goto err;
	self->smi_mapper = trans->sm_mapper;
	Dee_Incref(self->smi_mapper);
	return 0;
err:
	return -1;
}

INTERN DeeTypeObject SeqMappedIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqMappedIterator",
	/* .tp_doc      = */ DOC("(mapped:?Ert:SeqMapped)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&mappediter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&mappediter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&mappediter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&mappediter_init,
				TYPE_FIXED_ALLOCATOR(SeqMappedIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mappediter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&mappediter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&mappediter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &mappediter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mappediter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ mappediter_getsets,
	/* .tp_members       = */ mappediter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(SeqMapped, sm_mapper) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqMapped, sm_mapper) == offsetof(ProxyObject2, po_obj2));
#define mapped_fini  generic_proxy2_fini
#define mapped_visit generic_proxy2_visit

STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_bool generic_proxy_bool

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_iter(SeqMapped *__restrict self) {
	DREF SeqMappedIterator *result;
	result = DeeObject_MALLOC(SeqMappedIterator);
	if unlikely(!result)
		goto err;

	/* Create the underlying iterator. */
	result->smi_iter = DeeObject_Iter(self->sm_seq);
	if unlikely(!result->smi_iter)
		goto err_r;

	/* Assign the mapper functions. */
	result->smi_mapper = self->sm_mapper;
	Dee_Incref(self->sm_mapper);
	DeeObject_Init(result, &SeqMappedIterator_Type);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE struct type_member tpconst mapped_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(SeqMapped, sm_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(SeqMapped, sm_mapper), "->?DCallable"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst mapped_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqMappedIterator_Type),
	TYPE_MEMBER_END
};

STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_sizeob generic_proxy_sizeob

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mapped_getitem(SeqMapped *self, DeeObject *index) {
	DREF DeeObject *orig, *result;
	orig = DeeObject_GetItem(self->sm_seq, index);
	if unlikely(!orig)
		goto err;
	result = DeeObject_Call(self->sm_mapper, 1, &orig);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
mapped_getrange(SeqMapped *self, DeeObject *start, DeeObject *end) {
	DREF DeeObject *orig, *result;
	orig = DeeObject_GetRange(self->sm_seq, start, end);
	if unlikely(!orig)
		goto err;
	result = DeeSeq_Map(orig, self->sm_mapper);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}


STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_size      generic_proxy_size
#define mapped_size_fast generic_proxy_size_fast

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_getitem_index(SeqMapped *__restrict self, size_t index) {
	DREF DeeObject *inner[1], *result;
	inner[0] = DeeObject_GetItemIndex(self->sm_seq, index);
	if unlikely(!inner[0])
		goto err;
	result = DeeObject_Call(self->sm_mapper, 1, inner);
	Dee_Decref(inner[0]);
	return result;
err:
	return NULL;
}

struct mapped_foreach_data {
	DeeObject    *tfd_fun;  /* [1..1] Mapper function. */
	Dee_foreach_t tfd_proc; /* [1..1] Inner callback. */
	void         *tfd_arg;  /* [?..?] Cookie for `tfd_proc'. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
mapped_foreach_cb(void *arg, DeeObject *elem) {
	Dee_ssize_t result;
	struct mapped_foreach_data *data;
	data = (struct mapped_foreach_data *)arg;
	elem = DeeObject_Call(data->tfd_fun, 1, &elem);
	if unlikely(!elem)
		goto err;
	result = (*data->tfd_proc)(data->tfd_arg, elem);
	Dee_Decref(elem);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
mapped_foreach(SeqMapped *self, Dee_foreach_t proc, void *arg) {
	struct mapped_foreach_data data;
	data.tfd_fun  = self->sm_mapper;
	data.tfd_proc = proc;
	data.tfd_arg  = arg;
	return DeeObject_Foreach(self->sm_seq, &mapped_foreach_cb, &data);
}

struct mapped_enumerate_data {
	DeeObject      *ted_fun;  /* [1..1] Mapper function. */
	Dee_enumerate_t ted_proc; /* [1..1] Inner callback. */
	void           *ted_arg;  /* [?..?] Cookie for `ted_proc'. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
mapped_enumerate_cb(void *arg, DeeObject *index, DeeObject *value) {
	Dee_ssize_t result;
	struct mapped_enumerate_data *data;
	data = (struct mapped_enumerate_data *)arg;
	if (!value)
		return (*data->ted_proc)(data->ted_arg, index, NULL);
	value = DeeObject_Call(data->ted_fun, 1, &value);
	if unlikely(!value)
		goto err;
	result = (*data->ted_proc)(data->ted_arg, index, value);
	Dee_Decref(value);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
mapped_enumerate(SeqMapped *self, Dee_enumerate_t proc, void *arg) {
	struct mapped_enumerate_data data;
	data.ted_fun  = self->sm_mapper;
	data.ted_proc = proc;
	data.ted_arg  = arg;
	return DeeObject_Enumerate(self->sm_seq, &mapped_enumerate_cb, &data);
}

struct mapped_enumerate_index_data {
	DeeObject            *teid_fun;  /* [1..1] Mapper function. */
	Dee_enumerate_index_t teid_proc; /* [1..1] Inner callback. */
	void                 *teid_arg;  /* [?..?] Cookie for `teid_proc'. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
mapped_enumerate_index_cb(void *arg, size_t index, DeeObject *value) {
	Dee_ssize_t result;
	struct mapped_enumerate_index_data *data;
	data = (struct mapped_enumerate_index_data *)arg;
	if (!value)
		return (*data->teid_proc)(data->teid_arg, index, NULL);
	value = DeeObject_Call(data->teid_fun, 1, &value);
	if unlikely(!value)
		goto err;
	result = (*data->teid_proc)(data->teid_arg, index, value);
	Dee_Decref(value);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
mapped_enumerate_index(SeqMapped *self, Dee_enumerate_index_t proc,
                      void *arg, size_t start, size_t end) {
	struct mapped_enumerate_index_data data;
	data.teid_fun  = self->sm_mapper;
	data.teid_proc = proc;
	data.teid_arg  = arg;
	return DeeObject_EnumerateIndex(self->sm_seq, &mapped_enumerate_index_cb, &data, start, end);
}

STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_bounditem       generic_proxy_bounditem
#define mapped_hasitem         generic_proxy_hasitem
#define mapped_bounditem_index generic_proxy_bounditem_index
#define mapped_hasitem_index   generic_proxy_hasitem_index

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_getrange_index(SeqMapped *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DeeObject *orig, *result;
	orig = DeeObject_GetRangeIndex(self->sm_seq, start, end);
	if unlikely(!orig)
		goto err;
	result = DeeSeq_Map(orig, self->sm_mapper);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_getrange_index_n(SeqMapped *self, Dee_ssize_t start) {
	DREF DeeObject *orig, *result;
	orig = DeeObject_GetRangeIndexN(self->sm_seq, start);
	if unlikely(!orig)
		goto err;
	result = DeeSeq_Map(orig, self->sm_mapper);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mapped_trygetitem(SeqMapped *self, DeeObject *index) {
	DREF DeeObject *result;
	result = DeeObject_TryGetItem(self->sm_seq, index);
	if (ITER_ISOK(result)) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->sm_mapper, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_trygetitem_index(SeqMapped *self, size_t index) {
	DREF DeeObject *result;
	result = DeeObject_TryGetItemIndex(self->sm_seq, index);
	if (ITER_ISOK(result)) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->sm_mapper, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mapped_trygetitem_string_hash(SeqMapped *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_TryGetItemStringHash(self->sm_seq, key, hash);
	if (ITER_ISOK(result)) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->sm_mapper, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mapped_getitem_string_hash(SeqMapped *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_GetItemStringHash(self->sm_seq, key, hash);
	if (result) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->sm_mapper, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_bounditem_string_hash generic_proxy_bounditem_string_hash
#define mapped_hasitem_string_hash   generic_proxy_hasitem_string_hash

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mapped_trygetitem_string_len_hash(SeqMapped *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_TryGetItemStringLenHash(self->sm_seq, key, keylen, hash);
	if (ITER_ISOK(result)) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->sm_mapper, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mapped_getitem_string_len_hash(SeqMapped *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_GetItemStringLenHash(self->sm_seq, key, keylen, hash);
	if (result) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->sm_mapper, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_bounditem_string_len_hash generic_proxy_bounditem_string_len_hash
#define mapped_hasitem_string_len_hash   generic_proxy_hasitem_string_len_hash

PRIVATE struct type_seq mapped_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mapped_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mapped_sizeob,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mapped_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&mapped_getrange,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&mapped_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_t, void *))&mapped_enumerate,
	/* .tp_enumerate_index            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_index_t, void *, size_t, size_t))&mapped_enumerate_index,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&mapped_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&mapped_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&mapped_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&mapped_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&mapped_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&mapped_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&mapped_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&mapped_getrange_index,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&mapped_getrange_index_n,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mapped_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&mapped_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&mapped_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&mapped_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&mapped_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&mapped_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&mapped_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&mapped_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&mapped_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&mapped_hasitem_string_len_hash,
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
mapped_ctor(SeqMapped *__restrict self) {
	self->sm_seq = Dee_EmptySeq;
	self->sm_mapper = Dee_None;
	Dee_Incref(Dee_EmptySeq);
	Dee_Incref(Dee_None);
	return 0;
}

STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(SeqMapped, sm_mapper) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqMapped, sm_mapper) == offsetof(ProxyObject2, po_obj2));
#define mapped_copy generic_proxy2_copy_alias12
#define mapped_deep generic_proxy2_deepcopy

STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject2, po_obj1));
STATIC_ASSERT(offsetof(SeqMapped, sm_mapper) == offsetof(ProxyObject2, po_obj2));
#define mapped_init generic_proxy2_init

INTERN DeeTypeObject SeqMapped_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqMapped",
	/* .tp_doc      = */ DOC("(seq:?DSequence,mapper:?DCallable)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&mapped_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&mapped_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&mapped_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&mapped_init,
				TYPE_FIXED_ALLOCATOR(SeqMapped)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mapped_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&mapped_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&mapped_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &mapped_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ mapped_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ mapped_class_members
};



INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_Map(DeeObject *self, DeeObject *mapper) {
	DREF SeqMapped *result;
	/* Create a new mapper sequence. */
	result = DeeObject_MALLOC(SeqMapped);
	if unlikely(!result)
		goto done;
	result->sm_seq = self;
	result->sm_mapper = mapper;
	Dee_Incref(self);
	Dee_Incref(mapper);
	DeeObject_Init(result, &SeqMapped_Type);
done:
	return (DREF DeeObject *)result;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_MAPPED_C */
