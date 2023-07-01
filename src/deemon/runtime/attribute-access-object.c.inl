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
//#define DEFINE_DeeObject_GetAttr
//#define DEFINE_DeeObject_TGetAttr
//#define DEFINE_DeeObject_GetAttrStringHash
//#define DEFINE_DeeObject_TGetAttrStringHash
//#define DEFINE_DeeObject_GetAttrStringLenHash
//#define DEFINE_DeeObject_TGetAttrStringLenHash
//#define DEFINE_DeeObject_BoundAttr
//#define DEFINE_DeeObject_TBoundAttr
//#define DEFINE_DeeObject_BoundAttrStringHash
//#define DEFINE_DeeObject_TBoundAttrStringHash
//#define DEFINE_DeeObject_BoundAttrStringLenHash
//#define DEFINE_DeeObject_TBoundAttrStringLenHash
//#define DEFINE_DeeObject_CallAttr
//#define DEFINE_DeeObject_TCallAttr
//#define DEFINE_DeeObject_CallAttrStringHash
//#define DEFINE_DeeObject_TCallAttrStringHash
//#define DEFINE_DeeObject_CallAttrStringLenHash
//#define DEFINE_DeeObject_TCallAttrStringLenHash
//#define DEFINE_DeeObject_CallAttrKw
//#define DEFINE_DeeObject_TCallAttrKw
//#define DEFINE_DeeObject_CallAttrStringHashKw
//#define DEFINE_DeeObject_TCallAttrStringHashKw
//#define DEFINE_DeeObject_CallAttrStringLenHashKw
//#define DEFINE_DeeObject_TCallAttrStringLenHashKw
//#define DEFINE_DeeObject_CallAttrTuple
//#define DEFINE_DeeObject_TCallAttrTuple
//#define DEFINE_DeeObject_CallAttrStringHashTuple
//#define DEFINE_DeeObject_TCallAttrStringHashTuple
//#define DEFINE_DeeObject_CallAttrStringLenHashTuple
//#define DEFINE_DeeObject_TCallAttrStringLenHashTuple
//#define DEFINE_DeeObject_CallAttrTupleKw
//#define DEFINE_DeeObject_TCallAttrTupleKw
//#define DEFINE_DeeObject_CallAttrStringHashTupleKw
//#define DEFINE_DeeObject_TCallAttrStringHashTupleKw
//#define DEFINE_DeeObject_CallAttrStringLenHashTupleKw
//#define DEFINE_DeeObject_TCallAttrStringLenHashTupleKw
//#define DEFINE_DeeObject_VCallAttrf
//#define DEFINE_DeeObject_TVCallAttrf
//#define DEFINE_DeeObject_VCallAttrStringHashf
//#define DEFINE_DeeObject_TVCallAttrStringHashf
#define DEFINE_DeeObject_VCallAttrStringLenHashf
//#define DEFINE_DeeObject_TVCallAttrStringLenHashf
//#define DEFINE_DeeObject_HasAttr
//#define DEFINE_DeeObject_THasAttr
//#define DEFINE_DeeObject_HasAttrStringHash
//#define DEFINE_DeeObject_THasAttrStringHash
//#define DEFINE_DeeObject_HasAttrStringLenHash
//#define DEFINE_DeeObject_THasAttrStringLenHash
//#define DEFINE_DeeObject_DelAttr
//#define DEFINE_DeeObject_TDelAttr
//#define DEFINE_DeeObject_DelAttrStringHash
//#define DEFINE_DeeObject_TDelAttrStringHash
//#define DEFINE_DeeObject_DelAttrStringLenHash
//#define DEFINE_DeeObject_TDelAttrStringLenHash
//#define DEFINE_DeeObject_SetAttr
//#define DEFINE_DeeObject_TSetAttr
//#define DEFINE_DeeObject_SetAttrStringHash
//#define DEFINE_DeeObject_TSetAttrStringHash
//#define DEFINE_DeeObject_SetAttrStringLenHash
//#define DEFINE_DeeObject_TSetAttrStringLenHash
//#define DEFINE_DeeObject_FindAttr
//#define DEFINE_DeeObject_EnumAttr
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeObject_GetAttr) +                       \
     defined(DEFINE_DeeObject_TGetAttr) +                      \
     defined(DEFINE_DeeObject_GetAttrStringHash) +             \
     defined(DEFINE_DeeObject_TGetAttrStringHash) +            \
     defined(DEFINE_DeeObject_GetAttrStringLenHash) +          \
     defined(DEFINE_DeeObject_TGetAttrStringLenHash) +         \
     defined(DEFINE_DeeObject_BoundAttr) +                     \
     defined(DEFINE_DeeObject_TBoundAttr) +                    \
     defined(DEFINE_DeeObject_BoundAttrStringHash) +           \
     defined(DEFINE_DeeObject_TBoundAttrStringHash) +          \
     defined(DEFINE_DeeObject_BoundAttrStringLenHash) +        \
     defined(DEFINE_DeeObject_TBoundAttrStringLenHash) +       \
     defined(DEFINE_DeeObject_CallAttr) +                      \
     defined(DEFINE_DeeObject_TCallAttr) +                     \
     defined(DEFINE_DeeObject_CallAttrStringHash) +            \
     defined(DEFINE_DeeObject_TCallAttrStringHash) +           \
     defined(DEFINE_DeeObject_CallAttrStringLenHash) +         \
     defined(DEFINE_DeeObject_TCallAttrStringLenHash) +        \
     defined(DEFINE_DeeObject_CallAttrKw) +                    \
     defined(DEFINE_DeeObject_TCallAttrKw) +                   \
     defined(DEFINE_DeeObject_CallAttrStringHashKw) +          \
     defined(DEFINE_DeeObject_TCallAttrStringHashKw) +         \
     defined(DEFINE_DeeObject_CallAttrStringLenHashKw) +       \
     defined(DEFINE_DeeObject_TCallAttrStringLenHashKw) +      \
     defined(DEFINE_DeeObject_CallAttrTuple) +                 \
     defined(DEFINE_DeeObject_TCallAttrTuple) +                \
     defined(DEFINE_DeeObject_CallAttrStringHashTuple) +       \
     defined(DEFINE_DeeObject_TCallAttrStringHashTuple) +      \
     defined(DEFINE_DeeObject_CallAttrStringLenHashTuple) +    \
     defined(DEFINE_DeeObject_TCallAttrStringLenHashTuple) +   \
     defined(DEFINE_DeeObject_CallAttrTupleKw) +               \
     defined(DEFINE_DeeObject_TCallAttrTupleKw) +              \
     defined(DEFINE_DeeObject_CallAttrStringHashTupleKw) +     \
     defined(DEFINE_DeeObject_TCallAttrStringHashTupleKw) +    \
     defined(DEFINE_DeeObject_CallAttrStringLenHashTupleKw) +  \
     defined(DEFINE_DeeObject_TCallAttrStringLenHashTupleKw) + \
     defined(DEFINE_DeeObject_VCallAttrf) +                    \
     defined(DEFINE_DeeObject_TVCallAttrf) +                   \
     defined(DEFINE_DeeObject_VCallAttrStringHashf) +          \
     defined(DEFINE_DeeObject_TVCallAttrStringHashf) +         \
     defined(DEFINE_DeeObject_VCallAttrStringLenHashf) +       \
     defined(DEFINE_DeeObject_TVCallAttrStringLenHashf) +      \
     defined(DEFINE_DeeObject_HasAttr) +                       \
     defined(DEFINE_DeeObject_THasAttr) +                      \
     defined(DEFINE_DeeObject_HasAttrStringHash) +             \
     defined(DEFINE_DeeObject_THasAttrStringHash) +            \
     defined(DEFINE_DeeObject_HasAttrStringLenHash) +          \
     defined(DEFINE_DeeObject_THasAttrStringLenHash) +         \
     defined(DEFINE_DeeObject_DelAttr) +                       \
     defined(DEFINE_DeeObject_TDelAttr) +                      \
     defined(DEFINE_DeeObject_DelAttrStringHash) +             \
     defined(DEFINE_DeeObject_TDelAttrStringHash) +            \
     defined(DEFINE_DeeObject_DelAttrStringLenHash) +          \
     defined(DEFINE_DeeObject_TDelAttrStringLenHash) +         \
     defined(DEFINE_DeeObject_SetAttr) +                       \
     defined(DEFINE_DeeObject_TSetAttr) +                      \
     defined(DEFINE_DeeObject_SetAttrStringHash) +             \
     defined(DEFINE_DeeObject_TSetAttrStringHash) +            \
     defined(DEFINE_DeeObject_SetAttrStringLenHash) +          \
     defined(DEFINE_DeeObject_TSetAttrStringLenHash) +         \
     defined(DEFINE_DeeObject_FindAttr) +                      \
     defined(DEFINE_DeeObject_EnumAttr)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

