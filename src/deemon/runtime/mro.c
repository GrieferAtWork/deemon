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
#ifndef GUARD_DEEMON_RUNTIME_MRO_C
#define GUARD_DEEMON_RUNTIME_MRO_C 1
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/attribute.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/instancemethod.h>
#include <deemon/kwds.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none-operator.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/property.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* bcmpc(), ... */
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/overflow.h>
#include <hybrid/sched/yield.h>
#include <hybrid/sequence/list.h>
#include <hybrid/typecore.h>
/**/

#include "runtime_error.h"
/**/

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY
#define VALIST_ADDR(x) (&(x))
#else /* CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY */
#define VALIST_ADDR(x) (&(x)[0])
#endif /* !CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

#ifndef CONFIG_HAVE_strcmpz
#define CONFIG_HAVE_strcmpz
#undef strcmpz
#define strcmpz dee_strcmpz
DeeSystem_DEFINE_strcmpz(dee_strcmpz)
#endif /* !CONFIG_HAVE_strcmpz */

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */



INTDEF WUNUSED NONNULL((1)) DeeTypeObject *DCALL
type_member_typefor(struct type_member const *__restrict self);

/* Try to determine the type of object returned by `Dee_attrinfo_callget()'
 * When the type cannot be determined, return `NULL' instead. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
Dee_attrinfo_typeof(struct Dee_attrinfo *__restrict self) {
	switch (self->ai_type) {

	case Dee_ATTRINFO_MODSYM: {
		DeeModuleObject *mod;
		struct Dee_module_symbol const *sym;
		mod = (DeeModuleObject *)self->ai_decl;
		sym = self->ai_value.v_modsym;
		ASSERT(sym >= mod->mo_bucketv &&
		       sym <= mod->mo_bucketv + mod->mo_bucketm);
		if (sym->ss_flags & MODSYM_FPROPERTY)
			break;
		if (!(sym->ss_flags & MODSYM_FREADONLY))
			break;
		if likely(!(sym->ss_flags & MODSYM_FEXTERN)) {
			DeeObject *symval;
read_modsym:
			ASSERT(sym->ss_index < mod->mo_globalc);
			DeeModule_LockRead(mod);
			symval = mod->mo_globalv[sym->ss_index];
			if (symval) {
				DREF DeeTypeObject *result;
				result = Dee_TYPE(symval);
				Dee_Incref(result);
				DeeModule_LockEndRead(mod);
				return result;
			}
			DeeModule_LockEndRead(mod);
			break;
		}

		/* External symbol. */
		ASSERT(sym->ss_extern.ss_impid < mod->mo_importc);
		mod = mod->mo_importv[sym->ss_extern.ss_impid];
		goto read_modsym;
	}	break;

	case Dee_ATTRINFO_METHOD: {
		DeeTypeObject *result = &DeeObjMethod_Type;
		if (self->ai_value.v_method->m_flag & Dee_TYPE_METHOD_FKWDS)
			result = &DeeKwObjMethod_Type;
		Dee_Incref(result);
		return result;
	}	break;

	case Dee_ATTRINFO_MEMBER: {
		DeeTypeObject *result;
		result = type_member_typefor(self->ai_value.v_member);
		if (result) {
			Dee_Incref(result);
			return result;
		}
	}	break;

	case Dee_ATTRINFO_ATTR: {
		struct class_desc *desc;
		struct class_attribute const *attr;
		DeeObject *value;
		attr = self->ai_value.v_attr;
		if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
			break;
		if (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FMETHOD) {
			Dee_Incref(&DeeInstanceMethod_Type);
			return &DeeInstanceMethod_Type;
		}
		if (!(attr->ca_flag & Dee_CLASS_ATTRIBUTE_FCLASSMEM))
			break;
		if (!(attr->ca_flag & Dee_CLASS_ATTRIBUTE_FREADONLY))
			break;
		desc = DeeClass_DESC(self->ai_decl);
		Dee_class_desc_lock_read(desc);
		value = desc->cd_members[attr->ca_addr];
		if (value) {
			DREF DeeTypeObject *result;
			result = Dee_TYPE(value);
			Dee_Incref(result);
			Dee_class_desc_lock_endread(desc);
			return result;
		}
		Dee_class_desc_lock_endread(desc);
	}	break;

	case Dee_ATTRINFO_INSTANCE_METHOD: {
		DeeTypeObject *result = &DeeClsMethod_Type;
		if (self->ai_value.v_method->m_flag & Dee_TYPE_METHOD_FKWDS)
			result = &DeeKwClsMethod_Type;
		Dee_Incref(result);
		return result;
	}	break;

	case Dee_ATTRINFO_INSTANCE_GETSET:
		Dee_Incref(&DeeClsProperty_Type);
		return &DeeClsProperty_Type;

	case Dee_ATTRINFO_INSTANCE_MEMBER:
		Dee_Incref(&DeeClsMember_Type);
		return &DeeClsMember_Type;

	case Dee_ATTRINFO_INSTANCE_ATTR: {
		struct class_desc *desc;
		struct class_attribute const *attr;
		DeeObject *value;
		attr = self->ai_value.v_attr;
		if (!(attr->ca_flag & Dee_CLASS_ATTRIBUTE_FCLASSMEM)) {
			Dee_Incref(&DeeInstanceMember_Type);
			return &DeeInstanceMember_Type;
		}
		if (attr->ca_flag & Dee_CLASS_ATTRIBUTE_FGETSET) {
			Dee_Incref(&DeeProperty_Type);
			return &DeeProperty_Type;
		}
		desc = DeeClass_DESC(self->ai_decl);
		Dee_class_desc_lock_read(desc);
		value = desc->cd_members[attr->ca_addr];
		if (value) {
			DREF DeeTypeObject *result;
			result = Dee_TYPE(value);
			Dee_Incref(result);
			Dee_class_desc_lock_endread(desc);
			return result;
		}
		Dee_class_desc_lock_endread(desc);
	}	break;

	default: break;
	}
	return NULL;
}

/* Assert that the `Dee_ATTRINFO_*' helper macros function correctly. */
STATIC_ASSERT(!Dee_ATTRINFO_ISINSTANCE(Dee_ATTRINFO_CUSTOM));
STATIC_ASSERT(!Dee_ATTRINFO_ISINSTANCE(Dee_ATTRINFO_MODSYM));
STATIC_ASSERT(!Dee_ATTRINFO_ISINSTANCE(Dee_ATTRINFO_METHOD));
STATIC_ASSERT(!Dee_ATTRINFO_ISINSTANCE(Dee_ATTRINFO_GETSET));
STATIC_ASSERT(!Dee_ATTRINFO_ISINSTANCE(Dee_ATTRINFO_MEMBER));
STATIC_ASSERT(!Dee_ATTRINFO_ISINSTANCE(Dee_ATTRINFO_ATTR));

STATIC_ASSERT(Dee_ATTRINFO_ISINSTANCE(Dee_ATTRINFO_INSTANCE_METHOD));
STATIC_ASSERT(Dee_ATTRINFO_ISINSTANCE(Dee_ATTRINFO_INSTANCE_GETSET));
STATIC_ASSERT(Dee_ATTRINFO_ISINSTANCE(Dee_ATTRINFO_INSTANCE_MEMBER));
STATIC_ASSERT(Dee_ATTRINFO_ISINSTANCE(Dee_ATTRINFO_INSTANCE_ATTR));

STATIC_ASSERT(Dee_ATTRINFO_WITHOUTINSTANCE(Dee_ATTRINFO_INSTANCE_METHOD) == Dee_ATTRINFO_METHOD);
STATIC_ASSERT(Dee_ATTRINFO_WITHOUTINSTANCE(Dee_ATTRINFO_INSTANCE_GETSET) == Dee_ATTRINFO_GETSET);
STATIC_ASSERT(Dee_ATTRINFO_WITHOUTINSTANCE(Dee_ATTRINFO_INSTANCE_MEMBER) == Dee_ATTRINFO_MEMBER);
STATIC_ASSERT(Dee_ATTRINFO_WITHOUTINSTANCE(Dee_ATTRINFO_INSTANCE_ATTR) == Dee_ATTRINFO_ATTR);

STATIC_ASSERT(Dee_ATTRINFO_WITHINSTANCE(Dee_ATTRINFO_METHOD) == Dee_ATTRINFO_INSTANCE_METHOD);
STATIC_ASSERT(Dee_ATTRINFO_WITHINSTANCE(Dee_ATTRINFO_GETSET) == Dee_ATTRINFO_INSTANCE_GETSET);
STATIC_ASSERT(Dee_ATTRINFO_WITHINSTANCE(Dee_ATTRINFO_MEMBER) == Dee_ATTRINFO_INSTANCE_MEMBER);
STATIC_ASSERT(Dee_ATTRINFO_WITHINSTANCE(Dee_ATTRINFO_ATTR) == Dee_ATTRINFO_INSTANCE_ATTR);

/* Same as `Dee_attrinfo_typeof()', but `Dee_ATTRINFO_INSTANCE_*' attributes
 * as though they were the equivalent `Dee_ATTRINFO_*' (iow: the returned type
 * does not describe the type when accessed using the enumerated object, but
 * in the case of types and instance-attributes, the type when accessed using
 * an instance of the enumerated object) */
PUBLIC WUNUSED NONNULL((1)) DREF DeeTypeObject *DCALL
Dee_attrinfo_typeof_ininstance(struct Dee_attrinfo *__restrict self) {
	struct Dee_attrinfo regular;
	if (!Dee_ATTRINFO_ISINSTANCE(self->ai_type))
		return Dee_attrinfo_typeof(self);
	memcpy(&regular, self, sizeof(struct Dee_attrinfo));
	regular.ai_type = Dee_ATTRINFO_WITHOUTINSTANCE(regular.ai_type);
	return Dee_attrinfo_typeof(&regular);
}



PRIVATE ATTR_COLD NONNULL((1, 2)) int DCALL
err_bad_module_access_thisarg(struct Dee_attrdesc const *self, DeeObject *thisarg) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Bad thisarg for module access: %r (expected %r)",
	                       thisarg, self->ad_info.ai_decl);
}

PRIVATE ATTR_COLD NONNULL((1, 2)) int DCALL
err_bad_instance_access_thisarg(struct Dee_attrdesc const *self, DeeObject *thisarg) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Bad thisarg for instance access: %r (expected %r)",
	                       thisarg, self->ad_info.ai_decl);
}

PRIVATE WUNUSED int DCALL
bound_fromob(/*inherit(always)*/ DREF DeeObject *value) {
	if (value) {
		Dee_Decref(value);
		return Dee_BOUND_YES;
	}
	if (DeeError_Catch(&DeeError_UnboundAttribute))
		return Dee_BOUND_NO;
	return Dee_BOUND_ERR;
}


