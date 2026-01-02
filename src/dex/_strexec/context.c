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
#ifndef GUARD_DEX_STREXEC_CONTEXT_C
#define GUARD_DEX_STREXEC_CONTEXT_C 1
#define DEE_SOURCE

#include "libjit.h"
/**/

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/class.h>
#include <deemon/dict.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/instancemethod.h>
#include <deemon/kwds.h>
#include <deemon/module.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/system-features.h> /* memcpy() */
#include <deemon/util/atomic.h>
#include <deemon/util/objectlist.h>
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

INTERN NONNULL((1)) void DFCALL
JITLValue_Fini(JITLValue *__restrict self) {
	switch (self->lv_kind) {

	case JIT_LVALUE_RANGE:
		Dee_Decref(self->lv_range.lr_end);
		ATTR_FALLTHROUGH
	case JIT_LVALUE_ATTR:
	case JIT_LVALUE_ITEM:
		Dee_Decref(self->lv_item.li_index);
		ATTR_FALLTHROUGH
	case JIT_LVALUE_EXTERN:
	case JIT_LVALUE_GLOBAL:
	case JIT_LVALUE_ATTRSTR:
	case JIT_LVALUE_RVALUE:
	case JIT_LVALUE_THIS:
	case JIT_LVALUE_CLSATTRIB:
		Dee_Decref(self->lv_global);
		break;

	default: break;
	}
}

INTERN NONNULL((1, 2)) void DFCALL
JITLValue_Visit(JITLValue *__restrict self, Dee_visit_t proc, void *arg) {
	switch (self->lv_kind) {

	case JIT_LVALUE_RANGE:
		Dee_Visit(self->lv_range.lr_end);
		ATTR_FALLTHROUGH
	case JIT_LVALUE_ATTR:
	case JIT_LVALUE_ITEM:
		Dee_Visit(self->lv_item.li_index);
		ATTR_FALLTHROUGH
	case JIT_LVALUE_EXTERN:
	case JIT_LVALUE_GLOBAL:
	case JIT_LVALUE_ATTRSTR:
	case JIT_LVALUE_RVALUE:
	case JIT_LVALUE_THIS:
	case JIT_LVALUE_CLSATTRIB:
		Dee_Visit(self->lv_global);
		break;

	default: break;
	}
}


PRIVATE WUNUSED NONNULL((1)) bool DFCALL
update_symbol_objent(JITSymbol *__restrict self) {
	struct jit_object_table *tab;
	struct jit_object_entry *ent;
	ASSERT(self->js_kind == JIT_SYMBOL_OBJENT);
	ent = self->js_objent.jo_ent;
	tab = self->js_objent.jo_tab;
	if (ent < tab->ot_list)
		goto do_reload;
	if (ent > tab->ot_list + tab->ot_mask)
		goto do_reload;
	if (ent->oe_namestr != self->js_objent.jo_namestr)
		goto do_reload;
	if (ent->oe_namelen != self->js_objent.jo_namelen)
		goto do_reload;
	return true;
	{
		Dee_hash_t i, perturb, hash;
do_reload:
		hash = Dee_HashUtf8(self->js_objent.jo_namestr,
		                    self->js_objent.jo_namelen);
		i = perturb = hash & tab->ot_mask;
		for (;; JITObjectTable_NEXT(i, perturb)) {
			ent = &tab->ot_list[i & tab->ot_mask];
			if unlikely(!ent->oe_namestr)
				goto err_unloaded;
			if (ent->oe_namestr == (char *)ITER_DONE)
				continue;
			if (ent->oe_namehsh != hash)
				continue;
			if (ent->oe_namelen != self->js_objent.jo_namelen)
				continue;
			if (ent->oe_namestr == (char *)self->js_objent.jo_namestr)
				break; /* Exact same string */
			if (bcmpc(ent->oe_namestr, self->js_objent.jo_namestr,
			          self->js_objent.jo_namelen, sizeof(char)) != 0)
				continue;
			/* Found it! */
			break;
		}
		/* Update cached values. */
		self->js_objent.jo_ent     = ent;
		self->js_objent.jo_namestr = (char const *)ent->oe_namestr;
		self->js_objent.jo_namelen = ent->oe_namelen;
	}
	return true;
err_unloaded:
	DeeError_Throwf(&DeeError_SymbolError,
	                "Unknown variable `%$s'",
	                self->js_objent.jo_namelen,
	                self->js_objent.jo_namestr);
	return false;
}

PRIVATE WUNUSED ATTR_RETNONNULL NONNULL((1, 2)) struct class_desc *DCALL
class_desc_from_instance(struct instance_desc *__restrict self,
                         DeeObject *__restrict this_arg) {
	DeeTypeObject *tp;
	if (DeeType_Check(this_arg)) {
		tp = (DeeTypeObject *)this_arg;
		ASSERT(DeeType_IsClass(tp));
		ASSERT(self == Dee_class_desc_as_instance(tp->tp_class));
		return tp->tp_class;
	}
	tp = Dee_TYPE(this_arg);
	for (;;) {
		struct class_desc *result = tp->tp_class;
		if (DeeInstance_DESC(result, this_arg) == self)
			return result;
		tp = DeeType_Base(tp);
	}
}

PRIVATE WUNUSED ATTR_RETNONNULL NONNULL((1, 2)) struct instance_desc *DCALL
class_desc_as_instance_from_instance(struct instance_desc *__restrict self,
                                     DeeObject *__restrict this_arg) {
	struct class_desc *cls;
	cls = class_desc_from_instance(self, this_arg);
	return class_desc_as_instance(cls);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
fast_DeeInstance_GetAttribute(struct instance_desc *__restrict self,
                              DeeObject *__restrict this_arg,
                              struct class_attribute *__restrict attr) {
	DREF DeeObject *result;
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance_from_instance(self, this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		Dee_instance_desc_lock_read(self);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!getter)
			goto illegal;
		/* Invoke the getter. */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
			return DeeObject_ThisCallInherited(getter, this_arg, 0, NULL);
		return DeeObject_CallInherited(getter, 0, NULL);
	} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
		/* Construct a thiscall function. */
		DREF DeeObject *callback;
		Dee_instance_desc_lock_read(self);
		callback = self->id_vtab[attr->ca_addr];
		Dee_XIncref(callback);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!callback)
			goto unbound;
		Dee_Incref(this_arg);
		return DeeInstanceMethod_NewInherited(callback, this_arg);
	} else {
		/* Simply return the attribute as-is. */
		Dee_instance_desc_lock_read(self);
		result = self->id_vtab[attr->ca_addr];
		Dee_XIncref(result);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!result)
			goto unbound;
	}
	return result;
