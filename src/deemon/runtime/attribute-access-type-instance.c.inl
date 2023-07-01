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
//#define DEFINE_DeeType_GetInstanceAttrStringHash
//#define DEFINE_DeeType_GetInstanceAttrStringLenHash
//#define DEFINE_DeeType_BoundInstanceAttrStringHash
//#define DEFINE_DeeType_BoundInstanceAttrStringLenHash
//#define DEFINE_DeeType_CallInstanceAttrStringHash
//#define DEFINE_DeeType_CallInstanceAttrStringLenHash
//#define DEFINE_DeeType_CallInstanceAttrStringHashKw
//#define DEFINE_DeeType_CallInstanceAttrStringLenHashKw
//#define DEFINE_DeeType_CallInstanceAttrStringHashTuple
//#define DEFINE_DeeType_CallInstanceAttrStringLenHashTuple
//#define DEFINE_DeeType_CallInstanceAttrStringHashTupleKw
//#define DEFINE_DeeType_CallInstanceAttrStringLenHashTupleKw
#define DEFINE_DeeType_VCallInstanceAttrStringHashf
//#define DEFINE_DeeType_VCallInstanceAttrStringLenHashf
//#define DEFINE_DeeType_HasInstanceAttrStringHash
//#define DEFINE_DeeType_HasInstanceAttrStringLenHash
//#define DEFINE_DeeType_DelInstanceAttrStringHash
//#define DEFINE_DeeType_DelInstanceAttrStringLenHash
//#define DEFINE_DeeType_SetInstanceAttrStringHash
//#define DEFINE_DeeType_SetInstanceAttrStringLenHash
//#define DEFINE_DeeType_FindInstanceAttr
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeType_GetInstanceAttrStringHash) +            \
     defined(DEFINE_DeeType_GetInstanceAttrStringLenHash) +         \
     defined(DEFINE_DeeType_BoundInstanceAttrStringHash) +          \
     defined(DEFINE_DeeType_BoundInstanceAttrStringLenHash) +       \
     defined(DEFINE_DeeType_CallInstanceAttrStringHash) +           \
     defined(DEFINE_DeeType_CallInstanceAttrStringLenHash) +        \
     defined(DEFINE_DeeType_CallInstanceAttrStringHashKw) +         \
     defined(DEFINE_DeeType_CallInstanceAttrStringLenHashKw) +      \
     defined(DEFINE_DeeType_CallInstanceAttrStringHashTuple) +      \
     defined(DEFINE_DeeType_CallInstanceAttrStringLenHashTuple) +   \
     defined(DEFINE_DeeType_CallInstanceAttrStringHashTupleKw) +    \
     defined(DEFINE_DeeType_CallInstanceAttrStringLenHashTupleKw) + \
     defined(DEFINE_DeeType_VCallInstanceAttrStringHashf) +         \
     defined(DEFINE_DeeType_VCallInstanceAttrStringLenHashf) +      \
     defined(DEFINE_DeeType_HasInstanceAttrStringHash) +            \
     defined(DEFINE_DeeType_HasInstanceAttrStringLenHash) +         \
     defined(DEFINE_DeeType_DelInstanceAttrStringHash) +            \
     defined(DEFINE_DeeType_DelInstanceAttrStringLenHash) +         \
     defined(DEFINE_DeeType_SetInstanceAttrStringHash) +            \
     defined(DEFINE_DeeType_SetInstanceAttrStringLenHash) +         \
     defined(DEFINE_DeeType_FindInstanceAttr)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

