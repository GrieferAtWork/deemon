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
#ifndef GUARD_DEEMON_OBJECTS_INSTANCEMETHOD_C
#define GUARD_DEEMON_OBJECTS_INSTANCEMETHOD_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/callable.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/instancemethod.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/util/atomic.h>

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

typedef DeeInstanceMethodObject InstanceMethod;

/* Create a new instance method.
 * This is a simple wrapper object that simply invokes a thiscall on
 * `im_func', using `this_arg' as the this-argument when called normally.
 * In user-code, it is used to implement the temporary/split type when
 * an instance attribute with the `CLASS_ATTRIBUTE_FMETHOD' flag is loaded
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
	return (DREF DeeObject *)result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
im_ctor(InstanceMethod *__restrict self) {
	/* Initialize a stub instance-method. */
	self->im_func = Dee_None;
	self->im_this = Dee_None;
	Dee_Incref_n(Dee_None, 2);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
im_copy(InstanceMethod *__restrict self,
        InstanceMethod *__restrict other) {
	self->im_this = other->im_this;
	self->im_func = other->im_func;
	Dee_Incref(self->im_this);
	Dee_Incref(self->im_func);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
im_deepcopy(InstanceMethod *__restrict self,
            InstanceMethod *__restrict other) {
	self->im_this = DeeObject_DeepCopy(other->im_this);
	if unlikely(!self->im_this)
		goto err;
	self->im_func = DeeObject_DeepCopy(other->im_func);
	if unlikely(!self->im_func)
		goto err_im_this;
	return 0;
err_im_this:
	Dee_Decref(self->im_this);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
im_init_kw(InstanceMethod *__restrict self, size_t argc,
           DeeObject *const *argv, DeeObject *kw) {
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__func_thisarg,
	                    "oo:InstanceMethod",
	                    &self->im_func, &self->im_this))
		goto err;
	Dee_Incref(self->im_this);
	Dee_Incref(self->im_func);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
im_printrepr(InstanceMethod *__restrict self,
             dformatprinter printer, void *arg) {
	return DeeFormat_Printf(printer, arg,
	                        "InstanceMethod(func: %r, thisarg: %r)",
	                        self->im_func, self->im_this);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
im_call(InstanceMethod *self, size_t argc, DeeObject *const *argv) {
	return DeeObject_ThisCall(self->im_func, self->im_this, argc, argv);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
im_callkw(InstanceMethod *self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	return DeeObject_ThisCallKw(self->im_func, self->im_this, argc, argv, kw);
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
im_hash(InstanceMethod *__restrict self) {
	return Dee_HashCombine(DeeObject_Hash(self->im_func),
	                       DeeObject_Hash(self->im_this));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
im_compare_eq(InstanceMethod *self, InstanceMethod *other) {
	int result;
	if (DeeObject_AssertType(other, &DeeInstanceMethod_Type))
		goto err;
	result = DeeObject_CompareEq(self->im_func, other->im_func);
	if (result == 0)
		result = DeeObject_CompareEq(self->im_this, other->im_this);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
im_trycompare_eq(InstanceMethod *self, InstanceMethod *other) {
	int result;
	if (!DeeInstanceMethod_Check(other))
		return -1;
	result = DeeObject_TryCompareEq(self->im_func, other->im_func);
	if (result == 0)
		result = DeeObject_TryCompareEq(self->im_this, other->im_this);
	return result;
}

PRIVATE struct type_cmp im_cmp = {
	/* .tp_hash          = */ (dhash_t (DCALL *)(DeeObject *__restrict))&im_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&im_compare_eq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&im_trycompare_eq,
};


STATIC_ASSERT(sizeof(DeeSuperObject) == sizeof(InstanceMethod));
STATIC_ASSERT(offsetof(DeeSuperObject, s_type) == offsetof(InstanceMethod, im_func));
STATIC_ASSERT(offsetof(DeeSuperObject, s_self) == offsetof(InstanceMethod, im_this));

/* Since `super' and `InstanceMethod' share an identical
 * layout, we can re-use some operators here... */
INTDEF NONNULL((1)) void DCALL super_fini(DeeSuperObject *__restrict self);
INTDEF NONNULL((1, 2)) void DCALL super_visit(DeeSuperObject *__restrict self, dvisit_t proc, void *arg);
#define im_fini  super_fini
#define im_visit super_visit

PRIVATE WUNUSED NONNULL((1)) struct class_attribute *DCALL
instancemethod_getattr(InstanceMethod *__restrict self,
                       uint16_t *p_getset_index,
                       DeeTypeObject **p_decl_type) {
	struct class_attribute *result;
	DeeTypeObject *tp_iter;
	DeeTypeMRO mro;
	tp_iter = Dee_TYPE(self->im_this);
	DeeTypeMRO_Init(&mro, tp_iter);
	do {
		DeeClassDescriptorObject *desc;
		struct class_desc *my_class;
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
				if ((result->ca_flag & (CLASS_ATTRIBUTE_FCLASSMEM | CLASS_ATTRIBUTE_FMETHOD)) !=
				    (CLASS_ATTRIBUTE_FCLASSMEM | CLASS_ATTRIBUTE_FMETHOD))
					continue;
				if (result->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
					if (addr >= result->ca_addr + CLASS_GETSET_COUNT)
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
	struct class_attribute *attr;
	attr = instancemethod_getattr(self, NULL, NULL);
	if (attr)
		return_reference_((DeeObject *)attr->ca_name);
	return DeeObject_GetAttr(self->im_func, (DeeObject *)&str___name__);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
instancemethod_bound_name(InstanceMethod *__restrict self) {
	struct class_attribute *attr;
	attr = instancemethod_getattr(self, NULL, NULL);
	if (attr)
		return 1;
	return DeeObject_BoundAttr(self->im_func, (DeeObject *)&str___name__);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemethod_get_doc(InstanceMethod *__restrict self) {
	struct class_attribute *attr;
	attr = instancemethod_getattr(self, NULL, NULL);
	if (!attr)
		goto return_attr;
	if (!attr->ca_doc)
		return_none;
	return_reference_((DeeObject *)attr->ca_doc);
	{
		DREF DeeObject *result;
return_attr:
		result = DeeObject_GetAttr(self->im_func, (DeeObject *)&str___doc__);
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
	return DeeObject_GetAttr(self->im_func, (DeeObject *)&str___kwds__);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
instancemethod_bound_kwds(InstanceMethod *__restrict self) {
	return DeeObject_BoundAttr(self->im_func, (DeeObject *)&str___kwds__);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
instancemethod_get_type(InstanceMethod *__restrict self) {
	DeeTypeObject *result;
	if (instancemethod_getattr(self, NULL, &result))
		return_reference_(result);
	err_unbound_attribute_string(&DeeInstanceMethod_Type, STR___type__);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
instancemethod_bound_type(InstanceMethod *__restrict self) {
	DeeTypeObject *result;
	if (instancemethod_getattr(self, NULL, &result))
		return 1;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instancemethod_get_module(InstanceMethod *__restrict self) {
	return DeeObject_GetAttr(self->im_func, (DeeObject *)&str___module__);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
instancemethod_bound_module(InstanceMethod *__restrict self) {
	return DeeObject_BoundAttr(self->im_func, (DeeObject *)&str___module__);
}

PRIVATE struct type_getset tpconst im_getsets[] = {
	TYPE_GETTER_BOUND_F(STR___name__, &instancemethod_get_name, &instancemethod_bound_name,
	                    METHOD_FNOREFESCAPE,
	                    "->?Dstring\n"
	                    "#t{UnboundAttribute}"
	                    "The name of the function being bound"),
	TYPE_GETTER_F(STR___doc__, &instancemethod_get_doc,
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
	TYPE_MEMBER_FIELD_DOC("__this__", STRUCT_OBJECT, offsetof(InstanceMethod, im_this),
	                      "->\n"
	                      "The object to which @this ?DInstanceMethod is bound"),
	TYPE_MEMBER_FIELD_DOC("__func__", STRUCT_OBJECT, offsetof(InstanceMethod, im_func),
	                      "->?DCallable\n"
	                      "The unbound class-function that is being bound by this ?DInstanceMethod"),
	TYPE_MEMBER_END
};

PRIVATE struct type_operator const im_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
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

	                         "call(args!)->\n"
	                         "Invoke the $func used to construct @this "
	                         /**/ "InstanceMethod as ${func(thisarg, args...)}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeCallable_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (dfunptr_t)&im_ctor,
				/* .tp_copy_ctor   = */ (dfunptr_t)&im_copy,
				/* .tp_deep_ctor   = */ (dfunptr_t)&im_deepcopy,
				/* .tp_any_ctor    = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(InstanceMethod),
				/* .tp_any_ctor_kw = */ (dfunptr_t)&im_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&im_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ NULL,
		/* .tp_repr      = */ NULL,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ NULL,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&im_printrepr
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&im_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&im_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &im_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ im_getsets,
	/* .tp_members       = */ im_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&im_callkw,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ im_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(im_operators)
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_INSTANCEMETHOD_C */