unbound:
	return DeeRT_ErrCUnboundAttrCA(this_arg, attr);
illegal:
	DeeRT_ErrCRestrictedAttrCA(this_arg, attr, DeeRT_ATTRIBUTE_ACCESS_GET);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
fast_DeeInstance_BoundAttribute_asbool(struct instance_desc *__restrict self,
                                DeeObject *__restrict this_arg,
                                struct class_attribute *__restrict attr) {
	DREF DeeObject *result;
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance_from_instance(self, this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		Dee_instance_desc_lock_read(self);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!getter)
			goto unbound;

		/* Invoke the getter. */
		result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		         ? DeeObject_ThisCallInherited(getter, this_arg, 0, NULL)
		         : DeeObject_CallInherited(getter, 0, NULL);
		if likely(result) {
			Dee_Decref(result);
			return 1; /* Is bound */
		}
		if (DeeError_Catch(&DeeError_UnboundAttribute))
			goto unbound;
		return -1; /* err */
	} else {
		/* Simply return the attribute as-is. */
		return atomic_read(&self->id_vtab[attr->ca_addr]) != NULL;
	}
unbound:
	return 0; /* Is not bound */
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
fast_DeeInstance_DelAttribute(struct instance_desc *__restrict self,
                              DeeObject *__restrict this_arg,
                              struct class_attribute *__restrict attr) {
	ASSERT_OBJECT(this_arg);
	/* Make sure that the access is allowed. */
	if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
		goto illegal;
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance_from_instance(self, this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *delfun, *temp;
		Dee_instance_desc_lock_read(self);
		delfun = self->id_vtab[attr->ca_addr + CLASS_GETSET_DEL];
		Dee_XIncref(delfun);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!delfun)
			goto illegal;

		/* Invoke the getter. */
		temp = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		       ? DeeObject_ThisCallInherited(delfun, this_arg, 0, NULL)
		       : DeeObject_CallInherited(delfun, 0, NULL);
		if unlikely(!temp)
			goto err;
		Dee_Decref_unlikely(temp); /* *_unlikely because it's probably "none" */
	} else {
		DREF DeeObject *old_value;

		/* Simply unbind the field in the attr table. */
		Dee_instance_desc_lock_write(self);
		old_value = self->id_vtab[attr->ca_addr];
		self->id_vtab[attr->ca_addr] = NULL;
		Dee_instance_desc_lock_endwrite(self);
		Dee_XDecref(old_value);
	}
	return 0;
illegal:
	return DeeRT_ErrCRestrictedAttrCA(this_arg, attr, DeeRT_ATTRIBUTE_ACCESS_DEL);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
