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
#ifdef __INTELLISENSE__
#include "attribute.c"
//#define DEFINE_DeeObject_TGenericGetAttrString
//#define DEFINE_DeeObject_TGenericGetAttrStringLen
//#define DEFINE_DeeObject_TGenericBoundAttrString
//#define DEFINE_DeeObject_TGenericBoundAttrStringLen
//#define DEFINE_DeeObject_TGenericCallAttrString
//#define DEFINE_DeeObject_TGenericCallAttrStringLen
//#define DEFINE_DeeObject_TGenericCallAttrStringKw
//#define DEFINE_DeeObject_TGenericCallAttrStringLenKw
//#define DEFINE_DeeObject_TGenericCallAttrStringTuple
//#define DEFINE_DeeObject_TGenericCallAttrStringLenTuple
//#define DEFINE_DeeObject_TGenericCallAttrStringTupleKw
//#define DEFINE_DeeObject_TGenericCallAttrStringLenTupleKw
//#define DEFINE_DeeObject_TGenericVCallAttrStringf
//#define DEFINE_DeeObject_TGenericVCallAttrStringLenf
//#define DEFINE_DeeObject_TGenericHasAttrString
//#define DEFINE_DeeObject_TGenericHasAttrStringLen
//#define DEFINE_DeeObject_TGenericDelAttrString
//#define DEFINE_DeeObject_TGenericDelAttrStringLen
//#define DEFINE_DeeObject_TGenericSetAttrString
//#define DEFINE_DeeObject_TGenericSetAttrStringLen
#define DEFINE_DeeObject_TGenericFindAttr
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeObject_TGenericGetAttrString) +            \
     defined(DEFINE_DeeObject_TGenericGetAttrStringLen) +         \
     defined(DEFINE_DeeObject_TGenericBoundAttrString) +          \
     defined(DEFINE_DeeObject_TGenericBoundAttrStringLen) +       \
     defined(DEFINE_DeeObject_TGenericCallAttrString) +           \
     defined(DEFINE_DeeObject_TGenericCallAttrStringLen) +        \
     defined(DEFINE_DeeObject_TGenericCallAttrStringKw) +         \
     defined(DEFINE_DeeObject_TGenericCallAttrStringLenKw) +      \
     defined(DEFINE_DeeObject_TGenericCallAttrStringTuple) +      \
     defined(DEFINE_DeeObject_TGenericCallAttrStringLenTuple) +   \
     defined(DEFINE_DeeObject_TGenericCallAttrStringTupleKw) +    \
     defined(DEFINE_DeeObject_TGenericCallAttrStringLenTupleKw) + \
     defined(DEFINE_DeeObject_TGenericVCallAttrStringf) +         \
     defined(DEFINE_DeeObject_TGenericVCallAttrStringLenf) +      \
     defined(DEFINE_DeeObject_TGenericHasAttrString) +            \
     defined(DEFINE_DeeObject_TGenericHasAttrStringLen) +         \
     defined(DEFINE_DeeObject_TGenericDelAttrString) +            \
     defined(DEFINE_DeeObject_TGenericDelAttrStringLen) +         \
     defined(DEFINE_DeeObject_TGenericSetAttrString) +            \
     defined(DEFINE_DeeObject_TGenericSetAttrStringLen) +         \
     defined(DEFINE_DeeObject_TGenericFindAttr)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

