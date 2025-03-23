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
#ifdef __INTELLISENSE__
#include "attribute.c"
//#define DEFINE_DeeObject_TGenericGetAttrStringHash
//#define DEFINE_DeeObject_TGenericGetAttrStringLenHash
//#define DEFINE_DeeObject_TGenericBoundAttrStringHash
//#define DEFINE_DeeObject_TGenericBoundAttrStringLenHash
//#define DEFINE_DeeObject_TGenericCallAttrStringHash
//#define DEFINE_DeeObject_TGenericCallAttrStringLenHash
//#define DEFINE_DeeObject_TGenericCallAttrStringHashKw
//#define DEFINE_DeeObject_TGenericCallAttrStringLenHashKw
//#define DEFINE_DeeObject_TGenericCallAttrStringHashTuple
//#define DEFINE_DeeObject_TGenericCallAttrStringLenHashTuple
//#define DEFINE_DeeObject_TGenericCallAttrStringHashTupleKw
//#define DEFINE_DeeObject_TGenericCallAttrStringLenHashTupleKw
//#define DEFINE_DeeObject_VTGenericCallAttrStringHashf
//#define DEFINE_DeeObject_VTGenericCallAttrStringLenHashf
//#define DEFINE_DeeObject_TGenericHasAttrStringHash
//#define DEFINE_DeeObject_TGenericHasAttrStringLenHash
//#define DEFINE_DeeObject_TGenericDelAttrStringHash
//#define DEFINE_DeeObject_TGenericDelAttrStringLenHash
//#define DEFINE_DeeObject_TGenericSetAttrStringHash
//#define DEFINE_DeeObject_TGenericSetAttrStringLenHash
//#define DEFINE_DeeObject_TGenericFindAttrInfoStringHash
//#define DEFINE_DeeObject_TGenericFindAttrInfoStringLenHash
//#define DEFINE_DeeObject_TGenericFindPrivateAttrInfoStringHash
//#define DEFINE_DeeObject_TGenericFindPrivateAttrInfoStringLenHash
//#define DEFINE_DeeObject_TGenericFindAttr
#define DEFINE_DeeObject_TGenericIterAttr
//#define DEFINE_DeeObject_TGenericEnumAttr
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>
#include <deemon/mro.h>
#include <deemon/object.h>

#if (defined(DEFINE_DeeObject_TGenericGetAttrStringHash) +                \
     defined(DEFINE_DeeObject_TGenericGetAttrStringLenHash) +             \
     defined(DEFINE_DeeObject_TGenericBoundAttrStringHash) +              \
     defined(DEFINE_DeeObject_TGenericBoundAttrStringLenHash) +           \
     defined(DEFINE_DeeObject_TGenericCallAttrStringHash) +               \
     defined(DEFINE_DeeObject_TGenericCallAttrStringLenHash) +            \
     defined(DEFINE_DeeObject_TGenericCallAttrStringHashKw) +             \
     defined(DEFINE_DeeObject_TGenericCallAttrStringLenHashKw) +          \
     defined(DEFINE_DeeObject_TGenericCallAttrStringHashTuple) +          \
     defined(DEFINE_DeeObject_TGenericCallAttrStringLenHashTuple) +       \
     defined(DEFINE_DeeObject_TGenericCallAttrStringHashTupleKw) +        \
     defined(DEFINE_DeeObject_TGenericCallAttrStringLenHashTupleKw) +     \
     defined(DEFINE_DeeObject_VTGenericCallAttrStringHashf) +             \
     defined(DEFINE_DeeObject_VTGenericCallAttrStringLenHashf) +          \
     defined(DEFINE_DeeObject_TGenericHasAttrStringHash) +                \
     defined(DEFINE_DeeObject_TGenericHasAttrStringLenHash) +             \
     defined(DEFINE_DeeObject_TGenericDelAttrStringHash) +                \
     defined(DEFINE_DeeObject_TGenericDelAttrStringLenHash) +             \
     defined(DEFINE_DeeObject_TGenericSetAttrStringHash) +                \
     defined(DEFINE_DeeObject_TGenericSetAttrStringLenHash) +             \
     defined(DEFINE_DeeObject_TGenericFindAttrInfoStringHash) +           \
     defined(DEFINE_DeeObject_TGenericFindAttrInfoStringLenHash) +        \
     defined(DEFINE_DeeObject_TGenericFindPrivateAttrInfoStringHash) +    \
     defined(DEFINE_DeeObject_TGenericFindPrivateAttrInfoStringLenHash) + \
     defined(DEFINE_DeeObject_TGenericFindAttr) +                         \
     defined(DEFINE_DeeObject_TGenericIterAttr) +                         \
     defined(DEFINE_DeeObject_TGenericEnumAttr)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

