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
//#define DEFINE_DeeType_GetAttrStringHash
//#define DEFINE_DeeType_GetAttrStringLenHash
//#define DEFINE_DeeType_BoundAttrStringHash
//#define DEFINE_DeeType_BoundAttrStringLenHash
//#define DEFINE_DeeType_CallAttrStringHash
//#define DEFINE_DeeType_CallAttrStringLenHash
//#define DEFINE_DeeType_CallAttrStringHashKw
//#define DEFINE_DeeType_CallAttrStringLenHashKw
//#define DEFINE_DeeType_CallAttrStringHashTuple
//#define DEFINE_DeeType_CallAttrStringLenHashTuple
//#define DEFINE_DeeType_CallAttrStringHashTupleKw
//#define DEFINE_DeeType_CallAttrStringLenHashTupleKw
//#define DEFINE_DeeType_VCallAttrStringHashf
//#define DEFINE_DeeType_VCallAttrStringLenHashf
//#define DEFINE_DeeType_HasAttrStringHash
//#define DEFINE_DeeType_HasAttrStringLenHash
//#define DEFINE_DeeType_DelAttrStringHash
//#define DEFINE_DeeType_DelAttrStringLenHash
//#define DEFINE_DeeType_SetAttrStringHash
//#define DEFINE_DeeType_SetAttrStringLenHash
//#define DEFINE_DeeType_FindAttrInfoStringHash
#define DEFINE_DeeType_FindAttrInfoStringLenHash
//#define DEFINE_DeeType_FindAttr
//#define DEFINE_DeeType_EnumAttr
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>
#include <deemon/class.h>
#include <deemon/mro.h>
#include <deemon/object.h>

#include "runtime_error.h"

#if (defined(DEFINE_DeeType_GetAttrStringHash) +            \
     defined(DEFINE_DeeType_GetAttrStringLenHash) +         \
     defined(DEFINE_DeeType_BoundAttrStringHash) +          \
     defined(DEFINE_DeeType_BoundAttrStringLenHash) +       \
     defined(DEFINE_DeeType_CallAttrStringHash) +           \
     defined(DEFINE_DeeType_CallAttrStringLenHash) +        \
     defined(DEFINE_DeeType_CallAttrStringHashKw) +         \
     defined(DEFINE_DeeType_CallAttrStringLenHashKw) +      \
     defined(DEFINE_DeeType_CallAttrStringHashTuple) +      \
     defined(DEFINE_DeeType_CallAttrStringLenHashTuple) +   \
     defined(DEFINE_DeeType_CallAttrStringHashTupleKw) +    \
     defined(DEFINE_DeeType_CallAttrStringLenHashTupleKw) + \
     defined(DEFINE_DeeType_VCallAttrStringHashf) +         \
     defined(DEFINE_DeeType_VCallAttrStringLenHashf) +      \
     defined(DEFINE_DeeType_HasAttrStringHash) +            \
     defined(DEFINE_DeeType_HasAttrStringLenHash) +         \
     defined(DEFINE_DeeType_DelAttrStringHash) +            \
     defined(DEFINE_DeeType_DelAttrStringLenHash) +         \
     defined(DEFINE_DeeType_SetAttrStringHash) +            \
     defined(DEFINE_DeeType_SetAttrStringLenHash) +         \
     defined(DEFINE_DeeType_FindAttrInfoStringHash) +       \
     defined(DEFINE_DeeType_FindAttrInfoStringLenHash) +    \
     defined(DEFINE_DeeType_FindAttr) +                     \
     defined(DEFINE_DeeType_EnumAttr)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