#ifdef DEFINE_DeeObject_TGenericGetAttrString
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericGetAttrString
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_GetCachedAttr(tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_GetMethodAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_GET
#elif defined(DEFINE_DeeObject_TGenericGetAttrStringLen)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericGetAttrStringLen
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_GetCachedAttrLen(tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_GetMethodAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_GET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericBoundAttrString)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericBoundAttrString
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_BoundCachedAttr(tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) (DeeType_HasMethodAttr(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_BoundGetSetAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_BoundMemberAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_BOUND
#elif defined(DEFINE_DeeObject_TGenericBoundAttrStringLen)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericBoundAttrStringLen
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_BoundCachedAttrLen(tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) (DeeType_HasMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_BoundGetSetAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_BoundMemberAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_BOUND
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericCallAttrString)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrString
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttr(tp_self, self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttr(tp_invoker, tp_self, self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_CALL
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringLen)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringLen
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrLen(tp_self, self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_CALL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringKw)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringKw
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrKw(tp_self, self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrKw(tp_invoker, tp_self, self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_CALL_KW
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringLenKw)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringLenKw
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrLenKw(tp_self, self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrLenKw(tp_invoker, tp_self, self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_CALL_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringTuple)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringTuple
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrTuple(tp_self, self, attr, hash, args)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrTuple(tp_invoker, tp_self, self, attr, hash, args)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_CALL_TUPLE
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringLenTuple)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringLenTuple
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrLenTuple(tp_self, self, attr, attrlen, hash, args)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrLenTuple(tp_invoker, tp_self, self, attr, attrlen, hash, args)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringTupleKw)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringTupleKw
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrTupleKw(tp_self, self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrTupleKw(tp_invoker, tp_self, self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_CALL_TUPLE_KW
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringLenTupleKw)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringLenTupleKw
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrLenTupleKw(tp_self, self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrLenTupleKw(tp_invoker, tp_self, self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericVCallAttrStringf)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericVCallAttrStringf
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_VCallCachedAttrf(tp_self, self, attr, hash, format, args)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_VCallMethodAttrf(tp_invoker, tp_self, self, attr, hash, format, args)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_VCALLF
#elif defined(DEFINE_DeeObject_TGenericVCallAttrStringLenf)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericVCallAttrStringLenf
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_VCallCachedAttrLenf(tp_self, self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_VCallMethodAttrLenf(tp_invoker, tp_self, self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_VCALLF
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericHasAttrString)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericHasAttrString
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_HasCachedAttr(tp_self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_HasMethodAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_HasGetSetAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_HasMemberAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_IS_HAS
#elif defined(DEFINE_DeeObject_TGenericHasAttrStringLen)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericHasAttrStringLen
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_HasCachedAttrLen(tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_HasMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_HasGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_HasMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_IS_HAS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericDelAttrString)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericDelAttrString
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_DelCachedAttr(tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) \
	(DeeType_HasMethodAttr(tp_invoker, tp_self, attr, hash)       \
	 ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL)  \
	 : 1)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_DelGetSetAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_DelMemberAttr(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_DEL
#elif defined(DEFINE_DeeObject_TGenericDelAttrStringLen)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericDelAttrStringLen
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_DelCachedAttrLen(tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self)             \
	(DeeType_HasMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash)       \
	 ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) \
	 : 1)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_DelGetSetAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_DelMemberAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_DEL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericSetAttrString)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericSetAttrString
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_SetCachedAttr(tp_self, self, attr, hash, value)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) \
	(DeeType_HasMethodAttr(tp_invoker, tp_self, attr, hash)       \
	 ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET)  \
	 : 1)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_SetGetSetAttr(tp_invoker, tp_self, self, attr, hash, value)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_SetMemberAttr(tp_invoker, tp_self, self, attr, hash, value)
#define LOCAL_IS_SET
#elif defined(DEFINE_DeeObject_TGenericSetAttrStringLen)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericSetAttrStringLen
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_SetCachedAttrLen(tp_self, self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self)             \
	(DeeType_HasMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash)       \
	 ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) \
	 : 1)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_SetGetSetAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_SetMemberAttrLen(tp_invoker, tp_self, self, attr, attrlen, hash, value)
#define LOCAL_IS_SET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericFindAttr)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericFindAttr
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_FindCachedAttr(tp_self, self, retinfo, rules)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_FindMethodAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_FindGetSetAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_FindMemberAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_IS_FIND
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#ifndef LOCAL_IS_HAS
#define LOCAL_HAS_self
#endif /* !LOCAL_IS_HAS */

#define LOCAL_HAS_tp_self

#if (defined(LOCAL_IS_CALL) || defined(LOCAL_IS_CALL_KW) ||             \
     defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW) || \
     defined(LOCAL_IS_VCALLF))
#define LOCAL_IS_CALL_LIKE
#endif /* ... */


DECL_BEGIN

#if defined(LOCAL_IS_GET) || defined(LOCAL_IS_CALL_LIKE)
#define LOCAL_return_t              DREF DeeObject *
#define LOCAL_ATTR_NOT_FOUND_RESULT ITER_DONE
#elif defined(LOCAL_IS_HAS)
#define LOCAL_return_t              bool
#define LOCAL_ATTR_NOT_FOUND_RESULT false
#elif defined(LOCAL_IS_DEL) || defined(LOCAL_IS_SET) || defined(LOCAL_IS_FIND)
#define LOCAL_return_t              int
#define LOCAL_ATTR_NOT_FOUND_RESULT 1
#else /* ... */
#define LOCAL_return_t              int
#define LOCAL_ATTR_NOT_FOUND_RESULT (-2)
#endif /* !... */

