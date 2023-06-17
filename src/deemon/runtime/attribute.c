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
#ifndef GUARD_DEEMON_RUNTIME_ATTRIBUTE_C
#define GUARD_DEEMON_RUNTIME_ATTRIBUTE_C 1

#include <deemon/api.h>
#include <deemon/attribute.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* bzero(), ... */
#include <deemon/super.h>
#include <deemon/tuple.h>

#include <stdarg.h>

#include "../objects/seq/each.h"
#include "runtime_error.h"

/* Attribute access. */

DECL_BEGIN

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_getattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_callattr(DeeObject *self, DeeObject *attr, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_callattr_kw(DeeObject *self, DeeObject *attr, size_t argc, DeeObject *const *argv, DeeObject *kw);
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define type_callattr_tuple(self, attr, args)        type_callattr(self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define type_callattr_tuple_kw(self, attr, args, kw) type_callattr_kw(self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL type_delattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL type_setattr(DeeObject *self, DeeObject *attr, DeeObject *value);

/* For type-type, these should be accessed as members, not as class-wrappers:
 * >> import Type from deemon;
 * >> print Type.baseof(x); // Should be a bound instance-method,
 * >>                       // rather than an unbound class method!
 */
#undef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
#undef CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES
#undef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
#undef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
#define CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
//#define CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE /* Don't enable this again. - It's better if this is off. */
//#define CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
//#define CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES

PUBLIC WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
DeeObject_EnumAttr(DeeTypeObject *tp_self,
                   DeeObject *self, denum_t proc, void *arg) {
	dssize_t temp, result = 0;
	DeeTypeObject *iter = tp_self;
	do {
		if (iter->tp_attr) {
			if (!iter->tp_attr->tp_enumattr)
				break;
			temp = (*iter->tp_attr->tp_enumattr)(iter, self, proc, arg);
			if unlikely(temp < 0)
				goto err;
			result += temp;
			break;
		}
		if (DeeType_IsClass(iter)) {
			temp = DeeClass_EnumInstanceAttributes(iter, self, proc, arg);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		} else {
			if (iter->tp_methods) {
				/* Methods can be access from instances & classes! */
				temp = type_method_enum(iter, iter->tp_methods,
				                        ATTR_IMEMBER, proc, arg);
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
			if (iter->tp_getsets) {
				/* Getsets can be access from instances & classes! */
				temp = type_getset_enum(iter, iter->tp_getsets,
				                        ATTR_IMEMBER | ATTR_PROPERTY,
				                        proc, arg);
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
			if (iter->tp_members) {
				/* Members can be access from instances & classes! */
				temp = type_member_enum(iter, iter->tp_members,
				                        ATTR_IMEMBER, proc, arg);
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
		}
	} while ((iter = DeeType_Base(iter)) != NULL);
	return result;
err:
	return temp;
}


#ifdef CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES
struct tiny_set {
	DeeObject      **ts_map;      /* [owned_if(!= ts_smap)] Map of all objects. */
	size_t           ts_mask;     /* Current hash-mask */
	size_t           ts_size;     /* Number of contained objects. */
	DREF DeeObject  *ts_smap[16]; /* Statically allocated map. */
};

#define tiny_set_init(x)                           \
	((x)->ts_map = (x)->ts_smap, (x)->ts_size = 0, \
	 bzero((x)->ts_smap, sizeof((x)->ts_smap)),    \
	 (x)->ts_mask = COMPILER_LENOF((x)->ts_smap) - 1)
#define tiny_set_fini(x) \
	((x)->ts_map == (x)->ts_smap ? (void)0 : (void)Dee_Free((x)->ts_map))

LOCAL bool DCALL
tiny_set_contains(struct tiny_set *__restrict self,
                  DeeObject *__restrict obj) {
	dhash_t i, perturb;
	perturb = i = Dee_HashPointer(obj) & self->ts_mask;
	for (;; i = ((i << 2) + i + perturb + 1), perturb >>= 5) {
		DeeObject *item = self->ts_map[i & self->ts_mask];
		if (!item)
			break; /* Not found */
		if (item == obj)
			return true;
	}
	return false;
}

LOCAL bool DCALL
tiny_set_rehash(struct tiny_set *__restrict self) {
	dhash_t i, j, perturb;
	DeeObject **new_map;
	size_t new_mask = (self->ts_mask << 1) | 1;
	/* Allocate the new map. */
	new_map = (DeeObject **)Dee_Callocc(new_mask + 1, sizeof(DeeObject *));
	if unlikely(!new_map)
		return false;
	/* Rehash the old map. */
	for (i = 0; i <= self->ts_mask; ++i) {
		DeeObject *obj = self->ts_map[i];
		if (!obj)
			continue;
		perturb = j = Dee_HashPointer(obj) & new_mask;
		for (;; j = ((j << 2) + j + perturb + 1), perturb >>= 5) {
			if (new_map[j])
				continue;
			new_map[j] = obj;
			break;
		}
	}
	/* Install the new map. */
	if (self->ts_map != self->ts_smap)
		Dee_Free(self->ts_map);
	self->ts_map  = new_map;
	self->ts_mask = new_mask;
	return true;
}

LOCAL bool DCALL
tiny_set_insert(struct tiny_set *__restrict self,
                DeeObject *__restrict obj) {
	dhash_t i, perturb;
again:
	perturb = i = Dee_HashPointer(obj) & self->ts_mask;
	for (;; i = ((i << 2) + i + perturb + 1), perturb >>= 5) {
		DeeObject *item = self->ts_map[i & self->ts_mask];
		if (item == obj)
			break; /* Already inserted */
		if (item != NULL)
			continue; /* Used slot */
		/* Check if the set must be rehashed. */
		if (self->ts_size >= self->ts_mask - 1) {
			if (!tiny_set_rehash(self))
				return false;
			goto again;
		}
		self->ts_map[i & self->ts_mask] = obj;
		++self->ts_size;
		break;
	}
	return true;
}
#endif /* CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES */




INTERN dssize_t DCALL
DeeType_EnumAttr(DeeTypeObject *__restrict self,
                 denum_t proc, void *arg) {
	dssize_t temp, result = 0;
	DeeTypeObject *iter = self;
#ifdef CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES
	struct tiny_set finished_set;
	tiny_set_init(&finished_set);
#endif /* CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES */
	do {
#ifdef CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES
		if unlikely(!tiny_set_insert(&finished_set, (DeeObject *)self))
			goto err_m1;
#endif /* CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES */
		if (DeeType_IsClass(iter)) {
			temp = DeeClass_EnumClassAttributes(iter, proc, arg);
			if unlikely(temp < 0)
				goto err;
			result += temp;
			temp = DeeClass_EnumClassInstanceAttributes(iter, proc, arg);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		} else {
			if (iter->tp_class_methods) {
				temp = type_method_enum(iter, iter->tp_class_methods,
				                        ATTR_CMEMBER, proc, arg);
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
			if (iter->tp_class_getsets) {
				temp = type_getset_enum(iter, iter->tp_class_getsets,
				                        ATTR_CMEMBER | ATTR_PROPERTY,
				                        proc, arg);
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
			if (iter->tp_class_members) {
				temp = type_member_enum(iter, iter->tp_class_members,
				                        ATTR_CMEMBER, proc, arg);
				if unlikely(temp < 0)
					goto err;
				result += temp;
			}
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods) {
					/* Access instance methods using `DeeClsMethodObject' */
					temp = type_obmeth_enum(iter, proc, arg);
					if unlikely(temp < 0)
						goto err;
					result += temp;
				}
				if (iter->tp_getsets) {
					/* Access instance getsets using `DeeClsPropertyObject' */
					temp = type_obprop_enum(iter, proc, arg);
					if unlikely(temp < 0)
						goto err;
					result += temp;
				}
				if (iter->tp_members) {
					/* Access instance members using `DeeClsMemberObject' */
					temp = type_obmemb_enum(iter, proc, arg);
					if unlikely(temp < 0)
						goto err;
					result += temp;
				}
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
#ifdef CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES
		if (!tiny_set_contains(&finished_set, (DeeObject *)Dee_TYPE(iter))) {
			temp = DeeObject_TGenericEnumAttr(Dee_TYPE(iter), proc, arg);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
#else /* CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES */
		temp = DeeObject_TGenericEnumAttr(Dee_TYPE(iter), proc, arg);
		if unlikely(temp < 0)
			goto err;
		result += temp;
#endif /* !CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES */
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	temp = DeeObject_TGenericEnumAttr(Dee_TYPE(self), proc, arg);
	if unlikely(temp < 0)
		goto err;
	result += temp;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
done:
#ifdef CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES
	tiny_set_fini(&finished_set);
#endif /* CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES */
	return result;
#ifdef CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES
err_m1:
	temp = -1;
#endif /* CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES */
err:
	result = temp;
	goto done;
}

PUBLIC WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeObject_TGenericEnumAttr(DeeTypeObject *__restrict tp_self, denum_t proc, void *arg) {
	dssize_t temp, result = 0;
	ASSERT_OBJECT(tp_self);
	ASSERT(DeeType_Check(tp_self));
	do {
		if (tp_self->tp_methods) {
			temp = type_method_enum(tp_self, tp_self->tp_methods,
			                        ATTR_IMEMBER, proc, arg);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		if (tp_self->tp_getsets) {
			temp = type_getset_enum(tp_self, tp_self->tp_getsets,
			                        ATTR_IMEMBER | ATTR_PROPERTY,
			                        proc, arg);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
		if (tp_self->tp_members) {
			temp = type_member_enum(tp_self, tp_self->tp_members,
			                        ATTR_IMEMBER, proc, arg);
			if unlikely(temp < 0)
				goto err;
			result += temp;
		}
	} while ((tp_self = DeeType_Base(tp_self)) != NULL);
	return result;
err:
	return temp;
}

INTERN WUNUSED NONNULL((1, 2, 3)) int DCALL
DeeType_FindAttr(DeeTypeObject *__restrict self,
                 struct attribute_info *__restrict result,
                 struct attribute_lookup_rules const *__restrict rules) {
	int error;
	DeeTypeObject *iter;
	if ((error = DeeType_FindCachedClassAttr(self, result, rules)) <= 0)
		goto done;
	iter = self;
	do {
continue_at_iter:
		if (rules->alr_decl && rules->alr_decl != (DeeObject *)iter) {
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
			if ((error = DeeObject_TGenericFindAttr(iter, (DeeObject *)iter, result, rules)) <= 0)
				goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
			iter = self = DeeType_Base(iter);
			if (!iter)
				break;
			goto continue_at_iter;
		}
		if (DeeType_IsClass(iter)) {
			if ((error = DeeClass_FindClassAttribute(self, iter, result, rules)) <= 0)
				goto done;
			if ((error = DeeClass_FindClassInstanceAttribute(self, iter, result, rules)) <= 0)
				goto done;
		} else {
			if (iter->tp_class_methods &&
			    (error = DeeType_FindClassMethodAttr(self, iter, result, rules)) <= 0)
				goto done;
			if (iter->tp_class_getsets &&
			    (error = DeeType_FindClassGetSetAttr(self, iter, result, rules)) <= 0)
				goto done;
			if (iter->tp_class_members &&
			    (error = DeeType_FindClassMemberAttr(self, iter, result, rules)) <= 0)
				goto done;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    (error = DeeType_FindInstanceMethodAttr(self, iter, result, rules)) <= 0)
					goto done;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    (error = DeeType_FindInstanceGetSetAttr(self, iter, result, rules)) <= 0)
					goto done;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    (error = DeeType_FindInstanceMemberAttr(self, iter, result, rules)) <= 0)
					goto done;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if ((error = DeeObject_TGenericFindAttr(iter, (DeeObject *)iter, result, rules)) <= 0)
			goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
		;
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	return DeeObject_TGenericFindAttr(self, (DeeObject *)self, result, rules);
#else /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	return 1; /* Not found */
#endif /* !CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
done:
	return error;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeType_GetAttrString(DeeTypeObject *__restrict self,
                      char const *__restrict attr, dhash_t hash) {
	DeeTypeObject *iter;
	DREF DeeObject *result;
	if ((result = DeeType_GetCachedClassAttr(self, attr, hash)) != ITER_DONE)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryClassAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_GetClassAttribute(iter, cattr);
			}
			if ((cattr = DeeType_QueryInstanceAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_GetInstanceAttribute(iter, cattr);
			}
		} else {
			if (iter->tp_class_methods &&
			    (result = DeeType_GetClassMethodAttr(self, iter, attr, hash)) != ITER_DONE)
				goto done;
			if (iter->tp_class_getsets &&
			    (result = DeeType_GetClassGetSetAttr(self, iter, attr, hash)) != ITER_DONE)
				goto done;
			if (iter->tp_class_members &&
			    (result = DeeType_GetClassMemberAttr(self, iter, attr, hash)) != ITER_DONE)
				goto done;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    (result = DeeType_GetInstanceMethodAttr(self, iter, attr, hash)) != ITER_DONE)
					goto done;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    (result = DeeType_GetInstanceGetSetAttr(self, iter, attr, hash)) != ITER_DONE)
					goto done;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    (result = DeeType_GetInstanceMemberAttr(self, iter, attr, hash)) != ITER_DONE)
					goto done;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if ((result = DeeObject_GenericGetAttrString((DeeObject *)iter, attr, hash)) != ITER_DONE)
			goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	if ((result = DeeObject_GenericGetAttrString((DeeObject *)self, attr, hash)) != ITER_DONE)
		goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	err_unknown_attribute(self, attr, ATTR_ACCESS_GET);
err:
	return NULL;
done:
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeType_GetAttrStringLen(DeeTypeObject *__restrict self,
                         char const *__restrict attr,
                         size_t attrlen, dhash_t hash) {
	DeeTypeObject *iter;
	DREF DeeObject *result;
	if ((result = DeeType_GetCachedClassAttrLen(self, attr, attrlen, hash)) != ITER_DONE)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryClassAttributeStringLenWithHash(self, iter, attr, attrlen, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_GetClassAttribute(iter, cattr);
			}
			if ((cattr = DeeType_QueryInstanceAttributeStringLenWithHash(self, iter, attr, attrlen, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_GetInstanceAttribute(iter, cattr);
			}
		} else {
			if (iter->tp_class_methods &&
			    (result = DeeType_GetClassMethodAttrLen(self, iter, attr, attrlen, hash)) != ITER_DONE)
				goto done;
			if (iter->tp_class_getsets &&
			    (result = DeeType_GetClassGetSetAttrLen(self, iter, attr, attrlen, hash)) != ITER_DONE)
				goto done;
			if (iter->tp_class_members &&
			    (result = DeeType_GetClassMemberAttrLen(self, iter, attr, attrlen, hash)) != ITER_DONE)
				goto done;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    (result = DeeType_GetInstanceMethodAttrLen(self, iter, attr, attrlen, hash)) != ITER_DONE)
					goto done;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    (result = DeeType_GetInstanceGetSetAttrLen(self, iter, attr, attrlen, hash)) != ITER_DONE)
					goto done;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    (result = DeeType_GetInstanceMemberAttrLen(self, iter, attr, attrlen, hash)) != ITER_DONE)
					goto done;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if ((result = DeeObject_GenericGetAttrStringLen((DeeObject *)iter, attr, attrlen, hash)) != ITER_DONE)
			goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	if ((result = DeeObject_GenericGetAttrStringLen((DeeObject *)self, attr, attrlen, hash)) != ITER_DONE)
		goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	err_unknown_attribute_len(self,
	                          attr,
	                          attrlen,
	                          ATTR_ACCESS_GET);
err:
	return NULL;
done:
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeType_BoundAttrString(DeeTypeObject *__restrict self,
                        char const *__restrict attr, dhash_t hash) {
	int result;
	DeeTypeObject *iter;
	if ((result = DeeType_BoundCachedClassAttr(self, attr, hash)) != -2)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryClassAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_BoundClassAttribute(iter, cattr);
			}
			if ((cattr = DeeType_QueryInstanceAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				goto is_bound; /* Wrapper objects are always bound. */
			}
		} else {
			if (iter->tp_class_methods &&
			    DeeType_HasClassMethodAttr(self, iter, attr, hash))
				goto is_bound;
			if (iter->tp_class_getsets &&
			    (result = DeeType_BoundClassGetSetAttr(self, iter, attr, hash)) != -2)
				goto done;
			if (iter->tp_class_members &&
			    (result = DeeType_BoundClassMemberAttr(self, iter, attr, hash)) != -2)
				goto is_bound;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    DeeType_HasInstanceMethodAttr(self, iter, attr, hash))
					goto is_bound;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    DeeType_HasInstanceGetSetAttr(self, iter, attr, hash))
					goto is_bound;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    DeeType_HasInstanceMemberAttr(self, iter, attr, hash))
					goto is_bound;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if ((result = DeeObject_GenericBoundAttrString((DeeObject *)iter, attr, hash)) != -2)
			goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	return DeeObject_GenericBoundAttrString((DeeObject *)self, attr, hash);
#else /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	return -2;
#endif /* !CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
is_bound:
	return 1;
done:
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeType_BoundAttrStringLen(DeeTypeObject *__restrict self,
                           char const *__restrict attr,
                           size_t attrlen,
                           dhash_t hash) {
	int result;
	DeeTypeObject *iter;
	if ((result = DeeType_BoundCachedClassAttrLen(self, attr, attrlen, hash)) != -2)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryClassAttributeStringLenWithHash(self, iter, attr, attrlen, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_BoundClassAttribute(iter, cattr);
			}
			if ((cattr = DeeType_QueryInstanceAttributeStringLenWithHash(self, iter, attr, attrlen, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				goto is_bound; /* Wrapper objects are always bound. */
			}
		} else {
			if (iter->tp_class_methods &&
			    DeeType_HasClassMethodAttrLen(self, iter, attr, attrlen, hash))
				goto is_bound;
			if (iter->tp_class_getsets &&
			    (result = DeeType_BoundClassGetSetAttrLen(self, iter, attr, attrlen, hash)) != -2)
				goto done;
			if (iter->tp_class_members &&
			    (result = DeeType_BoundClassMemberAttrLen(self, iter, attr, attrlen, hash)) != -2)
				goto is_bound;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    DeeType_HasInstanceMethodAttrLen(self, iter, attr, attrlen, hash))
					goto is_bound;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    DeeType_HasInstanceGetSetAttrLen(self, iter, attr, attrlen, hash))
					goto is_bound;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    DeeType_HasInstanceMemberAttrLen(self, iter, attr, attrlen, hash))
					goto is_bound;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if ((result = DeeObject_GenericBoundAttrStringLen((DeeObject *)iter, attr, attrlen, hash)) != -2)
			goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	return DeeObject_GenericBoundAttrStringLen((DeeObject *)self, attr, attrlen, hash);
#else /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	return -2;
#endif /* !CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
is_bound:
	return 1;
done:
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeType_CallAttrString(DeeTypeObject *self,
                       char const *__restrict attr, dhash_t hash,
                       size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeTypeObject *iter;
	if ((result = DeeType_CallCachedClassAttr(self, attr, hash, argc, argv)) != ITER_DONE)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryClassAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_CallClassAttribute(iter, cattr, argc, argv);
			}
			if ((cattr = DeeType_QueryInstanceAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_CallInstanceAttribute(iter, cattr, argc, argv);
			}
		} else {
			if (iter->tp_class_methods &&
			    (result = DeeType_CallClassMethodAttr(self, iter, attr, hash, argc, argv)) != ITER_DONE)
				goto done;
			if (iter->tp_class_getsets &&
			    (result = DeeType_GetClassGetSetAttr(self, iter, attr, hash)) != ITER_DONE)
				goto done_invoke;
			if (iter->tp_class_members &&
			    (result = DeeType_GetClassMemberAttr(self, iter, attr, hash)) != ITER_DONE)
				goto done_invoke;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    (result = DeeType_CallInstanceMethodAttr(self, iter, attr, hash, argc, argv)) != ITER_DONE)
					goto done;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    (result = DeeType_CallInstanceGetSetAttr(self, iter, attr, hash, argc, argv)) != ITER_DONE)
					goto done;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    (result = DeeType_CallInstanceMemberAttr(self, iter, attr, hash, argc, argv)) != ITER_DONE)
					goto done;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if ((result = DeeObject_GenericCallAttrString((DeeObject *)iter, attr, hash, argc, argv)) != ITER_DONE)
			goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	if ((result = DeeObject_GenericCallAttrString((DeeObject *)self, attr, hash, argc, argv)) != ITER_DONE)
		goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	err_unknown_attribute(self, attr, ATTR_ACCESS_GET);
err:
	return NULL;
done_invoke:
	if (result) {
		DREF DeeObject *real_result;
		real_result = DeeObject_Call(result, argc, argv);
		Dee_Decref(result);
		result = real_result;
	}
done:
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeType_CallAttrStringLen(DeeTypeObject *self,
                          char const *__restrict attr,
                          size_t attrlen, dhash_t hash,
                          size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeTypeObject *iter;
	if ((result = DeeType_CallCachedClassAttrLen(self, attr, attrlen, hash, argc, argv)) != ITER_DONE)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryClassAttributeStringLenWithHash(self, iter, attr, attrlen, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_CallClassAttribute(iter, cattr, argc, argv);
			}
			if ((cattr = DeeType_QueryInstanceAttributeStringLenWithHash(self, iter, attr, attrlen, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_CallInstanceAttribute(iter, cattr, argc, argv);
			}
		} else {
			if (iter->tp_class_methods &&
			    (result = DeeType_CallClassMethodAttrLen(self, iter, attr, attrlen, hash, argc, argv)) != ITER_DONE)
				goto done;
			if (iter->tp_class_getsets &&
			    (result = DeeType_GetClassGetSetAttrLen(self, iter, attr, attrlen, hash)) != ITER_DONE)
				goto done_invoke;
			if (iter->tp_class_members &&
			    (result = DeeType_GetClassMemberAttrLen(self, iter, attr, attrlen, hash)) != ITER_DONE)
				goto done_invoke;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    (result = DeeType_CallInstanceMethodAttrLen(self, iter, attr, attrlen, hash, argc, argv)) != ITER_DONE)
					goto done;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    (result = DeeType_CallInstanceGetSetAttrLen(self, iter, attr, attrlen, hash, argc, argv)) != ITER_DONE)
					goto done;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    (result = DeeType_CallInstanceMemberAttrLen(self, iter, attr, attrlen, hash, argc, argv)) != ITER_DONE)
					goto done;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if ((result = DeeObject_GenericCallAttrStringLen((DeeObject *)iter, attr, attrlen, hash, argc, argv)) != ITER_DONE)
			goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	if ((result = DeeObject_GenericCallAttrStringLen((DeeObject *)self, attr, attrlen, hash, argc, argv)) != ITER_DONE)
		goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	err_unknown_attribute_len(self,
	                          attr,
	                          attrlen,
	                          ATTR_ACCESS_GET);
err:
	return NULL;
done_invoke:
	if (result) {
		DREF DeeObject *real_result;
		real_result = DeeObject_Call(result, argc, argv);
		Dee_Decref(result);
		result = real_result;
	}
done:
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeType_CallAttrStringKw(DeeTypeObject *self,
                         char const *__restrict attr, dhash_t hash,
                         size_t argc, DeeObject *const *argv,
                         DeeObject *kw) {
	DREF DeeObject *result;
	DeeTypeObject *iter;
	if ((result = DeeType_CallCachedClassAttrKw(self, attr, hash, argc, argv, kw)) != ITER_DONE)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryClassAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_CallClassAttributeKw(iter, cattr, argc, argv, kw);
			}
			if ((cattr = DeeType_QueryInstanceAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_CallInstanceAttributeKw(iter, cattr, argc, argv, kw);
			}
		} else {
			if (iter->tp_class_methods &&
			    (result = DeeType_CallClassMethodAttrKw(self, iter, attr, hash, argc, argv, kw)) != ITER_DONE)
				goto done;
			if (iter->tp_class_getsets &&
			    (result = DeeType_GetClassGetSetAttr(self, iter, attr, hash)) != ITER_DONE)
				goto done_invoke;
			if (iter->tp_class_members &&
			    (result = DeeType_GetClassMemberAttr(self, iter, attr, hash)) != ITER_DONE)
				goto done_invoke;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    (result = DeeType_CallInstanceMethodAttrKw(self, iter, attr, hash, argc, argv, kw)) != ITER_DONE)
					goto done;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    (result = DeeType_CallInstanceGetSetAttrKw(self, iter, attr, hash, argc, argv, kw)) != ITER_DONE)
					goto done;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    (result = DeeType_CallInstanceMemberAttrKw(self, iter, attr, hash, argc, argv, kw)) != ITER_DONE)
					goto done;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if ((result = DeeObject_GenericCallAttrStringKw((DeeObject *)iter, attr, hash, argc, argv, kw)) != ITER_DONE)
			goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	if ((result = DeeObject_GenericCallAttrStringKw((DeeObject *)self, attr, hash, argc, argv, kw)) != ITER_DONE)
		goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	err_unknown_attribute(self, attr, ATTR_ACCESS_GET);
err:
	return NULL;
done_invoke:
	if (result) {
		DREF DeeObject *real_result;
		real_result = DeeObject_CallKw(result, argc, argv, kw);
		Dee_Decref(result);
		result = real_result;
	}
done:
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeType_CallAttrStringLenKw(DeeTypeObject *self,
                            char const *__restrict attr, size_t attrlen,
                            dhash_t hash, size_t argc, DeeObject *const *argv,
                            DeeObject *kw) {
	DREF DeeObject *result;
	DeeTypeObject *iter;
	if ((result = DeeType_CallCachedClassAttrLenKw(self, attr, attrlen, hash, argc, argv, kw)) != ITER_DONE)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryClassAttributeStringLenWithHash(self, iter, attr, attrlen, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_CallClassAttributeKw(iter, cattr, argc, argv, kw);
			}
			if ((cattr = DeeType_QueryInstanceAttributeStringLenWithHash(self, iter, attr, attrlen, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_CallInstanceAttributeKw(iter, cattr, argc, argv, kw);
			}
		} else {
			if (iter->tp_class_methods &&
			    (result = DeeType_CallClassMethodAttrLenKw(self, iter, attr, attrlen, hash, argc, argv, kw)) != ITER_DONE)
				goto done;
			if (iter->tp_class_getsets &&
			    (result = DeeType_GetClassGetSetAttrLen(self, iter, attr, attrlen, hash)) != ITER_DONE)
				goto done_invoke;
			if (iter->tp_class_members &&
			    (result = DeeType_GetClassMemberAttrLen(self, iter, attr, attrlen, hash)) != ITER_DONE)
				goto done_invoke;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    (result = DeeType_CallInstanceMethodAttrLenKw(self, iter, attr, attrlen, hash, argc, argv, kw)) != ITER_DONE)
					goto done;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    (result = DeeType_CallInstanceGetSetAttrLenKw(self, iter, attr, attrlen, hash, argc, argv, kw)) != ITER_DONE)
					goto done;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    (result = DeeType_CallInstanceMemberAttrLenKw(self, iter, attr, attrlen, hash, argc, argv, kw)) != ITER_DONE)
					goto done;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if ((result = DeeObject_GenericCallAttrStringLenKw((DeeObject *)iter, attr, attrlen, hash, argc, argv, kw)) != ITER_DONE)
			goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	if ((result = DeeObject_GenericCallAttrStringLenKw((DeeObject *)self, attr, attrlen, hash, argc, argv, kw)) != ITER_DONE)
		goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	err_unknown_attribute_len(self,
	                          attr,
	                          attrlen,
	                          ATTR_ACCESS_GET);
err:
	return NULL;
done_invoke:
	if (result) {
		DREF DeeObject *real_result;
		real_result = DeeObject_CallKw(result, argc, argv, kw);
		Dee_Decref(result);
		result = real_result;
	}
done:
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeType_HasAttrString(DeeTypeObject *__restrict self,
                      char const *__restrict attr, dhash_t hash) {
	DeeTypeObject *iter;
	if (DeeType_HasCachedClassAttr(self, attr, hash))
		goto yes;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			if (DeeType_QueryClassAttributeStringWithHash(self, iter, attr, hash))
				goto yes;
			if (DeeType_QueryInstanceAttributeStringWithHash(self, iter, attr, hash))
				goto yes;
		} else {
			if (iter->tp_class_methods &&
			    DeeType_HasClassMethodAttr(self, iter, attr, hash))
				goto yes;
			if (iter->tp_class_getsets &&
			    DeeType_HasClassGetSetAttr(self, iter, attr, hash))
				goto yes;
			if (iter->tp_class_members &&
			    DeeType_HasClassMemberAttr(self, iter, attr, hash))
				goto yes;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    DeeType_HasInstanceMethodAttr(self, iter, attr, hash))
					goto yes;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    DeeType_HasInstanceGetSetAttr(self, iter, attr, hash))
					goto yes;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    DeeType_HasInstanceMemberAttr(self, iter, attr, hash))
					goto yes;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if (DeeObject_GenericHasAttrString((DeeObject *)iter, attr, hash))
			goto yes;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	if (DeeObject_GenericHasAttrString((DeeObject *)self, attr, hash))
		goto yes;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	return false;
yes:
	return true;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeType_HasAttrStringLen(DeeTypeObject *__restrict self,
                         char const *__restrict attr,
                         size_t attrlen, dhash_t hash) {
	DeeTypeObject *iter;
	if (DeeType_HasCachedClassAttrLen(self, attr, attrlen, hash))
		goto yes;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			if (DeeType_QueryClassAttributeStringLenWithHash(self, iter, attr, attrlen, hash))
				goto yes;
			if (DeeType_QueryInstanceAttributeStringLenWithHash(self, iter, attr, attrlen, hash))
				goto yes;
		} else {
			if (iter->tp_class_methods &&
			    DeeType_HasClassMethodAttrLen(self, iter, attr, attrlen, hash))
				goto yes;
			if (iter->tp_class_getsets &&
			    DeeType_HasClassGetSetAttrLen(self, iter, attr, attrlen, hash))
				goto yes;
			if (iter->tp_class_members &&
			    DeeType_HasClassMemberAttrLen(self, iter, attr, attrlen, hash))
				goto yes;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    DeeType_HasInstanceMethodAttrLen(self, iter, attr, attrlen, hash))
					goto yes;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    DeeType_HasInstanceGetSetAttrLen(self, iter, attr, attrlen, hash))
					goto yes;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    DeeType_HasInstanceMemberAttrLen(self, iter, attr, attrlen, hash))
					goto yes;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if (DeeObject_GenericHasAttrStringLen((DeeObject *)iter, attr, attrlen, hash))
			goto yes;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	if (DeeObject_GenericHasAttrStringLen((DeeObject *)self, attr, attrlen, hash))
		goto yes;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	return false;