fast_DeeInstance_SetAttribute(struct instance_desc *__restrict self,
                              DeeObject *this_arg,
                              struct class_attribute *__restrict attr,
                              DeeObject *value) {
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance_from_instance(self, this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *setter, *temp;
		/* Make sure that the access is allowed. */
		if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
			goto illegal;
		Dee_instance_desc_lock_read(self);
		setter = self->id_vtab[attr->ca_addr + CLASS_GETSET_SET];
		Dee_XIncref(setter);
		Dee_instance_desc_lock_endread(self);
		if unlikely(!setter)
			goto illegal;
		/* Invoke the getter. */
		temp = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		       ? DeeObject_ThisCallInherited(setter, this_arg, 1, (DeeObject **)&value)
		       : DeeObject_CallInherited(setter, 1, (DeeObject **)&value);
		if unlikely(!temp)
			goto err;
		Dee_Decref_unlikely(temp); /* *_unlikely because it's probably "none" */
	} else {
		DREF DeeObject *old_value;
		/* Simply override the field in the attr table. */
		Dee_instance_desc_lock_write(self);
		old_value = self->id_vtab[attr->ca_addr];
		if (old_value && (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
			Dee_instance_desc_lock_endwrite(self);
			goto illegal; /* readonly fields can only be set once. */
		} else {
			Dee_Incref(value);
			self->id_vtab[attr->ca_addr] = value;
		}
		Dee_instance_desc_lock_endwrite(self);

		/* Drop a reference from the old value. */
		Dee_XDecref(old_value);
	}
	return 0;
illegal:
	return DeeRT_ErrCRestrictedAttrCA(this_arg, attr, DeeRT_ATTRIBUTE_ACCESS_SET);
err:
	return -1;
}



INTERN WUNUSED NONNULL((1, 2)) int DFCALL
JITLValue_IsBound(JITLValue *__restrict self,
                  JITContext *__restrict context) {
	int result;
	ASSERT(self->lv_kind != JIT_LVALUE_NONE);
	switch (self->lv_kind) {

	case JIT_LVALUE_POINTER:
		return *self->lv_ptr != NULL;

	case JIT_LVALUE_OBJENT:
		if unlikely(!update_symbol_objent((JITSymbol *)self))
			goto err;
		result = self->lv_objent.lo_ent->oe_value != NULL;
		break;

	case JIT_LVALUE_EXTERN:
		result = DeeModule_BoundAttrSymbol(self->lv_extern.lx_mod,
		                                   self->lv_extern.lx_sym);
		if unlikely(Dee_BOUND_ISERR(result))
			goto err;
		result = Dee_BOUND_ISBOUND(result);
		break;

	case JIT_LVALUE_GLOBAL:
		if unlikely(!context->jc_globals)
			return 0;
		result = DeeObject_BoundItem(context->jc_globals,
		                             Dee_AsObject(self->lv_global));
		if unlikely(Dee_BOUND_ISERR(result))
			goto err;
		result = Dee_BOUND_ISBOUND(result);
		break;

	case JIT_LVALUE_GLOBALSTR:
		if unlikely(!context->jc_globals)
			return 0;
		result = DeeObject_BoundItemStringLenHash(context->jc_globals,
		                                          self->lv_globalstr.lg_namestr,
		                                          self->lv_globalstr.lg_namelen,
		                                          self->lv_globalstr.lg_namehsh);
		if unlikely(Dee_BOUND_ISERR(result))
			goto err;
		result = Dee_BOUND_ISBOUND(result);
		break;

	case JIT_LVALUE_CLSATTRIB:
		result = fast_DeeInstance_BoundAttribute_asbool(self->lv_clsattrib.lc_desc,
		                                                self->lv_clsattrib.lc_obj,
		                                                self->lv_clsattrib.lc_attr);
		break;

	case JIT_LVALUE_ATTR:
		result = DeeObject_BoundAttr(self->lv_attr.la_base,
		                             Dee_AsObject(self->lv_attr.la_name));
		if unlikely(Dee_BOUND_ISERR(result))
			goto err;
		result = Dee_BOUND_ISBOUND(result);
		break;

	case JIT_LVALUE_ATTRSTR:
		result = DeeObject_BoundAttrStringLenHash(self->lv_attrstr.la_base,
		                                          self->lv_attrstr.la_name,
		                                          self->lv_attrstr.la_size,
		                                          self->lv_attrstr.la_hash);
		if unlikely(Dee_BOUND_ISERR(result))
			goto err;
		result = Dee_BOUND_ISBOUND(result);
		break;

	case JIT_LVALUE_ITEM:
		result = DeeObject_BoundItem(self->lv_item.li_base,
		                             self->lv_item.li_index);
		if unlikely(Dee_BOUND_ISERR(result))
			goto err;
		result = Dee_BOUND_ISBOUND(result);
		break;

	case JIT_LVALUE_RANGE:
		context->jc_flags |= JITCONTEXT_FSYNERR;
		return DeeError_Throwf(&DeeError_SyntaxError,
		                       "Cannot test binding of expression. "
		                       "Expected a symbol, or attribute expression");

	case JIT_LVALUE_RVALUE:
	case JIT_LVALUE_THIS:
		result = 1;
		break;

	default: __builtin_unreachable();
	}
	return result;
err:
	return -1;
}




INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DFCALL
JITLValue_GetValue(JITLValue *__restrict self,
                   JITContext *__restrict context) {
	DREF DeeObject *result;
	ASSERT(self->lv_kind != JIT_LVALUE_NONE);
	switch (self->lv_kind) {

	case JIT_LVALUE_POINTER:
		result = *self->lv_ptr;
		if unlikely(!result) {
err_unbound:
			DeeError_Throwf(&DeeError_UnboundLocal,
			                "Unbound local variable");
			goto err;
		}
		Dee_Incref(result);
		break;

	case JIT_LVALUE_OBJENT:
		if unlikely(!update_symbol_objent((JITSymbol *)self))
			goto err;
		result = self->lv_objent.lo_ent->oe_value;
		if unlikely(!result)
			goto err_unbound;
		Dee_Incref(result);
		break;

	case JIT_LVALUE_EXTERN:
		result = DeeModule_GetAttrSymbol(self->lv_extern.lx_mod,
		                                 self->lv_extern.lx_sym);
		break;

	case JIT_LVALUE_GLOBAL:
		if unlikely(!context->jc_globals) {
			err_unknown_global(Dee_AsObject(self->lv_global));
			goto err;
		}
		result = DeeObject_GetItem(context->jc_globals,
		                           Dee_AsObject(self->lv_global));
		break;

	case JIT_LVALUE_GLOBALSTR:
		if unlikely(!context->jc_globals) {
			err_unknown_global_str_len(self->lv_globalstr.lg_namestr,
			                           self->lv_globalstr.lg_namelen);
			goto err;
		}
		result = DeeObject_GetItemStringLenHash(context->jc_globals,
		                                        self->lv_globalstr.lg_namestr,
		                                        self->lv_globalstr.lg_namelen,
		                                        self->lv_globalstr.lg_namehsh);
		break;

	case JIT_LVALUE_CLSATTRIB:
		result = fast_DeeInstance_GetAttribute(self->lv_clsattrib.lc_desc,
		                                       self->lv_clsattrib.lc_obj,
		                                       self->lv_clsattrib.lc_attr);
		break;

	case JIT_LVALUE_ATTR:
		result = DeeObject_GetAttr(self->lv_attr.la_base,
		                           Dee_AsObject(self->lv_attr.la_name));
		break;

	case JIT_LVALUE_ATTRSTR:
		result = DeeObject_GetAttrStringLenHash(self->lv_attr.la_base,
		                                        self->lv_attrstr.la_name,
		                                        self->lv_attrstr.la_size,
		                                        self->lv_attrstr.la_hash);
		break;

	case JIT_LVALUE_ITEM:
		result = DeeObject_GetItem(self->lv_item.li_base,
		                           self->lv_item.li_index);
		break;
	case JIT_LVALUE_RANGE:
		result = DeeObject_GetRange(self->lv_range.lr_base,
		                            self->lv_range.lr_start,
		                            self->lv_range.lr_end);
		break;

	case JIT_LVALUE_RVALUE:
	case JIT_LVALUE_THIS:
		result = self->lv_rvalue;
		Dee_Incref(result);
		break;

	default: __builtin_unreachable();
	}
	return result;
err:
	return NULL;
}




INTERN WUNUSED NONNULL((1, 2)) int DFCALL
JITLValue_DelValue(JITLValue *__restrict self,
                   JITContext *__restrict context) {
	int result;
	ASSERT(self->lv_kind != JIT_LVALUE_NONE);
	switch (self->lv_kind) {

	case JIT_LVALUE_POINTER: {
		DREF DeeObject *old_value;
		old_value     = *self->lv_ptr;
		*self->lv_ptr = NULL;
		Dee_XDecref(old_value);
		result = 0;
	}	break;

	case JIT_LVALUE_OBJENT: {
		struct jit_object_entry *ent;
		DREF DeeObject *old_value;
		if unlikely(!update_symbol_objent((JITSymbol *)self))
			goto err;
		ent           = self->lv_objent.lo_ent;
		old_value     = ent->oe_value;
		ent->oe_value = NULL;
		Dee_XDecref(old_value);
		result = 0;
	}	break;

	case JIT_LVALUE_EXTERN:
		result = DeeModule_DelAttrSymbol(self->lv_extern.lx_mod,
		                                 self->lv_extern.lx_sym);
		break;

	case JIT_LVALUE_GLOBAL:
		if unlikely(!context->jc_globals)
			return err_unknown_global(Dee_AsObject(self->lv_global));
		result = DeeObject_DelItem(context->jc_globals,
		                           Dee_AsObject(self->lv_global));
		break;

	case JIT_LVALUE_GLOBALSTR:
		if unlikely(!context->jc_globals) {
			return err_unknown_global_str_len(self->lv_globalstr.lg_namestr,
			                                  self->lv_globalstr.lg_namelen);
		}
		result = DeeObject_DelItemStringLenHash(context->jc_globals,
		                                        self->lv_globalstr.lg_namestr,
		                                        self->lv_globalstr.lg_namelen,
		                                        self->lv_globalstr.lg_namehsh);
		break;

	case JIT_LVALUE_CLSATTRIB:
		result = fast_DeeInstance_DelAttribute(self->lv_clsattrib.lc_desc,
		                                       self->lv_clsattrib.lc_obj,
		                                       self->lv_clsattrib.lc_attr);
		break;

	case JIT_LVALUE_ATTR:
		result = DeeObject_DelAttr(self->lv_attr.la_base,
		                           Dee_AsObject(self->lv_attr.la_name));
		break;

	case JIT_LVALUE_ATTRSTR:
		result = DeeObject_DelAttrStringLenHash(self->lv_attrstr.la_base,
		                                        self->lv_attrstr.la_name,
		                                        self->lv_attrstr.la_size,
		                                        self->lv_attrstr.la_hash);
		break;

	case JIT_LVALUE_ITEM:
		result = DeeObject_DelItem(self->lv_item.li_base,
		                           self->lv_item.li_index);
		break;
	case JIT_LVALUE_RANGE:
		result = DeeObject_DelRange(self->lv_range.lr_base,
		                            self->lv_range.lr_start,
		                            self->lv_range.lr_end);
		break;

	case JIT_LVALUE_RVALUE:
	case JIT_LVALUE_THIS:
		result = DeeError_Throwf(&DeeError_SyntaxError,
		                         "Cannot delete R-value");
		break;

	default: __builtin_unreachable();
	}
	return result;
err:
	return -1;
}




INTERN WUNUSED NONNULL((1, 2, 3)) int DFCALL
JITLValue_SetValue(JITLValue *__restrict self,
                   JITContext *__restrict context,
                   DeeObject *value) {
	int result;
	ASSERT(self->lv_kind != JIT_LVALUE_NONE);
	switch (self->lv_kind) {

	case JIT_LVALUE_POINTER: {
		DREF DeeObject *old_value;
		old_value     = *self->lv_ptr;
		*self->lv_ptr = value;
		Dee_Incref(value);
		Dee_XDecref(old_value);
		result = 0;
	}	break;

	case JIT_LVALUE_OBJENT: {
		struct jit_object_entry *ent;
		DREF DeeObject *old_value;
		if unlikely(!update_symbol_objent((JITSymbol *)self))
			goto err;
		Dee_Incref(value);
		ent           = self->lv_objent.lo_ent;
		old_value     = ent->oe_value;
		ent->oe_value = value;
		Dee_XDecref(old_value);
		result = 0;
	}	break;

	case JIT_LVALUE_EXTERN:
		result = DeeModule_SetAttrSymbol(self->lv_extern.lx_mod,
		                                 self->lv_extern.lx_sym,
		                                 value);
		break;

	case JIT_LVALUE_GLOBAL:
		if (!context->jc_globals) {
			context->jc_globals = DeeDict_New();
			if unlikely(!context->jc_globals)
				goto err;
		}
		result = DeeObject_SetItem(context->jc_globals,
		                           Dee_AsObject(self->lv_global),
		                           value);
		break;

	case JIT_LVALUE_GLOBALSTR:
		if (!context->jc_globals) {
			context->jc_globals = DeeDict_New();
			if unlikely(!context->jc_globals)
				goto err;
		}
		result = DeeObject_SetItemStringLenHash(context->jc_globals,
		                                        self->lv_globalstr.lg_namestr,
		                                        self->lv_globalstr.lg_namelen,
		                                        self->lv_globalstr.lg_namehsh,
		                                        value);
		break;

	case JIT_LVALUE_CLSATTRIB:
		result = fast_DeeInstance_SetAttribute(self->lv_clsattrib.lc_desc,
		                                       self->lv_clsattrib.lc_obj,
		                                       self->lv_clsattrib.lc_attr,
		                                       value);
		break;

	case JIT_LVALUE_ATTR:
		result = DeeObject_SetAttr(self->lv_attr.la_base,
		                           Dee_AsObject(self->lv_attr.la_name),
		                           value);
		break;

	case JIT_LVALUE_ATTRSTR:
		result = DeeObject_SetAttrStringLenHash(self->lv_attrstr.la_base,
		                                        self->lv_attrstr.la_name,
		                                        self->lv_attrstr.la_size,
		                                        self->lv_attrstr.la_hash,
		                                        value);
		break;

	case JIT_LVALUE_ITEM:
		result = DeeObject_SetItem(self->lv_item.li_base,
		                           self->lv_item.li_index,
		                           value);
		break;

	case JIT_LVALUE_RANGE:
		result = DeeObject_SetRange(self->lv_range.lr_base,
		                            self->lv_range.lr_start,
		                            self->lv_range.lr_end,
		                            value);
		break;

	case JIT_LVALUE_RVALUE:
	case JIT_LVALUE_THIS:
		result = DeeError_Throwf(&DeeError_SyntaxError,
		                         "Cannot store to R-value");
		break;

	default: __builtin_unreachable();
	}
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DFCALL
JITLValue_CallValue(JITLValue *__restrict self, JITContext *__restrict context,
                    DeeObject *args, DeeObject *kw) {
	DREF DeeObject *result;
	ASSERT(self->lv_kind != JIT_LVALUE_NONE);
	if (kw && !DeeObject_IsKw(kw)) {
		kw = DeeKw_ForceWrap(kw);
		if unlikely(!kw)
			goto err;
		result = JITLValue_CallValue(self, context, args, kw);
		Dee_Decref(kw);
		return result;
	}
	switch (self->lv_kind) {

	case JIT_LVALUE_ATTR:
		result = DeeObject_CallAttrTupleKw(self->lv_attr.la_base,
		                                   Dee_AsObject(self->lv_attr.la_name),
		                                   args, kw);
		break;

	case JIT_LVALUE_ATTRSTR:
		result = DeeObject_CallAttrStringLenHashTupleKw(self->lv_attr.la_base,
		                                                self->lv_attrstr.la_name,
		                                                self->lv_attrstr.la_size,
		                                                self->lv_attrstr.la_hash,
		                                                args, kw);
		break;

	default: {
		DREF DeeObject *function;
		/* Fallback: get the value object, then call it. */
		function = JITLValue_GetValue(self, context);
		if unlikely(!function)
			goto err;
		result = DeeObject_CallTupleKw(function, args, kw);
		Dee_Decref(function);
	}	break;

	}
	return result;
err:
	return NULL;
}



/* Finalize the given L-value list. */
INTERN NONNULL((1)) void DCALL
JITLValueList_Fini(JITLValueList *__restrict self) {
	size_t i;
	for (i = 0; i < self->ll_size; ++i)
		JITLValue_Fini(&self->ll_list[i]);
	Dee_Free(self->ll_list);
}

/* Append the given @value onto @self, returning -1 on error and 0 on success. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
JITLValueList_Append(JITLValueList *__restrict self,
                     /*inherit(on_success)*/ JITLValue *__restrict value) {
	ASSERT(self->ll_size <= self->ll_alloc);
	if (self->ll_size >= self->ll_alloc) {
		JITLValue *new_list;
		size_t new_alloc = self->ll_alloc * 2;
		if (!new_alloc)
			new_alloc = 1;
		ASSERT(new_alloc > self->ll_size);
		new_list = (JITLValue *)Dee_TryReallocc(self->ll_list, new_alloc, sizeof(JITLValue));
		if unlikely(!new_list) {
			new_alloc = self->ll_size + 1;
			new_list = (JITLValue *)Dee_Reallocc(self->ll_list, new_alloc, sizeof(JITLValue));
			if unlikely(!new_list)
				goto err;
		}
		self->ll_list  = new_list;
		self->ll_alloc = new_alloc;
	}
	/* Inherit all of the given data. */
	memcpy(&self->ll_list[self->ll_size],
	       value, sizeof(JITLValue));
	++self->ll_size;
	return 0;
err:
	return -1;
}

/* Append an R-value expression. */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
JITLValueList_AppendRValue(JITLValueList *__restrict self,
                           DeeObject *__restrict value) {
	ASSERT(self->ll_size <= self->ll_alloc);
	if (self->ll_size >= self->ll_alloc) {
		JITLValue *new_list;
		size_t new_alloc = self->ll_alloc * 2;
		if (!new_alloc)
			new_alloc = 1;
		ASSERT(new_alloc > self->ll_size);
		new_list = (JITLValue *)Dee_TryReallocc(self->ll_list, new_alloc, sizeof(JITLValue));
		if unlikely(!new_list) {
			new_alloc = self->ll_size + 1;
			new_list = (JITLValue *)Dee_Reallocc(self->ll_list, new_alloc, sizeof(JITLValue));
			if unlikely(!new_list)
				goto err;
		}
		self->ll_list  = new_list;
		self->ll_alloc = new_alloc;
	}
	/* Initialize an R-value descriptor. */
	self->ll_list[self->ll_size].lv_kind   = JIT_LVALUE_RVALUE;
	self->ll_list[self->ll_size].lv_rvalue = value;
	Dee_Incref(value);
	++self->ll_size;
	return 0;
err:
	return -1;
}


/* Pack `self' and append all of the referenced objects to the given object list. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
JITLValueList_CopyObjects(JITLValueList *__restrict self,
                          struct objectlist *__restrict dst,
                          JITContext *__restrict context) {
	size_t i;
	DREF DeeObject **buf;
	if (!self->ll_size)
		goto done;
	buf = objectlist_alloc(dst, self->ll_size);
	if unlikely(!buf)
		goto err;
	for (i = 0; i < self->ll_size; ++i) {
		DREF DeeObject *ob;
		ob = JITLValue_GetValue(&self->ll_list[i], context);
		if unlikely(!ob)
			goto err_i;
		buf[i] = ob; /* Inherit reference. */
	}
