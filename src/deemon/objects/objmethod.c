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
#ifndef GUARD_DEEMON_OBJECTS_OBJMETHOD_C
#define GUARD_DEEMON_OBJECTS_OBJMETHOD_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/callable.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* _Exit(), abort() */
#include <deemon/thread.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "generic-proxy.h"
/**/

#include <stdarg.h> /* va_list */
#include <stddef.h> /* size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

/* Construct a new `_ObjMethod' object. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObjMethod_New(Dee_objmethod_t func, DeeObject *__restrict self) {
	DREF DeeObjMethodObject *result;
	ASSERT_OBJECT(self);
	result = DeeObject_MALLOC(DeeObjMethodObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeObjMethod_Type);
	result->om_func = func;
	result->om_this = self;
	Dee_Incref(self);
done:
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeKwObjMethod_New(Dee_kwobjmethod_t func, DeeObject *__restrict self) {
	DREF DeeKwObjMethodObject *result;
	ASSERT_OBJECT(self);
	result = DeeObject_MALLOC(DeeKwObjMethodObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeKwObjMethod_Type);
	result->om_func = func;
	result->om_this = self;
	Dee_Incref(self);
done:
	return (DREF DeeObject *)result;
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) bool DCALL
typeobject_find_objmethod(DeeTypeObject *__restrict self, Dee_objmethod_t meth,
                          struct objmethod_origin *__restrict result) {
	DeeTypeMRO mro;
	DeeTypeMRO_Init(&mro, self);
	do {
		struct type_method const *iter;
		iter = self->tp_class_methods;
		if (iter)
			for (; iter->m_name; ++iter) {
				if (iter->m_func == meth) {
					result->omo_type  = self;
					result->omo_chain = self->tp_class_methods;
					result->omo_decl  = iter;
					return true;
				}
			}
	} while ((self = DeeTypeMRO_Next(&mro, self)) != NULL);
	return false;
}

/* Lookup the origin of the function bound by
 * the given `_ObjMethod', or `NULL' if unknown. */
PUBLIC WUNUSED NONNULL((1)) bool DCALL
DeeObjMethod_GetOrigin(DeeObject const *__restrict self,
                       struct objmethod_origin *__restrict result) {
	Dee_objmethod_t func;
	DeeTypeObject *tp_self;
	DeeTypeMRO mro;
	ASSERT_OBJECT(self);
	ASSERT(DeeObjMethod_Check(self) || DeeKwObjMethod_Check(self));
	func    = DeeObjMethod_FUNC(self);
	tp_self = Dee_TYPE(DeeObjMethod_SELF(self));
	DeeTypeMRO_Init(&mro, tp_self);
	do {
		struct type_method const *iter;
		if (tp_self == &DeeType_Type) {
			if (typeobject_find_objmethod((DeeTypeObject *)DeeObjMethod_SELF(self), func, result))
				return true;
		}
		iter = tp_self->tp_methods;
		if (iter)
			for (; iter->m_name; ++iter) {
				if (iter->m_func == func) {
					result->omo_type  = tp_self;
					result->omo_chain = tp_self->tp_methods;
					result->omo_decl  = iter;
					return true;
				}
			}
	} while ((tp_self = DeeTypeMRO_Next(&mro, tp_self)) != NULL);
	return false;
}

/* Lookup the origin of the function bound by
 * the given `_ClsMethod', or `NULL' if unknown. */
PUBLIC WUNUSED NONNULL((1)) bool DCALL
DeeClsMethod_GetOrigin(DeeObject const *__restrict self,
                       struct objmethod_origin *__restrict result) {
	Dee_objmethod_t func;
	struct type_method const *iter;
	ASSERT_OBJECT(self);
	ASSERT(DeeClsMethod_Check(self) || DeeKwClsMethod_Check(self));
	func = DeeClsMethod_FUNC(self);
	iter = DeeClsMethod_TYPE(self)->tp_methods;
	if (iter) {
		for (; iter->m_name; ++iter) {
			if (iter->m_func == func) {
				result->omo_type  = DeeClsMethod_TYPE(self);
				result->omo_chain = DeeClsMethod_TYPE(self)->tp_methods;
				result->omo_decl  = iter;
				return true;
			}
		}
	}
	return false;
}