yes:
	return true;
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeType_DelAttrString)(DeeTypeObject *__restrict self,
                              char const *__restrict attr, dhash_t hash) {
	int result;
	DeeTypeObject *iter;
	if ((result = DeeType_DelCachedClassAttr(self, attr, hash)) <= 0)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryClassAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_DelClassAttribute(iter, cattr);
			}
			if ((cattr = DeeType_QueryInstanceAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_DelInstanceAttribute(iter, cattr);
			}
		} else {
			if (iter->tp_class_methods &&
			    DeeType_HasClassMethodAttr(self, iter, attr, hash))
				goto err_noaccess;
			if (iter->tp_class_getsets &&
			    (result = DeeType_DelClassGetSetAttr(self, iter, attr, hash)) <= 0)
				goto done;
			if (iter->tp_class_members &&
			    (result = DeeType_DelClassMemberAttr(self, iter, attr, hash)) <= 0)
				goto done;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    DeeType_HasInstanceMethodAttr(self, iter, attr, hash))
					goto err_noaccess;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    DeeType_HasInstanceGetSetAttr(self, iter, attr, hash))
					goto err_noaccess;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    DeeType_HasInstanceMemberAttr(self, iter, attr, hash))
					goto err_noaccess;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if ((result = DeeObject_GenericDelAttrString((DeeObject *)iter, attr, hash)) <= 0)
			goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	if ((result = DeeObject_GenericDelAttrString((DeeObject *)self, attr, hash)) <= 0)
		goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	err_unknown_attribute(self, attr, ATTR_ACCESS_DEL);