#ifdef DEFINE_DeeObject_TGenericGetAttrStringHash
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericGetAttrStringHash
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_GetCachedAttrStringHash(tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_GetMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_GET
#elif defined(DEFINE_DeeObject_TGenericGetAttrStringLenHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericGetAttrStringLenHash
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_GetCachedAttrStringLenHash(tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_GetMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_GET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericBoundAttrStringHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericBoundAttrStringHash
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_BoundCachedAttrStringHash(tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) (DeeType_HasMethodAttrStringHash(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_BoundGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_BoundMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_BOUND
#elif defined(DEFINE_DeeObject_TGenericBoundAttrStringLenHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericBoundAttrStringLenHash
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_BoundCachedAttrStringLenHash(tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) (DeeType_HasMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_BoundGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_BoundMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_BOUND
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringHash
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringHash(tp_self, self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_CALL
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringLenHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringLenHash
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringLenHash(tp_self, self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_CALL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringHashKw)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringHashKw
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringHashKw(tp_self, self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringHashKw(tp_invoker, tp_self, self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_CALL_KW
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringLenHashKw)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringLenHashKw
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringLenHashKw(tp_self, self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringLenHashKw(tp_invoker, tp_self, self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_CALL_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringHashTuple)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringHashTuple
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringHashTuple(tp_self, self, attr, hash, args)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringHashTuple(tp_invoker, tp_self, self, attr, hash, args)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_CALL_TUPLE
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringLenHashTuple)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringLenHashTuple
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringLenHashTuple(tp_self, self, attr, attrlen, hash, args)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringLenHashTuple(tp_invoker, tp_self, self, attr, attrlen, hash, args)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringHashTupleKw)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringHashTupleKw
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringHashTupleKw(tp_self, self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringHashTupleKw(tp_invoker, tp_self, self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_CALL_TUPLE_KW
#elif defined(DEFINE_DeeObject_TGenericCallAttrStringLenHashTupleKw)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericCallAttrStringLenHashTupleKw
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringLenHashTupleKw(tp_self, self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringLenHashTupleKw(tp_invoker, tp_self, self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_VTGenericCallAttrStringHashf)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_VTGenericCallAttrStringHashf
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_VCallCachedAttrStringHashf(tp_self, self, attr, hash, format, args)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_VCallMethodAttrStringHashf(tp_invoker, tp_self, self, attr, hash, format, args)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_VCALLF
#elif defined(DEFINE_DeeObject_VTGenericCallAttrStringLenHashf)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_VTGenericCallAttrStringLenHashf
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_VCallCachedAttrStringLenHashf(tp_self, self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_VCallMethodAttrStringLenHashf(tp_invoker, tp_self, self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_VCALLF
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericHasAttrStringHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericHasAttrStringHash
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_HasCachedAttrStringHash(tp_self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_HasMethodAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_HasGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_HasMemberAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_IS_HAS
#elif defined(DEFINE_DeeObject_TGenericHasAttrStringLenHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericHasAttrStringLenHash
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_HasCachedAttrStringLenHash(tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_HasMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_HasGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_HasMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_IS_HAS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericDelAttrStringHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericDelAttrStringHash
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_DelCachedAttrStringHash(tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_DelMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_DelGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_DelMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_IS_DEL
#elif defined(DEFINE_DeeObject_TGenericDelAttrStringLenHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericDelAttrStringLenHash
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_DelCachedAttrStringLenHash(tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_DelMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_DelGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_DelMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_IS_DEL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericSetAttrStringHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericSetAttrStringHash
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_SetCachedAttrStringHash(tp_self, self, attr, hash, value)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_SetMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash, value)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_SetGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash, value)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_SetMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash, value)
#define LOCAL_IS_SET
#elif defined(DEFINE_DeeObject_TGenericSetAttrStringLenHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericSetAttrStringLenHash
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_SetCachedAttrStringLenHash(tp_self, self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_SetMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_SetGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_SetMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash, value)
#define LOCAL_IS_SET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericFindAttrInfoStringHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericFindAttrInfoStringHash
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_FindCachedAttrInfoStringHash(tp_self, attr, hash, retinfo)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_FindMethodAttrInfoStringHash(tp_invoker, tp_self, attr, hash, retinfo)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_FindGetSetAttrInfoStringHash(tp_invoker, tp_self, attr, hash, retinfo)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_FindMemberAttrInfoStringHash(tp_invoker, tp_self, attr, hash, retinfo)
#define LOCAL_IS_FINDINFO
#elif defined(DEFINE_DeeObject_TGenericFindAttrInfoStringLenHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericFindAttrInfoStringLenHash
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_FindCachedAttrInfoStringLenHash(tp_self, attr, attrlen, hash, retinfo)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_FindMethodAttrInfoStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, retinfo)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_FindGetSetAttrInfoStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, retinfo)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_FindMemberAttrInfoStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, retinfo)
#define LOCAL_IS_FINDINFO
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericFindPrivateAttrInfoStringHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericFindPrivateAttrInfoStringHash
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_FindMethodAttrInfoStringHash(tp_invoker, tp_self, attr, hash, retinfo)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_FindGetSetAttrInfoStringHash(tp_invoker, tp_self, attr, hash, retinfo)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_FindMemberAttrInfoStringHash(tp_invoker, tp_self, attr, hash, retinfo)
#define LOCAL_IS_FINDINFO
#define LOCAL_IS_PRIVATE
#elif defined(DEFINE_DeeObject_TGenericFindPrivateAttrInfoStringLenHash)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericFindPrivateAttrInfoStringLenHash
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_FindMethodAttrInfoStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, retinfo)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_FindGetSetAttrInfoStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, retinfo)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_FindMemberAttrInfoStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, retinfo)
#define LOCAL_IS_FINDINFO
#define LOCAL_IS_PRIVATE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGenericFindAttr)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericFindAttr
#ifdef CONFIG_EXPERIMENTAL_ATTRITER
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_FindCachedAttr(tp_self, self, specs, result)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_FindMethodAttr(tp_invoker, tp_self, specs, result)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_FindGetSetAttr(tp_invoker, tp_self, specs, result)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_FindMemberAttr(tp_invoker, tp_self, specs, result)
#else /* CONFIG_EXPERIMENTAL_ATTRITER */
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_FindCachedAttr(tp_self, self, retinfo, rules)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_FindMethodAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_FindGetSetAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_FindMemberAttr(tp_invoker, tp_self, retinfo, rules)
#endif /* !CONFIG_EXPERIMENTAL_ATTRITER */
#define LOCAL_IS_FIND
#elif defined(DEFINE_DeeObject_TGenericIterAttr)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericIterAttr
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) type_method_iterattr(tp_self, (tp_self)->tp_methods, Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CANGET | Dee_ATTRPERM_F_CANCALL, Dee_attriterchain_builder_getiterbuf(&builder), Dee_attriterchain_builder_getbufsize(&builder))
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) type_getset_iterattr(tp_self, (tp_self)->tp_getsets, Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_PROPERTY, Dee_attriterchain_builder_getiterbuf(&builder), Dee_attriterchain_builder_getbufsize(&builder))
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) type_member_iterattr(tp_self, (tp_self)->tp_members, Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_CANGET, Dee_attriterchain_builder_getiterbuf(&builder), Dee_attriterchain_builder_getbufsize(&builder))
#define LOCAL_IS_ITER
#elif defined(DEFINE_DeeObject_TGenericEnumAttr)
#define LOCAL_DeeObject_TGenericAccessAttr                        DeeObject_TGenericEnumAttr
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) type_method_enum(tp_self, (tp_self)->tp_methods, Dee_ATTRPERM_F_IMEMBER, proc, arg)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) type_getset_enum(tp_self, (tp_self)->tp_getsets, Dee_ATTRPERM_F_IMEMBER | Dee_ATTRPERM_F_PROPERTY, proc, arg)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) type_member_enum(tp_self, (tp_self)->tp_members, Dee_ATTRPERM_F_IMEMBER, proc, arg)
#define LOCAL_IS_ENUM
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#if !defined(LOCAL_IS_HAS) && !defined(LOCAL_IS_FINDINFO) && !defined(LOCAL_IS_ITER) && !defined(LOCAL_IS_ENUM)
#define LOCAL_HAS_self
#endif /* !LOCAL_IS_HAS && !LOCAL_IS_FINDINFO && !LOCAL_IS_ITER && !LOCAL_IS_ENUM */

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
#elif defined(LOCAL_IS_HAS) || defined(LOCAL_IS_FINDINFO)
#define LOCAL_return_t              bool
#define LOCAL_ATTR_NOT_FOUND_RESULT false
#elif defined(LOCAL_IS_DEL) || defined(LOCAL_IS_SET) || defined(LOCAL_IS_FIND)
#define LOCAL_return_t              int
#define LOCAL_ATTR_NOT_FOUND_RESULT 1
#elif defined(LOCAL_IS_ITER)
#define LOCAL_return_t              size_t
#define LOCAL_ATTR_NOT_FOUND_RESULT DONT_USE_THIS_MACRO
#elif defined(LOCAL_IS_ENUM)
#define LOCAL_return_t              dssize_t
#define LOCAL_ATTR_NOT_FOUND_RESULT DONT_USE_THIS_MACRO
#else /* ... */
#define LOCAL_return_t              int
#define LOCAL_ATTR_NOT_FOUND_RESULT Dee_BOUND_MISSING
#endif /* !... */