/* Perform standard operations on the attribute behind `struct Dee_attrdesc' */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Dee_attrdesc_callget(struct Dee_attrdesc const *self, DeeObject *thisarg) {
	switch (self->ad_info.ai_type) {

	case Dee_ATTRINFO_CUSTOM: {
		struct type_attr const *tp_attr;
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		tp_attr = self->ad_info.ai_value.v_custom;
		if (tp_attr->tp_getattr) {
			DREF DeeObject *attrob, *result;
			if (self->ad_perm & Dee_ATTRPERM_F_NAMEOBJ)
				return (*tp_attr->tp_getattr)(thisarg, (DeeObject *)Dee_attrdesc_nameobj(self));
			if (tp_attr->tp_getattr_string_hash)
				return (*tp_attr->tp_getattr_string_hash)(thisarg, self->ad_name, Dee_HashStr(self->ad_name));
			attrob = DeeString_New(self->ad_name);
			if unlikely(!attrob)
				goto err;
			result = (*tp_attr->tp_getattr)(thisarg, attrob);
			Dee_Decref_likely(attrob);
			return result;
		}
	}	break;

	case Dee_ATTRINFO_MODSYM:
		if unlikely(self->ad_info.ai_decl != thisarg) {
			err_bad_module_access_thisarg(self, thisarg);
			goto err;
		}
		return DeeModule_GetAttrSymbol((DeeModuleObject *)thisarg,
		                               self->ad_info.ai_value.v_modsym);

	case Dee_ATTRINFO_METHOD:
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		if (self->ad_info.ai_value.v_method->m_flag & Dee_TYPE_METHOD_FKWDS)
			return DeeKwObjMethod_New((Dee_kwobjmethod_t)self->ad_info.ai_value.v_method->m_func, thisarg);
		return DeeObjMethod_New(self->ad_info.ai_value.v_method->m_func, thisarg);

	case Dee_ATTRINFO_GETSET:
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		if (self->ad_info.ai_value.v_getset->gs_get)
			return (*self->ad_info.ai_value.v_getset->gs_get)(thisarg);
		break;

	case Dee_ATTRINFO_MEMBER:
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		return Dee_type_member_get(self->ad_info.ai_value.v_member, thisarg);

	case Dee_ATTRINFO_ATTR: {
		struct class_desc *desc;
		struct instance_desc *inst;
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		desc = DeeClass_DESC(self->ad_info.ai_decl);
		inst = DeeInstance_DESC(desc, thisarg);
		return DeeInstance_GetAttribute(desc, inst, thisarg, self->ad_info.ai_value.v_attr);
	}	break;

	case Dee_ATTRINFO_INSTANCE_METHOD:
	case Dee_ATTRINFO_INSTANCE_GETSET:
	case Dee_ATTRINFO_INSTANCE_MEMBER:
	case Dee_ATTRINFO_INSTANCE_ATTR: {
		if unlikely(thisarg != self->ad_info.ai_decl) {
			err_bad_instance_access_thisarg(self, thisarg);
			goto err;
		}
		switch (self->ad_info.ai_type) {
		case Dee_ATTRINFO_INSTANCE_METHOD:
			if (self->ad_info.ai_value.v_instance_method->m_flag & Dee_TYPE_METHOD_FKWDS)
				return DeeKwClsMethod_New((DeeTypeObject *)thisarg, (Dee_kwobjmethod_t)self->ad_info.ai_value.v_instance_method->m_func);
			return DeeClsMethod_New((DeeTypeObject *)thisarg, self->ad_info.ai_value.v_instance_method->m_func);
		case Dee_ATTRINFO_INSTANCE_GETSET: {
			struct type_getset const *gs = self->ad_info.ai_value.v_instance_getset;
			return DeeClsProperty_New((DeeTypeObject *)thisarg, gs);
		}	break;
		case Dee_ATTRINFO_INSTANCE_MEMBER:
			return DeeClsMember_New((DeeTypeObject *)thisarg, self->ad_info.ai_value.v_instance_member);
		case Dee_ATTRINFO_INSTANCE_ATTR:
			return DeeClass_GetInstanceAttribute((DeeTypeObject *)thisarg, self->ad_info.ai_value.v_instance_attr);
		default: __builtin_unreachable();
		}
	}	break;

	default: break;
	}
	err_cant_access_attribute_string((DeeTypeObject *)self->ad_info.ai_decl,
	                                 self->ad_name, ATTR_ACCESS_GET);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
Dee_attrdesc_callbound(struct Dee_attrdesc const *self, DeeObject *thisarg) {
	switch (self->ad_info.ai_type) {

	case Dee_ATTRINFO_CUSTOM: {
		struct type_attr const *tp_attr;
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		tp_attr = self->ad_info.ai_value.v_custom;
		if (tp_attr->tp_getattr) {
			DREF DeeObject *result;
			if (tp_attr->tp_boundattr && (self->ad_perm & Dee_ATTRPERM_F_NAMEOBJ))
				return (*tp_attr->tp_boundattr)(thisarg, (DeeObject *)Dee_attrdesc_nameobj(self));
			if (tp_attr->tp_boundattr_string_hash)
				return (*tp_attr->tp_boundattr_string_hash)(thisarg, self->ad_name, Dee_HashStr(self->ad_name));
			if (self->ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
				result = (*tp_attr->tp_getattr)(thisarg, (DeeObject *)Dee_attrdesc_nameobj(self));
			} else if (tp_attr->tp_getattr_string_hash) {
				result = (*tp_attr->tp_getattr_string_hash)(thisarg, self->ad_name, Dee_HashStr(self->ad_name));
			} else {
				DREF DeeObject *attrob;
				attrob = DeeString_New(self->ad_name);
				if unlikely(!attrob)
					goto err;
				result = (*tp_attr->tp_getattr)(thisarg, attrob);
				Dee_Decref_likely(attrob);
			}
			return bound_fromob(result);
		}
	}	break;

	case Dee_ATTRINFO_MODSYM:
		if unlikely(self->ad_info.ai_decl != thisarg) {
			err_bad_module_access_thisarg(self, thisarg);
			goto err;
		}
		return DeeModule_BoundAttrSymbol((DeeModuleObject *)thisarg,
		                                 self->ad_info.ai_value.v_modsym);

	case Dee_ATTRINFO_METHOD:
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		return Dee_BOUND_YES;

	case Dee_ATTRINFO_GETSET:
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		if (self->ad_info.ai_value.v_getset->gs_bound)
			return (*self->ad_info.ai_value.v_getset->gs_bound)(thisarg);
		if (self->ad_info.ai_value.v_getset->gs_get)
			return bound_fromob((*self->ad_info.ai_value.v_getset->gs_get)(thisarg));
		break;

	case Dee_ATTRINFO_MEMBER: {
		bool isbound;
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		isbound = Dee_type_member_bound(self->ad_info.ai_value.v_member, thisarg);
		return Dee_BOUND_FROMBOOL(isbound);
	}	break;

	case Dee_ATTRINFO_ATTR: {
		struct class_desc *desc;
		struct instance_desc *inst;
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		desc = DeeClass_DESC(self->ad_info.ai_decl);
		inst = DeeInstance_DESC(desc, thisarg);
		return DeeInstance_BoundAttribute(desc, inst, thisarg, self->ad_info.ai_value.v_attr);
	}	break;

	case Dee_ATTRINFO_INSTANCE_METHOD:
	case Dee_ATTRINFO_INSTANCE_GETSET:
	case Dee_ATTRINFO_INSTANCE_MEMBER:
	case Dee_ATTRINFO_INSTANCE_ATTR: {
		if unlikely(thisarg != self->ad_info.ai_decl) {
			err_bad_instance_access_thisarg(self, thisarg);
			goto err;
		}
		switch (self->ad_info.ai_type) {
		case Dee_ATTRINFO_INSTANCE_METHOD:
		case Dee_ATTRINFO_INSTANCE_GETSET:
		case Dee_ATTRINFO_INSTANCE_MEMBER:
			return Dee_BOUND_YES;
		case Dee_ATTRINFO_INSTANCE_ATTR:
			return DeeClass_BoundInstanceAttribute((DeeTypeObject *)thisarg, self->ad_info.ai_value.v_instance_attr);
		default: __builtin_unreachable();
		}
	}	break;

	default: break;
	}
	err_cant_access_attribute_string((DeeTypeObject *)self->ad_info.ai_decl,
	                                 self->ad_name, ATTR_ACCESS_GET);
err:
	return Dee_BOUND_ERR;
}

PUBLIC WUNUSED NONNULL((1, 2)) int DCALL
Dee_attrdesc_calldel(struct Dee_attrdesc const *self, DeeObject *thisarg) {
	switch (self->ad_info.ai_type) {

	case Dee_ATTRINFO_CUSTOM: {
		struct type_attr const *tp_attr;
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		tp_attr = self->ad_info.ai_value.v_custom;
		if (tp_attr->tp_delattr) {
			int result;
			DREF DeeObject *attrob;
			if (self->ad_perm & Dee_ATTRPERM_F_NAMEOBJ)
				return (*tp_attr->tp_delattr)(thisarg, (DeeObject *)Dee_attrdesc_nameobj(self));
			if (tp_attr->tp_delattr_string_hash)
				return (*tp_attr->tp_delattr_string_hash)(thisarg, self->ad_name, Dee_HashStr(self->ad_name));
			attrob = DeeString_New(self->ad_name);
			if unlikely(!attrob)
				goto err;
			result = (*tp_attr->tp_delattr)(thisarg, attrob);
			Dee_Decref_likely(attrob);
			return result;
		}
	}	break;

	case Dee_ATTRINFO_MODSYM:
		if unlikely(self->ad_info.ai_decl != thisarg) {
			err_bad_module_access_thisarg(self, thisarg);
			goto err;
		}
		return DeeModule_DelAttrSymbol((DeeModuleObject *)thisarg,
		                               self->ad_info.ai_value.v_modsym);

	case Dee_ATTRINFO_METHOD:
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		break;

	case Dee_ATTRINFO_GETSET:
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		if (self->ad_info.ai_value.v_getset->gs_del)
			return (*self->ad_info.ai_value.v_getset->gs_del)(thisarg);
		break;

	case Dee_ATTRINFO_MEMBER:
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		return Dee_type_member_del(self->ad_info.ai_value.v_member, thisarg);

	case Dee_ATTRINFO_ATTR: {
		struct class_desc *desc;
		struct instance_desc *inst;
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		desc = DeeClass_DESC(self->ad_info.ai_decl);
		inst = DeeInstance_DESC(desc, thisarg);
		return DeeInstance_DelAttribute(desc, inst, thisarg, self->ad_info.ai_value.v_attr);
	}	break;

	case Dee_ATTRINFO_INSTANCE_METHOD:
	case Dee_ATTRINFO_INSTANCE_GETSET:
	case Dee_ATTRINFO_INSTANCE_MEMBER:
	case Dee_ATTRINFO_INSTANCE_ATTR: {
		if unlikely(thisarg != self->ad_info.ai_decl) {
			err_bad_instance_access_thisarg(self, thisarg);
			goto err;
		}
		switch (self->ad_info.ai_type) {
		case Dee_ATTRINFO_INSTANCE_METHOD:
		case Dee_ATTRINFO_INSTANCE_GETSET:
		case Dee_ATTRINFO_INSTANCE_MEMBER:
			break;
		case Dee_ATTRINFO_INSTANCE_ATTR:
			return DeeClass_DelInstanceAttribute((DeeTypeObject *)thisarg, self->ad_info.ai_value.v_instance_attr);
		default: __builtin_unreachable();
		}
	}	break;

	default: break;
	}
	err_cant_access_attribute_string((DeeTypeObject *)self->ad_info.ai_decl,
	                                 self->ad_name, ATTR_ACCESS_DEL);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int DCALL
Dee_attrdesc_callset(struct Dee_attrdesc const *self, DeeObject *thisarg, DeeObject *value) {
	switch (self->ad_info.ai_type) {

	case Dee_ATTRINFO_CUSTOM: {
		struct type_attr const *tp_attr;
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		tp_attr = self->ad_info.ai_value.v_custom;
		if (tp_attr->tp_setattr) {
			int result;
			DREF DeeObject *attrob;
			if (self->ad_perm & Dee_ATTRPERM_F_NAMEOBJ)
				return (*tp_attr->tp_setattr)(thisarg, (DeeObject *)Dee_attrdesc_nameobj(self), value);
			if (tp_attr->tp_setattr_string_hash)
				return (*tp_attr->tp_setattr_string_hash)(thisarg, self->ad_name, Dee_HashStr(self->ad_name), value);
			attrob = DeeString_New(self->ad_name);
			if unlikely(!attrob)
				goto err;
			result = (*tp_attr->tp_setattr)(thisarg, attrob, value);
			Dee_Decref_likely(attrob);
			return result;
		}
	}	break;

	case Dee_ATTRINFO_MODSYM:
		if unlikely(self->ad_info.ai_decl != thisarg) {
			err_bad_module_access_thisarg(self, thisarg);
			goto err;
		}
		return DeeModule_SetAttrSymbol((DeeModuleObject *)thisarg,
		                               self->ad_info.ai_value.v_modsym,
		                               value);

	case Dee_ATTRINFO_METHOD:
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		break;

	case Dee_ATTRINFO_GETSET:
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		if (self->ad_info.ai_value.v_getset->gs_set)
			return (*self->ad_info.ai_value.v_getset->gs_set)(thisarg, value);
		break;

	case Dee_ATTRINFO_MEMBER:
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		return Dee_type_member_set(self->ad_info.ai_value.v_member, thisarg, value);

	case Dee_ATTRINFO_ATTR: {
		struct class_desc *desc;
		struct instance_desc *inst;
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		desc = DeeClass_DESC(self->ad_info.ai_decl);
		inst = DeeInstance_DESC(desc, thisarg);
		return DeeInstance_SetAttribute(desc, inst, thisarg, self->ad_info.ai_value.v_attr, value);
	}	break;

	case Dee_ATTRINFO_INSTANCE_METHOD:
	case Dee_ATTRINFO_INSTANCE_GETSET:
	case Dee_ATTRINFO_INSTANCE_MEMBER:
	case Dee_ATTRINFO_INSTANCE_ATTR: {
		if unlikely(thisarg != self->ad_info.ai_decl) {
			err_bad_instance_access_thisarg(self, thisarg);
			goto err;
		}
		switch (self->ad_info.ai_type) {
		case Dee_ATTRINFO_INSTANCE_METHOD:
		case Dee_ATTRINFO_INSTANCE_GETSET:
		case Dee_ATTRINFO_INSTANCE_MEMBER:
			break;
		case Dee_ATTRINFO_INSTANCE_ATTR:
			return DeeClass_SetInstanceAttribute((DeeTypeObject *)thisarg, self->ad_info.ai_value.v_instance_attr, value);
		default: __builtin_unreachable();
		}
	}	break;

	default: break;
	}
	err_cant_access_attribute_string((DeeTypeObject *)self->ad_info.ai_decl,
	                                 self->ad_name, ATTR_ACCESS_SET);
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Dee_attrdesc_callcall(struct Dee_attrdesc const *self, DeeObject *thisarg,
                      size_t argc, DeeObject *const *argv, DeeObject *kw) {
again:
	switch (self->ad_info.ai_type) {

	case Dee_ATTRINFO_CUSTOM: {
		struct type_attr const *tp_attr;
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		tp_attr = self->ad_info.ai_value.v_custom;
		if (tp_attr->tp_getattr) {
			DREF DeeObject *callable;
			if (self->ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
				if (tp_attr->tp_callattr_kw)
					return (*tp_attr->tp_callattr_kw)(thisarg, (DeeObject *)Dee_attrdesc_nameobj(self), argc, argv, kw);
				if (tp_attr->tp_callattr) {
					if (kw)
						goto assert_no_kw;
					return (*tp_attr->tp_callattr)(thisarg, (DeeObject *)Dee_attrdesc_nameobj(self), argc, argv);
				}
			}
			if (tp_attr->tp_callattr_string_hash_kw)
				return (*tp_attr->tp_callattr_string_hash_kw)(thisarg, self->ad_name, Dee_HashStr(self->ad_name), argc, argv, kw);
			if (tp_attr->tp_callattr_string_hash) {
				if (kw)
					goto assert_no_kw;
				return (*tp_attr->tp_callattr_string_hash)(thisarg, self->ad_name, Dee_HashStr(self->ad_name), argc, argv);
			}
			if (self->ad_perm & Dee_ATTRPERM_F_NAMEOBJ) {
				callable = (*tp_attr->tp_getattr)(thisarg, (DeeObject *)Dee_attrdesc_nameobj(self));
			} else if (tp_attr->tp_getattr_string_hash) {
				callable = (*tp_attr->tp_getattr_string_hash)(thisarg, self->ad_name, Dee_HashStr(self->ad_name));
			} else {
				DREF DeeObject *attrob;
				attrob = DeeString_New(self->ad_name);
				if unlikely(!attrob)
					goto err;
				callable = (*tp_attr->tp_getattr)(thisarg, attrob);
				Dee_Decref_likely(attrob);
			}
			if unlikely(!callable)
				goto err;
			return DeeObject_CallKwInherited(callable, argc, argv, kw);
		}
	}	break;

	case Dee_ATTRINFO_MODSYM:
	case Dee_ATTRINFO_GETSET:
	case Dee_ATTRINFO_MEMBER: {
		DREF DeeObject *callable;
		callable = Dee_attrdesc_callget(self, thisarg);
		if unlikely(!callable)
			goto err;
		return DeeObject_CallKwInherited(callable, argc, argv, kw);
	}	break;

	case Dee_ATTRINFO_METHOD:
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		if (self->ad_info.ai_value.v_method->m_flag & Dee_TYPE_METHOD_FKWDS) {
			return DeeKwObjMethod_CallFunc((Dee_kwobjmethod_t)self->ad_info.ai_value.v_method->m_func,
			                               thisarg, argc, argv, kw);
		}
		if (kw)
			goto assert_no_kw;
		return DeeObjMethod_CallFunc(self->ad_info.ai_value.v_method->m_func, thisarg, argc, argv);

	case Dee_ATTRINFO_ATTR: {
		struct class_desc *desc;
		struct instance_desc *inst;
		if (DeeObject_AssertTypeOrAbstract(thisarg, (DeeTypeObject *)self->ad_info.ai_decl))
			goto err;
		desc = DeeClass_DESC(self->ad_info.ai_decl);
		inst = DeeInstance_DESC(desc, thisarg);
		return DeeInstance_CallAttributeKw(desc, inst, thisarg, self->ad_info.ai_value.v_attr, argc, argv, kw);
	}	break;

	case Dee_ATTRINFO_INSTANCE_METHOD:
	case Dee_ATTRINFO_INSTANCE_GETSET:
	case Dee_ATTRINFO_INSTANCE_MEMBER:
	case Dee_ATTRINFO_INSTANCE_ATTR: {
		DeeObject *real_thisarg;
		if unlikely(thisarg != self->ad_info.ai_decl) {
			err_bad_instance_access_thisarg(self, thisarg);
			goto err;
		}
		if unlikely(!argc) {
			err_invalid_argc_va(self->ad_name, 0, 1);
			goto err;
		}
		real_thisarg = *argv++;
		--argc;
		if (DeeObject_AssertTypeOrAbstract(real_thisarg, (DeeTypeObject *)thisarg))
			goto err;
		switch (self->ad_info.ai_type) {
		case Dee_ATTRINFO_INSTANCE_METHOD:
			if (self->ad_info.ai_value.v_instance_method->m_flag & Dee_TYPE_METHOD_FKWDS) {
				return DeeKwObjMethod_CallFunc((Dee_kwobjmethod_t)self->ad_info.ai_value.v_instance_method->m_func,
				                               real_thisarg, argc, argv, kw);
			}
			return DeeObjMethod_CallFunc(self->ad_info.ai_value.v_instance_method->m_func,
			                             real_thisarg, argc, argv);
		case Dee_ATTRINFO_INSTANCE_GETSET: {
			struct type_getset const *gs = self->ad_info.ai_value.v_instance_getset;
			if unlikely(argc) {
				err_invalid_argc(self->ad_name, argc + 1, 1, 1);
				goto err;
			}
			if (gs->gs_get)
				return (*gs->gs_get)(real_thisarg);
		}	break;
		case Dee_ATTRINFO_INSTANCE_MEMBER: {
			if unlikely(argc) {
				err_invalid_argc(self->ad_name, argc + 1, 1, 1);
				goto err;
			}
			return Dee_type_member_get(self->ad_info.ai_value.v_instance_member, real_thisarg);
		}	break;
		case Dee_ATTRINFO_INSTANCE_ATTR:
			return DeeClass_CallInstanceAttributeKw((DeeTypeObject *)thisarg, self->ad_info.ai_value.v_instance_attr,
			                                        argc, argv, kw);
		default: __builtin_unreachable();
		}
	}	break;

	default: break;
	}
	err_cant_access_attribute_string((DeeTypeObject *)self->ad_info.ai_decl,
	                                 self->ad_name, ATTR_ACCESS_GET);
err:
	return NULL;
assert_no_kw:
	ASSERT(kw);
	if (DeeKwds_Check(kw)) {
		if (DeeKwds_SIZE(kw) != 0)
			goto err_no_keywords;
	} else {
		size_t temp = DeeObject_Size(kw);
		if unlikely(temp == (size_t)-1)
			goto err;
		if (temp != 0)
			goto err_no_keywords;
	}
	kw = NULL;
	goto again;
err_no_keywords:
	DeeError_Throwf(&DeeError_TypeError,
	                "Attribute `%r.%s' does not accept keyword arguments %r",
	                (DeeTypeObject *)self->ad_info.ai_decl, self->ad_name, kw);
	goto err;
}








/* Initialize a buffer for yielding empty (no) attributes. */
#define empty_attriter_next _DeeNone_reti1_2
PRIVATE struct Dee_attriter_type tpconst empty_attriter_type = {
	/* .ait_next = */ (int (DCALL *)(struct Dee_attriter *__restrict, /*out*/ struct Dee_attrdesc *__restrict))&empty_attriter_next,
};

PUBLIC WUNUSED size_t DCALL
Dee_attriter_initempty(struct Dee_attriter *iterbuf, size_t bufsize) {
	if likely(bufsize >= sizeof(struct Dee_attriter))
		Dee_attriter_init(iterbuf, &empty_attriter_type);
	return sizeof(struct Dee_attriter);
}


/* Test if more items can be enumerated from "self" */
PUBLIC WUNUSED NONNULL((1)) int DCALL
Dee_attriter_bool(struct Dee_attriter *__restrict self, size_t selfsize) {
	int result;
	struct Dee_attrdesc desc;
	struct Dee_attriter *copy;
	copy = (struct Dee_attriter *)Dee_Malloca(selfsize);
	if unlikely(!copy)
		goto err;
	if unlikely(Dee_attriter_copy(copy, self, selfsize))
		goto err_copy;
	result = Dee_attriter_next(copy, &desc);
	Dee_attriter_fini(copy);
	Dee_Freea(copy);
	if (result == 0) {
		Dee_attrdesc_fini(&desc);
		result = 1;
	} else if (result > 0) {
		result = 0;
	}
	return result;
err_copy:
	Dee_Freea(copy);
err:
	return -1;
}








/* Wrapper around `DeeObject_IterAttr()' that will handle all the instantiation/finalization of the
 * attribute iterator for you, whilst applying a given `filter' to select which arguments should
 * actually be enumerated. The given `cb' may also return any negative value to halt enumeration
 * prematurely (in which case that same negative value is returned by this function), or return a
 * positive value, which are then summed across all invocations prior to being returned.
 * @param: filter: Only enumerate attributes matched by this filter
 * @return: >= 0: Success (return value is the sum of invocations of `cb')
 * @return: -1:   An error was thrown
 * @return: < -1: Negative return value returned by `cb' */
PUBLIC WUNUSED NONNULL((1, 3, 4, 5)) Dee_ssize_t DCALL
DeeObject_EnumAttr(DeeTypeObject *tp_self, DeeObject *self,
                   struct Dee_attrhint const *__restrict filter,
                   Dee_enumattr_t cb, void *arg) {
#undef DeeObject_EnumAttr_USES_MALLOCA
#undef DeeObject_EnumAttr_USES_MALLOC
#if DEE_MALLOCA_MAX >= Dee_ITERATTR_DEFAULT_BUFSIZE
#define DeeObject_EnumAttr_USES_MALLOCA
#else /* DEE_MALLOCA_MAX >= Dee_ITERATTR_DEFAULT_BUFSIZE */
#define DeeObject_EnumAttr_USES_MALLOC
#endif /* DEE_MALLOCA_MAX < Dee_ITERATTR_DEFAULT_BUFSIZE */
	int status;
	struct Dee_attrdesc desc;
	Dee_ssize_t temp, result = 0;
	size_t reqsiz, bufsiz = Dee_ITERATTR_DEFAULT_BUFSIZE;
	struct Dee_attriter *iterbuf;
#ifdef DeeObject_EnumAttr_USES_MALLOCA
again_alloc_iterbuf:
	iterbuf = (struct Dee_attriter *)Dee_Malloca(bufsiz);
#endif /* DeeObject_EnumAttr_USES_MALLOCA */
#ifdef DeeObject_EnumAttr_USES_MALLOC
	iterbuf = (struct Dee_attriter *)Dee_Malloc(bufsiz);
#endif /* DeeObject_EnumAttr_USES_MALLOC */
	if unlikely(!iterbuf)
		goto err;
#ifdef DeeObject_EnumAttr_USES_MALLOC
again_iterattr:
#endif /* DeeObject_EnumAttr_USES_MALLOC */
	reqsiz = DeeObject_IterAttr(tp_self, self, iterbuf, bufsiz, filter);
	if unlikely(reqsiz == (size_t)-1)
		goto err_iterbuf;
	if (reqsiz > bufsiz) {
#ifdef DeeObject_EnumAttr_USES_MALLOCA
		Dee_Freea(iterbuf);
		bufsiz = reqsiz;
		goto again_alloc_iterbuf;
#endif /* DeeObject_EnumAttr_USES_MALLOCA */
#ifdef DeeObject_EnumAttr_USES_MALLOC
		struct Dee_attriter *new_iterbuf;
		new_iterbuf = (struct Dee_attriter *)Dee_Realloc(iterbuf, bufsiz);
		if unlikely(!new_iterbuf)
			goto err_iterbuf;
		iterbuf = new_iterbuf;
		goto again_iterattr;
#endif /* DeeObject_EnumAttr_USES_MALLOC */
	}

	/* Enumerate attributes */
	while ((status = Dee_attriter_next(iterbuf, &desc)) == 0) {
		temp = Dee_attrhint_matches(filter, &desc) ? (*cb)(arg, &desc) : 0;
		Dee_attrdesc_fini(&desc);
		if unlikely(temp < 0)
			goto err_temp_iterbuf_fini;
		result += temp;
	}
	if unlikely(status < 0)
		goto err_iterbuf;
	Dee_attriter_fini(iterbuf);
#ifdef DeeObject_EnumAttr_USES_MALLOCA
	Dee_Freea(iterbuf);
#endif /* DeeObject_EnumAttr_USES_MALLOCA */
#ifdef DeeObject_EnumAttr_USES_MALLOC
	Dee_Free(iterbuf);
#endif /* DeeObject_EnumAttr_USES_MALLOC */
	return result;
err_temp_iterbuf_fini:
	Dee_attriter_fini(iterbuf);
err_temp_iterbuf:
#ifdef DeeObject_EnumAttr_USES_MALLOCA
	Dee_Freea(iterbuf);
#endif /* DeeObject_EnumAttr_USES_MALLOCA */
#ifdef DeeObject_EnumAttr_USES_MALLOC
	Dee_Free(iterbuf);
#endif /* DeeObject_EnumAttr_USES_MALLOC */
err_temp:
	return temp;
err:
	temp = -1;
	goto err_temp;
err_iterbuf:
	temp = -1;
	goto err_temp_iterbuf;
}







/************************************************************************/
/* Special built-in attribute iterator for concating multiple others.   */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
Dee_attriterchain_next(struct Dee_attriterchain *__restrict self,
                       /*out*/ struct Dee_attrdesc *__restrict desc) {
	for (;;) {
		int result;
		bool writeok;
		struct Dee_attriter *current, *next;
		struct Dee_attriterchain_item *current_item;
		current = atomic_read(&self->aic_current);
		if (!current)
			return 1;
		result = Dee_attriter_next(current, desc);
		if (result <= 0)
			return result;

		/* Move on to the next iterator. */
		if (Dee_shared_rwlock_write(&self->aic_curlock))
			goto err;
		current_item = Dee_attriterchain_item_fromiter(current);
		next         = current_item->aici_next;
		writeok = atomic_cmpxch_or_write(&self->aic_current, current, next);
		Dee_shared_rwlock_endwrite(&self->aic_curlock);
		if (writeok)
			Dee_attriter_fini(current);
	}
	__builtin_unreachable();
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
Dee_attriterchain_fini(struct Dee_attriterchain *__restrict self) {
	struct Dee_attriter *iter;
	for (iter = self->aic_current; iter != NULL;
	     iter = Dee_attriterchain_item_fromiter(iter)->aici_next)
		Dee_attriter_fini(iter);
}

PRIVATE NONNULL((1, 2)) void DCALL
Dee_attriterchain_visit(struct Dee_attriterchain *__restrict self,
                        Dee_visit_t proc, void *arg) {
	struct Dee_attriter *iter;
	Dee_shared_rwlock_read_noint(&self->aic_curlock);
	for (iter = self->aic_current; iter != NULL;
	     iter = Dee_attriterchain_item_fromiter(iter)->aici_next)
		Dee_attriter_visit(iter);
	Dee_shared_rwlock_endread(&self->aic_curlock);
}

PRIVATE NONNULL((1)) void DCALL
Dee_attriterchain_moved(struct Dee_attriterchain *__restrict self, ptrdiff_t delta) {
#define ADJUST_PTR(T, ptr_lvalue) (void)((ptr_lvalue) = (T *)((byte_t *)(ptr_lvalue) + delta))
	struct Dee_attriter *iter;
	if unlikely(!self->aic_current)
		return;
	ADJUST_PTR(struct Dee_attriter, self->aic_current);
	for (iter = self->aic_current;;) {
		struct Dee_attriterchain_item *item;
		Dee_attriter_moved(iter, delta);
		item = Dee_attriterchain_item_fromiter(iter);
		if (!item->aici_next)
			break;
		ADJUST_PTR(struct Dee_attriter, item->aici_next);
		iter = item->aici_next;
	}
#undef ADJUST_PTR
}

PRIVATE NONNULL((1, 2)) int DCALL
Dee_attriterchain_copy(struct Dee_attriterchain *__restrict self,
                       struct Dee_attriterchain *__restrict other,
                       size_t other_bufsize) {
	struct Dee_attriter *src_iter;
	struct Dee_attriter *dst_iter;
	struct Dee_attriterchain_item *src_item;
	struct Dee_attriterchain_item *dst_item;
	struct Dee_attriter **p_dst_item;
	Dee_shared_rwlock_init(&self->aic_curlock);
	if (Dee_shared_rwlock_read(&other->aic_curlock))
		goto err;
	src_iter = atomic_read(&other->aic_current);
	if unlikely(!src_iter) {
		Dee_shared_rwlock_endread(&other->aic_curlock);
		self->aic_current = NULL;
		return 0;
	}
	dst_iter = &self->aic_first.aici_iter;
	p_dst_item = &self->aic_current;
	do {
		int temp;
		size_t src_iter_bufsize;
		src_item = Dee_attriterchain_item_fromiter(src_iter);
		dst_item = Dee_attriterchain_item_fromiter(dst_iter);
		if (src_item->aici_next) {
			src_iter_bufsize = (size_t)((byte_t *)src_item->aici_next -
			                            (byte_t *)(&src_item->aici_iter));
			ASSERT(src_iter_bufsize >= offsetof(struct Dee_attriterchain_item, aici_iter));
			src_iter_bufsize -= offsetof(struct Dee_attriterchain_item, aici_iter);
		} else {
			src_iter_bufsize = (size_t)(((byte_t *)other + other_bufsize) -
			                            (byte_t *)(&src_item->aici_iter));
		}

		/* Copy this singular iterator. */
		temp = Dee_attriter_copy(dst_iter, src_iter, src_iter_bufsize);
		if unlikely(temp) {
			Dee_shared_rwlock_endread(&other->aic_curlock);
			*p_dst_item = NULL;
			Dee_attriterchain_fini(self);
			return temp;
		}

		/* Link into the chain of copied iterators. */
		*p_dst_item = dst_iter;
		p_dst_item = &dst_item->aici_next;
		dst_iter = (struct Dee_attriter *)((byte_t *)dst_iter +
		                                   offsetof(struct Dee_attriterchain_item, aici_iter) +
		                                   src_iter_bufsize);
	} while ((src_iter = src_item->aici_next) != NULL);
	Dee_shared_rwlock_endread(&other->aic_curlock);
	*p_dst_item = NULL;
	return 0;
err:
	return -1;
}

PUBLIC_TPCONST struct Dee_attriter_type tpconst Dee_attriterchain_type = {
	/* .ait_next  = */ (int (DCALL *)(struct Dee_attriter *__restrict, /*out*/ struct Dee_attrdesc *__restrict))&Dee_attriterchain_next,
	/* .ait_copy  = */ (int (DCALL *)(struct Dee_attriter *__restrict, struct Dee_attriter *__restrict, size_t))&Dee_attriterchain_copy,
	/* .ait_fini  = */ (void (DCALL *)(struct Dee_attriter *__restrict))&Dee_attriterchain_fini,
	/* .ait_visit = */ (void (DCALL *)(struct Dee_attriter *__restrict, Dee_visit_t, void *))&Dee_attriterchain_visit,
	/* .ait_moved = */ (void (DCALL *)(struct Dee_attriter *__restrict, ptrdiff_t))&Dee_attriterchain_moved,
};


/* Initialize a builder for use with a given "iterbuf" and "bufsize" */
PUBLIC NONNULL((1)) void DCALL
Dee_attriterchain_builder_init(struct Dee_attriterchain_builder *__restrict self,
                               struct Dee_attriter *iterbuf, size_t bufsize) {
	struct Dee_attriterchain *iterchain = (struct Dee_attriterchain *)iterbuf;
	self->aicb_curiter = &iterchain->aic_first.aici_iter;
	self->aicb_require = offsetof(struct Dee_attriterchain, aic_first.aici_iter);
	if (!OVERFLOW_USUB(bufsize, offsetof(struct Dee_attriterchain, aic_first.aici_iter),
	                   &self->aicb_bufsize)) {
		DBG_memset(&iterchain->aic_current, 0xcc, sizeof(iterchain->aic_current));
		Dee_shared_rwlock_init(&iterchain->aic_curlock);
		Dee_attriter_init(iterchain, &Dee_attriterchain_type);
		self->aicb_iterchain = iterchain;
		self->aicb_pnext = &self->aicb_iterchain->aic_current;
	} else {
		self->aicb_bufsize = 0;
		self->aicb_pnext   = NULL;
	}
}

/* Finalize any iterator that had been constructed by the builder. */
PUBLIC NONNULL((1)) void DCALL
Dee_attriterchain_builder_fini(struct Dee_attriterchain_builder *__restrict self) {
	if (self->aicb_pnext) {
		*self->aicb_pnext = NULL;
		Dee_attriterchain_fini(self->aicb_iterchain);
	}
	DBG_memset(self, 0xcc, sizeof(*self));
}


/* Consume "n_bytes" from the builder's buffer and advance pointers. */
PUBLIC NONNULL((1)) void DCALL
Dee_attriterchain_builder_consume(struct Dee_attriterchain_builder *__restrict self,
                                  size_t n_bytes) {
	size_t old_bufsize = Dee_attriterchain_builder_getbufsize(self);
	size_t aligned_size = n_bytes;
	/* Account for extra space needed for the next chain element's header */
	aligned_size += offsetof(struct Dee_attriterchain_item, aici_iter);
	aligned_size += Dee_ALIGNOF_ATTRITER - 1;
	aligned_size &= ~(Dee_ALIGNOF_ATTRITER - 1);
	if (!OVERFLOW_USUB(self->aicb_bufsize, aligned_size, &self->aicb_bufsize)) {
		/* Was able to successfully write this item (and have enough space to write the next one). */
		struct Dee_attriterchain_item *curitem;
		ASSERT(self->aicb_pnext);
		curitem = COMPILER_CONTAINER_OF(self->aicb_curiter, struct Dee_attriterchain_item, aici_iter);
		if (curitem->aici_iter.ai_type == &Dee_attriterchain_type) {
			/* TODO: Optimize for this case by inlining the iterators of the inner chain */
		}
		*self->aicb_pnext  = self->aicb_curiter; /* Link in the newly initialize iterator */
		self->aicb_pnext   = &curitem->aici_next;
		self->aicb_curiter = (struct Dee_attriter *)((byte_t *)self->aicb_curiter + aligned_size);
	} else {
		if unlikely(n_bytes <= old_bufsize) {
			/* Must finalize the iterator because the was written,
			 * but now the buffer is too small for the header of
			 * the next item. */
			Dee_attriter_fini(self->aicb_curiter);
		}
		if (self->aicb_pnext) {
			/* Must finalize all iterators previously initialized */
			*self->aicb_pnext = NULL;
			self->aicb_pnext = NULL;
			Dee_attriterchain_fini(self->aicb_iterchain);
		}
		self->aicb_bufsize = 0;
	}
	self->aicb_require += aligned_size;
}


PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) struct type_method const *DCALL
locate_type_method(struct type_method const *__restrict chain, char const *name_ptr) {
	for (;; ++chain) {
		ASSERTF(chain->m_name, "Failed to locate %p:%q in chain", name_ptr, name_ptr);
		if (chain->m_name == name_ptr)
			return chain;
	}
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) struct type_getset const *DCALL
locate_type_getset(struct type_getset const *__restrict chain, char const *name_ptr) {
	for (;; ++chain) {
		ASSERTF(chain->gs_name, "Failed to locate %p:%q in chain", name_ptr, name_ptr);
		if (chain->gs_name == name_ptr)
			return chain;
	}
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) struct type_member const *DCALL
locate_type_member(struct type_member const *__restrict chain, char const *name_ptr) {
	for (;; ++chain) {
		ASSERTF(chain->m_name, "Failed to locate %p:%q in chain", name_ptr, name_ptr);
		if (chain->m_name == name_ptr)
			return chain;
	}
}





#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_lock_t membercache_list_lock = DEE_ATOMIC_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */

#define membercache_list_lock_available()  Dee_atomic_lock_available(&membercache_list_lock)
#define membercache_list_lock_acquired()   Dee_atomic_lock_acquired(&membercache_list_lock)
#define membercache_list_lock_tryacquire() Dee_atomic_lock_tryacquire(&membercache_list_lock)
#define membercache_list_lock_acquire()    Dee_atomic_lock_acquire(&membercache_list_lock)
#define membercache_list_lock_waitfor()    Dee_atomic_lock_waitfor(&membercache_list_lock)
#define membercache_list_lock_release()    Dee_atomic_lock_release(&membercache_list_lock)

LIST_HEAD(membercache_list_struct, Dee_membercache);

/* [0..1][lock(membercache_list_lock)]
 * Linked list of all existing type-member caches. */
PRIVATE struct membercache_list_struct
membercache_list = LIST_HEAD_INITIALIZER(membercache_list);

#define streq(a, b)            (strcmp(a, b) == 0)
#define streq_len(a, b, b_len) (strcmpz(a, b, b_len) == 0)

/* Finalize a given member-cache. */
INTERN NONNULL((1)) void DCALL
Dee_membercache_fini(struct Dee_membercache *__restrict self) {
	if (LIST_ISBOUND(self, mc_link)) {
		membercache_list_lock_acquire();
		if (LIST_ISBOUND(self, mc_link))
			LIST_REMOVE(self, mc_link);
		membercache_list_lock_release();
	}
	Dee_Free(self->mc_table);
	DBG_memset(self, 0xcc, sizeof(*self));
}

INTERN size_t DCALL
Dee_membercache_clearall(size_t max_clear) {
	size_t result = 0;
	struct Dee_membercache *cache;
again:
	membercache_list_lock_acquire();
	if (LIST_EMPTY(&membercache_list)) {
		membercache_list_lock_release();
	} else {
		DREF struct Dee_membercache_table *table;
		cache = LIST_FIRST(&membercache_list);

		/* Steal the table and wait for everyone to stop using it. */
		table = atomic_xch(&cache->mc_table, NULL);
		Dee_membercache_waitfor(cache);

		/* Pop this entry from the global chain. */
		ASSERT(LIST_ISBOUND(cache, mc_link));
		LIST_UNBIND(cache, mc_link);
		membercache_list_lock_release();

		if (table) {
			/* Track how much member this operation will be freeing up. */
			result += (table->mc_mask + 1) * sizeof(struct Dee_membercache);

			/* Drop the table reference that was held by the type. */
			Dee_membercache_table_decref(table);
		}

		if (result < max_clear)
			goto again;
	}
	return result;
}


/* Try to allocate a new member-cache table. */
#define Dee_membercache_table_trycalloc(mask)                                                         \
	(struct Dee_membercache_table *)Dee_TryCallococ(offsetof(struct Dee_membercache_table, mc_table), \
	                                                (mask) + 1, sizeof(struct Dee_membercache_slot))
STATIC_ASSERT(MEMBERCACHE_UNUSED == 0);

PRIVATE NONNULL((1, 2)) void DCALL
Dee_membercache_table_do_addslot(struct Dee_membercache_table *__restrict self,
                                 struct Dee_membercache_slot const *__restrict item) {
	dhash_t i, perturb;
	struct Dee_membercache_slot *slot;
	perturb = i = Dee_membercache_table_hashst(self, item->mcs_hash);
	for (;; Dee_membercache_table_hashnx(i, perturb)) {
		slot = Dee_membercache_table_hashit(self, i);
		if (slot->mcs_type == MEMBERCACHE_UNUSED)
			break;
		ASSERTF(slot->mcs_type != MEMBERCACHE_UNINITIALIZED,
		        "We're building a new cache-table, so how can "
		        "that new table contain UNINITIALIZED items?");
	}

	/* Found a free slot -> write to it! */
	memcpy(slot, item, sizeof(struct Dee_membercache_slot));
	++self->mc_size;
}

/* Try to construct a new member-cache table */
PRIVATE WUNUSED NONNULL((2)) DREF struct Dee_membercache_table *DCALL
Dee_membercache_table_new(struct Dee_membercache_table const *old_table,
                          struct Dee_membercache_slot const *__restrict item) {
	DREF struct Dee_membercache_table *result;
	size_t new_mask = 0;
	if (old_table != NULL)
		new_mask = old_table->mc_mask;
	new_mask = (new_mask << 1) | 1;
	if unlikely(new_mask == 1)
		new_mask = 16 - 1; /* Start out bigger than 2. */
	result = Dee_membercache_table_trycalloc(new_mask);
	if unlikely(!result)
		return NULL;

	/* Fill in the basic characteristics of the new cache-table. */
	result->mc_mask   = new_mask;
	result->mc_refcnt = 1;
	ASSERTF(result->mc_size == 0, "Should be the case because of calloc()");

	/* Migrate the old cache-table into the new one. */
	if (old_table != NULL) {
		size_t i;
		for (i = 0; i <= old_table->mc_mask; ++i) {
			uint16_t type;
			struct Dee_membercache_slot const *slot;
			slot = &old_table->mc_table[i];
			type = atomic_read(&slot->mcs_type);
			if (type == MEMBERCACHE_UNUSED)
				continue;
			if (type == MEMBERCACHE_UNINITIALIZED)
				continue; /* Don't migrate slots that aren't fully initialized. */

			/* Keep this cache-slot as part of the resulting cache-table. */
			Dee_membercache_table_do_addslot(result, slot);
		}
	}

	/* Add the caller's new item. */
	Dee_membercache_table_do_addslot(result, item);
	return result;
}


/* Try to add a slot to the given member-cache table.
 * @return:  2: Slot is being initialized by a different thread (not added)
 * @return:  1: Slot already exists in cache (not added)
 * @return:  0: Success
 * @return: -1: No more free slots (caller must allocate a new member-cache
 *              table, or try with `allow_bad_hash_ratios = true') */
PRIVATE NONNULL((1, 2)) int DCALL
Dee_membercache_table_addslot(struct Dee_membercache_table *__restrict self,
                              struct Dee_membercache_slot const *__restrict item,
                              bool allow_bad_hash_ratios) {
	dhash_t i, perturb;
	struct Dee_membercache_slot *slot;

	/* Try to allocate a slot within the table. */
	for (;;) {
		size_t size = atomic_read(&self->mc_size);
		if (allow_bad_hash_ratios ? (size * 1) >= self->mc_mask
		                          : (size * 2) >= self->mc_mask)
			return -1; /* You should (or need to) allocate a new table. */
		if (atomic_cmpxch_or_write(&self->mc_size, size, size + 1))
			break;
	}

	/* Re-check that the named attribute isn't already in-cache. */
again_search_slots:
	perturb = i = Dee_membercache_table_hashst(self, item->mcs_hash);
	for (;; Dee_membercache_table_hashnx(i, perturb)) {
		uint16_t type;
		slot = Dee_membercache_table_hashit(self, i);
		type = atomic_read(&slot->mcs_type);
		if (type == MEMBERCACHE_UNUSED)
			break;

#ifndef CONFIG_NO_THREADS
		if (type == MEMBERCACHE_UNINITIALIZED) {
			/* Encountered an uninitialized slot along the cache-chain.
			 *
			 * This means that some other thread is currently writing an
			 * item into the cache-table that has a similar hash to the
			 * item that we're writing right now.
			 *
			 * This means that there is a chance that the other thread
			 * is caching the same item as we are, and we have to make
			 * such that no member is cached twice within the same table,
			 * so to prevent any chance of that happening, we have to
			 * assume the worst, and believe that the other thread _is_
			 * trying to cache the same item as we are.
			 *
			 * As such, the only course of action we have left is to tell
			 * our caller that their new element is already cached, even
			 * if that *may* not actually be the case. */
			atomic_dec(&self->mc_size);
			return 2;
		}
#endif /* !CONFIG_NO_THREADS */

		if (slot->mcs_hash != item->mcs_hash)
			continue;
		if (slot->mcs_method.m_name != item->mcs_name &&
		    !streq(slot->mcs_name, item->mcs_name))
			continue;

		/* Already in cache!
		 * -> Free the slot we allocated above and indicate success to our caller. */
		atomic_dec(&self->mc_size);
		return 1;
	}

	/* Not found. - Try to allocate this empty slot.
	 * If it's no longer empty, start over */
	if (!atomic_cmpxch_or_write(&slot->mcs_type,
	                            MEMBERCACHE_UNUSED,
	                            MEMBERCACHE_UNINITIALIZED))
		goto again_search_slots;

	/* At this point, we've successfully allocated a slot within the cache-table.
	 * With that in mind, we must now fill in that slot, but we have to be careful
	 * about the order here, in that we _MUST_ fill in the type-field LAST!
	 *
	 * This is because by setting the type-field to something other than
	 * `MEMBERCACHE_UNINITIALIZED', we essentially commit the new slot
	 * into the cache. */
	memcpy((byte_t *)slot + COMPILER_OFFSETAFTER(struct Dee_membercache_slot, mcs_type),
	       (byte_t *)item + COMPILER_OFFSETAFTER(struct Dee_membercache_slot, mcs_type),
	       sizeof(struct Dee_membercache_slot) -
	       COMPILER_OFFSETAFTER(struct Dee_membercache_slot, mcs_type));
	COMPILER_WRITE_BARRIER(); /* The type-field must be written last! */
	atomic_write(&slot->mcs_type, item->mcs_type);
	return 0;
}


#ifndef Dee_DPRINT_IS_NOOP
PRIVATE char const membercache_type_names[][16] = {
	/* [MEMBERCACHE_UNUSED         ] = */ "??UNUSED",
	/* [MEMBERCACHE_UNINITIALIZED  ] = */ "??UNINITIALIZED",
	/* [MEMBERCACHE_METHOD         ] = */ "method",
	/* [MEMBERCACHE_GETSET         ] = */ "getset",
	/* [MEMBERCACHE_MEMBER         ] = */ "member",
	/* [MEMBERCACHE_ATTRIB         ] = */ "attrib",
	/* [MEMBERCACHE_INSTANCE_METHOD] = */ "instance_method",
	/* [MEMBERCACHE_INSTANCE_GETSET] = */ "instance_getset",
	/* [MEMBERCACHE_INSTANCE_MEMBER] = */ "instance_member",
	/* [MEMBERCACHE_INSTANCE_ATTRIB] = */ "instance_attrib",
};

#define PRIVATE_IS_KNOWN_TYPETYPE(x) \
	((x) == &DeeType_Type || (x) == &DeeFileType_Type)
#define MEMBERCACHE_GETTYPENAME(x)                                                                 \
	(PRIVATE_IS_KNOWN_TYPETYPE(COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_cache)->ob_type)         \
	 ? COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_cache)->tp_name                                  \
	 : PRIVATE_IS_KNOWN_TYPETYPE(COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_class_cache)->ob_type) \
	   ? COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_class_cache)->tp_name                          \
	   : "?")
