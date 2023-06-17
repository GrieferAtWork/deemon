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
//#define DEFINE_DeeType_GetAttrString
//#define DEFINE_DeeType_GetAttrStringLen
//#define DEFINE_DeeType_BoundAttrString
//#define DEFINE_DeeType_BoundAttrStringLen
//#define DEFINE_DeeType_CallAttrString
//#define DEFINE_DeeType_CallAttrStringLen
//#define DEFINE_DeeType_CallAttrStringKw
//#define DEFINE_DeeType_CallAttrStringLenKw
//#define DEFINE_DeeType_CallAttrStringTuple
//#define DEFINE_DeeType_CallAttrStringLenTuple
//#define DEFINE_DeeType_CallAttrStringTupleKw
//#define DEFINE_DeeType_CallAttrStringLenTupleKw
//#define DEFINE_DeeType_VCallAttrStringf
//#define DEFINE_DeeType_VCallAttrStringLenf
//#define DEFINE_DeeType_HasAttrString
//#define DEFINE_DeeType_HasAttrStringLen
//#define DEFINE_DeeType_DelAttrString
//#define DEFINE_DeeType_DelAttrStringLen
//#define DEFINE_DeeType_SetAttrString
#define DEFINE_DeeType_SetAttrStringLen
//#define DEFINE_DeeType_FindAttr
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeType_GetAttrString) +            \
     defined(DEFINE_DeeType_GetAttrStringLen) +         \
     defined(DEFINE_DeeType_BoundAttrString) +          \
     defined(DEFINE_DeeType_BoundAttrStringLen) +       \
     defined(DEFINE_DeeType_CallAttrString) +           \
     defined(DEFINE_DeeType_CallAttrStringLen) +        \
     defined(DEFINE_DeeType_CallAttrStringKw) +         \
     defined(DEFINE_DeeType_CallAttrStringLenKw) +      \
     defined(DEFINE_DeeType_CallAttrStringTuple) +      \
     defined(DEFINE_DeeType_CallAttrStringLenTuple) +   \
     defined(DEFINE_DeeType_CallAttrStringTupleKw) +    \
     defined(DEFINE_DeeType_CallAttrStringLenTupleKw) + \
     defined(DEFINE_DeeType_VCallAttrStringf) +         \
     defined(DEFINE_DeeType_VCallAttrStringLenf) +      \
     defined(DEFINE_DeeType_HasAttrString) +            \
     defined(DEFINE_DeeType_HasAttrStringLen) +         \
     defined(DEFINE_DeeType_DelAttrString) +            \
     defined(DEFINE_DeeType_DelAttrStringLen) +         \
     defined(DEFINE_DeeType_SetAttrString) +            \
     defined(DEFINE_DeeType_SetAttrStringLen) +         \
     defined(DEFINE_DeeType_FindAttr)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

