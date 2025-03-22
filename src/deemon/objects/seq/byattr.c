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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_BYATTR_C
#define GUARD_DEEMON_OBJECTS_SEQ_BYATTR_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/computed-operators.h>
#include <deemon/format.h>
#include <deemon/map.h>
#include <deemon/object.h>
#include <deemon/string.h>

/**/
#include "byattr.h"
#include "../generic-proxy.h"
/**/

#include <stddef.h> /* size_t, offsetof */

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1)) int DCALL
byattr_ctor(MapByAttr *__restrict self) {
	self->mba_map = DeeMapping_NewEmpty();
	return 0;
}

STATIC_ASSERT(offsetof(MapByAttr, mba_map) == offsetof(ProxyObject, po_obj));
#define byattr_copy generic_proxy__copy_alias
#define byattr_deep generic_proxy__deepcopy
#define byattr_init generic_proxy__init

struct byattr_enumattr_foreach_data {
	MapByAttr *befd_self;
	Dee_enum_t befd_proc;
	void      *befd_arg;
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
byattr_enumattr_foreach(void *arg, DeeObject *key, DeeObject *value) {
	Dee_ssize_t result;
	struct byattr_enumattr_foreach_data *cookie;
	cookie = (struct byattr_enumattr_foreach_data *)arg;
	result = 0;
	if (DeeString_Check(key)) {
		result = (*cookie->befd_proc)((DeeObject *)cookie->befd_self,
		                              DeeString_STR(key), NULL,
		                              ATTR_PERMGET | ATTR_PERMDEL | ATTR_PERMSET |
		                              ATTR_IMEMBER | ATTR_PROPERTY | ATTR_NAMEOBJ,
		                              Dee_TYPE(value), cookie->befd_arg);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
byattr_enumattr(DeeTypeObject *tp_self, MapByAttr *self,
                Dee_enum_t proc, void *arg) {
	struct byattr_enumattr_foreach_data cookie;
	(void)tp_self;
	cookie.befd_self = self;
	cookie.befd_proc = proc;
	cookie.befd_arg  = arg;
	/* FIXME: This can invoke user-defined code, which can break big
	 *        time due to the stack switching done by enumattr() */
	return DeeObject_ForeachPair(self->mba_map,
	                             &byattr_enumattr_foreach,
	                             &cookie);
}

STATIC_ASSERT(offsetof(MapByAttr, mba_map) == offsetof(ProxyObject, po_obj));
#define byattr_fini                      generic_proxy__fini
#define byattr_visit                     generic_proxy__visit
#define byattr_getattr                   generic_proxy__map_operator_getitem
#define byattr_delattr                   generic_proxy__map_operator_delitem
#define byattr_setattr                   generic_proxy__map_operator_setitem
#define byattr_hasattr                   generic_proxy__map_operator_hasitem
#define byattr_boundattr                 generic_proxy__map_operator_bounditem
#define byattr_getattr_string_hash       generic_proxy__map_operator_getitem_string_hash
#define byattr_delattr_string_hash       generic_proxy__map_operator_delitem_string_hash
#define byattr_setattr_string_hash       generic_proxy__map_operator_setitem_string_hash
#define byattr_hasattr_string_hash       generic_proxy__map_operator_hasitem_string_hash
#define byattr_boundattr_string_hash     generic_proxy__map_operator_bounditem_string_hash
#define byattr_getattr_string_len_hash   generic_proxy__map_operator_getitem_string_len_hash
#define byattr_delattr_string_len_hash   generic_proxy__map_operator_delitem_string_len_hash
#define byattr_setattr_string_len_hash   generic_proxy__map_operator_setitem_string_len_hash
#define byattr_hasattr_string_len_hash   generic_proxy__map_operator_hasitem_string_len_hash
#define byattr_boundattr_string_len_hash generic_proxy__map_operator_bounditem_string_len_hash


PRIVATE struct type_attr byattr_attr = {
	/* .tp_getattr                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&byattr_getattr,
	/* .tp_delattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *))&byattr_delattr,
	/* .tp_setattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&byattr_setattr,
	/* .tp_enumattr                  = */ (Dee_ssize_t (DCALL *)(DeeTypeObject *, DeeObject *, Dee_enum_t, void *))&byattr_enumattr,
	/* .tp_findattr                  = */ NULL,
	/* .tp_hasattr                   = */ (int (DCALL *)(DeeObject *, DeeObject *))&byattr_hasattr,
	/* .tp_boundattr                 = */ (int (DCALL *)(DeeObject *, DeeObject *))&byattr_boundattr,
	/* .tp_callattr                  = */ NULL,
	/* .tp_callattr_kw               = */ NULL,
	/* .tp_vcallattrf                = */ NULL,
	/* .tp_getattr_string_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&byattr_getattr_string_hash,
	/* .tp_delattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&byattr_delattr_string_hash,
	/* .tp_setattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&byattr_setattr_string_hash,
	/* .tp_hasattr_string_hash       = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&byattr_hasattr_string_hash,
	/* .tp_boundattr_string_hash     = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&byattr_boundattr_string_hash,
	/* .tp_callattr_string_hash      = */ NULL,
	/* .tp_callattr_string_hash_kw   = */ NULL,
	/* .tp_vcallattr_string_hashf    = */ NULL,
	/* .tp_getattr_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&byattr_getattr_string_len_hash,
	/* .tp_delattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&byattr_delattr_string_len_hash,
	/* .tp_setattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&byattr_setattr_string_len_hash,
	/* .tp_hasattr_string_len_hash   = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&byattr_hasattr_string_len_hash,
	/* .tp_boundattr_string_len_hash = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&byattr_boundattr_string_len_hash,
};

PRIVATE struct type_member tpconst byattr_members[] = {
	TYPE_MEMBER_FIELD_DOC("__map__", STRUCT_OBJECT, offsetof(MapByAttr, mba_map),
	                      "->?DMapping\n"
	                      "Underlying mapping"),
	TYPE_MEMBER_END
};


PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
byattr_printrepr(MapByAttr *__restrict self,
                 dformatprinter printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "%r.byattr", self->mba_map);
}

INTERN DeeTypeObject MapByAttr_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapByAttr",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?DMapping)"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&byattr_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&byattr_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&byattr_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&byattr_init,
				TYPE_FIXED_ALLOCATOR(MapByAttr)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&byattr_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&byattr_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&byattr_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &generic_proxy__cmp_recursive,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ &byattr_attr,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ byattr_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__5B2E0F4105586532),
};


/* Create a new byattr proxy for `map' */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
MapByAttr_New(DeeObject *__restrict map) {
	DREF MapByAttr *result;
	result = DeeObject_MALLOC(MapByAttr);
	if likely(result) {
		DeeObject_Init(result, &MapByAttr_Type);
		Dee_Incref(map);
		result->mba_map = map;
	}
	return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_BYATTR_C */