err:
	return -1;
err_noaccess:
	err_cant_access_attribute(iter, attr, ATTR_ACCESS_DEL);
	goto err;
done:
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeType_DelAttrStringLen)(DeeTypeObject *__restrict self,
                                 char const *__restrict attr,
                                 size_t attrlen, dhash_t hash) {
	int result;
	DeeTypeObject *iter;
	if ((result = DeeType_DelCachedClassAttrLen(self, attr, attrlen, hash)) <= 0)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryClassAttributeStringLenWithHash(self, iter, attr, attrlen, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_DelClassAttribute(iter, cattr);
			}
			if ((cattr = DeeType_QueryInstanceAttributeStringLenWithHash(self, iter, attr, attrlen, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_DelInstanceAttribute(iter, cattr);
			}
		} else {
			if (iter->tp_class_methods &&
			    DeeType_HasClassMethodAttrLen(self, iter, attr, attrlen, hash))
				goto err_noaccess;
			if (iter->tp_class_getsets &&
			    (result = DeeType_DelClassGetSetAttrLen(self, iter, attr, attrlen, hash)) <= 0)
				goto done;
			if (iter->tp_class_members &&
			    (result = DeeType_DelClassMemberAttrLen(self, iter, attr, attrlen, hash)) <= 0)
				goto done;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    DeeType_HasInstanceMethodAttrLen(self, iter, attr, attrlen, hash))
					goto err_noaccess;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    DeeType_HasInstanceGetSetAttrLen(self, iter, attr, attrlen, hash))
					goto err_noaccess;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    DeeType_HasInstanceMemberAttrLen(self, iter, attr, attrlen, hash))
					goto err_noaccess;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if ((result = DeeObject_GenericDelAttrStringLen((DeeObject *)iter, attr, attrlen, hash)) <= 0)
			goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	if ((result = DeeObject_GenericDelAttrStringLen((DeeObject *)self, attr, attrlen, hash)) <= 0)
		goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	err_unknown_attribute_len(self,
	                          attr,
	                          attrlen,
	                          ATTR_ACCESS_DEL);
