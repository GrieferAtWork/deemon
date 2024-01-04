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

/* Various attribute accessor functions which get special optimizations. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_getattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL type_delattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL type_setattr(DeeObject *self, DeeObject *attr, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL type_enumattr(DeeTypeObject *tp_self, DeeObject *self, denum_t proc, void *arg);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL module_getattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL module_delattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL module_setattr(DeeObject *self, DeeObject *attr, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL module_enumattr(DeeTypeObject *tp_self, DeeObject *self, denum_t proc, void *arg);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL super_getattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL super_delattr(DeeObject *self, DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL super_setattr(DeeObject *self, DeeObject *attr, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL super_enumattr(DeeTypeObject *tp_self, DeeObject *self, denum_t proc, void *arg);

#ifndef __INTELLISENSE__
DECL_END

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
#define DEFINE_DeeObject_VTGenericCallAttrStringHashf
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
//#define DEFINE_DeeObject_TGenericFindAttrInfoStringHash
//#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericFindAttrInfoStringLenHash
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
#define DEFINE_DeeType_VCallAttrStringHashf
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
//#define DEFINE_DeeType_FindAttrInfoStringHash
//#include "attribute-access-type.c.inl"
#define DEFINE_DeeType_FindAttrInfoStringLenHash
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

/* DeeObject_*Attr */
//#define DEFINE_DeeObject_TFindAttrInfoStringHash
//#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TFindAttrInfoStringLenHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_FindAttr
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_EnumAttr
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_HasAttr
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_HasAttrStringHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_HasAttrStringLenHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_BoundAttr
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_BoundAttrStringHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_BoundAttrStringLenHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_CallAttrKw
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_CallAttrStringHashKw
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_CallAttrStringLenHashKw
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_GetAttrStringHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_GetAttrStringLenHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_DelAttrStringHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_DelAttrStringLenHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_SetAttrStringHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_SetAttrStringLenHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_CallAttrStringHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_CallAttrStringLenHash
#include "attribute-access-object.c.inl"

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define DEFINE_DeeObject_CallAttrTuple
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_CallAttrTupleKw
#include "attribute-access-object.c.inl"
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

/* Raw object operator-level attribute access */
#define DEFINE_DeeObject_GetAttr
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_DelAttr
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_SetAttr
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_CallAttr
#include "attribute-access-object.c.inl"

#define DEFINE_DeeObject_TGetAttr
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TDelAttr
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TSetAttr
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TCallAttr
#include "attribute-access-object.c.inl"

/* Special-case optimizations for `VCallAttrf' */
#define DEFINE_DeeObject_VCallAttrf
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_VCallAttrStringHashf
#include "attribute-access-object.c.inl"

DECL_BEGIN
#endif /* !__INTELLISENSE__ */

PUBLIC WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeObject_VCallAttrStringLenHashf(DeeObject *self,
                                  char const *__restrict attr, size_t attrlen, dhash_t hash,
                                  char const *__restrict format, va_list args) {
	/* TODO: Encode `attr' as a string. */
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


#ifndef CONFIG_CALLTUPLE_OPTIMIZATIONS
PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_CallAttrTuple)(DeeObject *self,
                                /*String*/ DeeObject *attr,
                                DeeObject *args) {
	return DeeObject_CallAttr(self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_CallAttrTupleKw)(DeeObject *self,
                                  /*String*/ DeeObject *attr,
                                  DeeObject *args, DeeObject *kw) {
	return DeeObject_CallAttrKw(self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
}
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */


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



#ifdef CONFIG_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_VCallAttrPack, 16),
                    ASSEMBLY_NAME(DeeObject_CallAttr, 16));
#else /* !__NO_DEFINE_ALIAS */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_VCallAttrPack(DeeObject *self,
                        /*String*/ DeeObject *attr_name,
                        size_t argc, va_list args) {
	return DeeObject_CallAttr(self, attr_name, argc, (DeeObject **)args);
}
#endif /* __NO_DEFINE_ALIAS */
#else /* CONFIG_VA_LIST_IS_STACK_POINTER */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeObject_VCallAttrPack(DeeObject *self,
                        /*String*/ DeeObject *attr_name,
                        size_t argc, va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VPackSymbolic(argc, args);
	if unlikely(!args_tuple)
		goto err;
	result = DeeObject_CallAttr(self, attr_name, argc,
	                            DeeTuple_ELEM(args_tuple));
	DeeTuple_DecrefSymbolic(args_tuple);
	return result;
err:
	return NULL;
}
#endif /* !CONFIG_VA_LIST_IS_STACK_POINTER */

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
(DeeObject_CallAttrStringHashPack)(DeeObject *self,
                                   char const *__restrict attr,
                                   dhash_t hash, size_t argc, ...) {
	DREF DeeObject *result;
	va_list args;
	va_start(args, argc);
	result = DeeObject_VCallAttrStringHashPack(self, attr, hash, argc, args);
	va_end(args);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *
(DCALL DeeObject_VCallAttrStringf)(DeeObject *self,
                                   char const *__restrict attr,
                                   char const *__restrict format, va_list args) {
	return DeeObject_VCallAttrStringHashf(self, attr, Dee_HashStr(attr), format, args);
}

PUBLIC WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *
(DeeObject_CallAttrStringHashf)(DeeObject *self,
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
(DeeObject_CallAttrStringLenHashf)(DeeObject *self,
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

#endif /* !GUARD_DEEMON_RUNTIME_ATTRIBUTE_C */