#if defined(DEFINE_DeeObject_TGenericFindAttr)
#define LOCAL_ATTR_NONNULL NONNULL((1, 3, 4))
#elif defined(DEFINE_DeeObject_TGenericEnumAttr)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2))
#elif defined(LOCAL_HAS_tp_self) && defined(LOCAL_HAS_self) && defined(LOCAL_HAS_len) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_FINDINFO) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3, 6))
#elif defined(LOCAL_HAS_tp_self) && defined(LOCAL_HAS_self) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_FINDINFO) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3, 5))
#elif defined(LOCAL_HAS_tp_self) && defined(LOCAL_HAS_self)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3))
#elif (defined(LOCAL_HAS_tp_self) || defined(LOCAL_HAS_self)) && defined(LOCAL_HAS_len) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_FINDINFO) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 5))
#elif (defined(LOCAL_HAS_tp_self) || defined(LOCAL_HAS_self)) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_FINDINFO) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 4))
#elif defined(LOCAL_HAS_tp_self) || defined(LOCAL_HAS_self)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2))
#elif defined(LOCAL_HAS_len) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_FINDINFO) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((4))
#elif (defined(LOCAL_IS_SET) || defined(LOCAL_IS_FINDINFO) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((3))
#else /* ... */
#define LOCAL_ATTR_NONNULL NONNULL((1))
#endif /* !... */