done:
	return 0;
err_i:
	dst->ol_elemc -= (self->ll_size - i);
err:
	return -1;
}

/* Unpack `values' and assign each of the unpacked values to
 * the proper LValue of at the same position within `self'
 * @return:  0: Success.
 * @return: -1: An error occurred. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
JITLValueList_UnpackAssign(JITLValueList *__restrict self,
                           JITContext *__restrict context,
                           DeeObject *__restrict values) {
	size_t i, count = self->ll_size;
	DREF DeeObject **unpacked_values;
	unpacked_values = (DREF DeeObject **)Dee_Mallocac(count, sizeof(DREF DeeObject *));
	if unlikely(!unpacked_values)
		goto err;
	if unlikely(DeeSeq_Unpack(values, count, unpacked_values))
		goto err_unpacked_values;
	for (i = 0; i < count; ++i) {
		if unlikely(JITLValue_SetValue(&self->ll_list[i], context, unpacked_values[i]))
			goto err;
	}
	Dee_Decrefv(unpacked_values, count);
	Dee_Freea(unpacked_values);
	return 0;
err_unpacked_values:
	Dee_Freea(unpacked_values);
err:
	return -1;
}





/* Similar to `JITLexer_GetLValue()', but also finalize
 * the stored L-value, and set it to describe nothing.
 * NOTE: The stored L-value is _always_ reset! */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
