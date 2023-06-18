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
//#define DEFINE_DeeType_GetInstanceAttrString
//#define DEFINE_DeeType_GetInstanceAttrStringLen
//#define DEFINE_DeeType_BoundInstanceAttrString
//#define DEFINE_DeeType_BoundInstanceAttrStringLen
//#define DEFINE_DeeType_CallInstanceAttrString
//#define DEFINE_DeeType_CallInstanceAttrStringLen
//#define DEFINE_DeeType_CallInstanceAttrStringKw
//#define DEFINE_DeeType_CallInstanceAttrStringLenKw
//#define DEFINE_DeeType_CallInstanceAttrStringTuple
//#define DEFINE_DeeType_CallInstanceAttrStringLenTuple
//#define DEFINE_DeeType_CallInstanceAttrStringTupleKw
//#define DEFINE_DeeType_CallInstanceAttrStringLenTupleKw
//#define DEFINE_DeeType_VCallInstanceAttrStringf
//#define DEFINE_DeeType_VCallInstanceAttrStringLenf
#define DEFINE_DeeType_HasInstanceAttrString
//#define DEFINE_DeeType_HasInstanceAttrStringLen
//#define DEFINE_DeeType_DelInstanceAttrString
//#define DEFINE_DeeType_DelInstanceAttrStringLen
//#define DEFINE_DeeType_SetInstanceAttrString
//#define DEFINE_DeeType_SetInstanceAttrStringLen
//#define DEFINE_DeeType_FindInstanceAttr
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeType_GetInstanceAttrString) +            \
     defined(DEFINE_DeeType_GetInstanceAttrStringLen) +         \
     defined(DEFINE_DeeType_BoundInstanceAttrString) +          \
     defined(DEFINE_DeeType_BoundInstanceAttrStringLen) +       \
     defined(DEFINE_DeeType_CallInstanceAttrString) +           \
     defined(DEFINE_DeeType_CallInstanceAttrStringLen) +        \
     defined(DEFINE_DeeType_CallInstanceAttrStringKw) +         \
     defined(DEFINE_DeeType_CallInstanceAttrStringLenKw) +      \
     defined(DEFINE_DeeType_CallInstanceAttrStringTuple) +      \
     defined(DEFINE_DeeType_CallInstanceAttrStringLenTuple) +   \
     defined(DEFINE_DeeType_CallInstanceAttrStringTupleKw) +    \
     defined(DEFINE_DeeType_CallInstanceAttrStringLenTupleKw) + \
     defined(DEFINE_DeeType_VCallInstanceAttrStringf) +         \
     defined(DEFINE_DeeType_VCallInstanceAttrStringLenf) +      \
     defined(DEFINE_DeeType_HasInstanceAttrString) +            \
     defined(DEFINE_DeeType_HasInstanceAttrStringLen) +         \
     defined(DEFINE_DeeType_DelInstanceAttrString) +            \
     defined(DEFINE_DeeType_DelInstanceAttrStringLen) +         \
     defined(DEFINE_DeeType_SetInstanceAttrString) +            \
     defined(DEFINE_DeeType_SetInstanceAttrStringLen) +         \
     defined(DEFINE_DeeType_FindInstanceAttr)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

