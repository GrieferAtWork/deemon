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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAP_PROXY_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAP_PROXY_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error-rt.h>           /* DeeRT_ErrEmptySequence, DeeRT_ErrNestSequenceError */
#include <deemon/method-hints.h>       /* DeeObject_InvokeMethodHint, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/none.h>               /* DeeNone_Check */
#include <deemon/object.h>             /* DREF, DeeObject, DeeTypeObject, Dee_Decref, Dee_DecrefNokill, Dee_foreach_t, Dee_ssize_t, ITER_ISOK, OBJECT_HEAD_INIT, return_reference_ */
#include <deemon/seq.h>                /* DeeSeq_Type, DeeSeq_Unpack */
#include <deemon/set.h>                /* DeeSet_Type */
#include <deemon/type.h>               /* DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_visit_t, METHOD_FNOREFESCAPE, STRUCT_OBJECT_AB, TF_NONE, TP_FFINAL, TP_FNORMAL, TYPE_*, type_* */

#include "../generic-proxy.h"
#include "default-map-proxy.h"

#include <stddef.h> /* NULL, offsetof, size_t */

DECL_BEGIN

STATIC_ASSERT(offsetof(DefaultSequence_MapProxy, dsmp_map) ==
              offsetof(ProxyObject, po_obj));
#define ds_mk_init             generic_proxy__init
#define ds_mv_init             generic_proxy__init
#define ds_mk_serialize        generic_proxy__serialize
#define ds_mv_serialize        generic_proxy__serialize
#define ds_mk_copy             generic_proxy__copy_alias
#define ds_mv_copy             generic_proxy__copy_alias
#define ds_mv_fini             generic_proxy__fini
#define ds_mk_fini             generic_proxy__fini
#define ds_mv_visit            generic_proxy__visit
#define ds_mk_visit            generic_proxy__visit
#define ds_mk_iter             generic_proxy__map_iterkeys
#define ds_mv_iter             generic_proxy__map_itervalues
#define ds_mk_contains         generic_proxy__map_operator_contains
#define ds_mv_size             generic_proxy__map_operator_size
#define ds_mv_sizeob           generic_proxy__map_operator_sizeob
#define ds_mv_bounditem        generic_proxy__seq_operator_bounditem
#define ds_mv_bounditem_index  generic_proxy__seq_operator_bounditem_index
#define ds_mv_hasitem          generic_proxy__seq_operator_hasitem
#define ds_mv_hasitem_index    generic_proxy__seq_operator_hasitem_index
#define ds_mv_delitem          generic_proxy__seq_operator_delitem
#define ds_mv_delitem_index    generic_proxy__seq_operator_delitem_index
#define ds_mv_delrange         generic_proxy__seq_operator_delrange
#define ds_mv_delrange_index   generic_proxy__seq_operator_delrange_index
#define ds_mv_delrange_index_n generic_proxy__seq_operator_delrange_index_n

#define ds_mk_mh_seq_clear     generic_proxy__seq_clear
#define ds_mv_mh_seq_clear     generic_proxy__seq_clear
#define ds_mk_mh_set_remove    generic_proxy__map_remove
#define ds_mk_mh_set_removeall generic_proxy__map_removekeys

