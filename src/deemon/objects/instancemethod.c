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
#ifndef GUARD_DEEMON_OBJECTS_INSTANCEMETHOD_C
#define GUARD_DEEMON_OBJECTS_INSTANCEMETHOD_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_UnpackStructKw */
#include <deemon/callable.h>           /* DeeCallable_Type */
#include <deemon/class.h>              /* DeeClassDescriptorObject, DeeClass_DESC, Dee_CLASS_ATTRIBUTE_*, Dee_CLASS_GETSET_COUNT, Dee_class_attribute, Dee_class_desc, Dee_class_desc_lock_endread, Dee_class_desc_lock_read */
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>           /* DeeRT_ErrTUnboundAttr */
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/format.h>             /* DeeFormat_Printf */
#include <deemon/instancemethod.h>     /* DeeInstanceMethodObject */
#include <deemon/none.h>               /* DeeNone_Singleton, return_none */
#include <deemon/object.h>             /* ASSERT_OBJECT, DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_FROMBOOL, Dee_BOUND_YES, Dee_Decref_unlikely, Dee_Incref, Dee_Incref_n, Dee_TYPE, Dee_formatprinter_t, Dee_ssize_t, Dee_visit_t, OBJECT_HEAD_INIT, return_reference_ */
#include <deemon/string.h>             /* DeeString_STR */
#include <deemon/type.h>               /* DeeObject_Init, DeeTypeMRO, DeeTypeMRO_Init, DeeTypeMRO_Next, DeeType_IsClass, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, METHOD_F*, OPERATOR_*, STRUCT_OBJECT_AB, TF_NONE, TP_FNAMEOBJECT, TP_FNORMAL, TYPE_*, type_* */

#include "../runtime/kwlist.h"
#include "../runtime/strings.h"
#include "generic-proxy.h"

#include <stddef.h> /* NULL, offsetof, size_t */
#include <stdint.h> /* uint16_t */

DECL_BEGIN

typedef DeeInstanceMethodObject InstanceMethod;

/* Create a new instance method.
 *
 * This is a simple wrapper object that simply invokes a thiscall on
 * `im_func', using `this_arg' as the this-argument when called normally.
 *
 * In user-code, it is used to implement the temporary/split type when an
 * instance attribute with the `Dee_CLASS_ATTRIBUTE_FMETHOD' flag is loaded
 * as an object, rather than being called directly. */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeInstanceMethod_New(DeeObject *func,
                      DeeObject *this_arg) {
	DREF InstanceMethod *result;
	ASSERT_OBJECT(func);
	ASSERT_OBJECT(this_arg);
	result = DeeObject_MALLOC(InstanceMethod);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, &DeeInstanceMethod_Type);
	result->im_func = func;
	result->im_this = this_arg;
	Dee_Incref(func);
	Dee_Incref(this_arg);
	return Dee_AsObject(result);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeInstanceMethod_NewInherited(/*inherit(always)*/ DREF DeeObject *func,
                               /*inherit(always)*/ DREF DeeObject *this_arg) {
	DREF InstanceMethod *result;
	ASSERT_OBJECT(func);
	ASSERT_OBJECT(this_arg);
	result = DeeObject_MALLOC(InstanceMethod);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, &DeeInstanceMethod_Type);
	result->im_func = func;     /* Inherit reference */
	result->im_this = this_arg; /* Inherit reference */
	return Dee_AsObject(result);
err:
	Dee_Decref_unlikely(func);     /* *_unlikely because functions usually live until the module dies */
	Dee_Decref_unlikely(this_arg); /* *_unlikely because the instance is probably referenced elsewhere */
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
im_ctor(InstanceMethod *__restrict self) {
	/* Initialize a stub instance-method. */
	Dee_Incref_n(&DeeNone_Singleton, 2);
	self->im_func = Dee_AsObject(&DeeNone_Singleton);
	self->im_this = Dee_AsObject(&DeeNone_Singleton);
	return 0;
}