#if defined(DEFINE_DeeObject_TGenericFindAttr)
#define LOCAL_ATTR_NONNULL NONNULL((1, 3, 4))
#elif defined(LOCAL_HAS_tp_self) && defined(LOCAL_HAS_self) && defined(LOCAL_HAS_len) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3, 6))
#elif defined(LOCAL_HAS_tp_self) && defined(LOCAL_HAS_self) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3, 5))
#elif defined(LOCAL_HAS_tp_self) && defined(LOCAL_HAS_self)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3))
#elif (defined(LOCAL_HAS_tp_self) || defined(LOCAL_HAS_self)) && defined(LOCAL_HAS_len) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 5))
#elif (defined(LOCAL_HAS_tp_self) || defined(LOCAL_HAS_self)) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 4))
#elif defined(LOCAL_HAS_tp_self) || defined(LOCAL_HAS_self)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2))
#elif defined(LOCAL_HAS_len) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((4))
#elif (defined(LOCAL_IS_SET) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((3))
#else /* ... */
#define LOCAL_ATTR_NONNULL NONNULL((1))
#endif /* !... */


PUBLIC WUNUSED LOCAL_ATTR_NONNULL LOCAL_return_t
(DCALL LOCAL_DeeObject_TGenericAccessAttr)(/**/
#ifdef LOCAL_HAS_tp_self
                                           DeeTypeObject *tp_self,
#endif /* LOCAL_HAS_tp_self */
#ifdef LOCAL_HAS_self
                                           DeeObject *self,
#endif /* LOCAL_HAS_self */
#ifdef LOCAL_IS_FIND
                                           struct attribute_info *__restrict retinfo,
                                           struct attribute_lookup_rules const *__restrict rules
#else /* LOCAL_IS_FIND */
                                           char const *__restrict attr,
#ifdef LOCAL_HAS_len
                                           size_t attrlen,
#endif /* LOCAL_HAS_len */
                                           dhash_t hash
#ifdef LOCAL_IS_CALL
                                           , size_t argc, DeeObject *const *argv
#elif defined(LOCAL_IS_CALL_KW)
                                           , size_t argc, DeeObject *const *argv, DeeObject *kw
#elif defined(LOCAL_IS_CALL_TUPLE)
                                           , DeeObject *args
#elif defined(LOCAL_IS_CALL_TUPLE_KW)
                                           , DeeObject *args, DeeObject *kw
#elif defined(LOCAL_IS_VCALLF)
                                           , char const *__restrict format, va_list args
#endif /* LOCAL_IS_CALL */
#ifdef LOCAL_IS_SET
                                           , DeeObject *value
#endif /* LOCAL_IS_SET */
#endif /* !LOCAL_IS_FIND */
                                           ) {
#ifdef LOCAL_IS_CALL_LIKE
#define LOCAL_invoke_result_OR_done invoke_result
#else /* LOCAL_IS_CALL_LIKE */
#define LOCAL_invoke_result_OR_done done
#endif /* !LOCAL_IS_CALL_LIKE */
#ifndef LOCAL_HAS_tp_self
	DeeTypeObject *tp_self = Dee_TYPE(self);
#endif /* LOCAL_HAS_tp_self */
	LOCAL_return_t result;

	/* Verify arguments. */
#ifdef LOCAL_HAS_tp_self
	ASSERT_OBJECT_TYPE(tp_self, &DeeType_Type);
#endif /* LOCAL_HAS_tp_self */
#ifdef LOCAL_HAS_self
#ifdef LOCAL_HAS_tp_self
	ASSERT_OBJECT_TYPE(self, tp_self);
#else /* LOCAL_HAS_tp_self */
	ASSERT_OBJECT(self);
#endif /* !LOCAL_HAS_tp_self */
#endif /* LOCAL_HAS_self */

	/* Try to access the cached version of the attribute. */
	result = LOCAL_DeeType_AccessCachedAttr(tp_self, self);
	if (result == LOCAL_ATTR_NOT_FOUND_RESULT) {
		DeeTypeObject *iter = tp_self;
		do {
#ifdef LOCAL_IS_FIND
continue_at_iter:
			if (rules->alr_decl != NULL &&
			    rules->alr_decl != (DeeObject *)iter) {
				iter = DeeType_Base(iter);
				if (!iter)
					break;

				/* Also set tp_self, so we don't corrupt the cache by
				 * potentially failing to cache attributes that should
				 * have been visible. */
				tp_self = iter;
				goto continue_at_iter;
			}
#endif /* LOCAL_IS_FIND */

			if (iter->tp_methods) {
				result = LOCAL_DeeType_AccessMethodAttr(tp_self, iter, self);
				if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
					goto done;
			}
			if (iter->tp_getsets) {
				result = LOCAL_DeeType_AccessGetSetAttr(tp_self, iter, self);
				if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
					goto LOCAL_invoke_result_OR_done;
			}
			if (iter->tp_members) {
				result = LOCAL_DeeType_AccessMemberAttr(tp_self, iter, self);
				if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
					goto LOCAL_invoke_result_OR_done;
			}
		} while ((iter = DeeType_Base(iter)) != NULL);
	}
done:
	return result;
#ifdef LOCAL_IS_CALL_LIKE
invoke_result:
	if likely(result) {
		DREF DeeObject *real_result;
#ifdef LOCAL_IS_CALL
		real_result = DeeObject_Call(result, argc, argv);
#elif defined(LOCAL_IS_CALL_KW)
		real_result = DeeObject_CallKw(result, argc, argv, kw);
#elif defined(LOCAL_IS_CALL_TUPLE)
		real_result = DeeObject_CallTuple(result, args);
#elif defined(LOCAL_IS_CALL_TUPLE_KW)
		real_result = DeeObject_CallTupleKw(result, args, kw);
#elif defined(LOCAL_IS_VCALLF)
		real_result = DeeObject_VCallf(result, format, args);
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
		Dee_Decref(result);
		result = real_result;
	}
	return result;
#endif /* LOCAL_IS_CALL_LIKE */
#undef LOCAL_invoke_result_OR_done
}