#ifdef DEFINE_DeeType_GetAttrString
#define LOCAL_DeeType_AccessAttr                                    DeeType_GetAttrString
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_GetCachedClassAttr(self, attr, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_GetClassMethodAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_GetClassGetSetAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_GetClassMemberAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_GetInstanceMethodAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_GetInstanceGetSetAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_GetInstanceMemberAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericGetAttrString(self, attr, hash)
#define LOCAL_IS_GET
#elif defined(DEFINE_DeeType_GetAttrStringLen)
#define LOCAL_DeeType_AccessAttr                                    DeeType_GetAttrStringLen
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_GetCachedClassAttrLen(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_GetClassMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_GetClassGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_GetClassMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_GetInstanceMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_GetInstanceGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_GetInstanceMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericGetAttrStringLen(self, attr, attrlen, hash)
#define LOCAL_IS_GET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_BoundAttrString)
#define LOCAL_DeeType_AccessAttr                                    DeeType_BoundAttrString
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_BoundCachedClassAttr(self, attr, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    (DeeType_HasClassMethodAttr(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_BoundClassGetSetAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_BoundClassMemberAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasInstanceMethodAttr(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasInstanceGetSetAttr(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasInstanceMemberAttr(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericBoundAttrString(self, attr, hash)
#define LOCAL_IS_BOUND
#elif defined(DEFINE_DeeType_BoundAttrStringLen)
#define LOCAL_DeeType_AccessAttr                                    DeeType_BoundAttrStringLen
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_BoundCachedClassAttrLen(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    (DeeType_HasClassMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_BoundClassGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_BoundClassMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasInstanceMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasInstanceGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasInstanceMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericBoundAttrStringLen(self, attr, attrlen, hash)
#define LOCAL_IS_BOUND
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallAttrString)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrString
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttr(self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttr(tp_invoker, tp_self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttr(tp_invoker, tp_self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttr(tp_invoker, tp_self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttr(tp_invoker, tp_self, attr, hash, argc, argv)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrString(self, attr, hash, argc, argv)
#define LOCAL_IS_CALL
#elif defined(DEFINE_DeeType_CallAttrStringLen)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringLen
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrLen(self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringLen(self, attr, attrlen, hash, argc, argv)
#define LOCAL_IS_CALL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallAttrStringKw)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringKw
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrKw(self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringKw(self, attr, hash, argc, argv, kw)
#define LOCAL_IS_CALL_KW
#elif defined(DEFINE_DeeType_CallAttrStringLenKw)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringLenKw
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrLenKw(self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrLenKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrLenKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrLenKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrLenKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringLenKw(self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_IS_CALL_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallAttrStringTuple)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringTuple
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrTuple(self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrTuple(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrTuple(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrTuple(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrTuple(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringTuple(self, attr, hash, args, kw)
#define LOCAL_IS_CALL_TUPLE
#elif defined(DEFINE_DeeType_CallAttrStringLenTuple)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringLenTuple
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrLenTuple(self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrLenTuple(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrLenTuple(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrLenTuple(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrLenTuple(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringLenTuple(self, attr, attrlen, hash, args, kw)
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallAttrStringTupleKw)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringTupleKw
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrTupleKw(self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrTupleKw(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrTupleKw(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrTupleKw(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrTupleKw(tp_invoker, tp_self, attr, hash, args, kw)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringTupleKw(self, attr, hash, args, kw)
#define LOCAL_IS_CALL_TUPLE_KW
#elif defined(DEFINE_DeeType_CallAttrStringLenTupleKw)
#define LOCAL_DeeType_AccessAttr                                    DeeType_CallAttrStringLenTupleKw
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_CallCachedClassAttrLenTupleKw(self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_CallClassMethodAttrLenTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_CallInstanceMethodAttrLenTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_CallInstanceGetSetAttrLenTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_CallInstanceMemberAttrLenTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericCallAttrStringLenTupleKw(self, attr, attrlen, hash, args, kw)
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_VCallAttrStringf)
#define LOCAL_DeeType_AccessAttr                                    DeeType_VCallAttrStringf
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_VCallCachedClassAttrf(self, attr, hash, format, args)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_VCallClassMethodAttrf(tp_invoker, tp_self, attr, hash, format, args)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_VCallInstanceMethodAttrf(tp_invoker, tp_self, attr, hash, format, args)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_VCallInstanceGetSetAttrf(tp_invoker, tp_self, attr, hash, format, args)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_VCallInstanceMemberAttrf(tp_invoker, tp_self, attr, hash, format, args)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericVCallAttrStringf(self, attr, hash, format, args)
#define LOCAL_IS_VCALLF
#elif defined(DEFINE_DeeType_VCallAttrStringLenf)
#define LOCAL_DeeType_AccessAttr                                    DeeType_VCallAttrStringLenf
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_VCallCachedClassAttrLenf(self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_VCallClassMethodAttrLenf(tp_invoker, tp_self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_VCallInstanceMethodAttrLenf(tp_invoker, tp_self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_VCallInstanceGetSetAttrLenf(tp_invoker, tp_self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_VCallInstanceMemberAttrLenf(tp_invoker, tp_self, attr, attrlen, hash, format, args)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericVCallAttrStringLenf(self, attr, attrlen, hash, format, args)
#define LOCAL_IS_VCALLF
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_HasAttrString)
#define LOCAL_DeeType_AccessAttr                                    DeeType_HasAttrString
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_HasCachedClassAttr(self, attr, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_HasClassMethodAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_HasClassGetSetAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_HasClassMemberAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_HasInstanceMethodAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_HasInstanceGetSetAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_HasInstanceMemberAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericHasAttrString(self, attr, hash)
#define LOCAL_IS_HAS
#elif defined(DEFINE_DeeType_HasAttrStringLen)
#define LOCAL_DeeType_AccessAttr                                    DeeType_HasAttrStringLen
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_HasCachedClassAttrLen(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    DeeType_HasClassMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_HasClassGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_HasClassMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) DeeType_HasInstanceMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) DeeType_HasInstanceGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) DeeType_HasInstanceMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericHasAttrStringLen(self, attr, attrlen, hash)
#define LOCAL_IS_HAS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_DelAttrString)
#define LOCAL_DeeType_AccessAttr                                    DeeType_DelAttrString
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_DelCachedClassAttr(self, attr, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    (DeeType_HasClassMethodAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_DelClassGetSetAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_DelClassMemberAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasInstanceMethodAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasInstanceGetSetAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasInstanceMemberAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericDelAttrString(self, attr, hash)
#define LOCAL_IS_DEL
#elif defined(DEFINE_DeeType_DelAttrStringLen)
#define LOCAL_DeeType_AccessAttr                                    DeeType_DelAttrStringLen
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_DelCachedClassAttrLen(self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    (DeeType_HasClassMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_DelClassGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_DelClassMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasInstanceMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasInstanceGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasInstanceMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericDelAttrStringLen(self, attr, attrlen, hash)
#define LOCAL_IS_DEL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetAttrString)
#define LOCAL_DeeType_AccessAttr                                    DeeType_SetAttrString
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_SetCachedClassAttr(self, attr, hash, value)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    (DeeType_HasClassMethodAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_SetClassGetSetAttr(tp_invoker, tp_self, attr, hash, value)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_SetClassMemberAttr(tp_invoker, tp_self, attr, hash, value)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasInstanceMethodAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasInstanceGetSetAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasInstanceMemberAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericSetAttrString(self, attr, hash, value)
#define LOCAL_IS_SET
#elif defined(DEFINE_DeeType_SetAttrStringLen)
#define LOCAL_DeeType_AccessAttr                                    DeeType_SetAttrStringLen
#define LOCAL_DeeType_AccessCachedClassAttr(self)                   DeeType_SetCachedClassAttrLen(self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessClassMethodAttr(tp_invoker, tp_self)    (DeeType_HasClassMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self)    DeeType_SetClassGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self)    DeeType_SetClassMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessInstanceMethodAttr(tp_invoker, tp_self) (DeeType_HasInstanceMethodAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeType_AccessInstanceGetSetAttr(tp_invoker, tp_self) (DeeType_HasInstanceGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeType_AccessInstanceMemberAttr(tp_invoker, tp_self) (DeeType_HasInstanceMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
#define LOCAL_DeeObject_GenericAccessAttr(self)                     DeeObject_GenericSetAttrStringLen(self, attr, attrlen, hash, value)
#define LOCAL_IS_SET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_FindAttr)
#define LOCAL_DeeType_AccessAttr                                    DeeType_FindAttr
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

#if defined(DEFINE_DeeType_FindAttr)
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
#define LOCAL_DeeType_QueryClassAttribute(tp_invoker, tp_self)    DeeType_QueryClassAttributeStringLenWithHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_QueryInstanceAttribute(tp_invoker, tp_self) DeeType_QueryInstanceAttributeStringLenWithHash(tp_invoker, tp_self, attr, attrlen, hash)
#else /* LOCAL_HAS_len */
#define LOCAL_DeeType_QueryClassAttribute(tp_invoker, tp_self)    DeeType_QueryClassAttributeStringWithHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_QueryInstanceAttribute(tp_invoker, tp_self) DeeType_QueryInstanceAttributeStringWithHash(tp_invoker, tp_self, attr, hash)
#endif /* !LOCAL_HAS_len */

#ifdef LOCAL_IS_CALL_LIKE
#ifdef LOCAL_HAS_len
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self) DeeType_GetClassGetSetAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self) DeeType_GetClassMemberAttrLen(tp_invoker, tp_self, attr, attrlen, hash)
#else /* LOCAL_HAS_len */
#define LOCAL_DeeType_AccessClassGetSetAttr(tp_invoker, tp_self) DeeType_GetClassGetSetAttr(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessClassMemberAttr(tp_invoker, tp_self) DeeType_GetClassMemberAttr(tp_invoker, tp_self, attr, hash)
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


INTERN WUNUSED LOCAL_ATTR_NONNULL LOCAL_return_t DCALL
LOCAL_DeeType_AccessAttr(DeeTypeObject *self,
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
	result = LOCAL_DeeType_AccessCachedClassAttr(self);
	if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
		goto done;
	iter = self;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *cattr;
			cattr = LOCAL_DeeType_QueryClassAttribute(self, iter);
			if (cattr != NULL) {
#ifdef LOCAL_IS_HAS
				return true;
#else /* LOCAL_IS_HAS */
				if (!class_attribute_mayaccess(cattr, iter)) {
					err_class_protected_member(iter, cattr);
					goto err;
				}
				return LOCAL_DeeClass_AccessClassAttribute(iter, cattr);
#endif /* !LOCAL_IS_HAS */
			}
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
		} else {
			if (iter->tp_class_methods) {
				result = LOCAL_DeeType_AccessClassMethodAttr(self, iter);
				if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
					goto done;
			}
			if (iter->tp_class_getsets) {
				result = LOCAL_DeeType_AccessClassGetSetAttr(self, iter);
				if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
					goto LOCAL_invoke_result_OR_done;
			}
			if (iter->tp_class_members) {
				result = LOCAL_DeeType_AccessClassMemberAttr(self, iter);
				if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
					goto LOCAL_invoke_result_OR_done;
			}

#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
			if (iter != &DeeType_Type)
#endif /* CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE */
			{
				if (iter->tp_methods) { /* Access instance methods using `DeeClsMethodObject' */
					result = LOCAL_DeeType_AccessInstanceMethodAttr(self, iter);
					if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
						goto done;
				}
				if (iter->tp_getsets) { /* Access instance getsets using `DeeClsPropertyObject' */
					result = LOCAL_DeeType_AccessInstanceGetSetAttr(self, iter);
					if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
						goto done;
				}
				if (iter->tp_members) { /* Access instance members using `DeeClsMemberObject' */
					result = LOCAL_DeeType_AccessInstanceMemberAttr(self, iter);
					if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
						goto done;
				}
			}
		}

#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
		result = LOCAL_DeeObject_GenericAccessAttr((DeeObject *)iter);
		if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
			goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC */

	} while ((iter = DeeType_Base(iter)) != NULL);

#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
	result = LOCAL_DeeObject_GenericAccessAttr((DeeObject *)self);
	if (result != LOCAL_ATTR_NOT_FOUND_RESULT)
		goto done;
#endif /* CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC */

#ifndef LOCAL_IS_HAS
#ifdef LOCAL_HAS_len
	err_unknown_attribute_len(self, attr, attrlen, LOCAL_ATTR_ACCESS_OP);
#else /* LOCAL_HAS_len */
	err_unknown_attribute(self, attr, LOCAL_ATTR_ACCESS_OP);
#endif /* !LOCAL_HAS_len */
err:
#endif /* !LOCAL_IS_HAS */
	return LOCAL_ERROR_RETURN_VALUE;
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
done:
	return result;
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

#undef DEFINE_DeeType_GetAttrString
#undef DEFINE_DeeType_GetAttrStringLen
#undef DEFINE_DeeType_BoundAttrString
#undef DEFINE_DeeType_BoundAttrStringLen
#undef DEFINE_DeeType_CallAttrString
#undef DEFINE_DeeType_CallAttrStringLen
#undef DEFINE_DeeType_CallAttrStringKw
#undef DEFINE_DeeType_CallAttrStringLenKw
#undef DEFINE_DeeType_CallAttrStringTuple
#undef DEFINE_DeeType_CallAttrStringLenTuple
#undef DEFINE_DeeType_CallAttrStringTupleKw
#undef DEFINE_DeeType_CallAttrStringLenTupleKw
#undef DEFINE_DeeType_HasAttrString
#undef DEFINE_DeeType_HasAttrStringLen
#undef DEFINE_DeeType_DelAttrString
#undef DEFINE_DeeType_DelAttrStringLen
#undef DEFINE_DeeType_SetAttrString
#undef DEFINE_DeeType_SetAttrStringLen
#undef DEFINE_DeeType_FindAttr