STATIC_ASSERT(offsetof(InstanceMethod, im_func) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(InstanceMethod, im_func) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(InstanceMethod, im_this) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(InstanceMethod, im_this) == offsetof(ProxyObject2, po_obj2));
#define im_fini          generic_proxy2__fini
#define im_visit         generic_proxy2__visit
#define im_copy          generic_proxy2__copy_alias12
#define im_serialize     generic_proxy2__serialize
#define im_hash          generic_proxy2__hash_recursive_ordered
#define im_compare_eq    generic_proxy2__compare_eq_recursive
#define im_trycompare_eq generic_proxy2__trycompare_eq_recursive
#define im_cmp           generic_proxy2__cmp_recursive_ordered

STATIC_ASSERT(offsetof(InstanceMethod, im_func) == offsetof(ProxyObject2, po_obj1));
STATIC_ASSERT(offsetof(InstanceMethod, im_this) == offsetof(ProxyObject2, po_obj2));
#define im_init generic_proxy2__init

PRIVATE WUNUSED NONNULL((1)) int DCALL
im_init_kw(InstanceMethod *__restrict self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	STATIC_ASSERT((offsetof(InstanceMethod, im_func) + sizeof(void *)) ==
	              (offsetof(InstanceMethod, im_this)));
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__func_thisarg,
	                          "oo:InstanceMethod", &self->im_func))
		goto err;
	Dee_Incref(self->im_this);
	Dee_Incref(self->im_func);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