PUBLIC WUNUSED LOCAL_ATTR_NONNULL LOCAL_return_t
(DCALL LOCAL_DeeObject_TGenericAccessAttr)(/**/
#ifdef LOCAL_HAS_tp_self
                                           DeeTypeObject *tp_self,
#endif /* LOCAL_HAS_tp_self */
#ifdef LOCAL_IS_ITER
                                           struct Dee_attriter *iterbuf, size_t bufsize,
                                           struct Dee_attrhint *__restrict hint
#elif defined(LOCAL_IS_ENUM)
                                           Dee_enum_t proc, void *arg
#else /* LOCAL_IS_ITER */
#ifdef LOCAL_HAS_self
                                           DeeObject *self,
#endif /* LOCAL_HAS_self */
#ifdef LOCAL_IS_FIND
#ifdef CONFIG_EXPERIMENTAL_ATTRITER
                                           struct Dee_attrspec const *__restrict specs,
                                           struct Dee_attrdesc *__restrict result
#else /* CONFIG_EXPERIMENTAL_ATTRITER */
                                           struct Dee_attribute_info *__restrict retinfo,
                                           struct Dee_attribute_lookup_rules const *__restrict rules
#endif /* !CONFIG_EXPERIMENTAL_ATTRITER */
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
#ifdef LOCAL_IS_FINDINFO
                                           , struct Dee_attrinfo *__restrict retinfo
#endif /* LOCAL_IS_FINDINFO */
#endif /* !LOCAL_IS_FIND */
#endif /* !LOCAL_IS_ITER */
                                           ) {
#ifdef LOCAL_IS_CALL_LIKE
#define LOCAL_invoke_retval_OR_done invoke_retval
#else /* LOCAL_IS_CALL_LIKE */
#define LOCAL_invoke_retval_OR_done done
#endif /* !LOCAL_IS_CALL_LIKE */
#ifndef LOCAL_HAS_tp_self
	DeeTypeObject *tp_self = Dee_TYPE(self);
#endif /* LOCAL_HAS_tp_self */
	LOCAL_return_t retval;
#ifdef LOCAL_IS_ITER
	struct Dee_attriterchain_builder builder;
	Dee_attriterchain_builder_init(&builder, iterbuf, bufsize);
#elif defined(LOCAL_IS_ENUM)
	LOCAL_return_t final_retval = 0;
#endif /* LOCAL_IS_ENUM */

	/* Verify arguments. */
#ifdef LOCAL_HAS_tp_self
	ASSERT_OBJECT_TYPE(tp_self, &DeeType_Type);
#endif /* LOCAL_HAS_tp_self */
#ifdef LOCAL_HAS_self
#ifdef LOCAL_HAS_tp_self
	ASSERT_OBJECT_TYPE_A(self, tp_self);
#else /* LOCAL_HAS_tp_self */
	ASSERT_OBJECT(self);
#endif /* !LOCAL_HAS_tp_self */
#endif /* LOCAL_HAS_self */

	/* Try to access the cached version of the attribute. */
#ifdef LOCAL_DeeType_AccessCachedAttr
	retval = LOCAL_DeeType_AccessCachedAttr(tp_self, self);
	if (retval == LOCAL_ATTR_NOT_FOUND_RESULT)
#endif /* LOCAL_DeeType_AccessCachedAttr */
	{
#ifndef LOCAL_IS_PRIVATE
		DeeTypeMRO mro;
#endif /* !LOCAL_IS_PRIVATE */
		DeeTypeObject *iter = tp_self;
#ifndef LOCAL_IS_PRIVATE
		DeeTypeMRO_Init(&mro, iter);
#endif /* !LOCAL_IS_PRIVATE */
		do {
#ifdef LOCAL_IS_FIND
continue_at_iter:
#ifdef CONFIG_EXPERIMENTAL_ATTRITER
			if (specs->as_decl != NULL &&
			    specs->as_decl != (DeeObject *)iter)
#else /* CONFIG_EXPERIMENTAL_ATTRITER */
			if (rules->alr_decl != NULL &&
			    rules->alr_decl != (DeeObject *)iter)
#endif /* !CONFIG_EXPERIMENTAL_ATTRITER */
			{
#ifdef LOCAL_IS_PRIVATE
				break;
#else /* LOCAL_IS_PRIVATE */
				iter = DeeTypeMRO_Next(&mro, iter);
				if (!iter)
					break;

				/* Also set `tp_self', so we don't corrupt the cache by
				 * potentially failing to cache attributes that should
				 * have been visible. */
				tp_self = iter;
				goto continue_at_iter;
#endif /* !LOCAL_IS_PRIVATE */
			}
#endif /* LOCAL_IS_FIND */

#ifdef LOCAL_IS_ITER
#define LOCAL_process_retval(retval, done) \
	if unlikely(retval == (size_t)-1)      \
		goto err;                          \
	Dee_attriterchain_builder_consume(&builder, retval)
#elif defined(LOCAL_IS_ENUM)
#define LOCAL_process_retval(retval, done) \
	if unlikely(retval < 0)                \
		goto err;                          \
	final_retval += retval
#else /* LOCAL_IS_ENUM */
#define LOCAL_process_retval(retval, done)     \
	if (retval != LOCAL_ATTR_NOT_FOUND_RESULT) \
		goto done
#endif /* !LOCAL_IS_ENUM */

			if (iter->tp_methods) {
				retval = LOCAL_DeeType_AccessMethodAttr(tp_self, iter, self);
				LOCAL_process_retval(retval, done);
			}
			if (iter->tp_getsets) {
				retval = LOCAL_DeeType_AccessGetSetAttr(tp_self, iter, self);
				LOCAL_process_retval(retval, LOCAL_invoke_retval_OR_done);
			}
			if (iter->tp_members) {
				retval = LOCAL_DeeType_AccessMemberAttr(tp_self, iter, self);
				LOCAL_process_retval(retval, LOCAL_invoke_retval_OR_done);
			}

#undef LOCAL_process_retval
		}
#ifdef LOCAL_IS_PRIVATE
		__WHILE0;
		retval = LOCAL_ATTR_NOT_FOUND_RESULT;
#else /* LOCAL_IS_PRIVATE */
		while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL);
#endif /* !LOCAL_IS_PRIVATE */
	}
