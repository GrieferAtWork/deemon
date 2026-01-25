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
#ifndef GUARD_DEEMON_OBJECTS_PROPERTY_C
#define GUARD_DEEMON_OBJECTS_PROPERTY_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_UnpackKw */
#include <deemon/bool.h>
#include <deemon/code.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/property.h>
#include <deemon/serial.h>
#include <deemon/string.h>

#include "../runtime/kwlist.h"
#include "../runtime/strings.h"

#include <stddef.h> /* NULL, offsetof, size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

#define DO(err, expr)                    \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0


typedef DeePropertyObject Property;

PRIVATE WUNUSED NONNULL((1)) int DCALL
property_ctor(Property *__restrict self) {
	self->p_get = NULL;
	self->p_del = NULL;
	self->p_set = NULL;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
property_copy(Property *__restrict self,
              Property *__restrict other) {
	self->p_get = other->p_get;
	self->p_del = other->p_del;
	self->p_set = other->p_set;
	Dee_XIncref(self->p_get);
	Dee_XIncref(self->p_del);
	Dee_XIncref(self->p_set);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
property_deep(Property *__restrict self,
              Property *__restrict other) {
	self->p_get = NULL;
	self->p_del = NULL;
	self->p_set = NULL;
	if (other->p_get) {
		if unlikely((self->p_get = DeeObject_DeepCopy(other->p_get)) == NULL)
			goto err;
	}
	if (other->p_del) {
		if unlikely((self->p_del = DeeObject_DeepCopy(other->p_del)) == NULL)
			goto err_get;
	}
	if (other->p_set) {
		if unlikely((self->p_set = DeeObject_DeepCopy(other->p_set)) == NULL)
			goto err_del;
	}
	return 0;
err_del:
	Dee_XDecref(self->p_del);
err_get:
	Dee_XDecref(self->p_get);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
property_init_kw(Property *__restrict self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	self->p_get = NULL;
	self->p_del = NULL;
	self->p_set = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw,
	                    kwlist__getter_delete_setter,
	                    "|ooo:Property",
	                    &self->p_get, &self->p_del, &self->p_set))
		goto err;
	if (DeeNone_Check(self->p_get))
		self->p_get = NULL;
	if (DeeNone_Check(self->p_del))
		self->p_del = NULL;
	if (DeeNone_Check(self->p_set))
		self->p_set = NULL;
	Dee_XIncref(self->p_get);
	Dee_XIncref(self->p_del);
	Dee_XIncref(self->p_set);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
property_serialize(Property *__restrict self,
                   DeeSerial *__restrict writer,
                   Dee_seraddr_t addr) {
	int result = DeeSerial_XPutObject(writer, addr + offsetof(Property, p_get), self->p_get);
	if likely(result == 0)
		result = DeeSerial_XPutObject(writer, addr + offsetof(Property, p_del), self->p_del);
	if likely(result == 0)
		result = DeeSerial_XPutObject(writer, addr + offsetof(Property, p_set), self->p_set);
	return result;
}

PRIVATE NONNULL((1)) void DCALL
property_fini(Property *__restrict self) {
	Dee_XDecref(self->p_get);
	Dee_XDecref(self->p_del);
	Dee_XDecref(self->p_set);
}

PRIVATE NONNULL((1, 2)) void DCALL
property_visit(Property *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_XVisit(self->p_get);
	Dee_XVisit(self->p_del);
	Dee_XVisit(self->p_set);
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
property_hash(Property *__restrict self) {
	Dee_hash_t result = 0;
	if (self->p_get != NULL)
		result = Dee_HashCombine(result, DeeObject_Hash(self->p_get));
	if (self->p_del != NULL)
		result = Dee_HashCombine(result, DeeObject_Hash(self->p_del));
	if (self->p_set != NULL)
		result = Dee_HashCombine(result, DeeObject_Hash(self->p_set));
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
property_compare_eq(Property *self, Property *other) {
	int result = Dee_COMPARE_EQ;
	if (DeeObject_AssertType(other, &DeeProperty_Type))
		goto err;

	/* Make sure that the same callbacks are implemented. */
	if ((self->p_get != NULL) != (other->p_get != NULL))
		goto nope;
	if ((self->p_del != NULL) != (other->p_del != NULL))
		goto nope;
	if ((self->p_set != NULL) != (other->p_set != NULL))
		goto nope;

	/* Compare individual callbacks. */
	if (self->p_get)
		result = DeeObject_TryCompareEq(self->p_get, other->p_get);
	if (self->p_del && result == Dee_COMPARE_EQ)
		result = DeeObject_TryCompareEq(self->p_del, other->p_del);
	if (self->p_set && result == Dee_COMPARE_EQ)
		result = DeeObject_TryCompareEq(self->p_set, other->p_set);
	return result;
nope:
	return Dee_COMPARE_NE;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp property_cmp = {
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *__restrict))&property_hash,
	/* .tp_compare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&property_compare_eq,
	/* .tp_compare       = */ DEFIMPL_UNSUPPORTED(&default__compare__unsupported),
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL_UNSUPPORTED(&default__lo__unsupported),
	/* .tp_le            = */ DEFIMPL_UNSUPPORTED(&default__le__unsupported),
	/* .tp_gr            = */ DEFIMPL_UNSUPPORTED(&default__gr__unsupported),
	/* .tp_ge            = */ DEFIMPL_UNSUPPORTED(&default__ge__unsupported),
};

PRIVATE struct type_member tpconst property_members[] = {
	TYPE_MEMBER_FIELD_DOC("getter", STRUCT_OBJECT, offsetof(Property, p_get),
	                      "->?DCallable\n"
	                      "The getter callback"),
	TYPE_MEMBER_FIELD_DOC("delete", STRUCT_OBJECT, offsetof(Property, p_del),
	                      "->?DCallable\n"
	                      "The delete callback"),
	TYPE_MEMBER_FIELD_DOC("setter", STRUCT_OBJECT, offsetof(Property, p_set),
	                      "->?DCallable\n"
	                      "The setter callback"),
	TYPE_MEMBER_FIELD_DOC("get", STRUCT_OBJECT, offsetof(Property, p_get),
	                      "->?DCallable\n"
	                      "Alias for ?#getter"),
	TYPE_MEMBER_FIELD_DOC(STR_set, STRUCT_OBJECT, offsetof(Property, p_set),
	                      "->?DCallable\n"
	                      "Alias for ?#setter"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
property_canget(Property *__restrict self) {
	return_bool(self->p_get != NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
property_candel(Property *__restrict self) {
	return_bool(self->p_del != NULL);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
property_canset(Property *__restrict self) {
	return_bool(self->p_set != NULL);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
property_info(Property *__restrict self,
              struct Dee_function_info *__restrict info) {
	int result;
	if (self->p_get && DeeFunction_Check(self->p_get)) {
		result = DeeFunction_GetInfo(self->p_get, info);
		if (result <= 0)
			goto done;
	}
	if (self->p_del && DeeFunction_Check(self->p_del)) {
		result = DeeFunction_GetInfo(self->p_del, info);
		if (result <= 0)
			goto done;
	}
	if (self->p_set && DeeFunction_Check(self->p_set)) {
		result = DeeFunction_GetInfo(self->p_set, info);
		if (result <= 0)
			goto done;
	}

	/* No information available :( */
	info->fi_type   = NULL;
	info->fi_name   = NULL;
	info->fi_doc    = NULL;
	info->fi_opname = (Dee_operator_t)-1;
	info->fi_getset = (uint16_t)-1;
	result = 1;
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
property_callback_getattr(Property *self, DeeStringObject *name) {
	DREF DeeObject *result;
	if (self->p_get) {
		result = DeeObject_GetAttr(self->p_get, Dee_AsObject(name));
	} else if (self->p_del) {
		result = DeeObject_GetAttr(self->p_del, Dee_AsObject(name));
	} else if (self->p_set) {
		result = DeeObject_GetAttr(self->p_set, Dee_AsObject(name));
	} else {
		return ITER_DONE;
	}
	if (result == NULL && DeeError_Catch(&DeeError_AttributeError))
		result = ITER_DONE;
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
property_callback_boundattr(Property *self, DeeStringObject *name) {
	int result;
	if (self->p_get) {
		result = DeeObject_BoundAttr(self->p_get, Dee_AsObject(name));
	} else if (self->p_del) {
		result = DeeObject_BoundAttr(self->p_del, Dee_AsObject(name));
	} else if (self->p_set) {
		result = DeeObject_BoundAttr(self->p_set, Dee_AsObject(name));
	} else {
		result = Dee_BOUND_NO;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
property_get_name(Property *__restrict self) {
	struct Dee_function_info info;
	int error;
	DREF DeeObject *result;
	error = property_info(self, &info);
	if unlikely(error < 0)
		goto err;
	Dee_XDecref(info.fi_doc);
	Dee_XDecref(info.fi_type);
	if (info.fi_name)
		return Dee_AsObject(info.fi_name);
	result = property_callback_getattr(self, &str___name__);
	if (result != ITER_DONE)
		return result;
	DeeRT_ErrTUnboundAttr(&DeeProperty_Type, self, &str___name__);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
property_bound_name(Property *__restrict self) {
	int error;
	struct Dee_function_info info;
	error = property_info(self, &info);
	if unlikely(error < 0)
		goto err;
	Dee_XDecref(info.fi_doc);
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_name);
	if (info.fi_name)
		return Dee_BOUND_YES;
	return property_callback_boundattr(self, &str___name__);
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
property_get_doc(Property *__restrict self) {
	struct Dee_function_info info;
	int error;
	DREF DeeObject *result;
	error = property_info(self, &info);
	if unlikely(error < 0)
		goto err;
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_type);
	if (info.fi_doc)
		return Dee_AsObject(info.fi_doc);
	result = property_callback_getattr(self, &str___doc__);
	if (result != ITER_DONE)
		return result;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
property_bound_doc(Property *__restrict self) {
	struct Dee_function_info info;
	int error;
	error = property_info(self, &info);
	if unlikely(error < 0)
		goto err;
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_type);
	Dee_XDecref(info.fi_doc);
	if (info.fi_doc)
		return Dee_BOUND_YES;
	return property_callback_boundattr(self, &str___doc__);
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
property_get_type(Property *__restrict self) {
	struct Dee_function_info info;
	int error;
	DREF DeeTypeObject *result;
	error = property_info(self, &info);
	if unlikely(error < 0)
		goto err;
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	if (info.fi_type)
		return info.fi_type;
	result = (DREF DeeTypeObject *)property_callback_getattr(self, &str___type__);
	if (result != (DREF DeeTypeObject *)ITER_DONE)
		return result;
	DeeRT_ErrTUnboundAttr(&DeeProperty_Type, self, &str___type__);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
property_bound_type(Property *__restrict self) {
	struct Dee_function_info info;
	int error;
	error = property_info(self, &info);
	if unlikely(error < 0)
		goto err;
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	Dee_XDecref(info.fi_type);
	if (info.fi_type)
		return Dee_BOUND_YES;
	return property_callback_boundattr(self, &str___type__);
err:
	return Dee_BOUND_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
get_function_module(DeeFunctionObject *__restrict self) {
	DeeModuleObject *mod = self->fo_code->co_module;
	if likely(mod)
		return_reference_((DeeObject *)mod);
	return DeeRT_ErrUnboundAttr(self, &str___module__);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bound_function_module(DeeFunctionObject *__restrict self) {
	DeeModuleObject *mod = self->fo_code->co_module;
	return Dee_BOUND_FROMBOOL(mod);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
property_get_module(Property *__restrict self) {
	DREF DeeObject *result;
	if (self->p_get && DeeFunction_Check(self->p_get))
		return get_function_module((DeeFunctionObject *)self->p_get);
	if (self->p_del && DeeFunction_Check(self->p_del))
		return get_function_module((DeeFunctionObject *)self->p_del);
	if (self->p_set && DeeFunction_Check(self->p_set))
		return get_function_module((DeeFunctionObject *)self->p_set);
	result = property_callback_getattr(self, &str___module__);
	if (result != ITER_DONE)
		return result;
	return DeeRT_ErrTUnboundAttr(&DeeProperty_Type, self, &str___module__);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
property_bound_module(Property *__restrict self) {
	if (self->p_get && DeeFunction_Check(self->p_get))
		return bound_function_module((DeeFunctionObject *)self->p_get);
	if (self->p_del && DeeFunction_Check(self->p_del))
		return bound_function_module((DeeFunctionObject *)self->p_del);
	if (self->p_set && DeeFunction_Check(self->p_set))
		return bound_function_module((DeeFunctionObject *)self->p_set);
	return property_callback_boundattr(self, &str___module__);
}


PRIVATE struct type_getset tpconst property_getsets[] = {
	TYPE_GETTER_AB_F("canget", &property_canget,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this Property has a getter callback"),
	TYPE_GETTER_AB_F("candel", &property_candel,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this Property has a delete callback"),
	TYPE_GETTER_AB_F("canset", &property_canset,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "Returns ?t if @this Property has a setter callback"),
	TYPE_GETTER_BOUND_F(STR___name__, &property_get_name, &property_bound_name,
	                    METHOD_FNOREFESCAPE,
	                    "->?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "Returns the name of @this Property"),
	TYPE_GETTER_BOUND_F(STR___doc__, &property_get_doc, &property_bound_doc,
	                    METHOD_FNOREFESCAPE,
	                    "->?Dstring\n"
	                    "Returns the documentation string of @this Property"),
	TYPE_GETTER_BOUND_F(STR___type__, &property_get_type, &property_bound_type,
	                    METHOD_FNOREFESCAPE,
	                    "->?DType\n"
	                    "#t{UnboundAttribute}"
	                    "Returns the type implementing @this Property, or ?N if unknown"),
	TYPE_GETTER_BOUND_F(STR___module__, &property_get_module, &property_bound_module,
	                    METHOD_FNOREFESCAPE,
	                    "->?DModule\n"
	                    "#t{UnboundAttribute}"
	                    "Returns the module within which @this Property is declared"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
property_call(Property *self, size_t argc, DeeObject *const *argv) {
	if likely(self->p_get)
		return DeeObject_Call(self->p_get, argc, argv);
	DeeRT_ErrTUnboundAttrCStr(&DeeProperty_Type, Dee_AsObject(self), "getter");
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
property_call_kw(Property *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	if likely(self->p_get)
		return DeeObject_CallKw(self->p_get, argc, argv, kw);
	DeeRT_ErrTUnboundAttrCStr(&DeeProperty_Type, Dee_AsObject(self), "getter");
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
property_printrepr(Property *__restrict self,
                   Dee_formatprinter_t printer, void *arg) {
	/* TODO: Better distinction between *actual* properties, and
	 *       custom properties (w/ an attribute `iscustom: bool',
	 *       and us making the repr dependent on that) */
	Dee_ssize_t temp, result;
	struct Dee_function_info info;
	int error;
	error = property_info(self, &info);
	if unlikely(error < 0)
		goto err_m1;

	/* Special handling for when this is an unbound class property. */
	if (info.fi_type) {
		DeeModuleObject *mod = NULL;
		if (self->p_get && DeeFunction_Check(self->p_get))
			mod = ((DeeFunctionObject *)self->p_get)->fo_code->co_module;
		if (self->p_del && DeeFunction_Check(self->p_del) && !mod)
			mod = ((DeeFunctionObject *)self->p_del)->fo_code->co_module;
		if (self->p_set && DeeFunction_Check(self->p_set) && !mod)
			mod = ((DeeFunctionObject *)self->p_set)->fo_code->co_module;
		result = mod ? DeeFormat_PrintObjectRepr(printer, arg, (DeeObject *)mod)
		             : DeeFormat_PRINT(printer, arg, "<unknown module>");
		if likely(result >= 0) {
			if (info.fi_name) {
				temp = DeeFormat_Printf(printer, arg, ".%k.%k",
				                        info.fi_type, info.fi_name);
			} else {
				temp = DeeFormat_Printf(printer, arg, ".%k.<unknown>",
				                        info.fi_type);
			}
			if likely(temp >= 0) {
				result += temp;
			} else {
				result = temp;
			}
		}
		Dee_function_info_fini(&info);
		return result;
	}
	Dee_function_info_fini(&info);

	result = DeeFormat_PRINT(printer, arg, "Property(");
	if unlikely(result < 0)
		goto done;
	if (self->p_get)
		DO(err, DeeFormat_Printf(printer, arg, "getter: %r", self->p_get));
	if (self->p_del)
		DO(err, DeeFormat_Printf(printer, arg, "%sdelete: %r", self->p_get ? ", " : "", self->p_del));
	if (self->p_set)
		DO(err, DeeFormat_Printf(printer, arg, "%ssetter: %r", (self->p_get || self->p_del) ? ", " : "", self->p_set));
	DO(err, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err_m1:
	temp = -1;
err:
	return temp;
}

PRIVATE struct type_callable property_callable = {
	/* .tp_call_kw = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&property_call_kw,
	/* .tp_thiscall          = */ DEFIMPL(&default__thiscall__with__call),
	/* .tp_thiscall_kw       = */ DEFIMPL(&default__thiscall_kw__with__call_kw),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ DEFIMPL(&default__call_tuple__with__call),
	/* .tp_call_tuple_kw     = */ DEFIMPL(&default__call_tuple_kw__with__call_kw),
	/* .tp_thiscall_tuple    = */ DEFIMPL(&default__thiscall_tuple__with__thiscall),
	/* .tp_thiscall_tuple_kw = */ DEFIMPL(&default__thiscall_tuple_kw__with__thiscall_kw),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};


/* `Property from deemon' */
PUBLIC DeeTypeObject DeeProperty_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Property),
	/* .tp_doc      = */ DOC("(getter:?DCallable=!N,delete:?DCallable=!N,setter:?DCallable=!N)\n"
	                         "\n"

	                         "call(args!,kwds!!)->\n"
	                         "Same as ${this.get(args..., **kwds)}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ Property,
			/* tp_ctor:        */ &property_ctor,
			/* tp_copy_ctor:   */ &property_copy,
			/* tp_deep_ctor:   */ &property_deep,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ &property_init_kw,
			/* tp_serialize:   */ &property_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&property_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&property_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&property_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &property_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ property_getsets,
	/* .tp_members       = */ property_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&property_call,
	/* .tp_callable      = */ &property_callable,
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_PROPERTY_C */