#ifdef DEFINE_DeeType_GetInstanceAttrString
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_GetInstanceAttrString
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_GetCachedInstanceAttr(self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_GetIInstanceMethodAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_GetIInstanceGetSetAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_GetIInstanceMemberAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_IS_GET
#elif defined(DEFINE_DeeType_GetInstanceAttrStringLen)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_GetInstanceAttrStringLen
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_GetCachedInstanceAttrLen(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_GetIInstanceMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_GetIInstanceGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_GetIInstanceMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_IS_GET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_BoundInstanceAttrString)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_BoundInstanceAttrString
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_BoundCachedInstanceAttr(self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMethodAttr(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasIInstanceGetSetAttr(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMemberAttr(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define LOCAL_IS_BOUND
#elif defined(DEFINE_DeeType_BoundInstanceAttrStringLen)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_BoundInstanceAttrStringLen
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_BoundCachedInstanceAttrLen(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasIInstanceGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define LOCAL_IS_BOUND
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallInstanceAttrString)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrString
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttr(self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttr(tp_invoker, tp_self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttr(tp_invoker, tp_self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttr(tp_invoker, tp_self, attr, hash, argc, argv)
#define LOCAL_IS_CALL
#elif defined(DEFINE_DeeType_CallInstanceAttrStringLen)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringLen
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrLen(self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)
#define LOCAL_IS_CALL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallInstanceAttrStringKw)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringKw
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrKw(self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)
#define LOCAL_IS_CALL_KW
#elif defined(DEFINE_DeeType_CallInstanceAttrStringLenKw)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringLenKw
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrLenKw(self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrLenKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrLenKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrLenKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_IS_CALL_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallInstanceAttrStringTuple)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringTuple
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrTuple(self, attr, hash, args)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrTuple(tp_invoker, tp_self, attr, hash, args)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrTuple(tp_invoker, tp_self, attr, hash, args)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrTuple(tp_invoker, tp_self, attr, hash, args)
#define LOCAL_IS_CALL_TUPLE
#elif defined(DEFINE_DeeType_CallInstanceAttrStringLenTuple)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringLenTuple
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrLenTuple(self, attr, attrlen, hash, args)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrLenTuple(tp_invoker, tp_self, attr, attrlen, hash, args)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrLenTuple(tp_invoker, tp_self, attr, attrlen, hash, args)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrLenTuple(tp_invoker, tp_self, attr, attrlen, hash, args)
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallInstanceAttrStringTupleKw)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringTupleKw
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrTupleKw(self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrTupleKw(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrTupleKw(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrTupleKw(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_IS_CALL_TUPLE_KW
#elif defined(DEFINE_DeeType_CallInstanceAttrStringLenTupleKw)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringLenTupleKw
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrLenTupleKw(self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrLenTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrLenTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrLenTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_VCallInstanceAttrStringf)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_VCallInstanceAttrStringf
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_VCallCachedInstanceAttrf(self, attr, hash, format, args)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_VCallIInstanceMethodAttrf(tp_invoker, tp_self, attr, hash, format, args)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_VCallIInstanceGetSetAttrf(tp_invoker, tp_self, attr, hash, format, args)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_VCallIInstanceMemberAttrf(tp_invoker, tp_self, attr, hash, format, args)
#define LOCAL_IS_VCALLF
#elif defined(DEFINE_DeeType_VCallInstanceAttrStringLenf)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_VCallInstanceAttrStringLenf
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_VCallCachedInstanceAttrLenf(self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_VCallIInstanceMethodAttrLenf(tp_invoker, tp_self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_VCallIInstanceGetSetAttrLenf(tp_invoker, tp_self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_VCallIInstanceMemberAttrLenf(tp_invoker, tp_self, attr, attrlen, hash, format, args)
#define LOCAL_IS_VCALLF
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_HasInstanceAttrString)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_HasInstanceAttrString
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_HasCachedInstanceAttr(self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_HasIInstanceMethodAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_HasIInstanceGetSetAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_HasIInstanceMemberAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_IS_HAS
#elif defined(DEFINE_DeeType_HasInstanceAttrStringLen)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_HasInstanceAttrStringLen
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_HasCachedInstanceAttrLen(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_HasIInstanceMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_HasIInstanceGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_HasIInstanceMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_IS_HAS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_DelInstanceAttrString)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_DelInstanceAttrString
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_DelCachedInstanceAttr(self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMethodAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasIInstanceGetSetAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMemberAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define LOCAL_IS_DEL
#elif defined(DEFINE_DeeType_DelInstanceAttrStringLen)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_DelInstanceAttrStringLen
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_DelCachedInstanceAttrLen(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasIInstanceGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define LOCAL_IS_DEL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetInstanceAttrString)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_SetInstanceAttrString
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_SetCachedInstanceAttr(self, attr, hash, value)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMethodAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasIInstanceGetSetAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMemberAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define LOCAL_IS_SET
#elif defined(DEFINE_DeeType_SetInstanceAttrStringLen)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_SetInstanceAttrStringLen
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_SetCachedInstanceAttrLen(self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasIInstanceGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
#define LOCAL_IS_SET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_FindInstanceAttr)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_FindInstanceAttr
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_FindCachedInstanceAttr(self, retinfo, rules)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_FindIInstanceMethodAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_FindIInstanceGetSetAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_FindIInstanceMemberAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_IS_FIND
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#if (defined(LOCAL_IS_CALL) || defined(LOCAL_IS_CALL_KW) ||             \
     defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW) || \
     defined(LOCAL_IS_VCALLF))
#define LOCAL_IS_CALL_LIKE
#endif /* ... */


DECL_BEGIN

#ifdef LOCAL_IS_SET
#define LOCAL_ATTR_ACCESS_OP ATTR_ACCESS_SET
#elif defined(LOCAL_IS_DEL)
#define LOCAL_ATTR_ACCESS_OP ATTR_ACCESS_DEL
#else /* ... */
#define LOCAL_ATTR_ACCESS_OP ATTR_ACCESS_GET
#endif /* !... */

#if defined(LOCAL_IS_GET) || defined(LOCAL_IS_CALL_LIKE)
#define LOCAL_return_t              DREF DeeObject *
#define LOCAL_ATTR_NOT_FOUND_RESULT ITER_DONE
#define LOCAL_ERROR_RETURN_VALUE    NULL
#elif defined(LOCAL_IS_HAS)
#define LOCAL_return_t              bool
#define LOCAL_ATTR_NOT_FOUND_RESULT false
#define LOCAL_ERROR_RETURN_VALUE    false
#elif defined(LOCAL_IS_DEL) || defined(LOCAL_IS_SET) || defined(LOCAL_IS_FIND)
#define LOCAL_return_t              int
#define LOCAL_ATTR_NOT_FOUND_RESULT 1
#define LOCAL_ERROR_RETURN_VALUE    (-1)
#else /* ... */
#define LOCAL_return_t              int
#define LOCAL_ATTR_NOT_FOUND_RESULT (-2)
#define LOCAL_ERROR_RETURN_VALUE    (-1)
#endif /* !... */

#if defined(DEFINE_DeeType_FindInstanceAttr)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3))
#elif defined(LOCAL_HAS_len) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 5))
#elif defined(LOCAL_IS_SET) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 4))
#else /* ... */
#define LOCAL_ATTR_NONNULL NONNULL((1, 2))
#endif /* !... */

/* Helpers for accessing attributes and performing operations. */
#ifdef LOCAL_HAS_len
#define LOCAL_DeeType_QueryInstanceAttribute(tp_invoker, tp_self) DeeType_QueryIInstanceAttributeStringLenWithHash(tp_invoker, tp_self, attr, attrlen, hash)
#else /* LOCAL_HAS_len */
#define LOCAL_DeeType_QueryInstanceAttribute(tp_invoker, tp_self) DeeType_QueryIInstanceAttributeStringWithHash(tp_invoker, tp_self, attr, hash)
#endif /* !LOCAL_HAS_len */

#ifdef LOCAL_IS_GET
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_GetInstanceAttribute(class_type, cattr)
#elif defined(LOCAL_IS_BOUND)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_BoundInstanceAttribute(class_type, cattr)
#elif defined(LOCAL_IS_DEL)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_DelInstanceAttribute(class_type, cattr)
#elif defined(LOCAL_IS_SET)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_SetInstanceAttribute(class_type, cattr, value)
#elif defined(LOCAL_IS_CALL)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_CallInstanceAttribute(class_type, cattr, argc, argv)
#elif defined(LOCAL_IS_CALL_KW)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_CallInstanceAttributeKw(class_type, cattr, argc, argv, kw)
#elif defined(LOCAL_IS_CALL_TUPLE)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_CallInstanceAttributeTuple(class_type, cattr, args)
#elif defined(LOCAL_IS_CALL_TUPLE_KW)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_CallInstanceAttributeTupleKw(class_type, cattr, args, kw)
#elif defined(LOCAL_IS_VCALLF)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_VCallInstanceAttributef(class_type, cattr, format, args)
#endif /* ... */


INTERN WUNUSED LOCAL_ATTR_NONNULL LOCAL_return_t
(DCALL LOCAL_DeeType_AccessInstanceAttr)(DeeTypeObject *self,
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
	LOCAL_return_t result;
	DeeTypeObject *iter;

	/* Use `tp_cache' and search for regular attributes, loading
	 * them as though they were the equivalent typing in INSTANCE-mode.
	 * Attributes that weren't found must then be searched for in
	 * the non-class fields, before also being added to the `tp_cache'
	 * cache.
	 * -> `tp_cache' is purely reserved for instance-attributes (tp_methods, etc.)
	 * -> `tp_class_cache' is used for class-attributes primarily,
	 *     with instance-attributes overlaid when those don't overlap
	 *     with class attributes of the same name.
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
	result = LOCAL_DeeType_AccessCachedInstanceAttr(self);
	if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
#ifdef LOCAL_IS_FIND
			result = DeeClass_FindIInstanceAttribute(self, iter, retinfo, rules);
			if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
				goto done;
#else /* LOCAL_IS_FIND */
			struct class_attribute *cattr;
			cattr = LOCAL_DeeType_QueryInstanceAttribute(self, iter);
			if (cattr != NULL) {
#ifdef LOCAL_IS_HAS
				return true;
#else /* LOCAL_IS_HAS */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return LOCAL_DeeClass_AccessInstanceAttribute(iter, cattr);
#endif /* !LOCAL_IS_HAS */
			}
#endif /* !LOCAL_IS_FIND */
		} else {
			if (iter->tp_methods) { /* Access instance methods using `DeeClsMethodObject' */
				result = LOCAL_DeeType_AccessIInstanceMethodAttr(self, iter);
				if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
					goto done;
			}
			if (iter->tp_getsets) { /* Access instance getsets using `DeeClsPropertyObject' */
				result = LOCAL_DeeType_AccessIInstanceGetSetAttr(self, iter);
				if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
					goto done;
			}
			if (iter->tp_members) { /* Access instance members using `DeeClsMemberObject' */
				result = LOCAL_DeeType_AccessIInstanceMemberAttr(self, iter);
				if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
					goto done;
			}
		}
	} while ((iter = DeeType_Base(iter)) != NULL);

#if !defined(LOCAL_IS_HAS) && !defined(LOCAL_IS_FIND)
#ifdef LOCAL_HAS_len
	err_unknown_attribute_len(self, attr, attrlen, LOCAL_ATTR_ACCESS_OP);
#else /* LOCAL_HAS_len */
	err_unknown_attribute(self, attr, LOCAL_ATTR_ACCESS_OP);
#endif /* !LOCAL_HAS_len */
err:
	return LOCAL_ERROR_RETURN_VALUE;
#endif /* !LOCAL_IS_HAS && !LOCAL_IS_FIND */
done:
	return result;
#undef LOCAL_invoke_result_OR_done
}

#undef LOCAL_return_t
#undef LOCAL_ATTR_NOT_FOUND_RESULT
#undef LOCAL_ERROR_RETURN_VALUE
#undef LOCAL_DeeType_AccessInstanceAttr
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
#undef LOCAL_ATTR_NONNULL
#undef LOCAL_ATTR_ACCESS_OP

#undef LOCAL_DeeType_QueryInstanceAttribute
#undef LOCAL_DeeClass_AccessInstanceAttribute

#undef LOCAL_DeeType_AccessInstanceAttr
#undef LOCAL_DeeType_AccessCachedInstanceAttr
#undef LOCAL_DeeType_AccessIInstanceMethodAttr
#undef LOCAL_DeeType_AccessIInstanceGetSetAttr
#undef LOCAL_DeeType_AccessIInstanceMemberAttr


DECL_END

#undef DEFINE_DeeType_GetInstanceAttrString
#undef DEFINE_DeeType_GetInstanceAttrStringLen
#undef DEFINE_DeeType_BoundInstanceAttrString
#undef DEFINE_DeeType_BoundInstanceAttrStringLen
#undef DEFINE_DeeType_CallInstanceAttrString
#undef DEFINE_DeeType_CallInstanceAttrStringLen
#undef DEFINE_DeeType_CallInstanceAttrStringKw
#undef DEFINE_DeeType_CallInstanceAttrStringLenKw
#undef DEFINE_DeeType_CallInstanceAttrStringTuple
#undef DEFINE_DeeType_CallInstanceAttrStringLenTuple
#undef DEFINE_DeeType_CallInstanceAttrStringTupleKw
#undef DEFINE_DeeType_CallInstanceAttrStringLenTupleKw
#undef DEFINE_DeeType_HasInstanceAttrString
#undef DEFINE_DeeType_HasInstanceAttrStringLen
#undef DEFINE_DeeType_DelInstanceAttrString
#undef DEFINE_DeeType_DelInstanceAttrStringLen
#undef DEFINE_DeeType_SetInstanceAttrString
#undef DEFINE_DeeType_SetInstanceAttrStringLen
#undef DEFINE_DeeType_FindInstanceAttr