#if !defined(LOCAL_IS_ENUM) && !defined(LOCAL_IS_ITER)
done:
	return retval;
#else /* !LOCAL_IS_ENUM && !LOCAL_IS_ITER */
#ifdef LOCAL_IS_ITER
	return Dee_attriterchain_builder_pack(&builder);
#else /* LOCAL_IS_ITER */
	return final_retval;
#endif /* !LOCAL_IS_ITER */
err:
#ifdef LOCAL_IS_ITER
	Dee_attriterchain_builder_fini(&builder);
#endif /* LOCAL_IS_ITER */
	return retval;
#endif /* LOCAL_IS_ENUM || LOCAL_IS_ITER */
#ifdef LOCAL_IS_CALL_LIKE
invoke_retval:
	if likely(retval) {
		DREF DeeObject *real_retval;
#ifdef LOCAL_IS_CALL
		real_retval = DeeObject_Call(retval, argc, argv);
#elif defined(LOCAL_IS_CALL_KW)
		real_retval = DeeObject_CallKw(retval, argc, argv, kw);
#elif defined(LOCAL_IS_CALL_TUPLE)
		real_retval = DeeObject_CallTuple(retval, args);
#elif defined(LOCAL_IS_CALL_TUPLE_KW)
		real_retval = DeeObject_CallTupleKw(retval, args, kw);
#elif defined(LOCAL_IS_VCALLF)
		real_retval = DeeObject_VCallf(retval, format, args);
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
		Dee_Decref(retval);
		retval = real_retval;
	}
	return retval;
