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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_MAPPED_C
#define GUARD_DEEMON_OBJECTS_SEQ_MAPPED_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_FREE, DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack1 */
#include <deemon/callable.h>           /* DeeFunctionComposition_Of */
#include <deemon/computed-operators.h>
#include <deemon/method-hints.h>       /* DeeObject_InvokeMethodHint, Dee_seq_enumerate_index_t, Dee_seq_enumerate_t, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/none.h>               /* DeeNone_NewRef */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_COMPARE_ERR, Dee_COMPARE_NE, Dee_Decref*, Dee_Incref, Dee_foreach_t, Dee_hash_t, Dee_return_compare_if_ne, Dee_ssize_t, Dee_visit_t, ITER_ISOK, OBJECT_HEAD_INIT, return_reference */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_NewEmpty, DeeSeq_Type, Dee_EmptySeq */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, METHOD_FNOREFESCAPE, STRUCT_OBJECT_AB, TF_NONE, TP_FFINAL, TP_FNORMAL, TYPE_*, type_* */
#include <deemon/util/hash.h>          /* DeeObject_HashGeneric, Dee_HashCombine */

#include "../../runtime/method-hint-defaults.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "mapped.h"

#include <stddef.h> /* NULL, offsetof, size_t */

#ifndef CONFIG_TINY_DEEMON
#define WANT_mapped_getitem_index
#define WANT_mapped_foreach
#define WANT_mapped_mh_seq_enumerate
#define WANT_mapped_mh_seq_enumerate_index
#define WANT_mapped_getrange_index
#define WANT_mapped_getrange_index_n
#define WANT_mapped_trygetitem
#define WANT_mapped_trygetitem_index
#define WANT_mapped_map
#define WANT_mapped_seq_getfirst
#define WANT_mapped_seq_getlast
#define WANT_mapped_set_getfirst
#define WANT_mapped_set_getlast
#define WANT_mapped_mh_seq_erase
#define WANT_mapped_mh_seq_clear
#define WANT_mapped_mh_seq_reverse
#define WANT_mapped_mh_set_operator_bool
#define WANT_mapped_mh_set_operator_size
#define WANT_mapped_mh_set_operator_sizeob
#define WANT_mapped_mh_map_operator_size
#define WANT_mapped_mh_seq_any
#define WANT_mapped_mh_seq_all
#define WANT_mapped_mh_seq_parity
#define WANT_mapped_mh_seq_sort
#define WANT_mapped_mh_seq_sorted
#define WANT_mapped_mh_seq_pop
#define WANT_mapped_mh_seq_reversed
#define WANT_mapped_mh_seq_unpack
#define WANT_mapped_mh_seq_unpack_ub
#define WANT_mapped_mh_set_pop
#define WANT_mapped_mh_map_popitem
#define WANT_mapped_mh_seq_sum
#define WANT_mapped_mh_seq_min
#define WANT_mapped_mh_seq_max
#define WANT_mapped_mh_seq_locate
#define WANT_mapped_mh_seq_rlocate
#define WANT_mapped_mh_seq_removeif
#define WANT_mapped_mh_seq_count
#define WANT_mapped_mh_seq_contains
#define WANT_mapped_mh_seq_startswith
#define WANT_mapped_mh_seq_endswith
#define WANT_mapped_mh_seq_find
#define WANT_mapped_mh_seq_rfind
#define WANT_mapped_mh_seq_remove
#define WANT_mapped_mh_seq_rremove
#define WANT_mapped_mh_seq_removeall
#define WANT_mapped_mh_seq_bfind
#define WANT_mapped_mh_seq_bposition
#define WANT_mapped_mh_seq_brange
#endif /* !CONFIG_TINY_DEEMON */

DECL_BEGIN

STATIC_ASSERT(offsetof(SeqMappedIterator, smi_iter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqMappedIterator, smi_iter) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(SeqMappedIterator, smi_mapper) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqMappedIterator, smi_mapper) == offsetof(ProxyObject2, po_obj2));
#define mappediter_fini  generic_proxy2__fini
#define mappediter_visit generic_proxy2__visit

STATIC_ASSERT(offsetof(SeqMappedIterator, smi_iter) == offsetof(ProxyObject, po_obj));
#define mappediter_bool generic_proxy__bool

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
	orig = DeeObject_GetAttr(self->smi_iter, Dee_AsObject(&str_seq));
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
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT_AB, offsetof(SeqMappedIterator, smi_iter), "->?DIterator"),
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT_AB, offsetof(SeqMappedIterator, smi_mapper), "->?DCallable"),
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
		return Dee_COMPARE_NE;
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
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

STATIC_ASSERT(offsetof(SeqMappedIterator, smi_iter) == offsetof(ProxyObject2, po_obj1));
STATIC_ASSERT(offsetof(SeqMappedIterator, smi_mapper) == offsetof(ProxyObject2, po_obj2));
#define mappediter_copy      generic_proxy2__copy_recursive1_alias2 /* copy "smi_iter", alias "smi_mapper" */
#define mappediter_serialize generic_proxy2__serialize

PRIVATE WUNUSED NONNULL((1)) int DCALL
mappediter_ctor(SeqMappedIterator *__restrict self) {
	self->smi_iter = DeeObject_Iter(Dee_EmptySeq);
	if unlikely(!self->smi_iter)
		goto err;
	self->smi_mapper = DeeNone_NewRef();
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mappediter_init(SeqMappedIterator *__restrict self,
               size_t argc, DeeObject *const *argv) {
	SeqMapped *trans;
	DeeArg_Unpack1(err, argc, argv, "_SeqMappedIterator", &trans);
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqMappedIterator,
			/* tp_ctor:        */ &mappediter_ctor,
			/* tp_copy_ctor:   */ &mappediter_copy,
			/* tp_any_ctor:    */ &mappediter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &mappediter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mappediter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&mappediter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&mappediter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &mappediter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mappediter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ mappediter_getsets,
	/* .tp_members       = */ mappediter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(SeqMapped, sm_mapper) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqMapped, sm_mapper) == offsetof(ProxyObject2, po_obj2));
#define mapped_fini  generic_proxy2__fini
#define mapped_visit generic_proxy2__visit

STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_bool             generic_proxy__bool
#define mapped_size_fast        generic_proxy__size_fast
#define mapped_sizeob           generic_proxy__seq_operator_sizeob
#define mapped_size             generic_proxy__seq_operator_size
#define mapped_delitem          generic_proxy__seq_operator_delitem
#define mapped_delrange         generic_proxy__seq_operator_delrange
#define mapped_bounditem        generic_proxy__seq_operator_bounditem
#define mapped_bounditem_index  generic_proxy__seq_operator_bounditem_index
#define mapped_hasitem          generic_proxy__seq_operator_hasitem
#define mapped_hasitem_index    generic_proxy__seq_operator_hasitem_index
#define mapped_delitem_index    generic_proxy__seq_operator_delitem_index
#define mapped_delrange_index   generic_proxy__seq_operator_delrange_index
#define mapped_delrange_index_n generic_proxy__seq_operator_delrange_index_n


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_iter(SeqMapped *__restrict self) {
	DREF SeqMappedIterator *result;
	result = DeeObject_MALLOC(SeqMappedIterator);
	if unlikely(!result)
		goto err;

	/* Create the underlying iterator. */
	result->smi_iter = DeeObject_InvokeMethodHint(seq_operator_iter, self->sm_seq);
	if unlikely(!result->smi_iter)
		goto err_r;

	/* Assign the mapper functions. */
	result->smi_mapper = self->sm_mapper;
	Dee_Incref(self->sm_mapper);
	DeeObject_Init(result, &SeqMappedIterator_Type);
	return Dee_AsObject(result);
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}

PRIVATE struct type_member tpconst mapped_members[] = {
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT_AB, offsetof(SeqMapped, sm_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__mapper__", STRUCT_OBJECT_AB, offsetof(SeqMapped, sm_mapper), "->?DCallable"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst mapped_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SeqMappedIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mapped_getitem(SeqMapped *self, DeeObject *index) {
	DREF DeeObject *orig, *result;
	orig = DeeObject_InvokeMethodHint(seq_operator_getitem, self->sm_seq, index);
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
	orig = DeeObject_InvokeMethodHint(seq_operator_getrange, self->sm_seq, start, end);
	if unlikely(!orig)
		goto err;
	result = DeeSeq_Map(orig, self->sm_mapper);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}


#ifdef WANT_mapped_getitem_index
#define PTR_mapped_getitem_index &mapped_getitem_index
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_getitem_index(SeqMapped *__restrict self, size_t index) {
	DREF DeeObject *inner[1], *result;
	inner[0] = DeeObject_InvokeMethodHint(seq_operator_getitem_index, self->sm_seq, index);
	if unlikely(!inner[0])
		goto err;
	result = DeeObject_Call(self->sm_mapper, 1, inner);
	Dee_Decref(inner[0]);
	return result;
err:
	return NULL;
}
#else /* WANT_mapped_getitem_index */
#define PTR_mapped_getitem_index NULL
#endif /* !WANT_mapped_getitem_index */

#ifdef WANT_mapped_foreach
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

#define PTR_mapped_foreach &mapped_foreach
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
mapped_foreach(SeqMapped *self, Dee_foreach_t proc, void *arg) {
	struct mapped_foreach_data data;
	data.tfd_fun  = self->sm_mapper;
	data.tfd_proc = proc;
	data.tfd_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_operator_foreach, self->sm_seq, &mapped_foreach_cb, &data);
}
#else /* WANT_mapped_foreach */
#define PTR_mapped_foreach DEFIMPL(&default__seq_operator_foreach__with__seq_operator_iter)
#endif /* !WANT_mapped_foreach */