#define MEMBERCACHE_GETCLASSNAME(x)                                                                \
	(PRIVATE_IS_KNOWN_TYPETYPE(COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_cache)->ob_type)         \
	 ? "tp_cache"                                                                                  \
	 : PRIVATE_IS_KNOWN_TYPETYPE(COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_class_cache)->ob_type) \
	   ? "tp_class_cache"                                                                          \
	   : "?")


PRIVATE NONNULL((1, 2)) void DCALL
Dee_membercache_addslot_log_success(struct Dee_membercache *__restrict self,
                                    struct Dee_membercache_slot const *__restrict slot) {
	Dee_DPRINTF("[RT] Cached %s `%s.%s' in `%s' (%s)\n",
	            membercache_type_names[slot->mcs_type],
	            slot->mcs_decl->tp_name, slot->mcs_attrib.a_name,
	            MEMBERCACHE_GETTYPENAME(self),
	            MEMBERCACHE_GETCLASSNAME(self));
}
#else /* !Dee_DPRINT_IS_NOOP */
#define Dee_membercache_addslot_log_success(self, slot) (void)0
#endif /* Dee_DPRINT_IS_NOOP */

PRIVATE NONNULL((1, 2)) int DCALL
Dee_membercache_addslot(struct Dee_membercache *__restrict self,
                        struct Dee_membercache_slot const *__restrict slot) {
	DREF struct Dee_membercache_table *old_table;
	DREF struct Dee_membercache_table *new_table;

	/* Get a reference to the current table. */
	Dee_membercache_tabuse_inc(self);
	old_table = atomic_read(&self->mc_table);
	if (old_table != NULL) {
		int status;
		Dee_membercache_table_incref(old_table);
		Dee_membercache_tabuse_dec(self);

		/* Try to add the slot to an existing cache-table. */
do_operate_with_old_table:
		status = Dee_membercache_table_addslot(old_table, slot, false);
		if (status >= 0) {
			Dee_membercache_table_decref(old_table);
			return status;
		}
	} else {
		Dee_membercache_tabuse_dec(self);
	}

	/* Either there isn't a table, or we're unable to add more items
	 * to the table. In either case, try to construct a new table! */
	new_table = Dee_membercache_table_new(old_table, slot);
	if (!new_table) {
		/* Failed to create a new table -> try again to add to the
		 * existing table, but ignore bad hash characteristics this
		 * time around. */
		int result = -1;
		if (old_table != NULL) {
			/* It doesn't matter if this addslot() call succeeds or not... */
			result = Dee_membercache_table_addslot(old_table, slot, true);
			Dee_membercache_table_decref(old_table);
#ifndef Dee_DPRINT_IS_NOOP
			if (result == 0)
				Dee_membercache_addslot_log_success(self, slot);
#endif /* !Dee_DPRINT_IS_NOOP */
		}
		return result;
	}

	/* Drop our original reference to the old table. */
	if (old_table != NULL)
		Dee_membercache_table_decref(old_table);

	/* Got a new table -> now to store it within the type.
	 * But: do one more check if the currently set table
	 *      might actually be better than our `new_table' */
	if (atomic_read(&self->mc_table) != old_table) {
		Dee_membercache_tabuse_inc(self);
		old_table = atomic_read(&self->mc_table);
		if (old_table == NULL) {
			Dee_membercache_tabuse_dec(self);
		} else {
			Dee_membercache_table_incref(old_table);
			Dee_membercache_tabuse_dec(self);

			/* Try not to override a larger table with a smaller one! */
			if (old_table->mc_mask > new_table->mc_mask) {
				Dee_membercache_table_destroy(new_table);
				goto do_operate_with_old_table;
			}
			Dee_membercache_table_decref(old_table);
		}
	}

	/* Store our new table within the member-cache. */
	old_table = atomic_xch(&self->mc_table, new_table); /* Inherit reference */
	Dee_membercache_waitfor(self);

	/* Destroy the old table reference. */
	if (old_table)
		Dee_membercache_table_decref(old_table);

	/* Make sure that the member-cache controller is linked into the global list. */
	if (!LIST_ISBOUND(self, mc_link)) {
		membercache_list_lock_acquire();
		if likely(!LIST_ISBOUND(self, mc_link))
			LIST_INSERT_HEAD(&membercache_list, self, mc_link);
		membercache_list_lock_release();
	}

	Dee_membercache_addslot_log_success(self, slot);
	return 0;
}




INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addmethod(struct Dee_membercache *self,
                          DeeTypeObject *decl, dhash_t hash,
                          struct type_method const *method) {
	struct Dee_membercache_slot slot;
	slot.mcs_type = MEMBERCACHE_METHOD;
	slot.mcs_hash = hash;
	slot.mcs_decl = decl;
	memcpy(&slot.mcs_method, method, sizeof(struct type_method));
	return Dee_membercache_addslot(self, &slot);
}

INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addinstancemethod(struct Dee_membercache *self,
                                  DeeTypeObject *decl, dhash_t hash,
                                  struct type_method const *method) {
	struct Dee_membercache_slot slot;
	ASSERT(self != &decl->tp_cache);
	slot.mcs_type = MEMBERCACHE_INSTANCE_METHOD;
	slot.mcs_hash = hash;
	slot.mcs_decl = decl;
	memcpy(&slot.mcs_method, method, sizeof(struct type_method));
	return Dee_membercache_addslot(self, &slot);
}

INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addgetset(struct Dee_membercache *self,
                          DeeTypeObject *decl, dhash_t hash,
                          struct type_getset const *getset) {
	struct Dee_membercache_slot slot;
	slot.mcs_type = MEMBERCACHE_GETSET;
	slot.mcs_hash = hash;
	slot.mcs_decl = decl;
	memcpy(&slot.mcs_getset, getset, sizeof(struct type_getset));
	return Dee_membercache_addslot(self, &slot);
}

INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addinstancegetset(struct Dee_membercache *self,
                                  DeeTypeObject *decl, dhash_t hash,
                                  struct type_getset const *getset) {
	struct Dee_membercache_slot slot;
	ASSERT(self != &decl->tp_cache);
	slot.mcs_type = MEMBERCACHE_INSTANCE_GETSET;
	slot.mcs_hash = hash;
	slot.mcs_decl = decl;
	memcpy(&slot.mcs_getset, getset, sizeof(struct type_getset));
	return Dee_membercache_addslot(self, &slot);
}

INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addmember(struct Dee_membercache *self,
                          DeeTypeObject *decl, dhash_t hash,
                          struct type_member const *member) {
	struct Dee_membercache_slot slot;
	slot.mcs_type = MEMBERCACHE_MEMBER;
	slot.mcs_hash = hash;
	slot.mcs_decl = decl;
	memcpy(&slot.mcs_member, member, sizeof(struct type_member));
	return Dee_membercache_addslot(self, &slot);
}

INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addinstancemember(struct Dee_membercache *self,
                                  DeeTypeObject *decl, dhash_t hash,
                                  struct type_member const *member) {
	struct Dee_membercache_slot slot;
	ASSERT(self != &decl->tp_cache);
	slot.mcs_type = MEMBERCACHE_INSTANCE_MEMBER;
	slot.mcs_hash = hash;
	slot.mcs_decl = decl;
	memcpy(&slot.mcs_member, member, sizeof(struct type_member));
	return Dee_membercache_addslot(self, &slot);
}

INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addattrib(struct Dee_membercache *self,
                          DeeTypeObject *decl, dhash_t hash,
                          struct class_attribute *attrib) {
	struct Dee_membercache_slot slot;
	slot.mcs_type          = MEMBERCACHE_ATTRIB;
	slot.mcs_hash          = hash;
	slot.mcs_decl          = decl;
	slot.mcs_attrib.a_name = DeeString_STR(attrib->ca_name);
	slot.mcs_attrib.a_attr = attrib;
	slot.mcs_attrib.a_desc = DeeClass_DESC(decl);
	return Dee_membercache_addslot(self, &slot);
}

INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addinstanceattrib(struct Dee_membercache *self,
                                  DeeTypeObject *decl, dhash_t hash,
                                  struct class_attribute *attrib) {
	struct Dee_membercache_slot slot;
	ASSERT(self != &decl->tp_cache);
	slot.mcs_type          = MEMBERCACHE_INSTANCE_ATTRIB;
	slot.mcs_hash          = hash;
	slot.mcs_decl          = decl;
	slot.mcs_attrib.a_name = DeeString_STR(attrib->ca_name);
	slot.mcs_attrib.a_attr = attrib;
	slot.mcs_attrib.a_desc = DeeClass_DESC(decl);
	return Dee_membercache_addslot(self, &slot);
}


/* >> bool Dee_membercache_acquiretable(struct Dee_membercache *self, [[out]] DREF struct Dee_membercache_table **p_table);
 * >> void Dee_membercache_releasetable(struct Dee_membercache *self, [[in]] DREF struct Dee_membercache_table *table);
 * Helpers to acquire and release cache tables. */
#define Dee_membercache_acquiretable(self, p_table)                   \
	(Dee_membercache_tabuse_inc(self),                                \
	 *(p_table) = atomic_read(&(self)->mc_table),                     \
	 *(p_table) ? Dee_membercache_table_incref(*(p_table)) : (void)0, \
	 Dee_membercache_tabuse_dec(self),                                \
	 *(p_table) != NULL)
#define Dee_membercache_releasetable(self, table) \
	Dee_membercache_table_decref(table)

/* Patch a member cache slot
 * @return:  1: Slot cannot be patched like that
 * @return:  0: Success
 * @return: -1: No cache table allocated */