#endif /* LOCAL_IS_CALL_LIKE */
#undef LOCAL_invoke_retval_OR_done
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
#undef LOCAL_IS_FINDINFO
#undef LOCAL_IS_PRIVATE
#undef LOCAL_IS_FIND
#undef LOCAL_IS_ITER
#undef LOCAL_IS_ENUM
#undef LOCAL_HAS_len
#undef LOCAL_HAS_tp_self
#undef LOCAL_HAS_self
#undef LOCAL_ATTR_NONNULL

DECL_END

#undef DEFINE_DeeObject_TGenericGetAttrStringHash
#undef DEFINE_DeeObject_TGenericGetAttrStringLenHash
#undef DEFINE_DeeObject_TGenericBoundAttrStringHash
#undef DEFINE_DeeObject_TGenericBoundAttrStringLenHash
#undef DEFINE_DeeObject_TGenericCallAttrStringHash
#undef DEFINE_DeeObject_TGenericCallAttrStringLenHash
#undef DEFINE_DeeObject_TGenericCallAttrStringHashKw
#undef DEFINE_DeeObject_TGenericCallAttrStringLenHashKw
#undef DEFINE_DeeObject_TGenericCallAttrStringHashTuple
#undef DEFINE_DeeObject_TGenericCallAttrStringLenHashTuple
#undef DEFINE_DeeObject_TGenericCallAttrStringHashTupleKw
#undef DEFINE_DeeObject_TGenericCallAttrStringLenHashTupleKw
#undef DEFINE_DeeObject_VTGenericCallAttrStringHashf
#undef DEFINE_DeeObject_VTGenericCallAttrStringLenHashf
#undef DEFINE_DeeObject_TGenericHasAttrStringHash
#undef DEFINE_DeeObject_TGenericHasAttrStringLenHash
#undef DEFINE_DeeObject_TGenericDelAttrStringHash
#undef DEFINE_DeeObject_TGenericDelAttrStringLenHash
#undef DEFINE_DeeObject_TGenericSetAttrStringHash
#undef DEFINE_DeeObject_TGenericSetAttrStringLenHash
#undef DEFINE_DeeObject_TGenericFindAttrInfoStringHash
#undef DEFINE_DeeObject_TGenericFindAttrInfoStringLenHash
#undef DEFINE_DeeObject_TGenericFindPrivateAttrInfoStringHash
#undef DEFINE_DeeObject_TGenericFindPrivateAttrInfoStringLenHash
#undef DEFINE_DeeObject_TGenericFindAttr
#undef DEFINE_DeeObject_TGenericIterAttr
#undef DEFINE_DeeObject_TGenericEnumAttr
