/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_JIT_CONTEXT_C
#define GUARD_DEX_JIT_CONTEXT_C 1
#define DEE_SOURCE

#include "libjit.h"
/**/

#include <deemon/alloc.h>
#include <deemon/class.h>
#include <deemon/compiler/lexer.h>
#include <deemon/dict.h>
#include <deemon/error.h>
#include <deemon/instancemethod.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/system-features.h> /* memcpy() */
#include <deemon/util/lock.h>
#include <deemon/util/objectlist.h>

#include <hybrid/atomic.h>
#include <hybrid/unaligned.h>

DECL_BEGIN

INTERN void FCALL
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

INTERN void FCALL
JITLValue_Visit(JITLValue *__restrict self, dvisit_t proc, void *arg) {
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


PRIVATE bool FCALL
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
do_reload: {
	dhash_t i, perturb, hash;
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
		ASSERT(tp->tp_class);
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

#define CATCH_ATTRIBUTE_ERROR()                  \
	(DeeError_Catch(&DeeError_AttributeError) || \
	 DeeError_Catch(&DeeError_NotImplemented))


PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeInstance_GetAttribute(struct instance_desc *__restrict self,
                         DeeObject *__restrict this_arg,
                         struct class_attribute *__restrict attr) {
	DREF DeeObject *result;
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance_from_instance(self, this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		atomic_rwlock_read(&self->id_lock);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		atomic_rwlock_endread(&self->id_lock);
		if unlikely(!getter)
			goto illegal;
		/* Invoke the getter. */
		result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		         ? DeeObject_ThisCall(getter, this_arg, 0, NULL)
		         : DeeObject_Call(getter, 0, NULL);
		Dee_Decref(getter);
	} else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
		/* Construct a thiscall function. */
		DREF DeeObject *callback;
		atomic_rwlock_read(&self->id_lock);
		callback = self->id_vtab[attr->ca_addr];
		Dee_XIncref(callback);
		atomic_rwlock_endread(&self->id_lock);
		if unlikely(!callback)
			goto unbound;
		result = DeeInstanceMethod_New(callback, this_arg);
		Dee_Decref(callback);
	} else {
		/* Simply return the attribute as-is. */
		atomic_rwlock_read(&self->id_lock);
		result = self->id_vtab[attr->ca_addr];
		Dee_XIncref(result);
		atomic_rwlock_endread(&self->id_lock);
		if unlikely(!result)
			goto unbound;
	}
	return result;
unbound:
	err_unbound_attribute_c(class_desc_from_instance(self, this_arg),
	                        DeeString_STR(attr->ca_name));
	return NULL;
illegal:
	err_cant_access_attribute_c(class_desc_from_instance(self, this_arg),
	                            DeeString_STR(attr->ca_name),
	                            ATTR_ACCESS_GET);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeInstance_BoundAttribute(struct instance_desc *__restrict self,
                           DeeObject *__restrict this_arg,
                           struct class_attribute *__restrict attr) {
	DREF DeeObject *result;
	ASSERT_OBJECT(this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
		self = class_desc_as_instance_from_instance(self, this_arg);
	if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
		DREF DeeObject *getter;
		atomic_rwlock_read(&self->id_lock);
		getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
		Dee_XIncref(getter);
		atomic_rwlock_endread(&self->id_lock);
		if unlikely(!getter)
			goto unbound;
		/* Invoke the getter. */
		result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		         ? DeeObject_ThisCall(getter, this_arg, 0, NULL)
		         : DeeObject_Call(getter, 0, NULL);
		Dee_Decref(getter);
		if likely(result) {
			Dee_Decref(result);
			return 1;
		}
		if (CATCH_ATTRIBUTE_ERROR())
			return 0; /* return -3; */
		if (DeeError_Catch(&DeeError_UnboundAttribute))
			return 0;
		return -1;
	} else {
		/* Simply return the attribute as-is. */
#ifdef CONFIG_NO_THREADS
		return self->id_vtab[attr->ca_addr] != NULL;
#else /* CONFIG_NO_THREADS */
		return ATOMIC_READ(self->id_vtab[attr->ca_addr]) != NULL;
#endif /* !CONFIG_NO_THREADS */
	}
unbound:
	return 0;
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeInstance_DelAttribute(struct instance_desc *__restrict self,
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
		atomic_rwlock_read(&self->id_lock);
		delfun = self->id_vtab[attr->ca_addr + CLASS_GETSET_DEL];
		Dee_XIncref(delfun);
		atomic_rwlock_endread(&self->id_lock);
		if unlikely(!delfun)
			goto illegal;
		/* Invoke the getter. */
		temp = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		       ? DeeObject_ThisCall(delfun, this_arg, 0, NULL)
		       : DeeObject_Call(delfun, 0, NULL);
		Dee_Decref(delfun);
		if unlikely(!temp)
			goto err;
		Dee_Decref(temp);
	} else {
		DREF DeeObject *old_value;
		/* Simply unbind the field in the attr table. */
		atomic_rwlock_write(&self->id_lock);
		old_value = self->id_vtab[attr->ca_addr];
		self->id_vtab[attr->ca_addr] = NULL;
		atomic_rwlock_endwrite(&self->id_lock);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
		if unlikely(!old_value)
			goto unbound;
		Dee_Decref(old_value);
#else /* CONFIG_ERROR_DELETE_UNBOUND */
		Dee_XDecref(old_value);
#endif /* !CONFIG_ERROR_DELETE_UNBOUND */
	}
	return 0;
#ifdef CONFIG_ERROR_DELETE_UNBOUND
unbound:
	err_unbound_attribute_c(class_desc_from_instance(self, this_arg),
	                        DeeString_STR(attr->ca_name));
	goto err;
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
illegal:
	err_cant_access_attribute_c(class_desc_from_instance(self, this_arg),
	                            DeeString_STR(attr->ca_name),
	                            ATTR_ACCESS_DEL);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeInstance_SetAttribute(struct instance_desc *__restrict self,
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
		atomic_rwlock_read(&self->id_lock);
		setter = self->id_vtab[attr->ca_addr + CLASS_GETSET_SET];
		Dee_XIncref(setter);
		atomic_rwlock_endread(&self->id_lock);
		if unlikely(!setter)
			goto illegal;
		/* Invoke the getter. */
		temp = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
		       ? DeeObject_ThisCall(setter, this_arg, 1, (DeeObject **)&value)
		       : DeeObject_Call(setter, 1, (DeeObject **)&value);
		Dee_Decref(setter);
		if unlikely(!temp)
			goto err;
		Dee_Decref(temp);
	} else {
		DREF DeeObject *old_value;
		/* Simply override the field in the attr table. */
		atomic_rwlock_write(&self->id_lock);
		old_value = self->id_vtab[attr->ca_addr];
		if (old_value && (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
			atomic_rwlock_endwrite(&self->id_lock);
			goto illegal; /* readonly fields can only be set once. */
		} else {
			Dee_Incref(value);
			self->id_vtab[attr->ca_addr] = value;
		}
		atomic_rwlock_endwrite(&self->id_lock);
		/* Drop a reference from the old value. */
		Dee_XDecref(old_value);
	}
	return 0;
illegal:
	err_cant_access_attribute_c(class_desc_from_instance(self, this_arg),
	                            DeeString_STR(attr->ca_name),
	                            ATTR_ACCESS_SET);
err:
	return -1;
}



INTERN int FCALL
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
		if (result < -1)
			result = 0; /* Attribute doesn't exist */
		break;

	case JIT_LVALUE_GLOBAL:
		if unlikely(!context->jc_globals)
			return 0;
		result = DeeObject_BoundItem(context->jc_globals,
		                             (DeeObject *)self->lv_global,
		                             true);
		if (result < -1)
			result = 0; /* Attribute doesn't exist */
		break;

	case JIT_LVALUE_GLOBALSTR:
		if unlikely(!context->jc_globals)
			return 0;
		result = DeeObject_BoundItemStringLen(context->jc_globals,
		                                      self->lv_globalstr.lg_namestr,
		                                      self->lv_globalstr.lg_namelen,
		                                      self->lv_globalstr.lg_namehsh,
		                                      true);
		if (result < -1)
			result = 0; /* Attribute doesn't exist */
		break;

	case JIT_LVALUE_CLSATTRIB:
		result = DeeInstance_BoundAttribute(self->lv_clsattrib.lc_desc,
		                                    self->lv_clsattrib.lc_obj,
		                                    self->lv_clsattrib.lc_attr);
		break;

	case JIT_LVALUE_ATTR:
		result = DeeObject_BoundAttr(self->lv_attr.la_base,
		                             (DeeObject *)self->lv_attr.la_name);
		if (result < -1)
			result = 0; /* Attribute doesn't exist */
		break;

	case JIT_LVALUE_ATTRSTR:
		result = DeeObject_BoundAttrStringLenHash(self->lv_attrstr.la_base,
		                                          self->lv_attrstr.la_name,
		                                          self->lv_attrstr.la_size,
		                                          self->lv_attrstr.la_hash);
		if (result < -1)
			result = 0; /* Attribute doesn't exist */
		break;

	case JIT_LVALUE_ITEM:
		result = DeeObject_BoundItem(self->lv_item.li_base,
		                             self->lv_item.li_index,
		                             true);
		if (result < -1)
			result = 0; /* Item doesn't exist. */
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




INTERN WUNUSED DREF DeeObject *FCALL
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
			err_unknown_global((DeeObject *)self->lv_global);
			goto err;
		}
		result = DeeObject_GetItem(context->jc_globals,
		                           (DeeObject *)self->lv_global);
		break;

	case JIT_LVALUE_GLOBALSTR:
		if unlikely(!context->jc_globals) {
			err_unknown_global_str_len(self->lv_globalstr.lg_namestr,
			                           self->lv_globalstr.lg_namelen);
			goto err;
		}
		result = DeeObject_GetItemStringLen(context->jc_globals,
		                                    self->lv_globalstr.lg_namestr,
		                                    self->lv_globalstr.lg_namelen,
		                                    self->lv_globalstr.lg_namehsh);
		break;

	case JIT_LVALUE_CLSATTRIB:
		result = DeeInstance_GetAttribute(self->lv_clsattrib.lc_desc,
		                                  self->lv_clsattrib.lc_obj,
		                                  self->lv_clsattrib.lc_attr);
		break;

	case JIT_LVALUE_ATTR:
		result = DeeObject_GetAttr(self->lv_attr.la_base,
		                           (DeeObject *)self->lv_attr.la_name);
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




INTERN int FCALL
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
			return err_unknown_global((DeeObject *)self->lv_global);
		result = DeeObject_DelItem(context->jc_globals,
		                           (DeeObject *)self->lv_global);
		break;

	case JIT_LVALUE_GLOBALSTR:
		if unlikely(!context->jc_globals)
			return err_unknown_global_str_len(self->lv_globalstr.lg_namestr,
		                                  self->lv_globalstr.lg_namelen);
		result = DeeObject_DelItemStringLen(context->jc_globals,
		                                    self->lv_globalstr.lg_namestr,
		                                    self->lv_globalstr.lg_namelen,
		                                    self->lv_globalstr.lg_namehsh);
		break;

	case JIT_LVALUE_CLSATTRIB:
		result = DeeInstance_DelAttribute(self->lv_clsattrib.lc_desc,
		                                  self->lv_clsattrib.lc_obj,
		                                  self->lv_clsattrib.lc_attr);
		break;

	case JIT_LVALUE_ATTR:
		result = DeeObject_DelAttr(self->lv_attr.la_base,
		                           (DeeObject *)self->lv_attr.la_name);
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




INTERN int FCALL
JITLValue_SetValue(JITLValue *__restrict self,
                   JITContext *__restrict context,
                   DeeObject *__restrict value) {
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
		                           (DeeObject *)self->lv_global,
		                           value);
		break;

	case JIT_LVALUE_GLOBALSTR:
		if (!context->jc_globals) {
			context->jc_globals = DeeDict_New();
			if unlikely(!context->jc_globals)
				goto err;
		}
		result = DeeObject_SetItemStringLen(context->jc_globals,
		                                    self->lv_globalstr.lg_namestr,
		                                    self->lv_globalstr.lg_namelen,
		                                    self->lv_globalstr.lg_namehsh,
		                                    value);
		break;

	case JIT_LVALUE_CLSATTRIB:
		result = DeeInstance_SetAttribute(self->lv_clsattrib.lc_desc,
		                                  self->lv_clsattrib.lc_obj,
		                                  self->lv_clsattrib.lc_attr,
		                                  value);
		break;

	case JIT_LVALUE_ATTR:
		result = DeeObject_SetAttr(self->lv_attr.la_base,
		                           (DeeObject *)self->lv_attr.la_name,
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
		new_list = (JITLValue *)Dee_TryRealloc(self->ll_list,
		                                       new_alloc *
		                                       sizeof(JITLValue));
		if unlikely(!new_list) {
			new_alloc = self->ll_size + 1;
			new_list = (JITLValue *)Dee_Realloc(self->ll_list,
			                                    new_alloc *
			                                    sizeof(JITLValue));
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
		new_list = (JITLValue *)Dee_TryRealloc(self->ll_list,
		                                       new_alloc *
		                                       sizeof(JITLValue));
		if unlikely(!new_list) {
			new_alloc = self->ll_size + 1;
			new_list = (JITLValue *)Dee_Realloc(self->ll_list,
			                                    new_alloc *
			                                    sizeof(JITLValue));
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
	dst->ol_size -= (self->ll_size - i);
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
	DREF DeeObject *iterator, *elem;
	size_t fast_size, i = 0;
	int temp;
	/* Try to make use of the fast-sequence API. */
	fast_size = DeeFastSeq_GetSize(values);
	if (fast_size != DEE_FASTSEQ_NOTFAST) {
		if (self->ll_size != fast_size)
			return err_invalid_unpack_size(values, self->ll_size, fast_size);
		for (; i < fast_size; ++i) {
			elem = DeeFastSeq_GetItem(values, i);
			if unlikely(!elem)
				goto err;
			temp = JITLValue_SetValue(&self->ll_list[i],
			                          context,
			                          elem);
			Dee_Decref(elem);
			if unlikely(temp)
				goto err;
		}
		goto done;
	}
	if (DeeNone_Check(values)) {
		/* Special case: `none' can be unpacked into anything. */
		for (; i < fast_size; ++i) {
			if unlikely(JITLValue_SetValue(&self->ll_list[i],
				                            context,
				                            Dee_None))
			goto err;
		}
		goto done;
	}
	/* Fallback: Use an iterator. */
	if ((iterator = DeeObject_IterSelf(values)) == NULL)
		goto err;
	for (; i < self->ll_size; ++i) {
		elem = DeeObject_IterNext(iterator);
		if unlikely(!ITER_ISOK(elem)) {
			if (elem)
				err_invalid_unpack_size(values, self->ll_size, i);
			goto err_iter;
		}
		temp = JITLValue_SetValue(&self->ll_list[i],
		                          context,
		                          elem);
		Dee_Decref(elem);
		if unlikely(temp)
			goto err_iter;
	}
	/* Check to make sure that the iterator actually ends here. */
	elem = DeeObject_IterNext(iterator);
	if unlikely(elem != ITER_DONE) {
		if (elem)
			err_invalid_unpack_iter_size(values, iterator, self->ll_size);
		goto err_iter;
	}
	Dee_Decref(iterator);
	return 0;
done:
	return 0;
err_iter:
	Dee_Decref(iterator);
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
INTERN int FCALL
JITContext_Lookup(JITContext *__restrict self,
                  struct jit_symbol *__restrict result,
                  /*utf-8*/ char const *__restrict name,
                  size_t namelen, unsigned int mode) {
	JITObjectTable *tab;
	struct jit_object_entry *ent;
	dhash_t hash = Dee_HashUtf8(name, namelen);
	switch (mode & LOOKUP_SYM_VMASK) {

	case LOOKUP_SYM_VLOCAL:
		/* Search for a local symbol. */
		tab = JITContext_GetROLocals(self);
		if (!tab)
			break; /* No locals */
		ent = JITObjectTable_Lookup(tab,
		                            name,
		                            namelen,
		                            hash);
		if (!ent)
			break;
		/* Found a local variable entry. */
set_object_entry:
		switch (ent->oe_type) {

		case JIT_OBJECT_ENTRY_TYPE_ATTR_FIXED:
			result->js_kind              = JIT_SYMBOL_CLSATTRIB;
			result->js_clsattrib.jc_obj  = ent->oe_attr_fixed.af_obj;
			result->js_clsattrib.jc_attr = ent->oe_attr_fixed.af_attr;
			result->js_clsattrib.jc_desc = ent->oe_attr_fixed.af_desc;
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
			if (DeeObject_AssertType(result->js_clsattrib.jc_obj,
			                         ent->oe_attr.a_class)) {
				Dee_Decref(result->js_clsattrib.jc_obj);
				goto err;
			}
			result->js_kind              = JIT_SYMBOL_CLSATTRIB;
			result->js_clsattrib.jc_attr = ent->oe_attr.a_attr;
			result->js_clsattrib.jc_desc = DeeInstance_DESC(ent->oe_attr.a_class->tp_class,
			                                                result->js_clsattrib.jc_obj);
		}	break;

		default:
			result->js_kind              = JIT_SYMBOL_OBJENT;
			result->js_objent.jo_tab     = tab;
			result->js_objent.jo_ent     = ent;
			result->js_objent.jo_namestr = ent->oe_namestr;
			result->js_objent.jo_namelen = ent->oe_namelen;
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
		result->js_kind                 = JIT_SYMBOL_GLOBALSTR;
		result->js_globalstr.jg_namestr = name;
		result->js_globalstr.jg_namelen = namelen;
		result->js_globalstr.jg_namehsh = hash;
		goto done;

	default:
		tab = JITContext_GetROLocals(self);
		for (; tab; tab = tab->ot_prev.otp_tab) {
			ent = JITObjectTable_Lookup(tab,
			                            name,
			                            namelen,
			                            hash);
			if (ent)
				goto set_object_entry;
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
			error = DeeObject_HasItemStringLen(self->jc_globals,
			                                   name,
			                                   namelen,
			                                   hash);
			if unlikely(error < 0)
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
	ent = JITObjectTable_Create(tab,
	                            name,
	                            namelen,
	                            hash);
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

INTERN int FCALL
JITContext_LookupNth(JITContext *__restrict self,
                     struct jit_symbol *__restrict result,
                     /*utf-8*/ char const *__restrict name,
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



DECL_END

#endif /* !GUARD_DEX_JIT_CONTEXT_C */
