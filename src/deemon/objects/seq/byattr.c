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
#include <deemon/method-hints.h>
#include <deemon/mro.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/thread.h>
/**/

#include "../generic-proxy.h"
#include "byattr.h"
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

PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
byattr_iterattr(DeeTypeObject *UNUSED(tp_self), MapByAttr *self,
                struct Dee_attriter *iterbuf, size_t bufsize,
                struct Dee_attrhint const *__restrict hint);
PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
byattr_findattr(DeeTypeObject *UNUSED(tp_self), MapByAttr *self,
                struct Dee_attrspec const *__restrict specs,
                struct Dee_attrdesc *__restrict result);

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
	/* .tp_iterattr                  = */ (size_t (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attriter *, size_t, struct Dee_attrhint const *__restrict))&byattr_iterattr,
	/* .tp_findattr                  = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct Dee_attrspec const *__restrict, struct Dee_attrdesc *__restrict))&byattr_findattr,
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

struct byattr_attriter {
	Dee_ATTRITER_HEAD
	DREF DeeObject *mba_keysiter; /* [1..1][const] KeysIterator for the underlying mapping. */
};

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
byattr_attriter_next(struct byattr_attriter *__restrict self,
                     /*out*/ struct Dee_attrdesc *__restrict desc) {
	DREF DeeObject *key;
again:
	key = DeeObject_IterNext(self->mba_keysiter);
	if (!ITER_ISOK(key)) {
		if unlikely(!key)
			goto err;
		return 1;
	}
	if unlikely(!DeeString_Check(key)) {
		Dee_Decref(key);
		if (DeeThread_CheckInterrupt())
			goto err;
		goto again;
	}
	desc->ad_name = DeeString_STR(key); /* Inherit reference */
	desc->ad_doc  = NULL;
	desc->ad_perm = Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET |
	                Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_PROPERTY | Dee_ATTRPERM_F_NAMEOBJ;
	desc->ad_info.ai_decl = (DeeObject *)&MapByAttr_Type;
	desc->ad_info.ai_type = Dee_ATTRINFO_CUSTOM;
	desc->ad_info.ai_value.v_custom = &byattr_attr;
	desc->ad_type = NULL;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
byattr_attriter_copy(struct byattr_attriter *__restrict self,
                     struct byattr_attriter *__restrict other,
                     size_t other_bufsize) {
	(void)other_bufsize;
	self->mba_keysiter = DeeObject_Copy(other->mba_keysiter);
	if unlikely(!self->mba_keysiter)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
byattr_attriter_fini(struct byattr_attriter *__restrict self) {
	Dee_Decref_likely(self->mba_keysiter);
}

PRIVATE NONNULL((1)) void DCALL
byattr_attriter_visit(struct byattr_attriter *__restrict self,
                      Dee_visit_t proc, void *arg) {
	Dee_Visit(self->mba_keysiter);
}

PRIVATE struct Dee_attriter_type tpconst byattr_attriter_type = {
	/* .ait_next  = */ (int (DCALL *)(struct Dee_attriter *__restrict, /*out*/ struct Dee_attrdesc *__restrict))&byattr_attriter_next,
	/* .ait_copy  = */ (int (DCALL *)(struct Dee_attriter *__restrict, struct Dee_attriter *__restrict, size_t))&byattr_attriter_copy,
	/* .ait_fini  = */ (void (DCALL *)(struct Dee_attriter *__restrict))&byattr_attriter_fini,
	/* .ait_visit = */ (void (DCALL *)(struct Dee_attriter *__restrict, Dee_visit_t, void *))&byattr_attriter_visit,
};


PRIVATE WUNUSED NONNULL((1, 2, 5)) size_t DCALL
byattr_iterattr(DeeTypeObject *UNUSED(tp_self), MapByAttr *self,
                struct Dee_attriter *iterbuf, size_t bufsize,
                struct Dee_attrhint const *__restrict UNUSED(hint)) {
	struct byattr_attriter *iter = (struct byattr_attriter *)iterbuf;
	if (bufsize >= sizeof(struct byattr_attriter)) {
		iter->mba_keysiter = DeeObject_InvokeMethodHint(map_iterkeys, self->mba_map);
		if unlikely(!iter->mba_keysiter)
			goto err;
		Dee_attriter_init(iter, &byattr_attriter_type);
	}
	return sizeof(struct byattr_attriter);
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
byattr_findattr(DeeTypeObject *UNUSED(tp_self), MapByAttr *self,
                struct Dee_attrspec const *__restrict specs,
                struct Dee_attrdesc *__restrict result) {
	int has = DeeObject_InvokeMethodHint(map_operator_hasitem_string_hash,
	                                     self->mba_map, specs->as_name,
	                                     specs->as_hash);
	if unlikely(has < 0)
		goto err;
	if (has) {
		result->ad_name = specs->as_name;
		result->ad_doc  = NULL;
		result->ad_perm = Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET |
		                  Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_PROPERTY;
		result->ad_info.ai_type = Dee_ATTRINFO_CUSTOM;
		result->ad_info.ai_decl = (DeeObject *)self;
		result->ad_info.ai_value.v_custom = &byattr_attr;
		result->ad_type = NULL;
		return 0;
	}
	return 1;
err:
	return -1;
}


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
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
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