#ifdef DEFINE_DeeType_GetAttrStringHash
#define LOCAL_DeeType_AccessAttr                                    DeeType_GetAttrStringHash
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_GetCachedClassAttrStringHash(self, attr, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_GetClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_GetClassGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_GetClassMemberAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_GetInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_GetInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_GetInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericGetAttrStringHash(self, attr, hash)
#define LOCAL_IS_GET
#elif defined(DEFINE_DeeType_GetAttrStringLenHash)
#define LOCAL_DeeType_AccessAttr                                    DeeType_GetAttrStringLenHash
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_GetCachedClassAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_GetClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_GetClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_GetClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_GetInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_GetInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_GetInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericGetAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_IS_GET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_BoundAttrStringHash)
#define LOCAL_DeeType_AccessAttr                                    DeeType_BoundAttrStringHash
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_BoundCachedClassAttrStringHash(self, attr, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_BoundClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_BoundClassGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_BoundClassMemberAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_BoundInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_BoundInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_BoundInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericBoundAttrStringHash(self, attr, hash)
#define LOCAL_IS_BOUND
#elif defined(DEFINE_DeeType_BoundAttrStringLenHash)
#define LOCAL_DeeType_AccessAttr                                    DeeType_BoundAttrStringLenHash
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_BoundCachedClassAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_BoundClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_BoundClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_BoundClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_BoundInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_BoundInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_BoundInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericBoundAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_IS_BOUND
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallAttrStringHash)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringHash
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrStringHash(self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash, argc, argv)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringHash(self, attr, hash, argc, argv)
#define LOCAL_IS_CALL
#elif defined(DEFINE_DeeType_CallAttrStringLenHash)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringLenHash
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrStringLenHash(self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringLenHash(self, attr, attrlen, hash, argc, argv)
#define LOCAL_IS_CALL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallAttrStringHashKw)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringHashKw
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrStringHashKw(self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrStringHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrStringHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrStringHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrStringHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringHashKw(self, attr, hash, argc, argv, kw)
#define LOCAL_IS_CALL_KW
#elif defined(DEFINE_DeeType_CallAttrStringLenHashKw)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringLenHashKw
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrStringLenHashKw(self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringLenHashKw(self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_IS_CALL_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallAttrStringHashTuple)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringHashTuple
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrStringHashTuple(self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrStringHashTuple(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrStringHashTuple(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrStringHashTuple(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrStringHashTuple(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringHashTuple(self, attr, hash, args, kw)
#define LOCAL_IS_CALL_TUPLE
#elif defined(DEFINE_DeeType_CallAttrStringLenHashTuple)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringLenHashTuple
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrStringLenHashTuple(self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrStringLenHashTuple(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrStringLenHashTuple(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrStringLenHashTuple(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrStringLenHashTuple(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringLenHashTuple(self, attr, attrlen, hash, args, kw)
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallAttrStringHashTupleKw)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringHashTupleKw
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrStringHashTupleKw(self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrStringHashTupleKw(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrStringHashTupleKw(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrStringHashTupleKw(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrStringHashTupleKw(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringHashTupleKw(self, attr, hash, args, kw)
#define LOCAL_IS_CALL_TUPLE_KW
#elif defined(DEFINE_DeeType_CallAttrStringLenHashTupleKw)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringLenHashTupleKw
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrStringLenHashTupleKw(self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrStringLenHashTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrStringLenHashTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrStringLenHashTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrStringLenHashTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringLenHashTupleKw(self, attr, attrlen, hash, args, kw)
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_VCallAttrStringHashf)
#define LOCAL_DeeType_AccessAttr                                    DeeType_VCallAttrStringHashf
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_VCallCachedClassAttrStringHashf(self, attr, hash, format, args)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_VCallClassMethodAttrStringHashf(tp_invoker, tp_self, attr, hash, format, args)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_VCallInstanceMethodAttrStringHashf(tp_invoker, tp_self, attr, hash, format, args)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_VCallInstanceGetSetAttrStringHashf(tp_invoker, tp_self, attr, hash, format, args)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_VCallInstanceMemberAttrStringHashf(tp_invoker, tp_self, attr, hash, format, args)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_VGenericCallAttrStringHashf(self, attr, hash, format, args)
#define LOCAL_IS_VCALLF
#elif defined(DEFINE_DeeType_VCallAttrStringLenHashf)
#define LOCAL_DeeType_AccessAttr                                    DeeType_VCallAttrStringLenHashf
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_VCallCachedClassAttrStringLenHashf(self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_VCallClassMethodAttrStringLenHashf(tp_invoker, tp_self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_VCallInstanceMethodAttrLenHashf(tp_invoker, tp_self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_VCallInstanceGetSetAttrLenHashf(tp_invoker, tp_self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_VCallInstanceMemberAttrLenHashf(tp_invoker, tp_self, attr, attrlen, hash, format, args)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_VGenericCallAttrStringLenHashf(self, attr, attrlen, hash, format, args)
#define LOCAL_IS_VCALLF
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_HasAttrStringHash)
#define LOCAL_DeeType_AccessAttr                                    DeeType_HasAttrStringHash
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_HasCachedClassAttrStringHash(self, attr, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_HasClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_HasClassGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_HasClassMemberAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_HasInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_HasInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_HasInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericHasAttrStringHash(self, attr, hash)
#define LOCAL_IS_HAS
#elif defined(DEFINE_DeeType_HasAttrStringLenHash)
#define LOCAL_DeeType_AccessAttr                                    DeeType_HasAttrStringLenHash
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_HasCachedClassAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_HasClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_HasClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_HasClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_HasInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_HasInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_HasInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericHasAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_IS_HAS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_DelAttrStringHash)
#define LOCAL_DeeType_AccessAttr                                    DeeType_DelAttrStringHash
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_DelCachedClassAttrStringHash(self, attr, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_DelClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_DelClassGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_DelClassMemberAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_DelInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_DelInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_DelInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericDelAttrStringHash(self, attr, hash)
#define LOCAL_IS_DEL
#elif defined(DEFINE_DeeType_DelAttrStringLenHash)
#define LOCAL_DeeType_AccessAttr                                    DeeType_DelAttrStringLenHash
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_DelCachedClassAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_DelClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_DelClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_DelClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_DelInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_DelInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_DelInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericDelAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_IS_DEL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetAttrStringHash)
#define LOCAL_DeeType_AccessAttr                                    DeeType_SetAttrStringHash
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_SetCachedClassAttrStringHash(self, attr, hash, value)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_SetClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash, value)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_SetClassGetSetAttrStringHash(tp_invoker, tp_self, attr, hash, value)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_SetClassMemberAttrStringHash(tp_invoker, tp_self, attr, hash, value)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_SetInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash, value)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_SetInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash, value)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_SetInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash, value)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericSetAttrStringHash(self, attr, hash, value)
#define LOCAL_IS_SET
#elif defined(DEFINE_DeeType_SetAttrStringLenHash)
#define LOCAL_DeeType_AccessAttr                                    DeeType_SetAttrStringLenHash
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_SetCachedClassAttrStringLenHash(self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_SetClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_SetClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_SetClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_SetInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_SetInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_SetInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, value)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericSetAttrStringLenHash(self, attr, attrlen, hash, value)
#define LOCAL_IS_SET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_FindAttrInfoStringHash)
#define LOCAL_DeeType_AccessAttr                                    DeeType_FindAttrInfoStringHash
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_FindCachedClassAttrInfoStringHash(self, attr, hash, retinfo)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_FindClassMethodAttrInfoStringHash(tp_invoker, tp_self, attr, hash, retinfo)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_FindClassGetSetAttrInfoStringHash(tp_invoker, tp_self, attr, hash, retinfo)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_FindClassMemberAttrInfoStringHash(tp_invoker, tp_self, attr, hash, retinfo)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_FindInstanceMethodAttrInfoStringHash(tp_invoker, tp_self, attr, hash, retinfo)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_FindInstanceGetSetAttrInfoStringHash(tp_invoker, tp_self, attr, hash, retinfo)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_FindInstanceMemberAttrInfoStringHash(tp_invoker, tp_self, attr, hash, retinfo)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericFindAttrInfoStringHash(self, attr, hash, retinfo)
#define LOCAL_IS_FINDINFO
#elif defined(DEFINE_DeeType_FindAttrInfoStringLenHash)
#define LOCAL_DeeType_AccessAttr                                    DeeType_FindAttrInfoStringLenHash
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_FindCachedClassAttrInfoStringLenHash(self, attr, attrlen, hash, retinfo)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_FindClassMethodAttrInfoStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, retinfo)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_FindClassGetSetAttrInfoStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, retinfo)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_FindClassMemberAttrInfoStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, retinfo)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_FindInstanceMethodAttrInfoStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, retinfo)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_FindInstanceGetSetAttrInfoStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, retinfo)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_FindInstanceMemberAttrInfoStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, retinfo)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericFindAttrInfoStringLenHash(self, attr, attrlen, hash, retinfo)
#define LOCAL_HAS_len
#define LOCAL_IS_FINDINFO
#elif defined(DEFINE_DeeType_FindAttr)
#define LOCAL_DeeType_AccessAttr                                    DeeType_FindAttr
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_FindCachedClassAttr(self, retinfo, rules)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_FindClassMethodAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_FindClassGetSetAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_FindClassMemberAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_FindInstanceMethodAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_FindInstanceGetSetAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_FindInstanceMemberAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericFindAttr(self, retinfo, rules)
#define LOCAL_IS_FIND
#elif defined(DEFINE_DeeType_EnumAttr)
#define LOCAL_DeeType_AccessAttr                                    DeeType_EnumAttr
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    type_method_enum(tp_self, (tp_self)->tp_class_methods, ATTR_CMEMBER, proc, arg)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    type_getset_enum(tp_self, (tp_self)->tp_class_getsets, ATTR_CMEMBER | ATTR_PROPERTY, proc, arg)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    type_member_enum(tp_self, (tp_self)->tp_class_members, ATTR_CMEMBER, proc, arg)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) type_obmeth_enum(tp_self, proc, arg)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) type_obprop_enum(tp_self, proc, arg)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) type_obmemb_enum(tp_self, proc, arg)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericEnumAttr(self, proc, arg)
#define LOCAL_IS_ENUM
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
#elif defined(LOCAL_IS_FINDINFO)
#define LOCAL_return_t              bool
#define LOCAL_ATTR_NOT_FOUND_RESULT false
#define LOCAL_ERROR_RETURN_VALUE    DONT_USE_THIS_MACRO
#elif defined(LOCAL_IS_DEL) || defined(LOCAL_IS_SET) || defined(LOCAL_IS_FIND)
#define LOCAL_return_t              int
#define LOCAL_ATTR_NOT_FOUND_RESULT 1
#define LOCAL_ERROR_RETURN_VALUE    (-1)
#elif defined(LOCAL_IS_ENUM)
#define LOCAL_return_t              Dee_ssize_t
#define LOCAL_ATTR_NOT_FOUND_RESULT DONT_USE_THIS_MACRO
#define LOCAL_ERROR_RETURN_VALUE    DONT_USE_THIS_MACRO
#else /* ... */
#define LOCAL_return_t              int
#define LOCAL_ATTR_NOT_FOUND_RESULT Dee_BOUND_MISSING
#define LOCAL_ERROR_RETURN_VALUE    Dee_BOUND_ERR
#endif /* !... */

#if defined(DEFINE_DeeType_FindAttr)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3))
#elif defined(LOCAL_HAS_len) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_FINDINFO) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 5))
#elif defined(LOCAL_IS_SET) || defined(LOCAL_IS_FINDINFO) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 4))
#else /* ... */
#define LOCAL_ATTR_NONNULL NONNULL((1, 2))
#endif /* !... */

/* Helpers for accessing attributes and performing operations. */
#ifdef LOCAL_HAS_len
#define LOCAL_DeeType_QueryClassAttribute(tp_invoker, tp_self)    DeeType_QueryClassAttributeStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_QueryInstanceAttribute(tp_invoker, tp_self) DeeType_QueryInstanceAttributeStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#else /* LOCAL_HAS_len */
#define LOCAL_DeeType_QueryClassAttribute(tp_invoker, tp_self)    DeeType_QueryClassAttributeStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_QueryInstanceAttribute(tp_invoker, tp_self) DeeType_QueryInstanceAttributeStringHash(tp_invoker, tp_self, attr, hash)
#endif /* !LOCAL_HAS_len */

#ifdef LOCAL_IS_CALL_LIKE
#ifdef LOCAL_HAS_len
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self) DeeType_GetClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self) DeeType_GetClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#else /* LOCAL_HAS_len */
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self) DeeType_GetClassGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self) DeeType_GetClassMemberAttrStringHash(tp_invoker, tp_self, attr, hash)
#endif /* !LOCAL_HAS_len */
#endif /* LOCAL_IS_CALL_LIKE */

#ifdef LOCAL_IS_GET
#define LOCAL_DeeClass_AccessClassAttribute(class_type, cattr)    DeeClass_GetClassAttribute(class_type, cattr)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_GetInstanceAttribute(class_type, cattr)
#elif defined(LOCAL_IS_BOUND)
#define LOCAL_DeeClass_AccessClassAttribute(class_type, cattr)    DeeClass_BoundClassAttribute(class_type, cattr)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_BoundInstanceAttribute(class_type, cattr)
#elif defined(LOCAL_IS_DEL)
#define LOCAL_DeeClass_AccessClassAttribute(class_type, cattr)    DeeClass_DelClassAttribute(class_type, cattr)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_DelInstanceAttribute(class_type, cattr)
#elif defined(LOCAL_IS_SET)
#define LOCAL_DeeClass_AccessClassAttribute(class_type, cattr)    DeeClass_SetClassAttribute(class_type, cattr, value)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_SetInstanceAttribute(class_type, cattr, value)
#elif defined(LOCAL_IS_CALL)
#define LOCAL_DeeClass_AccessClassAttribute(class_type, cattr)    DeeClass_CallClassAttribute(class_type, cattr, argc, argv)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_CallInstanceAttribute(class_type, cattr, argc, argv)
#elif defined(LOCAL_IS_CALL_KW)
#define LOCAL_DeeClass_AccessClassAttribute(class_type, cattr)    DeeClass_CallClassAttributeKw(class_type, cattr, argc, argv, kw)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_CallInstanceAttributeKw(class_type, cattr, argc, argv, kw)
#elif defined(LOCAL_IS_CALL_TUPLE)
#define LOCAL_DeeClass_AccessClassAttribute(class_type, cattr)    DeeClass_CallClassAttributeTuple(class_type, cattr, args)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_CallInstanceAttributeTuple(class_type, cattr, args)
#elif defined(LOCAL_IS_CALL_TUPLE_KW)
#define LOCAL_DeeClass_AccessClassAttribute(class_type, cattr)    DeeClass_CallClassAttributeTupleKw(class_type, cattr, args, kw)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_CallInstanceAttributeTupleKw(class_type, cattr, args, kw)
#elif defined(LOCAL_IS_VCALLF)
#define LOCAL_DeeClass_AccessClassAttribute(class_type, cattr)    DeeClass_VCallClassAttributef(class_type, cattr, format, args)
#define LOCAL_DeeClass_AccessInstanceAttribute(class_type, cattr) DeeClass_VCallInstanceAttributef(class_type, cattr, format, args)
#endif /* ... */


INTERN WUNUSED LOCAL_ATTR_NONNULL LOCAL_return_t
(DCALL LOCAL_DeeType_AccessAttr)(DeeTypeObject *self,
#ifdef LOCAL_IS_FIND
                                 struct Dee_attribute_info *__restrict retinfo,
                                 struct Dee_attribute_lookup_rules const *__restrict rules
#elif defined(LOCAL_IS_ENUM)
                                 denum_t proc, void *arg
#else /* ... */
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
#endif /* !... */
                                 ) {
#ifdef LOCAL_IS_CALL_LIKE
#define LOCAL_invoke_result_OR_done invoke_result
#else /* LOCAL_IS_CALL_LIKE */
#define LOCAL_invoke_result_OR_done done
#endif /* !LOCAL_IS_CALL_LIKE */
#ifdef LOCAL_IS_ENUM
	LOCAL_return_t final_result = 0;
#endif /* LOCAL_IS_ENUM */
	LOCAL_return_t result;
	DeeTypeObject *iter;
	DeeTypeMRO mro;

	/* Try to access the cached version of the attribute. */
#ifdef LOCAL_DeeType_AccessCachedClassAttr
	result = LOCAL_DeeType_AccessCachedClassAttr(self);
	if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
		goto done;
#endif /* LOCAL_DeeType_AccessCachedClassAttr */

	iter = self;
	DeeTypeMRO_Init(&mro, iter);
	do {
#ifdef LOCAL_IS_ENUM
#define LOCAL_process_result(result, done) \
	if unlikely(result < 0)                \
		goto err;                          \
	final_result += result
#else /* LOCAL_IS_ENUM */
#define LOCAL_process_result(result, done)     \
	if (result != LOCAL_ATTR_NOT_FOUND_RESULT) \
		goto done
#endif /* !LOCAL_IS_ENUM */

#ifdef LOCAL_IS_FIND
continue_at_iter:
		if (rules->alr_decl && rules->alr_decl != (DeeObject *)iter) {
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
			result = LOCAL_DeeObject_GenericAccessAttr((DeeObject *)iter);
			LOCAL_process_result(result, done);
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */
			iter = DeeTypeMRO_Next(&mro, iter);
			if (!iter)
				break;

			/* Also set `self', so we don't corrupt the cache by
			 * potentially failing to cache attributes that should
			 * have been visible. */
			self = iter;
			goto continue_at_iter;
		}
#endif /* LOCAL_IS_FIND */

		if (DeeType_IsClass(iter)) {
#ifdef LOCAL_IS_FIND
			result = DeeClass_FindClassAttribute(self, iter, retinfo, rules);
			LOCAL_process_result(result, done);
			result = DeeClass_FindClassInstanceAttribute(self, iter, retinfo, rules);
			LOCAL_process_result(result, done);
#elif defined(LOCAL_IS_ENUM)
			result = DeeClass_EnumClassAttributes(iter, proc, arg);
			LOCAL_process_result(result, done);
			result = DeeClass_EnumClassInstanceAttributes(iter, proc, arg);
			LOCAL_process_result(result, done);
#else /* ... */
			struct class_attribute *cattr;
			cattr = LOCAL_DeeType_QueryClassAttribute(self, iter);
			if (cattr != NULL) {
#ifdef LOCAL_IS_HAS
				return true;
#elif defined(LOCAL_IS_FINDINFO)
				retinfo->ai_type = Dee_ATTRINFO_ATTR;
				retinfo->ai_decl = (DeeObject *)iter;
				retinfo->ai_value.v_attr = cattr;
				return true;
#else /* ... */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return LOCAL_DeeClass_AccessClassAttribute(iter, cattr);
#endif /* !... */
			}
			cattr = LOCAL_DeeType_QueryInstanceAttribute(self, iter);
			if (cattr != NULL) {
#ifdef LOCAL_IS_HAS
				return true;
#elif defined(LOCAL_IS_FINDINFO)
				retinfo->ai_type = Dee_ATTRINFO_INSTANCE_ATTR;
				retinfo->ai_decl = (DeeObject *)iter;
				retinfo->ai_value.v_instance_attr = cattr;
				return true;
#else /* ... */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return LOCAL_DeeClass_AccessInstanceAttribute(iter, cattr);
#endif /* !... */
			}
#endif /* !... */
		} else {
			if (iter->tp_class_methods) {
				result = LOCAL_DeeType_AccessClassMethodAttr(self, iter);
				LOCAL_process_result(result, done);
			}
			if (iter->tp_class_getsets) {
				result = LOCAL_DeeType_AccessClassGetSetAttr(self, iter);
				LOCAL_process_result(result, LOCAL_invoke_result_OR_done);
			}
			if (iter->tp_class_members) {
				result = LOCAL_DeeType_AccessClassMemberAttr(self, iter);
				LOCAL_process_result(result, LOCAL_invoke_result_OR_done);
			}

#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods) { /* Access instance methods using `DeeClsMethodObject' */
					result = LOCAL_DeeType_AccessInstanceMethodAttr(self, iter);
					LOCAL_process_result(result, done);
				}
				if (iter->tp_getsets) { /* Access instance getsets using `DeeClsPropertyObject' */
					result = LOCAL_DeeType_AccessInstanceGetSetAttr(self, iter);
					LOCAL_process_result(result, done);
				}
				if (iter->tp_members) { /* Access instance members using `DeeClsMemberObject' */
					result = LOCAL_DeeType_AccessInstanceMemberAttr(self, iter);
					LOCAL_process_result(result, done);
				}
			}
		}

#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		result = LOCAL_DeeObject_GenericAccessAttr((DeeObject *)iter);
		LOCAL_process_result(result, done);
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */

#undef LOCAL_process_result
	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL);

#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	result = LOCAL_DeeObject_GenericAccessAttr((DeeObject *)self);
	if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
		goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */

#if !defined(LOCAL_IS_HAS) && !defined(LOCAL_IS_FINDINFO) && !defined(LOCAL_IS_FIND) && !defined(LOCAL_IS_ENUM)
#ifdef LOCAL_HAS_len
	err_unknown_attribute_string_len(self, attr, attrlen, LOCAL_ATTR_ACCESS_OP);
#else /* LOCAL_HAS_len */
	err_unknown_attribute_string(self, attr, LOCAL_ATTR_ACCESS_OP);
#endif /* !LOCAL_HAS_len */
err:
	return LOCAL_ERROR_RETURN_VALUE;
#endif /* !LOCAL_IS_HAS && !LOCAL_IS_FIND && !LOCAL_IS_ENUM */
#ifdef LOCAL_IS_CALL_LIKE
invoke_result:
	if (result) {
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
#endif /* LOCAL_IS_CALL_LIKE */
#ifndef LOCAL_IS_ENUM
done:
	return result;
#else /* !LOCAL_IS_ENUM */
	return final_result;
err:
	return result;
#endif /* LOCAL_IS_ENUM */
#undef LOCAL_invoke_result_OR_done
}

#undef LOCAL_return_t
#undef LOCAL_ATTR_NOT_FOUND_RESULT
#undef LOCAL_ERROR_RETURN_VALUE
#undef LOCAL_DeeType_AccessAttr
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
#undef LOCAL_IS_FINDINFO
#undef LOCAL_IS_ENUM
#undef LOCAL_HAS_len
#undef LOCAL_ATTR_NONNULL
#undef LOCAL_ATTR_ACCESS_OP

#undef LOCAL_DeeType_QueryClassAttribute
#undef LOCAL_DeeType_QueryInstanceAttribute
#undef LOCAL_DeeType_AccessCachedClassAttr
#undef LOCAL_DeeClass_AccessClassAttribute
#undef LOCAL_DeeClass_AccessInstanceAttribute
#undef LOCAL_DeeType_AccessClassMethodAttr
#undef LOCAL_DeeType_AccessClassGetSetAttr
#undef LOCAL_DeeType_AccessClassMemberAttr
#undef LOCAL_DeeType_AccessInstanceMethodAttr
#undef LOCAL_DeeType_AccessInstanceGetSetAttr
#undef LOCAL_DeeType_AccessInstanceMemberAttr
#undef LOCAL_DeeObject_GenericAccessAttr

DECL_END

#undef DEFINE_DeeType_GetAttrStringHash
#undef DEFINE_DeeType_GetAttrStringLenHash
#undef DEFINE_DeeType_BoundAttrStringHash
#undef DEFINE_DeeType_BoundAttrStringLenHash
#undef DEFINE_DeeType_CallAttrStringHash
#undef DEFINE_DeeType_CallAttrStringLenHash
#undef DEFINE_DeeType_CallAttrStringHashKw
#undef DEFINE_DeeType_CallAttrStringLenHashKw
#undef DEFINE_DeeType_CallAttrStringHashTuple
#undef DEFINE_DeeType_CallAttrStringLenHashTuple
#undef DEFINE_DeeType_CallAttrStringHashTupleKw
#undef DEFINE_DeeType_CallAttrStringLenHashTupleKw
#undef DEFINE_DeeType_VCallAttrStringHashf
#undef DEFINE_DeeType_VCallAttrStringLenHashf
#undef DEFINE_DeeType_HasAttrStringHash
#undef DEFINE_DeeType_HasAttrStringLenHash
#undef DEFINE_DeeType_DelAttrStringHash
#undef DEFINE_DeeType_DelAttrStringLenHash
#undef DEFINE_DeeType_SetAttrStringHash
#undef DEFINE_DeeType_SetAttrStringLenHash
#undef DEFINE_DeeType_FindAttrInfoStringHash
#undef DEFINE_DeeType_FindAttrInfoStringLenHash
#undef DEFINE_DeeType_FindAttr
#undef DEFINE_DeeType_EnumAttr