#undef LOCAL_return_t
#undef LOCAL_ATTR_NOT_FOUND_RESULT
#undef LOCAL_DeeType_AccessCachedAttr
#undef LOCAL_DeeType_AccessMethodAttr
#undef LOCAL_DeeType_AccessGetSetAttr
#undef LOCAL_DeeType_AccessMemberAttr
#undef LOCAL_DeeObject_TGenericAccessAttr
#undef LOCAL_IS_GET
#undef LOCAL_IS_BOUND
#undef LOCAL_IS_CALL
#undef LOCAL_IS_CALL_KW
#undef LOCAL_IS_CALL_TUPLE
#undef LOCAL_IS_CALL_TUPLE_KW
#undef LOCAL_IS_VCALLF
#undef LOCAL_IS_CALL_LIKE
#undef LOCAL_IS_HAS
#undef LOCAL_IS_DEL
#undef LOCAL_IS_SET
#undef LOCAL_IS_FIND
#undef LOCAL_HAS_len
#undef LOCAL_HAS_tp_self
#undef LOCAL_HAS_self
#undef LOCAL_ATTR_NONNULL

DECL_END

#undef DEFINE_DeeObject_TGenericGetAttrString
#undef DEFINE_DeeObject_TGenericGetAttrStringLen
#undef DEFINE_DeeObject_TGenericBoundAttrString
#undef DEFINE_DeeObject_TGenericBoundAttrStringLen
#undef DEFINE_DeeObject_TGenericCallAttrString
#undef DEFINE_DeeObject_TGenericCallAttrStringLen
#undef DEFINE_DeeObject_TGenericCallAttrStringKw
#undef DEFINE_DeeObject_TGenericCallAttrStringLenKw
#undef DEFINE_DeeObject_TGenericCallAttrStringTuple
#undef DEFINE_DeeObject_TGenericCallAttrStringLenTuple
#undef DEFINE_DeeObject_TGenericCallAttrStringTupleKw
#undef DEFINE_DeeObject_TGenericCallAttrStringLenTupleKw
#undef DEFINE_DeeObject_TGenericHasAttrString
#undef DEFINE_DeeObject_TGenericHasAttrStringLen
#undef DEFINE_DeeObject_TGenericDelAttrString
#undef DEFINE_DeeObject_TGenericDelAttrStringLen
#undef DEFINE_DeeObject_TGenericSetAttrString
#undef DEFINE_DeeObject_TGenericSetAttrStringLen
#undef DEFINE_DeeObject_TGenericFindAttr
