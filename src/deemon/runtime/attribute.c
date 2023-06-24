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

/* For type-type, these should be accessed as members, not as class-wrappers:
 * >> import Type from deemon;
 * >> print Type.baseof(x); // Should be a bound instance-method,
 * >>                       // rather than an unbound class method!
 */
#undef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
#undef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
#undef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
#define CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
//#define CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE /* Don't enable this again. - It's better if this is off. */
//#define CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_getattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_callattr(DeeObject *self, DeeObject *attr, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_callattr_kw(DeeObject *self, DeeObject *attr, size_t argc, DeeObject *const *argv, DeeObject *kw);
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define type_callattr_tuple(self, attr, args)        type_callattr(self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define type_callattr_tuple_kw(self, attr, args, kw) type_callattr_kw(self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL type_delattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL type_setattr(DeeObject *self, DeeObject *attr, DeeObject *value);

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


INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL module_getattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL module_delattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL module_setattr(DeeObject *self, DeeObject *attr, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_getattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_delattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL super_setattr(DeeObject *self, DeeObject *attr, DeeObject *value);


/* @return: 1 : does exists
 * @return: 0 : doesn't exist
 * @return: -1: Error. */
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
	if (DeeType_HasCachedAttrStringHash(iter, DeeString_STR(attr), hash))
		goto yes;
	iter = iter;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			if (DeeType_QueryAttributeHash(tp_self, iter, attr, hash))
				goto yes;
		} else {
			if (iter->tp_methods &&
			    DeeType_HasMethodAttrStringHash(tp_self, iter, DeeString_STR(attr), hash))
				goto yes;
			if (iter->tp_getsets &&
			    DeeType_HasGetSetAttrStringHash(tp_self, iter, DeeString_STR(attr), hash))
				goto yes;
			if (iter->tp_members &&
			    DeeType_HasMemberAttrStringHash(tp_self, iter, DeeString_STR(attr), hash))
				goto yes;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_getattr) {
				DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
				DREF DeeObject *found_object;
				tp_getattr = iter->tp_attr->tp_getattr;
				if (tp_getattr == &module_getattr)
					return DeeModule_HasAttrString((DeeModuleObject *)self, DeeString_STR(attr), hash);
				if (tp_getattr == &type_getattr)
					return DeeType_HasAttrStringHash((DeeTypeObject *)self, DeeString_STR(attr), hash);
				if (tp_getattr == &super_getattr) {
					iter    = DeeSuper_TYPE(self);
					self    = DeeSuper_SELF(self);
					tp_self = iter;
					goto again;
				}
				found_object = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr);
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

/* @return: 1 : does exists
 * @return: 0 : doesn't exist
 * @return: -1: Error. */
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
	if (DeeType_HasCachedAttrStringHash(iter, attr, hash))
		goto yes;
	iter = iter;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			if (DeeType_QueryAttributeStringHash(tp_self, iter, attr, hash))
				goto yes;
		} else {
			if (iter->tp_methods &&
			    DeeType_HasMethodAttrStringHash(tp_self, iter, attr, hash))
				goto yes;
			if (iter->tp_getsets &&
			    DeeType_HasGetSetAttrStringHash(tp_self, iter, attr, hash))
				goto yes;
			if (iter->tp_members &&
			    DeeType_HasMemberAttrStringHash(tp_self, iter, attr, hash))
				goto yes;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_getattr) {
				DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
				DREF DeeObject *found_object, *attr_name_ob;
				tp_getattr = iter->tp_attr->tp_getattr;
				if (tp_getattr == &module_getattr)
					return DeeModule_HasAttrString((DeeModuleObject *)self, attr, hash);
				if (tp_getattr == &type_getattr)
					return DeeType_HasAttrStringHash((DeeTypeObject *)self, attr, hash);
				if (tp_getattr == &super_getattr) {
					iter    = DeeSuper_TYPE(self);
					self    = DeeSuper_SELF(self);
					tp_self = iter;
					goto again;
				}
				attr_name_ob = DeeString_NewWithHash(attr, hash);
				if unlikely(!attr_name_ob)
					goto err;
				found_object = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr_name_ob);
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

/* @return: 1 : does exists
 * @return: 0 : doesn't exist
 * @return: -1: Error. */
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
	if (DeeType_HasCachedAttrStringLenHash(iter, attr, attrlen, hash))
		goto yes;
	iter = iter;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			if (DeeType_QueryAttributeStringLenHash(tp_self, iter, attr, attrlen, hash))
				goto yes;
		} else {
			if (iter->tp_methods &&
			    DeeType_HasMethodAttrStringLenHash(tp_self, iter, attr, attrlen, hash))
				goto yes;
			if (iter->tp_getsets &&
			    DeeType_HasGetSetAttrStringLenHash(tp_self, iter, attr, attrlen, hash))
				goto yes;
			if (iter->tp_members &&
			    DeeType_HasMemberAttrStringLenHash(tp_self, iter, attr, attrlen, hash))
				goto yes;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_getattr) {
				DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
				DREF DeeObject *found_object, *attr_name_ob;
				tp_getattr = iter->tp_attr->tp_getattr;
				if (tp_getattr == &module_getattr)
					return DeeModule_HasAttrStringLen((DeeModuleObject *)self, attr, attrlen, hash);
				if (tp_getattr == &type_getattr)
					return DeeType_HasAttrStringLenHash((DeeTypeObject *)self, attr, attrlen, hash);
				if (tp_getattr == &super_getattr) {
					iter    = DeeSuper_TYPE(self);
					self    = DeeSuper_SELF(self);
					tp_self = iter;
					goto again;
				}
				attr_name_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
				if unlikely(!attr_name_ob)
					goto err;
				found_object = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr_name_ob);
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
	if ((result = DeeType_BoundCachedAttrStringHash(iter, self, DeeString_STR(attr), hash)) != -2)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeHash(tp_self, iter, attr, hash)) != NULL) {
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
			    DeeType_HasMethodAttrStringHash(tp_self, iter, DeeString_STR(attr), hash))
				goto is_bound;
			if (iter->tp_getsets &&
			    (result = DeeType_BoundGetSetAttrStringHash(tp_self, iter, self, DeeString_STR(attr), hash)) != -2)
				goto done;
			if (iter->tp_members &&
			    (result = DeeType_BoundMemberAttrStringHash(tp_self, iter, self, DeeString_STR(attr), hash)) != -2)
				goto done;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_getattr) {
				DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
				DREF DeeObject *found_object;
				tp_getattr = iter->tp_attr->tp_getattr;
				if (tp_getattr == &module_getattr)
					return DeeModule_BoundAttrString((DeeModuleObject *)self, DeeString_STR(attr), hash);
				if (tp_getattr == &type_getattr)
					return DeeType_BoundAttrStringHash((DeeTypeObject *)self, DeeString_STR(attr), hash);
				if (tp_getattr == &super_getattr) {
					iter    = DeeSuper_TYPE(self);
					self    = DeeSuper_SELF(self);
					tp_self = iter;
					goto again;
				}
				found_object = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr);
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
	if ((result = DeeType_BoundCachedAttrStringHash(iter, self, attr, hash)) != -2)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringHash(tp_self, iter, attr, hash)) != NULL) {
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
			    DeeType_HasMethodAttrStringHash(tp_self, iter, attr, hash))
				goto is_bound;
			if (iter->tp_getsets &&
			    (result = DeeType_BoundGetSetAttrStringHash(tp_self, iter, self, attr, hash)) != -2)
				goto done;
			if (iter->tp_members &&
			    (result = DeeType_BoundMemberAttrStringHash(tp_self, iter, self, attr, hash)) != -2)
				goto done;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_getattr) {
				DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
				DREF DeeObject *found_object, *attr_name_ob;
				tp_getattr = iter->tp_attr->tp_getattr;
				if (tp_getattr == &module_getattr)
					return DeeModule_BoundAttrString((DeeModuleObject *)self, attr, hash);
				if (tp_getattr == &type_getattr)
					return DeeType_BoundAttrStringHash((DeeTypeObject *)self, attr, hash);
				if (tp_getattr == &super_getattr) {
					iter    = DeeSuper_TYPE(self);
					self    = DeeSuper_SELF(self);
					tp_self = iter;
					goto again;
				}
				attr_name_ob = DeeString_NewWithHash(attr, hash);
				if unlikely(!attr_name_ob)
					goto err;
				found_object = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr_name_ob);
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
	if ((result = DeeType_BoundCachedAttrStringLenHash(iter, self, attr, attrlen, hash)) != -2)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringLenHash(tp_self, iter, attr, attrlen, hash)) != NULL) {
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
			    DeeType_HasMethodAttrStringLenHash(tp_self, iter, attr, attrlen, hash))
				goto is_bound;
			if (iter->tp_getsets &&
			    (result = DeeType_BoundGetSetAttrStringLenHash(tp_self, iter, self, attr, attrlen, hash)) != -2)
				goto done;
			if (iter->tp_members &&
			    (result = DeeType_BoundMemberAttrStringLenHash(tp_self, iter, self, attr, attrlen, hash)) != -2)
				goto done;
		}
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
do_iter_attr:
			if likely(iter->tp_attr->tp_getattr) {
				DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
				DREF DeeObject *found_object, *attr_name_ob;
				tp_getattr = iter->tp_attr->tp_getattr;
				if (tp_getattr == &module_getattr)
					return DeeModule_BoundAttrStringLen((DeeModuleObject *)self, attr, attrlen, hash);
				if (tp_getattr == &type_getattr)
					return DeeType_BoundAttrStringLenHash((DeeTypeObject *)self, attr, attrlen, hash);
				if (tp_getattr == &super_getattr) {
					iter    = DeeSuper_TYPE(self);
					self    = DeeSuper_SELF(self);
					tp_self = iter;
					goto again;
				}
				attr_name_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
				if unlikely(!attr_name_ob)
					goto err;
				found_object = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr_name_ob);
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
	if ((result = DeeType_CallCachedAttrStringHashKw(iter, self,
	                                       DeeString_STR(attr),
	                                       hash, argc, argv, kw)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeHash(tp_self, iter, attr, hash)) != NULL) {
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
		    (result = DeeType_CallMethodAttrStringHashKw(tp_self, iter, self, DeeString_STR(attr), hash, argc, argv, kw)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttrStringHash(tp_self, iter, self, DeeString_STR(attr), hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttrStringHash(tp_self, iter, self, DeeString_STR(attr), hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
do_iter_attr:
			tp_getattr = iter->tp_attr->tp_getattr;
			if (tp_getattr == &type_getattr)
				return type_callattr_kw(self, attr, argc, argv, kw);
			if (tp_getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (tp_getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttrKw(((SeqEachBase *)self)->se_seq, attr, argc, argv, kw);
			if (tp_getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttrKw(self, attr, argc, argv, kw);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!tp_getattr)
				break;
			result = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr);
			goto done_invoke;
		}
	}
	err_unknown_attribute(tp_self, attr, ATTR_ACCESS_GET);
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
	if ((result = DeeType_CallCachedAttrStringHashKw(iter, self, attr, hash, argc, argv, kw)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringHash(tp_self, iter, attr, hash)) != NULL) {
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
		    (result = DeeType_CallMethodAttrStringHashKw(tp_self, iter, self, attr, hash, argc, argv, kw)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttrStringHash(tp_self, iter, self, attr, hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttrStringHash(tp_self, iter, self, attr, hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
			DREF DeeObject *attr_name_ob;
do_iter_attr:
			tp_getattr = iter->tp_attr->tp_getattr;
			if (tp_getattr == &type_getattr)
				return DeeType_CallAttrStringHashKw((DeeTypeObject *)self, attr, hash, argc, argv, kw);
			if (tp_getattr == &module_getattr) {
				result = DeeModule_GetAttrString((DeeModuleObject *)self,
				                                 attr,
				                                 hash);
				goto done_invoke;
			}
			if (tp_getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (tp_getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttrStringKw(((SeqEachBase *)self)->se_seq, attr, hash, argc, argv, kw);
			if (tp_getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttrStringKw(self, attr, hash, argc, argv, kw);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!tp_getattr)
				break;
			attr_name_ob = DeeString_NewWithHash(attr, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			goto done_invoke;
		}
	}
	err_unknown_attribute_string(tp_self, attr, ATTR_ACCESS_GET);
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
	if ((result = DeeType_CallCachedAttrStringLenHashKw(iter, self, attr, attrlen, hash, argc, argv, kw)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringLenHash(tp_self, iter, attr, attrlen, hash)) != NULL) {
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
		    (result = DeeType_CallMethodAttrStringLenHashKw(tp_self, iter, self, attr, attrlen, hash, argc, argv, kw)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttrStringLenHash(tp_self, iter, self, attr, attrlen, hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttrStringLenHash(tp_self, iter, self, attr, attrlen, hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
			DREF DeeObject *attr_name_ob;
do_iter_attr:
			tp_getattr = iter->tp_attr->tp_getattr;
			if (tp_getattr == &type_getattr)
				return DeeType_CallAttrStringLenHashKw((DeeTypeObject *)self, attr, attrlen, hash, argc, argv, kw);
			if (tp_getattr == &module_getattr) {
				result = DeeModule_GetAttrStringLen((DeeModuleObject *)self,
				                                    attr,
				                                    attrlen,
				                                    hash);
				goto done_invoke;
			}
			if (tp_getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (tp_getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttrStringLenKw(((SeqEachBase *)self)->se_seq, attr, attrlen, hash, argc, argv, kw);
			if (tp_getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttrStringLenKw(self, attr, attrlen, hash, argc, argv, kw);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!tp_getattr)
				break;
			attr_name_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			goto done_invoke;
		}
	}
	err_unknown_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_GET);
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
	if ((result = DeeType_CallCachedAttrStringHashTuple(iter, self, DeeString_STR(attr), hash, args)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeHash(tp_self, iter, attr, hash)) != NULL) {
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
		    (result = DeeType_CallMethodAttrStringHashTuple(tp_self, iter, self, DeeString_STR(attr), hash, args)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttrStringHash(tp_self, iter, self, DeeString_STR(attr), hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttrStringHash(tp_self, iter, self, DeeString_STR(attr), hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
do_iter_attr:
			tp_getattr = iter->tp_attr->tp_getattr;
			if (tp_getattr == &type_getattr)
				return type_callattr_tuple(self, attr, args);
			if (tp_getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (tp_getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttrTuple(((SeqEachBase *)self)->se_seq, attr, args);
			if (tp_getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttrTuple(self, attr, args);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!tp_getattr)
				break;
			result = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr);
			goto done_invoke;
		}
	}
	err_unknown_attribute(tp_self, attr, ATTR_ACCESS_GET);
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
	if ((result = DeeType_CallCachedAttrStringHashTupleKw(iter, self, DeeString_STR(attr), hash, args, kw)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeHash(tp_self, iter, attr, hash)) != NULL) {
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
		    (result = DeeType_CallMethodAttrStringHashTupleKw(tp_self, iter, self, DeeString_STR(attr), hash, args, kw)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttrStringHash(tp_self, iter, self, DeeString_STR(attr), hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttrStringHash(tp_self, iter, self, DeeString_STR(attr), hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
do_iter_attr:
			tp_getattr = iter->tp_attr->tp_getattr;
			if (tp_getattr == &type_getattr)
				return type_callattr_tuple_kw(self, attr, args, kw);
			if (tp_getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (tp_getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttrTupleKw(((SeqEachBase *)self)->se_seq, attr, args, kw);
			if (tp_getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttrTupleKw(self, attr, args, kw);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!tp_getattr)
				break;
			result = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr);
			goto done_invoke;
		}
	}
	err_unknown_attribute(tp_self, attr, ATTR_ACCESS_GET);
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
	if ((result = DeeType_GetCachedAttrStringHash(iter, self, attr, hash)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringHash(tp_self, iter, attr, hash)) != NULL) {
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
		    (result = DeeType_GetMethodAttrStringHash(tp_self, iter, self, attr, hash)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttrStringHash(tp_self, iter, self, attr, hash)) != ITER_DONE)
			goto done;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttrStringHash(tp_self, iter, self, attr, hash)) != ITER_DONE)
			goto done;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
do_iter_attr:
			tp_getattr = iter->tp_attr->tp_getattr;
			if (tp_getattr == &type_getattr)
				return DeeType_GetAttrStringHash((DeeTypeObject *)self, attr, hash);
			if (tp_getattr == &module_getattr)
				return DeeModule_GetAttrString((DeeModuleObject *)self, attr, hash);
			if (tp_getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
			if (!tp_getattr)
				break;
			attr_name_ob = DeeString_NewWithHash(attr, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			return result;
		}
	}
	err_unknown_attribute_string(tp_self, attr, ATTR_ACCESS_GET);
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
	if ((result = DeeType_GetCachedAttrStringLenHash(iter, self, attr, attrlen, hash)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringLenHash(tp_self, iter, attr, attrlen, hash)) != NULL) {
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
		    (result = DeeType_GetMethodAttrStringLenHash(tp_self, iter, self, attr, attrlen, hash)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttrStringLenHash(tp_self, iter, self, attr, attrlen, hash)) != ITER_DONE)
			goto done;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttrStringLenHash(tp_self, iter, self, attr, attrlen, hash)) != ITER_DONE)
			goto done;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
do_iter_attr:
			tp_getattr = iter->tp_attr->tp_getattr;
			if (tp_getattr == &type_getattr)
				return DeeType_GetAttrStringLenHash((DeeTypeObject *)self, attr, attrlen, hash);
			if (tp_getattr == &module_getattr)
				return DeeModule_GetAttrStringLen((DeeModuleObject *)self, attr, attrlen, hash);
			if (tp_getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
			if (!tp_getattr)
				break;
			attr_name_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			return result;
		}
	}
	err_unknown_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_GET);
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
	if ((result = DeeType_DelCachedAttrStringHash(iter, self, attr, hash)) <= 0)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringHash(tp_self, iter, attr, hash)) != NULL) {
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
		    DeeType_HasMethodAttrStringHash(tp_self, iter, attr, hash))
			goto err_noaccess;
		if (iter->tp_getsets &&
		    (result = DeeType_DelGetSetAttrStringHash(tp_self, iter, self, attr, hash)) <= 0)
			goto done;
		if (iter->tp_members &&
		    (result = DeeType_DelMemberAttrStringHash(tp_self, iter, self, attr, hash)) <= 0)
			goto done;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			int (DCALL *tp_delattr)(DeeObject *, DeeObject *);
do_iter_attr:
			tp_delattr = iter->tp_attr->tp_delattr;
			if (tp_delattr == &type_delattr)
				return DeeType_DelAttrStringHash((DeeTypeObject *)self, attr, hash);
			if (tp_delattr == &module_delattr)
				return DeeModule_DelAttrString((DeeModuleObject *)self, attr, hash);
			if (tp_delattr == &super_delattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
			if (!tp_delattr)
				break;
			attr_name_ob = DeeString_NewWithHash(attr, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = DeeType_invoke_attr_tp_delattr(iter, tp_delattr, self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			return result;
		}
	}
	err_unknown_attribute_string(tp_self, attr, ATTR_ACCESS_DEL);
err:
	return -1;
done:
	return result;
err_noaccess:
	err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_DEL);
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
	if ((result = DeeType_DelCachedAttrStringLenHash(iter, self, attr, attrlen, hash)) <= 0)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringLenHash(tp_self, iter, attr, attrlen, hash)) != NULL) {
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
		    DeeType_HasMethodAttrStringLenHash(tp_self, iter, attr, attrlen, hash))
			goto err_noaccess;
		if (iter->tp_getsets &&
		    (result = DeeType_DelGetSetAttrStringLenHash(tp_self, iter, self, attr, attrlen, hash)) <= 0)
			goto done;
		if (iter->tp_members &&
		    (result = DeeType_DelMemberAttrStringLenHash(tp_self, iter, self, attr, attrlen, hash)) <= 0)
			goto done;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			int (DCALL *tp_delattr)(DeeObject *, DeeObject *);
do_iter_attr:
			tp_delattr = iter->tp_attr->tp_delattr;
			if (tp_delattr == &type_delattr)
				return DeeType_DelAttrStringLenHash((DeeTypeObject *)self, attr, attrlen, hash);
			if (tp_delattr == &module_delattr)
				return DeeModule_DelAttrStringLen((DeeModuleObject *)self, attr, attrlen, hash);
			if (tp_delattr == &super_delattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
			if (!tp_delattr)
				break;
			attr_name_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = DeeType_invoke_attr_tp_delattr(iter, tp_delattr, self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			return result;
		}
	}
	err_unknown_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL);
err:
	return -1;
done:
	return result;
err_noaccess:
	err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL);
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
	if ((result = DeeType_SetCachedAttrStringHash(iter, self, attr, hash, value)) <= 0)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringHash(tp_self, iter, attr, hash)) != NULL) {
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
		    DeeType_HasMethodAttrStringHash(tp_self, iter, attr, hash))
			goto err_noaccess;
		if (iter->tp_getsets &&
		    (result = DeeType_SetGetSetAttrStringHash(tp_self, iter, self, attr, hash, value)) <= 0)
			goto done;
		if (iter->tp_members &&
		    (result = DeeType_SetMemberAttrStringHash(tp_self, iter, self, attr, hash, value)) <= 0)
			goto done;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			int (DCALL *tp_setattr)(DeeObject *, DeeObject *, DeeObject *);
do_iter_attr:
			tp_setattr = iter->tp_attr->tp_setattr;
			if (tp_setattr == &type_setattr)
				return DeeType_SetAttrStringHash((DeeTypeObject *)self, attr, hash, value);
			if (tp_setattr == &module_setattr)
				return DeeModule_SetAttrString((DeeModuleObject *)self, attr, hash, value);
			if (tp_setattr == &super_setattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
			if (!tp_setattr)
				break;
			attr_name_ob = DeeString_NewWithHash(attr, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = DeeType_invoke_attr_tp_setattr(iter, tp_setattr, self, attr_name_ob, value);
			Dee_Decref(attr_name_ob);
			return result;
		}
	}
	err_unknown_attribute_string(tp_self, attr, ATTR_ACCESS_SET);
err:
	return -1;
done:
	return result;
err_noaccess:
	err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_SET);
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
	if ((result = DeeType_SetCachedAttrStringLenHash(iter, self, attr, attrlen, hash, value)) <= 0)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringLenHash(tp_self, iter, attr, attrlen, hash)) != NULL) {
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
		    DeeType_HasMethodAttrStringLenHash(tp_self, iter, attr, attrlen, hash))
			goto err_noaccess;
		if (iter->tp_getsets &&
		    (result = DeeType_SetGetSetAttrStringLenHash(tp_self, iter, self, attr, attrlen, hash, value)) <= 0)
			goto done;
		if (iter->tp_members &&
		    (result = DeeType_SetMemberAttrStringLenHash(tp_self, iter, self, attr, attrlen, hash, value)) <= 0)
			goto done;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			int (DCALL *tp_setattr)(DeeObject *, DeeObject *, DeeObject *);