JITLexer_PackLValue(JITLexer *__restrict self) {
	DREF DeeObject *result;
	result = JITLValue_GetValue(&self->jl_lvalue,
	                            self->jl_context);
	JITLValue_Fini(&self->jl_lvalue);
	self->jl_lvalue.lv_kind = JIT_LVALUE_NONE;
	return result;
}



INTERN NONNULL((1)) void DCALL
_JITContext_PopLocals(JITContext *__restrict self) {
	JITObjectTable *tab;
	ASSERT(self->jc_locals.otp_ind == 0);
	tab = self->jc_locals.otp_tab;
	if (!tab)
		return;
	self->jc_locals = tab->ot_prev;
	JITObjectTable_Fini(tab);
	JITObjectTable_Free(tab);
}


/* Get a pointer to the first locals object-table for the current scope,
 * either for reading (in which case `NULL' is indicative of an empty scope),
 * or for writing (in which case `NULL' indicates an error) */
INTERN WUNUSED NONNULL((1)) JITObjectTable *DCALL
JITContext_GetRWLocals(JITContext *__restrict self) {
	JITObjectTable *result;
	/*ASSERT(self->jc_locals.otp_ind >= 1);*/
	result = self->jc_locals.otp_tab;
	if (result && self->jc_locals.otp_ind == 1)
		return result;
	/* Must create a new table for our own usage. */
	result = JITObjectTable_Alloc();
	if unlikely(!result)
		goto done;
	JITObjectTable_Init(result);
	--self->jc_locals.otp_ind;
	result->ot_prev         = self->jc_locals;
	self->jc_locals.otp_tab = result;
	self->jc_locals.otp_ind = 1;
done:
	return result;
}