err:
	return -1;
err_noaccess:
	err_cant_access_attribute_len(iter,
	                              attr,
	                              attrlen,
	                              ATTR_ACCESS_DEL);
	goto err;
done:
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeType_SetAttrString)(DeeTypeObject *self,
                              char const *__restrict attr,
                              dhash_t hash, DeeObject *value) {
	int result;
	DeeTypeObject *iter;
	if ((result = DeeType_SetCachedClassAttr(self, attr, hash, value)) <= 0)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryClassAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_SetClassAttribute(iter, cattr, value);
			}
			if ((cattr = DeeType_QueryInstanceAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_SetInstanceAttribute(iter, cattr, value);
			}
		} else {
			if (iter->tp_class_methods &&
			    DeeType_HasClassMethodAttr(self, iter, attr, hash))
				goto err_noaccess;
			if (iter->tp_class_getsets &&
			    (result = DeeType_SetClassGetSetAttr(self, iter, attr, hash, value)) <= 0)
				goto done;
			if (iter->tp_class_members &&
			    (result = DeeType_SetClassMemberAttr(self, iter, attr, hash, value)) <= 0)
				goto done;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    DeeType_HasInstanceMethodAttr(self, iter, attr, hash))
					goto err_noaccess;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    DeeType_HasInstanceGetSetAttr(self, iter, attr, hash))
					goto err_noaccess;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    DeeType_HasInstanceMemberAttr(self, iter, attr, hash))
					goto err_noaccess;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if ((result = DeeObject_GenericDelAttrString((DeeObject *)iter, attr, hash)) <= 0)
			goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	if ((result = DeeObject_GenericDelAttrString((DeeObject *)self, attr, hash)) <= 0)
		goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	err_unknown_attribute(self, attr, ATTR_ACCESS_SET);
err:
	return -1;
err_noaccess:
	err_cant_access_attribute(iter, attr, ATTR_ACCESS_SET);
	goto err;
done:
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 5)) int
(DCALL DeeType_SetAttrStringLen)(DeeTypeObject *self, char const *__restrict attr,
                                 size_t attrlen, dhash_t hash, DeeObject *value) {
	int result;
	DeeTypeObject *iter;
	if ((result = DeeType_SetCachedClassAttrLen(self, attr, attrlen, hash, value)) <= 0)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryClassAttributeStringLenWithHash(self, iter, attr, attrlen, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_SetClassAttribute(iter, cattr, value);
			}
			if ((cattr = DeeType_QueryInstanceAttributeStringLenWithHash(self, iter, attr, attrlen, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_SetInstanceAttribute(iter, cattr, value);
			}
		} else {
			if (iter->tp_class_methods &&
			    DeeType_HasClassMethodAttrLen(self, iter, attr, attrlen, hash))
				goto err_noaccess;
			if (iter->tp_class_getsets &&
			    (result = DeeType_SetClassGetSetAttrLen(self, iter, attr, attrlen, hash, value)) <= 0)
				goto done;
			if (iter->tp_class_members &&
			    (result = DeeType_SetClassMemberAttrLen(self, iter, attr, attrlen, hash, value)) <= 0)
				goto done;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
				    DeeType_HasInstanceMethodAttrLen(self, iter, attr, attrlen, hash))
					goto err_noaccess;
				if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
				    DeeType_HasInstanceGetSetAttrLen(self, iter, attr, attrlen, hash))
					goto err_noaccess;
				if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
				    DeeType_HasInstanceMemberAttrLen(self, iter, attr, attrlen, hash))
					goto err_noaccess;
			}
		}
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		if ((result = DeeObject_GenericDelAttrStringLen((DeeObject *)iter, attr, attrlen, hash)) <= 0)
			goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
	} while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	if ((result = DeeObject_GenericDelAttrStringLen((DeeObject *)self, attr, attrlen, hash)) <= 0)
		goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */
	err_unknown_attribute_len(self,
	                          attr,
	                          attrlen,
	                          ATTR_ACCESS_SET);
err:
	return -1;
err_noaccess:
	err_cant_access_attribute_len(iter,
	                              attr,
	                              attrlen,
	                              ATTR_ACCESS_SET);
	goto err;
done:
	return result;
}


INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeType_GetInstanceAttrString(DeeTypeObject *__restrict self,
                              char const *__restrict attr, dhash_t hash) {
	/* Use `tp_cache' and search for regular attributes, loading
	 * them as though they were the equivalent typing in INSTANCE-mode.
	 * Attributes that weren't found must then be searched for in
	 * the non-class fields, before also being added to the `tp_cache'
	 * cache.
	 * -> `tp_cache' is purely reserved for instance-attributes (tp_methods, etc.)
	 * -> `tp_class_cache' is used for class-attributes primarily,
	 *     with instance-attributes overlaid when those don't overlap
	 *     with class attributes of the same attr.
	 * The GetInstanceAttr-API is meant as a 3 namespace that contains the same
	 * attributes as already defined for instance-attributes (aka. those from `tp_cache'),
	 * though instead of producing bound attributes, unbound wrappers (as produced
	 * by the overlay onto the regular class-attribute namespace (aka. `tp_class_cache'))
	 * are accessed.
	 * >> import stat from posix;
	 * >> local x = stat(".");
	 * >> print x.isreg;                       // callable (bound)   -- tp_methods       -- instance->tp_cache
	 * >> print stat.isreg;                    // class-function     -- tp_class_members -- class->tp_class_cache
	 * >> // Access to the unbound function `/posix/stat/i:isreg':
	 * >> print stat.getinstanceattr("isreg"); // callable (unbound) -- tp_methods       -- class->tp_cache
	 */
	DeeTypeObject *iter;
	DREF DeeObject *result;
	if ((result = DeeType_GetCachedInstanceAttr(self, attr, hash)) != ITER_DONE)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryIInstanceAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_GetInstanceAttribute(iter, cattr);
			}
		} else {
			if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
			    (result = DeeType_GetIInstanceMethodAttr(self, iter, attr, hash)) != ITER_DONE)
				goto done;
			if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
			    (result = DeeType_GetIInstanceGetSetAttr(self, iter, attr, hash)) != ITER_DONE)
				goto done;
			if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
			    (result = DeeType_GetIInstanceMemberAttr(self, iter, attr, hash)) != ITER_DONE)
				goto done;
		}
	} while ((iter = DeeType_Base(iter)) != NULL);
	err_unknown_attribute(self, attr, ATTR_ACCESS_GET);
err:
	return NULL;
done:
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeType_BoundInstanceAttrString(DeeTypeObject *__restrict self,
                                char const *__restrict attr, dhash_t hash) {
	DeeTypeObject *iter;
	int result;
	if ((result = DeeType_BoundCachedInstanceAttr(self, attr, hash)) != -2)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryIInstanceAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_BoundInstanceAttribute(iter, cattr);
			}
		} else {
			if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
			    DeeType_HasIInstanceMethodAttr(self, iter, attr, hash))
				goto yes;
			if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
			    DeeType_HasIInstanceGetSetAttr(self, iter, attr, hash))
				goto yes;
			if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
			    DeeType_HasIInstanceMemberAttr(self, iter, attr, hash))
				goto yes;
		}
	} while ((iter = DeeType_Base(iter)) != NULL);
	return -2;
yes:
	return 1;
done:
	return result;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeType_CallInstanceAttrStringKw(DeeTypeObject *self,
                                 char const *__restrict attr, dhash_t hash,
                                 size_t argc, DeeObject *const *argv,
                                 DeeObject *kw) {
	DeeTypeObject *iter;
	DREF DeeObject *result;
	if ((result = DeeType_CallCachedInstanceAttrKw(self, attr, hash, argc, argv, kw)) != ITER_DONE)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryIInstanceAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_CallInstanceAttributeKw(iter, cattr, argc, argv, kw);
			}
		} else {
			if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
			    (result = DeeType_CallIInstanceMethodAttrKw(self, iter, attr, hash, argc, argv, kw)) != ITER_DONE)
				goto done;
			if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
			    (result = DeeType_GetIInstanceGetSetAttr(self, iter, attr, hash)) != ITER_DONE)
				goto done_invoke;
			if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
			    (result = DeeType_GetIInstanceMemberAttr(self, iter, attr, hash)) != ITER_DONE)
				goto done_invoke;
		}
	} while ((iter = DeeType_Base(iter)) != NULL);
	err_unknown_attribute(self, attr, ATTR_ACCESS_GET);
err:
	return NULL;
done_invoke:
	if (result) {
		DREF DeeObject *real_result;
		real_result = DeeObject_CallKw(result, argc, argv, kw);
		Dee_Decref(result);
		result = real_result;
	}
done:
	return result;
}

INTERN WUNUSED NONNULL((1, 2)) bool DCALL
DeeType_HasInstanceAttrString(DeeTypeObject *__restrict self,
                              char const *__restrict attr, dhash_t hash) {
	DeeTypeObject *iter;
	if (DeeType_HasCachedInstanceAttr(self, attr, hash))
		goto yes;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			if (DeeType_QueryIInstanceAttributeStringWithHash(self, iter, attr, hash))
				goto yes;
		} else {
			if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
			    DeeType_HasIInstanceMethodAttr(self, iter, attr, hash))
				goto yes;
			if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
			    DeeType_HasIInstanceGetSetAttr(self, iter, attr, hash))
				goto yes;
			if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
			    DeeType_HasIInstanceMemberAttr(self, iter, attr, hash))
				goto yes;
		}
	} while ((iter = DeeType_Base(iter)) != NULL);
	return false;
yes:
	return true;
}

INTERN WUNUSED NONNULL((1, 2)) int
(DCALL DeeType_DelInstanceAttrString)(DeeTypeObject *__restrict self,
                                      char const *__restrict attr, dhash_t hash) {
	DeeTypeObject *iter;
	int result;
	if ((result = DeeType_DelCachedInstanceAttr(self, attr, hash)) <= 0)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryIInstanceAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_DelInstanceAttribute(iter, cattr);
			}
		} else {
			if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
			    DeeType_HasIInstanceMethodAttr(self, iter, attr, hash))
				goto err_noaccess;
			if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
			    DeeType_HasIInstanceGetSetAttr(self, iter, attr, hash))
				goto err_noaccess;
			if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
			    DeeType_HasIInstanceMemberAttr(self, iter, attr, hash))
				goto err_noaccess;
		}
	} while ((iter = DeeType_Base(iter)) != NULL);
	err_unknown_attribute(self, attr, ATTR_ACCESS_GET);
err:
	return -1;
err_noaccess:
	err_cant_access_attribute(iter, attr, ATTR_ACCESS_DEL);
	goto err;
done:
	return result;
}

