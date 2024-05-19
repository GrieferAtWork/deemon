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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_TRANSFORM_C
#define GUARD_DEEMON_OBJECTS_SEQ_TRANSFORM_C 1

#include "transform.h"

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

DECL_BEGIN

PRIVATE NONNULL((1)) void DCALL
transiter_fini(TransformationIterator *__restrict self) {
	Dee_Decref(self->ti_iter);
	Dee_Decref(self->ti_func);
}

PRIVATE NONNULL((1, 2)) void DCALL
transiter_visit(TransformationIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->ti_iter);
	Dee_Visit(self->ti_func);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
transiter_bool(TransformationIterator *__restrict self) {
	return DeeObject_Bool(self->ti_iter);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
transiter_next(TransformationIterator *__restrict self) {
	DREF DeeObject *result;
	result = DeeObject_IterNext(self->ti_iter);
	if (ITER_ISOK(result)) {
		DREF DeeObject *new_result;
		/* Invoke the transformation callback. */
		new_result = DeeObject_Call(self->ti_func, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
transiter_seq_get(TransformationIterator *__restrict self) {
	/* Forward access to this attribute to the pointed-to iterator. */
	DREF DeeObject *orig, *result;
	orig = DeeObject_GetAttr(self->ti_iter, (DeeObject *)&str_seq);
	if unlikely(!orig)
		goto err;
	result = DeeSeq_Transform(orig, self->ti_func);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}

PRIVATE struct type_getset tpconst transiter_getsets[] = {
	TYPE_GETTER_F(STR_seq, &transiter_seq_get, METHOD_FNOREFESCAPE, "->?Ert:SeqTransformation"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst transiter_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(TransformationIterator, ti_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(TransformationIterator, ti_func), "->?DCallable"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
transiter_hash(TransformationIterator *self) {
	return Dee_HashCombine(DeeObject_HashGeneric(self->ti_func),
	                       DeeObject_Hash(self->ti_iter));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
transiter_compare_eq(TransformationIterator *self, TransformationIterator *other) {
	if (DeeObject_AssertTypeExact(other, &SeqTransformationIterator_Type))
		goto err;
	if (self->ti_func != other->ti_func)
		return 1;
	return DeeObject_CompareEq(self->ti_iter, other->ti_iter);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
transiter_compare(TransformationIterator *self, TransformationIterator *other) {
	if (DeeObject_AssertTypeExact(other, &SeqTransformationIterator_Type))
		goto err;
	Dee_return_compare_if_ne(self->ti_func, other->ti_func);
	return DeeObject_Compare(self->ti_iter, other->ti_iter);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp transiter_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&transiter_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&transiter_compare_eq,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&transiter_compare,
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
transiter_copy(TransformationIterator *__restrict self,
               TransformationIterator *__restrict other) {
	self->ti_iter = DeeObject_Copy(other->ti_iter);
	if unlikely(!self->ti_iter)
		goto err;
	self->ti_func = other->ti_func;
	Dee_Incref(self->ti_func);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
transiter_deep(TransformationIterator *__restrict self,
               TransformationIterator *__restrict other) {
	self->ti_iter = DeeObject_Copy(other->ti_iter);
	if unlikely(!self->ti_iter)
		goto err;
	self->ti_func = DeeObject_Copy(other->ti_func);
	if unlikely(!self->ti_func)
		goto err_iter;
	return 0;
err_iter:
	Dee_Decref(self->ti_iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
transiter_ctor(TransformationIterator *__restrict self) {
	self->ti_iter = DeeObject_Iter(Dee_EmptySeq);
	if unlikely(!self->ti_iter)
		goto err;
	self->ti_func = Dee_None;
	Dee_Incref(Dee_None);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
transiter_init(TransformationIterator *__restrict self,
               size_t argc, DeeObject *const *argv) {
	Transformation *trans;
	if (DeeArg_Unpack(argc, argv, "o:_SeqTransformationIterator", &trans))
		goto err;
	if (DeeObject_AssertTypeExact(trans, &SeqTransformation_Type))
		goto err;
	self->ti_iter = DeeObject_Iter(trans->t_seq);
	if unlikely(!self->ti_iter)
		goto err;
	self->ti_func = trans->t_fun;
	Dee_Incref(self->ti_func);
	return 0;
err:
	return -1;
}

INTERN DeeTypeObject SeqTransformationIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqTransformationIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&transiter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&transiter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&transiter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&transiter_init,
				TYPE_FIXED_ALLOCATOR(TransformationIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&transiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&transiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&transiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &transiter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&transiter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ transiter_getsets,
	/* .tp_members       = */ transiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE NONNULL((1)) void DCALL
trans_fini(Transformation *__restrict self) {
	Dee_Decref(self->t_seq);
	Dee_Decref(self->t_fun);
}

PRIVATE NONNULL((1, 2)) void DCALL
trans_visit(Transformation *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->t_seq);
	Dee_Visit(self->t_fun);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
trans_bool(Transformation *__restrict self) {
	return DeeObject_Bool(self->t_seq);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
trans_iter(Transformation *__restrict self) {
	DREF TransformationIterator *result;
	result = DeeObject_MALLOC(TransformationIterator);
	if unlikely(!result)
		goto err;
	/* Create the underlying iterator. */
	result->ti_iter = DeeObject_Iter(self->t_seq);
	if unlikely(!result->ti_iter)
		goto err_r;
	/* Assign the transformation functions. */
	result->ti_func = self->t_fun;
	Dee_Incref(self->t_fun);
	DeeObject_Init(result, &SeqTransformationIterator_Type);
	return (DREF DeeObject *)result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE struct type_member tpconst trans_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(Transformation, t_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(Transformation, t_fun), "->?DCallable"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst trans_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqTransformationIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
trans_sizeob(Transformation *__restrict self) {
	return DeeObject_SizeOb(self->t_seq);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
trans_getitem(Transformation *self, DeeObject *index) {
	DREF DeeObject *orig, *result;
	orig = DeeObject_GetItem(self->t_seq, index);
	if unlikely(!orig)
		goto err;
	result = DeeObject_Call(self->t_fun, 1, &orig);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
trans_getrange(Transformation *self, DeeObject *start, DeeObject *end) {
	DREF DeeObject *orig, *result;
	orig = DeeObject_GetRange(self->t_seq, start, end);
	if unlikely(!orig)
		goto err;
	result = DeeSeq_Transform(orig, self->t_fun);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
trans_size(Transformation *__restrict self) {
	return DeeObject_Size(self->t_seq);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
trans_size_fast(Transformation *__restrict self) {
	return DeeObject_SizeFast(self->t_seq);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
trans_getitem_index(Transformation *__restrict self, size_t index) {
	DREF DeeObject *inner[1], *result;
	inner[0] = DeeObject_GetItemIndex(self->t_seq, index);
	if unlikely(!inner[0])
		goto err;
	result = DeeObject_Call(self->t_fun, 1, inner);
	Dee_Decref(inner[0]);
	return result;
err:
	return NULL;
}

struct trans_foreach_data {
	DeeObject    *tfd_fun;  /* [1..1] Function to call in order to apply transformation */
	Dee_foreach_t tfd_proc; /* [1..1] Inner callback. */
	void         *tfd_arg;  /* [?..?] Cookie for `tfd_proc'. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
trans_foreach_cb(void *arg, DeeObject *elem) {
	Dee_ssize_t result;
	struct trans_foreach_data *data;
	data = (struct trans_foreach_data *)arg;
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
trans_foreach(Transformation *self, Dee_foreach_t proc, void *arg) {
	struct trans_foreach_data data;
	data.tfd_fun  = self->t_fun;
	data.tfd_proc = proc;
	data.tfd_arg  = arg;
	return DeeObject_Foreach(self->t_seq, &trans_foreach_cb, &data);
}

struct trans_enumerate_data {
	DeeObject      *ted_fun;  /* [1..1] Function to call in order to apply transformation */
	Dee_enumerate_t ted_proc; /* [1..1] Inner callback. */
	void           *ted_arg;  /* [?..?] Cookie for `ted_proc'. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
trans_enumerate_cb(void *arg, DeeObject *index, DeeObject *value) {
	Dee_ssize_t result;
	struct trans_enumerate_data *data;
	data = (struct trans_enumerate_data *)arg;
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
trans_enumerate(Transformation *self, Dee_enumerate_t proc, void *arg) {
	struct trans_enumerate_data data;
	data.ted_fun  = self->t_fun;
	data.ted_proc = proc;
	data.ted_arg  = arg;
	return DeeObject_Enumerate(self->t_seq, &trans_enumerate_cb, &data);
}

struct trans_enumerate_index_data {
	DeeObject            *teid_fun;  /* [1..1] Function to call in order to apply transformation */
	Dee_enumerate_index_t teid_proc; /* [1..1] Inner callback. */
	void                 *teid_arg;  /* [?..?] Cookie for `teid_proc'. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
trans_enumerate_index_cb(void *arg, size_t index, DeeObject *value) {
	Dee_ssize_t result;
	struct trans_enumerate_index_data *data;
	data = (struct trans_enumerate_index_data *)arg;
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
trans_enumerate_index(Transformation *self, Dee_enumerate_index_t proc,
                      void *arg, size_t start, size_t end) {
	struct trans_enumerate_index_data data;
	data.teid_fun  = self->t_fun;
	data.teid_proc = proc;
	data.teid_arg  = arg;
	return DeeObject_EnumerateIndex(self->t_seq, &trans_enumerate_index_cb, &data, start, end);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
trans_bounditem(Transformation *self, DeeObject *index) {
	return DeeObject_BoundItem(self->t_seq, index);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
trans_hasitem(Transformation *self, DeeObject *index) {
	return DeeObject_HasItem(self->t_seq, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
trans_bounditem_index(Transformation *self, size_t index) {
	return DeeObject_BoundItemIndex(self->t_seq, index);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
trans_hasitem_index(Transformation *self, size_t index) {
	return DeeObject_HasItemIndex(self->t_seq, index);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
trans_getrange_index(Transformation *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DeeObject *orig, *result;
	orig = DeeObject_GetRangeIndex(self->t_seq, start, end);
	if unlikely(!orig)
		goto err;
	result = DeeSeq_Transform(orig, self->t_fun);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
trans_getrange_index_n(Transformation *self, Dee_ssize_t start) {
	DREF DeeObject *orig, *result;
	orig = DeeObject_GetRangeIndexN(self->t_seq, start);
	if unlikely(!orig)
		goto err;
	result = DeeSeq_Transform(orig, self->t_fun);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
trans_trygetitem(Transformation *self, DeeObject *index) {
	DREF DeeObject *result;
	result = DeeObject_TryGetItem(self->t_seq, index);
	if (ITER_ISOK(result)) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->t_fun, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
trans_trygetitem_index(Transformation *self, size_t index) {
	DREF DeeObject *result;
	result = DeeObject_TryGetItemIndex(self->t_seq, index);
	if (ITER_ISOK(result)) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->t_fun, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
trans_trygetitem_string_hash(Transformation *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_TryGetItemStringHash(self->t_seq, key, hash);
	if (ITER_ISOK(result)) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->t_fun, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
trans_getitem_string_hash(Transformation *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_GetItemStringHash(self->t_seq, key, hash);
	if (result) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->t_fun, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
trans_bounditem_string_hash(Transformation *self, char const *key, Dee_hash_t hash) {
	return DeeObject_BoundItemStringHash(self->t_seq, key, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
trans_hasitem_string_hash(Transformation *self, char const *key, Dee_hash_t hash) {
	return DeeObject_HasItemStringHash(self->t_seq, key, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
trans_trygetitem_string_len_hash(Transformation *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_TryGetItemStringLenHash(self->t_seq, key, keylen, hash);
	if (ITER_ISOK(result)) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->t_fun, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
trans_getitem_string_len_hash(Transformation *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result;
	result = DeeObject_GetItemStringLenHash(self->t_seq, key, keylen, hash);
	if (result) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->t_fun, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
trans_bounditem_string_len_hash(Transformation *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return DeeObject_BoundItemStringLenHash(self->t_seq, key, keylen, hash);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
trans_hasitem_string_len_hash(Transformation *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return DeeObject_HasItemStringLenHash(self->t_seq, key, keylen, hash);
}

PRIVATE struct type_nsi tpconst trans_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&trans_size,
			/* .nsi_getsize_fast = */ (dfunptr_t)NULL,
			/* .nsi_getitem      = */ (dfunptr_t)&trans_getitem_index,
		}
	}
};

PRIVATE struct type_seq trans_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&trans_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&trans_sizeob,
	/* .tp_contains                   = */ NULL,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&trans_getitem,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&trans_getrange,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ &trans_nsi,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&trans_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_t, void *))&trans_enumerate,
	/* .tp_enumerate_index            = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_index_t, void *, size_t, size_t))&trans_enumerate_index,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&trans_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&trans_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&trans_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&trans_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&trans_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&trans_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&trans_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&trans_getrange_index,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))&trans_getrange_index_n,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&trans_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&trans_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&trans_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&trans_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&trans_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&trans_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&trans_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&trans_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&trans_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&trans_hasitem_string_len_hash,
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
trans_ctor(Transformation *__restrict self) {
	self->t_seq = Dee_EmptySeq;
	self->t_fun = Dee_None;
	Dee_Incref(Dee_EmptySeq);
	Dee_Incref(Dee_None);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
trans_copy(Transformation *__restrict self,
           Transformation *__restrict other) {
	self->t_seq = other->t_seq;
	self->t_fun = other->t_fun;
	Dee_Incref(self->t_seq);
	Dee_Incref(self->t_fun);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
trans_deep(Transformation *__restrict self,
           Transformation *__restrict other) {
	self->t_seq = DeeObject_DeepCopy(other->t_seq);
	if unlikely(!self->t_seq)
		goto err;
	self->t_fun = DeeObject_DeepCopy(other->t_fun);
	if unlikely(!self->t_fun)
		goto err_seq;
	return 0;
err_seq:
	Dee_Decref(self->t_seq);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
trans_init(Transformation *__restrict self,
           size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "oo:_SeqTransformation", &self->t_seq, &self->t_fun))
		goto err;
	Dee_Incref(self->t_seq);
	Dee_Incref(self->t_fun);
	return 0;
err:
	return -1;
}

INTERN DeeTypeObject SeqTransformation_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqTransformation",
	/* .tp_doc      = */ DOC("(seq:?DSequence,fun:?DCallable)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&trans_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&trans_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&trans_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&trans_init,
				TYPE_FIXED_ALLOCATOR(Transformation)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&trans_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&trans_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&trans_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &trans_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ trans_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ trans_class_members
};



INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSeq_Transform(DeeObject *self,
                 DeeObject *transformation) {
	DREF Transformation *result;
	/* Create a new transformation sequence. */
	result = DeeObject_MALLOC(Transformation);
	if unlikely(!result)
		goto done;
	result->t_seq = self;
	result->t_fun = transformation;
	Dee_Incref(self);
	Dee_Incref(transformation);
	DeeObject_Init(result, &SeqTransformation_Type);
done:
	return (DREF DeeObject *)result;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_TRANSFORM_C */
