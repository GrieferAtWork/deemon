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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAP_PROXY_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAP_PROXY_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/set.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "default-api.h"

/**/
#include "default-map-proxy.h"

DECL_BEGIN

#define ds_mk_copy     generic_proxy_copy_alias
#define ds_mv_copy     generic_proxy_copy_alias
#define ds_mv_deepcopy generic_proxy_deepcopy
#define ds_mk_deepcopy generic_proxy_deepcopy

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

#define ds_mv_fini  generic_proxy_fini
#define ds_mk_fini  generic_proxy_fini
#define ds_mv_visit generic_proxy_visit
#define ds_mk_visit generic_proxy_visit

PRIVATE NONNULL((1)) DREF DeeObject *DCALL
ds_mk_iter(DefaultSequence_MapProxy *__restrict self) {
	return DeeMap_InvokeIterKeys(self->dsmp_map);
}

PRIVATE NONNULL((1)) DREF DeeObject *DCALL
ds_mv_iter(DefaultSequence_MapProxy *__restrict self) {
	return DeeMap_InvokeIterValues(self->dsmp_map);
}

PRIVATE struct type_seq ds_mk_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_mk_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL, // TODO: (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ds_mk_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL, // TODO: (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ds_mk_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL, /* XXX: When the map can't have unbound keys, this is equal to its length */
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL,
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
	/* .tp_asvector                   = */ NULL,
	/* .tp_unpack                     = */ NULL,
	/* .tp_unpack_ex                  = */ NULL,
	/* .tp_unpack_ub                  = */ NULL,
};

PRIVATE struct type_seq ds_mv_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_mv_iter,
	/* .tp_sizeob                     = */ NULL, // TODO: (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ds_mv_sizeob,
	/* .tp_contains                   = */ NULL, // TODO: (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ds_mv_contains,
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_foreach                    = */ NULL, // TODO: (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&ds_mv_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_enumerate                  = */ NULL,
	/* .tp_enumerate_index            = */ NULL, // TODO: (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_enumerate_index_t, void *, size_t, size_t))&ds_mv_enumerate_index,
	/* .tp_iterkeys                   = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ NULL, // TODO: (size_t (DCALL *)(DeeObject *__restrict))&ds_mv_size,
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ NULL, // TODO: (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_mv_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ NULL, // TODO: (int (DCALL *)(DeeObject *, size_t))&ds_mv_delitem_index,
	/* .tp_setitem_index              = */ NULL, // TODO: (int (DCALL *)(DeeObject *, size_t, DeeObject *))&ds_mv_setitem_index,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL, // TODO: (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&ds_mv_trygetitem_index,
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
	/* .tp_asvector                   = */ NULL,
	/* .tp_unpack                     = */ NULL,
	/* .tp_unpack_ex                  = */ NULL,
	/* .tp_unpack_ub                  = */ NULL,
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ds_mk_remove(DefaultSequence_MapProxy *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:remove", &key))
		goto err;
	result = DeeMap_InvokeRemove(self->dsmp_map, key);
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
	if unlikely(DeeMap_InvokeRemoveKeys(self->dsmp_map, keys))
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
	item = DeeMap_InvokePopItem(self->dsmp_map);
	if unlikely(!item)
		goto err;
	if (!DeeNone_Check(item)) {
		DREF DeeObject *key_and_value[2];
		int temp = DeeObject_Unpack(item, 2, key_and_value);
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
	if unlikely(DeeSeq_InvokeClear(self->dsmp_map))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE struct type_method tpconst ds_mk_methods[] = {
	/* TODO: byhash(ob) { return Mapping.byhash(this.__map__, ob).map(e -> e.first); } */
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
	/* .tp_name     = */ "_SeqMapKeys",
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ds_mk_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
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
	/* .tp_name     = */ "_SeqMapValues",
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
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ds_mv_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
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
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAP_PROXY_C */