INTERN WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeType_SetInstanceAttrString)(DeeTypeObject *self,
                                      char const *__restrict attr,
                                      dhash_t hash, DeeObject *value) {
	DeeTypeObject *iter;
	int result;
	if ((result = DeeType_SetCachedInstanceAttr(self, attr, hash, value)) <= 0)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryIInstanceAttributeStringWithHash(self, iter, attr, hash)) != NULL) {
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return DeeClass_SetInstanceAttribute(iter, cattr, value);
			}
		} else {
			if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
			    DeeType_HasIInstanceMethodAttr(self, iter, attr, hash))
				goto err_noaccess;
			if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
			    DeeType_HasIInstanceGetSetAttr(self, iter, attr, hash))
				goto err_noaccess;
			if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
			    DeeType_HasIInstanceMemberAttr(self, iter, attr, hash))
				goto err_noaccess;
		}
	} while ((iter = DeeType_Base(iter)) != NULL);
	err_unknown_attribute(self, attr, ATTR_ACCESS_SET);
err:
	return -1;
err_noaccess:
	err_cant_access_attribute(iter, attr, ATTR_ACCESS_SET);
	goto err;
done:
	return result;
}


INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL module_getattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL module_delattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL module_setattr(DeeObject *self, DeeObject *attr, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_getattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_delattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL super_setattr(DeeObject *self, DeeObject *attr, DeeObject *value);


PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_HasAttr)(DeeObject *self, /*String*/ DeeObject *attr) {
	DeeTypeObject *iter, *tp_self;
	dhash_t hash;
	ASSERT_OBJECT(self);
	ASSERT_OBJECT_TYPE_EXACT(attr, &DeeString_Type);
	iter = tp_self = Dee_TYPE(self);
	hash           = DeeString_Hash(attr);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	if (DeeType_HasCachedAttr(iter, DeeString_STR(attr), hash))
		goto yes;
	iter = iter;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			if (DeeType_QueryAttributeWithHash(tp_self, iter, attr, hash))
				goto yes;
		} else {
			if (iter->tp_methods &&
			    DeeType_HasMethodAttr(tp_self, iter, DeeString_STR(attr), hash))
				goto yes;
			if (iter->tp_getsets &&
			    DeeType_HasGetSetAttr(tp_self, iter, DeeString_STR(attr), hash))
				goto yes;
			if (iter->tp_members &&
			    DeeType_HasMemberAttr(tp_self, iter, DeeString_STR(attr), hash))
				goto yes;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_getattr) {
				DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
				DREF DeeObject *found_object;
				getattr = iter->tp_attr->tp_getattr;
				if (getattr == &module_getattr)
					return DeeModule_HasAttrString((DeeModuleObject *)self, DeeString_STR(attr), hash);
				if (getattr == &type_getattr)
					return DeeType_HasAttrString((DeeTypeObject *)self, DeeString_STR(attr), hash);
				if (getattr == &super_getattr) {
					iter    = DeeSuper_TYPE(self);
					self    = DeeSuper_SELF(self);
					tp_self = iter;
					goto again;
				}
				if (getattr == &instance_getattr) {
					found_object = instance_tgetattr(iter, self, attr);
				} else {
					found_object = (*getattr)(self, attr);
				}
				if likely(found_object) {
					Dee_Decref(found_object);
					goto yes;
				}
				if (!CATCH_ATTRIBUTE_ERROR())
					goto err;
				return 0;
			}
			/* Don't consider attributes from lower levels for custom attr access. */
			break;
		}
	}
	return 0;
yes:
	return 1;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_HasAttrStringHash)(DeeObject *__restrict self,
                                    char const *__restrict attr,
                                    dhash_t hash) {
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	if (DeeType_HasCachedAttr(iter, attr, hash))
		goto yes;
	iter = iter;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			if (DeeType_QueryAttributeStringWithHash(tp_self, iter, attr, hash))
				goto yes;
		} else {
			if (iter->tp_methods &&
			    DeeType_HasMethodAttr(tp_self, iter, attr, hash))
				goto yes;
			if (iter->tp_getsets &&
			    DeeType_HasGetSetAttr(tp_self, iter, attr, hash))
				goto yes;
			if (iter->tp_members &&
			    DeeType_HasMemberAttr(tp_self, iter, attr, hash))
				goto yes;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_getattr) {
				DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
				DREF DeeObject *found_object, *attr_name_ob;
				getattr = iter->tp_attr->tp_getattr;
				if (getattr == &module_getattr)
					return DeeModule_HasAttrString((DeeModuleObject *)self, attr, hash);
				if (getattr == &type_getattr)
					return DeeType_HasAttrString((DeeTypeObject *)self, attr, hash);
				if (getattr == &super_getattr) {
					iter    = DeeSuper_TYPE(self);
					self    = DeeSuper_SELF(self);
					tp_self = iter;
					goto again;
				}
				attr_name_ob = DeeString_NewWithHash(attr, hash);
				if unlikely(!attr_name_ob)
					goto err;
				if (getattr == &instance_getattr) {
					found_object = instance_tgetattr(iter, self, attr_name_ob);
				} else {
					found_object = (*getattr)(self, attr_name_ob);
				}
				Dee_Decref(attr_name_ob);
				if likely(found_object) {
					Dee_Decref(found_object);
					goto yes;
				}
				if (!CATCH_ATTRIBUTE_ERROR())
					goto err;
				return 0;
			}
			/* Don't consider attributes from lower levels for custom attr access. */
			break;
		}
	}
	return 0;
yes:
	return 1;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_HasAttrStringLenHash)(DeeObject *__restrict self,
                                       char const *__restrict attr,
                                       size_t attrlen,
                                       dhash_t hash) {
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	if (DeeType_HasCachedAttrLen(iter, attr, attrlen, hash))
		goto yes;
	iter = iter;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			if (DeeType_QueryAttributeStringLenWithHash(tp_self, iter, attr, attrlen, hash))
				goto yes;
		} else {
			if (iter->tp_methods &&
			    DeeType_HasMethodAttrLen(tp_self, iter, attr, attrlen, hash))
				goto yes;
			if (iter->tp_getsets &&
			    DeeType_HasGetSetAttrLen(tp_self, iter, attr, attrlen, hash))
				goto yes;
			if (iter->tp_members &&
			    DeeType_HasMemberAttrLen(tp_self, iter, attr, attrlen, hash))
				goto yes;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_getattr) {
				DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
				DREF DeeObject *found_object, *attr_name_ob;
				getattr = iter->tp_attr->tp_getattr;
				if (getattr == &module_getattr)
					return DeeModule_HasAttrStringLen((DeeModuleObject *)self, attr, attrlen, hash);
				if (getattr == &type_getattr)
					return DeeType_HasAttrStringLen((DeeTypeObject *)self, attr, attrlen, hash);
				if (getattr == &super_getattr) {
					iter    = DeeSuper_TYPE(self);
					self    = DeeSuper_SELF(self);
					tp_self = iter;
					goto again;
				}
				attr_name_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
				if unlikely(!attr_name_ob)
					goto err;
				if (getattr == &instance_getattr) {
					found_object = instance_tgetattr(iter, self, attr_name_ob);
				} else {
					found_object = (*getattr)(self, attr_name_ob);
				}
				Dee_Decref(attr_name_ob);
				if likely(found_object) {
					Dee_Decref(found_object);
					goto yes;
				}
				if (!CATCH_ATTRIBUTE_ERROR())
					goto err;
				return 0;
			}
			/* Don't consider attributes from lower levels for custom attr access. */
			break;
		}
	}
	return 0;
yes:
	return 1;
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_BoundAttr)(DeeObject *self, /*String*/ DeeObject *attr) {
	int result;
	dhash_t hash;
	DeeTypeObject *iter;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT_TYPE_EXACT(attr, &DeeString_Type);
	tp_self = Dee_TYPE(self);
	iter    = tp_self;
	hash    = DeeString_Hash(attr);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	/* Search through the cache for the requested attribute. */
	if ((result = DeeType_BoundCachedAttr(iter, self, DeeString_STR(attr), hash)) != -2)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeWithHash(tp_self, iter, attr, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_BoundAttribute(desc,
				                                  DeeInstance_DESC(desc,
				                                                   self),
				                                  self,
				                                  cattr);
			}
		} else {
			if (iter->tp_methods &&
			    DeeType_HasMethodAttr(tp_self, iter, DeeString_STR(attr), hash))
				goto is_bound;
			if (iter->tp_getsets &&
			    (result = DeeType_BoundGetSetAttr(tp_self, iter, self, DeeString_STR(attr), hash)) != -2)
				goto done;
			if (iter->tp_members &&
			    (result = DeeType_BoundMemberAttr(tp_self, iter, self, DeeString_STR(attr), hash)) != -2)
				goto done;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_getattr) {
				DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
				DREF DeeObject *found_object;
				getattr = iter->tp_attr->tp_getattr;
				if (getattr == &module_getattr)
					return DeeModule_BoundAttrString((DeeModuleObject *)self, DeeString_STR(attr), hash);
				if (getattr == &type_getattr)
					return DeeType_BoundAttrString((DeeTypeObject *)self, DeeString_STR(attr), hash);
				if (getattr == &super_getattr) {
					iter    = DeeSuper_TYPE(self);
					self    = DeeSuper_SELF(self);
					tp_self = iter;
					goto again;
				}
				if (iter->tp_attr->tp_getattr == &instance_getattr) {
					found_object = instance_tgetattr(iter, self, attr);
				} else {
					found_object = (*iter->tp_attr->tp_getattr)(self, attr);
				}
				if likely(found_object) {
					Dee_Decref(found_object);
					goto is_bound;
				}
				if (CATCH_ATTRIBUTE_ERROR())
					return -3;
				if (DeeError_Catch(&DeeError_UnboundAttribute))
					return 0;
				goto err;
			}
			/* Don't consider attributes from lower levels for custom attr access. */
			break;
		}
	}
	return -2;
done:
	return result;
is_bound:
	return 1;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_BoundAttrStringHash)(DeeObject *__restrict self,
                                      char const *__restrict attr,
                                      dhash_t hash) {
	int result;
	DeeTypeObject *iter;
	DeeTypeObject *tp_self;
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	/* Search through the cache for the requested attribute. */
	if ((result = DeeType_BoundCachedAttr(iter, self, attr, hash)) != -2)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringWithHash(tp_self, iter, attr, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_BoundAttribute(desc,
				                                  DeeInstance_DESC(desc,
				                                                   self),
				                                  self,
				                                  cattr);
			}
		} else {
			if (iter->tp_methods &&
			    DeeType_HasMethodAttr(tp_self, iter, attr, hash))
				goto is_bound;
			if (iter->tp_getsets &&
			    (result = DeeType_BoundGetSetAttr(tp_self, iter, self, attr, hash)) != -2)
				goto done;
			if (iter->tp_members &&
			    (result = DeeType_BoundMemberAttr(tp_self, iter, self, attr, hash)) != -2)
				goto done;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_getattr) {
				DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
				DREF DeeObject *found_object, *attr_name_ob;
				getattr = iter->tp_attr->tp_getattr;
				if (getattr == &module_getattr)
					return DeeModule_BoundAttrString((DeeModuleObject *)self, attr, hash);
				if (getattr == &type_getattr)
					return DeeType_BoundAttrString((DeeTypeObject *)self, attr, hash);
				if (getattr == &super_getattr) {
					iter    = DeeSuper_TYPE(self);
					self    = DeeSuper_SELF(self);
					tp_self = iter;
					goto again;
				}
				attr_name_ob = DeeString_NewWithHash(attr, hash);
				if unlikely(!attr_name_ob)
					goto err;
				if (iter->tp_attr->tp_getattr == &instance_getattr) {
					found_object = instance_tgetattr(iter, self, attr_name_ob);
				} else {
					found_object = (*iter->tp_attr->tp_getattr)(self, attr_name_ob);
				}
				Dee_Decref(attr_name_ob);
				if likely(found_object) {
					Dee_Decref(found_object);
					goto is_bound;
				}
				if (CATCH_ATTRIBUTE_ERROR())
					return -3;
				if (DeeError_Catch(&DeeError_UnboundAttribute))
					return 0;
				goto err;
			}
			/* Don't consider attributes from lower levels for custom attr access. */
			break;
		}
	}
	return -2;
