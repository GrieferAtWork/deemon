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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMATTR_C
#define GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMATTR_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_*, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack1 */
#include <deemon/bool.h>               /* Dee_True, return_bool */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error-rt.h>           /* DeeRT_Err* */
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/map.h>                /* DeeMapping_Type */
#include <deemon/mro.h>                /* DeeObject_IterAttr, Dee_ATTRPERM_F_NAMEOBJ, Dee_ITERATTR_DEFAULT_BUFSIZE, Dee_attrdesc, Dee_attrdesc_fini, Dee_attrdesc_nameobj, Dee_attrhint, Dee_attrhint_initall, Dee_attriter_*, _Dee_attrdesc_fini_WITHOUT_NAME */
#include <deemon/none.h>               /* DeeNone_NewRef, Dee_None */
#include <deemon/object.h>             /* DeeObject_*, Dee_BOUND_MISSING, Dee_Decref, Dee_Incref */
#include <deemon/seq.h>                /* DeeIterator_Type */
#include <deemon/string.h>             /* DeeString* */
#include <deemon/system-features.h>    /* memset */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_VAR, Dee_Visit, Dee_visit_t, METHOD_FNOREFESCAPE, STRUCT_*, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/types.h>              /* DREF, DeeObject, DeeTypeObject, Dee_AsObject, Dee_TYPE, Dee_hash_t, ITER_DONE, OBJECT_HEAD_INIT */

#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "map-fromattr.h"

#include <stddef.h> /* NULL, offsetof, ptrdiff_t, size_t */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

/************************************************************************/
/* MapFromAttrKeysIterator                                              */
/************************************************************************/

PRIVATE NONNULL((1)) void DCALL
mfaki_fini(MapFromAttrIterator *__restrict self) {
	/* !!! Iterator must be finalized while still holding reference to `self->mfai_obj' */
	Dee_attriter_fini(&self->mfai_iter);
	Dee_Decref(self->mfai_obj);
}

PRIVATE NONNULL((1, 2)) void DCALL
mfaki_visit(MapFromAttrIterator *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_attriter_visit(&self->mfai_iter);
	Dee_Visit(self->mfai_obj);
}

PRIVATE NONNULL((1)) DREF MapFromAttrIterator *DCALL
mfaki_ofobj(DeeObject *__restrict ob) {
	struct Dee_attrhint hint;
	DREF MapFromAttrIterator *result;
	size_t req_bufsize;
	size_t cur_bufsize = Dee_ITERATTR_DEFAULT_BUFSIZE;
	result = (DREF MapFromAttrIterator *)DeeObject_Malloc(offsetof(MapFromAttrIterator, mfai_iter) +
	                                                      cur_bufsize);
	if unlikely(!result)
		goto err;
	Dee_attrhint_initall(&hint);
again_iterattr:
	req_bufsize = DeeObject_IterAttr(Dee_TYPE(ob), ob, &result->mfai_iter, cur_bufsize, &hint);
	if unlikely(req_bufsize == (size_t)-1)
		goto err_r;
	if (req_bufsize > cur_bufsize) {
		DREF MapFromAttrIterator *new_result;
		new_result = (DREF MapFromAttrIterator *)DeeObject_Realloc(result,
		                                                           offsetof(MapFromAttrIterator, mfai_iter) +
		                                                           req_bufsize);
		if unlikely(!new_result)
			goto err_r;
		result      = new_result;
		cur_bufsize = req_bufsize;
		goto again_iterattr;
	} else if (req_bufsize < cur_bufsize) {
		/* Free unused memory */
		DREF MapFromAttrIterator *new_result;
		new_result = (DREF MapFromAttrIterator *)DeeObject_TryRealloc(result,
		                                                              offsetof(MapFromAttrIterator, mfai_iter) +
		                                                              req_bufsize);
		if (likely(new_result) && unlikely(result != new_result)) {
			/* Special case: must update pointers within the iterator
			 *               to reflect the new memory location. */
			ptrdiff_t delta;
			delta  = (byte_t *)new_result - (byte_t *)result;
			result = new_result;
			Dee_attriter_moved(&result->mfai_iter, delta);
		}
	}
	result->mfai_itsz = req_bufsize;
	result->mfai_obj  = ob;
	Dee_Incref(ob);
	DeeObject_Init(result, &MapFromAttrKeysIterator_Type);
	return result;
err_r:
	DeeObject_Free(result);
err:
	return NULL;
}