PRIVATE NONNULL((1, 2, 3, 6, 7)) int DCALL
Dee_membercache_patch(struct Dee_membercache *self, DeeTypeObject *decl,
                      char const *attr, Dee_hash_t hash, uintptr_t attr_type,
                      bool (DCALL *do_patch)(struct Dee_membercache_slot *slot,
                                             void const *new_data,
                                             void const *old_data),
                      void const *new_data, void const *old_data) {
	int result = 1;
	Dee_hash_t i, perturb;
	DREF struct Dee_membercache_table *table;
	if unlikely(!Dee_membercache_acquiretable(self, &table))
		return -1;
	perturb = i = Dee_membercache_table_hashst(table, hash);
	for (;; Dee_membercache_table_hashnx(i, perturb)) {
		struct Dee_membercache_slot *item;
		uint16_t type;
		item = Dee_membercache_table_hashit(table, i);
		type = atomic_read(&item->mcs_type);
		if (type == MEMBERCACHE_UNUSED)
			break;
		if (item->mcs_hash != hash)
			continue;
		if unlikely(type == MEMBERCACHE_UNINITIALIZED)
			continue; /* Don't dereference uninitialized items! */
		if (strcmp(item->mcs_name, attr) != 0)
			continue;

		/* Ensure that the attribute type matches. */
		if (type != attr_type)
			return 1;

		/* Found it! -> now patch it */
		if ((*do_patch)(item, new_data, old_data)) {
			atomic_write(&item->mcs_decl, decl);
			result = 0;
			Dee_DPRINTF("[RT] Patched %s `%s.%s' in `%s' (%s)\n",
			            membercache_type_names[attr_type],
			            decl->tp_name, attr,
			            MEMBERCACHE_GETTYPENAME(self),
			            MEMBERCACHE_GETCLASSNAME(self));
		}
		break;
	}
	Dee_membercache_releasetable(self, table);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
do_patch_method(struct Dee_membercache_slot *slot, void const *new_data, void const *old_data) {
	bool result = false;
	struct type_method const *new_method = (struct type_method const *)new_data;
	struct type_method const *old_method = (struct type_method const *)old_data;
	if ((slot->mcs_method.m_flag & TYPE_METHOD_FKWDS) !=
	    (new_method->m_flag & TYPE_METHOD_FKWDS))
		return false;
	if (old_method) {
		result = atomic_cmpxch(&slot->mcs_method.m_func,
		                       old_method->m_func,
		                       new_method->m_func);
	} else {
		atomic_write(&slot->mcs_method.m_func, new_method->m_func);
		result = true;
	}
	return result;
}


PRIVATE NONNULL((1, 2, 4)) int DCALL
Dee_membercache_patchmethod(struct Dee_membercache *self, DeeTypeObject *decl, Dee_hash_t hash,
                            struct type_method const *new_method,
                            /*0..1*/ struct type_method const *old_method) {
#ifdef CONFIG_NO_THREADS
	int result = Dee_membercache_addmethod(self, decl, hash, new_method);
#else  /* CONFIG_NO_THREADS */
	int result;
	while ((result = Dee_membercache_addmethod(self, decl, hash, new_method)) == 2)
		SCHED_YIELD();
#endif /* !CONFIG_NO_THREADS */
	if (result > 0) {
		/* Entry already exists (try to patch it) */
		result = Dee_membercache_patch(self, decl, new_method->m_name, hash,
		                               MEMBERCACHE_METHOD, &do_patch_method,
		                               new_method, old_method);
	}
	return result;
}

PRIVATE NONNULL((1, 2, 4)) int DCALL
Dee_membercache_patchinstancemethod(struct Dee_membercache *self, DeeTypeObject *decl, Dee_hash_t hash,
                                    struct type_method const *new_method,
                                    /*0..1*/ struct type_method const *old_method) {
#ifdef CONFIG_NO_THREADS
	int result = Dee_membercache_addinstancemethod(self, decl, hash, new_method);
#else  /* CONFIG_NO_THREADS */
	int result;
	while ((result = Dee_membercache_addinstancemethod(self, decl, hash, new_method)) == 2)
		SCHED_YIELD();
#endif /* !CONFIG_NO_THREADS */
	if (result > 0) {
		/* Entry already exists (try to patch it) */
		result = Dee_membercache_patch(self, decl, new_method->m_name, hash,
		                               MEMBERCACHE_INSTANCE_METHOD, &do_patch_method,
		                               new_method, old_method);
	}
	return result;
}


PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
do_patch_getset(struct Dee_membercache_slot *slot, void const *new_data, void const *old_data) {
	bool result = false;
	struct type_getset const *new_getset = (struct type_getset const *)new_data;
	struct type_getset const *old_getset = (struct type_getset const *)old_data;
	if (old_getset) {
		if (atomic_cmpxch(&slot->mcs_getset.gs_get, old_getset->gs_get, new_getset->gs_get))
			result = true;
		if (atomic_cmpxch(&slot->mcs_getset.gs_del, old_getset->gs_del, new_getset->gs_del))
			result = true;
		if (atomic_cmpxch(&slot->mcs_getset.gs_set, old_getset->gs_set, new_getset->gs_set))
			result = true;
		if (atomic_cmpxch(&slot->mcs_getset.gs_bound, old_getset->gs_bound, new_getset->gs_bound))
			result = true;
	} else {
		atomic_write(&slot->mcs_getset.gs_get, new_getset->gs_get);
		atomic_write(&slot->mcs_getset.gs_del, new_getset->gs_del);
		atomic_write(&slot->mcs_getset.gs_set, new_getset->gs_set);
		atomic_write(&slot->mcs_getset.gs_bound, new_getset->gs_bound);
		result = true;
	}
	return result;
}


PRIVATE NONNULL((1, 2, 4)) int DCALL
Dee_membercache_patchgetset(struct Dee_membercache *self, DeeTypeObject *decl, Dee_hash_t hash,
                            struct type_getset const *new_getset,
                            /*0..1*/ struct type_getset const *old_getset) {
#ifdef CONFIG_NO_THREADS
	int result = Dee_membercache_addgetset(self, decl, hash, new_getset);
#else  /* CONFIG_NO_THREADS */
	int result;
	while ((result = Dee_membercache_addgetset(self, decl, hash, new_getset)) == 2)
		SCHED_YIELD();
#endif /* !CONFIG_NO_THREADS */
	if (result > 0) {
		/* Entry already exists (try to patch it) */
		result = Dee_membercache_patch(self, decl, new_getset->gs_name, hash,
		                               MEMBERCACHE_GETSET, &do_patch_getset,
		                               new_getset, old_getset);
	}
	return result;
}

PRIVATE NONNULL((1, 2, 4)) int DCALL
Dee_membercache_patchinstancegetset(struct Dee_membercache *self, DeeTypeObject *decl, Dee_hash_t hash,
                                    struct type_getset const *new_getset,
                                    /*0..1*/ struct type_getset const *old_getset) {
#ifdef CONFIG_NO_THREADS
	int result = Dee_membercache_addinstancegetset(self, decl, hash, new_getset);
#else  /* CONFIG_NO_THREADS */
	int result;
	while ((result = Dee_membercache_addinstancegetset(self, decl, hash, new_getset)) == 2)
		SCHED_YIELD();
#endif /* !CONFIG_NO_THREADS */
	if (result > 0) {
		/* Entry already exists (try to patch it) */
		result = Dee_membercache_patch(self, decl, new_getset->gs_name, hash,
		                               MEMBERCACHE_INSTANCE_GETSET, &do_patch_getset,
		                               new_getset, old_getset);
	}
	return result;
}



/* Try to add the specified attribute to the cache of "self".
 * - If this fails due to OOM, return `-1', but DON'T throw an exception
 * If the MRO cache already contains an entry for the named attribute:
 * - Verify that the existing entry is for the same type of attribute (`MEMBERCACHE_*'),
 *   such that it can be patched without having to alter `mcs_type' (since having to do
 *   so would result in a non-resolvable race condition where another thread is currently
 *   dereferencing the function pointers from the existing entry).
 *   If this verification fails, return `1'.
 *   - For `DeeTypeMRO_Patch*Method', it is also verified that both the
 *     old and new function pointers share a common `TYPE_METHOD_FKWDS'.
 * - If the type matches, the pre-existing cache entries pointers are patched such that
 *   they will now reference those from the given parameters.
 *   Note that for this purpose, this exchange is atomic for each individual function
 *   pointer (but not all pointers as a whole) -- in the case of `DeeTypeMRO_Patch*GetSet',
 *   another thread may invoke (e.g.) an out-dated `gs_del' after `gs_get' was already
 *   patched.
 *
 * NOTE: Generally, only use these functions for self-optimizing methods in base-classes
 *       that wish to skip certain type-dependent verification steps during future calls.
 *       (e.g. `Sequence.first', `Mapping.keys')
 *
 * @param: old_*: [0..1] When non-NULL, use these values for compare-exchange operations.
 *                       But also note that when there are many function pointers, some may
 *                       be set, while others cannot be -- here, an attempt to exchange
 *                       pointers is made for *all* pointers, and success is indicated if
 *                       at least 1 pointer could be exchanged.
 * @return:  1:   Failure (cache entry cannot be patched like this)
 * @return:  0:   Success
 * @return: -1:   Patching failed due to OOM (but no error was thrown!) */

PUBLIC NONNULL((1, 2, 4)) int DCALL
DeeTypeMRO_PatchMethod(DeeTypeObject *self, DeeTypeObject *decl, Dee_hash_t hash,
                       struct type_method const *new_method, /*0..1*/ struct type_method const *old_method) {
	int result = Dee_membercache_patchmethod(&self->tp_cache, decl, hash, new_method, old_method);
	if likely(result == 0)
		result = Dee_membercache_patchinstancemethod(&self->tp_class_cache, decl, hash, new_method, old_method);
	return result;
}

PUBLIC NONNULL((1, 2, 4)) int DCALL
DeeTypeMRO_PatchGetSet(DeeTypeObject *self, DeeTypeObject *decl, Dee_hash_t hash,
                       struct type_getset const *new_getset, /*0..1*/ struct type_getset const *old_getset) {
	int result = Dee_membercache_patchgetset(&self->tp_cache, decl, hash, new_getset, old_getset);
	if likely(result == 0)
		result = Dee_membercache_patchinstancegetset(&self->tp_class_cache, decl, hash, new_getset, old_getset);
	return result;
}

PUBLIC NONNULL((1, 2, 4)) int DCALL
DeeTypeMRO_PatchClassMethod(DeeTypeObject *self, DeeTypeObject *decl, Dee_hash_t hash,
                            struct type_method const *new_method, /*0..1*/ struct type_method const *old_method) {
	return Dee_membercache_patchmethod(&self->tp_class_cache, decl, hash, new_method, old_method);
}

PUBLIC NONNULL((1, 2, 4)) int DCALL
DeeTypeMRO_PatchClassGetSet(DeeTypeObject *self, DeeTypeObject *decl, Dee_hash_t hash,
                            struct type_getset const *new_getset, /*0..1*/ struct type_getset const *old_getset) {
	return Dee_membercache_patchgetset(&self->tp_class_cache, decl, hash, new_getset, old_getset);
}



#ifndef CONFIG_CALLTUPLE_OPTIMIZATIONS
/* TODO: For binary compat:
 * #define DEFINE_DeeType_CallCachedAttrStringHashTuple
 * #define DEFINE_DeeType_CallCachedClassAttrStringHashTuple
 * #define DEFINE_DeeType_CallCachedAttrStringHashTupleKw
 * #define DEFINE_DeeType_CallCachedClassAttrStringHashTupleKw
 */
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */


INTERN WUNUSED NONNULL((1, 2, 3, 5, 6)) int DCALL /* METHOD */
type_method_findattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                     struct type_method const *chain, uint16_t chain_perm,
                     struct Dee_attrspec const *__restrict specs,
                     struct Dee_attrdesc *__restrict result) {
	ASSERT(chain_perm & (Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CMEMBER));
	ASSERT(chain_perm & Dee_ATTRPERM_F_CANGET);
	ASSERT(chain_perm & Dee_ATTRPERM_F_CANCALL);
	if ((chain_perm & specs->as_perm_mask) != specs->as_perm_value)
		goto nope;
	for (; chain->m_name; ++chain) {
		if (!streq(chain->m_name, specs->as_name))
			continue;
		Dee_membercache_addmethod(cache, decl, specs->as_hash, chain);
		ASSERT(!(chain_perm & Dee_ATTRPERM_F_NAMEOBJ));
		ASSERT(!(chain_perm & Dee_ATTRPERM_F_DOCOBJ));
		result->ad_name = chain->m_name;
		result->ad_doc  = chain->m_doc;
		result->ad_perm = chain_perm;
		result->ad_info.ai_type = Dee_ATTRINFO_METHOD;
		result->ad_info.ai_decl = (DeeObject *)decl;
		result->ad_info.ai_value.v_method = chain;
		result->ad_type = NULL;
		return 0;
	}
nope:
	return 1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeType_FindInstanceMethodAttr(DeeTypeObject *tp_invoker,
                               DeeTypeObject *tp_self,
                               struct Dee_attrspec const *__restrict specs,
                               struct Dee_attrdesc *__restrict result) {
	uint16_t const chain_perm = Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CMEMBER |
	                            Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANCALL |
	                            Dee_ATTRPERM_F_WRAPPER;
	struct type_method const *chain;
	if ((chain_perm & specs->as_perm_mask) != specs->as_perm_value)
		goto nope;
	for (chain = tp_self->tp_methods; chain->m_name; ++chain) {
		if (!streq(chain->m_name, specs->as_name))
			continue;
		Dee_membercache_addinstancemethod(&tp_invoker->tp_class_cache, tp_self, specs->as_hash, chain);
		ASSERT(!(chain_perm & Dee_ATTRPERM_F_NAMEOBJ));
		ASSERT(!(chain_perm & Dee_ATTRPERM_F_DOCOBJ));
		result->ad_name         = chain->m_name;
		result->ad_doc          = chain->m_doc;
		result->ad_perm         = chain_perm;
		result->ad_info.ai_decl = (DeeObject *)tp_self;
		result->ad_info.ai_type = Dee_ATTRINFO_INSTANCE_METHOD;
		result->ad_info.ai_value.v_instance_method = chain;
		result->ad_type = NULL;
		return 0;
	}
nope:
	return 1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 5, 6)) int DCALL /* GETSET */