done:
	return result;
is_bound:
	return 1;
err:
	return -1;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_BoundAttrStringLenHash)(DeeObject *__restrict self,
                                         char const *__restrict attr,
                                         size_t attrlen,
                                         dhash_t hash) {
	int result;
	DeeTypeObject *iter;
	DeeTypeObject *tp_self;
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	/* Search through the cache for the requested attribute. */
	if ((result = DeeType_BoundCachedAttrLen(iter, self, attr, attrlen, hash)) != -2)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringLenWithHash(tp_self, iter, attr, attrlen, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_BoundAttribute(desc,
				                                  DeeInstance_DESC(desc,
				                                                   self),
				                                  self,
				                                  cattr);
			}
		} else {
			if (iter->tp_methods &&
			    DeeType_HasMethodAttrLen(tp_self, iter, attr, attrlen, hash))
				goto is_bound;
			if (iter->tp_getsets &&
			    (result = DeeType_BoundGetSetAttrLen(tp_self, iter, self, attr, attrlen, hash)) != -2)
				goto done;
			if (iter->tp_members &&
			    (result = DeeType_BoundMemberAttrLen(tp_self, iter, self, attr, attrlen, hash)) != -2)
				goto done;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_getattr) {
				DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
				DREF DeeObject *found_object, *attr_name_ob;
				getattr = iter->tp_attr->tp_getattr;
				if (getattr == &module_getattr)
					return DeeModule_BoundAttrStringLen((DeeModuleObject *)self, attr, attrlen, hash);
				if (getattr == &type_getattr)
					return DeeType_BoundAttrStringLen((DeeTypeObject *)self, attr, attrlen, hash);
				if (getattr == &super_getattr) {
					iter    = DeeSuper_TYPE(self);
					self    = DeeSuper_SELF(self);
					tp_self = iter;
					goto again;
				}
				attr_name_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
				if unlikely(!attr_name_ob)
					goto err;
				if (iter->tp_attr->tp_getattr == &instance_getattr) {
					found_object = instance_tgetattr(iter, self, attr_name_ob);
				} else {
					found_object = (*iter->tp_attr->tp_getattr)(self, attr_name_ob);
				}
				Dee_Decref(attr_name_ob);
				if likely(found_object) {
					Dee_Decref(found_object);
					goto is_bound;
				}
				if (CATCH_ATTRIBUTE_ERROR())
					return -3;
				if (DeeError_Catch(&DeeError_UnboundAttribute))
					return 0;
				goto err;
			}
			/* Don't consider attributes from lower levels for custom attr access. */
			break;
		}
	}
	return -2;
done:
	return result;
is_bound:
	return 1;
err:
	return -1;
}