PRIVATE NONNULL((1)) DREF MapFromAttrIterator *DCALL
mfaki_copy(MapFromAttrIterator *__restrict self) {
	DREF MapFromAttrIterator *result;
	result = (DREF MapFromAttrIterator *)DeeObject_Malloc(offsetof(MapFromAttrIterator, mfai_iter) +
	                                                      self->mfai_itsz);
	if unlikely(!result)
		goto err;
	if unlikely(Dee_attriter_copy(&result->mfai_iter, &self->mfai_iter, self->mfai_itsz))
		goto err_r;
	result->mfai_obj = self->mfai_obj;
	Dee_Incref(self->mfai_obj);
	result->mfai_itsz = self->mfai_itsz;
	DeeObject_Init(result, &MapFromAttrKeysIterator_Type);
	return result;
err_r:
	DeeObject_Free(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
mfaki_bool(MapFromAttrIterator *__restrict self) {
	return Dee_attriter_bool(&self->mfai_iter, self->mfai_itsz);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
mfaki_next(MapFromAttrIterator *__restrict self) {
	int status;
	struct Dee_attrdesc desc;
	DREF DeeStringObject *result;
	DBG_memset(&desc, 0xcc, sizeof(desc));
	status = Dee_attriter_next(&self->mfai_iter, &desc);
	if (status != 0) {
		if likely(status > 0)
			return (DREF DeeStringObject *)ITER_DONE;
		goto err;
	}
	if (desc.ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
		result = Dee_attrdesc_nameobj(&desc);
		_Dee_attrdesc_fini_WITHOUT_NAME(&desc);
	} else {
		result = (DREF DeeStringObject *)DeeString_New(desc.ad_name);
		Dee_attrdesc_fini(&desc);
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED DREF MapFromAttrIterator *DCALL mfaki_ctor(void) {
	return mfaki_ofobj(Dee_None);
}

PRIVATE WUNUSED DREF MapFromAttrIterator *DCALL
mfaki_init(size_t argc, DeeObject *const *argv) {
	MapFromAttr *map;
	DeeArg_Unpack1(err, argc, argv, "_MapFromAttrKeysIterator", &map);
	if (DeeObject_AssertTypeExact(map, &MapFromAttr_Type))
		goto err;
	return mfaki_ofobj(map->mfa_ob);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF MapFromAttrIterator *DCALL
mfaki_class_of(DeeTypeObject *UNUSED(tp_self), size_t argc, DeeObject *const *argv) {
	DeeObject *ob;
	DeeArg_Unpack1(err, argc, argv, "of", &ob);
	return mfaki_ofobj(ob);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF MapFromAttr *DCALL
mfaki_getseq(MapFromAttrIterator *__restrict self) {
	DREF MapFromAttr *result;
	result = DeeObject_MALLOC(MapFromAttr);
	if unlikely(!result)
		goto err;
	result->mfa_ob = self->mfai_obj;
	Dee_Incref(result->mfa_ob);
	DeeObject_Init(result, &MapFromAttr_Type);
	return result;
err:
	return NULL;
}

PRIVATE struct type_getset tpconst mfaki_getsets[] = {
	TYPE_GETTER_AB_F(STR_seq, &mfaki_getseq, METHOD_FNOREFESCAPE, "->?Ert:MapFromAttr"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst mfaki_members[] = {
	TYPE_MEMBER_FIELD("__ob__", STRUCT_OBJECT_AB, offsetof(MapFromAttrIterator, mfai_obj)),
	TYPE_MEMBER_FIELD("__itersz__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(MapFromAttrIterator, mfai_itsz)),
	TYPE_MEMBER_END
};

PRIVATE struct type_method tpconst mfaki_class_methods[] = {
	TYPE_METHOD_F("of", &mfaki_class_of, METHOD_FNOREFESCAPE,
	              "(ob)->?.\n"
	              "Convenience wrapper for ${Mapping.fromattr(ob).keys.operator iter()}"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject MapFromAttrKeysIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_MapFromAttrKeysIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(map:?Ert:MapFromAttr)\n"
	                         "\n"
	                         "next->?T2?Dstring?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FVARIABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &mfaki_ctor,
			/* tp_copy_ctor:   */ &mfaki_copy,
			/* tp_any_ctor:    */ &mfaki_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL, /* Not serializable (would require an extra operator in `struct Dee_attriter_type') */
			/* tp_free:        */ NULL
		),
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&mfaki_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__27F47A9BEBC0B992),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&mfaki_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__712535FF7E4C26E5),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ mfaki_getsets,
	/* .tp_members       = */ mfaki_members,
	/* .tp_class_methods = */ mfaki_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};







/************************************************************************/
/* MapFromAttr                                                          */
/************************************************************************/

STATIC_ASSERT(offsetof(MapFromAttr, mfa_ob) == offsetof(ProxyObject, po_obj));
#define mfa_copy      generic_proxy__copy_alias
#define mfa_init      generic_proxy__init
#define mfa_serialize generic_proxy__serialize
#define mfa_fini      generic_proxy__fini
#define mfa_visit     generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
mfa_ctor(MapFromAttr *__restrict self) {
	self->mfa_ob = DeeNone_NewRef();
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF MapFromAttrIterator *DCALL
mfa_iterkeys(MapFromAttr *__restrict self) {
	return mfaki_ofobj(self->mfa_ob);
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
mfa_getitem(MapFromAttr *self, DeeObject *key) {
	DREF DeeObject *result;
	if (DeeObject_AssertTypeExact(key, &DeeString_Type))
		goto err;
	result = DeeObject_GetAttr(self->mfa_ob, key);
	if unlikely(!result) {
		DREF DeeObject *error;
		if ((error = DeeError_CatchError(&DeeError_UnboundAttribute)) != NULL) {
			DeeRT_ErrUnboundKeyWithCause(Dee_AsObject(self), key, error);
		} else if ((error = DeeError_CatchError(&DeeError_AttributeError)) != NULL) {
			DeeRT_ErrUnknownKeyWithCause(Dee_AsObject(self), key, error);
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
		DREF DeeObject *error;
		if ((error = DeeError_CatchError(&DeeError_AttributeError)) != NULL)
			DeeRT_ErrUnknownKeyWithCause(Dee_AsObject(self), key, error);
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
		DREF DeeObject *error;
		if ((error = DeeError_CatchError(&DeeError_UnboundAttribute)) != NULL) {
			DeeRT_ErrUnboundKeyStrWithCause(Dee_AsObject(self), key, error);
		} else if ((error = DeeError_CatchError(&DeeError_AttributeError)) != NULL) {
			DeeRT_ErrUnknownKeyStrWithCause(Dee_AsObject(self), key, error);
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
		DREF DeeObject *error;
		if ((error = DeeError_CatchError(&DeeError_AttributeError)) != NULL)
			DeeRT_ErrUnboundKeyStrWithCause(Dee_AsObject(self), key, error);
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
		DREF DeeObject *error;
		if ((error = DeeError_CatchError(&DeeError_UnboundAttribute)) != NULL) {
			DeeRT_ErrUnboundKeyStrLenWithCause(Dee_AsObject(self), key, keylen, error);
		} else if ((error = DeeError_CatchError(&DeeError_AttributeError)) != NULL) {
			DeeRT_ErrUnknownKeyStrLenWithCause(Dee_AsObject(self), key, keylen, error);
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
		DREF DeeObject *error;
		if ((error = DeeError_CatchError(&DeeError_AttributeError)) != NULL)
			DeeRT_ErrUnknownKeyStrLenWithCause(Dee_AsObject(self), key, keylen, error);
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
	TYPE_MEMBER_FIELD("__ob__", STRUCT_OBJECT_AB, offsetof(MapFromAttr, mfa_ob)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst mfa_class_members[] = {
	TYPE_MEMBER_CONST(STR_KeysIterator, &MapFromAttrKeysIterator_Type),
	TYPE_MEMBER_CONST("__map_getitem_always_bound__", Dee_True),
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
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ MapFromAttr,
			/* tp_ctor:        */ &mfa_ctor,
			/* tp_copy_ctor:   */ &mfa_copy,
			/* tp_any_ctor:    */ &mfa_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &mfa_serialize
		),
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
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&mfa_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__E5A99B058858326C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__4CF94EE41850B0EF),
	/* .tp_seq           = */ &mfa_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
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
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_MAP_FROMATTR_C */