do_iter_attr:
			tp_setattr = iter->tp_attr->tp_setattr;
			if (tp_setattr == &type_setattr)
				return DeeType_SetAttrStringLenHash((DeeTypeObject *)self, attr, attrlen, hash, value);
			if (tp_setattr == &module_setattr)
				return DeeModule_SetAttrStringLen((DeeModuleObject *)self, attr, attrlen, hash, value);
			if (tp_setattr == &super_setattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
			if (!tp_setattr)
				break;
			attr_name_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = DeeType_invoke_attr_tp_setattr(iter, tp_setattr, self, attr_name_ob, value);
			Dee_Decref(attr_name_ob);
			return result;
		}
	}
	err_unknown_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET);
err:
	return -1;
done:
	return result;
err_noaccess:
	err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET);
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
	if ((result = DeeType_CallCachedAttrStringHash(iter, self, attr, hash, argc, argv)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringHash(tp_self, iter, attr, hash)) != NULL) {
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
		    (result = DeeType_CallMethodAttrStringHash(tp_self, iter, self, attr, hash, argc, argv)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttrStringHash(tp_self, iter, self, attr, hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttrStringHash(tp_self, iter, self, attr, hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
do_iter_attr:
			tp_getattr = iter->tp_attr->tp_getattr;
			if (tp_getattr == &type_getattr)
				return DeeType_CallAttrStringHash((DeeTypeObject *)self, attr, hash, argc, argv);
			if (tp_getattr == &module_getattr) {
				result = DeeModule_GetAttrString((DeeModuleObject *)self, attr, hash);
				goto done_invoke;
			}
			if (tp_getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (tp_getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttrString(((SeqEachBase *)self)->se_seq, attr, hash, argc, argv);
			if (tp_getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttrString(self, attr, hash, argc, argv);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!tp_getattr)
				break;
			attr_name_ob = DeeString_NewWithHash(attr, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			goto done_invoke;
		}
	}
	err_unknown_attribute_string(tp_self, attr, ATTR_ACCESS_GET);
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
	if ((result = DeeType_CallCachedAttrStringLenHash(iter, self, attr, attrlen, hash, argc, argv)) != ITER_DONE)
		goto done;
	for (;;) {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			if ((cattr = DeeType_QueryAttributeStringLenHash(tp_self, iter, attr, attrlen, hash)) != NULL) {
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
		    (result = DeeType_CallMethodAttrStringLenHash(tp_self, iter, self, attr, attrlen, hash, argc, argv)) != ITER_DONE)
			goto done;
		if (iter->tp_getsets &&
		    (result = DeeType_GetGetSetAttrStringLenHash(tp_self, iter, self, attr, attrlen, hash)) != ITER_DONE)
			goto done_invoke;
		if (iter->tp_members &&
		    (result = DeeType_GetMemberAttrStringLenHash(tp_self, iter, self, attr, attrlen, hash)) != ITER_DONE)
			goto done_invoke;
		iter = DeeType_Base(iter);
		if (!iter)
			break;
		if (iter->tp_attr) {
			DREF DeeObject *attr_name_ob;
			DREF DeeObject *(DCALL *tp_getattr)(DeeObject *, DeeObject *);
do_iter_attr:
			tp_getattr = iter->tp_attr->tp_getattr;
			if (tp_getattr == &type_getattr)
				return DeeType_CallAttrStringLenHash((DeeTypeObject *)self, attr, attrlen, hash, argc, argv);
			if (tp_getattr == &module_getattr) {
				result = DeeModule_GetAttrStringLen((DeeModuleObject *)self, attr, attrlen, hash);
				goto done_invoke;
			}
			if (tp_getattr == &super_getattr) {
				iter    = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = iter;
				goto again;
			}
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
			if (tp_getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
				return DeeSeqEach_CallAttrStringLen(((SeqEachBase *)self)->se_seq, attr, attrlen, hash, argc, argv);
			if (tp_getattr == (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
				return DeeSeqEach_CallAttrStringLen(self, attr, attrlen, hash, argc, argv);
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */
			if (!tp_getattr)
				break;
			attr_name_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
			if unlikely(!attr_name_ob)
				goto err;
			result = DeeType_invoke_attr_tp_getattr(iter, tp_getattr, self, attr_name_ob);
			Dee_Decref(attr_name_ob);
			goto done_invoke;
		}
	}
	err_unknown_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_GET);
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

/* @return: 1 : does exists
 * @return: 0 : doesn't exist
 * @return: -1: Error. */
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
/* DeeObject_TGeneric*Attr */
#define DEFINE_DeeObject_TGenericGetAttrStringHash
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericGetAttrStringLenHash
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericBoundAttrStringHash
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericBoundAttrStringLenHash
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericCallAttrStringHash
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericCallAttrStringLenHash
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericCallAttrStringHashKw
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericCallAttrStringLenHashKw
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericHasAttrStringHash
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericHasAttrStringLenHash
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericDelAttrStringHash
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericDelAttrStringLenHash
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericSetAttrStringHash
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericSetAttrStringLenHash
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericFindAttr
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericEnumAttr
#include "attribute-access-generic.c.inl"

/* DeeType_*Attr */
#define DEFINE_DeeType_GetAttrStringHash
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_GetAttrStringLenHash
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_BoundAttrStringHash
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_BoundAttrStringLenHash
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_CallAttrStringHash
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_CallAttrStringLenHash
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_CallAttrStringHashKw
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_CallAttrStringLenHashKw
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_HasAttrStringHash
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_HasAttrStringLenHash
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_DelAttrStringHash
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_DelAttrStringLenHash
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_SetAttrStringHash
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_SetAttrStringLenHash
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_FindAttr
#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_EnumAttr
#include "attribute-access-type.c.inl"

/* DeeType_*InstanceAttr */
#define DEFINE_DeeType_GetInstanceAttrStringHash
#include "attribute-access-type-instance.c.inl"
#define DEFINE_DeeType_BoundInstanceAttrStringHash
#include "attribute-access-type-instance.c.inl"
#define DEFINE_DeeType_CallInstanceAttrStringHashKw
#include "attribute-access-type-instance.c.inl"
#define DEFINE_DeeType_HasInstanceAttrStringHash
#include "attribute-access-type-instance.c.inl"
#define DEFINE_DeeType_DelInstanceAttrStringHash
#include "attribute-access-type-instance.c.inl"
#define DEFINE_DeeType_SetInstanceAttrStringHash
#include "attribute-access-type-instance.c.inl"

#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_RUNTIME_ATTRIBUTE_C */
