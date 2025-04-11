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
#ifndef GUARD_DEEMON_RUNTIME_ATTRIBUTE_C
#define GUARD_DEEMON_RUNTIME_ATTRIBUTE_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/attribute.h>
#include <deemon/object.h>
#include <deemon/system-features.h> /* bzero(), ... */
/**/

#include <stdarg.h> /* va_list */
#include <stddef.h> /* size_t */

DECL_BEGIN

/* Attribute access. */

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
//#define DEFINE_DeeObject_TGenericFindPrivateAttrInfoStringHash
//#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericFindPrivateAttrInfoStringLenHash
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericFindAttr
#include "attribute-access-generic.c.inl"
#define DEFINE_DeeObject_TGenericIterAttr
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
#define DEFINE_DeeType_IterAttr
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
#define DEFINE_DeeObject_TFindPrivateAttrInfoStringLenHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_FindAttr
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_IterAttr
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
#define DEFINE_DeeObject_TCallAttrKw
#include "attribute-access-object.c.inl"
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define DEFINE_DeeObject_TCallAttrTuple
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TCallAttrTupleKw
#include "attribute-access-object.c.inl"
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#define DEFINE_DeeObject_THasAttr
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TBoundAttr
#include "attribute-access-object.c.inl"

#define DEFINE_DeeObject_TGetAttrStringHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TDelAttrStringHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TSetAttrStringHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TCallAttrStringHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TCallAttrStringHashKw
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_THasAttrStringHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TBoundAttrStringHash
#include "attribute-access-object.c.inl"

#define DEFINE_DeeObject_TGetAttrStringLenHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TDelAttrStringLenHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TSetAttrStringLenHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TCallAttrStringLenHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TCallAttrStringLenHashKw
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_THasAttrStringLenHash
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TBoundAttrStringLenHash
#include "attribute-access-object.c.inl"



/* Special-case optimizations for `VCallAttrf' */
#define DEFINE_DeeObject_VCallAttrf
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_VCallAttrStringHashf
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TVCallAttrf
#include "attribute-access-object.c.inl"
#define DEFINE_DeeObject_TVCallAttrStringHashf
#include "attribute-access-object.c.inl"

DECL_BEGIN
#endif /* !__INTELLISENSE__ */

PUBLIC WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL
DeeObject_VCallAttrStringLenHashf(DeeObject *self,
                                  char const *__restrict attr, size_t attrlen, Dee_hash_t hash,
                                  char const *__restrict format, va_list args) {
	DREF DeeObject *result;
	char *attrcopy = (char *)Dee_Mallocac(attrlen + 1, sizeof(char));
	if unlikely(!attrcopy)
		goto err;
	*(char *)mempcpyc(attrcopy, attr, attrlen, sizeof(char)) = '\0';
	result = DeeObject_VCallAttrStringHashf(self, attr, hash, format, args);
	Dee_Freea(attrcopy);
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

PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *
(DCALL DeeObject_TCallAttrTuple)(DeeTypeObject *tp_self, DeeObject *self,
                                 /*String*/ DeeObject *attr,
                                 DeeObject *args) {
	return DeeObject_TCallAttr(tp_self, self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args));
}

PUBLIC WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *
(DCALL DeeObject_TCallAttrTupleKw)(DeeTypeObject *tp_self, DeeObject *self,
                                   /*String*/ DeeObject *attr,
                                   DeeObject *args, DeeObject *kw) {
	return DeeObject_TCallAttrKw(tp_self, self, attr, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw);
}
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */


PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_GetAttrString)(DeeObject *__restrict self,
                                char const *__restrict attr) {
	return DeeObject_GetAttrStringHash(self, attr, Dee_HashStr(attr));
}

/* >> DeeObject_HasAttr() -- deemon.hasattr(<self>, <attr>);
 * Check if `self' has an attribute `attr'. Same as the builtin `deemon.hasattr()'
 * function. Note that an attribute that is currently unbound, differs from one
 * that does not exist at all. This function will return `1' (true) for the former,
 * but `0' (false) for the later. During normal attribute access, this difference
 * is reflected by the type of exception: `UnboundAttribute' and `AttributeError'.
 * @return: >  0: Attribute exists
 * @return: == 0: Attribute doesn't exist
 * @return: <  0: An error was thrown. */
PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_HasAttrString)(DeeObject *__restrict self,
                                char const *__restrict attr) {
	return DeeObject_HasAttrStringHash(self, attr, Dee_HashStr(attr));
}

/* >> DeeObject_BoundAttr() -- <self>.<attr> is bound;
 * @return: Dee_BOUND_YES:     Attribute is bound.
 * @return: Dee_BOUND_NO:      Attribute isn't bound.
 * @return: Dee_BOUND_MISSING: The attribute doesn't exist.
 * @return: Dee_BOUND_ERR:     An error occurred. */
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
DEFINE_PUBLIC_ALIAS(DCALL_ASSEMBLY_NAME(DeeObject_VCallAttrPack, 16),
                    DCALL_ASSEMBLY_NAME(DeeObject_CallAttr, 16));
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
DEFINE_PUBLIC_ALIAS(DCALL_ASSEMBLY_NAME(DeeObject_VCallAttrStringPack, 16),
                    DCALL_ASSEMBLY_NAME(DeeObject_CallAttrString, 16));
DEFINE_PUBLIC_ALIAS(DCALL_ASSEMBLY_NAME(DeeObject_VCallAttrStringHashPack, 20),
                    DCALL_ASSEMBLY_NAME(DeeObject_CallAttrStringHash, 20));
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
                                          Dee_hash_t hash, size_t argc, va_list args) {
	return DeeObject_CallAttrStringHash(self, attr, hash, argc, (DeeObject **)args);
}
#endif /* __NO_DEFINE_ALIAS */
#else /* CONFIG_VA_LIST_IS_STACK_POINTER */
PUBLIC WUNUSED NONNULL((1, 2)) DREF DeeObject *
(DCALL DeeObject_VCallAttrStringHashPack)(DeeObject *self,
                                          char const *__restrict attr,
                                          Dee_hash_t hash, size_t argc, va_list args) {
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
                                   Dee_hash_t hash, size_t argc, ...) {
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
                                char const *__restrict attr, Dee_hash_t hash,
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
                                   size_t attrlen, Dee_hash_t hash,
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