#ifdef DEFINE_DeeObject_GetAttr
#define LOCAL_DeeObject_AccessAttr DeeObject_GetAttr
#define LOCAL_IS_GET
#elif defined(DEFINE_DeeObject_TGetAttr)
#define LOCAL_DeeObject_AccessAttr DeeObject_TGetAttr
#define LOCAL_IS_GET
#define LOCAL_HAS_tp_self
#elif defined(DEFINE_DeeObject_GetAttrStringHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_GetAttrStringHash
#define LOCAL_IS_GET
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_TGetAttrStringHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_TGetAttrStringHash
#define LOCAL_IS_GET
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_GetAttrStringLenHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_GetAttrStringLenHash
#define LOCAL_IS_GET
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TGetAttrStringLenHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_TGetAttrStringLenHash
#define LOCAL_IS_GET
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_BoundAttr)
#define LOCAL_DeeObject_AccessAttr DeeObject_BoundAttr
#define LOCAL_IS_BOUND
#elif defined(DEFINE_DeeObject_TBoundAttr)
#define LOCAL_DeeObject_AccessAttr DeeObject_TBoundAttr
#define LOCAL_IS_BOUND
#define LOCAL_HAS_tp_self
#elif defined(DEFINE_DeeObject_BoundAttrStringHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_BoundAttrStringHash
#define LOCAL_IS_BOUND
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_TBoundAttrStringHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_TBoundAttrStringHash
#define LOCAL_IS_BOUND
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_BoundAttrStringLenHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_BoundAttrStringLenHash
#define LOCAL_IS_BOUND
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TBoundAttrStringLenHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_TBoundAttrStringLenHash
#define LOCAL_IS_BOUND
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_CallAttr)
#define LOCAL_DeeObject_AccessAttr DeeObject_CallAttr
#define LOCAL_IS_CALL
#elif defined(DEFINE_DeeObject_TCallAttr)
#define LOCAL_DeeObject_AccessAttr DeeObject_TCallAttr
#define LOCAL_IS_CALL
#define LOCAL_HAS_tp_self
#elif defined(DEFINE_DeeObject_CallAttrStringHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_CallAttrStringHash
#define LOCAL_IS_CALL
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_TCallAttrStringHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_TCallAttrStringHash
#define LOCAL_IS_CALL
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_CallAttrStringLenHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_CallAttrStringLenHash
#define LOCAL_IS_CALL
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TCallAttrStringLenHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_TCallAttrStringLenHash
#define LOCAL_IS_CALL
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_CallAttrKw)
#define LOCAL_DeeObject_AccessAttr DeeObject_CallAttrKw
#define LOCAL_IS_CALL_KW
#elif defined(DEFINE_DeeObject_TCallAttrKw)
#define LOCAL_DeeObject_AccessAttr DeeObject_TCallAttrKw
#define LOCAL_IS_CALL_KW
#define LOCAL_HAS_tp_self
#elif defined(DEFINE_DeeObject_CallAttrStringHashKw)
#define LOCAL_DeeObject_AccessAttr DeeObject_CallAttrStringHashKw
#define LOCAL_IS_CALL_KW
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_TCallAttrStringHashKw)
#define LOCAL_DeeObject_AccessAttr DeeObject_TCallAttrStringHashKw
#define LOCAL_IS_CALL_KW
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_CallAttrStringLenHashKw)
#define LOCAL_DeeObject_AccessAttr DeeObject_CallAttrStringLenHashKw
#define LOCAL_IS_CALL_KW
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TCallAttrStringLenHashKw)
#define LOCAL_DeeObject_AccessAttr DeeObject_TCallAttrStringLenHashKw
#define LOCAL_IS_CALL_KW
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_CallAttrTuple)
#define LOCAL_DeeObject_AccessAttr DeeObject_CallAttrTuple
#define LOCAL_IS_CALL_TUPLE
#elif defined(DEFINE_DeeObject_TCallAttrTuple)
#define LOCAL_DeeObject_AccessAttr DeeObject_TCallAttrTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_HAS_tp_self
#elif defined(DEFINE_DeeObject_CallAttrStringHashTuple)
#define LOCAL_DeeObject_AccessAttr DeeObject_CallAttrStringHashTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_TCallAttrStringHashTuple)
#define LOCAL_DeeObject_AccessAttr DeeObject_TCallAttrStringHashTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_CallAttrStringLenHashTuple)
#define LOCAL_DeeObject_AccessAttr DeeObject_CallAttrStringLenHashTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TCallAttrStringLenHashTuple)
#define LOCAL_DeeObject_AccessAttr DeeObject_TCallAttrStringLenHashTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_CallAttrTupleKw)
#define LOCAL_DeeObject_AccessAttr DeeObject_CallAttrTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#elif defined(DEFINE_DeeObject_TCallAttrTupleKw)
#define LOCAL_DeeObject_AccessAttr DeeObject_TCallAttrTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_HAS_tp_self
#elif defined(DEFINE_DeeObject_CallAttrStringHashTupleKw)
#define LOCAL_DeeObject_AccessAttr DeeObject_CallAttrStringHashTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_TCallAttrStringHashTupleKw)
#define LOCAL_DeeObject_AccessAttr DeeObject_TCallAttrStringHashTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_CallAttrStringLenHashTupleKw)
#define LOCAL_DeeObject_AccessAttr DeeObject_CallAttrStringLenHashTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TCallAttrStringLenHashTupleKw)
#define LOCAL_DeeObject_AccessAttr DeeObject_TCallAttrStringLenHashTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_VCallAttrf)
#define LOCAL_DeeObject_AccessAttr DeeObject_VCallAttrf
#define LOCAL_IS_VCALLF
#elif defined(DEFINE_DeeObject_TVCallAttrf)
#define LOCAL_DeeObject_AccessAttr DeeObject_TVCallAttrf
#define LOCAL_IS_VCALLF
#define LOCAL_HAS_tp_self
#elif defined(DEFINE_DeeObject_VCallAttrStringHashf)
#define LOCAL_DeeObject_AccessAttr DeeObject_VCallAttrStringHashf
#define LOCAL_IS_VCALLF
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_TVCallAttrStringHashf)
#define LOCAL_DeeObject_AccessAttr DeeObject_TVCallAttrStringHashf
#define LOCAL_IS_VCALLF
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_VCallAttrStringLenHashf)
#define LOCAL_DeeObject_AccessAttr DeeObject_VCallAttrStringLenHashf
#define LOCAL_IS_VCALLF
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TVCallAttrStringLenHashf)
#define LOCAL_DeeObject_AccessAttr DeeObject_TVCallAttrStringLenHashf
#define LOCAL_IS_VCALLF
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_HasAttr)
#define LOCAL_DeeObject_AccessAttr DeeObject_HasAttr
#define LOCAL_IS_HAS
#elif defined(DEFINE_DeeObject_THasAttr)
#define LOCAL_DeeObject_AccessAttr DeeObject_THasAttr
#define LOCAL_IS_HAS
#define LOCAL_HAS_tp_self
#elif defined(DEFINE_DeeObject_HasAttrStringHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_HasAttrStringHash
#define LOCAL_IS_HAS
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_THasAttrStringHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_THasAttrStringHash
#define LOCAL_IS_HAS
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_HasAttrStringLenHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_HasAttrStringLenHash
#define LOCAL_IS_HAS
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_THasAttrStringLenHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_THasAttrStringLenHash
#define LOCAL_IS_HAS
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_DelAttr)
#define LOCAL_DeeObject_AccessAttr DeeObject_DelAttr
#define LOCAL_IS_DEL
#elif defined(DEFINE_DeeObject_TDelAttr)
#define LOCAL_DeeObject_AccessAttr DeeObject_TDelAttr
#define LOCAL_IS_DEL
#define LOCAL_HAS_tp_self
#elif defined(DEFINE_DeeObject_DelAttrStringHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_DelAttrStringHash
#define LOCAL_IS_DEL
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_TDelAttrStringHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_TDelAttrStringHash
#define LOCAL_IS_DEL
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_DelAttrStringLenHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_DelAttrStringLenHash
#define LOCAL_IS_DEL
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TDelAttrStringLenHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_TDelAttrStringLenHash
#define LOCAL_IS_DEL
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_SetAttr)
#define LOCAL_DeeObject_AccessAttr DeeObject_SetAttr
#define LOCAL_IS_SET
#elif defined(DEFINE_DeeObject_TSetAttr)
#define LOCAL_DeeObject_AccessAttr DeeObject_TSetAttr
#define LOCAL_IS_SET
#define LOCAL_HAS_tp_self
#elif defined(DEFINE_DeeObject_SetAttrStringHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_SetAttrStringHash
#define LOCAL_IS_SET
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_TSetAttrStringHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_TSetAttrStringHash
#define LOCAL_IS_SET
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#elif defined(DEFINE_DeeObject_SetAttrStringLenHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_SetAttrStringLenHash
#define LOCAL_IS_SET
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_TSetAttrStringLenHash)
#define LOCAL_DeeObject_AccessAttr DeeObject_TSetAttrStringLenHash
#define LOCAL_IS_SET
#define LOCAL_HAS_tp_self
#define LOCAL_HAS_string
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeObject_FindAttr)
#define LOCAL_DeeObject_AccessAttr DeeObject_FindAttr
#define LOCAL_IS_FIND
#define LOCAL_HAS_tp_self
#elif defined(DEFINE_DeeObject_EnumAttr)
#define LOCAL_DeeObject_AccessAttr DeeObject_EnumAttr
#define LOCAL_IS_ENUM
#define LOCAL_HAS_tp_self
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#ifdef LOCAL_HAS_string
#define LOCAL_HAS_hash
#endif /* !LOCAL_HAS_string */