im_printrepr(InstanceMethod *__restrict self,
             Dee_formatprinter_t printer, void *arg) {
	return DeeFormat_Printf(printer, arg,
	                        "InstanceMethod(func: %r, thisarg: %r)",
	                        self->im_func, self->im_this);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
im_call(InstanceMethod *self, size_t argc, DeeObject *const *argv) {
	return DeeObject_ThisCall(self->im_func, self->im_this, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
im_call_kw(InstanceMethod *self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	return DeeObject_ThisCallKw(self->im_func, self->im_this, argc, argv, kw);
}

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
im_call_tuple(InstanceMethod *self, DeeObject *args) {
	return DeeObject_ThisCallTuple(self->im_func, self->im_this, args);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
im_call_tuple_kw(InstanceMethod *self,  DeeObject *args, DeeObject *kw) {
	return DeeObject_ThisCallTupleKw(self->im_func, self->im_this, args, kw);
}
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

PRIVATE WUNUSED NONNULL((1)) struct Dee_class_attribute *DCALL
instancemethod_getattr(InstanceMethod *__restrict self,
                       uint16_t *p_getset_index,
                       DeeTypeObject **p_decl_type) {
	struct Dee_class_attribute *result;
	DeeTypeObject *tp_iter;
	DeeTypeMRO mro;
	tp_iter = Dee_TYPE(self->im_this);
	DeeTypeMRO_Init(&mro, tp_iter);
	do {
		DeeClassDescriptorObject *desc;
		struct Dee_class_desc *my_class;
		uint16_t addr;
		size_t i;
		if (!DeeType_IsClass(tp_iter))
			break;
		my_class = DeeClass_DESC(tp_iter);
		desc     = my_class->cd_desc;
		Dee_class_desc_lock_read(my_class);
		for (addr = 0; addr < desc->cd_cmemb_size; ++addr) {
			if (my_class->cd_members[addr] != self->im_func)
				continue;
			Dee_class_desc_lock_endread(my_class);
			/* Found the address at which the function is located. */
			for (i = 0; i <= desc->cd_iattr_mask; ++i) {
				result = &desc->cd_iattr_list[i];
				if (!result->ca_name)
					continue;
				if (addr < result->ca_addr)
					continue;
				if ((result->ca_flag & (Dee_CLASS_ATTRIBUTE_FCLASSMEM | Dee_CLASS_ATTRIBUTE_FMETHOD)) !=
				    (Dee_CLASS_ATTRIBUTE_FCLASSMEM | Dee_CLASS_ATTRIBUTE_FMETHOD))
					continue;
				if (result->ca_flag & Dee_CLASS_ATTRIBUTE_FGETSET) {
					if (addr >= result->ca_addr + Dee_CLASS_GETSET_COUNT)
						continue;
					if (p_getset_index)
						*p_getset_index = (uint16_t)(addr - result->ca_addr);
				} else {
					if (addr > result->ca_addr)
						continue;
					if (p_getset_index)
						*p_getset_index = 0;
				}
				if (p_decl_type)
					*p_decl_type = tp_iter;
				return result;
			}
			Dee_class_desc_lock_read(my_class);
		}
		Dee_class_desc_lock_endread(my_class);
	} while ((tp_iter = DeeTypeMRO_Next(&mro, tp_iter)) != NULL);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemethod_get_name(InstanceMethod *__restrict self) {
	struct Dee_class_attribute *attr;
	attr = instancemethod_getattr(self, NULL, NULL);
	if (attr)
		return_reference_(Dee_AsObject(attr->ca_name));
	return DeeObject_GetAttr(self->im_func, Dee_AsObject(&str___name__));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
instancemethod_bound_name(InstanceMethod *__restrict self) {
	struct Dee_class_attribute *attr;
	attr = instancemethod_getattr(self, NULL, NULL);
	if (attr)
		return Dee_BOUND_YES;
	return DeeObject_BoundAttr(self->im_func, Dee_AsObject(&str___name__));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemethod_get_doc(InstanceMethod *__restrict self) {
	struct Dee_class_attribute *attr;
	attr = instancemethod_getattr(self, NULL, NULL);
	if (!attr)
		goto return_attr;
	if (!attr->ca_doc)
		return_none;
	return_reference_(Dee_AsObject(attr->ca_doc));
	{
		DREF DeeObject *result;
return_attr:
		result = DeeObject_GetAttr(self->im_func, Dee_AsObject(&str___doc__));
		if unlikely(!result) {
			if (DeeError_Catch(&DeeError_AttributeError) ||
			    DeeError_Catch(&DeeError_NotImplemented))
				return_none;
		}
		return result;
	}
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemethod_get_kwds(InstanceMethod *__restrict self) {
	return DeeObject_GetAttr(self->im_func, Dee_AsObject(&str___kwds__));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
instancemethod_bound_kwds(InstanceMethod *__restrict self) {
	return DeeObject_BoundAttr(self->im_func, Dee_AsObject(&str___kwds__));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
instancemethod_get_type(InstanceMethod *__restrict self) {
	DeeTypeObject *result;
	if (instancemethod_getattr(self, NULL, &result))
		return_reference_(result);
	DeeRT_ErrTUnboundAttr(&DeeInstanceMethod_Type, Dee_AsObject(self), Dee_AsObject(&str___type__));
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
instancemethod_bound_type(InstanceMethod *__restrict self) {
	struct Dee_class_attribute *attr;
	DeeTypeObject *result;
	attr = instancemethod_getattr(self, NULL, &result);
	return Dee_BOUND_FROMBOOL(attr);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemethod_get_module(InstanceMethod *__restrict self) {
	return DeeObject_GetAttr(self->im_func, Dee_AsObject(&str___module__));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
instancemethod_bound_module(InstanceMethod *__restrict self) {
	return DeeObject_BoundAttr(self->im_func, Dee_AsObject(&str___module__));
}

PRIVATE struct type_callable im_callable = {
	/* .tp_call_kw           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&im_call_kw,
	/* .tp_thiscall          = */ DEFIMPL(&default__thiscall__with__call),
	/* .tp_thiscall_kw       = */ DEFIMPL(&default__thiscall_kw__with__call_kw),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_call_tuple        = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&im_call_tuple,
	/* .tp_call_tuple_kw     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&im_call_tuple_kw,
	/* .tp_thiscall_tuple    = */ DEFIMPL(&default__thiscall_tuple__with__thiscall),
	/* .tp_thiscall_tuple_kw = */ DEFIMPL(&default__thiscall_tuple_kw__with__thiscall_kw),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

PRIVATE struct type_getset tpconst im_getsets[] = {
	TYPE_GETTER_BOUND_F(STR___name__, &instancemethod_get_name, &instancemethod_bound_name,
	                    METHOD_FNOREFESCAPE,
	                    "->?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "The name of the function being bound"),
	TYPE_GETTER_AB_F(STR___doc__, &instancemethod_get_doc,
	                 METHOD_FNOREFESCAPE,
	                 "->?X2?Dstring?N\n"
	                 "The documentation string of the function being bound, or ?N if unknown"),
	TYPE_GETTER_BOUND_F(STR___kwds__, &instancemethod_get_kwds, &instancemethod_bound_kwds,
	                    METHOD_FNOREFESCAPE,
	                    "->?S?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "Returns a sequence of keyword argument names accepted by @this function\n"
	                    "If @this function doesn't accept keyword arguments, an empty sequence is returned"),
	TYPE_GETTER_BOUND_F(STR___type__, &instancemethod_get_type, &instancemethod_bound_type,
	                    METHOD_FNOREFESCAPE,
	                    "->?DType\n"
	                    "#t{UnboundAttribute}"
	                    "The type implementing the function that is being bound"),
	TYPE_GETTER_BOUND_F(STR___module__, &instancemethod_get_module, &instancemethod_bound_module,
	                    METHOD_FNOREFESCAPE,
	                    "->?DModule\n"
	                    "#t{UnboundAttribute}"
	                    "The type within which the bound function was defined "
	                    "(alias for ?A__module__?DFunction though ${__func__.__module__})\n"
	                    "If something other than a user-level function was set for ?#__func__, "
	                    /**/ "a $\"__module__\" attribute will be loaded from it, with its value "
	                    /**/ "then forwarded"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst im_members[] = {
	TYPE_MEMBER_FIELD_DOC("__this__", STRUCT_OBJECT_AB, offsetof(InstanceMethod, im_this),
	                      "->\n"
	                      "The object to which @this ?DInstanceMethod is bound"),
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT_AB, offsetof(InstanceMethod, im_func),
	                      "->?DCallable\n"
	                      "The unbound class-function that is being bound by this ?DInstanceMethod"),
	TYPE_MEMBER_END
};

PRIVATE struct type_operator const im_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FIELDS_CONSTREPR),
	TYPE_OPERATOR_FLAGS(OPERATOR_000A_CALL, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FUNC_IS_CONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FIELDS_CONSTCMPEQ),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_FIELDS_CONSTCMPEQ),
};

PUBLIC DeeTypeObject DeeInstanceMethod_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_InstanceMethod),
	/* .tp_doc      = */ DOC("(func:?DCallable,thisarg)\n"
	                         "Construct an object-bound instance method that can be used to invoke @func\n"
	                         "\n"

	                         "call(args!,kwds!!)->\n"
	                         "Invoke the $func used to construct @this ?. "
	                         /**/ "as ${func(thisarg, args..., **kwds)}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ InstanceMethod,
			/* tp_ctor:        */ &im_ctor,
			/* tp_copy_ctor:   */ &im_copy,
			/* tp_any_ctor:    */ &im_init,
			/* tp_any_ctor_kw: */ &im_init_kw,
			/* tp_serialize:   */ &im_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&im_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&im_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&im_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ &im_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ im_getsets,
	/* .tp_members       = */ im_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&im_call,
	/* .tp_callable      = */ &im_callable,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ im_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(im_operators),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_INSTANCEMETHOD_C */