/* Lookup a given symbol within a specific JIT context
 * @param: mode: Set of `LOOKUP_SYM_*'
 * @return: 0:  The specified symbol was found, and `result' was filled
 * @return: -1: An error occurred. */
INTERN WUNUSED ATTR_INS(3, 4) NONNULL((1, 2)) int DFCALL
JITContext_Lookup(JITContext *__restrict self,
                  JITSymbol *__restrict result,
                  /*utf-8*/ char const *name,
                  size_t namelen, unsigned int mode) {
	DREF DeeObject *star_import;
	struct Dee_module_symbol *star_import_symbol;
	JITObjectTable *tab;
	struct jit_object_entry *ent;
	Dee_hash_t hash = Dee_HashUtf8(name, namelen);
	switch (mode & LOOKUP_SYM_VMASK) {

	case LOOKUP_SYM_VLOCAL:
		/* Search for a local symbol. */
		tab = JITContext_GetROLocals(self);
		if (!tab)
			break; /* No locals */
		ent = JITObjectTable_Lookup(tab, name, namelen, hash);
		if (!ent) {
			/* Check for a *-import */
			star_import = JITObjectTable_FindImportStar(tab, name, namelen, hash,
			                                            &star_import_symbol);
			if (star_import == ITER_DONE)
				break;
set_star_import:
			if unlikely(!star_import)
				goto err;
			if (star_import_symbol) {
				ASSERT(DeeModule_Check(star_import));
				result->js_kind = JIT_SYMBOL_EXTERN;
				result->js_extern.jx_mod = (DREF DeeModuleObject *)star_import; /* Inherit reference */
				result->js_extern.jx_sym = star_import_symbol;
			} else {
				result->js_kind = JIT_SYMBOL_ATTRSTR;
				result->js_attrstr.ja_base = star_import; /* Inherit reference */
				result->js_attrstr.ja_name = name;
				result->js_attrstr.ja_size = namelen;
				result->js_attrstr.ja_hash = hash;
			}
			goto done;
		}

		/* Found a local variable entry. */
set_object_entry:
		switch (ent->oe_type) {

		case JIT_OBJECT_ENTRY_TYPE_LOCAL:
			result->js_kind = JIT_SYMBOL_OBJENT;
			result->js_objent.jo_tab     = tab;
			result->js_objent.jo_ent     = ent;
			result->js_objent.jo_namestr = ent->oe_namestr;
			result->js_objent.jo_namelen = ent->oe_namelen;
			break;

		case JIT_OBJECT_ENTRY_TYPE_ATTR_FIXED:
			result->js_kind = JIT_SYMBOL_CLSATTRIB;
			result->js_clsattrib.jc_obj  = ent->oe_attr_fixed.af_obj;
			result->js_clsattrib.jc_attr = ent->oe_attr_fixed.af_attr;
			result->js_clsattrib.jc_desc = ent->oe_attr_fixed.af_desc;
			Dee_Incref(result->js_clsattrib.jc_obj);
			break;

		case JIT_OBJECT_ENTRY_TYPE_ATTR: {
			JITLValue lv;
			/* XXX: Cache the this-argument? */
			if unlikely(namelen == COMPILER_STRLEN(JIT_RTSYM_THIS) &&
			            bcmpc(name, JIT_RTSYM_THIS,
			                  COMPILER_STRLEN(JIT_RTSYM_THIS),
			                  sizeof(char)) == 0)
				goto err_unknown_var;
			if (JITContext_Lookup(self, (JITSymbol *)&lv, JIT_RTSYM_THIS,
			                      COMPILER_STRLEN(JIT_RTSYM_THIS), LOOKUP_SYM_NORMAL))
				goto err;
			result->js_clsattrib.jc_obj = JITLValue_GetValue(&lv, self);
			JITLValue_Fini(&lv);
			if unlikely(!result->js_clsattrib.jc_obj)
				goto err;
			if (DeeObject_AssertTypeOrAbstract(result->js_clsattrib.jc_obj,
			                                   ent->oe_attr.a_class)) {
				Dee_Decref(result->js_clsattrib.jc_obj);
				goto err;
			}
			result->js_kind = JIT_SYMBOL_CLSATTRIB;
			result->js_clsattrib.jc_attr = ent->oe_attr.a_attr;
			result->js_clsattrib.jc_desc = DeeInstance_DESC(ent->oe_attr.a_class->tp_class,
			                                                result->js_clsattrib.jc_obj);
		}	break;

		case JIT_OBJECT_ENTRY_EXTERN_SYMBOL:
			result->js_kind = JIT_SYMBOL_EXTERN;
			result->js_extern.jx_mod = ent->oe_extern_symbol.es_mod;
			result->js_extern.jx_sym = ent->oe_extern_symbol.es_sym;
			Dee_Incref(result->js_extern.jx_mod);
			break;

		case JIT_OBJECT_ENTRY_EXTERN_ATTR:
			result->js_kind = JIT_SYMBOL_ATTR;
			result->js_attr.ja_base = ent->oe_extern_attr.ea_base;
			result->js_attr.ja_name = ent->oe_extern_attr.ea_name;
			Dee_Incref(result->js_attr.ja_base);
			Dee_Incref(result->js_attr.ja_name);
			break;

		case JIT_OBJECT_ENTRY_EXTERN_ATTRSTR:
			result->js_kind = JIT_SYMBOL_ATTRSTR;
			result->js_attrstr.ja_base = ent->oe_extern_attrstr.eas_base;
			result->js_attrstr.ja_name = ent->oe_extern_attrstr.eas_name;
			result->js_attrstr.ja_size = ent->oe_extern_attrstr.eas_size;
			result->js_attrstr.ja_hash = ent->oe_extern_attrstr.eas_hash;
			Dee_Incref(result->js_attrstr.ja_base);
			break;

		default:
			__builtin_unreachable();
		}
done:
		return 0;

	case LOOKUP_SYM_VGLOBAL:
set_global:
#if 0
		if (!self->jc_globals) {
			self->jc_globals = DeeDict_New();
			if unlikely(!self->jc_globals)
				goto err;
		}
#endif
		result->js_kind = JIT_SYMBOL_GLOBALSTR;
		result->js_globalstr.jg_namestr = name;
		result->js_globalstr.jg_namelen = namelen;
		result->js_globalstr.jg_namehsh = hash;
		goto done;

	default:
		tab = JITContext_GetROLocals(self);
		for (; tab; tab = tab->ot_prev.otp_tab) {
			ent = JITObjectTable_Lookup(tab, name, namelen, hash);
			if (ent)
				goto set_object_entry;
			star_import = JITObjectTable_FindImportStar(tab, name, namelen, hash,
			                                            &star_import_symbol);
			if (star_import != ITER_DONE)
				goto set_star_import;
		}

		/* If we're not allowed to declare things, always
		 * assume that this is referring to a global */
		if (!(mode & LOOKUP_SYM_ALLOWDECL))
			goto set_global;

		/* While inside of the global scope, untyped
		 * symbols are always declared as global! */
		if (JITContext_IsGlobalScope(self))
			goto set_global;

		/* Check if the symbol exists within the global symbol table. */
		if (self->jc_globals) {
			int error;
			error = DeeObject_HasItemStringLenHash(self->jc_globals,
			                                   name, namelen, hash);
			if unlikely(Dee_HAS_ISERR(error))
				goto err;
			if (error)
				goto set_global; /* Known global */
		}
		break;
	}
	if unlikely(!(mode & LOOKUP_SYM_ALLOWDECL))
		goto err_unknown_var;

	/* Create a new local variable. */
	tab = JITContext_GetRWLocals(self);
	if unlikely(!tab)
		goto err;
	ent = JITObjectTable_Create(tab, name, namelen, hash);
	if unlikely(!ent)
		goto err;
	goto set_object_entry;
err_unknown_var:
	DeeError_Throwf(&DeeError_SymbolError,
	                "Unknown variable `%$s'",
	                namelen, name);
err:
	return -1;
}