#if (defined(LOCAL_IS_CALL) || defined(LOCAL_IS_CALL_KW) ||             \
     defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW) || \
     defined(LOCAL_IS_VCALLF))
#define LOCAL_IS_CALL_LIKE
#endif /* ... */


DECL_BEGIN

#if defined(LOCAL_IS_GET) || defined(LOCAL_IS_CALL_LIKE)
#define LOCAL_return_t              DREF DeeObject *
#define LOCAL_ATTR_NOT_FOUND_RESULT ITER_DONE
#define LOCAL_ERROR_RESULT          NULL
#elif defined(LOCAL_IS_HAS)
#define LOCAL_return_t              int
#define LOCAL_ATTR_NOT_FOUND_RESULT 0
#define LOCAL_ERROR_RESULT          (-1)
#elif defined(LOCAL_IS_DEL) || defined(LOCAL_IS_SET) || defined(LOCAL_IS_FIND)
#define LOCAL_return_t              int
#define LOCAL_ATTR_NOT_FOUND_RESULT 1
#define LOCAL_ERROR_RESULT          (-1)
#elif defined(LOCAL_IS_ENUM)
#define LOCAL_return_t              dssize_t
#define LOCAL_ATTR_NOT_FOUND_RESULT DONT_USE_THIS_MACRO
#define LOCAL_ERROR_RESULT          DONT_USE_THIS_MACRO
#else /* ... */
#define LOCAL_return_t              int
#define LOCAL_ATTR_NOT_FOUND_RESULT (-2)
#define LOCAL_ERROR_RESULT          (-1)
#endif /* !... */

/* Helpers to query class instance attributes */
#ifndef LOCAL_HAS_string
#define LOCAL_DeeType_QueryAttribute(tp_invoker, tp_self) DeeType_QueryAttributeHash(tp_invoker, tp_self, attr, hash)
#elif defined(LOCAL_HAS_len)
#define LOCAL_DeeType_QueryAttribute(tp_invoker, tp_self) DeeType_QueryAttributeStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#else /* ... */
#define LOCAL_DeeType_QueryAttribute(tp_invoker, tp_self) DeeType_QueryAttributeStringHash(tp_invoker, tp_self, attr, hash)
#endif /* !... */

/* Helpers to access class instance attributes */
#ifdef LOCAL_IS_GET
#define LOCAL_DeeInstance_AccessAttribute(desc, self, this_arg, attr) DeeInstance_GetAttribute(desc, self, this_arg, attr)
#elif defined(LOCAL_IS_BOUND)
#define LOCAL_DeeInstance_AccessAttribute(desc, self, this_arg, attr) DeeInstance_BoundAttribute(desc, self, this_arg, attr)
#elif defined(LOCAL_IS_CALL)
#define LOCAL_DeeInstance_AccessAttribute(desc, self, this_arg, attr) DeeInstance_CallAttribute(desc, self, this_arg, attr, argc, argv)
#elif defined(LOCAL_IS_CALL_KW)
#define LOCAL_DeeInstance_AccessAttribute(desc, self, this_arg, attr) DeeInstance_CallAttributeKw(desc, self, this_arg, attr, argc, argv, kw)
#elif defined(LOCAL_IS_CALL_TUPLE)
#define LOCAL_DeeInstance_AccessAttribute(desc, self, this_arg, attr) DeeInstance_CallAttributeTuple(desc, self, this_arg, attr, args)
#elif defined(LOCAL_IS_CALL_TUPLE_KW)
#define LOCAL_DeeInstance_AccessAttribute(desc, self, this_arg, attr) DeeInstance_CallAttributeTupleKw(desc, self, this_arg, attr, args, kw)
#elif defined(LOCAL_IS_VCALLF)
#define LOCAL_DeeInstance_AccessAttribute(desc, self, this_arg, attr) DeeInstance_VCallAttributef(desc, self, this_arg, attr, format, args)
#elif defined(LOCAL_IS_DEL)
#define LOCAL_DeeInstance_AccessAttribute(desc, self, this_arg, attr) DeeInstance_DelAttribute(desc, self, this_arg, attr)
#elif defined(LOCAL_IS_SET)
#define LOCAL_DeeInstance_AccessAttribute(desc, self, this_arg, attr) DeeInstance_SetAttribute(desc, self, this_arg, attr, value)
#endif /* ... */

