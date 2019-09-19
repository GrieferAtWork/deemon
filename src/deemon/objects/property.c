/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_PROPERTY_C
#define GUARD_DEEMON_OBJECTS_PROPERTY_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/format.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/property.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/util/cache.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

typedef DeePropertyObject Property;

PRIVATE int DCALL
property_ctor(Property *__restrict self) {
	self->p_get = NULL;
	self->p_del = NULL;
	self->p_set = NULL;
	return 0;
}

PRIVATE int DCALL
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

PRIVATE int DCALL
property_deep(Property *__restrict self,
              Property *__restrict other) {
	self->p_get = NULL;
	self->p_del = NULL;
	self->p_set = NULL;
	if (other->p_get &&
	    unlikely((self->p_get = DeeObject_DeepCopy(other->p_get)) == NULL))
		goto err;
	if (other->p_del &&
	    unlikely((self->p_del = DeeObject_DeepCopy(other->p_del)) == NULL))
		goto err_get;
	if (other->p_set &&
	    unlikely((self->p_set = DeeObject_DeepCopy(other->p_set)) == NULL))
		goto err_del;
	return 0;
err_del:
	Dee_XDecref(self->p_del);
err_get:
	Dee_XDecref(self->p_get);
err:
	return -1;
}

PRIVATE int DCALL
property_init_kw(Property *__restrict self, size_t argc,
                 DeeObject **__restrict argv, DeeObject *kw) {
	PRIVATE DEFINE_KWLIST(kwlist, { K(getter), K(delete), K(setter), KEND });
	self->p_get = NULL;
	self->p_del = NULL;
	self->p_set = NULL;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "|ooo:Property",
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

PRIVATE void DCALL
property_fini(Property *__restrict self) {
	Dee_XDecref(self->p_get);
	Dee_XDecref(self->p_del);
	Dee_XDecref(self->p_set);
}

PRIVATE void DCALL
property_visit(Property *__restrict self, dvisit_t proc, void *arg) {
	Dee_XVisit(self->p_get);
	Dee_XVisit(self->p_del);
	Dee_XVisit(self->p_set);
}

PRIVATE dhash_t DCALL
property_hash(Property *__restrict self) {
	return ((self->p_get ? DeeObject_Hash(self->p_get) : 0) ^
	        (self->p_del ? DeeObject_Hash(self->p_del) : 0) ^
	        (self->p_set ? DeeObject_Hash(self->p_set) : 0));
}

PRIVATE DREF DeeObject *DCALL
property_repr(Property *__restrict self) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	if unlikely(UNICODE_PRINTER_PRINT(&printer, "Property(") < 0)
		goto err;
	if (self->p_get &&
	    unlikely(unicode_printer_printf(&printer, "getter: %r%s", self->p_get,
	                                    self->p_del || self->p_set ? ", " : "") < 0))
		goto err;
	if (self->p_del &&
	    unlikely(unicode_printer_printf(&printer, "delete: %r%s", self->p_del,
	                                    self->p_set ? ", " : "") < 0))
		goto err;
	if (self->p_set &&
	    unlikely(unicode_printer_printf(&printer, "setter: %r", self->p_set) < 0))
		goto err;
	if (unicode_printer_putascii(&printer, ')'))
		goto err;
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
property_eq(Property *__restrict self,
            Property *__restrict other) {
	int temp;
	if (DeeObject_AssertType((DeeObject *)other, &DeeProperty_Type))
		return NULL;
	if (((self->p_get != NULL) != (other->p_get != NULL)) ||
	    ((self->p_del != NULL) != (other->p_del != NULL)) ||
	    ((self->p_set != NULL) != (other->p_set != NULL)))
		return_false;
	if (self->p_get && (temp = DeeObject_CompareEq(self->p_get, other->p_get)) <= 0)
		goto handle_temp;
	if (self->p_del && (temp = DeeObject_CompareEq(self->p_del, other->p_del)) <= 0)
		goto handle_temp;
	if (self->p_set && (temp = DeeObject_CompareEq(self->p_set, other->p_set)) <= 0)
		goto handle_temp;
	return_true;
handle_temp:
	return (unlikely(temp < 0))
	       ? NULL
	       : (Dee_Incref(Dee_False), Dee_False);
}

PRIVATE struct type_cmp property_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&property_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&property_eq
};

PRIVATE struct type_member property_members[] = {
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
	                      "Alias for #getter"),
	TYPE_MEMBER_FIELD_DOC("set", STRUCT_OBJECT, offsetof(Property, p_set),
	                      "->?DCallable\n"
	                      "Alias for #setter"),
	TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
property_canget(Property *__restrict self) {
	return_bool_(self->p_get != NULL);
}

PRIVATE DREF DeeObject *DCALL
property_candel(Property *__restrict self) {
	return_bool_(self->p_del != NULL);
}

PRIVATE DREF DeeObject *DCALL
property_canset(Property *__restrict self) {
	return_bool_(self->p_set != NULL);
}

PRIVATE int DCALL
property_info(Property *__restrict self,
              struct function_info *__restrict info) {
	int result = 1;
	if (self->p_get && DeeFunction_Check(self->p_get) &&
	    (result = DeeFunction_GetInfo(self->p_get, info)) <= 0)
		goto done;
	if (self->p_del && DeeFunction_Check(self->p_del) &&
	    (result = DeeFunction_GetInfo(self->p_del, info)) <= 0)
		goto done;
	if (self->p_set && DeeFunction_Check(self->p_set) &&
	    (result = DeeFunction_GetInfo(self->p_set, info)) <= 0)
		goto done;
done:
	return result;
}