PUBLIC WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_CallAttrKw(DeeObject *self, /*String*/ DeeObject *attr,
                     size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	dhash_t hash;
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	ASSERT_OBJECT_TYPE_EXACT(attr, &DeeString_Type);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	hash = DeeString_Hash(attr);
	if ((result = DeeType_CallCachedAttrKw(iter, self,
	                                       DeeString_STR(attr),
	                                       hash, argc, argv, kw)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeWithHash(tp_self, iter, attr, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_CallAttributeKw(desc,
				                                   DeeInstance_DESC(desc,
				                                                    self),
				                                   self, cattr, argc, argv, kw);
			}
		}
		if (iter->tp_methods &&
		    (result = DeeType_CallMethodAttrKw(tp_self, iter, self, DeeString_STR(attr), hash, argc, argv, kw)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttr(tp_self, iter, self, DeeString_STR(attr), hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttr(tp_self, iter, self, DeeString_STR(attr), hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
do_iter_attr:
			getattr = iter->tp_attr->tp_getattr;
			if (getattr == &type_getattr)
				return type_callattr_kw(self, attr, argc, argv, kw);
			if (getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttrKw(((SeqEachBase *)self)->se_seq, attr, argc, argv, kw);
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttrKw(self, attr, argc, argv, kw);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!getattr)
				break;
			result = (*getattr)(self, attr);
			goto done_invoke;
		}
	}
	err_unknown_attribute(tp_self,
	                      DeeString_STR(attr),
	                      ATTR_ACCESS_GET);
err:
	return NULL;
done_invoke:
	if likely(result) {
		DREF DeeObject *callback_result;
		callback_result = DeeObject_CallKw(result, argc, argv, kw);
		Dee_Decref(result);
		result = callback_result;
	}
done:
	return result;
}

PUBLIC WUNUSED ATTR_INS(5, 4) NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_CallAttrStringHashKw(DeeObject *self,
                               char const *__restrict attr, dhash_t hash,
                               size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	if ((result = DeeType_CallCachedAttrKw(iter, self, attr, hash, argc, argv, kw)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringWithHash(tp_self, iter, attr, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_CallAttributeKw(desc,
				                                   DeeInstance_DESC(desc,
				                                                    self),
				                                   self, cattr, argc, argv, kw);
			}
		}
		if (iter->tp_methods &&
		    (result = DeeType_CallMethodAttrKw(tp_self, iter, self, attr, hash, argc, argv, kw)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttr(tp_self, iter, self, attr, hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttr(tp_self, iter, self, attr, hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
			DREF DeeObject *attr_name_ob;
do_iter_attr:
			getattr = iter->tp_attr->tp_getattr;
			if (getattr == &type_getattr)
				return DeeType_CallAttrStringKw((DeeTypeObject *)self, attr, hash, argc, argv, kw);
			if (getattr == &module_getattr) {
				result = DeeModule_GetAttrString((DeeModuleObject *)self,
				                                 attr,
				                                 hash);
				goto done_invoke;
			}
			if (getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttrStringKw(((SeqEachBase *)self)->se_seq, attr, hash, argc, argv, kw);
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttrStringKw(self, attr, hash, argc, argv, kw);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!getattr)
				break;
			attr_name_ob = DeeString_NewWithHash(attr, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = (*getattr)(self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			goto done_invoke;
		}
	}
	err_unknown_attribute(tp_self,
	                      attr,
	                      ATTR_ACCESS_GET);
err:
	return NULL;
done_invoke:
	if likely(result) {
		DREF DeeObject *callback_result;
		callback_result = DeeObject_CallKw(result, argc, argv, kw);
		Dee_Decref(result);
		result = callback_result;
	}
done:
	return result;
}

PUBLIC WUNUSED ATTR_INS(6, 5) NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_CallAttrStringLenHashKw(DeeObject *self,
                                  char const *__restrict attr,
                                  size_t attrlen, dhash_t hash,
                                  size_t argc, DeeObject *const *argv,
                                  DeeObject *kw) {
	DREF DeeObject *result;
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	if ((result = DeeType_CallCachedAttrLenKw(iter, self, attr, attrlen, hash, argc, argv, kw)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringLenWithHash(tp_self, iter, attr, attrlen, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_CallAttributeKw(desc,
				                                   DeeInstance_DESC(desc,
				                                                    self),
				                                   self, cattr, argc, argv, kw);
			}
		}
		if (iter->tp_methods &&
		    (result = DeeType_CallMethodAttrLenKw(tp_self, iter, self, attr, attrlen, hash, argc, argv, kw)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttrLen(tp_self, iter, self, attr, attrlen, hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttrLen(tp_self, iter, self, attr, attrlen, hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
			DREF DeeObject *attr_name_ob;
do_iter_attr:
			getattr = iter->tp_attr->tp_getattr;
			if (getattr == &type_getattr)
				return DeeType_CallAttrStringLenKw((DeeTypeObject *)self, attr, attrlen, hash, argc, argv, kw);
			if (getattr == &module_getattr) {
				result = DeeModule_GetAttrStringLen((DeeModuleObject *)self,
				                                    attr,
				                                    attrlen,
				                                    hash);
				goto done_invoke;
			}
			if (getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttrStringLenKw(((SeqEachBase *)self)->se_seq, attr, attrlen, hash, argc, argv, kw);
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttrStringLenKw(self, attr, attrlen, hash, argc, argv, kw);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!getattr)
				break;
			attr_name_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = (*getattr)(self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			goto done_invoke;
		}
	}
	err_unknown_attribute_len(tp_self,
	                          attr,
	                          attrlen,
	                          ATTR_ACCESS_GET);
err:
	return NULL;
done_invoke:
	if likely(result) {
		DREF DeeObject *callback_result;
		callback_result = DeeObject_CallKw(result, argc, argv, kw);
		Dee_Decref(result);
		result = callback_result;
	}
done:
	return result;
}

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_CallAttrTuple)(DeeObject *self,
                                /*String*/ DeeObject *attr,
                                DeeObject *args) {
	DREF DeeObject *result;
	dhash_t hash;
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	ASSERT_OBJECT_TYPE_EXACT(attr, &DeeString_Type);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	hash = DeeString_Hash(attr);
	if ((result = DeeType_CallCachedAttrTuple(iter, self, DeeString_STR(attr), hash, args)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeWithHash(tp_self, iter, attr, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_CallAttributeTuple(desc,
				                                      DeeInstance_DESC(desc,
				                                                       self),
				                                      self, cattr, args);
			}
		}
		if (iter->tp_methods &&
		    (result = DeeType_CallMethodAttrTuple(tp_self, iter, self, DeeString_STR(attr), hash, args)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttr(tp_self, iter, self, DeeString_STR(attr), hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttr(tp_self, iter, self, DeeString_STR(attr), hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
do_iter_attr:
			getattr = iter->tp_attr->tp_getattr;
			if (getattr == &type_getattr)
				return type_callattr_tuple(self, attr, args);
			if (getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttrTuple(((SeqEachBase *)self)->se_seq, attr, args);
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttrTuple(self, attr, args);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!getattr)
				break;
			result = (*getattr)(self, attr);
			goto done_invoke;
		}
	}
	err_unknown_attribute(tp_self,
	                      DeeString_STR(attr),
	                      ATTR_ACCESS_GET);
err:
	return NULL;
done_invoke:
	if likely(result) {
		DREF DeeObject *callback_result;
		callback_result = DeeObject_CallTuple(result, args);
		Dee_Decref(result);
		result = callback_result;
	}
done:
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_CallAttrTupleKw)(DeeObject *self, /*String*/ DeeObject *attr,
                                  DeeObject *args, DeeObject *kw) {
	DREF DeeObject *result;
	dhash_t hash;
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	ASSERT_OBJECT_TYPE_EXACT(attr, &DeeString_Type);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	hash = DeeString_Hash(attr);
	if ((result = DeeType_CallCachedAttrTupleKw(iter, self, DeeString_STR(attr), hash, args, kw)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeWithHash(tp_self, iter, attr, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_CallAttributeTupleKw(desc,
				                                        DeeInstance_DESC(desc,
				                                                         self),
				                                        self, cattr, args, kw);
			}
		}
		if (iter->tp_methods &&
		    (result = DeeType_CallMethodAttrTupleKw(tp_self, iter, self, DeeString_STR(attr), hash, args, kw)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttr(tp_self, iter, self, DeeString_STR(attr), hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttr(tp_self, iter, self, DeeString_STR(attr), hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
do_iter_attr:
			getattr = iter->tp_attr->tp_getattr;
			if (getattr == &type_getattr)
				return type_callattr_tuple_kw(self, attr, args, kw);
			if (getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttrTupleKw(((SeqEachBase *)self)->se_seq, attr, args, kw);
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttrTupleKw(self, attr, args, kw);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!getattr)
				break;
			result = (*getattr)(self, attr);
			goto done_invoke;
		}
	}
	err_unknown_attribute(tp_self,
	                      DeeString_STR(attr),
	                      ATTR_ACCESS_GET);
err:
	return NULL;
done_invoke:
	if likely(result) {
		DREF DeeObject *callback_result;
		callback_result = DeeObject_CallTupleKw(result, args, kw);
		Dee_Decref(result);
		result = callback_result;
	}
done:
	return result;
}

#else /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_CallAttrTuple)(DeeObject *self,
                                /*String*/ DeeObject *attr,
                                DeeObject *args) {
	return DeeObject_CallAttr(self,
	                          attr,
	                          DeeTuple_SIZE(args),
	                          DeeTuple_ELEM(args));
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_CallAttrTupleKw)(DeeObject *self,
                                  /*String*/ DeeObject *attr,
                                  DeeObject *args, DeeObject *kw) {
	return DeeObject_CallAttrKw(self,
	                            attr,
	                            DeeTuple_SIZE(args),
	                            DeeTuple_ELEM(args),
	                            kw);
}
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */


PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_GetAttrStringHash(DeeObject *__restrict self,
                            char const *__restrict attr,
                            dhash_t hash) {
	DREF DeeObject *result;
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	if ((result = DeeType_GetCachedAttr(iter, self, attr, hash)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringWithHash(tp_self, iter, attr, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_GetAttribute(desc,
				                                DeeInstance_DESC(desc,
				                                                 self),
				                                self, cattr);
			}
		}
		if (iter->tp_methods &&
		    (result = DeeType_GetMethodAttr(tp_self, iter, self, attr, hash)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttr(tp_self, iter, self, attr, hash)) != ITER_DONE)
			goto done;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttr(tp_self, iter, self, attr, hash)) != ITER_DONE)
			goto done;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
do_iter_attr:
			getattr = iter->tp_attr->tp_getattr;
			if (getattr == &type_getattr)
				return DeeType_GetAttrString((DeeTypeObject *)self, attr, hash);
			if (getattr == &module_getattr)
				return DeeModule_GetAttrString((DeeModuleObject *)self, attr, hash);
			if (getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
			if (!getattr)
				break;
			attr_name_ob = DeeString_NewWithHash(attr, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = (*getattr)(self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			return result;
		}
	}
	err_unknown_attribute(tp_self,
	                      attr,
	                      ATTR_ACCESS_GET);
err:
	return NULL;
done:
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_GetAttrStringLenHash(DeeObject *__restrict self,
                               char const *__restrict attr,
                               size_t attrlen, dhash_t hash) {
	DREF DeeObject *result;
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	if ((result = DeeType_GetCachedAttrLen(iter, self, attr, attrlen, hash)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringLenWithHash(tp_self, iter, attr, attrlen, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_GetAttribute(desc,
				                                DeeInstance_DESC(desc,
				                                                 self),
				                                self, cattr);
			}
		}
		if (iter->tp_methods &&
		    (result = DeeType_GetMethodAttrLen(tp_self, iter, self, attr, attrlen, hash)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttrLen(tp_self, iter, self, attr, attrlen, hash)) != ITER_DONE)
			goto done;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttrLen(tp_self, iter, self, attr, attrlen, hash)) != ITER_DONE)
			goto done;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
do_iter_attr:
			getattr = iter->tp_attr->tp_getattr;
			if (getattr == &type_getattr)
				return DeeType_GetAttrStringLen((DeeTypeObject *)self, attr, attrlen, hash);
			if (getattr == &module_getattr)
				return DeeModule_GetAttrStringLen((DeeModuleObject *)self, attr, attrlen, hash);
			if (getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
			if (!getattr)
				break;
			attr_name_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = (*getattr)(self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			return result;
		}
	}
	err_unknown_attribute_len(tp_self,
	                          attr,
	                          attrlen,
	                          ATTR_ACCESS_GET);
err:
	return NULL;
done:
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_DelAttrStringHash)(DeeObject *__restrict self,
                                    char const *__restrict attr,
                                    dhash_t hash) {
	int result;
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	if ((result = DeeType_DelCachedAttr(iter, self, attr, hash)) <= 0)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringWithHash(tp_self, iter, attr, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_DelAttribute(desc,
				                                DeeInstance_DESC(desc,
				                                                 self),
				                                self, cattr);
			}
		}
		if (iter->tp_methods &&
		    DeeType_HasMethodAttr(tp_self, iter, attr, hash))
			goto err_noaccess;
		if (iter->tp_getsets &&
		    (result = DeeType_DelGetSetAttr(tp_self, iter, self, attr, hash)) <= 0)
			goto done;
		if (iter->tp_members &&
		    (result = DeeType_DelMemberAttr(tp_self, iter, self, attr, hash)) <= 0)
			goto done;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			int (DCALL *delattr)(DeeObject *, DeeObject *);
do_iter_attr:
			delattr = iter->tp_attr->tp_delattr;
			if (delattr == &type_delattr)
				return DeeType_DelAttrString((DeeTypeObject *)self, attr, hash);
			if (delattr == &module_delattr)
				return DeeModule_DelAttrString((DeeModuleObject *)self, attr, hash);
			if (delattr == &super_delattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
			if (!delattr)
				break;
			attr_name_ob = DeeString_NewWithHash(attr, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = (*delattr)(self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			return result;
		}
	}
	err_unknown_attribute(tp_self,
	                      attr,
	                      ATTR_ACCESS_DEL);
err:
	return -1;
done:
	return result;
err_noaccess:
	err_cant_access_attribute(tp_self,
	                          attr,
	                          ATTR_ACCESS_DEL);
	goto err;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_DelAttrStringLenHash)(DeeObject *__restrict self,
                                       char const *__restrict attr,
                                       size_t attrlen, dhash_t hash) {
	int result;
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	if ((result = DeeType_DelCachedAttrLen(iter, self, attr, attrlen, hash)) <= 0)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringLenWithHash(tp_self, iter, attr, attrlen, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_DelAttribute(desc,
				                                DeeInstance_DESC(desc,
				                                                 self),
				                                self, cattr);
			}
		}
		if (iter->tp_methods &&
		    DeeType_HasMethodAttrLen(tp_self, iter, attr, attrlen, hash))
			goto err_noaccess;
		if (iter->tp_getsets &&
		    (result = DeeType_DelGetSetAttrLen(tp_self, iter, self, attr, attrlen, hash)) <= 0)
			goto done;
		if (iter->tp_members &&
		    (result = DeeType_DelMemberAttrLen(tp_self, iter, self, attr, attrlen, hash)) <= 0)
			goto done;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			int (DCALL *delattr)(DeeObject *, DeeObject *);
do_iter_attr:
			delattr = iter->tp_attr->tp_delattr;
			if (delattr == &type_delattr)
				return DeeType_DelAttrStringLen((DeeTypeObject *)self, attr, attrlen, hash);
			if (delattr == &module_delattr)
				return DeeModule_DelAttrStringLen((DeeModuleObject *)self, attr, attrlen, hash);
			if (delattr == &super_delattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
			if (!delattr)
				break;
			attr_name_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = (*delattr)(self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			return result;
		}
	}
	err_unknown_attribute_len(tp_self,
	                          attr,
	                          attrlen,
	                          ATTR_ACCESS_DEL);
err:
	return -1;
done:
	return result;
err_noaccess:
	err_cant_access_attribute_len(tp_self,
	                              attr,
	                              attrlen,
	                              ATTR_ACCESS_DEL);
	goto err;
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) int
(DCALL DeeObject_SetAttrStringHash)(DeeObject *self,
                                    char const *__restrict attr,
                                    dhash_t hash, DeeObject *value) {
	int result;
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	if ((result = DeeType_SetCachedAttr(iter, self, attr, hash, value)) <= 0)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringWithHash(tp_self, iter, attr, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_SetAttribute(desc,
				                                DeeInstance_DESC(desc,
				                                                 self),
				                                self, cattr, value);
			}
		}
		if (iter->tp_methods &&
		    DeeType_HasMethodAttr(tp_self, iter, attr, hash))
			goto err_noaccess;
		if (iter->tp_getsets &&
		    (result = DeeType_SetGetSetAttr(tp_self, iter, self, attr, hash, value)) <= 0)
			goto done;
		if (iter->tp_members &&
		    (result = DeeType_SetMemberAttr(tp_self, iter, self, attr, hash, value)) <= 0)
			goto done;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			int (DCALL *setattr)(DeeObject *, DeeObject *, DeeObject *);
do_iter_attr:
			setattr = iter->tp_attr->tp_setattr;
			if (setattr == &type_setattr)
				return DeeType_SetAttrString((DeeTypeObject *)self, attr, hash, value);
			if (setattr == &module_setattr)
				return DeeModule_SetAttrString((DeeModuleObject *)self, attr, hash, value);
			if (setattr == &super_setattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
			if (!setattr)
				break;
			attr_name_ob = DeeString_NewWithHash(attr, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = (*setattr)(self, attr_name_ob, value);
			Dee_Decref(attr_name_ob);
			return result;
		}
	}
	err_unknown_attribute(tp_self,
	                      attr,
	                      ATTR_ACCESS_SET);
err:
	return -1;
done:
	return result;
err_noaccess:
	err_cant_access_attribute(tp_self,
	                          attr,
	                          ATTR_ACCESS_SET);
	goto err;
}

PUBLIC WUNUSED NONNULL((1, 2, 5)) int
(DCALL DeeObject_SetAttrStringLenHash)(DeeObject *self,
                                       char const *__restrict attr,
                                       size_t attrlen, dhash_t hash,
                                       DeeObject *value) {
	int result;
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	if ((result = DeeType_SetCachedAttrLen(iter, self, attr, attrlen, hash, value)) <= 0)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringLenWithHash(tp_self, iter, attr, attrlen, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_SetAttribute(desc,
				                                DeeInstance_DESC(desc,
				                                                 self),
				                                self, cattr, value);
			}
		}
		if (iter->tp_methods &&
		    DeeType_HasMethodAttrLen(tp_self, iter, attr, attrlen, hash))
			goto err_noaccess;
		if (iter->tp_getsets &&
		    (result = DeeType_SetGetSetAttrLen(tp_self, iter, self, attr, attrlen, hash, value)) <= 0)
			goto done;
		if (iter->tp_members &&
		    (result = DeeType_SetMemberAttrLen(tp_self, iter, self, attr, attrlen, hash, value)) <= 0)
			goto done;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			int (DCALL *setattr)(DeeObject *, DeeObject *, DeeObject *);
do_iter_attr:
			setattr = iter->tp_attr->tp_setattr;
			if (setattr == &type_setattr)
				return DeeType_SetAttrStringLen((DeeTypeObject *)self, attr, attrlen, hash, value);
			if (setattr == &module_setattr)
				return DeeModule_SetAttrStringLen((DeeModuleObject *)self, attr, attrlen, hash, value);
			if (setattr == &super_setattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
			if (!setattr)
				break;
			attr_name_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = (*setattr)(self, attr_name_ob, value);
			Dee_Decref(attr_name_ob);
			return result;
		}
	}
	err_unknown_attribute_len(tp_self,
	                          attr,
	                          attrlen,
	                          ATTR_ACCESS_SET);
err:
	return -1;
done:
	return result;
err_noaccess:
	err_cant_access_attribute_len(tp_self,
	                              attr,
	                              attrlen,
	                              ATTR_ACCESS_SET);
	goto err;
}


PUBLIC WUNUSED ATTR_INS(5, 4) NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_CallAttrStringHash(DeeObject *self,
                             char const *__restrict attr, dhash_t hash,
                             size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	if ((result = DeeType_CallCachedAttr(iter, self, attr, hash, argc, argv)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringWithHash(tp_self, iter, attr, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_CallAttribute(desc,
				                                 DeeInstance_DESC(desc,
				                                                  self),
				                                 self, cattr, argc, argv);
			}
		}
		if (iter->tp_methods &&
		    (result = DeeType_CallMethodAttr(tp_self, iter, self, attr, hash, argc, argv)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttr(tp_self, iter, self, attr, hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttr(tp_self, iter, self, attr, hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
do_iter_attr:
			getattr = iter->tp_attr->tp_getattr;
			if (getattr == &type_getattr)
				return DeeType_CallAttrString((DeeTypeObject *)self, attr, hash, argc, argv);
			if (getattr == &module_getattr) {
				result = DeeModule_GetAttrString((DeeModuleObject *)self, attr, hash);
				goto done_invoke;
			}
			if (getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttrString(((SeqEachBase *)self)->se_seq, attr, hash, argc, argv);
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttrString(self, attr, hash, argc, argv);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!getattr)
				break;
			attr_name_ob = DeeString_NewWithHash(attr, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = (*getattr)(self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			goto done_invoke;
		}
	}
	err_unknown_attribute(tp_self,
	                      attr,
	                      ATTR_ACCESS_GET);
err:
	return NULL;
done_invoke:
	if likely(result) {
		DREF DeeObject *callback_result;
		callback_result = DeeObject_Call(result, argc, argv);
		Dee_Decref(result);
		result = callback_result;
	}
done:
	return result;
}

PUBLIC WUNUSED ATTR_INS(6, 5) NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_CallAttrStringLenHash(DeeObject *self,
                                char const *__restrict attr,
                                size_t attrlen, dhash_t hash,
                                size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeTypeObject *iter, *tp_self;
	ASSERT_OBJECT(self);
	iter = tp_self = Dee_TYPE(self);
again:
	if (iter->tp_attr)
		goto do_iter_attr;
	if ((result = DeeType_CallCachedAttrLen(iter, self, attr, attrlen, hash, argc, argv)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringLenWithHash(tp_self, iter, attr, attrlen, hash)) != NULL) {
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(iter);
				return DeeInstance_CallAttribute(desc,
				                                 DeeInstance_DESC(desc,
				                                                  self),
				                                 self, cattr, argc, argv);
			}
		}
		if (iter->tp_methods &&
		    (result = DeeType_CallMethodAttrLen(tp_self, iter, self, attr, attrlen, hash, argc, argv)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttrLen(tp_self, iter, self, attr, attrlen, hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttrLen(tp_self, iter, self, attr, attrlen, hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			DREF DeeObject *(DCALL *getattr)(DeeObject *, DeeObject *);
do_iter_attr:
			getattr = iter->tp_attr->tp_getattr;
			if (getattr == &type_getattr)
				return DeeType_CallAttrStringLen((DeeTypeObject *)self, attr, attrlen, hash, argc, argv);
			if (getattr == &module_getattr) {
				result = DeeModule_GetAttrStringLen((DeeModuleObject *)self, attr, attrlen, hash);
				goto done_invoke;
			}
			if (getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttrStringLen(((SeqEachBase *)self)->se_seq, attr, attrlen, hash, argc, argv);
			if (getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttrStringLen(self, attr, attrlen, hash, argc, argv);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!getattr)
				break;
			attr_name_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = (*getattr)(self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			goto done_invoke;
		}
	}
	err_unknown_attribute_len(tp_self,
	                          attr,
	                          attrlen,
	                          ATTR_ACCESS_GET);
err:
	return NULL;
done_invoke:
	if likely(result) {
		DREF DeeObject *callback_result;
		callback_result = DeeObject_Call(result, argc, argv);
		Dee_Decref(result);
		result = callback_result;
	}
done:
	return result;
}


PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_GetAttrString)(DeeObject *__restrict self,
                                char const *__restrict attr) {
	return DeeObject_GetAttrStringHash(self, attr, Dee_HashStr(attr));
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_HasAttrString)(DeeObject *__restrict self,
                                char const *__restrict attr) {
	return DeeObject_HasAttrStringHash(self, attr, Dee_HashStr(attr));
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_BoundAttrString)(DeeObject *__restrict self,
                                  char const *__restrict attr) {
	return DeeObject_BoundAttrStringHash(self, attr, Dee_HashStr(attr));
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_DelAttrString)(DeeObject *__restrict self,
                                char const *__restrict attr) {
	return DeeObject_DelAttrStringHash(self, attr, Dee_HashStr(attr));
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) int
(DCALL DeeObject_SetAttrString)(DeeObject *self,
                                char const *__restrict attr,
                                DeeObject *value) {
	return DeeObject_SetAttrStringHash(self, attr, Dee_HashStr(attr), value);
}

PUBLIC WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CallAttrString)(DeeObject *self,
                                 char const *__restrict attr,
                                 size_t argc, DeeObject *const *argv) {
	return DeeObject_CallAttrStringHash(self, attr, Dee_HashStr(attr), argc, argv);
}

PUBLIC WUNUSED ATTR_INS(4, 3) NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_CallAttrStringKw)(DeeObject *self, char const *__restrict attr,
                                   size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return DeeObject_CallAttrStringHashKw(self, attr, Dee_HashStr(attr), argc, argv, kw);
}



PUBLIC ATTR_SENTINEL WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeObject_CallAttrPack(DeeObject *self,
                       /*String*/ DeeObject *attr,
                       size_t argc, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, argc);
	result = DeeObject_VCallAttrPack(self, attr, argc, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeObject_VCallAttrf(DeeObject *self, /*String*/ DeeObject *attr,
                     char const *__restrict format, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_CallAttr(self, attr,
	                            DeeTuple_SIZE(args_tuple),
	                            DeeTuple_ELEM(args_tuple));
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
DeeObject_CallAttrf(DeeObject *self,
                    /*String*/ DeeObject *attr,
                    char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VCallAttrf(self, attr, format, args);
	va_end(args);
	return result;
}

#ifdef CONFIG_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_VCallAttrStringPack, 16),
                    ASSEMBLY_NAME(DeeObject_CallAttrString, 16));
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_VCallAttrStringHashPack, 20),
                    ASSEMBLY_NAME(DeeObject_CallAttrStringHash, 20));
#else /* !__NO_DEFINE_ALIAS */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_VCallAttrStringPack)(DeeObject *self,
                                      char const *__restrict attr,
                                      size_t argc, va_list args) {
	return DeeObject_CallAttrString(self, attr, argc, (DeeObject **)args);
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_VCallAttrStringHashPack)(DeeObject *self,
                                          char const *__restrict attr,
                                          dhash_t hash, size_t argc, va_list args) {
	return DeeObject_CallAttrStringHash(self, attr, hash, argc, (DeeObject **)args);
}
#endif /* __NO_DEFINE_ALIAS */
#else /* CONFIG_VA_LIST_IS_STACK_POINTER */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_VCallAttrStringHashPack)(DeeObject *self,
                                          char const *__restrict attr,
                                          dhash_t hash, size_t argc, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VPackSymbolic(argc, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_CallAttrStringHash(self, attr, hash, argc,
	                                      DeeTuple_ELEM(args_tuple));
	DeeTuple_DecrefSymbolic(args_tuple);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_VCallAttrStringPack)(DeeObject *self,
                                      char const *__restrict attr,
                                      size_t argc, va_list args) {
	return DeeObject_VCallAttrStringHashPack(self, attr, Dee_HashStr(attr), argc, args);
}
#endif /* !CONFIG_VA_LIST_IS_STACK_POINTER */

PUBLIC ATTR_SENTINEL WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DeeObject_CallAttrStringPack)(DeeObject *self,
                               char const *__restrict attr,
                               size_t argc, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, argc);
	result = DeeObject_VCallAttrStringPack(self, attr, argc, args);
	va_end(args);
	return result;
}

PUBLIC ATTR_SENTINEL WUNUSED NONNULL((1, 2)) DREF DeeObject *
DeeObject_CallAttrStringHashPack(DeeObject *self,
                                 char const *__restrict attr,
                                 dhash_t hash, size_t argc, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, argc);
	result = DeeObject_VCallAttrStringHashPack(self, attr, hash, argc, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *DCALL
DeeObject_VCallAttrStringHashf(DeeObject *self,
                               char const *__restrict attr, dhash_t hash,
                               char const *__restrict format, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_CallAttrStringHash(self, attr, hash,
	                                      DeeTuple_SIZE(args_tuple),
	                                      DeeTuple_ELEM(args_tuple));
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeObject_VCallAttrStringLenHashf(DeeObject *self,
                                  char const *__restrict attr, size_t attrlen, dhash_t hash,
                                  char const *__restrict format, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_CallAttrStringLenHash(self, attr, attrlen, hash,
	                                         DeeTuple_SIZE(args_tuple),
	                                         DeeTuple_ELEM(args_tuple));
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_VCallAttrStringf)(DeeObject *self,
                                   char const *__restrict attr,
                                   char const *__restrict format, va_list args) {
	return DeeObject_VCallAttrStringHashf(self, attr, Dee_HashStr(attr), format, args);
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *
DeeObject_CallAttrStringHashf(DeeObject *self,
                              char const *__restrict attr, dhash_t hash,
                              char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VCallAttrStringHashf(self, attr, hash, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *
DeeObject_CallAttrStringLenHashf(DeeObject *self,
                                 char const *__restrict attr,
                                 size_t attrlen, dhash_t hash,
                                 char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VCallAttrStringLenHashf(self, attr, attrlen,
	                                           hash, format, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DeeObject_CallAttrStringf)(DeeObject *self,
                            char const *__restrict attr,
                            char const *__restrict format, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, format);
	result = DeeObject_VCallAttrStringf(self, attr, format, args);
	va_end(args);
	return result;
}


DECL_END

#ifndef __INTELLISENSE__
#define DEFINE_DeeObject_TGenericGetAttrString
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericGetAttrStringLen
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericBoundAttrString
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericBoundAttrStringLen
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericCallAttrString
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericCallAttrStringLen
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericCallAttrStringKw
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericCallAttrStringLenKw
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericHasAttrString
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericHasAttrStringLen
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericDelAttrString
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericDelAttrStringLen
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericSetAttrString
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericSetAttrStringLen
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericFindAttr
#include "attribute-access-generic.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_RUNTIME_ATTRIBUTE_C */