/* Helpers to access attributes */
#if defined(LOCAL_IS_GET) || defined(LOCAL_IS_CALL_LIKE)
#ifndef LOCAL_HAS_string
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrHash(tp_invoker, tp_self, self, attr, hash)
#elif defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#else /* ... */
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_GetGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_GetMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#endif /* !... */
#ifdef LOCAL_IS_GET
#ifndef LOCAL_HAS_string
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_GetCachedAttrHash(tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_GetMethodAttrHash(tp_invoker, tp_self, self, attr, hash)
#elif defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_GetCachedAttrStringLenHash(tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_GetMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#else /* ... */
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_GetCachedAttrStringHash(tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_GetMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#endif /* !... */
#elif defined(LOCAL_IS_CALL)
#ifndef LOCAL_HAS_string
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrHash(tp_self, self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrHash(tp_invoker, tp_self, self, attr, hash, argc, argv)
#elif defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringLenHash(tp_self, self, attr, attrlen, hash, argc, argv)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash, argc, argv)
#else /* ... */
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringHash(tp_self, self, attr, hash, argc, argv)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash, argc, argv)
#endif /* !... */
#elif defined(LOCAL_IS_CALL_KW)
#ifndef LOCAL_HAS_string
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrHashKw(tp_self, self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrHashKw(tp_invoker, tp_self, self, attr, hash, argc, argv, kw)
#elif defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringLenHashKw(tp_self, self, attr, attrlen, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringLenHashKw(tp_invoker, tp_self, self, attr, attrlen, hash, argc, argv, kw)
#else /* ... */
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringHashKw(tp_self, self, attr, hash, argc, argv, kw)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringHashKw(tp_invoker, tp_self, self, attr, hash, argc, argv, kw)
#endif /* !... */
#elif defined(LOCAL_IS_CALL_TUPLE)
#ifndef LOCAL_HAS_string
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrHashTuple(tp_self, self, attr, hash, args)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrHashTuple(tp_invoker, tp_self, self, attr, hash, args)
#elif defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringLenHashTuple(tp_self, self, attr, attrlen, hash, args)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringLenHashTuple(tp_invoker, tp_self, self, attr, attrlen, hash, args)
#else /* ... */
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringHashTuple(tp_self, self, attr, hash, args)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringHashTuple(tp_invoker, tp_self, self, attr, hash, args)
#endif /* !... */
#elif defined(LOCAL_IS_CALL_TUPLE_KW)
#ifndef LOCAL_HAS_string
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrHashTupleKw(tp_self, self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrHashTupleKw(tp_invoker, tp_self, self, attr, hash, args, kw)
#elif defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringLenHashTupleKw(tp_self, self, attr, attrlen, hash, args, kw)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringLenHashTupleKw(tp_invoker, tp_self, self, attr, attrlen, hash, args, kw)
#else /* ... */
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_CallCachedAttrStringHashTupleKw(tp_self, self, attr, hash, args, kw)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_CallMethodAttrStringHashTupleKw(tp_invoker, tp_self, self, attr, hash, args, kw)
#endif /* !... */
#elif defined(LOCAL_IS_VCALLF)
#ifndef LOCAL_HAS_string
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_VCallCachedAttrHashf(tp_self, self, attr, hash, format, args)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_VCallMethodAttrHashf(tp_invoker, tp_self, self, attr, hash, format, args)
#elif defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_VCallCachedAttrStringLenHashf(tp_self, self, attr, attrlen, hash, format, args)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_VCallMethodAttrStringLenHashf(tp_invoker, tp_self, self, attr, attrlen, hash, format, args)
#else /* ... */
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_VCallCachedAttrStringHashf(tp_self, self, attr, hash, format, args)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_VCallMethodAttrStringHashf(tp_invoker, tp_self, self, attr, hash, format, args)
#endif /* !... */
#endif
#elif defined(LOCAL_IS_BOUND)
#ifndef LOCAL_HAS_string
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_BoundCachedAttrHash(tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_BoundMethodAttrHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_BoundGetSetAttrHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_BoundMemberAttrHash(tp_invoker, tp_self, self, attr, hash)
#elif defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_BoundCachedAttrStringLenHash(tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_BoundMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_BoundGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_BoundMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#else /* ... */
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_BoundCachedAttrStringHash(tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_BoundMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_BoundGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_BoundMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#endif /* !... */
#elif defined(LOCAL_IS_HAS)
#ifndef LOCAL_HAS_string
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_HasCachedAttrHash(tp_self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_HasMethodAttrHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_HasGetSetAttrHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_HasMemberAttrHash(tp_invoker, tp_self, attr, hash)
#elif defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_HasCachedAttrStringLenHash(tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_HasMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_HasGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_HasMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)
#else /* ... */
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_HasCachedAttrStringHash(tp_self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_HasMethodAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_HasGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_HasMemberAttrStringHash(tp_invoker, tp_self, attr, hash)
#endif /* !... */
#elif defined(LOCAL_IS_DEL)
#ifndef LOCAL_HAS_string
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_DelCachedAttrHash(tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_DelMethodAttrHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_DelGetSetAttrHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_DelMemberAttrHash(tp_invoker, tp_self, self, attr, hash)
#elif defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_DelCachedAttrStringLenHash(tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_DelMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_DelGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_DelMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)
#else /* ... */
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_DelCachedAttrStringHash(tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_DelMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_DelGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_DelMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash)
#endif /* !... */
#elif defined(LOCAL_IS_SET)
#ifndef LOCAL_HAS_string
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_SetCachedAttrHash(tp_self, self, attr, hash, value)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_SetMethodAttrHash(tp_invoker, tp_self, self, attr, hash, value)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_SetGetSetAttrHash(tp_invoker, tp_self, self, attr, hash, value)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_SetMemberAttrHash(tp_invoker, tp_self, self, attr, hash, value)
#elif defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_SetCachedAttrStringLenHash(tp_self, self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_SetMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_SetGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash, value)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_SetMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash, value)
#else /* ... */
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_SetCachedAttrStringHash(tp_self, self, attr, hash, value)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_SetMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash, value)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_SetGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash, value)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_SetMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash, value)
#endif /* !... */
#elif defined(LOCAL_IS_FIND)
#define LOCAL_DeeType_AccessCachedAttr(tp_self, self)             DeeType_FindCachedAttr(tp_self, self, retinfo, rules)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) DeeType_FindMethodAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) DeeType_FindGetSetAttr(tp_invoker, tp_self, retinfo, rules)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) DeeType_FindMemberAttr(tp_invoker, tp_self, retinfo, rules)
#elif defined(LOCAL_IS_ENUM)
#define LOCAL_DeeType_AccessMethodAttr(tp_invoker, tp_self, self) type_method_enum(tp_self, (tp_self)->tp_methods, ATTR_IMEMBER, proc, arg)
#define LOCAL_DeeType_AccessGetSetAttr(tp_invoker, tp_self, self) type_getset_enum(tp_self, (tp_self)->tp_getsets, ATTR_IMEMBER | ATTR_PROPERTY, proc, arg)
#define LOCAL_DeeType_AccessMemberAttr(tp_invoker, tp_self, self) type_member_enum(tp_self, (tp_self)->tp_members, ATTR_IMEMBER, proc, arg)
#endif /* ... */

/* Access code for how an attribute is being accessed */
#ifdef LOCAL_IS_SET
#define LOCAL_ATTR_ACCESS_OP ATTR_ACCESS_SET
#elif defined(LOCAL_IS_DEL)
#define LOCAL_ATTR_ACCESS_OP ATTR_ACCESS_DEL
#else /* ... */
#define LOCAL_ATTR_ACCESS_OP ATTR_ACCESS_GET
#endif /* !... */

/* Helpers to throw errors */
#ifndef LOCAL_HAS_string
#define LOCAL_err_unknown_attribute(tp_self, access) err_unknown_attribute(tp_self, attr, access)
#elif defined(LOCAL_HAS_len)
#define LOCAL_err_unknown_attribute(tp_self, access) err_unknown_attribute_string_len(tp_self, attr, attrlen, access)
#else /* ... */
#define LOCAL_err_unknown_attribute(tp_self, access) err_unknown_attribute_string(tp_self, attr, access)
#endif /* !... */

/* Access code for how an attribute is being accessed */
#if defined(LOCAL_IS_ENUM) || defined(LOCAL_IS_FIND)
#define LOCAL_tp_accessattr                                                         tp_enumattr
#define LOCAL_DECLARE_tp_accessattr(tp_accessattr)                                  dssize_t (DCALL *tp_accessattr)(DeeTypeObject *tp_self, DeeObject *self, Dee_enum_t proc, void *arg)
#define LOCAL_DeeType_invoke_attr_tp_accessattr(tp_iter, tp_accessattr, self, attr) (*(tp_accessattr))(tp_iter, self, proc, arg)
#define LOCAL_type_accessattr                                                       type_enumattr
#define LOCAL_module_accessattr                                                     module_enumattr
#define LOCAL_super_accessattr                                                      super_enumattr
#elif defined(LOCAL_IS_SET)
#define LOCAL_tp_accessattr                                                         tp_setattr
#define LOCAL_DECLARE_tp_accessattr(tp_accessattr)                                  int (DCALL *tp_accessattr)(DeeObject *, DeeObject *, DeeObject *)
#define LOCAL_DeeType_invoke_attr_tp_accessattr(tp_iter, tp_accessattr, self, attr) DeeType_invoke_attr_tp_setattr(tp_iter, tp_accessattr, self, attr, value)
#define LOCAL_type_accessattr                                                       type_setattr
#define LOCAL_module_accessattr                                                     module_setattr
#define LOCAL_super_accessattr                                                      super_setattr
#elif defined(LOCAL_IS_DEL)
#define LOCAL_tp_accessattr                                                         tp_delattr
#define LOCAL_DECLARE_tp_accessattr(tp_accessattr)                                  int (DCALL *tp_accessattr)(DeeObject *, DeeObject *)
#define LOCAL_DeeType_invoke_attr_tp_accessattr(tp_iter, tp_accessattr, self, attr) DeeType_invoke_attr_tp_delattr(tp_iter, tp_accessattr, self, attr)
#define LOCAL_type_accessattr                                                       type_delattr
#define LOCAL_module_accessattr                                                     module_delattr
#define LOCAL_super_accessattr                                                      super_delattr
#else /* ... */
#define LOCAL_tp_accessattr                                                         tp_getattr
#define LOCAL_DECLARE_tp_accessattr(tp_accessattr)                                  DREF DeeObject *(DCALL *tp_accessattr)(DeeObject *, DeeObject *)
#define LOCAL_DeeType_invoke_attr_tp_accessattr(tp_iter, tp_accessattr, self, attr) DeeType_invoke_attr_tp_getattr(tp_iter, tp_accessattr, self, attr)
#define LOCAL_type_accessattr                                                       type_getattr
#define LOCAL_module_accessattr                                                     module_getattr
#define LOCAL_super_accessattr                                                      super_getattr
#define LOCAL_seqeach_accessattr                                                    (*(DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeach_getattr)
#define LOCAL_seqeachw_accessattr                                                   (*(DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&seqeachw_getattr)
#endif /* !... */


/* Accessors for `Sequence.each'. */
#ifdef CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS
#if defined(LOCAL_IS_CALL) && !defined(LOCAL_HAS_string)
#define LOCAL_DeeSeqEach_AccessAttr(self) DeeSeqEach_CallAttr(self, attr, argc, argv)
#elif defined(LOCAL_IS_CALL) && defined(LOCAL_HAS_len)
#define LOCAL_DeeSeqEach_AccessAttr(self) DeeSeqEach_CallAttrStringLenHash(self, attr, attrlen, hash, argc, argv)
#elif defined(LOCAL_IS_CALL) && defined(LOCAL_HAS_len)
#define LOCAL_DeeSeqEach_AccessAttr(self) DeeSeqEach_CallAttrStringHash(self, attr, hash, argc, argv)
#elif defined(LOCAL_IS_CALL_KW) && !defined(LOCAL_HAS_string)
#define LOCAL_DeeSeqEach_AccessAttr(self) DeeSeqEach_CallAttrKw(self, attr, argc, argv, kw)
#elif defined(LOCAL_IS_CALL_KW) && defined(LOCAL_HAS_len)
#define LOCAL_DeeSeqEach_AccessAttr(self) DeeSeqEach_CallAttrStringLenHashKw(self, attr, attrlen, hash, argc, argv, kw)
#elif defined(LOCAL_IS_CALL_KW) && defined(LOCAL_HAS_len)
#define LOCAL_DeeSeqEach_AccessAttr(self) DeeSeqEach_CallAttrStringHashKw(self, attr, hash, argc, argv, kw)
#elif defined(LOCAL_IS_CALL_TUPLE) && !defined(LOCAL_HAS_string)
#define LOCAL_DeeSeqEach_AccessAttr(self) DeeSeqEach_CallAttrTuple(self, attr, args)
#elif defined(LOCAL_IS_CALL_TUPLE) && defined(LOCAL_HAS_len)
#define LOCAL_DeeSeqEach_AccessAttr(self) DeeSeqEach_CallAttrStringLenHashTuple(self, attr, attrlen, hash, args)
#elif defined(LOCAL_IS_CALL_TUPLE) && defined(LOCAL_HAS_len)
#define LOCAL_DeeSeqEach_AccessAttr(self) DeeSeqEach_CallAttrStringHashTuple(self, attr, hash, args)
#elif defined(LOCAL_IS_CALL_TUPLE_KW) && !defined(LOCAL_HAS_string)
#define LOCAL_DeeSeqEach_AccessAttr(self) DeeSeqEach_CallAttrTupleKw(self, attr, args, kw)
#elif defined(LOCAL_IS_CALL_TUPLE_KW) && defined(LOCAL_HAS_len)
#define LOCAL_DeeSeqEach_AccessAttr(self) DeeSeqEach_CallAttrStringLenHashTupleKw(self, attr, attrlen, hash, args, kw)
#elif defined(LOCAL_IS_CALL_TUPLE_KW) && defined(LOCAL_HAS_len)
#define LOCAL_DeeSeqEach_AccessAttr(self) DeeSeqEach_CallAttrStringHashTupleKw(self, attr, hash, args, kw)
#endif /* ... */
#endif /* CONFIG_HAVE_SEQEACH_ATTRIBUTE_OPTIMIZATIONS */


/* Accessors for special types. */
#if defined(LOCAL_IS_GET) && !defined(LOCAL_HAS_string)
//#define LOCAL_DeeType_AccessAttr(self)       DeeType_GetAttr(self, attr)
//#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_GetAttr(self, attr)
//#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_GetAttrHash(self, attr, hash)
//#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_GetAttrHash(self, attr, hash)
#elif defined(LOCAL_IS_GET) && defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessAttr(self)       DeeType_GetAttrStringLen(self, attr, attrlen)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_GetAttrStringLen(self, attr, attrlen)
#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_GetAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_GetAttrStringLenHash(self, attr, attrlen, hash)
#elif defined(LOCAL_IS_GET)
#define LOCAL_DeeType_AccessAttr(self)       DeeType_GetAttrString(self, attr)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_GetAttrString(self, attr)
#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_GetAttrStringHash(self, attr, hash)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_GetAttrStringHash(self, attr, hash)
#elif defined(LOCAL_IS_BOUND) && !defined(LOCAL_HAS_string)
#define LOCAL_DeeType_AccessAttr(self)       DeeType_BoundAttr(self, attr)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_BoundAttr(self, attr)
#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_BoundAttrHash(self, attr, hash)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_BoundAttrHash(self, attr, hash)
#elif defined(LOCAL_IS_BOUND) && defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessAttr(self)       DeeType_BoundAttrStringLen(self, attr, attrlen)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_BoundAttrStringLen(self, attr, attrlen)
#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_BoundAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_BoundAttrStringLenHash(self, attr, attrlen, hash)
#elif defined(LOCAL_IS_BOUND)
#define LOCAL_DeeType_AccessAttr(self)       DeeType_BoundAttrString(self, attr)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_BoundAttrString(self, attr)
#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_BoundAttrStringHash(self, attr, hash)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_BoundAttrStringHash(self, attr, hash)
#elif defined(LOCAL_IS_HAS) && !defined(LOCAL_HAS_string)
#define LOCAL_DeeType_AccessAttr(self)       DeeType_HasAttr(self, attr)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_HasAttr(self, attr)
#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_HasAttrHash(self, attr, hash)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_HasAttrHash(self, attr, hash)
#elif defined(LOCAL_IS_HAS) && defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessAttr(self)       DeeType_HasAttrStringLen(self, attr, attrlen)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_HasAttrStringLen(self, attr, attrlen)
#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_HasAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_HasAttrStringLenHash(self, attr, attrlen, hash)
#elif defined(LOCAL_IS_HAS)
#define LOCAL_DeeType_AccessAttr(self)       DeeType_HasAttrString(self, attr)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_HasAttrString(self, attr)
#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_HasAttrStringHash(self, attr, hash)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_HasAttrStringHash(self, attr, hash)
#elif defined(LOCAL_IS_DEL) && !defined(LOCAL_HAS_string)
//#define LOCAL_DeeType_AccessAttr(self)       DeeType_DelAttr(self, attr)
//#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_DelAttr(self, attr)
//#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_DelAttrHash(self, attr, hash)
//#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_DelAttrHash(self, attr, hash)
#elif defined(LOCAL_IS_DEL) && defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessAttr(self)       DeeType_DelAttrStringLen(self, attr, attrlen)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_DelAttrStringLen(self, attr, attrlen)
#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_DelAttrStringLenHash(self, attr, attrlen, hash)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_DelAttrStringLenHash(self, attr, attrlen, hash)
#elif defined(LOCAL_IS_DEL)
#define LOCAL_DeeType_AccessAttr(self)       DeeType_DelAttrString(self, attr)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_DelAttrString(self, attr)
#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_DelAttrStringHash(self, attr, hash)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_DelAttrStringHash(self, attr, hash)
#elif defined(LOCAL_IS_SET) && !defined(LOCAL_HAS_string)
//#define LOCAL_DeeType_AccessAttr(self)       DeeType_SetAttr(self, attr, value)
//#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_SetAttr(self, attr, value)
//#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_SetAttrHash(self, attr, hash, value)
//#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_SetAttrHash(self, attr, hash, value)
#elif defined(LOCAL_IS_SET) && defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessAttr(self)       DeeType_SetAttrStringLen(self, attr, attrlen, value)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_SetAttrStringLen(self, attr, attrlen, value)
#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_SetAttrStringLenHash(self, attr, attrlen, hash, value)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_SetAttrStringLenHash(self, attr, attrlen, hash, value)
#elif defined(LOCAL_IS_SET)
#define LOCAL_DeeType_AccessAttr(self)       DeeType_SetAttrString(self, attr, value)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_SetAttrString(self, attr, value)
#define LOCAL_DeeType_AccessAttrHash(self)   DeeType_SetAttrStringHash(self, attr, hash, value)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_SetAttrStringHash(self, attr, hash, value)
#elif defined(LOCAL_IS_FIND)
#define LOCAL_DeeType_AccessAttr(self)   DeeType_FindAttr(self, retinfo, rules)
#define LOCAL_DeeModule_AccessAttr(self) DeeModule_FindAttr(self, retinfo, rules)
#elif defined(LOCAL_IS_ENUM)
//#define LOCAL_DeeType_AccessAttr(self)   DeeType_EnumAttr(self, proc, arg)
//#define LOCAL_DeeModule_AccessAttr(self) DeeModule_EnumAttr(self, proc, arg)
#endif /* ... */

#ifdef LOCAL_IS_CALL_LIKE
#if defined(LOCAL_IS_CALL) && !defined(LOCAL_HAS_string)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_GetAttr(self, attr)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_GetAttrHash(self, attr, hash)
#elif defined(LOCAL_IS_CALL) && defined(LOCAL_HAS_len)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_GetAttrStringLen(self, attr, attrlen)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_GetAttrStringLenHash(self, attr, attrlen, hash)
#elif defined(LOCAL_IS_CALL)
#define LOCAL_DeeModule_AccessAttr(self)     DeeModule_GetAttrString(self, attr)
#define LOCAL_DeeModule_AccessAttrHash(self) DeeModule_GetAttrStringHash(self, attr, hash)
#endif /* ... */
#if defined(LOCAL_IS_CALL) && !defined(LOCAL_HAS_string)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_CallAttr(self, attr, argc, argv)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_CallAttrHash(self, attr, hash, argc, argv)
#elif defined(LOCAL_IS_CALL) && defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_CallAttrStringLen(self, attr, attrlen, argc, argv)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_CallAttrStringLenHash(self, attr, attrlen, hash, argc, argv)
#elif defined(LOCAL_IS_CALL)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_CallAttrString(self, attr, argc, argv)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_CallAttrStringHash(self, attr, hash, argc, argv)
#elif defined(LOCAL_IS_CALL_KW) && !defined(LOCAL_HAS_string)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_CallAttrKw(self, attr, argc, argv, kw)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_CallAttrHashKw(self, attr, hash, argc, argv, kw)
#elif defined(LOCAL_IS_CALL_KW) && defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_CallAttrStringLenKw(self, attr, attrlen, argc, argv, kw)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_CallAttrStringLenHashKw(self, attr, attrlen, hash, argc, argv, kw)
#elif defined(LOCAL_IS_CALL_KW)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_CallAttrStringKw(self, attr, argc, argv, kw)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_CallAttrStringHashKw(self, attr, hash, argc, argv, kw)
#elif defined(LOCAL_IS_CALL_TUPLE) && !defined(LOCAL_HAS_string)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_CallAttrTuple(self, attr, args)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_CallAttrHashTuple(self, attr, hash, args)
#elif defined(LOCAL_IS_CALL_TUPLE) && defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_CallAttrStringLenTuple(self, attr, attrlen, args)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_CallAttrStringLenHashTuple(self, attr, attrlen, hash, args)
#elif defined(LOCAL_IS_CALL_TUPLE)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_CallAttrStringTuple(self, attr, args)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_CallAttrStringHashTuple(self, attr, hash, args)
#elif defined(LOCAL_IS_CALL_TUPLE_KW) && !defined(LOCAL_HAS_string)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_CallAttrTupleKw(self, attr, args, kw)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_CallAttrHashTupleKw(self, attr, hash, args, kw)
#elif defined(LOCAL_IS_CALL_TUPLE_KW) && defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_CallAttrStringLenTupleKw(self, attr, attrlen, args, kw)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_CallAttrStringLenHashTupleKw(self, attr, attrlen, hash, args, kw)
#elif defined(LOCAL_IS_CALL_TUPLE_KW)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_CallAttrStringTupleKw(self, attr, args, kw)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_CallAttrStringHashTupleKw(self, attr, hash, args, kw)
#elif defined(LOCAL_IS_VCALLF) && !defined(LOCAL_HAS_string)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_VCallAttrf(self, attr, format, args)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_VCallAttrHashf(self, attr, hash, format, args)
#elif defined(LOCAL_IS_VCALLF) && defined(LOCAL_HAS_len)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_VCallAttrStringLenf(self, attr, attrlen, format, args)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_VCallAttrStringLenHashf(self, attr, attrlen, hash, format, args)
#elif defined(LOCAL_IS_VCALLF)
#define LOCAL_DeeType_AccessAttr(self)     DeeType_VCallAttrStringf(self, attr, format, args)
#define LOCAL_DeeType_AccessAttrHash(self) DeeType_VCallAttrStringHashf(self, attr, hash, format, args)
#endif /* ... */
#endif /* LOCAL_IS_CALL_LIKE */



/* What visibility should the function have? */
#if defined(LOCAL_HAS_tp_self) && !defined(LOCAL_IS_ENUM) && !defined(LOCAL_IS_FIND)
#define LOCAL_DECL INTERN
#else /* ... */
#define LOCAL_DECL PUBLIC
#endif /* !... */

/* Figure out the non-null attribute. */
#if defined(DEFINE_DeeObject_FindAttr)
#define LOCAL_ATTR_NONNULL NONNULL((1, 3, 4))
#elif defined(DEFINE_DeeObject_EnumAttr)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2))
#elif defined(LOCAL_HAS_tp_self) && defined(LOCAL_HAS_len) && defined(LOCAL_HAS_hash) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3, 6))
#elif defined(LOCAL_HAS_tp_self) && (defined(LOCAL_HAS_len) || defined(LOCAL_HAS_hash)) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3, 5))
#elif defined(LOCAL_HAS_tp_self) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3, 4))
#elif defined(LOCAL_HAS_tp_self)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3))
#elif defined(LOCAL_HAS_len) && defined(LOCAL_HAS_hash) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 5))
#elif (defined(LOCAL_HAS_len) || defined(LOCAL_HAS_hash)) && (defined(LOCAL_IS_SET) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW))
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 4))
#elif defined(LOCAL_IS_SET) || defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3))
#else /* ... */
#define LOCAL_ATTR_NONNULL NONNULL((1, 2))
#endif /* !... */



#ifdef LOCAL_IS_FIND
#ifndef DEE_OBJECT_FINDATTR_HELPERS_DEFINED
#define DEE_OBJECT_FINDATTR_HELPERS_DEFINED
struct attribute_lookup_data {
	struct attribute_info               *ald_info;    /* [1..1] The result info. */
	struct attribute_lookup_rules const *ald_rules;   /* [1..1] Lookup rules */
	bool                                 ald_fnddecl; /* [valid_if(ald_rules->alr_decl != NULL)]
	                                                   * Set to true after `alr_decl' had been encountered. */
};

PRIVATE dssize_t DCALL
attribute_lookup_enum(DeeObject *__restrict declarator,
                      char const *__restrict attr_name, char const *attr_doc,
                      uint16_t perm, DeeTypeObject *attr_type,
                      struct attribute_lookup_data *__restrict arg) {
	dhash_t attr_hash;
	struct attribute_info *result;
	struct attribute_lookup_rules const *rules = arg->ald_rules;
	if (rules->alr_decl) {
		if (declarator != rules->alr_decl) {
			if (arg->ald_fnddecl)
				return -3; /* The requested declarator came and went without a match... */
			return 0;
		}
		arg->ald_fnddecl = true;
	}
	if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
		return 0;
	if (perm & ATTR_NAMEOBJ) {
		attr_hash = DeeString_Hash((DeeObject *)COMPILER_CONTAINER_OF(attr_name, DeeStringObject, s_str));
	} else {
		attr_hash = Dee_HashStr(attr_name);
	}
	if (attr_hash != rules->alr_hash)
		return 0;
	if (strcmp(attr_name, arg->ald_rules->alr_name) != 0)
		return 0;

	/* This is the one! */
	result = arg->ald_info;
	if (!attr_doc) {
		ASSERT(!(perm & ATTR_DOCOBJ));
		result->a_doc = NULL;
	} else if (perm & ATTR_DOCOBJ) {
		result->a_doc = attr_doc;
		Dee_Incref(COMPILER_CONTAINER_OF(attr_doc, DeeStringObject, s_str));
	} else {
		result->a_doc = attr_doc;
	}
	result->a_decl = declarator;
	Dee_Incref(declarator);
	result->a_perm     = perm;
	result->a_attrtype = attr_type;
	Dee_XIncref(attr_type);
	return -2; /* Stop enumeration! */
}

#endif /* !DEE_OBJECT_FINDATTR_HELPERS_DEFINED */
#endif /* LOCAL_IS_FIND */




/************************************************************************/
/* Access an object attribute                                           */
/************************************************************************/
LOCAL_DECL WUNUSED LOCAL_ATTR_NONNULL LOCAL_return_t
(DCALL LOCAL_DeeObject_AccessAttr)(/**/
#ifdef LOCAL_HAS_tp_self
                                   DeeTypeObject *tp_self,
#endif /* LOCAL_HAS_tp_self */
                                   DeeObject *self
#ifdef LOCAL_IS_ENUM
                                   , denum_t proc, void *arg
#else /* LOCAL_IS_ENUM */
#ifdef LOCAL_IS_FIND
                                   , struct attribute_info *__restrict retinfo
                                   , struct attribute_lookup_rules const *__restrict rules
#else /* LOCAL_IS_FIND */
#ifdef LOCAL_HAS_string
                                   , char const *__restrict attr
#else /* LOCAL_HAS_string */
                                   , /*String*/ DeeObject *attr
#endif /* !LOCAL_HAS_string */
#ifdef LOCAL_HAS_len
                                   , size_t attrlen
#endif /* LOCAL_HAS_len */
#ifdef LOCAL_HAS_hash
                                   , dhash_t hash
#endif /* LOCAL_HAS_hash */
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
#elif defined(LOCAL_IS_SET)
                                   , DeeObject *value
#endif /* ... */
#endif /* !LOCAL_IS_FIND */
#endif /* !LOCAL_IS_ENUM */
                                   ) {
#if defined(LOCAL_IS_BOUND) || defined(LOCAL_IS_HAS)
#define LOCAL_IS_TEST_FUNCTION
#endif /* LOCAL_IS_BOUND || LOCAL_IS_HAS */
#ifdef LOCAL_IS_CALL_LIKE
#define LOCAL_invoke_result_OR_done invoke_result
#else /* LOCAL_IS_CALL_LIKE */
#define LOCAL_invoke_result_OR_done done
#endif /* !LOCAL_IS_CALL_LIKE */
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
	DeeTypeObject *tp_iter;
#ifndef LOCAL_HAS_tp_self
	DeeTypeObject *tp_self = Dee_TYPE(self);
#endif /* LOCAL_HAS_tp_self */
#ifdef LOCAL_IS_ENUM
	LOCAL_return_t final_result = 0;
#endif /* LOCAL_IS_ENUM */
#if !defined(LOCAL_HAS_hash) && defined(LOCAL_DeeType_AccessCachedAttr) && !defined(LOCAL_IS_ENUM) && !defined(LOCAL_IS_FIND)
#define LOCAL_HAS_local_hash
	dhash_t hash;
#endif /* !LOCAL_HAS_hash && LOCAL_DeeType_AccessCachedAttr && !LOCAL_IS_ENUM && !LOCAL_IS_FIND */
#ifdef LOCAL_HAS_hash
#define LOCAL_DeeType_AccessAttr_LOCAL(self)   LOCAL_DeeType_AccessAttrHash(self)
#define LOCAL_DeeModule_AccessAttr_LOCAL(self) LOCAL_DeeModule_AccessAttrHash(self)
#else /* LOCAL_HAS_hash */
#define LOCAL_DeeType_AccessAttr_LOCAL(self)   LOCAL_DeeType_AccessAttr(self)
#define LOCAL_DeeModule_AccessAttr_LOCAL(self) LOCAL_DeeModule_AccessAttr(self)
#endif /* !LOCAL_HAS_hash */
	LOCAL_return_t result;

	/* Verify arguments. */
#ifdef LOCAL_HAS_tp_self
	ASSERT_OBJECT_TYPE(tp_self, &DeeType_Type);
#endif /* LOCAL_HAS_tp_self */
#ifdef LOCAL_HAS_tp_self
	ASSERT_OBJECT_TYPE(self, tp_self);
#else /* LOCAL_HAS_tp_self */
	ASSERT_OBJECT(self);
#endif /* !LOCAL_HAS_tp_self */
#if !defined(LOCAL_HAS_string) && !defined(LOCAL_IS_ENUM) && !defined(LOCAL_IS_FIND)
	ASSERT_OBJECT_TYPE_EXACT(attr, &DeeString_Type);
#endif /* !LOCAL_HAS_string && !LOCAL_IS_ENUM && !LOCAL_IS_FIND */

#if defined(LOCAL_HAS_string) || !(defined(LOCAL_IS_GET) || defined(LOCAL_IS_DEL) || defined(LOCAL_IS_SET) || defined(LOCAL_IS_ENUM))
#define LOCAL_HAS_super_optimizations
#endif /* LOCAL_HAS_string || !(LOCAL_IS_GET || LOCAL_IS_DEL || LOCAL_IS_SET || LOCAL_IS_ENUM) */

	tp_iter = tp_self;
#ifdef LOCAL_HAS_super_optimizations
again:
#endif /* LOCAL_HAS_super_optimizations */
	if (tp_iter->tp_attr != NULL)
		goto do_tp_iter_attr;

	/* Calculate hash if not given by caller. */
#ifdef LOCAL_HAS_local_hash
#ifndef LOCAL_HAS_string
	hash = DeeString_Hash(attr);
#elif defined(LOCAL_HAS_len)
	hash = Dee_HashPtr(attr, attrlen);
#else /* ... */
	hash = Dee_HashStr(attr);
#endif /* !... */
#endif /* LOCAL_HAS_local_hash */

	/* Try to access the attribute from cache. */
#ifdef LOCAL_DeeType_AccessCachedAttr
	result = LOCAL_DeeType_AccessCachedAttr(tp_iter, self);
	LOCAL_process_result(result, done);
#endif /* LOCAL_DeeType_AccessCachedAttr */

	/* Slow path: must check for the attribute everywhere. */
	for (;;) {
		if (DeeType_IsClass(tp_iter)) {
#ifdef LOCAL_IS_ENUM
			result = DeeClass_EnumInstanceAttributes(tp_iter, self, proc, arg);
			LOCAL_process_result(result, done);
#elif defined(LOCAL_IS_FIND)
			result = DeeClass_FindInstanceAttribute(tp_self, tp_iter, self, retinfo, rules);
			LOCAL_process_result(result, done);
#else /* LOCAL_IS_ENUM */
			struct class_attribute *cattr;
			cattr = LOCAL_DeeType_QueryAttribute(tp_self, tp_iter);
			if (cattr != NULL) {
#ifdef LOCAL_IS_HAS
				return 1;
#else /* LOCAL_IS_HAS */
				struct class_desc *desc;
				/* Check if we're allowed to access this cattr. */
				if (!class_attribute_mayaccess(cattr, tp_iter)) {
					err_class_protected_member(tp_iter, cattr);
					goto err;
				}
				desc = DeeClass_DESC(tp_iter);
				return LOCAL_DeeInstance_AccessAttribute(desc, DeeInstance_DESC(desc, self),
				                                         self, cattr);
#endif /* !LOCAL_IS_HAS */
			}
#endif /* !LOCAL_IS_ENUM */
		} else {
			/* Check for C-level attribute declarations */
			if (tp_iter->tp_methods) {
				result = LOCAL_DeeType_AccessMethodAttr(tp_self, tp_iter, self);
				LOCAL_process_result(result, done);
			}
			if (tp_iter->tp_getsets) {
				result = LOCAL_DeeType_AccessGetSetAttr(tp_self, tp_iter, self);
				LOCAL_process_result(result, LOCAL_invoke_result_OR_done);
			}
			if (tp_iter->tp_members) {
				result = LOCAL_DeeType_AccessMemberAttr(tp_self, tp_iter, self);
				LOCAL_process_result(result, LOCAL_invoke_result_OR_done);
			}
		}

		/* Move on to the next base class. */
		tp_iter = DeeType_Base(tp_iter);
		if (!tp_iter)
			break;

		/* Check for user-defined attribute operators. */
		if (tp_iter->tp_attr != NULL) {
#ifdef LOCAL_IS_TEST_FUNCTION
			DREF DeeObject *found_object;
#define LOCAL_result_OR_found_object found_object
#else /* LOCAL_IS_TEST_FUNCTION */
#define LOCAL_result_OR_found_object result
#endif /* !LOCAL_IS_TEST_FUNCTION */
			LOCAL_DECLARE_tp_accessattr(tp_accessattr);
do_tp_iter_attr:
			tp_accessattr = tp_iter->tp_attr->LOCAL_tp_accessattr;

			/* Special optimizations for certain object types. */

#ifdef LOCAL_DeeType_AccessAttr
			if (tp_accessattr == &LOCAL_type_accessattr)
				return LOCAL_DeeType_AccessAttr_LOCAL((DeeTypeObject *)self);
#endif /* LOCAL_DeeType_AccessAttr */

#ifdef LOCAL_DeeModule_AccessAttr
			if (tp_accessattr == &LOCAL_module_accessattr) {
				result = LOCAL_DeeModule_AccessAttr_LOCAL((DeeModuleObject *)self);
				goto LOCAL_invoke_result_OR_done;
			}
#endif /* LOCAL_DeeModule_AccessAttr */

			/* Operations that can't be mapped directly to the actual operators have super-optimizations */
#ifdef LOCAL_HAS_super_optimizations
			if (tp_accessattr == &LOCAL_super_accessattr) {
				tp_iter = DeeSuper_TYPE(self);
				self    = DeeSuper_SELF(self);
				tp_self = tp_iter;
				goto again;
			}
#endif /* LOCAL_HAS_super_optimizations */

			/* More special handling for `Sequence.each' */
#if defined(LOCAL_DeeSeqEach_AccessAttr) && defined(LOCAL_seqeach_accessattr)
			if (tp_accessattr == &LOCAL_seqeach_accessattr)
				return LOCAL_DeeSeqEach_AccessAttr(((SeqEachBase *)self)->se_seq);
			if (tp_accessattr == &LOCAL_seqeachw_accessattr)
				return LOCAL_DeeSeqEach_AccessAttr(self);
#endif /* LOCAL_DeeSeqEach_AccessAttr && LOCAL_seqeach_accessattr */

			/* Check for special case: the required operator isn't implemented. */
			if unlikely(!tp_accessattr) {
#if defined(LOCAL_IS_TEST_FUNCTION) || defined(LOCAL_IS_FIND)
				result = LOCAL_ATTR_NOT_FOUND_RESULT;
#endif /* LOCAL_IS_TEST_FUNCTION || LOCAL_IS_FIND */
				break;
			}

			/* Invoke the user-defined operator. */
#ifdef LOCAL_IS_ENUM
			result = LOCAL_DeeType_invoke_attr_tp_accessattr(tp_iter, tp_accessattr, self, attr);
			LOCAL_process_result(result, done);
#elif defined(LOCAL_IS_FIND)
			{
				dssize_t enum_error;
				struct attribute_lookup_data data;
				data.ald_info    = retinfo;
				data.ald_rules   = rules;
				data.ald_fnddecl = false;
				enum_error       = (*tp_accessattr)(tp_iter, self, (denum_t)&attribute_lookup_enum, &data);
				if (enum_error == -1)
					return LOCAL_ERROR_RESULT; /* Error... */
				if (enum_error == 0 || enum_error == -3) {
					/* Not found -- Don't consider attributes from lower levels for custom member access. */
					result = LOCAL_ATTR_NOT_FOUND_RESULT;
				} else {
					ASSERT(enum_error == -2);      /* Found it! */
					result = 0;
				}
			}
#elif !defined(LOCAL_HAS_string)
			LOCAL_result_OR_found_object = LOCAL_DeeType_invoke_attr_tp_accessattr(tp_iter, tp_accessattr, self, attr);
#else /* !LOCAL_HAS_string */
			{
				DREF DeeObject *attr_ob;
				/* Must create a string object for the caller-given c-string. */
#ifdef LOCAL_HAS_len
				attr_ob = DeeString_NewSizedWithHash(attr, attrlen, hash);
#else /* LOCAL_HAS_len */
				/* Don't use `DeeString_NewAutoWithHash()' here:
				 * """ [...] only use this function with statically allocated strings [...] """ */
				attr_ob = DeeString_NewWithHash(attr, hash);
#endif /* !LOCAL_HAS_len */
				if unlikely(!attr_ob)
					goto err;
				LOCAL_result_OR_found_object = LOCAL_DeeType_invoke_attr_tp_accessattr(tp_iter, tp_accessattr, self, attr_ob);
				Dee_Decref(attr_ob);
			}
#endif /* !LOCAL_HAS_string */

#ifdef LOCAL_IS_TEST_FUNCTION
			if (found_object) {
				Dee_Decref(found_object);
				return 1;
			}
#ifdef LOCAL_IS_BOUND
			if (DeeError_Catch(&DeeError_UnboundAttribute))
				return 0;
#endif /* LOCAL_IS_BOUND */
			if (CATCH_ATTRIBUTE_ERROR()) {
#ifdef LOCAL_IS_BOUND
				/* Special case: A user-defined getattr operator threw an error
				 *               indicating that the attribute doesn't exists */
				return -3;
#else /* LOCAL_IS_BOUND */
				return 0;
#endif /* !LOCAL_IS_BOUND */
			}
			goto err;
#elif defined(LOCAL_IS_ENUM)
			break;
#else /* ... */
			goto LOCAL_invoke_result_OR_done;
#endif /* !... */
#undef LOCAL_result_OR_found_object
		}
	}

#if !defined(LOCAL_IS_TEST_FUNCTION) && !defined(LOCAL_IS_ENUM) && !defined(LOCAL_IS_FIND)
	LOCAL_err_unknown_attribute(tp_self, LOCAL_ATTR_ACCESS_OP);
err:
	return LOCAL_ERROR_RESULT;
#endif /* !LOCAL_IS_TEST_FUNCTION && !LOCAL_IS_ENUM && !LOCAL_IS_FIND */
#ifndef LOCAL_IS_ENUM
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
#endif /* LOCAL_IS_CALL_LIKE */
done:
	return result;
#ifdef LOCAL_IS_TEST_FUNCTION
err:
	return LOCAL_ERROR_RESULT;
#endif /* LOCAL_IS_TEST_FUNCTION */
#else /* !LOCAL_IS_ENUM */
	return final_result;
err:
	return result;
#endif /* LOCAL_IS_ENUM */
#undef LOCAL_HAS_local_hash
#undef LOCAL_process_result
#undef LOCAL_invoke_result_OR_done
#undef LOCAL_IS_TEST_FUNCTION
#undef LOCAL_HAS_super_optimizations
#undef LOCAL_DeeType_AccessAttr_LOCAL
#undef LOCAL_DeeModule_AccessAttr_LOCAL
}

#undef LOCAL_DeeType_AccessAttr
#undef LOCAL_DeeType_AccessAttrHash
#undef LOCAL_DeeModule_AccessAttr
#undef LOCAL_DeeModule_AccessAttrHash
#undef LOCAL_DeeSeqEach_AccessAttr

#undef LOCAL_tp_accessattr
#undef LOCAL_DECLARE_tp_accessattr
#undef LOCAL_DeeType_invoke_attr_tp_accessattr
#undef LOCAL_type_accessattr
#undef LOCAL_module_accessattr
#undef LOCAL_super_accessattr
#undef LOCAL_seqeach_accessattr
#undef LOCAL_seqeachw_accessattr

#undef LOCAL_return_t
#undef LOCAL_ATTR_NOT_FOUND_RESULT
#undef LOCAL_ERROR_RESULT
#undef LOCAL_DeeType_QueryAttribute
#undef LOCAL_DeeInstance_AccessAttribute
#undef LOCAL_DeeType_AccessCachedAttr
#undef LOCAL_DeeType_AccessMethodAttr
#undef LOCAL_DeeType_AccessGetSetAttr
#undef LOCAL_DeeType_AccessMemberAttr
#undef LOCAL_DeeObject_AccessAttr
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
#undef LOCAL_IS_ENUM
#undef LOCAL_HAS_string
#undef LOCAL_HAS_hash
#undef LOCAL_HAS_len
#undef LOCAL_HAS_tp_self
#undef LOCAL_DECL
#undef LOCAL_ATTR_NONNULL
#undef LOCAL_ATTR_ACCESS_OP
#undef LOCAL_err_unknown_attribute

DECL_END

#undef DEFINE_DeeObject_GetAttr
#undef DEFINE_DeeObject_TGetAttr
#undef DEFINE_DeeObject_GetAttrStringHash
#undef DEFINE_DeeObject_TGetAttrStringHash
#undef DEFINE_DeeObject_GetAttrStringLenHash
#undef DEFINE_DeeObject_TGetAttrStringLenHash
#undef DEFINE_DeeObject_BoundAttr
#undef DEFINE_DeeObject_TBoundAttr
#undef DEFINE_DeeObject_BoundAttrStringHash
#undef DEFINE_DeeObject_TBoundAttrStringHash
#undef DEFINE_DeeObject_BoundAttrStringLenHash
#undef DEFINE_DeeObject_TBoundAttrStringLenHash
#undef DEFINE_DeeObject_CallAttr
#undef DEFINE_DeeObject_TCallAttr
#undef DEFINE_DeeObject_CallAttrStringHash
#undef DEFINE_DeeObject_TCallAttrStringHash
#undef DEFINE_DeeObject_CallAttrStringLenHash
#undef DEFINE_DeeObject_TCallAttrStringLenHash
#undef DEFINE_DeeObject_CallAttrKw
#undef DEFINE_DeeObject_TCallAttrKw
#undef DEFINE_DeeObject_CallAttrStringHashKw
#undef DEFINE_DeeObject_TCallAttrStringHashKw
#undef DEFINE_DeeObject_CallAttrStringLenHashKw
#undef DEFINE_DeeObject_TCallAttrStringLenHashKw
#undef DEFINE_DeeObject_CallAttrTuple
#undef DEFINE_DeeObject_TCallAttrTuple
#undef DEFINE_DeeObject_CallAttrStringHashTuple
#undef DEFINE_DeeObject_TCallAttrStringHashTuple
#undef DEFINE_DeeObject_CallAttrStringLenHashTuple
#undef DEFINE_DeeObject_TCallAttrStringLenHashTuple
#undef DEFINE_DeeObject_CallAttrTupleKw
#undef DEFINE_DeeObject_TCallAttrTupleKw
#undef DEFINE_DeeObject_CallAttrStringHashTupleKw
#undef DEFINE_DeeObject_TCallAttrStringHashTupleKw
#undef DEFINE_DeeObject_CallAttrStringLenHashTupleKw
#undef DEFINE_DeeObject_TCallAttrStringLenHashTupleKw
#undef DEFINE_DeeObject_VCallAttrf
#undef DEFINE_DeeObject_TVCallAttrf
#undef DEFINE_DeeObject_VCallAttrStringHashf
#undef DEFINE_DeeObject_TVCallAttrStringHashf
#undef DEFINE_DeeObject_VCallAttrStringLenHashf
#undef DEFINE_DeeObject_TVCallAttrStringLenHashf
#undef DEFINE_DeeObject_HasAttr
#undef DEFINE_DeeObject_THasAttr
#undef DEFINE_DeeObject_HasAttrStringHash
#undef DEFINE_DeeObject_THasAttrStringHash
#undef DEFINE_DeeObject_HasAttrStringLenHash
#undef DEFINE_DeeObject_THasAttrStringLenHash
#undef DEFINE_DeeObject_DelAttr
#undef DEFINE_DeeObject_TDelAttr
#undef DEFINE_DeeObject_DelAttrStringHash
#undef DEFINE_DeeObject_TDelAttrStringHash
#undef DEFINE_DeeObject_DelAttrStringLenHash
#undef DEFINE_DeeObject_TDelAttrStringLenHash
#undef DEFINE_DeeObject_SetAttr
#undef DEFINE_DeeObject_TSetAttr
#undef DEFINE_DeeObject_SetAttrStringHash
#undef DEFINE_DeeObject_TSetAttrStringHash
#undef DEFINE_DeeObject_SetAttrStringLenHash
#undef DEFINE_DeeObject_TSetAttrStringLenHash
#undef DEFINE_DeeObject_FindAttr
#undef DEFINE_DeeObject_EnumAttr