struct ds_mX_foreach_cb_data {
	Dee_foreach_t dmxfcd_cb;  /* [1..1] Nested callback. */
	void         *dmxfcd_arg; /* [?..?] Cookie for `dmxfcd_cb' */
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_mk_enumerate_cb(void *arg, DeeObject *key, DeeObject *UNUSED(value)) {
	struct ds_mX_foreach_cb_data *data = (struct ds_mX_foreach_cb_data *)arg;
	return (*data->dmxfcd_cb)(data->dmxfcd_arg, key);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_mk_foreach(DefaultSequence_MapProxy *self, Dee_foreach_t cb, void *arg) {
	struct ds_mX_foreach_cb_data data;
	data.dmxfcd_cb  = cb;
	data.dmxfcd_arg = arg;
	return DeeObject_InvokeMethodHint(map_enumerate, self->dsmp_map, &ds_mk_enumerate_cb, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_mv_enumerate_cb(void *arg, DeeObject *UNUSED(key), DeeObject *value) {
	struct ds_mX_foreach_cb_data *data = (struct ds_mX_foreach_cb_data *)arg;
	return value ? (*data->dmxfcd_cb)(data->dmxfcd_arg, value) : 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ds_mv_foreach(DefaultSequence_MapProxy *self, Dee_foreach_t cb, void *arg) {
	struct ds_mX_foreach_cb_data data;
	data.dmxfcd_cb  = cb;
	data.dmxfcd_arg = arg;
	return DeeObject_InvokeMethodHint(map_enumerate, self->dsmp_map, &ds_mv_enumerate_cb, &data);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_mk_mh_set_pop(DefaultSequence_MapProxy *self) {
	DREF DeeObject *item;
	item = DeeObject_InvokeMethodHint(map_popitem, self->dsmp_map);
	if unlikely(!item)
		goto err;
	if (!DeeNone_Check(item)) {
		DREF DeeObject *key_and_value[2];
		int temp = DeeSeq_Unpack(item, 2, key_and_value);
		Dee_Decref(item);
		if unlikely(temp)
			goto err;
		Dee_Decref(key_and_value[1]);
		return key_and_value[0];
	}
	Dee_DecrefNokill(item);
	DeeRT_ErrEmptySequence(self->dsmp_map);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ds_mk_mh_set_pop_with_default(DefaultSequence_MapProxy *self,
                              DeeObject *def) {
	DREF DeeObject *item;
	item = DeeObject_InvokeMethodHint(map_popitem, self->dsmp_map);
	if unlikely(!item)
		goto err;
	if (!DeeNone_Check(item)) {
		DREF DeeObject *key_and_value[2];
		int temp = DeeSeq_Unpack(item, 2, key_and_value);
		Dee_Decref(item);
		if unlikely(temp)
			goto err;
		Dee_Decref(key_and_value[1]);
		return key_and_value[0];
	}
	Dee_DecrefNokill(item);
	return_reference_(def);
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ds_mv_getitem(DefaultSequence_MapProxy *self, DeeObject *index) {
	int temp;
	DREF DeeObject *item;
	DREF DeeObject *key_and_value[2];
	item = DeeObject_InvokeMethodHint(seq_operator_getitem, self->dsmp_map, index);
	if unlikely(!item)
		goto err_maybe_nest;
	temp = DeeSeq_Unpack(item, 2, key_and_value);
	Dee_Decref(item);
	if unlikely(temp)
		goto err;
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
err_maybe_nest:
	DeeRT_ErrNestSequenceError(self->dsmp_map, self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ds_mv_trygetitem(DefaultSequence_MapProxy *self, DeeObject *index) {
	int temp;
	DREF DeeObject *item;
	DREF DeeObject *key_and_value[2];
	item = DeeObject_InvokeMethodHint(seq_operator_trygetitem, self->dsmp_map, index);
	if unlikely(!ITER_ISOK(item)) {
		if unlikely(!item)
			goto err_maybe_nest;
		return item;
	}
	temp = DeeSeq_Unpack(item, 2, key_and_value);
	Dee_Decref(item);
	if unlikely(temp)
		goto err;
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
err_maybe_nest:
	DeeRT_ErrNestSequenceError(self->dsmp_map, self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_mv_getitem_index(DefaultSequence_MapProxy *self, size_t index) {
	int temp;
	DREF DeeObject *item;
	DREF DeeObject *key_and_value[2];
	item = DeeObject_InvokeMethodHint(seq_operator_getitem_index, self->dsmp_map, index);
	if unlikely(!item)
		goto err_maybe_nest;
	temp = DeeSeq_Unpack(item, 2, key_and_value);
	Dee_Decref(item);
	if unlikely(temp)
		goto err;
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
err_maybe_nest:
	DeeRT_ErrNestSequenceError(self->dsmp_map, self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_mv_trygetitem_index(DefaultSequence_MapProxy *self, size_t index) {
	int temp;
	DREF DeeObject *item;
	DREF DeeObject *key_and_value[2];
	item = DeeObject_InvokeMethodHint(seq_operator_trygetitem_index, self->dsmp_map, index);
	if unlikely(!ITER_ISOK(item)) {
		if unlikely(!item)
			goto err_maybe_nest;
		return item;
	}
	temp = DeeSeq_Unpack(item, 2, key_and_value);
	Dee_Decref(item);
	if unlikely(temp)
		goto err;
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
err_maybe_nest:
	DeeRT_ErrNestSequenceError(self->dsmp_map, self);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
ds_mv_setitem(DefaultSequence_MapProxy *self, DeeObject *index, DeeObject *value) {
	int result;
	DREF DeeObject *item;
	DREF DeeObject *key;
	item = DeeObject_InvokeMethodHint(seq_operator_getitem, self->dsmp_map, index);
	if unlikely(!item)
		goto err_maybe_nest;
	key = DeeObject_InvokeMethodHint(seq_getfirst, item);
	Dee_Decref(item);
	if unlikely(!key)
		goto err;
	result = DeeObject_InvokeMethodHint(map_operator_setitem, self->dsmp_map, key, value);
	Dee_Decref(key);
	return result;
err_maybe_nest:
	DeeRT_ErrNestSequenceError(self->dsmp_map, self);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
ds_mv_setitem_index(DefaultSequence_MapProxy *self, size_t index, DeeObject *value) {
	int result;
	DREF DeeObject *item;
	DREF DeeObject *key;
	item = DeeObject_InvokeMethodHint(seq_operator_getitem_index, self->dsmp_map, index);
	if unlikely(!item)
		goto err_maybe_nest;
	key = DeeObject_InvokeMethodHint(seq_getfirst, item);
	Dee_Decref(item);
	if unlikely(!key)
		goto err;
	result = DeeObject_InvokeMethodHint(map_operator_setitem, self->dsmp_map, key, value);
	Dee_Decref(key);
	return result;
err_maybe_nest:
	DeeRT_ErrNestSequenceError(self->dsmp_map, self);
err:
	return -1;
}




PRIVATE struct type_seq ds_mk_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_mk_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ds_mk_contains,
	/* .tp_getitem                    = */ DEFIMPL_UNSUPPORTED(&default__getitem__unsupported),
	/* .tp_delitem                    = */ DEFIMPL_UNSUPPORTED(&default__delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL_UNSUPPORTED(&default__setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ds_mk_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ DEFIMPL_UNSUPPORTED(&default__bounditem__unsupported),
	/* .tp_hasitem                    = */ DEFIMPL_UNSUPPORTED(&default__hasitem__unsupported),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_foreach), /* XXX: When the map can't have unbound keys, this is equal to its length */
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL_UNSUPPORTED(&default__getitem_index__unsupported),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL_UNSUPPORTED(&default__delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL_UNSUPPORTED(&default__setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL_UNSUPPORTED(&default__bounditem_index__unsupported),
	/* .tp_hasitem_index              = */ DEFIMPL_UNSUPPORTED(&default__hasitem_index__unsupported),
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL_UNSUPPORTED(&default__trygetitem__unsupported),
	/* .tp_trygetitem_index           = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_index__unsupported),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_string_hash__unsupported),
	/* .tp_getitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__getitem_string_hash__unsupported),
	/* .tp_delitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ DEFIMPL_UNSUPPORTED(&default__bounditem_string_hash__unsupported),
	/* .tp_hasitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__hasitem_string_hash__unsupported),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_string_len_hash__unsupported),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__getitem_string_len_hash__unsupported),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL_UNSUPPORTED(&default__bounditem_string_len_hash__unsupported),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__hasitem_string_len_hash__unsupported),
};

PRIVATE struct type_seq ds_mv_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_mv_iter,
	/* .tp_sizeob                     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_mv_sizeob,
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ds_mv_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&ds_mv_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&ds_mv_setitem,
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&ds_mv_delrange,
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ds_mv_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&ds_mv_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&ds_mv_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&ds_mv_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_mv_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&ds_mv_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&ds_mv_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&ds_mv_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&ds_mv_hasitem_index,
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&ds_mv_delrange_index,
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index),
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&ds_mv_delrange_index_n,
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ds_mv_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_mv_trygetitem_index,
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


PRIVATE struct type_method tpconst ds_mk_methods[] = {
	/* TODO: byhash(ob) { return Mapping.byhash(this.__map__, ob).map(e -> e.first); } */
	TYPE_METHOD_HINTREF(Set_remove),
	TYPE_METHOD_HINTREF(Set_removeall),
	TYPE_METHOD_HINTREF(Set_pop),
#define ds_mv_methods (ds_mk_methods + 3)
	TYPE_METHOD_HINTREF(Sequence_clear),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst ds_mk_method_hints[] = {
	TYPE_METHOD_HINT_F(set_remove, &ds_mk_mh_set_remove, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_removeall, &ds_mk_mh_set_removeall, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_pop, &ds_mk_mh_set_pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_pop_with_default, &ds_mk_mh_set_pop_with_default, METHOD_FNOREFESCAPE),
#define ds_mv_method_hints (ds_mk_method_hints + 4)
	TYPE_METHOD_HINT_F(seq_clear, &ds_mk_mh_seq_clear, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

#define ds_mv_members ds_mk_members
PRIVATE struct type_member tpconst ds_mk_members[] = {
	TYPE_MEMBER_FIELD_DOC("__map__", STRUCT_OBJECT_AB, offsetof(DefaultSequence_MapProxy, dsmp_map),
	                      "->?DMapping\n"
	                      "The underlying mapping-object"),
	TYPE_MEMBER_END
};



INTERN DeeTypeObject DefaultSequence_MapKeys_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapKeys",
	/* .tp_doc      = */ DOC("(map:?DMapping)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultSequence_MapProxy,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &ds_mk_copy,
			/* tp_any_ctor:    */ &ds_mk_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &ds_mk_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_mk_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_foreach),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_set_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ds_mk_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__8B471346AD5C3673),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__48CC5897A5CA5795),
	/* .tp_seq           = */ &ds_mk_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ ds_mk_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_mk_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ ds_mk_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

INTERN DeeTypeObject DefaultSequence_MapValues_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapValues",
	/* .tp_doc      = */ DOC("(map:?DMapping)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultSequence_MapProxy,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &ds_mv_copy,
			/* tp_any_ctor:    */ &ds_mv_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &ds_mv_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_mv_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_size),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ds_mv_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__79A80CB66CBC8DB1),
	/* .tp_seq           = */ &ds_mv_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ ds_mv_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_mv_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ ds_mv_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAP_PROXY_C */
