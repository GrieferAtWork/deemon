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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMATTR_C
#define GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMATTR_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/error.h>
#include <deemon/map.h>
#include <deemon/method-hints.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
/**/

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "map-fromattr.h"
/**/

#include <stddef.h> /* size_t, offsetof */

DECL_BEGIN

/************************************************************************/
/* MapFromAttrKeysIterator                                              */
/************************************************************************/

STATIC_ASSERT(offsetof(MapFromAttrIterator, mfai_iter) == offsetof(ProxyObject, po_obj));
#define mfaki_fini  generic_proxy__fini
#define mfaki_visit generic_proxy__visit
#define mfaki_copy  generic_proxy__copy_recursive
#define mfaki_deep  generic_proxy__deepcopy
#define mfaki_bool  generic_proxy__bool
#define mfaki_cmp   generic_proxy__cmp_recursive

INTDEF WUNUSED NONNULL((1)) DREF DeeAttributeObject *DCALL /* from "../attribute.c" */
enumattriter_next(DeeEnumAttrIteratorObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL /* from "../attribute.c" */
attr_get_name(DeeAttributeObject *__restrict self);

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
mfaki_next(MapFromAttrIterator *__restrict self) {
	DREF DeeStringObject *result;
	DREF DeeAttributeObject *attr;
	ASSERT_OBJECT_TYPE_EXACT(self->mfai_iter, &DeeEnumAttrIterator_Type);
	attr = enumattriter_next(self->mfai_iter);
	if (!ITER_ISOK(attr))
		return (DREF DeeStringObject *)attr;
	result = attr_get_name(attr);
	Dee_Decref(attr);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mfaki_ctor(MapFromAttrIterator *__restrict self) {
	self->mfai_iter = (DREF DeeEnumAttrIteratorObject *)DeeObject_NewDefault(&DeeEnumAttrIterator_Type);
	return likely(self->mfai_iter) ? 0 : -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mfaki_init_fromob(MapFromAttrIterator *__restrict self, DeeObject *ob) {
	DREF DeeEnumAttrObject *enumattr;
	enumattr = (DREF DeeEnumAttrObject *)DeeObject_New(&DeeEnumAttr_Type, 1, (DeeObject *const *)&ob);
	if unlikely(!enumattr)
		goto err;
	self->mfai_iter = (DREF DeeEnumAttrIteratorObject *)DeeObject_Iter((DeeObject *)enumattr);
	Dee_Decref_unlikely(enumattr);
	if unlikely(!self->mfai_iter)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mfaki_init(MapFromAttrIterator *__restrict self,
           size_t argc, DeeObject *const *argv) {
	MapFromAttr *map;
	_DeeArg_Unpack1(err, argc, argv, "_MapFromAttrKeysIterator", &map);
	if (DeeObject_AssertTypeExact(map, &MapFromAttr_Type))
		goto err;
	return mfaki_init_fromob(self, map->mfa_ob);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
mfaki_getob(MapFromAttrIterator *__restrict self) {
	DeeEnumAttrIteratorObject *iter = self->mfai_iter;
	DeeEnumAttrObject *iterseq = iter->ei_seq;
	ASSERT(iterseq->ea_obj);
#ifdef CONFIG_EXPERIMENTAL_ATTRITER
	return_reference(iterseq->ea_obj);
#else /* CONFIG_EXPERIMENTAL_ATTRITER */
	if (iterseq->ea_type == Dee_TYPE(iterseq->ea_obj))
		return_reference(iterseq->ea_obj);
	return DeeSuper_New(iterseq->ea_type, iterseq->ea_obj);
#endif /* !CONFIG_EXPERIMENTAL_ATTRITER */
}

PRIVATE WUNUSED NONNULL((1)) DREF MapFromAttr *DCALL
mfaki_getseq(MapFromAttrIterator *__restrict self) {
	DREF MapFromAttr *result;
	DREF DeeObject *ob = mfaki_getob(self);
	if unlikely(!ob)
		goto err;
	result = DeeObject_MALLOC(MapFromAttr);
	if unlikely(!result)
		goto err_ob;
	result->mfa_ob = ob; /* Inherit reference */
	DeeObject_Init(result, &MapFromAttr_Type);
	return result;
err_ob:
	Dee_Decref(ob);
err:
	return NULL;
}

PRIVATE struct type_getset tpconst mfaki_getsets[] = {
	TYPE_GETTER_AB_F(STR_seq, &mfaki_getseq, METHOD_FNOREFESCAPE, "->?Ert:MapFromAttr"),
	TYPE_GETTER_AB_F_NODOC("__ob__", &mfaki_getob, METHOD_FNOREFESCAPE),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst mfaki_members[] = {
	TYPE_MEMBER_FIELD_DOC("__iter__", STRUCT_OBJECT, offsetof(MapFromAttrIterator, mfai_iter),
	                      "->?Ert:EnumAttrIterator"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject MapFromAttrKeysIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapFromAttrKeysIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?Ert:MapFromAttr)\n"
	                         "\n"
	                         "next->?T2?Dstring?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&mfaki_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&mfaki_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&mfaki_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&mfaki_init,
				TYPE_FIXED_ALLOCATOR(MapFromAttrIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mfaki_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&mfaki_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&mfaki_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &mfaki_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mfaki_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ mfaki_getsets,
	/* .tp_members       = */ mfaki_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__E31EBEB26CC72F83),
};







/************************************************************************/
/* MapFromAttr                                                          */
/************************************************************************/

STATIC_ASSERT(offsetof(MapFromAttr, mfa_ob) == offsetof(ProxyObject, po_obj));
#define mfa_copy  generic_proxy__copy_alias
#define mfa_deep  generic_proxy__deepcopy
#define mfa_init  generic_proxy__init
#define mfa_fini  generic_proxy__fini
#define mfa_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
mfa_ctor(MapFromAttr *__restrict self) {
	self->mfa_ob = DeeNone_NewRef();
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF MapFromAttrIterator *DCALL
mfa_iterkeys(MapFromAttr *__restrict self) {
	DREF MapFromAttrIterator *result;
	result = DeeObject_MALLOC(MapFromAttrIterator);
	if unlikely(!result)
		goto err;
	if unlikely(mfaki_init_fromob(result, self->mfa_ob))
		goto err_r;
	DeeObject_Init(result, &MapFromAttrKeysIterator_Type);
	return result;
err_r:
	DeeObject_FREE(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mfa_getitem(MapFromAttr *self, DeeObject *key) {
	DREF DeeObject *result;
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	result = DeeObject_GetAttr(self->mfa_ob, key);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_UnboundAttribute)) {
			err_unbound_key((DeeObject *)self, key);
		} else if (DeeError_Catch(&DeeError_AttributeError)) {
			err_unknown_key((DeeObject *)self, key);
		}
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mfa_delitem(MapFromAttr *self, DeeObject *key) {
	int result;
	if (!DeeString_Check(key))
		return 0;
	result = DeeObject_DelAttr(self->mfa_ob, key);
	if (unlikely(result) && DeeError_Catch(&DeeError_AttributeError))
		result = 0;
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
mfa_setitem(MapFromAttr *self, DeeObject *key, DeeObject *value) {
	int result;
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	result = DeeObject_SetAttr(self->mfa_ob, key, value);
	if unlikely(result) {
		if (DeeError_Catch(&DeeError_AttributeError))
			err_unknown_key((DeeObject *)self, key);
	}
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mfa_hasitem(MapFromAttr *self, DeeObject *key) {
	if (!DeeString_Check(key))
		return 0;
	return DeeObject_HasAttr(self->mfa_ob, key);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mfa_bounditem(MapFromAttr *self, DeeObject *key) {
	if (!DeeString_Check(key))
		return Dee_BOUND_MISSING;
	return DeeObject_BoundAttr(self->mfa_ob, key);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mfa_contains(MapFromAttr *self, DeeObject *key) {
	int result = mfa_hasitem(self, key);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mfa_getitem_string_hash(MapFromAttr *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result = DeeObject_GetAttrStringHash(self->mfa_ob, key, hash);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_UnboundAttribute)) {
			err_unbound_key_str((DeeObject *)self, key);
		} else if (DeeError_Catch(&DeeError_AttributeError)) {
			err_unknown_key_str((DeeObject *)self, key);
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mfa_delitem_string_hash(MapFromAttr *self, char const *key, Dee_hash_t hash) {
	int result = DeeObject_DelAttrStringHash(self->mfa_ob, key, hash);
	if (unlikely(result) && DeeError_Catch(&DeeError_AttributeError))
		result = 0;
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL
mfa_setitem_string_hash(MapFromAttr *self, char const *key, Dee_hash_t hash, DeeObject *value) {
	int result = DeeObject_SetAttrStringHash(self->mfa_ob, key, hash, value);
	if unlikely(result) {
		if (DeeError_Catch(&DeeError_AttributeError))
			err_unknown_key_str((DeeObject *)self, key);
	}
	return result;
}

STATIC_ASSERT(offsetof(MapFromAttr, mfa_ob) == offsetof(ProxyObject, po_obj));
#define mfa_hasitem_string_hash   generic_proxy__hasattr_string_hash
#define mfa_bounditem_string_hash generic_proxy__boundattr_string_hash


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mfa_getitem_string_len_hash(MapFromAttr *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result = DeeObject_GetAttrStringLenHash(self->mfa_ob, key, keylen, hash);
	if unlikely(!result) {
		if (DeeError_Catch(&DeeError_UnboundAttribute)) {
			err_unbound_key_str_len((DeeObject *)self, key, keylen);
		} else if (DeeError_Catch(&DeeError_AttributeError)) {
			err_unknown_key_str_len((DeeObject *)self, key, keylen);
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
mfa_delitem_string_len_hash(MapFromAttr *self, char const *key, size_t keylen, Dee_hash_t hash) {
	int result = DeeObject_DelAttrStringLenHash(self->mfa_ob, key, keylen, hash);
	if (unlikely(result) && DeeError_Catch(&DeeError_AttributeError))
		result = 0;
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL
mfa_setitem_string_len_hash(MapFromAttr *self, char const *key, size_t keylen,
                            Dee_hash_t hash, DeeObject *value) {
	int result = DeeObject_SetAttrStringLenHash(self->mfa_ob, key, keylen, hash, value);
	if unlikely(result) {
		if (DeeError_Catch(&DeeError_AttributeError))
			err_unknown_key_str_len((DeeObject *)self, key, keylen);
	}
	return result;
}

STATIC_ASSERT(offsetof(MapFromAttr, mfa_ob) == offsetof(ProxyObject, po_obj));
#define mfa_hasitem_string_len_hash   generic_proxy__hasattr_string_len_hash
#define mfa_bounditem_string_len_hash generic_proxy__boundattr_string_len_hash



PRIVATE struct type_seq mfa_seq = {
	/* .tp_iter                       = */ DEFIMPL(&default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem),
	/* .tp_sizeob                     = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mfa_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&mfa_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&mfa_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&mfa_setitem,
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL_UNSUPPORTED(&default__foreach__unsupported),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__map_operator_foreach_pair__with__map_operator_iter),
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&mfa_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&mfa_hasitem,
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__getitem_index__with__getitem),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__delitem_index__with__delitem),
	/* .tp_setitem_index              = */ DEFIMPL(&default__setitem_index__with__setitem),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__bounditem_index__with__bounditem),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__hasitem_index__with__hasitem),
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__trygetitem__with__getitem),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__trygetitem_index__with__getitem_index),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__getitem_string_hash),
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&mfa_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&mfa_delitem_string_hash,
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&mfa_setitem_string_hash,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&mfa_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&mfa_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__getitem_string_len_hash),
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&mfa_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&mfa_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&mfa_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&mfa_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&mfa_hasitem_string_len_hash,
	/* .tp_asvector                   = */ NULL,
	/* .tp_asvector_nothrow           = */ NULL,
};

PRIVATE struct type_getset tpconst mfa_getsets[] = {
	TYPE_GETTER_AB_F(STR___map_iterkeys__, &mfa_iterkeys, METHOD_FNOREFESCAPE,
	                 "->?Ert:MapFromAttrKeysIterator"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst mfa_members[] = {
	TYPE_MEMBER_FIELD("__ob__", STRUCT_OBJECT, offsetof(MapFromAttr, mfa_ob)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst mfa_class_members[] = {
	TYPE_MEMBER_CONST(STR_KeysIterator, &MapFromAttrKeysIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject MapFromAttr_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapFromAttr",
	/* .tp_doc      = */ DOC("(ob)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMapping_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&mfa_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&mfa_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&mfa_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&mfa_init,
				TYPE_FIXED_ALLOCATOR(MapFromAttr)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&mfa_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_iter),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&map_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&mfa_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__E5A99B058858326C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__2BD018178123F93E),
	/* .tp_seq           = */ &mfa_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ mfa_getsets,
	/* .tp_members       = */ mfa_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ mfa_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__5B2E0F4105586532),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMATTR_C */