type_getset_findattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                     struct type_getset const *chain, uint16_t chain_perm,
                     struct Dee_attrspec const *__restrict specs,
                     struct Dee_attrdesc *__restrict result) {
	ASSERT(chain_perm & (Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CMEMBER));
	ASSERT(chain_perm & Dee_ATTRPERM_F_PROPERTY);
	for (; chain->gs_name; ++chain) {
		uint16_t item_perm;
		if (!streq(chain->gs_name, specs->as_name))
			continue;
		item_perm = chain_perm;
		if (chain->gs_get)
			item_perm |= Dee_ATTRPERM_F_CANGET;
		if (chain->gs_del)
			item_perm |= Dee_ATTRPERM_F_CANDEL;
		if (chain->gs_set)
			item_perm |= Dee_ATTRPERM_F_CANSET;
		if ((item_perm & specs->as_perm_mask) != specs->as_perm_value)
			continue;
		Dee_membercache_addgetset(cache, decl, specs->as_hash, chain);
		ASSERT(!(item_perm & Dee_ATTRPERM_F_NAMEOBJ));
		ASSERT(!(item_perm & Dee_ATTRPERM_F_DOCOBJ));
		result->ad_name         = chain->gs_name;
		result->ad_doc          = chain->gs_doc;
		result->ad_perm         = item_perm;
		result->ad_info.ai_decl = (DeeObject *)decl;
		result->ad_info.ai_type = Dee_ATTRINFO_GETSET;
		result->ad_info.ai_value.v_getset = chain;
		result->ad_type = NULL;
		return 0;
	}
	return 1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeType_FindInstanceGetSetAttr(DeeTypeObject *tp_invoker,
                               DeeTypeObject *tp_self,
                               struct Dee_attrspec const *__restrict specs,
                               struct Dee_attrdesc *__restrict result) {
	uint16_t const chain_perm  = Dee_ATTRPERM_F_PROPERTY | Dee_ATTRPERM_F_WRAPPER |
	                            Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CMEMBER;
	struct type_getset const *chain;
	for (chain = tp_self->tp_getsets; chain->gs_name; ++chain) {
		uint16_t item_perm;
		if (!streq(chain->gs_name, specs->as_name))
			continue;
		item_perm = chain_perm;
		if (chain->gs_get)
			item_perm |= Dee_ATTRPERM_F_CANGET;
		if (chain->gs_del)
			item_perm |= Dee_ATTRPERM_F_CANDEL;
		if (chain->gs_set)
			item_perm |= Dee_ATTRPERM_F_CANSET;
		if ((item_perm & specs->as_perm_mask) != specs->as_perm_value)
			continue;
		Dee_membercache_addinstancegetset(&tp_invoker->tp_class_cache,
		                                  tp_self, specs->as_hash, chain);
		ASSERT(!(item_perm & Dee_ATTRPERM_F_NAMEOBJ));
		ASSERT(!(item_perm & Dee_ATTRPERM_F_DOCOBJ));
		result->ad_name         = chain->gs_name;
		result->ad_doc          = chain->gs_doc;
		result->ad_perm         = item_perm;
		result->ad_info.ai_type = Dee_ATTRINFO_INSTANCE_GETSET;
		result->ad_info.ai_decl = (DeeObject *)tp_self;
		result->ad_info.ai_value.v_instance_getset = chain;
		result->ad_type = NULL;
		return 0;
	}
	return 1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 5, 6)) int DCALL /* MEMBER */
type_member_findattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                     struct type_member const *chain, uint16_t chain_perm,
                     struct Dee_attrspec const *__restrict specs,
                     struct Dee_attrdesc *__restrict result) {
	ASSERT(chain_perm & (Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CMEMBER));
	ASSERT(chain_perm & Dee_ATTRPERM_F_CANGET);
	for (; chain->m_name; ++chain) {
		uint16_t item_perm;
		if (!streq(chain->m_name, specs->as_name))
			continue;
		item_perm = chain_perm;
		if (!TYPE_MEMBER_ISCONST(chain) &&
		    !(chain->m_desc.md_field.mdf_type & STRUCT_CONST))
			item_perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
		if ((item_perm & specs->as_perm_mask) != specs->as_perm_value)
			continue;
		Dee_membercache_addmember(cache, decl, specs->as_hash, chain);
		ASSERT(!(item_perm & Dee_ATTRPERM_F_NAMEOBJ));
		ASSERT(!(item_perm & Dee_ATTRPERM_F_DOCOBJ));
		result->ad_name         = chain->m_name;
		result->ad_doc          = chain->m_doc;
		result->ad_perm         = item_perm;
		result->ad_info.ai_type = Dee_ATTRINFO_MEMBER;
		result->ad_info.ai_decl = (DeeObject *)decl;
		result->ad_info.ai_value.v_member = chain;
		result->ad_type = NULL;
		return 0;
	}
	return 1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeType_FindInstanceMemberAttr(DeeTypeObject *tp_invoker,
                               DeeTypeObject *tp_self,
                               struct Dee_attrspec const *__restrict specs,
                               struct Dee_attrdesc *__restrict result) {
	uint16_t const chain_perm = Dee_ATTRPERM_F_WRAPPER | Dee_ATTRPERM_F_IMEMBER |
	                            Dee_ATTRPERM_F_CMEMBER | Dee_ATTRPERM_F_CANGET;
	struct type_member const *chain;
	for (chain = tp_self->tp_members; chain->m_name; ++chain) {
		uint16_t item_perm;
		if (!streq(chain->m_name, specs->as_name))
			continue;
		item_perm = chain_perm;
		if (!TYPE_MEMBER_ISCONST(chain) &&
		    !(chain->m_desc.md_field.mdf_type & STRUCT_CONST))
			item_perm |= (Dee_ATTRPERM_F_CANDEL | Dee_ATTRPERM_F_CANSET);
		if ((item_perm & specs->as_perm_mask) != specs->as_perm_value)
			continue;
		Dee_membercache_addinstancemember(&tp_invoker->tp_class_cache,
		                                  tp_self, specs->as_hash, chain);
		ASSERT(!(item_perm & Dee_ATTRPERM_F_NAMEOBJ));
		ASSERT(!(item_perm & Dee_ATTRPERM_F_DOCOBJ));
		result->ad_name         = chain->m_name;
		result->ad_doc          = chain->m_doc;
		result->ad_perm         = item_perm;
		result->ad_info.ai_type = Dee_ATTRINFO_INSTANCE_MEMBER;
		result->ad_info.ai_decl = (DeeObject *)tp_self;
		result->ad_info.ai_value.v_instance_member = chain;
		result->ad_type = NULL;
		return 0;
	}
	return 1;
}

DECL_END

/* Define cache accessor functions */
#ifndef __INTELLISENSE__
#define DEFINE_DeeType_GetCachedAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_GetCachedAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_GetCachedClassAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_GetCachedClassAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_GetCachedInstanceAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_GetCachedInstanceAttrStringLenHash
#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_BoundCachedAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_BoundCachedAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_BoundCachedClassAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_BoundCachedClassAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_BoundCachedInstanceAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_BoundCachedInstanceAttrStringLenHash
#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_HasCachedAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_HasCachedAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_HasCachedClassAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_HasCachedClassAttrStringLenHash
#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_DelCachedAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_DelCachedAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_DelCachedClassAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_DelCachedClassAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_DelCachedInstanceAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_DelCachedInstanceAttrStringLenHash
#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_SetCachedAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetCachedAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetCachedClassAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetCachedClassAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetCachedInstanceAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetCachedInstanceAttrStringLenHash
#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_SetBasicCachedAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetBasicCachedAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetBasicCachedClassAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetBasicCachedClassAttrStringLenHash
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_SetBasicCachedInstanceAttrStringHash
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_SetBasicCachedInstanceAttrStringLenHash
//#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_CallCachedAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedClassAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedClassAttrStringLenHash
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedInstanceAttrStringHash
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedInstanceAttrStringLenHash
//#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_CallCachedAttrStringHashKw
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedAttrStringLenHashKw
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedClassAttrStringHashKw
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedClassAttrStringLenHashKw
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedInstanceAttrStringHashKw
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedInstanceAttrStringLenHashKw
#include "mro-impl-cache.c.inl"

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define DEFINE_DeeType_CallCachedAttrStringHashTuple
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedAttrStringLenHashTuple
//#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedClassAttrStringHashTuple
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedClassAttrStringLenHashTuple
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedInstanceAttrStringHashTuple
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedInstanceAttrStringLenHashTuple
//#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_CallCachedAttrStringHashTupleKw
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedAttrStringLenHashTupleKw
//#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedClassAttrStringHashTupleKw
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedClassAttrStringLenHashTupleKw
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedInstanceAttrStringHashTupleKw
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedInstanceAttrStringLenHashTupleKw
//#include "mro-impl-cache.c.inl"
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

#define DEFINE_DeeType_VCallCachedAttrStringHashf
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_VCallCachedAttrStringLenHashf
//#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_VCallCachedClassAttrStringHashf
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_VCallCachedClassAttrStringLenHashf
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_VCallCachedInstanceAttrStringHashf
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_VCallCachedInstanceAttrStringLenHashf
//#include "mro-impl-cache.c.inl"

//#define DEFINE_DeeType_FindCachedAttrInfoStringHash
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_FindCachedClassAttrInfoStringHash
//#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_FindCachedAttrInfoStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_FindCachedClassAttrInfoStringLenHash
#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_FindCachedAttr
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_FindCachedClassAttr
#include "mro-impl-cache.c.inl"
#endif /* !__INTELLISENSE__ */

#ifndef __INTELLISENSE__
#include "mro-impl.c.inl"
#define DEFINE_MRO_ATTRLEN_FUNCTIONS
#include "mro-impl.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_RUNTIME_MRO_C */