PRIVATE DREF DeeObject *DCALL
property_getattr(Property *__restrict self,
                 DeeObject *__restrict name) {
	if (self->p_get)
		return DeeObject_GetAttr(self->p_get, name);
	if (self->p_del)
		return DeeObject_GetAttr(self->p_del, name);
	if (self->p_set)
		return DeeObject_GetAttr(self->p_set, name);
	return ITER_DONE;
}

PRIVATE DREF DeeObject *DCALL
property_get_name(Property *__restrict self) {
	struct function_info info;
	int error;
	DREF DeeObject *result;
	error = property_info(self, &info);
	if unlikely(error < 0)
		goto err;
	Dee_XDecref(info.fi_doc);
	Dee_XDecref(info.fi_type);
	if (info.fi_name)
		return (DREF DeeObject *)info.fi_name;
	result = property_getattr(self, &str___name__);
	if (result != ITER_DONE)
		return result;
	return_none;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
property_get_doc(Property *__restrict self) {
	struct function_info info;
	int error;
	DREF DeeObject *result;
	error = property_info(self, &info);
	if unlikely(error < 0)
		goto err;
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_type);
	if (info.fi_doc)
		return (DREF DeeObject *)info.fi_doc;
	result = property_getattr(self, &str___doc__);
	if (result != ITER_DONE)
		return result;
	return_none;
err:
	return NULL;
}

PRIVATE DREF DeeTypeObject *DCALL
property_get_type(Property *__restrict self) {
	struct function_info info;
	int error;
	DREF DeeTypeObject *result;
	error = property_info(self, &info);
	if unlikely(error < 0)
		goto err;
	Dee_XDecref(info.fi_name);
	Dee_XDecref(info.fi_doc);
	if (info.fi_type)
		return info.fi_type;
	result = (DREF DeeTypeObject *)property_getattr(self, &str___type__);
	if (result == (DREF DeeTypeObject *)ITER_DONE) {
		result = (DREF DeeTypeObject *)Dee_None;
		Dee_Incref(result);
	}
	return result;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
get_function_module(DeeObject *__restrict self) {
	return_reference_((DeeObject *)(((DeeFunctionObject *)self)->fo_code->co_module));
}

PRIVATE DREF DeeObject *DCALL
property_get_module(Property *__restrict self) {
	DREF DeeObject *result;
	if (self->p_get && DeeFunction_Check(self->p_get))
		return get_function_module(self->p_get);
	if (self->p_del && DeeFunction_Check(self->p_del))
		return get_function_module(self->p_get);
	if (self->p_set && DeeFunction_Check(self->p_set))
		return get_function_module(self->p_get);
	result = property_getattr(self, &str___module__);
	if (result != ITER_DONE)
		return result;
	return_none;
}



PRIVATE struct type_getset property_getsets[] = {
	{ "canget",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&property_canget, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns :true if @this Property has a getter callback") },
	{ "candel",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&property_candel, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns :true if @this Property has a delete callback") },
	{ "canset",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&property_canset, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Returns :true if @this Property has a setter callback") },
	{ DeeString_STR(&str___name__),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&property_get_name, NULL, NULL,
	  DOC("->?X2?Dstring?N\n"
	      "Returns the name of @this Property, or :none if unknown") },
	{ DeeString_STR(&str___doc__),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&property_get_doc, NULL, NULL,
	  DOC("->?X2?Dstring?N\n"
	      "Returns the documentation string of @this Property, or :none if unknown") },
	{ DeeString_STR(&str___type__),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&property_get_type, NULL, NULL,
	  DOC("->?X2?Dstring?N\n"
	      "Returns the type implementing @this Property, or :none if unknown") },
	{ DeeString_STR(&str___module__),
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&property_get_module, NULL, NULL,
	  DOC("->?X2?DModule?N\n"
	      "Returns the module within which @this Property is declared, or :none if unknown") },
	{ NULL }
};

PRIVATE DREF DeeObject *DCALL
property_call(Property *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	if likely(self->p_get)
		return DeeObject_Call(self->p_get, argc, argv);
	err_unbound_attribute(&DeeProperty_Type, DeeString_STR(&str_get));
	return NULL;
}


PUBLIC DeeTypeObject DeeProperty_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Property),
	/* .tp_doc      = */ DOC("(getter:?DCallable=!N,delete:?DCallable=!N,setter:?DCallable=!N)\n"
	                        "\n"
	                        "call(args!)->\n"
	                        "Same as ${this.get(args...)}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor        = */ (void *)&property_ctor,
				/* .tp_copy_ctor   = */ (void *)&property_copy,
				/* .tp_deep_ctor   = */ (void *)&property_deep,
				/* .tp_any_ctor    = */ (void *)NULL,
				TYPE_FIXED_ALLOCATOR(Property),
				/* .tp_any_ctor_kw = */ (void *)&property_init_kw,
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&property_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&property_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **__restrict))&property_call,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&property_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &property_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ property_getsets,
	/* .tp_members       = */ property_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_PROPERTY_C */