INTERN WUNUSED ATTR_INS(3, 4) NONNULL((1, 2)) int DFCALL
JITContext_LookupNth(JITContext *__restrict self,
                     JITSymbol *__restrict result,
                     /*utf-8*/ char const *name,
                     size_t namelen, size_t nth) {
	(void)self;
	(void)result;
	(void)name;
	(void)namelen;
	(void)nth;
	/* TODO */
	return DeeError_Throwf(&DeeError_SymbolError,
	                       "Unknown variable `%$s'",
	                       namelen, name);
}




/* Return a reference to an object that implements attribute
 * operators such that it allows access to JIT globals. */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
JITContext_GetCurrentModule(JITContext *__restrict self) {
	DREF DeeObject *result;
	if (self->jc_globals == NULL) {
		self->jc_globals = DeeDict_New();
		if unlikely(!self->jc_globals)
			goto err;
	}

	/* FIXME: The current-module object isn't supposed to change id:
	 * >> import . as a;
	 * >> import . as b;
	 * >> assert a == b;  // OK
	 * >> assert a === b; // FAIL  (because it's actually 2 different objects)
	 */
	result = self->jc_globals;
	result = DeeObject_GetAttrString(result, "byattr");
	return result;
err:
	return NULL;
}



/* Import a named module and bind it as a local variable. */
INTERN WUNUSED NONNULL((1, 2)) int DFCALL
JITContext_DoImportModule(JITContext *__restrict self,
                          struct jit_import_item const *__restrict spec) {
	JITObjectTable *tab;
	struct jit_object_entry *ent;
	DREF DeeObject *source_module;
	char const *source_name;
	size_t source_size;
	if (spec->ii_import_name) {
		source_name = DeeString_AsUtf8((DeeObject *)spec->ii_import_name);
		if unlikely(!source_name)
			goto err;
		source_size = WSTR_LENGTH(source_name);
	} else {
		source_name = spec->ii_symbol_name;
		source_size = spec->ii_symbol_size;
	}

	if (*source_name == '.') {
		/* Special case: module-relative import */
		if (source_size == 1) {
			/* Special case: current module */
			source_module = JITContext_GetCurrentModule(self);
		} else if (!self->jc_impbase) {
			err_cannot_import_relative(source_name, source_size);
			goto err;
		} else if (self->jc_import) {
			/* Special case: invoke the user-defined import hook */
			DeeObject *args[2];
			args[0] = (DeeObject *)spec->ii_import_name;
			args[1] = Dee_AsObject(self->jc_impbase);
			if (args[0]) {
				source_module = DeeObject_Call(self->jc_import, 2, args);
			} else {
				args[0] = DeeString_NewUtf8(source_name, source_size, STRING_ERROR_FSTRICT);
				if unlikely(!args[0])
					goto err;
				source_module = DeeObject_Call(self->jc_import, 2, args);
				Dee_Decref(args[0]);
			}
		} else {
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
			goto do_import_module;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
			source_module = Dee_AsObject(DeeModule_ImportRelString(self->jc_impbase,
			                                                       source_name,
			                                                       source_size));
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		}
	} else if (self->jc_import) {
		/* Absolute module import with user-defined import hook. */
		if (spec->ii_import_name) {
			source_module = DeeObject_Call(self->jc_import, 1, (DeeObject **)&spec->ii_import_name);
		} else {
			DREF DeeStringObject *import_name;
			import_name = (DREF DeeStringObject *)DeeString_NewUtf8(source_name, source_size, STRING_ERROR_FSTRICT);
			if unlikely(!import_name)
				goto err;
			source_module = DeeObject_Call(self->jc_import, 1, (DeeObject **)&import_name);
			Dee_Decref(import_name);
		}
	} else {
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
do_import_module:
		source_module = Dee_AsObject(DeeModule_ImportString(source_name, source_size,
		                                                    Dee_AsObject(self->jc_impbase),
		                                                    DeeModule_IMPORT_F_NORMAL));
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		source_module = Dee_AsObject(DeeModule_ImportGlobalString(source_name, source_size));
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	}
	if unlikely(!source_module)
		goto err;

	/* Load the local symbol table for writing. */
	tab = JITContext_GetRWLocals(self);
	if unlikely(!tab)
		goto err_source_module;
	ent = JITObjectTable_Create(tab, spec->ii_symbol_name, spec->ii_symbol_size,
	                            Dee_HashUtf8(spec->ii_symbol_name, spec->ii_symbol_size));
	if (!ent)
		goto err_source_module;

	/* Store the module as a local variable. */
	jit_object_entry_fini(ent);
	ent->oe_type  = JIT_OBJECT_ENTRY_TYPE_LOCAL;
	ent->oe_value = source_module; /* Inherit reference */
	return 0;
err_source_module:
	Dee_Decref(source_module);
err:
	return -1;
}