#ifdef WANT_mapped_mh_seq_enumerate
#define NEED_mapped_methods
#define NEED_mapped_method_hints
struct mapped_mh_seq_enumerate_data {
	DeeObject          *ted_fun;  /* [1..1] Mapper function. */
	Dee_seq_enumerate_t ted_proc; /* [1..1] Inner callback. */
	void               *ted_arg;  /* [?..?] Cookie for `ted_proc'. */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
mapped_mh_seq_enumerate_cb(void *arg, DeeObject *index, DeeObject *value) {
	Dee_ssize_t result;
	struct mapped_mh_seq_enumerate_data *data;
	data = (struct mapped_mh_seq_enumerate_data *)arg;
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
mapped_mh_seq_enumerate(SeqMapped *self, Dee_seq_enumerate_t proc, void *arg) {
	struct mapped_mh_seq_enumerate_data data;
	data.ted_fun  = self->sm_mapper;
	data.ted_proc = proc;
	data.ted_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_enumerate, self->sm_seq, &mapped_mh_seq_enumerate_cb, &data);
}
#endif /* !WANT_mapped_mh_seq_enumerate */


#ifdef WANT_mapped_mh_seq_enumerate_index
#define NEED_mapped_methods
#define NEED_mapped_method_hints
struct mapped_mh_seq_enumerate_index_data {
	DeeObject                *teid_fun;  /* [1..1] Mapper function. */
	Dee_seq_enumerate_index_t teid_proc; /* [1..1] Inner callback. */
	void                     *teid_arg;  /* [?..?] Cookie for `teid_proc'. */
};

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
mapped_mh_seq_enumerate_index_cb(void *arg, size_t index, DeeObject *value) {
	Dee_ssize_t result;
	struct mapped_mh_seq_enumerate_index_data *data;
	data = (struct mapped_mh_seq_enumerate_index_data *)arg;
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
mapped_mh_seq_enumerate_index(SeqMapped *self, Dee_seq_enumerate_index_t proc,
                              void *arg, size_t start, size_t end) {
	struct mapped_mh_seq_enumerate_index_data data;
	data.teid_fun  = self->sm_mapper;
	data.teid_proc = proc;
	data.teid_arg  = arg;
	return DeeObject_InvokeMethodHint(seq_enumerate_index, self->sm_seq, &mapped_mh_seq_enumerate_index_cb, &data, start, end);
}
#endif /* WANT_mapped_mh_seq_enumerate_index */


#ifdef WANT_mapped_getrange_index
#define PTR_mapped_getrange_index &mapped_getrange_index
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_getrange_index(SeqMapped *self, Dee_ssize_t start, Dee_ssize_t end) {
	DREF DeeObject *orig, *result;
	orig = DeeObject_InvokeMethodHint(seq_operator_getrange_index, self->sm_seq, start, end);
	if unlikely(!orig)
		goto err;
	result = DeeSeq_Map(orig, self->sm_mapper);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}
#else /* WANT_mapped_getrange_index */
#define PTR_mapped_getrange_index NULL
#endif /* !WANT_mapped_getrange_index */

#ifdef WANT_mapped_getrange_index_n
#define PTR_mapped_getrange_index_n &mapped_getrange_index_n
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_getrange_index_n(SeqMapped *self, Dee_ssize_t start) {
	DREF DeeObject *orig, *result;
	orig = DeeObject_InvokeMethodHint(seq_operator_getrange_index_n, self->sm_seq, start);
	if unlikely(!orig)
		goto err;
	result = DeeSeq_Map(orig, self->sm_mapper);
	Dee_Decref(orig);
	return result;
err:
	return NULL;
}
#else /* WANT_mapped_getrange_index_n */
#define PTR_mapped_getrange_index_n NULL
#endif /* !WANT_mapped_getrange_index_n */

#ifdef WANT_mapped_trygetitem
#define PTR_mapped_trygetitem &mapped_trygetitem
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mapped_trygetitem(SeqMapped *self, DeeObject *index) {
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(seq_operator_trygetitem, self->sm_seq, index);
	if (ITER_ISOK(result)) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->sm_mapper, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}
#else /* WANT_mapped_trygetitem */
#define PTR_mapped_trygetitem NULL
#endif /* !WANT_mapped_trygetitem */

#ifdef WANT_mapped_trygetitem_index
#define PTR_mapped_trygetitem_index &mapped_trygetitem_index
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_trygetitem_index(SeqMapped *self, size_t index) {
	DREF DeeObject *result;
	result = DeeObject_InvokeMethodHint(seq_operator_trygetitem_index, self->sm_seq, index);
	if (ITER_ISOK(result)) {
		DREF DeeObject *new_result;
		new_result = DeeObject_Call(self->sm_mapper, 1, &result);
		Dee_Decref(result);
		result = new_result;
	}
	return result;
}
#else /* WANT_mapped_trygetitem_index */
#define PTR_mapped_trygetitem_index NULL
#endif /* !WANT_mapped_trygetitem_index */


PRIVATE struct type_seq mapped_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mapped_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mapped_sizeob,
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mapped_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&mapped_delitem,
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&mapped_getrange,
	/* .tp_delrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&mapped_delrange,
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))PTR_mapped_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&mapped_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&mapped_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&mapped_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&mapped_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))PTR_mapped_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&mapped_delitem_index,
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&mapped_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&mapped_hasitem_index,
	/* .tp_getrange_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))PTR_mapped_getrange_index,
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&mapped_delrange_index,
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ (DREF DeeObject *(DCALL *)(DeeObject *, Dee_ssize_t))PTR_mapped_getrange_index_n,
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&mapped_delrange_index_n,
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))PTR_mapped_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))PTR_mapped_trygetitem_index,
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
mapped_ctor(SeqMapped *__restrict self) {
	self->sm_seq    = DeeSeq_NewEmpty();
	self->sm_mapper = DeeNone_NewRef();
	return 0;
}

STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(SeqMapped, sm_mapper) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(SeqMapped, sm_mapper) == offsetof(ProxyObject2, po_obj2));
#define mapped_copy      generic_proxy2__copy_alias12
#define mapped_serialize generic_proxy2__serialize

STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject2, po_obj1));
STATIC_ASSERT(offsetof(SeqMapped, sm_mapper) == offsetof(ProxyObject2, po_obj2));
#define mapped_init generic_proxy2__init

/* Return the composition of "key" being applied on-top of "self" */
LOCAL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mapped_compose(SeqMapped *self, DeeObject *key) {
	DeeObject *mappers[2];
	mappers[0] = key;
	mappers[1] = self->sm_seq;
	return DeeFunctionComposition_Of(2, mappers);
}


#ifdef WANT_mapped_map
#define NEED_mapped_methods
PRIVATE WUNUSED NONNULL((1)) DREF SeqMapped *DCALL
mapped_map(SeqMapped *self, size_t argc, DeeObject *const *argv) {
	DREF SeqMapped *result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("map", params: "
	mapper:?DCallable
", docStringPrefix: "mapped");]]]*/
#define mapped_map_params "mapper:?DCallable"
	struct {
		DeeObject *mapper;
	} args;
	DeeArg_Unpack1(err, argc, argv, "map", &args.mapper);
/*[[[end]]]*/
	result = DeeObject_MALLOC(SeqMapped);
	if unlikely(!result)
		goto err;
	result->sm_mapper = mapped_compose(self, args.mapper); /* Inherit reference */
	if unlikely(!result->sm_mapper)
		goto err_r;
	result->sm_seq = self->sm_seq;
	Dee_Incref(result->sm_seq);
	DeeObject_Init(result, &SeqMapped_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}
#endif /* WANT_mapped_map */


#ifdef WANT_mapped_seq_getfirst
#define NEED_mapped_getsets
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_seq_getfirst(SeqMapped *__restrict self) {
	DREF DeeObject *result, *mapped;
	result = DeeObject_InvokeMethodHint(seq_getfirst, self->sm_seq);
	if unlikely(!result)
		goto err;
	mapped = DeeObject_Call(self->sm_mapper, 1, &result);
	Dee_Decref(result);
	return mapped;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_seq_trygetfirst(SeqMapped *__restrict self) {
	DREF DeeObject *result, *mapped;
	result = DeeObject_InvokeMethodHint(seq_trygetfirst, self->sm_seq);
	if unlikely(!ITER_ISOK(result))
		return result;
	mapped = DeeObject_Call(self->sm_mapper, 1, &result);
	Dee_Decref(result);
	return mapped;
}

STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_seq_boundfirst generic_proxy__seq_boundfirst
#define mapped_seq_delfirst   generic_proxy__seq_delfirst
#endif /* WANT_mapped_seq_getfirst */


#ifdef WANT_mapped_seq_getlast
#define NEED_mapped_getsets
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_seq_getlast(SeqMapped *__restrict self) {
	DREF DeeObject *result, *mapped;
	result = DeeObject_InvokeMethodHint(seq_getlast, self->sm_seq);
	if unlikely(!result)
		goto err;
	mapped = DeeObject_Call(self->sm_mapper, 1, &result);
	Dee_Decref(result);
	return mapped;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_seq_trygetlast(SeqMapped *__restrict self) {
	DREF DeeObject *result, *mapped;
	result = DeeObject_InvokeMethodHint(seq_trygetlast, self->sm_seq);
	if unlikely(!ITER_ISOK(result))
		return result;
	mapped = DeeObject_Call(self->sm_mapper, 1, &result);
	Dee_Decref(result);
	return mapped;
}

STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_seq_boundlast generic_proxy__seq_boundlast
#define mapped_seq_dellast   generic_proxy__seq_dellast
#endif /* WANT_mapped_seq_getfirst */


#ifdef WANT_mapped_set_getfirst
#define NEED_mapped_getsets
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_set_getfirst(SeqMapped *__restrict self) {
	DREF DeeObject *result, *mapped;
	result = DeeObject_InvokeMethodHint(set_getfirst, self->sm_seq);
	if unlikely(!result)
		goto err;
	mapped = DeeObject_Call(self->sm_mapper, 1, &result);
	Dee_Decref(result);
	return mapped;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_set_trygetfirst(SeqMapped *__restrict self) {
	DREF DeeObject *result, *mapped;
	result = DeeObject_InvokeMethodHint(set_trygetfirst, self->sm_seq);
	if unlikely(!ITER_ISOK(result))
		return result;
	mapped = DeeObject_Call(self->sm_mapper, 1, &result);
	Dee_Decref(result);
	return mapped;
}

STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_set_boundfirst generic_proxy__set_boundfirst
#define mapped_set_delfirst   generic_proxy__set_delfirst
#endif /* WANT_mapped_set_getfirst */


#ifdef WANT_mapped_set_getlast
#define NEED_mapped_getsets
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_set_getlast(SeqMapped *__restrict self) {
	DREF DeeObject *result, *mapped;
	result = DeeObject_InvokeMethodHint(set_getlast, self->sm_seq);
	if unlikely(!result)
		goto err;
	mapped = DeeObject_Call(self->sm_mapper, 1, &result);
	Dee_Decref(result);
	return mapped;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_set_trygetlast(SeqMapped *__restrict self) {
	DREF DeeObject *result, *mapped;
	result = DeeObject_InvokeMethodHint(set_trygetlast, self->sm_seq);
	if unlikely(!ITER_ISOK(result))
		return result;
	mapped = DeeObject_Call(self->sm_mapper, 1, &result);
	Dee_Decref(result);
	return mapped;
}

STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_set_boundlast generic_proxy__set_boundlast
#define mapped_set_dellast   generic_proxy__set_dellast
#endif /* WANT_mapped_set_getlast */


#ifdef WANT_mapped_mh_seq_erase
#define NEED_mapped_methods
#define NEED_mapped_method_hints
STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_mh_seq_erase generic_proxy__seq_erase
#endif /* WANT_mapped_mh_seq_erase */

#ifdef WANT_mapped_mh_seq_clear
#define NEED_mapped_methods
#define NEED_mapped_method_hints
STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_mh_seq_clear generic_proxy__seq_clear
#endif /* WANT_mapped_mh_seq_clear */

#ifdef WANT_mapped_mh_seq_reverse
#define NEED_mapped_methods
#define NEED_mapped_method_hints
STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_mh_seq_reverse generic_proxy__seq_reverse
#endif /* WANT_mapped_mh_seq_reverse */

#ifdef WANT_mapped_mh_set_operator_bool
#define NEED_mapped_methods
#define NEED_mapped_method_hints
STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_mh_set_operator_bool generic_proxy__set_operator_bool
#endif /* WANT_mapped_mh_set_operator_bool */

#ifdef WANT_mapped_mh_set_operator_size
#define NEED_mapped_methods
#define NEED_mapped_method_hints
STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_mh_set_operator_size   generic_proxy__set_operator_size
#define mapped_mh_set_operator_sizeob generic_proxy__set_operator_sizeob
#endif /* WANT_mapped_mh_set_operator_size */

#ifdef WANT_mapped_mh_map_operator_size
#define NEED_mapped_methods
#define NEED_mapped_method_hints
STATIC_ASSERT(offsetof(SeqMapped, sm_seq) == offsetof(ProxyObject, po_obj));
#define mapped_mh_map_operator_size   generic_proxy__map_operator_size
#define mapped_mh_map_operator_sizeob generic_proxy__map_operator_sizeob
#endif /* WANT_mapped_mh_map_operator_size */

#ifdef WANT_mapped_mh_seq_any
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1)) int DCALL
mapped_mh_seq_any(SeqMapped *__restrict self) {
	return DeeObject_InvokeMethodHint(seq_any_with_key, self->sm_seq, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mapped_mh_seq_any_with_range(SeqMapped *__restrict self, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_any_with_range_and_key, self->sm_seq,
	                                  start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mapped_mh_seq_any_with_key(SeqMapped *self, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_any_with_key, self->sm_seq, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
mapped_mh_seq_any_with_range_and_key(SeqMapped *self, size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_any_with_range_and_key, self->sm_seq,
	                                    start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}
#endif /* WANT_mapped_mh_seq_any */


#ifdef WANT_mapped_mh_seq_all
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1)) int DCALL
mapped_mh_seq_all(SeqMapped *__restrict self) {
	return DeeObject_InvokeMethodHint(seq_all_with_key, self->sm_seq, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mapped_mh_seq_all_with_range(SeqMapped *__restrict self, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_all_with_range_and_key, self->sm_seq,
	                                  start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mapped_mh_seq_all_with_key(SeqMapped *self, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_all_with_key, self->sm_seq, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
mapped_mh_seq_all_with_range_and_key(SeqMapped *self, size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_all_with_range_and_key, self->sm_seq,
	                                    start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}
#endif /* WANT_mapped_mh_seq_all */


#ifdef WANT_mapped_mh_seq_parity
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1)) int DCALL
mapped_mh_seq_parity(SeqMapped *__restrict self) {
	return DeeObject_InvokeMethodHint(seq_parity_with_key, self->sm_seq, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mapped_mh_seq_parity_with_range(SeqMapped *__restrict self, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_parity_with_range_and_key, self->sm_seq,
	                                  start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mapped_mh_seq_parity_with_key(SeqMapped *self, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_parity_with_key, self->sm_seq, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
mapped_mh_seq_parity_with_range_and_key(SeqMapped *self, size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_parity_with_range_and_key, self->sm_seq,
	                                    start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}
#endif /* WANT_mapped_mh_seq_parity */


#ifdef WANT_mapped_mh_seq_sort
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1)) int DCALL
mapped_mh_seq_sort(SeqMapped *__restrict self, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_sort_with_key, self->sm_seq,
	                                  start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
mapped_mh_seq_sort_with_key(SeqMapped *self, size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_sort_with_key, self->sm_seq, start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}
#endif /* WANT_mapped_mh_seq_sort */


#ifdef WANT_mapped_mh_seq_sorted
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_mh_seq_sorted(SeqMapped *__restrict self, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_sorted_with_key, self->sm_seq,
	                                  start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
mapped_mh_seq_sorted_with_key(SeqMapped *self, size_t start, size_t end, DeeObject *key) {
	DREF DeeObject *result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_sorted_with_key, self->sm_seq, start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return NULL;
}
#endif /* WANT_mapped_mh_seq_sorted */


#ifdef WANT_mapped_mh_seq_pop
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_mh_seq_pop(SeqMapped *self, Dee_ssize_t index) {
	DREF DeeObject *item, *result;
	item = DeeObject_InvokeMethodHint(seq_pop, self->sm_seq, index);
	if unlikely(!item)
		goto err;
	result = DeeObject_Call(self->sm_mapper, 1, &item);
	Dee_Decref(item);
	return result;
err:
	return NULL;
}
#endif /* WANT_mapped_mh_seq_pop */


#ifdef WANT_mapped_mh_seq_reversed
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1)) DREF SeqMapped *DCALL
mapped_mh_seq_reversed(SeqMapped *__restrict self, size_t start, size_t end) {
	DREF DeeObject *seq;
	DREF SeqMapped *result = DeeObject_MALLOC(SeqMapped);
	if unlikely(!result)
		goto err;
	seq = DeeObject_InvokeMethodHint(seq_reversed, self->sm_seq, start, end);
	if unlikely(!seq)
		goto err_r;
	result->sm_mapper = self->sm_mapper;
	Dee_Incref(result->sm_mapper);
	result->sm_seq = seq; /* Inherit reference */
	DeeObject_Init(result, &SeqMapped_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}
#endif /* WANT_mapped_mh_seq_reversed */


#ifdef WANT_mapped_mh_seq_unpack
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
mapped_mapv_inherited(SeqMapped *__restrict self, size_t count,
                      /*inherit(on_failure)*/ DREF DeeObject *result[]) {
	size_t i;
	for (i = 0; i < count; ++i) {
		DREF DeeObject *mapped;
		mapped = DeeObject_Call(self->sm_mapper, 1, &result[i]);
		if unlikely(!mapped)
			goto err_r;
		Dee_Decref(result[i]);
		result[i] = mapped; /* Inherit reference */
	}
	return 0;
err_r:
	Dee_Decrefv(result, count);
/*err:*/
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
mapped_mh_seq_unpack(SeqMapped *__restrict self, size_t count,
                     DREF DeeObject *result[]) {
	if unlikely(DeeObject_InvokeMethodHint(seq_unpack, self->sm_seq, count, result))
		goto err;
	return mapped_mapv_inherited(self, count, result);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
mapped_mh_seq_unpack_ex(SeqMapped *__restrict self, size_t min_count,
                        size_t max_count, DREF DeeObject *result[]) {
	size_t count;
	count = DeeObject_InvokeMethodHint(seq_unpack_ex, self->sm_seq, min_count, max_count, result);
	if likely(count != (size_t)-1) {
		if unlikely(mapped_mapv_inherited(self, count, result))
			count = (size_t)-1;
	}
	return count;
}
#endif /* WANT_mapped_mh_seq_unpack */


#ifdef WANT_mapped_mh_seq_unpack_ub
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
mapped_mh_seq_unpack_ub(SeqMapped *__restrict self, size_t min_count,
                        size_t max_count, DREF DeeObject *result[]) {
	size_t i, count;
	count = DeeObject_InvokeMethodHint(seq_unpack_ub, self->sm_seq, min_count, max_count, result);
	if unlikely(count == (size_t)-1)
		goto err;
	for (i = 0; i < count; ++i) {
		if (result[i]) {
			DREF DeeObject *mapped;
			mapped = DeeObject_Call(self->sm_mapper, 1, &result[i]);
			if unlikely(!mapped)
				goto err_r;
			Dee_Decref(result[i]);
			result[i] = mapped; /* Inherit reference */
		}
	}
	return count;
err_r:
	Dee_Decrefv(result, count);
err:
	return (size_t)-1;
}
#endif /* WANT_mapped_mh_seq_unpack_ub */


#ifdef WANT_mapped_mh_set_pop
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_mh_set_pop(SeqMapped *__restrict self) {
	DREF DeeObject *item, *result;
	item = DeeObject_InvokeMethodHint(set_pop, self->sm_seq);
	if unlikely(!item)
		goto err;
	result = DeeObject_Call(self->sm_mapper, 1, &item);
	Dee_Decref(item);
	return result;
err:
	return NULL;
}

PRIVATE DeeObject mapped_mh_set_pop_dummy = { OBJECT_HEAD_INIT(&DeeObject_Type) };

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mapped_mh_set_pop_with_default(SeqMapped *self, DeeObject *default_) {
	DREF DeeObject *item, *result;
	item = DeeObject_InvokeMethodHint(set_pop_with_default, self->sm_seq, &mapped_mh_set_pop_dummy);
	if unlikely(!item)
		goto err;
	if (item == &mapped_mh_set_pop_dummy) {
		Dee_DecrefNokill(&mapped_mh_set_pop_dummy);
		return_reference(default_);
	}
	result = DeeObject_Call(self->sm_mapper, 1, &item);
	Dee_Decref(item);
	return result;
err:
	return NULL;
}
#endif /* WANT_mapped_mh_set_pop */


#ifdef WANT_mapped_mh_map_popitem
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mapped_mh_map_popitem(SeqMapped *__restrict self) {
	DREF DeeObject *item, *result;
	item = DeeObject_InvokeMethodHint(map_popitem, self->sm_seq);
	if unlikely(!item)
		goto err;
	result = DeeObject_Call(self->sm_mapper, 1, &item);
	Dee_Decref(item);
	return result;
err:
	return NULL;
}
#endif /* WANT_mapped_mh_map_popitem */


#ifdef WANT_mapped_mh_seq_sum
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mapped_mh_seq_sum(SeqMapped *self, DeeObject *def) {
	return DeeObject_InvokeMethodHint(seq_sum_with_key, self->sm_seq, def, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
mapped_mh_seq_sum_with_range(SeqMapped *__restrict self, size_t start, size_t end, DeeObject *def) {
	return DeeObject_InvokeMethodHint(seq_sum_with_range_and_key, self->sm_seq,
	                                  start, end, def, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
mapped_mh_seq_sum_with_key(SeqMapped *self, DeeObject *def, DeeObject *key) {
	DREF DeeObject *result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_sum_with_key, self->sm_seq, def, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL
mapped_mh_seq_sum_with_range_and_key(SeqMapped *self, size_t start, size_t end, DeeObject *def, DeeObject *key) {
	DREF DeeObject *result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_sum_with_range_and_key, self->sm_seq,
	                                    start, end, def, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return NULL;
}
#endif /* WANT_mapped_mh_seq_sum */


#ifdef WANT_mapped_mh_seq_min
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mapped_mh_seq_min(SeqMapped *self, DeeObject *def) {
	return DeeObject_InvokeMethodHint(seq_min_with_key, self->sm_seq, def, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
mapped_mh_seq_min_with_range(SeqMapped *__restrict self, size_t start, size_t end, DeeObject *def) {
	return DeeObject_InvokeMethodHint(seq_min_with_range_and_key, self->sm_seq,
	                                  start, end, def, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
mapped_mh_seq_min_with_key(SeqMapped *self, DeeObject *def, DeeObject *key) {
	DREF DeeObject *result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_min_with_key, self->sm_seq, def, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL
mapped_mh_seq_min_with_range_and_key(SeqMapped *self, size_t start, size_t end, DeeObject *def, DeeObject *key) {
	DREF DeeObject *result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_min_with_range_and_key, self->sm_seq,
	                                    start, end, def, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return NULL;
}
#endif /* WANT_mapped_mh_seq_min */


#ifdef WANT_mapped_mh_seq_max
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mapped_mh_seq_max(SeqMapped *self, DeeObject *def) {
	return DeeObject_InvokeMethodHint(seq_max_with_key, self->sm_seq, def, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL
mapped_mh_seq_max_with_range(SeqMapped *__restrict self, size_t start, size_t end, DeeObject *def) {
	return DeeObject_InvokeMethodHint(seq_max_with_range_and_key, self->sm_seq,
	                                  start, end, def, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
mapped_mh_seq_max_with_key(SeqMapped *self, DeeObject *def, DeeObject *key) {
	DREF DeeObject *result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_max_with_key, self->sm_seq, def, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 4, 5)) DREF DeeObject *DCALL
mapped_mh_seq_max_with_range_and_key(SeqMapped *self, size_t start, size_t end, DeeObject *def, DeeObject *key) {
	DREF DeeObject *result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_max_with_range_and_key, self->sm_seq,
	                                    start, end, def, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return NULL;
}
#endif /* WANT_mapped_mh_seq_max */


#ifdef WANT_mapped_mh_seq_locate
#define NEED_mapped_methods
#define NEED_mapped_method_hints

#ifdef WANT_mapped_mh_set_pop
#define mapped_mh_seq_locate_dummy mapped_mh_set_pop_dummy
#else /* WANT_mapped_mh_set_pop */
PRIVATE DeeObject mapped_mh_seq_locate_dummy = { OBJECT_HEAD_INIT(&DeeObject_Type) };
#endif /* !WANT_mapped_mh_set_pop */

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
mapped_mh_seq_locate(SeqMapped *self, DeeObject *match, DeeObject *def) {
	DREF DeeObject *result, *mapped;
	DREF DeeObject *composition = mapped_compose(self, match);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_locate, self->sm_seq, composition,
	                                    &mapped_mh_seq_locate_dummy);
	Dee_Decref_likely(composition);
	if unlikely(!result)
		goto err;
	if (result == &mapped_mh_seq_locate_dummy) {
		Dee_DecrefNokill(&mapped_mh_seq_locate_dummy);
		return_reference(def);
	}
	mapped = DeeObject_Call(self->sm_mapper, 1, &result);
	Dee_Decref(result);
	return mapped;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
mapped_mh_seq_locate_with_range(SeqMapped *self, DeeObject *match,
                                size_t start, size_t end, DeeObject *def) {
	DREF DeeObject *result, *mapped;
	DREF DeeObject *composition = mapped_compose(self, match);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_locate_with_range, self->sm_seq, composition,
	                                    start, end, &mapped_mh_seq_locate_dummy);
	Dee_Decref_likely(composition);
	if unlikely(!result)
		goto err;
	if (result == &mapped_mh_seq_locate_dummy) {
		Dee_DecrefNokill(&mapped_mh_seq_locate_dummy);
		return_reference(def);
	}
	mapped = DeeObject_Call(self->sm_mapper, 1, &result);
	Dee_Decref(result);
	return mapped;
err:
	return NULL;
}
#endif /* WANT_mapped_mh_seq_locate */


#ifdef WANT_mapped_mh_seq_rlocate
#define NEED_mapped_methods
#define NEED_mapped_method_hints

#ifdef WANT_mapped_mh_set_pop
#define mapped_mh_seq_rlocate_dummy mapped_mh_set_pop_dummy
#elif defined(WANT_mapped_mh_seq_locate)
#define mapped_mh_seq_rlocate_dummy mapped_mh_seq_locate_dummy
#else /* ... */
PRIVATE DeeObject mapped_mh_seq_rlocate_dummy = { OBJECT_HEAD_INIT(&DeeObject_Type) };
#endif /* !... */

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
mapped_mh_seq_rlocate(SeqMapped *self, DeeObject *match, DeeObject *def) {
	DREF DeeObject *result, *mapped;
	DREF DeeObject *composition = mapped_compose(self, match);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_rlocate, self->sm_seq, composition,
	                                    &mapped_mh_seq_rlocate_dummy);
	Dee_Decref_likely(composition);
	if unlikely(!result)
		goto err;
	if (result == &mapped_mh_seq_rlocate_dummy) {
		Dee_DecrefNokill(&mapped_mh_seq_rlocate_dummy);
		return_reference(def);
	}
	mapped = DeeObject_Call(self->sm_mapper, 1, &result);
	Dee_Decref(result);
	return mapped;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
mapped_mh_seq_rlocate_with_range(SeqMapped *self, DeeObject *match,
                                 size_t start, size_t end, DeeObject *def) {
	DREF DeeObject *result, *mapped;
	DREF DeeObject *composition = mapped_compose(self, match);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_rlocate_with_range, self->sm_seq, composition,
	                                    start, end, &mapped_mh_seq_rlocate_dummy);
	Dee_Decref_likely(composition);
	if unlikely(!result)
		goto err;
	if (result == &mapped_mh_seq_rlocate_dummy) {
		Dee_DecrefNokill(&mapped_mh_seq_rlocate_dummy);
		return_reference(def);
	}
	mapped = DeeObject_Call(self->sm_mapper, 1, &result);
	Dee_Decref(result);
	return mapped;
err:
	return NULL;
}
#endif /* WANT_mapped_mh_seq_rlocate */


#ifdef WANT_mapped_mh_seq_removeif
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
mapped_mh_seq_removeif(SeqMapped *self, DeeObject *should, size_t start, size_t end, size_t max) {
	size_t result;
	DREF DeeObject *composition = mapped_compose(self, should);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_removeif, self->sm_seq, composition, start, end, max);
	Dee_Decref_likely(composition);
	return result;
err:
	return (size_t)-1;
}
#endif /* WANT_mapped_mh_seq_removeif */



#ifdef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
/* Can implement pretty much all find(), remove(), etc.
 * functions by injecting "sm_mapper" as an (additional) key */

#ifdef WANT_mapped_mh_seq_count
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
mapped_mh_seq_count(SeqMapped *self, DeeObject *item) {
	return DeeObject_InvokeMethodHint(seq_count_with_key, self->sm_seq, item, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) size_t DCALL
mapped_mh_seq_count_with_key(SeqMapped *self, DeeObject *item, DeeObject *key) {
	size_t result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_count_with_key, self->sm_seq, item, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
mapped_mh_seq_count_with_range(SeqMapped *self, DeeObject *item, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_count_with_range_and_key, self->sm_seq, item, start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
mapped_mh_seq_count_with_range_and_key(SeqMapped *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_count_with_range_and_key, self->sm_seq, item, start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return (size_t)-1;
}
#endif /* WANT_mapped_mh_seq_count */


#ifdef WANT_mapped_mh_seq_contains
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mapped_mh_seq_contains(SeqMapped *self, DeeObject *item) {
	return DeeObject_InvokeMethodHint(seq_contains_with_key, self->sm_seq, item, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
mapped_mh_seq_contains_with_key(SeqMapped *self, DeeObject *item, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_contains_with_key, self->sm_seq, item, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mapped_mh_seq_contains_with_range(SeqMapped *self, DeeObject *item, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_contains_with_range_and_key, self->sm_seq, item, start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
mapped_mh_seq_contains_with_range_and_key(SeqMapped *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_contains_with_range_and_key, self->sm_seq, item, start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}
#endif /* WANT_mapped_mh_seq_contains */


#ifdef WANT_mapped_mh_seq_startswith
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mapped_mh_seq_startswith(SeqMapped *self, DeeObject *item) {
	return DeeObject_InvokeMethodHint(seq_startswith_with_key, self->sm_seq, item, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
mapped_mh_seq_startswith_with_key(SeqMapped *self, DeeObject *item, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_startswith_with_key, self->sm_seq, item, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mapped_mh_seq_startswith_with_range(SeqMapped *self, DeeObject *item, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_startswith_with_range_and_key, self->sm_seq, item, start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
mapped_mh_seq_startswith_with_range_and_key(SeqMapped *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_startswith_with_range_and_key, self->sm_seq, item, start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}
#endif /* WANT_mapped_mh_seq_startswith */


#ifdef WANT_mapped_mh_seq_endswith
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mapped_mh_seq_endswith(SeqMapped *self, DeeObject *item) {
	return DeeObject_InvokeMethodHint(seq_endswith_with_key, self->sm_seq, item, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
mapped_mh_seq_endswith_with_key(SeqMapped *self, DeeObject *item, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_endswith_with_key, self->sm_seq, item, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mapped_mh_seq_endswith_with_range(SeqMapped *self, DeeObject *item, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_endswith_with_range_and_key, self->sm_seq, item, start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
mapped_mh_seq_endswith_with_range_and_key(SeqMapped *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_endswith_with_range_and_key, self->sm_seq, item, start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}
#endif /* WANT_mapped_mh_seq_endswith */


#ifdef WANT_mapped_mh_seq_find
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
mapped_mh_seq_find(SeqMapped *self, DeeObject *item, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_find_with_key, self->sm_seq, item, start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
mapped_mh_seq_find_with_key(SeqMapped *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_find_with_key, self->sm_seq, item, start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return (size_t)-1;
}
#endif /* WANT_mapped_mh_seq_find */


#ifdef WANT_mapped_mh_seq_rfind
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
mapped_mh_seq_rfind(SeqMapped *self, DeeObject *item, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_rfind_with_key, self->sm_seq, item, start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
mapped_mh_seq_rfind_with_key(SeqMapped *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_rfind_with_key, self->sm_seq, item, start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return (size_t)-1;
}
#endif /* WANT_mapped_mh_seq_rfind */


#ifdef WANT_mapped_mh_seq_remove
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mapped_mh_seq_remove(SeqMapped *self, DeeObject *item, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_remove_with_key, self->sm_seq, item, start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
mapped_mh_seq_remove_with_key(SeqMapped *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_remove_with_key, self->sm_seq, item, start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}
#endif /* WANT_mapped_mh_seq_remove */


#ifdef WANT_mapped_mh_seq_rremove
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mapped_mh_seq_rremove(SeqMapped *self, DeeObject *item, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_rremove_with_key, self->sm_seq, item, start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
mapped_mh_seq_rremove_with_key(SeqMapped *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_rremove_with_key, self->sm_seq, item, start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}
#endif /* WANT_mapped_mh_seq_rremove */


#ifdef WANT_mapped_mh_seq_removeall
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
mapped_mh_seq_removeall(SeqMapped *self, DeeObject *item, size_t start, size_t end, size_t max) {
	return DeeObject_InvokeMethodHint(seq_removeall_with_key, self->sm_seq, item, start, end, max, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 6)) size_t DCALL
mapped_mh_seq_removeall_with_key(SeqMapped *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key) {
	size_t result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_removeall_with_key, self->sm_seq, item, start, end, max, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return (size_t)-1;
}
#endif /* WANT_mapped_mh_seq_removeall */


#ifdef WANT_mapped_mh_seq_bfind
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
mapped_mh_seq_bfind(SeqMapped *self, DeeObject *item, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_bfind_with_key, self->sm_seq, item, start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
mapped_mh_seq_bfind_with_key(SeqMapped *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_bfind_with_key, self->sm_seq, item, start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return (size_t)-1;
}
#endif /* WANT_mapped_mh_seq_bfind */


#ifdef WANT_mapped_mh_seq_bposition
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
mapped_mh_seq_bposition(SeqMapped *self, DeeObject *item, size_t start, size_t end) {
	return DeeObject_InvokeMethodHint(seq_bposition_with_key, self->sm_seq, item, start, end, self->sm_mapper);
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
mapped_mh_seq_bposition_with_key(SeqMapped *self, DeeObject *item, size_t start, size_t end, DeeObject *key) {
	size_t result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_bposition_with_key, self->sm_seq, item, start, end, composition);
	Dee_Decref_likely(composition);
	return result;
err:
	return (size_t)-1;
}
#endif /* WANT_mapped_mh_seq_bposition */


#ifdef WANT_mapped_mh_seq_brange
#define NEED_mapped_methods
#define NEED_mapped_method_hints
PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
mapped_mh_seq_brange(SeqMapped *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]) {
	return DeeObject_InvokeMethodHint(seq_brange_with_key, self->sm_seq, item, start, end, self->sm_mapper, result_range);
}

PRIVATE WUNUSED NONNULL((1, 2, 5, 6)) int DCALL
mapped_mh_seq_brange_with_key(SeqMapped *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]) {
	int result;
	DREF DeeObject *composition = mapped_compose(self, key);
	if unlikely(!composition)
		goto err;
	result = DeeObject_InvokeMethodHint(seq_brange_with_key, self->sm_seq, item, start, end, composition, result_range);
	Dee_Decref_likely(composition);
	return result;
err:
	return -1;
}
#endif /* WANT_mapped_mh_seq_brange */
#endif /* CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */


#ifndef NEED_mapped_getsets
#define mapped_getsets NULL
#else /* !NEED_mapped_getsets */
PRIVATE struct type_getset tpconst mapped_getsets[] = {
#ifdef WANT_mapped_seq_getfirst
	TYPE_GETSET_BOUND_NODOC(STR_first, &mapped_seq_getfirst, &mapped_seq_delfirst, NULL, &mapped_seq_boundfirst),
#endif /* WANT_mapped_seq_getfirst */
#ifdef WANT_mapped_seq_getlast
	TYPE_GETSET_BOUND_NODOC(STR_last, &mapped_seq_getlast, &mapped_seq_dellast, NULL, &mapped_seq_boundlast),
#endif /* WANT_mapped_seq_getlast */
#ifdef WANT_mapped_set_getfirst
	TYPE_GETSET_BOUND_NODOC(STR___set_first__, &mapped_set_getfirst, &mapped_set_delfirst, NULL, &mapped_set_boundfirst),
#endif /* WANT_mapped_set_getfirst */
#ifdef WANT_mapped_set_getlast
	TYPE_GETSET_BOUND_NODOC(STR___set_last__, &mapped_set_getlast, &mapped_set_dellast, NULL, &mapped_set_boundlast),
#endif /* WANT_mapped_set_getlast */
	TYPE_GETSET_END
};
#endif /* NEED_mapped_getsets */

#ifndef NEED_mapped_methods
#define mapped_methods NULL
#else /* !NEED_mapped_methods */
PRIVATE struct type_method tpconst mapped_methods[] = {
#ifdef WANT_mapped_map
	/* Override "Sequence.map()" such that instead of having 2 nested "SeqMapped" objects,
	 * there is only one whose callback is an instance of "FunctionComposition_Type". */
	TYPE_METHOD("map", &mapped_map,
	            "(" mapped_map_params ")->?.\n"
	            "#pmapper{A key function invoked to map members of @this Sequence}"
	            "Same as ?Amap?DSequence, but instead of chaining sequences on-top "
	            /**/ "of more sequences, simply ?Acompose?DCallable @mapper with "
	            /**/ "?#__mapper__ and return the result as a new ?."),
#endif /* WANT_mapped_map */

#if defined(WANT_mapped_mh_seq_enumerate) || defined(WANT_mapped_mh_seq_enumerate_index)
	TYPE_METHOD_HINTREF(__seq_enumerate__),
#endif /* WANT_mapped_mh_seq_enumerate || WANT_mapped_mh_seq_enumerate_index */

#ifdef WANT_mapped_mh_seq_erase
	TYPE_METHOD_HINTREF(Sequence_erase),
#endif /* WANT_mapped_mh_seq_erase */
#ifdef WANT_mapped_mh_seq_clear
	TYPE_METHOD_HINTREF(Sequence_clear),
#endif /* WANT_mapped_mh_seq_clear */
#ifdef WANT_mapped_mh_seq_reverse
	TYPE_METHOD_HINTREF(Sequence_reverse),
#endif /* WANT_mapped_mh_seq_reverse */
#ifdef WANT_mapped_mh_set_operator_bool
	TYPE_METHOD_HINTREF(__set_bool__),
#endif /* WANT_mapped_mh_set_operator_bool */
#ifdef WANT_mapped_mh_set_operator_size
	TYPE_METHOD_HINTREF(__set_size__),
#endif /* WANT_mapped_mh_set_operator_size */
#ifdef WANT_mapped_mh_map_operator_size
	TYPE_METHOD_HINTREF(__map_size__),
#endif /* WANT_mapped_mh_map_operator_size */
#ifdef WANT_mapped_mh_seq_any
	TYPE_METHOD_HINTREF(Sequence_any),
#endif /* WANT_mapped_mh_seq_any */
#ifdef WANT_mapped_mh_seq_all
	TYPE_METHOD_HINTREF(Sequence_all),
#endif /* WANT_mapped_mh_seq_all */
#ifdef WANT_mapped_mh_seq_parity
	TYPE_METHOD_HINTREF(Sequence_parity),
#endif /* WANT_mapped_mh_seq_parity */
#ifdef WANT_mapped_mh_seq_sort
	TYPE_METHOD_HINTREF(Sequence_sort),
#endif /* WANT_mapped_mh_seq_sort */
#ifdef WANT_mapped_mh_seq_sorted
	TYPE_METHOD_HINTREF(Sequence_sorted),
#endif /* WANT_mapped_mh_seq_sorted */
#ifdef WANT_mapped_mh_seq_pop
	TYPE_METHOD_HINTREF(Sequence_pop),
#endif /* WANT_mapped_mh_seq_pop */
#ifdef WANT_mapped_mh_seq_reversed
	TYPE_METHOD_HINTREF(Sequence_reversed),
#endif /* WANT_mapped_mh_seq_reversed */
#ifdef WANT_mapped_mh_seq_unpack
	TYPE_METHOD_HINTREF(Sequence_unpack),
#endif /* WANT_mapped_mh_seq_unpack */
#ifdef WANT_mapped_mh_seq_unpack_ub
	TYPE_METHOD_HINTREF(Sequence_unpackub),
#endif /* WANT_mapped_mh_seq_unpack_ub */
#ifdef WANT_mapped_mh_seq_sum
	TYPE_METHOD_HINTREF(Sequence_sum),
#endif /* WANT_mapped_mh_seq_sum */
#ifdef WANT_mapped_mh_seq_min
	TYPE_METHOD_HINTREF(Sequence_min),
#endif /* WANT_mapped_mh_seq_min */
#ifdef WANT_mapped_mh_seq_max
	TYPE_METHOD_HINTREF(Sequence_max),
#endif /* WANT_mapped_mh_seq_max */
#ifdef WANT_mapped_mh_seq_locate
	TYPE_METHOD_HINTREF(Sequence_locate),
#endif /* WANT_mapped_mh_seq_locate */
#ifdef WANT_mapped_mh_seq_rlocate
	TYPE_METHOD_HINTREF(Sequence_rlocate),
#endif /* WANT_mapped_mh_seq_rlocate */
#ifdef WANT_mapped_mh_seq_removeif
	TYPE_METHOD_HINTREF(Sequence_removeif),
#endif /* WANT_mapped_mh_seq_removeif */
#ifdef WANT_mapped_mh_set_pop
	TYPE_METHOD_HINTREF(__set_pop__),
#endif /* WANT_mapped_mh_set_pop */
#ifdef WANT_mapped_mh_map_popitem
	TYPE_METHOD_HINTREF(__map_popitem__),
#endif /* WANT_mapped_mh_map_popitem */

#ifdef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
#ifdef WANT_mapped_mh_seq_count
	TYPE_METHOD_HINTREF(Sequence_count),
#endif /* WANT_mapped_mh_seq_count */
#ifdef WANT_mapped_mh_seq_contains
	TYPE_METHOD_HINTREF(Sequence_contains),
#endif /* WANT_mapped_mh_seq_contains */
#ifdef WANT_mapped_mh_seq_startswith
	TYPE_METHOD_HINTREF(Sequence_startswith),
#endif /* WANT_mapped_mh_seq_startswith */
#ifdef WANT_mapped_mh_seq_endswith
	TYPE_METHOD_HINTREF(Sequence_endswith),
#endif /* WANT_mapped_mh_seq_endswith */
#ifdef WANT_mapped_mh_seq_find
	TYPE_METHOD_HINTREF(Sequence_find),
#endif /* WANT_mapped_mh_seq_find */
#ifdef WANT_mapped_mh_seq_rfind
	TYPE_METHOD_HINTREF(Sequence_rfind),
#endif /* WANT_mapped_mh_seq_rfind */
#ifdef WANT_mapped_mh_seq_remove
	TYPE_METHOD_HINTREF(Sequence_remove),
#endif /* WANT_mapped_mh_seq_remove */
#ifdef WANT_mapped_mh_seq_rremove
	TYPE_METHOD_HINTREF(Sequence_rremove),
#endif /* WANT_mapped_mh_seq_rremove */
#ifdef WANT_mapped_mh_seq_removeall
	TYPE_METHOD_HINTREF(Sequence_removeall),
#endif /* WANT_mapped_mh_seq_removeall */
#ifdef WANT_mapped_mh_seq_bfind
	TYPE_METHOD_HINTREF(Sequence_bfind),
#endif /* WANT_mapped_mh_seq_bfind */
#ifdef WANT_mapped_mh_seq_bposition
	TYPE_METHOD_HINTREF(Sequence_bposition),
#endif /* WANT_mapped_mh_seq_bposition */
#ifdef WANT_mapped_mh_seq_brange
	TYPE_METHOD_HINTREF(Sequence_brange),
#endif /* WANT_mapped_mh_seq_brange */
#endif /* CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	TYPE_METHOD_END
};
#endif /* NEED_mapped_methods */

#ifndef NEED_mapped_method_hints
#define mapped_method_hints NULL
#else /* !NEED_mapped_method_hints */
PRIVATE struct type_method_hint tpconst mapped_method_hints[] = {
#ifdef WANT_mapped_mh_seq_enumerate
	TYPE_METHOD_HINT(seq_enumerate, &mapped_mh_seq_enumerate),
#endif /* WANT_mapped_mh_seq_enumerate */
#ifdef WANT_mapped_mh_seq_enumerate_index
	TYPE_METHOD_HINT(seq_enumerate_index, &mapped_mh_seq_enumerate_index),
#endif /* WANT_mapped_mh_seq_enumerate_index */
#ifdef WANT_mapped_seq_getfirst
	TYPE_METHOD_HINT(seq_trygetfirst, &mapped_seq_trygetfirst),
#endif /* WANT_mapped_seq_getfirst */
#ifdef WANT_mapped_seq_getlast
	TYPE_METHOD_HINT(seq_trygetlast, &mapped_seq_trygetlast),
#endif /* WANT_mapped_seq_getlast */
#ifdef WANT_mapped_set_getfirst
	TYPE_METHOD_HINT(set_trygetfirst, &mapped_set_trygetfirst),
#endif /* WANT_mapped_set_getfirst */
#ifdef WANT_mapped_set_getlast
	TYPE_METHOD_HINT(set_trygetlast, &mapped_set_trygetlast),
#endif /* WANT_mapped_set_getlast */
#ifdef WANT_mapped_mh_seq_erase
	TYPE_METHOD_HINT(seq_erase, &mapped_mh_seq_erase),
#endif /* WANT_mapped_mh_seq_erase */
#ifdef WANT_mapped_mh_seq_clear
	TYPE_METHOD_HINT(seq_clear, &mapped_mh_seq_clear),
#endif /* WANT_mapped_mh_seq_clear */
#ifdef WANT_mapped_mh_seq_reverse
	TYPE_METHOD_HINT(seq_reverse, &mapped_mh_seq_reverse),
#endif /* WANT_mapped_mh_seq_reverse */
#ifdef WANT_mapped_mh_set_operator_bool
	TYPE_METHOD_HINT(set_operator_bool, &mapped_mh_set_operator_bool),
#endif /* WANT_mapped_mh_set_operator_bool */
#ifdef WANT_mapped_mh_set_operator_size
	TYPE_METHOD_HINT(set_operator_size, &mapped_mh_set_operator_size),
	TYPE_METHOD_HINT(set_operator_sizeob, &mapped_mh_set_operator_sizeob),
#endif /* WANT_mapped_mh_set_operator_size */
#ifdef WANT_mapped_mh_map_operator_size
	TYPE_METHOD_HINT(map_operator_size, &mapped_mh_map_operator_size),
	TYPE_METHOD_HINT(map_operator_sizeob, &mapped_mh_map_operator_sizeob),
#endif /* WANT_mapped_mh_map_operator_size */
#ifdef WANT_mapped_mh_seq_any
	TYPE_METHOD_HINT(seq_any, &mapped_mh_seq_any),
	TYPE_METHOD_HINT(seq_any_with_key, &mapped_mh_seq_any_with_key),
	TYPE_METHOD_HINT(seq_any_with_range, &mapped_mh_seq_any_with_range),
	TYPE_METHOD_HINT(seq_any_with_range_and_key, &mapped_mh_seq_any_with_range_and_key),
#endif /* WANT_mapped_mh_seq_any */
#ifdef WANT_mapped_mh_seq_all
	TYPE_METHOD_HINT(seq_all, &mapped_mh_seq_all),
	TYPE_METHOD_HINT(seq_all_with_key, &mapped_mh_seq_all_with_key),
	TYPE_METHOD_HINT(seq_all_with_range, &mapped_mh_seq_all_with_range),
	TYPE_METHOD_HINT(seq_all_with_range_and_key, &mapped_mh_seq_all_with_range_and_key),
#endif /* WANT_mapped_mh_seq_all */
#ifdef WANT_mapped_mh_seq_parity
	TYPE_METHOD_HINT(seq_parity, &mapped_mh_seq_parity),
	TYPE_METHOD_HINT(seq_parity_with_key, &mapped_mh_seq_parity_with_key),
	TYPE_METHOD_HINT(seq_parity_with_range, &mapped_mh_seq_parity_with_range),
	TYPE_METHOD_HINT(seq_parity_with_range_and_key, &mapped_mh_seq_parity_with_range_and_key),
#endif /* WANT_mapped_mh_seq_parity */
#ifdef WANT_mapped_mh_seq_sort
	TYPE_METHOD_HINT(seq_sort, &mapped_mh_seq_sort),
	TYPE_METHOD_HINT(seq_sort_with_key, &mapped_mh_seq_sort_with_key),
#endif /* WANT_mapped_mh_seq_sort */
#ifdef WANT_mapped_mh_seq_sorted
	TYPE_METHOD_HINT(seq_sorted, &mapped_mh_seq_sorted),
	TYPE_METHOD_HINT(seq_sorted_with_key, &mapped_mh_seq_sorted_with_key),
#endif /* WANT_mapped_mh_seq_sorted */

#ifdef WANT_mapped_mh_seq_pop
	TYPE_METHOD_HINT(seq_pop, &mapped_mh_seq_pop),
#endif /* WANT_mapped_mh_seq_pop */
#ifdef WANT_mapped_mh_seq_reversed
	TYPE_METHOD_HINT(seq_reversed, &mapped_mh_seq_reversed),
#endif /* WANT_mapped_mh_seq_reversed */
#ifdef WANT_mapped_mh_seq_unpack
	TYPE_METHOD_HINT(seq_unpack, &mapped_mh_seq_unpack),
	TYPE_METHOD_HINT(seq_unpack_ex, &mapped_mh_seq_unpack_ex),
#endif /* WANT_mapped_mh_seq_unpack */
#ifdef WANT_mapped_mh_seq_unpack_ub
	TYPE_METHOD_HINT(seq_unpack_ub, &mapped_mh_seq_unpack_ub),
#endif /* WANT_mapped_mh_seq_unpack_ub */
#ifdef WANT_mapped_mh_seq_sum
	TYPE_METHOD_HINT(seq_sum, &mapped_mh_seq_sum),
	TYPE_METHOD_HINT(seq_sum_with_key, &mapped_mh_seq_sum_with_key),
	TYPE_METHOD_HINT(seq_sum_with_range, &mapped_mh_seq_sum_with_range),
	TYPE_METHOD_HINT(seq_sum_with_range_and_key, &mapped_mh_seq_sum_with_range_and_key),
#endif /* WANT_mapped_mh_seq_sum */
#ifdef WANT_mapped_mh_seq_min
	TYPE_METHOD_HINT(seq_min, &mapped_mh_seq_min),
	TYPE_METHOD_HINT(seq_min_with_key, &mapped_mh_seq_min_with_key),
	TYPE_METHOD_HINT(seq_min_with_range, &mapped_mh_seq_min_with_range),
	TYPE_METHOD_HINT(seq_min_with_range_and_key, &mapped_mh_seq_min_with_range_and_key),
#endif /* WANT_mapped_mh_seq_min */
#ifdef WANT_mapped_mh_seq_max
	TYPE_METHOD_HINT(seq_max, &mapped_mh_seq_max),
	TYPE_METHOD_HINT(seq_max_with_key, &mapped_mh_seq_max_with_key),
	TYPE_METHOD_HINT(seq_max_with_range, &mapped_mh_seq_max_with_range),
	TYPE_METHOD_HINT(seq_max_with_range_and_key, &mapped_mh_seq_max_with_range_and_key),
#endif /* WANT_mapped_mh_seq_max */
#ifdef WANT_mapped_mh_seq_locate
	TYPE_METHOD_HINT(seq_locate, &mapped_mh_seq_locate),                       /* Just use `DeeFunctionComposition_Of(2, {match, self->sm_mapper})' as new "match" */
	TYPE_METHOD_HINT(seq_locate_with_range, &mapped_mh_seq_locate_with_range), /* Just use `DeeFunctionComposition_Of(2, {match, self->sm_mapper})' as new "match" */
#endif /* WANT_mapped_mh_seq_locate */
#ifdef WANT_mapped_mh_seq_rlocate
	TYPE_METHOD_HINT(seq_rlocate, &mapped_mh_seq_rlocate),                       /* Just use `DeeFunctionComposition_Of(2, {match, self->sm_mapper})' as new "match" */
	TYPE_METHOD_HINT(seq_rlocate_with_range, &mapped_mh_seq_rlocate_with_range), /* Just use `DeeFunctionComposition_Of(2, {match, self->sm_mapper})' as new "match" */
#endif /* WANT_mapped_mh_seq_rlocate */
#ifdef WANT_mapped_mh_seq_removeif
	TYPE_METHOD_HINT(seq_removeif, &mapped_mh_seq_removeif), /* Just use `DeeFunctionComposition_Of(2, {should, self->sm_mapper})' as new "should" */
#endif /* WANT_mapped_mh_seq_removeif */

#ifdef WANT_mapped_mh_set_pop
	TYPE_METHOD_HINT(set_pop, &mapped_mh_set_pop),
	TYPE_METHOD_HINT(set_pop_with_default, &mapped_mh_set_pop_with_default),
#endif /* WANT_mapped_mh_set_pop */
#ifdef WANT_mapped_mh_map_popitem
	TYPE_METHOD_HINT(map_popitem, &mapped_mh_map_popitem),
#endif /* WANT_mapped_mh_map_popitem */

#ifdef CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM
	/* Can implement pretty much all find(), remove(), etc.
	 * functions by injecting "sm_mapper" as an (additional) key */
#ifdef WANT_mapped_mh_seq_count
	TYPE_METHOD_HINT(seq_count, &mapped_mh_seq_count),
	TYPE_METHOD_HINT(seq_count_with_key, &mapped_mh_seq_count_with_key),
	TYPE_METHOD_HINT(seq_count_with_range, &mapped_mh_seq_count_with_range),
	TYPE_METHOD_HINT(seq_count_with_range_and_key, &mapped_mh_seq_count_with_range_and_key),
#endif /* WANT_mapped_mh_seq_count */
#ifdef WANT_mapped_mh_seq_contains
	TYPE_METHOD_HINT(seq_contains, &mapped_mh_seq_contains),
	TYPE_METHOD_HINT(seq_contains_with_key, &mapped_mh_seq_contains_with_key),
	TYPE_METHOD_HINT(seq_contains_with_range, &mapped_mh_seq_contains_with_range),
	TYPE_METHOD_HINT(seq_contains_with_range_and_key, &mapped_mh_seq_contains_with_range_and_key),
#endif /* WANT_mapped_mh_seq_contains */
#ifdef WANT_mapped_mh_seq_startswith
	TYPE_METHOD_HINT(seq_startswith, &mapped_mh_seq_startswith),
	TYPE_METHOD_HINT(seq_startswith_with_key, &mapped_mh_seq_startswith_with_key),
	TYPE_METHOD_HINT(seq_startswith_with_range, &mapped_mh_seq_startswith_with_range),
	TYPE_METHOD_HINT(seq_startswith_with_range_and_key, &mapped_mh_seq_startswith_with_range_and_key),
#endif /* WANT_mapped_mh_seq_startswith */
#ifdef WANT_mapped_mh_seq_endswith
	TYPE_METHOD_HINT(seq_endswith, &mapped_mh_seq_endswith),
	TYPE_METHOD_HINT(seq_endswith_with_key, &mapped_mh_seq_endswith_with_key),
	TYPE_METHOD_HINT(seq_endswith_with_range, &mapped_mh_seq_endswith_with_range),
	TYPE_METHOD_HINT(seq_endswith_with_range_and_key, &mapped_mh_seq_endswith_with_range_and_key),
#endif /* WANT_mapped_mh_seq_endswith */
#ifdef WANT_mapped_mh_seq_find
	TYPE_METHOD_HINT(seq_find, &mapped_mh_seq_find),
	TYPE_METHOD_HINT(seq_find_with_key, &mapped_mh_seq_find_with_key),
#endif /* WANT_mapped_mh_seq_find */
#ifdef WANT_mapped_mh_seq_rfind
	TYPE_METHOD_HINT(seq_rfind, &mapped_mh_seq_rfind),
	TYPE_METHOD_HINT(seq_rfind_with_key, &mapped_mh_seq_rfind_with_key),
#endif /* WANT_mapped_mh_seq_rfind */
#ifdef WANT_mapped_mh_seq_remove
	TYPE_METHOD_HINT(seq_remove, &mapped_mh_seq_remove),
	TYPE_METHOD_HINT(seq_remove_with_key, &mapped_mh_seq_remove_with_key),
#endif /* WANT_mapped_mh_seq_remove */
#ifdef WANT_mapped_mh_seq_rremove
	TYPE_METHOD_HINT(seq_rremove, &mapped_mh_seq_rremove),
	TYPE_METHOD_HINT(seq_rremove_with_key, &mapped_mh_seq_rremove_with_key),
#endif /* WANT_mapped_mh_seq_rremove */
#ifdef WANT_mapped_mh_seq_removeall
	TYPE_METHOD_HINT(seq_removeall, &mapped_mh_seq_removeall),
	TYPE_METHOD_HINT(seq_removeall_with_key, &mapped_mh_seq_removeall_with_key),
#endif /* WANT_mapped_mh_seq_removeall */
#ifdef WANT_mapped_mh_seq_bfind
	TYPE_METHOD_HINT(seq_bfind, &mapped_mh_seq_bfind),
	TYPE_METHOD_HINT(seq_bfind_with_key, &mapped_mh_seq_bfind_with_key),
#endif /* WANT_mapped_mh_seq_bfind */
#ifdef WANT_mapped_mh_seq_bposition
	TYPE_METHOD_HINT(seq_bposition, &mapped_mh_seq_bposition),
	TYPE_METHOD_HINT(seq_bposition_with_key, &mapped_mh_seq_bposition_with_key),
#endif /* WANT_mapped_mh_seq_bposition */
#ifdef WANT_mapped_mh_seq_brange
	TYPE_METHOD_HINT(seq_brange, &mapped_mh_seq_brange),
	TYPE_METHOD_HINT(seq_brange_with_key, &mapped_mh_seq_brange_with_key),
#endif /* WANT_mapped_mh_seq_brange */
#endif /* CONFIG_EXPERIMENTAL_KEY_NOT_APPLIED_TO_ITEM */
	TYPE_METHOD_HINT_END
};
#endif /* NEED_mapped_method_hints */

INTERN DeeTypeObject SeqMapped_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SeqMapped",
	/* .tp_doc      = */ DOC("(seq:?DSequence,mapper:?DCallable)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ SeqMapped,
			/* tp_ctor:        */ &mapped_ctor,
			/* tp_copy_ctor:   */ &mapped_copy,
			/* tp_any_ctor:    */ &mapped_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &mapped_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mapped_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&mapped_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&mapped_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__247219960F1E745D),
	/* .tp_seq           = */ &mapped_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ mapped_methods,
	/* .tp_getsets       = */ mapped_getsets,
	/* .tp_members       = */ mapped_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ mapped_class_members,
	/* .tp_method_hints  = */ mapped_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
	return Dee_AsObject(result);
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_MAPPED_C */