#ifdef DEFINE_DeeType_GetInstanceAttrStringHash
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_GetInstanceAttrStringHash
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_GetCachedInstanceAttrStringHash(self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_GetIInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_GetIInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_GetIInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_IS_GET
#elif defined(DEFINE_DeeType_GetInstanceAttrStringLenHash)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_GetInstanceAttrStringLenHash
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_GetCachedInstanceAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_GetIInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_GetIInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_GetIInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_IS_GET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_BoundInstanceAttrStringHash)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_BoundInstanceAttrStringHash
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_BoundCachedInstanceAttrStringHash(self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasIInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define LOCAL_IS_BOUND
#elif defined(DEFINE_DeeType_BoundInstanceAttrStringLenHash)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_BoundInstanceAttrStringLenHash
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_BoundCachedInstanceAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasIInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define LOCAL_IS_BOUND
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallInstanceAttrStringHash)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringHash
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrStringHash(self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash, argc, argv)
#define LOCAL_IS_CALL
#elif defined(DEFINE_DeeType_CallInstanceAttrStringLenHash)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringLenHash
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrStringLenHash(self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)
#define LOCAL_IS_CALL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallInstanceAttrStringHashKw)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringHashKw
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrStringHashKw(self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrStringHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrStringHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrStringHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)
#define LOCAL_IS_CALL_KW
#elif defined(DEFINE_DeeType_CallInstanceAttrStringLenHashKw)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringLenHashKw
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrStringLenHashKw(self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_IS_CALL_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallInstanceAttrStringHashTuple)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringHashTuple
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrStringHashTuple(self, attr, hash, args)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrStringHashTuple(tp_invoker, tp_self, attr, hash, args)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrStringHashTuple(tp_invoker, tp_self, attr, hash, args)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrStringHashTuple(tp_invoker, tp_self, attr, hash, args)
#define LOCAL_IS_CALL_TUPLE
#elif defined(DEFINE_DeeType_CallInstanceAttrStringLenHashTuple)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringLenHashTuple
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrStringLenHashTuple(self, attr, attrlen, hash, args)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrStringLenHashTuple(tp_invoker, tp_self, attr, attrlen, hash, args)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrStringLenHashTuple(tp_invoker, tp_self, attr, attrlen, hash, args)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrStringLenHashTuple(tp_invoker, tp_self, attr, attrlen, hash, args)
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallInstanceAttrStringHashTupleKw)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringHashTupleKw
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrStringHashTupleKw(self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrStringHashTupleKw(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrStringHashTupleKw(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrStringHashTupleKw(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_IS_CALL_TUPLE_KW
#elif defined(DEFINE_DeeType_CallInstanceAttrStringLenHashTupleKw)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_CallInstanceAttrStringLenHashTupleKw
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_CallCachedInstanceAttrStringLenHashTupleKw(self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallIInstanceMethodAttrStringLenHashTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallIInstanceGetSetAttrStringLenHashTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallIInstanceMemberAttrStringLenHashTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_VCallInstanceAttrStringHashf)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_VCallInstanceAttrStringHashf
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_VCallCachedInstanceAttrStringHashf(self, attr, hash, format, args)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_VCallIInstanceMethodAttrStringHashf(tp_invoker, tp_self, attr, hash, format, args)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_VCallIInstanceGetSetAttrStringHashf(tp_invoker, tp_self, attr, hash, format, args)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_VCallIInstanceMemberAttrStringHashf(tp_invoker, tp_self, attr, hash, format, args)
#define LOCAL_IS_VCALLF
#elif defined(DEFINE_DeeType_VCallInstanceAttrStringLenHashf)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_VCallInstanceAttrStringLenHashf
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_VCallCachedInstanceAttrStringLenHashf(self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_VCallIInstanceMethodAttrStringLenHashf(tp_invoker, tp_self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_VCallIInstanceGetSetAttrStringLenHashf(tp_invoker, tp_self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_VCallIInstanceMemberAttrStringLenHashf(tp_invoker, tp_self, attr, attrlen, hash, format, args)
#define LOCAL_IS_VCALLF
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_HasInstanceAttrStringHash)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_HasInstanceAttrStringHash
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_HasCachedInstanceAttrStringHash(self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_HasIInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_HasIInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_HasIInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_IS_HAS
#elif defined(DEFINE_DeeType_HasInstanceAttrStringLenHash)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_HasInstanceAttrStringLenHash
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_HasCachedInstanceAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) DeeType_HasIInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) DeeType_HasIInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) DeeType_HasIInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_IS_HAS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_DelInstanceAttrStringHash)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_DelInstanceAttrStringHash
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_DelCachedInstanceAttrStringHash(self, attr, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasIInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define LOCAL_IS_DEL
#elif defined(DEFINE_DeeType_DelInstanceAttrStringLenHash)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_DelInstanceAttrStringLenHash
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_DelCachedInstanceAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasIInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define LOCAL_IS_DEL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetInstanceAttrStringHash)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_SetInstanceAttrStringHash
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_SetCachedInstanceAttrStringHash(self, attr, hash, value)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasIInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define LOCAL_IS_SET
#elif defined(DEFINE_DeeType_SetInstanceAttrStringLenHash)
#define LOCAL_DeeType_AccessInstanceAttr                             DeeType_SetInstanceAttrStringLenHash
#define LOCAL_DeeType_AccessCachedInstanceAttr(self)                 DeeType_SetCachedInstanceAttrStringLenHash(self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessIInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeType_AccessIInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasIInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeType_AccessIInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasIInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
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
#define LOCAL_DeeType_QueryInstanceAttribute(tp_invoker, tp_self) DeeType_QueryIInstanceAttributeStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#else /* LOCAL_HAS_len */
#define LOCAL_DeeType_QueryInstanceAttribute(tp_invoker, tp_self) DeeType_QueryIInstanceAttributeStringHash(tp_invoker, tp_self, attr, hash)
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
	err_unknown_attribute_string_len(self, attr, attrlen, LOCAL_ATTR_ACCESS_OP);
#else /* LOCAL_HAS_len */
	err_unknown_attribute_string(self, attr, LOCAL_ATTR_ACCESS_OP);
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

#undef DEFINE_DeeType_GetInstanceAttrStringHash
#undef DEFINE_DeeType_GetInstanceAttrStringLenHash
#undef DEFINE_DeeType_BoundInstanceAttrStringHash
#undef DEFINE_DeeType_BoundInstanceAttrStringLenHash
#undef DEFINE_DeeType_CallInstanceAttrStringHash
#undef DEFINE_DeeType_CallInstanceAttrStringLenHash
#undef DEFINE_DeeType_CallInstanceAttrStringHashKw
#undef DEFINE_DeeType_CallInstanceAttrStringLenHashKw
#undef DEFINE_DeeType_CallInstanceAttrStringHashTuple
#undef DEFINE_DeeType_CallInstanceAttrStringLenHashTuple
#undef DEFINE_DeeType_CallInstanceAttrStringHashTupleKw
#undef DEFINE_DeeType_CallInstanceAttrStringLenHashTupleKw
#undef DEFINE_DeeType_HasInstanceAttrStringHash
#undef DEFINE_DeeType_HasInstanceAttrStringLenHash
#undef DEFINE_DeeType_DelInstanceAttrStringHash
#undef DEFINE_DeeType_DelInstanceAttrStringLenHash
#undef DEFINE_DeeType_SetInstanceAttrStringHash
#undef DEFINE_DeeType_SetInstanceAttrStringLenHash
#undef DEFINE_DeeType_FindInstanceAttr
