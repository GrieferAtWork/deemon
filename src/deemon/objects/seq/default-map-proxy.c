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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAP_PROXY_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAP_PROXY_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"

/**/
#include "default-map-proxy.h"
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

#define ds_mk_copy     generic_proxy__copy_alias
#define ds_mv_copy     generic_proxy__copy_alias
#define ds_mv_deepcopy generic_proxy__deepcopy
#define ds_mk_deepcopy generic_proxy__deepcopy

PRIVATE WUNUSED NONNULL((1)) int DCALL
ds_mk_init(DefaultSequence_MapProxy *__restrict self,
           size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqMapKeys", &self->dsmp_map))
		goto err;
	Dee_Incref(self->dsmp_map);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ds_mv_init(DefaultSequence_MapProxy *__restrict self,
           size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SeqMapValues", &self->dsmp_map))
		goto err;
	Dee_Incref(self->dsmp_map);
	return 0;
err:
	return -1;
}

#define ds_mv_fini  generic_proxy__fini
#define ds_mk_fini  generic_proxy__fini
#define ds_mv_visit generic_proxy__visit
#define ds_mk_visit generic_proxy__visit

PRIVATE NONNULL((1)) DREF DeeObject *DCALL
ds_mk_iter(DefaultSequence_MapProxy *__restrict self) {
	return DeeObject_InvokeMethodHint(map_iterkeys, self->dsmp_map);
}

PRIVATE NONNULL((1)) DREF DeeObject *DCALL
ds_mv_iter(DefaultSequence_MapProxy *__restrict self) {
	return DeeObject_InvokeMethodHint(map_itervalues, self->dsmp_map);
}

PRIVATE struct type_seq ds_mk_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_mk_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains), // TODO: (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ds_mk_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter), // TODO: (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ds_mk_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_foreach), /* XXX: When the map can't have unbound keys, this is equal to its length */
};

PRIVATE struct type_seq ds_mv_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_mv_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size), // TODO: (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_mv_sizeob,
	/* .tp_contains                   = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains), // TODO: (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ds_mv_contains,
	/* .tp_getitem                    = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter), // TODO: (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ds_mv_foreach,
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_foreach), // TODO: (size_t (DCALL *)(DeeObject *__restrict))&ds_mv_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach), // TODO: (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_mv_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported), // TODO: (int (DCALL *)(DeeObject *, size_t))&ds_mv_delitem_index,
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported), // TODO: (int (DCALL *)(DeeObject *, size_t, DeeObject *))&ds_mv_setitem_index,
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach), // TODO: (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_mv_trygetitem_index,
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_mk_remove(DefaultSequence_MapProxy *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:remove", &key))
		goto err;
	result = DeeObject_InvokeMethodHint(map_remove, self->dsmp_map, key);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_mk_removeall(DefaultSequence_MapProxy *self, size_t argc, DeeObject *const *argv) {
	DeeObject *keys;
	if (DeeArg_Unpack(argc, argv, "o:removeall", &keys))
		goto err;
	if unlikely(DeeObject_InvokeMethodHint(map_removekeys, self->dsmp_map, keys))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_mk_pop(DefaultSequence_MapProxy *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *item;
	DeeObject *def = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:pop", &def))
		goto err;
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
	if (def)
		return_reference_(def);
	err_empty_sequence(self->dsmp_map);
err:
	return NULL;
}

#define ds_mv_clear ds_mk_clear
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_mk_clear(DefaultSequence_MapProxy *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":clear"))
		goto err;
	if unlikely(DeeObject_InvokeMethodHint(seq_clear, self->dsmp_map))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE struct type_method tpconst ds_mk_methods[] = {
	/* TODO: byhash(ob) { return Mapping.byhash(this.__map__, ob).map(e -> e.first); } */
	/* TODO: CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS: Use method hints instead of explicit callbacks */
	TYPE_METHOD(STR_remove, &ds_mk_remove,
	            "(key)->?Dbool\n"
	            "${"
	            /**/ "function remove(key): bool {\n"
	            /**/ "	return Mapping.remove(this.__map__, key);\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD(STR_removeall, &ds_mk_removeall,
	            "(keys:?S?O)\n"
	            "${"
	            /**/ "function removeall(keys) {\n"
	            /**/ "	Mapping.removekeys(this.__map__, keys);\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD(STR_pop, &ds_mk_pop,
	            "(def?)->\n"
	            "${"
	            /**/ "function pop(def?) {\n"
	            /**/ "	local item = Mapping.popitem(this.__map__);\n"
	            /**/ "	if (item !is none) {\n"
	            /**/ "		local result, none = item...;\n"
	            /**/ "		return result;\n"
	            /**/ "	}\n"
	            /**/ "	throw ValueError(...);\n"
	            /**/ "}"
	            "}"),
#define ds_mv_methods (ds_mk_methods + 3)
	TYPE_METHOD(STR_clear, &ds_mk_clear,
	            "()"
	            "${"
	            /**/ "function clear() {\n"
	            /**/ "	Sequence.clear(this.__map__);\n"
	            /**/ "}"
	            "}"),
	TYPE_METHOD_END
};

#define ds_mv_members ds_mk_members
PRIVATE struct type_member tpconst ds_mk_members[] = {
	TYPE_MEMBER_FIELD_DOC("__map__", STRUCT_OBJECT, offsetof(DefaultSequence_MapProxy, dsmp_map),
	                      "->?DMapping\n"
	                      "The underlying mapping-object"),
	TYPE_MEMBER_END
};



INTERN DeeTypeObject DefaultSequence_MapKeys_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapKeys",
	/* .tp_doc      = */ DOC("(map)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&ds_mk_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&ds_mk_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&ds_mk_init,
				TYPE_FIXED_ALLOCATOR(DefaultSequence_MapProxy)
			}
		},
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ds_mk_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__AFC6A8FA89E9F0A6),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__7188129899C2A8D6),
	/* .tp_seq           = */ &ds_mk_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ ds_mk_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_mk_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
};

INTERN DeeTypeObject DefaultSequence_MapValues_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapValues",
	/* .tp_doc      = */ DOC("(map)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&ds_mv_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&ds_mv_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&ds_mv_init,
				TYPE_FIXED_ALLOCATOR(DefaultSequence_MapProxy)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ds_mv_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_foreach),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ds_mv_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__9211580AA9433079),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &ds_mv_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ ds_mv_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ds_mv_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL, /* TODO: seq_enumerate_index */
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAP_PROXY_C */