/* Import a named symbol from a module and binding it to a local symbol. */
INTERN WUNUSED NONNULL((1, 2, 3)) int DFCALL
JITContext_DoImportSymbol(JITContext *__restrict self,
                          struct jit_import_item const *__restrict spec,
                          DeeObject *__restrict source_module) {
	JITObjectTable *tab;
	struct jit_object_entry *ent;
	Dee_hash_t symbol_hash;
	tab = JITContext_GetRWLocals(self);
	if unlikely(!tab)
		goto err;
	symbol_hash = Dee_HashUtf8(spec->ii_symbol_name, spec->ii_symbol_size);
	ent = JITObjectTable_Create(tab, spec->ii_symbol_name, spec->ii_symbol_size, symbol_hash);
	if (!ent)
		goto err;
	jit_object_entry_fini(ent);
	if (DeeModule_Check(source_module)) {
		struct Dee_module_symbol *modsym;
		char const *source_name;
		size_t source_size;
		Dee_hash_t source_hash;
		if (spec->ii_import_name) {
			source_name = DeeString_AsUtf8((DeeObject *)spec->ii_import_name);
			if unlikely(!source_name)
				goto err;
			source_size = WSTR_LENGTH(source_name);
			source_hash = DeeString_Hash(spec->ii_import_name);
		} else {
			source_name = spec->ii_symbol_name;
			source_size = spec->ii_symbol_size;
			source_hash = symbol_hash;
		}
		modsym = DeeModule_GetSymbolStringLenHash((DeeModuleObject *)source_module,
		                                          source_name, source_size, source_hash);
		if unlikely(!modsym) {
			DeeError_Throwf(&DeeError_SyntaxError,
			                "Symbol `%$s' could not be found in module `%s'",
			                source_size, source_name,
			                DeeModule_GetShortName((DeeModuleObject *)source_module));
			goto err;
		}
		ent->oe_type = JIT_OBJECT_ENTRY_EXTERN_SYMBOL;
		ent->oe_extern_symbol.es_mod = (DeeModuleObject *)source_module;
		ent->oe_extern_symbol.es_sym = modsym;
		Dee_Incref(source_module);
	} else if (spec->ii_import_name) {
		ent->oe_type = JIT_OBJECT_ENTRY_EXTERN_ATTR;
		ent->oe_extern_attr.ea_base = source_module;
		ent->oe_extern_attr.ea_name = spec->ii_import_name;
		Dee_Incref(spec->ii_import_name);
		Dee_Incref(source_module);
	} else {
		ent->oe_type = JIT_OBJECT_ENTRY_EXTERN_ATTRSTR;
		ent->oe_extern_attrstr.eas_base = source_module;
		ent->oe_extern_attrstr.eas_name = spec->ii_symbol_name;
		ent->oe_extern_attrstr.eas_size = spec->ii_symbol_size;
		ent->oe_extern_attrstr.eas_hash = symbol_hash;
		Dee_Incref(source_module);
	}
	return 0;
err:
	return -1;
}

/* Import a all symbols from a module and bind them to local symbols. */
INTERN WUNUSED NONNULL((1, 2)) int DFCALL
JITContext_DoImportStar(JITContext *__restrict self,
                        DeeObject *__restrict source_module) {
	JITObjectTable *tab;
	tab = JITContext_GetRWLocals(self);
	if unlikely(!tab)
		goto err;
	return JITObjectTable_AddImportStar(tab, source_module);
err:
	return -1;
}


DECL_END

#endif /* !GUARD_DEX_STREXEC_CONTEXT_C */