STATIC_ASSERT(offsetof(DeeObjMethodObject, om_this) == offsetof(ProxyObject, po_obj));
#define objmethod_fini  generic_proxy__fini
#define objmethod_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
objmethod_call(DeeObjMethodObject *self, size_t argc, DeeObject *const *argv) {
	return DeeObjMethod_CallFunc(self->om_func, self->om_this, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
objmethod_hash(DeeObjMethodObject *__restrict self) {
	return Dee_HashCombine(Dee_HashPointer(self->om_func),
	                       DeeObject_Hash(self->om_this));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
objmethod_compare_eq(DeeObjMethodObject *self, DeeObjMethodObject *other) {
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	Dee_return_compare_if_ne(self->om_func, other->om_func);
	return DeeObject_CompareEq(self->om_this, other->om_this);
err:
	return Dee_COMPARE_ERR;
}


PRIVATE struct type_cmp objmethod_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&objmethod_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&objmethod_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL_UNSUPPORTED(&default__lo__unsupported),
	/* .tp_le            = */ DEFIMPL_UNSUPPORTED(&default__le__unsupported),
	/* .tp_gr            = */ DEFIMPL_UNSUPPORTED(&default__gr__unsupported),
	/* .tp_ge            = */ DEFIMPL_UNSUPPORTED(&default__ge__unsupported),
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
objmethod_bound_origin(DeeObjMethodObject *__restrict self) {
	struct objmethod_origin origin;
	bool bound = DeeObjMethod_GetOrigin((DeeObject *)self, &origin);
	return Dee_BOUND_FROMBOOL(bound);
}

#define objmethod_bound_func objmethod_bound_origin
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
objmethod_get_func(DeeObjMethodObject *__restrict self) {
	struct objmethod_origin origin;
	if likely(DeeObjMethod_GetOrigin((DeeObject *)self, &origin))
		return DeeClsMethod_New(origin.omo_type, self->om_func);
	return DeeRT_ErrUnboundAttrCStr(self, "__func__");
}

#define objmethod_bound_name objmethod_bound_origin
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
objmethod_get_name(DeeObjMethodObject *__restrict self) {
	struct objmethod_origin origin;
	if likely(DeeObjMethod_GetOrigin((DeeObject *)self, &origin))
		return DeeString_NewAuto(origin.omo_decl->m_name);
	return DeeRT_ErrUnboundAttr(self, &str___name__);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
objmethod_get_doc(DeeObjMethodObject *__restrict self) {
	struct objmethod_origin origin;
	if (likely(DeeObjMethod_GetOrigin((DeeObject *)self, &origin)) &&
	    origin.omo_decl->m_doc != NULL)
		return DeeString_NewAutoUtf8(origin.omo_decl->m_doc);
	return_none;
}

#define objmethod_bound_type objmethod_bound_origin
PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
objmethod_get_type(DeeObjMethodObject *__restrict self) {
	struct objmethod_origin origin;
	if likely(DeeObjMethod_GetOrigin((DeeObject *)self, &origin))
		return_reference_(origin.omo_type);
	return (DREF DeeTypeObject *)DeeRT_ErrUnboundAttr(self, &str___type__);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
objmethod_get_module(DeeObjMethodObject *__restrict self) {
	struct objmethod_origin origin;
	if likely(DeeObjMethod_GetOrigin((DeeObject *)self, &origin)) {
		DREF DeeObject *result = DeeType_GetModule(origin.omo_type);
		if likely(result)
			return result;
	}
	return DeeRT_ErrUnboundAttr(self, &str___module__);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
objmethod_bound_module(DeeObjMethodObject *__restrict self) {
	struct objmethod_origin origin;
	if likely(DeeObjMethod_GetOrigin((DeeObject *)self, &origin)) {
		DREF DeeObject *result = DeeType_GetModule(origin.omo_type);
		if likely(result) {
			Dee_Decref_unlikely(result);
			return 1;
		}
	}
	return 0;
}

DOC_DEF(objmethod_get_func_doc,
        "->?DCallable\n"
        "#t{UnboundAttribute}"
        "The unbound class-function that is being bound by this object-method");
DOC_DEF(objmethod_get_name_doc,
        "->?Dstring\n"
        "#t{UnboundAttribute}"
        "The name of the function");
DOC_DEF(objmethod_get_doc_doc,
        "->?X2?Dstring?N\n"
        "The documentation string of the function being bound, or ?N if unknown");
DOC_DEF(objmethod_get_kwds_doc,
        "->?S?Dstring\n"
        "#t{UnboundAttribute}"
        "Returns a sequence of keyword argument names accepted by @this function\n"
        "If @this function doesn't accept keyword arguments, an empty sequence is returned");
DOC_DEF(objmethod_get_type_doc,
        "->?DType\n"
        "#t{UnboundAttribute}"
        "The type implementing the function that is being bound");
DOC_DEF(objmethod_get_module_doc,
        "->?DModule\n"
        "#t{UnboundAttribute}"
        "The module implementing the function that is being bound");

PRIVATE struct type_getset tpconst objmethod_getsets[] = {
	TYPE_GETTER_BOUND_F("__func__", &objmethod_get_func, &objmethod_bound_func,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    DOC_GET(objmethod_get_func_doc)),
	TYPE_GETTER_BOUND_F(STR___name__, &objmethod_get_name, &objmethod_bound_name,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    DOC_GET(objmethod_get_name_doc)),
	TYPE_GETTER_AB_F(STR___doc__, &objmethod_get_doc,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 DOC_GET(objmethod_get_doc_doc)),
	TYPE_GETTER_BOUND_F(STR___type__, &objmethod_get_type, &objmethod_bound_type,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    DOC_GET(objmethod_get_type_doc)),
	TYPE_GETTER_BOUND_F(STR___module__, &objmethod_get_module, &objmethod_bound_module,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    DOC_GET(objmethod_get_module_doc)),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst objmethod_members[] = {
	TYPE_MEMBER_FIELD_DOC("__this__", STRUCT_OBJECT, offsetof(DeeObjMethodObject, om_this),
	                      "The object to which @this object-method is bound"),
#define OBJMETHOD_MEMBERS_INDEXOF_KWDS 1
	TYPE_MEMBER_CONST_DOC(STR___kwds__, Dee_EmptySeq, objmethod_get_kwds_doc), /* NOTE: _MUST_ always come last! */
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
objmethod_print(DeeObjMethodObject *__restrict self,
                Dee_formatprinter_t printer, void *arg) {
	struct objmethod_origin origin;
	if likely(DeeObjMethod_GetOrigin((DeeObject *)self, &origin)) {
		return DeeFormat_Printf(printer, arg,
		                        "<object method %k.%s, bound to %r>",
		                        origin.omo_type,
		                        origin.omo_decl->m_name, self->om_this);
	} else {
		return DeeFormat_Printf(printer, arg,
		                        "<object method <unknown>.%s, bound to %r>",
		                        origin.omo_decl->m_name, self->om_this);
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
objmethod_printrepr(DeeObjMethodObject *__restrict self,
                    Dee_formatprinter_t printer, void *arg) {
	struct objmethod_origin origin;
	char const *name = "<unknown>";
	if likely(DeeObjMethod_GetOrigin((DeeObject *)self, &origin))
		name = origin.omo_decl->m_name;
	return DeeFormat_Printf(printer, arg, "%r.%s", self->om_this, name);
}


PRIVATE struct type_operator const objmethod_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FIELDS_CONSTSTR),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FIELDS_CONSTREPR),
	TYPE_OPERATOR_FLAGS(OPERATOR_000A_CALL, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FUNC_IS_CONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FIELDS_CONSTCMPEQ),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FIELDS_CONSTCMPEQ),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FIELDS_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FIELDS_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FIELDS_CONSTCMP),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FIELDS_CONSTCMP),
};

PUBLIC DeeTypeObject DeeObjMethod_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ObjMethod",
	/* .tp_doc      = */ DOC("call(args!)->"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeObjMethodObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&objmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&objmethod_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&objmethod_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&objmethod_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &objmethod_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ objmethod_getsets,
	/* .tp_members       = */ objmethod_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&objmethod_call,
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ objmethod_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(objmethod_operators),
};


typedef struct {
	PROXY_OBJECT_HEAD(dk_owner) /* [1..1][const] The owner of `dk_start'. */
	char const       *dk_start; /* [1..1][const] Doc string. */
} DocKwds;

typedef struct {
	PROXY_OBJECT_HEAD_EX(DocKwds, dki_kwds) /* [1..1][const] The associated sequence. */
	DWEAK char const             *dki_iter; /* [1..1] Iterator position (start of next keyword name). */
} DocKwdsIterator;
#define DOCKWDSITER_RDITER(self) atomic_read(&(self)->dki_iter)

INTDEF DeeTypeObject DocKwds_Type;
INTDEF DeeTypeObject DocKwdsIterator_Type;

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dockwdsiter_next(DocKwdsIterator *__restrict self) {
	char const *pos, *newpos;
	char const *name_end;
	bool is_escaped;
	for (;;) {
		pos        = atomic_read(&self->dki_iter);
		newpos     = pos;
		is_escaped = false;
		for (;; ++newpos) {
			char ch = *newpos;
			if (!ch)
				break;
			if (ch == '\\') {
				++newpos;
				if (!*newpos)
					break;
				is_escaped = true;
				continue;
			}
			if (ch == ',' || ch == ')' ||
			    ch == ':' || ch == '!' ||
			    ch == '?')
				break;
		}
		name_end = newpos;
		if (*newpos == '?') {
			++newpos;
		} else if (*newpos == '!') {
			++newpos;
			if (*newpos == '!')
				++newpos;
		}
		if (*newpos == ':') {
			++newpos;
			for (;; ++newpos) {
				char ch = *newpos;
				if (ch == '\\') {
					++newpos;
					if (!*newpos)
						break;
					continue;
				}
				if (ch == ',') {
					++newpos;
					break;
				}
				if (ch == ')')
					break;
			}
		}
		if (atomic_cmpxch_weak_or_write(&self->dki_iter, pos, newpos))
			break;
	}
	if (pos >= name_end)
		return ITER_DONE;
	if (is_escaped) {
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		char const *iter, *flush_start;
		iter = flush_start = pos;
		for (; iter < name_end; ++iter) {
			if (*iter != '\\')
				continue;
			if (unicode_printer_print(&printer, flush_start, (size_t)(iter - flush_start)) < 0)
				goto err_printer;
			flush_start = ++iter;
			ASSERT(iter <= name_end);
			if (iter >= name_end)
				break;
		}
		if (unicode_printer_print(&printer, flush_start, (size_t)(name_end - flush_start)) < 0)
			goto err_printer;
		return unicode_printer_pack(&printer);
err_printer:
		unicode_printer_fini(&printer);
		return NULL;
	}
	return DeeString_NewUtf8(pos,
	                         (size_t)(name_end - pos),
	                         STRING_ERROR_FIGNORE);
}


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dockwdsiter_copy(DocKwdsIterator *__restrict self,
                 DocKwdsIterator *__restrict other) {
	self->dki_kwds = other->dki_kwds;
	self->dki_iter = DOCKWDSITER_RDITER(other);
	Dee_Incref(self->dki_kwds);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dockwdsiter_init(DocKwdsIterator *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	_DeeArg_Unpack1(err, argc, argv, "_DocKwdsIterator", &self->dki_kwds);
	if (DeeObject_AssertTypeExact(self->dki_kwds, &DocKwds_Type))
		goto err;
	Dee_Incref(self->dki_kwds);
	self->dki_iter = self->dki_kwds->dk_start + 1;
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(DocKwdsIterator, dki_kwds) == offsetof(ProxyObject, po_obj));
#define dockwdsiter_fini  generic_proxy__fini
#define dockwdsiter_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
dockwdsiter_hash(DocKwdsIterator *self) {
	return Dee_HashPointer(DOCKWDSITER_RDITER(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dockwdsiter_compare(DocKwdsIterator *self, DocKwdsIterator *other) {
	if (DeeObject_AssertTypeExact(other, &DocKwdsIterator_Type))
		goto err;
	Dee_return_compareT(char const *, DOCKWDSITER_RDITER(self),
	                    /*         */ DOCKWDSITER_RDITER(other));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp dockwdsiter_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&dockwdsiter_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&dockwdsiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dockwdsiter_getdocstr(DocKwdsIterator *__restrict self) {
	return DeeString_NewAutoUtf8(self->dki_kwds->dk_start);
}

PRIVATE struct type_getset tpconst dockwdsiter_getsets[] = {
	TYPE_GETTER_AB_F("__doc__", &dockwdsiter_getdocstr,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dstring"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst dockwdsiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(DocKwdsIterator, dki_kwds), "->?Ert:DocKwds"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DocKwdsIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DocKwdsIterator",
	/* .tp_doc      = */ DOC("(kwds:?Ert:DocKwds)\n"
	                         "\n"
	                         "next->?Dstring"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)&dockwdsiter_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)&dockwdsiter_init,
				TYPE_FIXED_ALLOCATOR(DocKwdsIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dockwdsiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dockwdsiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &dockwdsiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dockwdsiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ dockwdsiter_getsets,
	/* .tp_members       = */ dockwdsiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


STATIC_ASSERT(offsetof(DocKwds, dk_owner) == offsetof(ProxyObject, po_obj));
#define dockwds_fini  generic_proxy__fini
#define dockwds_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1)) DREF DocKwdsIterator *DCALL
dockwds_iter(DocKwds *__restrict self) {
	DREF DocKwdsIterator *result;
	result = DeeObject_MALLOC(DocKwdsIterator);
	if unlikely(!result)
		goto done;
	ASSERT(self->dk_start);
	ASSERT(self->dk_start[0] == '(');
	Dee_Incref(self);
	result->dki_kwds = self;
	result->dki_iter = self->dk_start + 1;
	DeeObject_Init(result, &DocKwdsIterator_Type);
done:
	return result;
}

PRIVATE struct type_seq dockwds_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dockwds_iter,
	/* .tp_sizeob   = */ DEFIMPL(&default__seq_operator_sizeob__with__seq_operator_size),
	/* .tp_contains = */ DEFIMPL(&default__seq_operator_contains__with__seq_contains),
	/* .tp_getitem  = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem  = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem  = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_getitem),
	/* .tp_size                       = */ DEFIMPL(&default__seq_operator_size__with__seq_operator_iter),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_getitem_index),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
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

PRIVATE struct type_member tpconst dockwds_members[] = {
	TYPE_MEMBER_FIELD("__docstr__", STRUCT_CONST | STRUCT_CSTR, offsetof(DocKwds, dk_start)),
	TYPE_MEMBER_FIELD("__owner__", STRUCT_OBJECT, offsetof(DocKwds, dk_owner)),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst dockwds_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DocKwdsIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
dockwds_ctor(DocKwds *__restrict self) {
	self->dk_owner = DeeString_New("()");
	if unlikely(!self->dk_owner)
		goto err;
	self->dk_start = DeeString_STR(self->dk_owner);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dockwds_copy(DocKwds *__restrict self,
             DocKwds *__restrict other) {
	self->dk_owner = other->dk_owner;
	self->dk_start = other->dk_start;
	Dee_Incref(self->dk_owner);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dockwds_init(DocKwds *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeObject *text;
	_DeeArg_Unpack1(err, argc, argv, "_DocKwds", &text);
	if (DeeObject_AssertTypeExact(text, &DeeString_Type))
		goto err;
	if (DeeString_STR(text)[0] != '(') {
		DeeError_Throwf(&DeeError_ValueError,
		                "The given string %r does not start with `('");
		goto err;
	}
	Dee_Incref(text);
	self->dk_owner = text;
	self->dk_start = DeeString_STR(text);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dockwds_print(DocKwds *__restrict self,
              Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "<keywords for %k>", self->dk_owner);
}


INTERN DeeTypeObject DocKwds_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DocKwds",
	/* .tp_doc      = */ DOC("()\n"
	                         "(text:?Dstring)"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)dockwds_ctor,
				/* .tp_copy_ctor = */ (Dee_funptr_t)dockwds_copy,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)dockwds_init,
				TYPE_FIXED_ALLOCATOR(DocKwds)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dockwds_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_iter),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&dockwds_print,
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&dockwds_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__B8EC3298B952DF3A),
	/* .tp_seq           = */ &dockwds_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ dockwds_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dockwds_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* TODO: Operator flags */
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
doc_decode_kwds(DeeObject *owner, char const *doc) {
	DREF DocKwds *result;
	if (!doc)
		goto no_kwds;
	if (doc[0] != '(')
		goto no_kwds;
	result = DeeObject_MALLOC(DocKwds);
	if unlikely(!result)
		goto done;
	Dee_Incref(owner);
	result->dk_owner = owner;
	result->dk_start = doc;
	DeeObject_Init(result, &DocKwds_Type);
done:
	return (DREF DeeObject *)result;
no_kwds:
	return DeeSeq_NewEmpty();
}




STATIC_ASSERT(offsetof(DeeObjMethodObject, om_this) == offsetof(DeeKwObjMethodObject, om_this));
STATIC_ASSERT(offsetof(DeeObjMethodObject, om_func) == offsetof(DeeKwObjMethodObject, om_func));
#define kwobjmethod_fini  objmethod_fini
#define kwobjmethod_visit objmethod_visit

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwobjmethod_call(DeeKwObjMethodObject *self, size_t argc, DeeObject *const *argv) {
	return DeeKwObjMethod_CallFunc(self->om_func, self->om_this, argc, argv, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwobjmethod_call_kw(DeeKwObjMethodObject *self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	return DeeKwObjMethod_CallFunc(self->om_func, self->om_this, argc, argv, kw);
}
#define kwobjmethod_cmp objmethod_cmp

#define kwobjmethod_bound_func objmethod_bound_origin
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwobjmethod_get_func(DeeKwObjMethodObject *__restrict self) {
	struct objmethod_origin origin;
	if likely(DeeKwObjMethod_GetOrigin((DeeObject *)self, &origin))
		return DeeKwClsMethod_New(origin.omo_type, self->om_func);
	return DeeRT_ErrUnboundAttrCStr(self, "__func__");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwobjmethod_get_kwds(DeeKwObjMethodObject *__restrict self) {
	struct objmethod_origin origin;
	if likely(DeeKwObjMethod_GetOrigin((DeeObject *)self, &origin))
		return doc_decode_kwds((DeeObject *)self, origin.omo_decl->m_doc);
	return DeeRT_ErrUnboundAttr(self, &str___kwds__);
}
#define kwobjmethod_bound_kwds objmethod_bound_origin

#define kwobjmethod_get_name       objmethod_get_name
#define kwobjmethod_bound_name     objmethod_bound_name
#define kwobjmethod_get_doc        objmethod_get_doc
#define kwobjmethod_get_type       objmethod_get_type
#define kwobjmethod_bound_type     objmethod_bound_type
#define kwobjmethod_get_module     objmethod_get_module
#define kwobjmethod_bound_module   objmethod_bound_module
#define kwobjmethod_get_func_doc   objmethod_get_func_doc
#define kwobjmethod_get_name_doc   objmethod_get_name_doc
#define kwobjmethod_get_doc_doc    objmethod_get_doc_doc
#define kwobjmethod_get_kwds_doc   objmethod_get_kwds_doc
#define kwobjmethod_get_type_doc   objmethod_get_type_doc
#define kwobjmethod_get_module_doc objmethod_get_module_doc

PRIVATE struct type_getset tpconst kwobjmethod_getsets[] = {
	TYPE_GETTER_BOUND_F("__func__", &kwobjmethod_get_func, &kwobjmethod_bound_func,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    DOC_GET(kwobjmethod_get_func_doc)),
	TYPE_GETTER_BOUND_F(STR___name__, &kwobjmethod_get_name, &kwobjmethod_bound_name,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    DOC_GET(kwobjmethod_get_name_doc)),
	TYPE_GETTER_AB_F(STR___doc__, &kwobjmethod_get_doc,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 DOC_GET(kwobjmethod_get_doc_doc)),
	TYPE_GETTER_BOUND_F(STR___kwds__, &kwobjmethod_get_kwds, &kwobjmethod_bound_kwds,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    DOC_GET(kwobjmethod_get_kwds_doc)),
	TYPE_GETTER_BOUND_F(STR___type__, &kwobjmethod_get_type, &kwobjmethod_bound_type,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    DOC_GET(kwobjmethod_get_type_doc)),
	TYPE_GETTER_BOUND_F(STR___module__, &kwobjmethod_get_module, &kwobjmethod_bound_module,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    DOC_GET(kwobjmethod_get_module_doc)),
	TYPE_GETSET_END
};
#define kwobjmethod_members   objmethod_members
#define kwobjmethod_print     objmethod_print
#define kwobjmethod_printrepr objmethod_printrepr
#define kwobjmethod_operators objmethod_operators

PRIVATE struct type_callable kwobjmethod_callable = {
	/* .tp_call_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&kwobjmethod_call_kw,
	/* .tp_thiscall          = */ DEFIMPL(&default__thiscall__with__call),
	/* .tp_thiscall_kw       = */ DEFIMPL(&default__thiscall_kw__with__call_kw),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ DEFIMPL(&default__call_tuple__with__call),
	/* .tp_call_tuple_kw     = */ DEFIMPL(&default__call_tuple_kw__with__call_kw),
	/* .tp_thiscall_tuple    = */ DEFIMPL(&default__thiscall_tuple__with__thiscall),
	/* .tp_thiscall_tuple_kw = */ DEFIMPL(&default__thiscall_tuple_kw__with__thiscall_kw),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

PUBLIC DeeTypeObject DeeKwObjMethod_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_KwObjMethod",
	/* .tp_doc      = */ DOC("call(args!,kwds!!)->"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeKwObjMethodObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&kwobjmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&kwobjmethod_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&kwobjmethod_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&kwobjmethod_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &kwobjmethod_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ kwobjmethod_getsets,
	/* .tp_members       = */ kwobjmethod_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&kwobjmethod_call,
	/* .tp_callable      = */ &kwobjmethod_callable,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ kwobjmethod_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(kwobjmethod_operators),
};


/* Construct a new `_ClassMethod' object. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF /*ClsMethod*/ DeeObject *DCALL
DeeClsMethod_New(DeeTypeObject *__restrict type,
                 Dee_objmethod_t func) {
	DeeClsMethodObject *result;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	result = DeeObject_MALLOC(DeeClsMethodObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeClsMethod_Type);
	result->cm_type = type;
	result->cm_func = func;
	Dee_Incref(type);
done:
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF /*KwClsMethod*/ DeeObject *DCALL
DeeKwClsMethod_New(DeeTypeObject *__restrict type,
                   Dee_kwobjmethod_t func) {
	DeeKwClsMethodObject *result;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	result = DeeObject_MALLOC(DeeKwClsMethodObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeKwClsMethod_Type);
	result->cm_type = type;
	result->cm_func = func;
	Dee_Incref(type);
done:
	return (DREF DeeObject *)result;
}


PRIVATE ATTR_COLD int DCALL err_classmethod_argc_zero(void) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "ClassMethod objects must be called "
	                       /**/ "with at least 1 argument");
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmethod_call(DeeClsMethodObject *self, size_t argc, DeeObject *const *argv) {
	if unlikely(!argc)
		goto err_argc_zero;

	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(argv[0], self->cm_type))
		goto err;

	/* Use the first argument as the this-argument. */
	return (*self->cm_func)(argv[0], argc - 1, argv + 1);
err_argc_zero:
	err_classmethod_argc_zero();
err:
	return NULL;
}


#if 1
STATIC_ASSERT(offsetof(DeeClsMethodObject, cm_type) == offsetof(DeeObjMethodObject, om_this));
STATIC_ASSERT(offsetof(DeeClsMethodObject, cm_func) == offsetof(DeeObjMethodObject, om_func));
#define clsmethod_fini  objmethod_fini
#define clsmethod_visit objmethod_visit
#else
PRIVATE NONNULL((1)) void DCALL
clsmethod_fini(DeeClsMethodObject *__restrict self) {
	Dee_Decref(self->cm_type);
}

PRIVATE NONNULL((1, 2)) void DCALL
clsmethod_visit(DeeClsMethodObject *__restrict self,
                dvisit_t proc, void *arg) {
	Dee_Visit(self->cm_type);
}
#endif

PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) char const *DCALL
clsmethod_getname(DeeClsMethodObject *__restrict self) {
	struct objmethod_origin origin;
	if likely(DeeClsMethod_GetOrigin((DeeObject *)self, &origin))
		return origin.omo_decl->m_name;
	return "<unknown>";
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
clsmethod_print(DeeClsMethodObject *__restrict self,
                Dee_formatprinter_t printer, void *arg) {
	char const *name = clsmethod_getname(self);
	return DeeFormat_Printf(printer, arg,
	                        "<class method %k.%s>",
	                        self->cm_type, name);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
clsmethod_printrepr(DeeClsMethodObject *__restrict self,
                    Dee_formatprinter_t printer, void *arg) {
	char const *name = clsmethod_getname(self);
	return DeeFormat_Printf(printer, arg, "%r.%s",
	                        self->cm_type, name);
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
clsmethod_hash(DeeClsMethodObject *__restrict self) {
	return Dee_HashPointer(self->cm_func);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
clsmethod_compare_eq(DeeClsMethodObject *self, DeeClsMethodObject *other) {
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	return self->cm_func == other->cm_func ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp clsmethod_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&clsmethod_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&clsmethod_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL_UNSUPPORTED(&default__lo__unsupported),
	/* .tp_le            = */ DEFIMPL_UNSUPPORTED(&default__le__unsupported),
	/* .tp_gr            = */ DEFIMPL_UNSUPPORTED(&default__gr__unsupported),
	/* .tp_ge            = */ DEFIMPL_UNSUPPORTED(&default__ge__unsupported),
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
clsmethod_bound_origin(DeeClsMethodObject *__restrict self) {
	struct objmethod_origin origin;
	return DeeClsMethod_GetOrigin((DeeObject *)self, &origin) ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmethod_get_name(DeeClsMethodObject *__restrict self) {
	struct objmethod_origin origin;
	if likely(DeeClsMethod_GetOrigin((DeeObject *)self, &origin))
		return DeeString_NewAuto(origin.omo_decl->m_name);
	return DeeRT_ErrUnboundAttr(self, &str___name__);
}
#define clsmethod_bound_name clsmethod_bound_origin

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmethod_get_doc(DeeClsMethodObject *__restrict self) {
	struct objmethod_origin origin;
	if (likely(DeeClsMethod_GetOrigin((DeeObject *)self, &origin)) &&
	    origin.omo_decl->m_doc != NULL)
		return DeeString_NewAutoUtf8(origin.omo_decl->m_doc);
	return_none;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwclsmethod_get_kwds(DeeClsMethodObject *__restrict self) {
	struct objmethod_origin origin;
	if likely(DeeKwClsMethod_GetOrigin((DeeObject *)self, &origin))
		return doc_decode_kwds((DeeObject *)self, origin.omo_decl->m_doc);
	return DeeRT_ErrUnboundAttr(self, &str___kwds__);
}
#define kwclsmethod_bound_kwds clsmethod_bound_origin

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmethod_get_module(DeeClsMethodObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self->cm_type);
	if likely(result)
		return result;
	return DeeRT_ErrUnboundAttr(self, &str___module__);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
clsmethod_bound_module(DeeClsMethodObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self->cm_type);
	if likely(result) {
		Dee_Decref_unlikely(result);
		return Dee_BOUND_YES;
	}
	return Dee_BOUND_NO;
}

PRIVATE struct type_getset tpconst kwclsmethod_getsets[] = {
	TYPE_GETTER_BOUND_F(STR___kwds__, &kwclsmethod_get_kwds, &kwclsmethod_bound_kwds,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    DOC_GET(objmethod_get_kwds_doc)),
#define clsmethod_getsets (kwclsmethod_getsets + 1)
	TYPE_GETTER_BOUND_F(STR___name__, &clsmethod_get_name, &clsmethod_bound_name,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "The name of @this method"),
	TYPE_GETTER_AB_F(STR___doc__, &clsmethod_get_doc,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "The documentation string of @this method, or ?N if unknown"),
	TYPE_GETTER_BOUND_F(STR___module__, &clsmethod_get_module, &clsmethod_bound_module,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DModule\n"
	                    "#t{UnboundAttribute}"
	                    "The module implementing @this method"),
	TYPE_GETSET_END
};
#define clsmethod_get_kwds_doc objmethod_get_kwds_doc



PRIVATE WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *DCALL
clsmethod_thiscall(DeeClsMethodObject *self, DeeObject *thisarg,
                   size_t argc, DeeObject *const *argv) {
	/* Must ensure proper typing of the this-argument. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->cm_type))
		goto err;
	return (*self->cm_func)(thisarg, argc, argv);
err:
	return NULL;
}


PRIVATE struct type_callable clsmethod_callable = {
	/* .tp_call_kw     = */ DEFIMPL(&default__call_kw__with__call),
	/* .tp_thiscall    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *))&clsmethod_thiscall,
	/* .tp_thiscall_kw = */ DEFIMPL(&default__thiscall_kw__with__thiscall),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ DEFIMPL(&default__call_tuple__with__call),
	/* .tp_call_tuple_kw     = */ DEFIMPL(&default__call_tuple_kw__with__call_kw),
	/* .tp_thiscall_tuple    = */ DEFIMPL(&default__thiscall_tuple__with__thiscall),
	/* .tp_thiscall_tuple_kw = */ DEFIMPL(&default__thiscall_tuple_kw__with__thiscall_kw),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};


PRIVATE struct type_member tpconst kwclsmethod_members[] = {
	TYPE_MEMBER_CONST_DOC(STR___kwds__, Dee_EmptySeq, clsmethod_get_kwds_doc),
#define clsmethod_members (kwclsmethod_members + 1)
	TYPE_MEMBER_FIELD_DOC(STR___type__, STRUCT_OBJECT, offsetof(DeeClsMethodObject, cm_type),
	                      "->?DType\n"
	                      "The type implementing @this method"),
	TYPE_MEMBER_END
};

PRIVATE struct type_operator const clsmethod_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FIELDS_CONSTSTR),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FIELDS_CONSTREPR),
	TYPE_OPERATOR_FLAGS(OPERATOR_000A_CALL, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FUNC_IS_CONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
};

PUBLIC DeeTypeObject DeeClsMethod_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassMethod",
	/* .tp_doc      = */ DOC("call(thisarg,args!)->"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeClsMethodObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&clsmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&clsmethod_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&clsmethod_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&clsmethod_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &clsmethod_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ clsmethod_getsets,
	/* .tp_members       = */ clsmethod_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsmethod_call,
	/* .tp_callable      = */ &clsmethod_callable,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ clsmethod_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(clsmethod_operators),
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwclsmethod_call(DeeKwClsMethodObject *self, size_t argc, DeeObject *const *argv) {
	if unlikely(!argc)
		goto err_argc_zero;

	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(argv[0], self->cm_type))
		goto err;

	/* Use the first argument as the this-argument. */
	return (*self->cm_func)(argv[0], argc - 1, argv + 1, NULL);
err_argc_zero:
	err_classmethod_argc_zero();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwclsmethod_call_kw(DeeKwClsMethodObject *self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	if unlikely(!argc)
		goto err_argc_zero;

	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(argv[0], self->cm_type))
		goto err;

	/* Use the first argument as the this-argument. */
	return (*self->cm_func)(argv[0], argc - 1, argv + 1, kw);
err_argc_zero:
	err_classmethod_argc_zero();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kwclsmethod_thiscall(DeeKwClsMethodObject *self,
                     DeeObject *thisarg, size_t argc,
                     DeeObject *const *argv) {
	/* Must ensure proper typing of the this-argument. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->cm_type))
		goto err;
	return (*self->cm_func)(thisarg, argc, argv, NULL);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
kwclsmethod_thiscall_kw(DeeKwClsMethodObject *self,
                        DeeObject *thisarg, size_t argc,
                        DeeObject *const *argv, DeeObject *kw) {
	/* Must ensure proper typing of the this-argument. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->cm_type))
		goto err;
	return (*self->cm_func)(thisarg, argc, argv, kw);
err:
	return NULL;
}

STATIC_ASSERT(offsetof(DeeKwClsMethodObject, cm_func) == offsetof(DeeClsMethodObject, cm_func));
STATIC_ASSERT(offsetof(DeeKwClsMethodObject, cm_type) == offsetof(DeeClsMethodObject, cm_type));
#define kwclsmethod_fini      clsmethod_fini
#define kwclsmethod_print     clsmethod_print
#define kwclsmethod_printrepr clsmethod_printrepr
#define kwclsmethod_visit     clsmethod_visit
#define kwclsmethod_hash      clsmethod_hash
#define kwclsmethod_cmp       clsmethod_cmp
#define kwclsmethod_operators clsmethod_operators

PRIVATE struct type_callable kwclsmethod_callable = {
	/* .tp_call_kw      = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&kwclsmethod_call_kw,
	/* .tp_thiscall     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *))&kwclsmethod_thiscall,
	/* .tp_thiscall_kw  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *, DeeObject *))&kwclsmethod_thiscall_kw,
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ DEFIMPL(&default__call_tuple__with__call),
	/* .tp_call_tuple_kw     = */ DEFIMPL(&default__call_tuple_kw__with__call_kw),
	/* .tp_thiscall_tuple    = */ DEFIMPL(&default__thiscall_tuple__with__thiscall),
	/* .tp_thiscall_tuple_kw = */ DEFIMPL(&default__thiscall_tuple_kw__with__thiscall_kw),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

PUBLIC DeeTypeObject DeeKwClsMethod_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_KwClassMethod",
	/* .tp_doc      = */ DOC("call(thisarg,args!,kwds!!)->"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeKwClsMethodObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&kwclsmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&kwclsmethod_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&kwclsmethod_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&kwclsmethod_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &kwclsmethod_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ kwclsmethod_getsets,
	/* .tp_members       = */ kwclsmethod_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&kwclsmethod_call,
	/* .tp_callable      = */ &kwclsmethod_callable,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ kwclsmethod_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(kwclsmethod_operators),
};



/* Lookup the origin of the function bound by
 * the given `_ClassProperty', or `NULL' if unknown. */
PUBLIC WUNUSED NONNULL((1)) bool DCALL
DeeClsProperty_GetOrigin(DeeObject const *__restrict self,
                         struct clsproperty_origin *__restrict result) {
	struct type_getset const *iter;
	ASSERT_OBJECT(self);
	ASSERT(DeeClsProperty_Check(self));
	iter = DeeClsProperty_TYPE(self)->tp_getsets;
	if (iter) {
		for (; iter->gs_name; ++iter) {
			if (iter->gs_get == DeeClsProperty_GET(self) &&
			    iter->gs_del == DeeClsProperty_DEL(self) &&
			    iter->gs_set == DeeClsProperty_SET(self)) {
				result->cpo_type  = DeeClsProperty_TYPE(self);
				result->cpo_chain = DeeClsProperty_TYPE(self)->tp_getsets;
				result->cpo_decl  = iter;
				return true;
			}
		}
	}
	return false;
}


/* Create a new unbound class property object. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF /*ClsProperty*/ DeeObject *
(DCALL DeeClsProperty_New)(DeeTypeObject *__restrict type,
                           struct Dee_type_getset const *getset) {
	return DeeClsProperty_NewEx(type,
	                            getset->gs_get, getset->gs_del,
	                            getset->gs_set, getset->gs_bound);
}

PUBLIC WUNUSED NONNULL((1)) DREF /*ClsProperty*/ DeeObject *DCALL
DeeClsProperty_NewEx(DeeTypeObject *__restrict type,
                     Dee_getmethod_t get, Dee_delmethod_t del,
                     Dee_setmethod_t set, Dee_boundmethod_t bound) {
	DeeClsPropertyObject *result;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	result = DeeObject_MALLOC(DeeClsPropertyObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeClsProperty_Type);
	result->cp_type = type;
	Dee_Incref(type);
	result->cp_get   = get;
	result->cp_del   = del;
	result->cp_set   = set;
	result->cp_bound = bound;
done:
	return (DREF DeeObject *)result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
clsproperty_hash(DeeClsPropertyObject *__restrict self) {
	Dee_hash_t result;
	result = Dee_HashPointer(self->cp_get);
	result = Dee_HashCombine(result, Dee_HashPointer(self->cp_del));
	result = Dee_HashCombine(result, Dee_HashPointer(self->cp_set));
	result = Dee_HashCombine(result, Dee_HashPointer(self->cp_bound));
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
clsproperty_compare_eq(DeeClsPropertyObject *self,
                       DeeClsPropertyObject *other) {
	if (DeeObject_AssertTypeExact(other, &DeeClsProperty_Type))
		goto err;
	return (self->cp_get == other->cp_get &&
	        self->cp_del == other->cp_del &&
	        self->cp_set == other->cp_set &&
	        self->cp_bound == other->cp_bound)
	       ? 0
	       : 1;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp clsproperty_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&clsproperty_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&clsproperty_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL_UNSUPPORTED(&default__lo__unsupported),
	/* .tp_le            = */ DEFIMPL_UNSUPPORTED(&default__le__unsupported),
	/* .tp_gr            = */ DEFIMPL_UNSUPPORTED(&default__gr__unsupported),
	/* .tp_ge            = */ DEFIMPL_UNSUPPORTED(&default__ge__unsupported),
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_get(DeeClsPropertyObject *__restrict self,
                size_t argc, DeeObject *const *argv) {
	DeeObject *thisarg;
	if unlikely(!self->cp_get)
		goto err_unbound;
	_DeeArg_Unpack1(err, argc, argv, "get", &thisarg);
	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->cp_type))
		goto err;
	return (*self->cp_get)(thisarg);
err_unbound:
	DeeRT_ErrRestrictedAttrCStr(self, "get", DeeRT_ATTRIBUTE_ACCESS_CALL);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_get_kw(DeeClsPropertyObject *__restrict self,
                   size_t argc, DeeObject *const *argv,
                   DeeObject *kw) {
	DeeObject *thisarg;
	if unlikely(!self->cp_get)
		goto err_unbound;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__thisarg, "o:get", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->cp_type))
		goto err;
	return (*self->cp_get)(thisarg);
err_unbound:
	DeeRT_ErrRestrictedAttrCStr(self, "get", DeeRT_ATTRIBUTE_ACCESS_CALL);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_isbound_kw(DeeClsPropertyObject *__restrict self,
                       size_t argc, DeeObject *const *argv,
                       DeeObject *kw) {
	int isbound;
	DeeObject *thisarg;
	if unlikely(!self->cp_get)
		goto err_unbound;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__thisarg, "o:isbound", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->cp_type))
		goto err;
	if (self->cp_bound) {
		isbound = (*self->cp_bound)(thisarg);
		if (Dee_BOUND_ISERR(isbound))
			goto err;
	} else {
		DREF DeeObject *value;
		value = (*self->cp_get)(thisarg);
		if (value) {
			Dee_Decref(value);
			isbound = Dee_BOUND_YES;
		} else if (DeeError_Catch(&DeeError_UnboundAttribute)) {
			isbound = Dee_BOUND_NO;
		} else {
			goto err;
		}
	}
	return_bool(Dee_BOUND_ISBOUND(isbound));
err_unbound:
	DeeRT_ErrRestrictedAttrCStr(self, "get", DeeRT_ATTRIBUTE_ACCESS_CALL);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_delete(DeeClsPropertyObject *__restrict self,
                   size_t argc, DeeObject *const *argv,
                   DeeObject *kw) {
	DeeObject *thisarg;
	if unlikely(!self->cp_del)
		goto err_unbound;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__thisarg, "o:delete", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->cp_type))
		goto err;
	if unlikely((*self->cp_del)(thisarg))
		goto err;
	return_none;
err_unbound:
	DeeRT_ErrRestrictedAttrCStr(self, "delete", DeeRT_ATTRIBUTE_ACCESS_CALL);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_set(DeeClsPropertyObject *__restrict self,
                size_t argc, DeeObject *const *argv,
                DeeObject *kw) {
	DeeObject *thisarg, *value;
	if unlikely(!self->cp_set)
		goto err_unbound;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__thisarg_value, "oo:set", &thisarg, &value))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->cp_type))
		goto err;
	if unlikely((*self->cp_set)(thisarg, value))
		goto err;
	return_none;
err_unbound:
	DeeRT_ErrRestrictedAttrCStr(self, "set", DeeRT_ATTRIBUTE_ACCESS_CALL);
err:
	return NULL;
}

PRIVATE struct type_method tpconst clsproperty_methods[] = {
	TYPE_KWMETHOD_F(STR_get, &clsproperty_get_kw,
	                METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FUNC_IS_CONSTCALL,
	                "(thisarg)->"),
	TYPE_KWMETHOD_F("delete", &clsproperty_delete, METHOD_FNOREFESCAPE, "(thisarg)"),
	TYPE_KWMETHOD_F(STR_set, &clsproperty_set, METHOD_FNOREFESCAPE, "(thisarg,value)"),
	TYPE_KWMETHOD_F("getter", &clsproperty_get_kw,
	                METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FUNC_IS_CONSTCALL,
	                "(thisarg)->\nAlias for ?#get"),
	TYPE_KWMETHOD_F("setter", &clsproperty_set, METHOD_FNOREFESCAPE, "(thisarg,value)\nAlias for ?#set"),
	TYPE_KWMETHOD_F("isbound", &clsproperty_isbound_kw,
	                METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FUNC_IS_CONSTCALL,
	                "(thisarg)->?Dbool\n"
	                "Check if the attribute is bound"),
	TYPE_METHOD_END
};

PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) char const *DCALL
clsproperty_getname(DeeClsPropertyObject *__restrict self) {
	struct clsproperty_origin origin;
	if likely(DeeClsProperty_GetOrigin((DeeObject *)self, &origin))
		return origin.cpo_decl->gs_name;
	return "<unknown>";
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
clsproperty_print(DeeClsPropertyObject *__restrict self,
                  Dee_formatprinter_t printer, void *arg) {
	char permissions[COMPILER_LENOF(",get,del,set")], *p = permissions;
	char const *name = clsproperty_getname(self);
	if (self->cp_get)
		p = stpcpy(p, ",get");
	if (self->cp_del)
		p = stpcpy(p, ",del");
	if (self->cp_set)
		p = stpcpy(p, ",set");
	if (p == permissions)
		(void)strcpy(p, ",-");
	return DeeFormat_Printf(printer, arg,
	                        "<class property %k.%s (%s)>",
	                        self->cp_type, name,
	                        permissions + 1);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
clsproperty_printrepr(DeeClsPropertyObject *__restrict self,
                      Dee_formatprinter_t printer, void *arg) {
	char const *name = clsproperty_getname(self);
	return DeeFormat_Printf(printer, arg, "%r.%s", self->cp_type, name);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
clsproperty_bound_origin(DeeClsPropertyObject *__restrict self) {
	struct clsproperty_origin origin;
	return DeeClsProperty_GetOrigin((DeeObject *)self, &origin) ? 1 : 0;
}

#define clsproperty_bound_name clsproperty_bound_origin
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_get_name(DeeClsPropertyObject *__restrict self) {
	struct clsproperty_origin origin;
	if likely(DeeClsProperty_GetOrigin((DeeObject *)self, &origin))
		return DeeString_NewAuto(origin.cpo_decl->gs_name);
	return DeeRT_ErrUnboundAttr(self, &str___name__);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_get_doc(DeeClsPropertyObject *__restrict self) {
	struct clsproperty_origin origin;
	if (likely(DeeClsProperty_GetOrigin((DeeObject *)self, &origin)) &&
	    origin.cpo_decl->gs_doc != NULL)
		return DeeString_NewAutoUtf8(origin.cpo_decl->gs_doc);
	return_none;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_get_module(DeeClsPropertyObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self->cp_type);
	if likely(result)
		return result;
	return DeeRT_ErrUnboundAttr(self, &str___module__);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
clsproperty_bound_module(DeeClsPropertyObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self->cp_type);
	if likely(result) {
		Dee_Decref_unlikely(result);
		return 1;
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_canget(DeeClsPropertyObject *__restrict self) {
	return_bool(self->cp_get != NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_candel(DeeClsPropertyObject *__restrict self) {
	return_bool(self->cp_del != NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsproperty_canset(DeeClsPropertyObject *__restrict self) {
	return_bool(self->cp_set != NULL);
}

PRIVATE struct type_getset tpconst clsproperty_getsets[] = {
	TYPE_GETTER_AB_F("canget", &clsproperty_canget,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this property has a getter callback"),
	TYPE_GETTER_AB_F("candel", &clsproperty_candel,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this property has a delete callback"),
	TYPE_GETTER_AB_F("canset", &clsproperty_canset,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this property has a setter callback"),
	TYPE_GETTER_BOUND_F(STR___name__, &clsproperty_get_name, &clsproperty_bound_name,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "The name of @this property"),
	TYPE_GETTER_AB_F(STR___doc__, &clsproperty_get_doc,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "The documentation string of @this property, or ?N if unknown"),
	TYPE_GETTER_BOUND_F(STR___module__, &clsproperty_get_module, &clsproperty_bound_module,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DModule\n"
	                    "#t{UnboundAttribute}"
	                    "The module implementing @this property"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst clsproperty_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR___type__, STRUCT_OBJECT, offsetof(DeeClsPropertyObject, cp_type),
	                      "->?DType\n"
	                      "The type implementing @this property"),
	TYPE_MEMBER_END
};

/* Make sure that we're allowed to re-use operators from ClassMethod. */
STATIC_ASSERT(offsetof(DeeClsPropertyObject, cp_type) ==
              offsetof(DeeClsMethodObject, cm_type));

PRIVATE struct type_callable clsproperty_callable = {
	/* .tp_call_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&clsproperty_get_kw,
	/* .tp_thiscall          = */ DEFIMPL(&default__thiscall__with__call),
	/* .tp_thiscall_kw       = */ DEFIMPL(&default__thiscall_kw__with__call_kw),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ DEFIMPL(&default__call_tuple__with__call),
	/* .tp_call_tuple_kw     = */ DEFIMPL(&default__call_tuple_kw__with__call_kw),
	/* .tp_thiscall_tuple    = */ DEFIMPL(&default__thiscall_tuple__with__thiscall),
	/* .tp_thiscall_tuple_kw = */ DEFIMPL(&default__thiscall_tuple_kw__with__thiscall_kw),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

#define clsproperty_operators clsmethod_operators
PUBLIC DeeTypeObject DeeClsProperty_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassProperty",
	/* .tp_doc      = */ DOC("call(thisarg)->"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeClsPropertyObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&clsmethod_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&clsproperty_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&clsproperty_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&clsmethod_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &clsproperty_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ clsproperty_methods,
	/* .tp_getsets       = */ clsproperty_getsets,
	/* .tp_members       = */ clsproperty_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsproperty_get,
	/* .tp_callable      = */ &clsproperty_callable,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ clsproperty_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(clsproperty_operators),
};



/* Create a new unbound class member object. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF /*ClsMember*/ DeeObject *DCALL
DeeClsMember_New(DeeTypeObject *__restrict type,
                 struct type_member const *desc) {
	DREF DeeClsMemberObject *result;
	ASSERT_OBJECT_TYPE(type, &DeeType_Type);
	result = DeeObject_MALLOC(DeeClsMemberObject);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeClsMember_Type);
	result->cm_memb = *desc;
	result->cm_type = type;
	Dee_Incref(type);
done:
	return (DREF DeeObject *)result;
}

STATIC_ASSERT(offsetof(DeeClsMemberObject, cm_type) == offsetof(ProxyObject, po_obj));
#define clsmember_fini  generic_proxy__fini
#define clsmember_visit generic_proxy__visit

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
clsmember_print(DeeClsMemberObject *__restrict self,
                Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg,
	                        "<class member %k.%s>",
	                        self->cm_type,
	                        self->cm_memb.m_name);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
clsmember_printrepr(DeeClsMemberObject *__restrict self,
                    Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg, "%r.%s",
	                        self->cm_type,
	                        self->cm_memb.m_name);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_get(DeeClsMemberObject *self, size_t argc,
              DeeObject *const *argv) {
	DeeObject *thisarg;
	_DeeArg_Unpack1(err, argc, argv, "get", &thisarg);
	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->cm_type))
		goto err;
	return type_member_get(&self->cm_memb, thisarg);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_get_kw(DeeClsMemberObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DeeObject *thisarg;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__thisarg, "o:get", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->cm_type))
		goto err;
	return type_member_get(&self->cm_memb, thisarg);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_isbound(DeeClsMemberObject *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	bool isbound;
	DeeObject *thisarg;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__thisarg, "o:isbound", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->cm_type))
		goto err;
	isbound = type_member_bound(&self->cm_memb, thisarg);
	return_bool(isbound);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_delete(DeeClsMemberObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DeeObject *thisarg;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__thisarg, "o:delete", &thisarg))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->cm_type))
		goto err;
	if (type_member_del(&self->cm_memb, thisarg))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_set(DeeClsMemberObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DeeObject *thisarg, *value;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__thisarg, "oo:set", &thisarg, &value))
		goto err;
	/* Allow non-instance objects for generic types. */
	if (DeeObject_AssertTypeOrAbstract(thisarg, self->cm_type))
		goto err;
	if (type_member_set(&self->cm_memb, thisarg, value))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE struct type_method tpconst clsmember_methods[] = {
	TYPE_KWMETHOD_F(STR_get, &clsmember_get_kw, METHOD_FNOREFESCAPE, "(thisarg)->"),
	TYPE_KWMETHOD_F("delete", &clsmember_delete, METHOD_FNOREFESCAPE, "(thisarg)"),
	TYPE_KWMETHOD_F(STR_set, &clsmember_set, METHOD_FNOREFESCAPE, "(thisarg,value)"),
	TYPE_KWMETHOD_F("isbound", &clsmember_isbound, METHOD_FNOREFESCAPE, "(thisarg)->?Dbool"),
	TYPE_METHOD_END
};

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
clsmember_hash(DeeClsMemberObject *__restrict self) {
	return (Dee_HashPointer(self->cm_type) ^
	        Dee_HashPointer(self->cm_memb.m_name) ^
	        Dee_HashPointer(self->cm_memb.m_desc.md_const));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
clsmember_compare_eq(DeeClsMemberObject *self,
                     DeeClsMemberObject *other) {
	if (DeeObject_AssertTypeExact(other, &DeeClsMember_Type))
		goto err;
	return ((self->cm_type == other->cm_type) &&
	        (self->cm_memb.m_name == other->cm_memb.m_name) &&
	        (self->cm_memb.m_desc.md_const == other->cm_memb.m_desc.md_const))
	       ? 0
	       : 1;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp clsmember_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&clsmember_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&clsmember_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL_UNSUPPORTED(&default__lo__unsupported),
	/* .tp_le            = */ DEFIMPL_UNSUPPORTED(&default__le__unsupported),
	/* .tp_gr            = */ DEFIMPL_UNSUPPORTED(&default__gr__unsupported),
	/* .tp_ge            = */ DEFIMPL_UNSUPPORTED(&default__ge__unsupported),
};

PRIVATE struct type_member tpconst clsmember_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR___type__, STRUCT_OBJECT,
	                      offsetof(DeeClsMemberObject, cm_type),
	                      "->?DType\n"
	                      "The type implementing @this member"),
	TYPE_MEMBER_FIELD_DOC(STR___name__, STRUCT_CONST | STRUCT_CSTR,
	                      offsetof(DeeClsMemberObject, cm_memb.m_name),
	                      "The name of @this member"),
	TYPE_MEMBER_FIELD_DOC(STR___doc__, STRUCT_CONST | STRUCT_CSTR_OPT,
	                      offsetof(DeeClsMemberObject, cm_memb.m_doc),
	                      "->?X2?Dstring?N\n"
	                      "The documentation string of @this member, or ?N if unknown"),
	TYPE_MEMBER_CONST_DOC("canget", Dee_True, "Always evaluates to ?t"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_canset(DeeClsMemberObject *__restrict self) {
	if (TYPE_MEMBER_ISCONST(&self->cm_memb))
		return_false;
	return_bool(!(self->cm_memb.m_desc.md_field.mdf_type & STRUCT_CONST));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
clsmember_get_module(DeeClsMemberObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self->cm_type);
	if likely(result)
		return result;
	return DeeRT_ErrUnboundAttr(self, &str___module__);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
clsmember_bound_module(DeeClsMemberObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self->cm_type);
	if likely(result) {
		Dee_Decref_unlikely(result);
		return Dee_BOUND_YES;
	}
	return Dee_BOUND_NO;
}

PRIVATE struct type_getset tpconst clsmember_getsets[] = {
	TYPE_GETTER_AB_F("candel", &clsmember_canset,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Alias for #canset"),
	TYPE_GETTER_AB_F("canset", &clsmember_canset,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this member can be modified"),
	TYPE_GETTER_BOUND_F(STR___module__, &clsmember_get_module, &clsmember_bound_module,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DModule\n"
	                    "#t{UnboundAttribute}"
	                    "The module implementing @this member"),
	TYPE_GETSET_END
};

PRIVATE struct type_callable clsmember_callable = {
	/* .tp_call_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&clsmember_get_kw,
	/* .tp_thiscall          = */ DEFIMPL(&default__thiscall__with__call),
	/* .tp_thiscall_kw       = */ DEFIMPL(&default__thiscall_kw__with__call_kw),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ DEFIMPL(&default__call_tuple__with__call),
	/* .tp_call_tuple_kw     = */ DEFIMPL(&default__call_tuple_kw__with__call_kw),
	/* .tp_thiscall_tuple    = */ DEFIMPL(&default__thiscall_tuple__with__thiscall),
	/* .tp_thiscall_tuple_kw = */ DEFIMPL(&default__thiscall_tuple_kw__with__thiscall_kw),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

#define clsmember_operators clsmethod_operators
PUBLIC DeeTypeObject DeeClsMember_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_ClassMember",
	/* .tp_doc      = */ DOC("call(thisarg)->"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeClsMemberObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&clsmember_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&clsmember_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&clsmember_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&clsmember_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &clsmember_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ clsmember_methods,
	/* .tp_getsets       = */ clsmember_getsets,
	/* .tp_members       = */ clsmember_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&clsmember_get,
	/* .tp_callable      = */ &clsmember_callable,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ clsmember_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(clsmember_operators),
};

PRIVATE WUNUSED NONNULL((1, 2)) struct module_symbol *DCALL
cmethod_getmodsym(DeeModuleObject *__restrict mod,
                  Dee_cmethod_t func_ptr) {
	uint16_t addr;
	struct module_symbol *result;
	DeeModule_LockRead(mod);
	for (addr = 0; addr < mod->mo_globalc; ++addr) {
		DeeObject *glob = mod->mo_globalv[addr];
		if (!glob)
			continue;
		if (!DeeCMethod_Check(glob) &&
		    !DeeKwCMethod_Check(glob))
			continue;
		if (DeeCMethod_FUNC(glob) != func_ptr)
			continue;
		DeeModule_LockEndRead(mod);
		result = DeeModule_GetSymbolID(mod, addr);
		if (result)
			return result;
		DeeModule_LockRead(mod);
	}
	DeeModule_LockEndRead(mod);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) struct type_member const *DCALL
type_member_search_cmethod(DeeTypeObject *type,
                           struct type_member const *chain,
                           DeeTypeObject **__restrict ptarget_type,
                           Dee_cmethod_t func_ptr) {
	for (; chain->m_name; ++chain) {
		if (!TYPE_MEMBER_ISCONST(chain))
			continue;
		if (DeeType_Check(chain->m_desc.md_const)) {
			/* XXX: Recursively search sub-types? (would require keeping a working set to prevent recursion) */
		} else if (DeeCMethod_Check(chain->m_desc.md_const) ||
		           DeeKwCMethod_Check(chain->m_desc.md_const)) {
			if (DeeCMethod_FUNC(chain->m_desc.md_const) == func_ptr)
				goto gotit;
		}
	}
	return NULL;
gotit:
	*ptarget_type = type;
	return chain;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) struct type_member const *DCALL
type_seach_cmethod(DeeTypeObject *self, Dee_cmethod_t func_ptr,
                   DeeTypeObject **__restrict ptarget_type) {
	struct type_member const *result;
	DeeTypeMRO mro;
	DeeTypeMRO_Init(&mro, self);
	do {
		if (self->tp_class_members &&
		    (result = type_member_search_cmethod(self, self->tp_class_members, ptarget_type, func_ptr)) != NULL)
			goto gotit;
		if (self->tp_members &&
		    (result = type_member_search_cmethod(self, self->tp_members, ptarget_type, func_ptr)) != NULL)
			goto gotit;
	} while ((self = DeeTypeMRO_Next(&mro, self)) != NULL);
	return NULL;
gotit:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) struct type_member const *DCALL
cmethod_gettypefield(DeeModuleObject *mod,
                     DREF DeeTypeObject **__restrict ptype,
                     Dee_cmethod_t func_ptr) {
	uint16_t addr;
	struct type_member const *result;
	DeeModule_LockRead(mod);
	for (addr = 0; addr < mod->mo_globalc; ++addr) {
		DeeObject *glob = mod->mo_globalv[addr];
		if (!glob)
			continue;
		/* Check for cmethod objects defined as TYPE_MEMBER_CONST() fields of exported types. */
		if (!DeeType_Check(glob))
			continue;
		Dee_Incref(glob);
		DeeModule_LockEndRead(mod);
		result = type_seach_cmethod((DeeTypeObject *)glob, func_ptr, ptype);
		if (result) {
			Dee_Incref(*ptype);
			Dee_Decref_unlikely(glob);
			return result;
		}
		Dee_Decref_unlikely(glob);
		DeeModule_LockRead(mod);
	}
	DeeModule_LockEndRead(mod);
	return NULL;
}


/* Try to figure out doc information about `func' */
PUBLIC WUNUSED NONNULL((1, 2)) bool DCALL
Dee_cmethod_origin_init(struct cmethod_origin *__restrict result, Dee_cmethod_t func) {
	bzero(result, sizeof(*result));
	result->cmo_module = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&func);
	if likely(result->cmo_module) {
		result->cmo_modsym = cmethod_getmodsym(result->cmo_module, func);
		if (result->cmo_modsym) {
			result->cmo_name = result->cmo_modsym->ss_name;
			result->cmo_doc  = result->cmo_modsym->ss_doc;
		} else {
			result->cmo_member = cmethod_gettypefield(result->cmo_module, &result->cmo_type, func);
			if (result->cmo_member) {
				result->cmo_name = result->cmo_member->m_name;
				result->cmo_doc  = result->cmo_member->m_doc;
			}
		}
	}
	if (result->cmo_name)
		return true;
	Dee_cmethod_origin_fini(result);
	return false;
}






/* Make sure that we can re-use some functions from `ClassMethod' */
STATIC_ASSERT(offsetof(DeeCMethodObject, cm_func) ==
              offsetof(DeeClsMethodObject, cm_func));

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_call(DeeCMethodObject *self, size_t argc, DeeObject *const *argv) {
	return DeeCMethod_CallFunc(self->cm_func, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_get_module(DeeCMethodObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if likely(result)
		return result;
	return DeeRT_ErrUnboundAttr(self, &str___module__);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cmethod_bound_module(DeeCMethodObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (result) {
		Dee_Decref_unlikely(result);
		return Dee_BOUND_YES;
	}
	return Dee_BOUND_NO;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cmethod_bound_origin(DeeCMethodObject *__restrict self) {
	struct cmethod_origin origin;
	bool bound = DeeKwCMethod_GetOrigin(self, &origin);
	return Dee_BOUND_FROMBOOL(bound);
}

#define kwcmethod_bound_kwds cmethod_bound_origin
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwcmethod_get_kwds(DeeCMethodObject *__restrict self) {
	struct cmethod_origin origin;
	if (DeeKwCMethod_GetOrigin(self, &origin)) {
		DREF DeeObject *result;
		if (origin.cmo_doc) {
			DeeObject *owner = (DeeObject *)self;
			if (origin.cmo_module)
				owner = (DeeObject *)origin.cmo_module;
			if (origin.cmo_type)
				owner = (DeeObject *)origin.cmo_type;
			result = doc_decode_kwds(owner, origin.cmo_doc);
		} else {
			result = Dee_EmptySeq;
			Dee_Incref(result);
		}
		Dee_cmethod_origin_fini(&origin);
		return result;
	}
	return DeeRT_ErrUnboundAttr(self, &str___kwds__);
}

#define cmethod_bound_name cmethod_bound_origin
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_get_name(DeeCMethodObject *__restrict self) {
	struct cmethod_origin origin;
	if (DeeKwCMethod_GetOrigin(self, &origin)) {
		DREF DeeObject *result = DeeString_NewAuto(origin.cmo_name);
		Dee_cmethod_origin_fini(&origin);
		return result;
	}
	return DeeRT_ErrUnboundAttr(self, &str___name__);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_get_type(DeeCMethodObject *__restrict self) {
	struct cmethod_origin origin;
	if (DeeKwCMethod_GetOrigin(self, &origin)) {
		Dee_XDecref(origin.cmo_module);
		if (origin.cmo_type)
			return (DREF DeeObject *)origin.cmo_type;
	}
	return DeeRT_ErrUnboundAttr(self, &str___type__);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
cmethod_bound_type(DeeCMethodObject *__restrict self) {
	struct cmethod_origin origin;
	if (DeeKwCMethod_GetOrigin(self, &origin)) {
		Dee_XDecref(origin.cmo_module);
		Dee_XDecref(origin.cmo_type);
		if (origin.cmo_type)
			return Dee_BOUND_YES;
	}
	return Dee_BOUND_NO;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
cmethod_get_doc(DeeCMethodObject *__restrict self) {
	struct cmethod_origin origin;
	if (DeeKwCMethod_GetOrigin(self, &origin)) {
		if (origin.cmo_doc) {
			DREF DeeObject *result = DeeString_NewAutoUtf8(origin.cmo_doc);
			Dee_cmethod_origin_fini(&origin);
			return result;
		}
		Dee_cmethod_origin_fini(&origin);
	}
	return_none;
}

#define cmethod_get_kwds_doc objmethod_get_kwds_doc

#define cmethod_members (objmethod_members + OBJMETHOD_MEMBERS_INDEXOF_KWDS)
#define cmethod_getsets (kwcmethod_getsets + 1)
PRIVATE struct type_getset tpconst kwcmethod_getsets[] = {
	TYPE_GETTER_BOUND_F(STR___kwds__, &kwcmethod_get_kwds, &kwcmethod_bound_kwds,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    DOC_GET(cmethod_get_kwds_doc)),
	TYPE_GETTER_BOUND_F(STR___module__, &cmethod_get_module, &cmethod_bound_module,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DModule\n"
	                    "#t{UnboundAttribute}"
	                    "Returns the module defining @this method"),
	TYPE_GETTER_BOUND_F(STR___type__, &cmethod_get_type, &cmethod_bound_type,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DType\n"
	                    "#t{UnboundAttribute}"
	                    "Returns the type as part of which @this method was declared."),
	TYPE_GETTER_BOUND_F(STR___name__, &cmethod_get_name, &cmethod_bound_name,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "Returns the name of @this method"),
	TYPE_GETTER_AB_F(STR___doc__, &cmethod_get_doc,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "Returns the documentation string of @this method, or ?N if unknown"),
	TYPE_GETSET_END
};


PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) char const *DCALL
cmethod_get_print_typename(DeeCMethodObject *__restrict self) {
	char const *result = "c-method";
	if (DeeKwCMethod_Check(self))
		result = "kw-c-method";
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cmethod_print(DeeCMethodObject *__restrict self,
              Dee_formatprinter_t printer, void *arg) {
	DREF DeeModuleObject *mod;
	char const *type_name = cmethod_get_print_typename(self);
	Dee_ssize_t result;
	mod = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (mod != NULL) {
		DREF DeeTypeObject *type;
		struct type_member const *member;
		struct module_symbol *symbol;
		symbol = cmethod_getmodsym(mod, self->cm_func);
		if (symbol != NULL) {
			result = DeeFormat_Printf(printer, arg, "<%s %k.%s>",
			                          type_name, mod, symbol->ss_name);
			Dee_Decref(mod);
			goto done;
		}
		member = cmethod_gettypefield(mod, &type, self->cm_func);
		if (member) {
			result = DeeFormat_Printf(printer, arg, "<%s %k.%k.%s>",
			                          type_name, mod, type, member->m_name);
			Dee_Decref(type);
			Dee_Decref(mod);
			goto done;
		}
		result = DeeFormat_Printf(printer, arg,
		                          "<%s %k.<unknown>>",
		                          type_name, mod);
		Dee_Decref(mod);
	} else {
		result = DeeFormat_Printf(printer, arg,
		                          "<%s <unknown>>",
		                          type_name);
	}
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
cmethod_printrepr(DeeCMethodObject *__restrict self,
                  Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t result;
	DREF DeeModuleObject *mod;
	mod = (DREF DeeModuleObject *)DeeModule_FromStaticPointer(*(void **)&self->cm_func);
	if (mod) {
		struct module_symbol *symbol;
		struct type_member const *member;
		DREF DeeTypeObject *type;
		symbol = cmethod_getmodsym(mod, self->cm_func);
		if (symbol) {
			result = DeeFormat_Printf(printer, arg, "%r.%s",
			                          mod, symbol->ss_name);
			Dee_Decref(mod);
			return result;
		}
		member = cmethod_gettypefield(mod, &type, self->cm_func);
		if (member) {
			result = DeeFormat_Printf(printer, arg, "%r.%k.%s",
			                          mod, type, member->m_name);
			Dee_Decref(type);
			Dee_Decref(mod);
			return result;
		}
		result = DeeFormat_Printf(printer, arg, "%r.<unknown %s>",
		                          mod, cmethod_get_print_typename(self));
		Dee_Decref(mod);
	} else {
		result = DeeFormat_Printf(printer, arg, "<unknown %s>",
		                          cmethod_get_print_typename(self));
	}
	return result;
}

#define cmethod_operators clsmethod_operators

PUBLIC DeeTypeObject DeeCMethod_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_CMethod",
	/* .tp_doc      = */ DOC("call(args!)->"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeCMethodObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&cmethod_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&cmethod_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &clsmethod_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ cmethod_getsets,
	/* .tp_members       = */ cmethod_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&cmethod_call,
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ cmethod_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(cmethod_operators),
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwcmethod_call(DeeKwCMethodObject *self, size_t argc, DeeObject *const *argv) {
	return DeeKwCMethod_CallFunc(self->kcm_func, argc, argv, NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
kwcmethod_call_kw(DeeKwCMethodObject *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	return DeeKwCMethod_CallFunc(self->kcm_func, argc, argv, kw);
}

/* Make sure that we can re-use some functions from `CMethod' */
STATIC_ASSERT(offsetof(DeeKwCMethodObject, kcm_func) == offsetof(DeeCMethodObject, cm_func));
#define kwcmethod_print     cmethod_print
#define kwcmethod_printrepr cmethod_printrepr
#define kwcmethod_operators cmethod_operators

PRIVATE struct type_callable kwcmethod_callable = {
	/* .tp_call_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&kwcmethod_call_kw,
	/* .tp_thiscall          = */ DEFIMPL(&default__thiscall__with__call),
	/* .tp_thiscall_kw       = */ DEFIMPL(&default__thiscall_kw__with__call_kw),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ DEFIMPL(&default__call_tuple__with__call),
	/* .tp_call_tuple_kw     = */ DEFIMPL(&default__call_tuple_kw__with__call_kw),
	/* .tp_thiscall_tuple    = */ DEFIMPL(&default__thiscall_tuple__with__thiscall),
	/* .tp_thiscall_tuple_kw = */ DEFIMPL(&default__thiscall_tuple_kw__with__thiscall_kw),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

PUBLIC DeeTypeObject DeeKwCMethod_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_KwCMethod",
	/* .tp_doc      = */ DOC("call(args!,kwds!!)->"),
	/* .tp_flags    = */ TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (Dee_funptr_t)NULL,
				/* .tp_copy_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_deep_ctor = */ (Dee_funptr_t)NULL,
				/* .tp_any_ctor  = */ (Dee_funptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(DeeKwCMethodObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&default__str__with__print),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&kwcmethod_print,
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&kwcmethod_printrepr,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &clsmethod_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__2019F6A38C2B50B6),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ kwcmethod_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&kwcmethod_call,
	/* .tp_callable      = */ &kwcmethod_callable,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ kwcmethod_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(kwcmethod_operators),
};


/* Helpers for dynamically creating C method wrapper objects.
 * You really shouldn't use these (unless you *really* need to
 * wrap functions dynamically with no way of creating CMETHOD
 * objects statically). These are really only here to allow
 * for portable bindings of the deemon API in languages other
 * than C.
 *
 * If at all possible, use `Dee_DEFINE_CMETHOD' instead! */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCMethod_New(Dee_cmethod_t func, uintptr_t flags) {
	DREF DeeCMethodObject *result;
	result = DeeObject_MALLOC(DeeCMethodObject);
	if likely(result) {
		result->cm_func  = func;
		result->cm_flags = flags;
		DeeObject_Init(result, &DeeCMethod_Type);
	}
	return (DREF DeeObject *)result;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeKwCMethod_New(Dee_kwcmethod_t func, uintptr_t flags) {
	DREF DeeKwCMethodObject *result;
	result = DeeObject_MALLOC(DeeKwCMethodObject);
	if likely(result) {
		result->kcm_func  = func;
		result->kcm_flags = flags;
		DeeObject_Init(result, &DeeKwCMethod_Type);
	}
	return (DREF DeeObject *)result;
}


/* Invoke a given c-function callback.
 * Since this is the main way through which external functions are invoked,
 * we use this point to add some debug checks for proper use of exceptions.
 * That means we assert that the exception depth is manipulated as follows:
 * >> Dee_ASSERT(old_except_depth == new_except_depth + (return == NULL ? 1 : 0));
 *
 * This way we can quickly determine improper use of exceptions in most cases,
 * even before the interpreter would crash because it tried to handle exceptions
 * when there were none.
 *
 * A very common culprit for improper exception propagation is the return value
 * of printf-style format functions. Usually, code can just check for a non-zero
 * return value to determine if an integer-returning function has failed.
 * However, printf-style format functions return a negative value when that
 * happens, but return the positive sum of all printer calls on success (which
 * unless nothing was printed, is also non-zero).
 *
 * This can easily be fixed by replacing:
 * >> if (DeeFormat_Printf(...)) goto err;
 *
 * with this:
 * >> if (DeeFormat_Printf(...) < 0) goto err;
 */
#ifndef NDEBUG
INTDEF void DCALL assert_print_usercode_trace(void);

PRIVATE ATTR_NORETURN void DCALL
fatal_invalid_except(DeeObject *__restrict return_value,
                     uint16_t excepted, void *callback_addr) {
	Dee_DPRINT_SET_ENABLED(true); /* We're about to crash, so don't hide any details */
	Dee_DPRINTF("\n\n"
	            "FATAL ERROR: Exception depth was improperly modified:\n"
	            "After a return value %p from C-function %p, the exception "
	            /**/ "depth should have been %u, but was actually %u\n"
	            "For details, see the C documentation of `DeeCMethod_CallFunc'",
	            return_value, callback_addr, excepted, DeeThread_Self()->t_exceptsz);
	assert_print_usercode_trace();
	Dee_BREAKPOINT();
#if defined(CONFIG_HAVE_abort) && !defined(CONFIG_HAVE_abort_IS_ASSERT_XFAIL)
	abort();
#elif defined(CONFIG_HAVE__Exit)
	_Exit(EXIT_FAILURE);
#else /* ... */
	for (;;) {
		char volatile *volatile ptr;
		ptr  = (char volatile *)NULL;
		*ptr = 'X';
	}
#endif /* !... */
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCMethod_CallFunc_d(Dee_cmethod_t funptr, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeThreadObject *caller;
	uint16_t old_depth;
	caller    = DeeThread_Self();
	old_depth = caller->t_exceptsz;
	result    = (*funptr)(argc, argv);
	if unlikely(result ? old_depth != caller->t_exceptsz
	                   : old_depth + 1 != caller->t_exceptsz)
		fatal_invalid_except(result, old_depth + !result, (void *)funptr);
	return result;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeKwCMethod_CallFunc_d(Dee_kwcmethod_t funptr, size_t argc,
                        DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeThreadObject *caller;
	uint16_t old_depth;
	caller    = DeeThread_Self();
	old_depth = caller->t_exceptsz;
	result    = (*funptr)(argc, argv, kw);
	if unlikely(result ? old_depth != caller->t_exceptsz
	                   : old_depth + 1 != caller->t_exceptsz)
		fatal_invalid_except(result, old_depth + !result, (void *)funptr);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObjMethod_CallFunc_d(Dee_objmethod_t funptr, DeeObject *thisarg,
                        size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeThreadObject *caller;
	uint16_t old_depth;
	caller    = DeeThread_Self();
	old_depth = caller->t_exceptsz;
	result    = (*funptr)(thisarg, argc, argv);
	if unlikely(result ? old_depth != caller->t_exceptsz
	                   : old_depth + 1 != caller->t_exceptsz)
		fatal_invalid_except(result, old_depth + !result, (void *)funptr);
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeKwObjMethod_CallFunc_d(Dee_kwobjmethod_t funptr, DeeObject *thisarg,
                          size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeThreadObject *caller;
	uint16_t old_depth;
	caller    = DeeThread_Self();
	old_depth = caller->t_exceptsz;
	result    = (*funptr)(thisarg, argc, argv, kw);
	if unlikely(result ? old_depth != caller->t_exceptsz
	                   : old_depth + 1 != caller->t_exceptsz)
		fatal_invalid_except(result, old_depth + !result, (void *)funptr);
	return result;
}
#endif /* !NDEBUG */




/* Helpers for calling native deemon function with format arguments. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeCMethod_CallFuncf(Dee_cmethod_t funptr,
                     char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeCMethod_VCallFuncf(funptr, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
DeeObjMethod_CallFuncf(Dee_objmethod_t funptr, DeeObject *thisarg,
                       char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObjMethod_VCallFuncf(funptr, thisarg, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeKwCMethod_CallFuncf(Dee_kwcmethod_t funptr,
                       char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeKwCMethod_VCallFuncf(funptr, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
DeeKwObjMethod_CallFuncf(Dee_kwobjmethod_t funptr, DeeObject *thisarg,
                         char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeKwObjMethod_VCallFuncf(funptr, thisarg, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeCMethod_VCallFuncf(Dee_cmethod_t funptr,
                      char const *__restrict format, va_list args) {
	/* XXX: Optimizations? */
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeCMethod_CallFunc(funptr,
	                             DeeTuple_SIZE(args_tuple),
	                             DeeTuple_ELEM(args_tuple));
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeKwCMethod_VCallFuncf(Dee_kwcmethod_t funptr,
                        char const *__restrict format, va_list args) {
	/* XXX: Optimizations? */
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeKwCMethod_CallFunc(funptr,
	                               DeeTuple_SIZE(args_tuple),
	                               DeeTuple_ELEM(args_tuple),
	                               NULL);
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeObjMethod_VCallFuncf(Dee_objmethod_t funptr, DeeObject *thisarg,
                        char const *__restrict format, va_list args) {
	/* XXX: Optimizations? */
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObjMethod_CallFunc(funptr, thisarg,
	                               DeeTuple_SIZE(args_tuple),
	                               DeeTuple_ELEM(args_tuple));
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeKwObjMethod_VCallFuncf(Dee_kwobjmethod_t funptr, DeeObject *thisarg,
                          char const *__restrict format, va_list args) {
	/* XXX: Optimizations? */
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeKwObjMethod_CallFunc(funptr, thisarg,
	                                 DeeTuple_SIZE(args_tuple),
	                                 DeeTuple_ELEM(args_tuple),
	                                 NULL);
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_OBJMETHOD_C */
