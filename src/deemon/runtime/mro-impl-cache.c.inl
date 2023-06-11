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
#include "mro.c"

#undef DeeType_GetCachedAttr
#undef DeeType_GetCachedAttrLen
#undef DeeType_GetCachedClassAttr
#undef DeeType_GetCachedClassAttrLen
#undef DeeType_GetCachedInstanceAttr
#undef DeeType_GetCachedInstanceAttrLen
#undef DeeType_BoundCachedAttr
#undef DeeType_BoundCachedAttrLen
#undef DeeType_BoundCachedClassAttr
#undef DeeType_BoundCachedClassAttrLen
#undef DeeType_BoundCachedInstanceAttr
#undef DeeType_BoundCachedInstanceAttrLen
#undef DeeType_HasCachedAttr
#undef DeeType_HasCachedAttrLen
#undef DeeType_HasCachedClassAttr
#undef DeeType_HasCachedClassAttrLen
#undef DeeType_HasCachedInstanceAttr
#undef DeeType_HasCachedInstanceAttrLen
#undef DeeType_DelCachedAttr
#undef DeeType_DelCachedAttrLen
#undef DeeType_DelCachedClassAttr
#undef DeeType_DelCachedClassAttrLen
#undef DeeType_DelCachedInstanceAttr
#undef DeeType_DelCachedInstanceAttrLen
#undef DeeType_SetCachedAttr
#undef DeeType_SetCachedAttrLen
#undef DeeType_SetCachedClassAttr
#undef DeeType_SetCachedClassAttrLen
#undef DeeType_SetCachedInstanceAttr
#undef DeeType_SetCachedInstanceAttrLen
#undef DeeType_SetBasicCachedAttr
#undef DeeType_SetBasicCachedAttrLen
#undef DeeType_SetBasicCachedClassAttr
#undef DeeType_SetBasicCachedClassAttrLen
#undef DeeType_SetBasicCachedInstanceAttr
#undef DeeType_SetBasicCachedInstanceAttrLen
#undef DeeType_CallCachedAttr
#undef DeeType_CallCachedAttrLen
#undef DeeType_CallCachedClassAttr
#undef DeeType_CallCachedClassAttrLen
#undef DeeType_CallCachedInstanceAttr
#undef DeeType_CallCachedInstanceAttrLen
#undef DeeType_CallCachedAttr
#undef DeeType_CallCachedAttrLen
#undef DeeType_CallCachedClassAttr
#undef DeeType_CallCachedClassAttrLen
#undef DeeType_CallCachedInstanceAttr
#undef DeeType_CallCachedInstanceAttrLen
#undef DeeType_CallCachedAttrTuple
#undef DeeType_CallCachedAttrLenTuple
#undef DeeType_CallCachedClassAttrTuple
#undef DeeType_CallCachedClassAttrLenTuple
#undef DeeType_CallCachedInstanceAttrTuple
#undef DeeType_CallCachedInstanceAttrLenTuple
#undef DeeType_CallCachedAttr
#undef DeeType_CallCachedAttrLen
#undef DeeType_CallCachedClassAttr
#undef DeeType_CallCachedClassAttrLen
#undef DeeType_CallCachedInstanceAttr
#undef DeeType_CallCachedInstanceAttrLen
#undef DeeType_VCallCachedAttrf
#undef DeeType_VCallCachedAttrLenf
#undef DeeType_VCallCachedClassAttrf
#undef DeeType_VCallCachedClassAttrLenf
#undef DeeType_VCallCachedInstanceAttrf
#undef DeeType_VCallCachedInstanceAttrLenf
#undef DeeType_FindCachedAttr
#undef DeeType_FindCachedClassAttr

//#define DEFINE_DeeType_GetCachedAttr
//#define DEFINE_DeeType_GetCachedAttrLen
//#define DEFINE_DeeType_GetCachedClassAttr
//#define DEFINE_DeeType_GetCachedClassAttrLen
//#define DEFINE_DeeType_GetCachedInstanceAttr
//#define DEFINE_DeeType_GetCachedInstanceAttrLen
//#define DEFINE_DeeType_BoundCachedAttr
//#define DEFINE_DeeType_BoundCachedAttrLen
//#define DEFINE_DeeType_BoundCachedClassAttr
//#define DEFINE_DeeType_BoundCachedClassAttrLen
//#define DEFINE_DeeType_BoundCachedInstanceAttr
//#define DEFINE_DeeType_BoundCachedInstanceAttrLen
//#define DEFINE_DeeType_HasCachedAttr
//#define DEFINE_DeeType_HasCachedAttrLen
//#define DEFINE_DeeType_HasCachedClassAttr
//#define DEFINE_DeeType_HasCachedClassAttrLen
//#define DEFINE_DeeType_DelCachedAttr
//#define DEFINE_DeeType_DelCachedAttrLen
//#define DEFINE_DeeType_DelCachedClassAttr
//#define DEFINE_DeeType_DelCachedClassAttrLen
//#define DEFINE_DeeType_DelCachedInstanceAttr
//#define DEFINE_DeeType_DelCachedInstanceAttrLen
//#define DEFINE_DeeType_SetCachedAttr
//#define DEFINE_DeeType_SetCachedAttrLen
//#define DEFINE_DeeType_SetCachedClassAttr
//#define DEFINE_DeeType_SetCachedClassAttrLen
//#define DEFINE_DeeType_SetCachedInstanceAttr
//#define DEFINE_DeeType_SetCachedInstanceAttrLen
//#define DEFINE_DeeType_SetBasicCachedAttr
//#define DEFINE_DeeType_SetBasicCachedAttrLen
//#define DEFINE_DeeType_SetBasicCachedClassAttr
//#define DEFINE_DeeType_SetBasicCachedClassAttrLen
//#define DEFINE_DeeType_SetBasicCachedInstanceAttr
//#define DEFINE_DeeType_SetBasicCachedInstanceAttrLen
//#define DEFINE_DeeType_CallCachedAttr
//#define DEFINE_DeeType_CallCachedAttrLen
//#define DEFINE_DeeType_CallCachedClassAttr
//#define DEFINE_DeeType_CallCachedClassAttrLen
//#define DEFINE_DeeType_CallCachedInstanceAttr
//#define DEFINE_DeeType_CallCachedInstanceAttrLen
//#define DEFINE_DeeType_CallCachedAttrKw
//#define DEFINE_DeeType_CallCachedAttrLenKw
//#define DEFINE_DeeType_CallCachedClassAttrKw
//#define DEFINE_DeeType_CallCachedClassAttrLenKw
//#define DEFINE_DeeType_CallCachedInstanceAttrKw
//#define DEFINE_DeeType_CallCachedInstanceAttrLenKw
//#define DEFINE_DeeType_CallCachedAttrTuple
//#define DEFINE_DeeType_CallCachedAttrLenTuple
//#define DEFINE_DeeType_CallCachedClassAttrTuple
//#define DEFINE_DeeType_CallCachedClassAttrLenTuple
//#define DEFINE_DeeType_CallCachedInstanceAttrTuple
//#define DEFINE_DeeType_CallCachedInstanceAttrLenTuple
//#define DEFINE_DeeType_CallCachedAttrTupleKw
//#define DEFINE_DeeType_CallCachedAttrLenTupleKw
//#define DEFINE_DeeType_CallCachedClassAttrTupleKw
//#define DEFINE_DeeType_CallCachedClassAttrLenTupleKw
//#define DEFINE_DeeType_CallCachedInstanceAttrTupleKw
//#define DEFINE_DeeType_CallCachedInstanceAttrLenTupleKw
//#define DEFINE_DeeType_VCallCachedAttrf
//#define DEFINE_DeeType_VCallCachedAttrLenf
//#define DEFINE_DeeType_VCallCachedClassAttrf
//#define DEFINE_DeeType_VCallCachedClassAttrLenf
//#define DEFINE_DeeType_VCallCachedInstanceAttrf
//#define DEFINE_DeeType_VCallCachedInstanceAttrLenf
//#define DEFINE_DeeType_FindCachedAttr
#define DEFINE_DeeType_FindCachedClassAttr
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeType_GetCachedAttr) +                    \
     defined(DEFINE_DeeType_GetCachedAttrLen) +                 \
     defined(DEFINE_DeeType_GetCachedClassAttr) +               \
     defined(DEFINE_DeeType_GetCachedClassAttrLen) +            \
     defined(DEFINE_DeeType_GetCachedInstanceAttr) +            \
     defined(DEFINE_DeeType_GetCachedInstanceAttrLen) +         \
     defined(DEFINE_DeeType_BoundCachedAttr) +                  \
     defined(DEFINE_DeeType_BoundCachedAttrLen) +               \
     defined(DEFINE_DeeType_BoundCachedClassAttr) +             \
     defined(DEFINE_DeeType_BoundCachedClassAttrLen) +          \
     defined(DEFINE_DeeType_BoundCachedInstanceAttr) +          \
     defined(DEFINE_DeeType_BoundCachedInstanceAttrLen) +       \
     defined(DEFINE_DeeType_HasCachedAttr) +                    \
     defined(DEFINE_DeeType_HasCachedAttrLen) +                 \
     defined(DEFINE_DeeType_HasCachedClassAttr) +               \
     defined(DEFINE_DeeType_HasCachedClassAttrLen) +            \
     defined(DEFINE_DeeType_DelCachedAttr) +                    \
     defined(DEFINE_DeeType_DelCachedAttrLen) +                 \
     defined(DEFINE_DeeType_DelCachedClassAttr) +               \
     defined(DEFINE_DeeType_DelCachedClassAttrLen) +            \
     defined(DEFINE_DeeType_DelCachedInstanceAttr) +            \
     defined(DEFINE_DeeType_DelCachedInstanceAttrLen) +         \
     defined(DEFINE_DeeType_SetCachedAttr) +                    \
     defined(DEFINE_DeeType_SetCachedAttrLen) +                 \
     defined(DEFINE_DeeType_SetCachedClassAttr) +               \
     defined(DEFINE_DeeType_SetCachedClassAttrLen) +            \
     defined(DEFINE_DeeType_SetCachedInstanceAttr) +            \
     defined(DEFINE_DeeType_SetCachedInstanceAttrLen) +         \
     defined(DEFINE_DeeType_SetBasicCachedAttr) +               \
     defined(DEFINE_DeeType_SetBasicCachedAttrLen) +            \
     defined(DEFINE_DeeType_SetBasicCachedClassAttr) +          \
     defined(DEFINE_DeeType_SetBasicCachedClassAttrLen) +       \
     defined(DEFINE_DeeType_SetBasicCachedInstanceAttr) +       \
     defined(DEFINE_DeeType_SetBasicCachedInstanceAttrLen) +    \
     defined(DEFINE_DeeType_CallCachedAttr) +                   \
     defined(DEFINE_DeeType_CallCachedAttrLen) +                \
     defined(DEFINE_DeeType_CallCachedClassAttr) +              \
     defined(DEFINE_DeeType_CallCachedClassAttrLen) +           \
     defined(DEFINE_DeeType_CallCachedInstanceAttr) +           \
     defined(DEFINE_DeeType_CallCachedInstanceAttrLen) +        \
     defined(DEFINE_DeeType_CallCachedAttrKw) +                 \
     defined(DEFINE_DeeType_CallCachedAttrLenKw) +              \
     defined(DEFINE_DeeType_CallCachedClassAttrKw) +            \
     defined(DEFINE_DeeType_CallCachedClassAttrLenKw) +         \
     defined(DEFINE_DeeType_CallCachedInstanceAttrKw) +         \
     defined(DEFINE_DeeType_CallCachedInstanceAttrLenKw) +      \
     defined(DEFINE_DeeType_CallCachedAttrTuple) +              \
     defined(DEFINE_DeeType_CallCachedAttrLenTuple) +           \
     defined(DEFINE_DeeType_CallCachedClassAttrTuple) +         \
     defined(DEFINE_DeeType_CallCachedClassAttrLenTuple) +      \
     defined(DEFINE_DeeType_CallCachedInstanceAttrTuple) +      \
     defined(DEFINE_DeeType_CallCachedInstanceAttrLenTuple) +   \
     defined(DEFINE_DeeType_CallCachedAttrTupleKw) +            \
     defined(DEFINE_DeeType_CallCachedAttrLenTupleKw) +         \
     defined(DEFINE_DeeType_CallCachedClassAttrTupleKw) +       \
     defined(DEFINE_DeeType_CallCachedClassAttrLenTupleKw) +    \
     defined(DEFINE_DeeType_CallCachedInstanceAttrTupleKw) +    \
     defined(DEFINE_DeeType_CallCachedInstanceAttrLenTupleKw) + \
     defined(DEFINE_DeeType_VCallCachedAttrf) +                 \
     defined(DEFINE_DeeType_VCallCachedAttrLenf) +              \
     defined(DEFINE_DeeType_VCallCachedClassAttrf) +            \
     defined(DEFINE_DeeType_VCallCachedClassAttrLenf) +         \
     defined(DEFINE_DeeType_VCallCachedInstanceAttrf) +         \
     defined(DEFINE_DeeType_VCallCachedInstanceAttrLenf) +      \
     defined(DEFINE_DeeType_FindCachedAttr) +                   \
     defined(DEFINE_DeeType_FindCachedClassAttr)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */


DECL_BEGIN

#ifdef DEFINE_DeeType_GetCachedAttr
#define LOCAL_DeeType_AccessCachedAttr DeeType_GetCachedAttr
#define LOCAL_IS_GET
#elif defined(DEFINE_DeeType_GetCachedAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_GetCachedAttrLen
#define LOCAL_IS_GET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_GetCachedClassAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_GetCachedClassAttr
#define LOCAL_IS_GET
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_GetCachedClassAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_GetCachedClassAttrLen
#define LOCAL_IS_GET
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_GetCachedInstanceAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_GetCachedInstanceAttr
#define LOCAL_IS_GET
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_GetCachedInstanceAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_GetCachedInstanceAttrLen
#define LOCAL_IS_GET
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_BoundCachedAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_BoundCachedAttr
#define LOCAL_IS_BOUND
#elif defined(DEFINE_DeeType_BoundCachedAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_BoundCachedAttrLen
#define LOCAL_IS_BOUND
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_BoundCachedClassAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_BoundCachedClassAttr
#define LOCAL_IS_BOUND
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_BoundCachedClassAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_BoundCachedClassAttrLen
#define LOCAL_IS_BOUND
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_BoundCachedInstanceAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_BoundCachedInstanceAttr
#define LOCAL_IS_BOUND
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_BoundCachedInstanceAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_BoundCachedInstanceAttrLen
#define LOCAL_IS_BOUND
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_HasCachedAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_HasCachedAttr
#define LOCAL_IS_HAS
#elif defined(DEFINE_DeeType_HasCachedAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_HasCachedAttrLen
#define LOCAL_IS_HAS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_HasCachedClassAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_HasCachedClassAttr
#define LOCAL_IS_HAS
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_HasCachedClassAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_HasCachedClassAttrLen
#define LOCAL_IS_HAS
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_HasCachedInstanceAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_HasCachedInstanceAttr
#define LOCAL_IS_HAS
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_HasCachedInstanceAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_HasCachedInstanceAttrLen
#define LOCAL_IS_HAS
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_DelCachedAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_DelCachedAttr
#define LOCAL_IS_DEL
#elif defined(DEFINE_DeeType_DelCachedAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_DelCachedAttrLen
#define LOCAL_IS_DEL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_DelCachedClassAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_DelCachedClassAttr
#define LOCAL_IS_DEL
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_DelCachedClassAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_DelCachedClassAttrLen
#define LOCAL_IS_DEL
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_DelCachedInstanceAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_DelCachedInstanceAttr
#define LOCAL_IS_DEL
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_DelCachedInstanceAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_DelCachedInstanceAttrLen
#define LOCAL_IS_DEL
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetCachedAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetCachedAttr
#define LOCAL_IS_SET
#elif defined(DEFINE_DeeType_SetCachedAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetCachedAttrLen
#define LOCAL_IS_SET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetCachedClassAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetCachedClassAttr
#define LOCAL_IS_SET
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_SetCachedClassAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetCachedClassAttrLen
#define LOCAL_IS_SET
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetCachedInstanceAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetCachedInstanceAttr
#define LOCAL_IS_SET
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_SetCachedInstanceAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetCachedInstanceAttrLen
#define LOCAL_IS_SET
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetBasicCachedAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetBasicCachedAttr
#define LOCAL_IS_SET_BASIC
#elif defined(DEFINE_DeeType_SetBasicCachedAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetBasicCachedAttrLen
#define LOCAL_IS_SET_BASIC
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetBasicCachedClassAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetBasicCachedClassAttr
#define LOCAL_IS_SET_BASIC
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_SetBasicCachedClassAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetBasicCachedClassAttrLen
#define LOCAL_IS_SET_BASIC
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetBasicCachedInstanceAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetBasicCachedInstanceAttr
#define LOCAL_IS_SET_BASIC
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_SetBasicCachedInstanceAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetBasicCachedInstanceAttrLen
#define LOCAL_IS_SET_BASIC
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttr
#define LOCAL_IS_CALL
#elif defined(DEFINE_DeeType_CallCachedAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrLen
#define LOCAL_IS_CALL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedClassAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttr
#define LOCAL_IS_CALL
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_CallCachedClassAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrLen
#define LOCAL_IS_CALL
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedInstanceAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttr
#define LOCAL_IS_CALL
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrLen)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrLen
#define LOCAL_IS_CALL
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedAttrKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrKw
#define LOCAL_IS_CALL_KW
#elif defined(DEFINE_DeeType_CallCachedAttrLenKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrLenKw
#define LOCAL_IS_CALL_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedClassAttrKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrKw
#define LOCAL_IS_CALL_KW
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_CallCachedClassAttrLenKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrLenKw
#define LOCAL_IS_CALL_KW
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrKw
#define LOCAL_IS_CALL_KW
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrLenKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrLenKw
#define LOCAL_IS_CALL_KW
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedAttrTuple)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrTuple
#define LOCAL_IS_CALL_TUPLE
#elif defined(DEFINE_DeeType_CallCachedAttrLenTuple)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrLenTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedClassAttrTuple)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_CallCachedClassAttrLenTuple)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrLenTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrTuple)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrLenTuple)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrLenTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedAttrTupleKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#elif defined(DEFINE_DeeType_CallCachedAttrLenTupleKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrLenTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedClassAttrTupleKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_CallCachedClassAttrLenTupleKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrLenTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrTupleKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrLenTupleKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrLenTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_VCallCachedAttrf)
#define LOCAL_DeeType_AccessCachedAttr DeeType_VCallCachedAttrf
#define LOCAL_IS_VCALLF
#elif defined(DEFINE_DeeType_VCallCachedAttrLenf)
#define LOCAL_DeeType_AccessCachedAttr DeeType_VCallCachedAttrLenf
#define LOCAL_IS_VCALLF
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_VCallCachedClassAttrf)
#define LOCAL_DeeType_AccessCachedAttr DeeType_VCallCachedClassAttrf
#define LOCAL_IS_VCALLF
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_VCallCachedClassAttrLenf)
#define LOCAL_DeeType_AccessCachedAttr DeeType_VCallCachedClassAttrLenf
#define LOCAL_IS_VCALLF
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_VCallCachedInstanceAttrf)
#define LOCAL_DeeType_AccessCachedAttr DeeType_VCallCachedInstanceAttrf
#define LOCAL_IS_VCALLF
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_VCallCachedInstanceAttrLenf)
#define LOCAL_DeeType_AccessCachedAttr DeeType_VCallCachedInstanceAttrLenf
#define LOCAL_IS_VCALLF
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_FindCachedAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_FindCachedAttr
#define LOCAL_IS_FIND
#elif defined(DEFINE_DeeType_FindCachedClassAttr)
#define LOCAL_DeeType_AccessCachedAttr DeeType_FindCachedClassAttr
#define LOCAL_IS_FIND
#define LOCAL_IS_CLASS
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#if !defined(LOCAL_IS_CLASS) && !defined(LOCAL_IS_INSTANCE) && !defined(LOCAL_IS_HAS)
#define LOCAL_HAS_self
#endif /* !LOCAL_IS_CLASS && !LOCAL_IS_INSTANCE && !LOCAL_IS_HAS */

#ifdef LOCAL_IS_CLASS
#define LOCAL_tp_cache tp_class_cache
#else /* LOCAL_IS_CLASS */
#define LOCAL_tp_cache tp_cache
#endif /* !LOCAL_IS_CLASS */

#if (defined(LOCAL_IS_SET) || defined(LOCAL_IS_SET_BASIC) ||            \
     defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW) || \
     defined(LOCAL_IS_VCALLF))
#if defined(LOCAL_HAS_self) && defined(LOCAL_HAS_len)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3, 6))
#elif defined(LOCAL_HAS_self) || defined(LOCAL_HAS_len)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3, 5))
#else /* ... */
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 4))
#endif /* !... */
#elif defined(LOCAL_IS_FIND)
#ifdef LOCAL_IS_CLASS
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3))
#else /* LOCAL_IS_CLASS */
#define LOCAL_ATTR_NONNULL NONNULL((1, 3, 4))
#endif /* !LOCAL_IS_CLASS */
#else /* ... */
#ifdef LOCAL_HAS_self
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3))
#else /* LOCAL_HAS_self */
#define LOCAL_ATTR_NONNULL NONNULL((1, 2))
#endif /* !LOCAL_HAS_self */
#endif /* !... */



#if defined(LOCAL_IS_CALL)
#define LOCAL_HAS_argv
#elif defined(LOCAL_IS_CALL_KW)
#define LOCAL_HAS_argv
#define LOCAL_HAS_kw
#elif defined(LOCAL_IS_CALL_TUPLE)
#define LOCAL_HAS_args
#elif defined(LOCAL_IS_CALL_TUPLE_KW)
#define LOCAL_HAS_args
#define LOCAL_HAS_kw
#elif defined(LOCAL_IS_VCALLF)
#define LOCAL_HAS_format
#endif /* ... */

#ifdef LOCAL_HAS_kw
#define LOCAL_kw kw
#else /* LOCAL_HAS_kw */
#define LOCAL_kw NULL
#endif /* !LOCAL_HAS_kw */

#ifdef LOCAL_HAS_argv
#define LOCAL_argc argc
#define LOCAL_argv argv
#elif defined(LOCAL_HAS_args)
#define LOCAL_argc DeeTuple_SIZE(args)
#define LOCAL_argv DeeTuple_ELEM(args)
#endif /* !LOCAL_HAS_argv */

#if defined(LOCAL_HAS_args) && defined(LOCAL_kw)
#define LOCAL_invoke_object_thisarg(ob, thisarg) DeeObject_ThisCallTupleKw(ob, thisarg, args, LOCAL_kw)
#define LOCAL_invoke_object(ob)                  DeeObject_CallTupleKw(ob, args, LOCAL_kw)
#define LOCAL_invoke_attribute(desc, self, thisarg, attr) \
	DeeInstance_CallAttributeTupleKw(desc, self, thisarg, attr, args, LOCAL_kw)
#define LOCAL_invoke_instance_attribute(class_type, attr) \
	DeeClass_CallInstanceAttributeTupleKw(class_type, attr, args, LOCAL_kw)
#define LOCAL_unpack_one_for_getter(p_thisarg) \
	DeeArg_UnpackKw(LOCAL_argc, LOCAL_argv, LOCAL_kw, getter_kwlist, "o:get", p_thisarg)
#elif defined(LOCAL_HAS_args)
#define LOCAL_invoke_object_thisarg(ob, thisarg) DeeObject_ThisCallTuple(ob, thisarg, args)
#define LOCAL_invoke_object(ob)                  DeeObject_CallTuple(ob, args)
#define LOCAL_invoke_attribute(desc, self, thisarg, attr) \
	DeeInstance_CallAttributeTuple(desc, self, thisarg, attr, args)
#define LOCAL_invoke_instance_attribute(class_type, attr) \
	DeeClass_CallInstanceAttributeTuple(class_type, attr, args)
#define LOCAL_unpack_one_for_getter(p_thisarg) \
	DeeArg_Unpack(LOCAL_argc, LOCAL_argv, "o:get", p_thisarg)
#elif defined(LOCAL_argc) && defined(LOCAL_kw)
#define LOCAL_invoke_object_thisarg(ob, thisarg) DeeObject_ThisCallKw(ob, thisarg, LOCAL_argc, LOCAL_argv, LOCAL_kw)
#define LOCAL_invoke_object(ob)                  DeeObject_CallKw(ob, LOCAL_argc, LOCAL_argv, LOCAL_kw)
#define LOCAL_invoke_attribute(desc, self, thisarg, attr) \
	DeeInstance_CallAttributeKw(desc, self, thisarg, attr, LOCAL_argc, LOCAL_argv, LOCAL_kw)
#define LOCAL_invoke_instance_attribute(class_type, attr) \
	DeeClass_CallInstanceAttributeKw(class_type, attr, LOCAL_argc, LOCAL_argv, LOCAL_kw)
#define LOCAL_unpack_one_for_getter(p_thisarg) \
	DeeArg_UnpackKw(LOCAL_argc, LOCAL_argv, LOCAL_kw, getter_kwlist, "o:get", p_thisarg)
#elif defined(LOCAL_argc)
#define LOCAL_invoke_object_thisarg(ob, thisarg) DeeObject_ThisCall(ob, thisarg, LOCAL_argc, LOCAL_argv)
#define LOCAL_invoke_object(ob)                  DeeObject_Call(ob, LOCAL_argc, LOCAL_argv)
#define LOCAL_invoke_attribute(desc, self, thisarg, attr) \
	DeeInstance_CallAttribute(desc, self, thisarg, attr, LOCAL_argc, LOCAL_argv)
#define LOCAL_invoke_instance_attribute(class_type, attr) \
	DeeClass_CallInstanceAttribute(class_type, attr, LOCAL_argc, LOCAL_argv)
#define LOCAL_unpack_one_for_getter(p_thisarg) \
	DeeArg_Unpack(LOCAL_argc, LOCAL_argv, "o:get", p_thisarg)
#elif defined(LOCAL_HAS_format)
#define LOCAL_invoke_object_thisarg(ob, thisarg) DeeObject_VThisCallf(ob, thisarg, format, args)
#define LOCAL_invoke_object(ob)                  DeeObject_VCallf(ob, format, args)
#define LOCAL_invoke_attribute(desc, self, thisarg, attr) \
	DeeInstance_VCallAttributef(desc, self, thisarg, attr, format, args)
#define LOCAL_invoke_instance_attribute(class_type, attr) \
	DeeClass_VCallInstanceAttributef(class_type, attr, format, args)
#define LOCAL_unpack_one_for_getter(p_thisarg) \
	((*(p_thisarg) = Dee_VPackf(format, args)) != NULL ? 0 : -1)
#define LOCAL_unpack_one_for_getter_cleanup(thisarg) Dee_Decref(thisarg)
#endif /* ... */

#ifndef LOCAL_unpack_one_for_getter_cleanup
#define LOCAL_unpack_one_for_getter_cleanup(thisarg) (void)0
#endif /* !LOCAL_unpack_one_for_getter_cleanup */

#ifdef LOCAL_argc
#define LOCAL_invoke_dkwobjmethod_thisarg(func, thisarg) (*(func))(thisarg, LOCAL_argc, LOCAL_argv, LOCAL_kw)
#define LOCAL_invoke_dobjmethod_thisarg(func, thisarg)   (*(func))(thisarg, LOCAL_argc, LOCAL_argv)
#define LOCAL_invoke_dkwobjmethod(func, decl)            (*(func))(LOCAL_argv[0], LOCAL_argc - 1, LOCAL_argv + 1, LOCAL_kw)
#define LOCAL_invoke_dobjmethod(func, decl)              (*(func))(LOCAL_argv[0], LOCAL_argc - 1, LOCAL_argv + 1)
#define LOCAL_MUST_ASSERT_ARGC
#elif defined(LOCAL_HAS_format)
#define LOCAL_invoke_dkwobjmethod_thisarg(func, thisarg) \
	dkwobjmethod_vthiscallf(func, thisarg, format, args)
#ifndef DKWOBJMETHOD_VTHISCALLF_DEFINED
#define DKWOBJMETHOD_VTHISCALLF_DEFINED
PRIVATE NONNULL((1, 2, 3)) DREF DeeObject *DCALL
dkwobjmethod_vthiscallf(dkwobjmethod_t self,
                        DeeObject *thisarg,
                        char const *__restrict format,
                        va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = (*self)(thisarg,
	                 DeeTuple_SIZE(args_tuple),
	                 DeeTuple_ELEM(args_tuple),
	                 NULL);
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}
#endif /* !DKWOBJMETHOD_VTHISCALLF_DEFINED */

#define LOCAL_invoke_dobjmethod_thisarg(func, thisarg) \
	dobjmethod_vthiscallf(func, thisarg, format, args)
#ifndef DOBJMETHOD_VTHISCALLF_DEFINED
#define DOBJMETHOD_VTHISCALLF_DEFINED
PRIVATE NONNULL((1, 2, 3)) DREF DeeObject *DCALL
dobjmethod_vthiscallf(dobjmethod_t self,
                      DeeObject *thisarg,
                      char const *__restrict format,
                      va_list args) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	result = (*self)(thisarg,
	                 DeeTuple_SIZE(args_tuple),
	                 DeeTuple_ELEM(args_tuple));
	Dee_Decref(args_tuple);
	return result;
err:
	return NULL;
}
#endif /* !DOBJMETHOD_VTHISCALLF_DEFINED */

#ifdef LOCAL_HAS_len
#define LOCAL_invoke_dkwobjmethod(func, decl) \
	dkwobjmethod_vcallf_len(func, format, args, decl, attr, attrlen)
#define LOCAL_invoke_dobjmethod(func, decl) \
	dobjmethod_vcallf_len(func, format, args, decl, attr, attrlen)
#ifndef DKWOBJMETHOD_VCALLF_LEN_DEFINED
#define DKWOBJMETHOD_VCALLF_LEN_DEFINED
PRIVATE NONNULL((1, 2, 4, 5)) DREF DeeObject *DCALL
dkwobjmethod_vcallf_len(dkwobjmethod_t self,
                        char const *__restrict format,
                        va_list args,
                        DeeTypeObject *__restrict decl,
                        char const *__restrict attr, size_t attrlen) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	if unlikely(DeeTuple_SIZE(args_tuple) == 0)
		goto err_noargs;
	if unlikely(!(decl->tp_flags & TP_FABSTRACT)) {
		if (DeeObject_AssertType(DeeTuple_GET(args_tuple, 0), decl))
			goto err_args_tuple;
	}
	result = (*self)(DeeTuple_GET(args_tuple, 0),
	                 DeeTuple_SIZE(args_tuple) - 1,
	                 DeeTuple_ELEM(args_tuple) + 1,
	                 NULL);
	Dee_Decref(args_tuple);
	return result;
err_noargs:
	err_classmethod_requires_at_least_1_argument_len(decl, attr, attrlen);
err_args_tuple:
	Dee_Decref(args_tuple);
err:
	return NULL;
}
#endif /* !DKWOBJMETHOD_VCALLF_LEN_DEFINED */
#ifndef DOBJMETHOD_VCALLF_LEN_DEFINED
#define DOBJMETHOD_VCALLF_LEN_DEFINED
PRIVATE NONNULL((1, 2, 4, 5)) DREF DeeObject *DCALL
dobjmethod_vcallf_len(dobjmethod_t self,
                      char const *__restrict format,
                      va_list args,
                      DeeTypeObject *__restrict decl,
                      char const *__restrict attr, size_t attrlen) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	if unlikely(DeeTuple_SIZE(args_tuple) == 0)
		goto err_noargs;
	if unlikely(!(decl->tp_flags & TP_FABSTRACT)) {
		if (DeeObject_AssertType(DeeTuple_GET(args_tuple, 0), decl))
			goto err_args_tuple;
	}
	result = (*self)(DeeTuple_GET(args_tuple, 0),
	                 DeeTuple_SIZE(args_tuple) - 1,
	                 DeeTuple_ELEM(args_tuple) + 1);
	Dee_Decref(args_tuple);
	return result;
err_noargs:
	err_classmethod_requires_at_least_1_argument_len(decl, attr, attrlen);
err_args_tuple:
	Dee_Decref(args_tuple);
err:
	return NULL;
}
#endif /* !DOBJMETHOD_VCALLF_LEN_DEFINED */
#else /* LOCAL_HAS_len */
#define LOCAL_invoke_dkwobjmethod(func, decl) \
	dkwobjmethod_vcallf(func, format, args, decl, attr)
#define LOCAL_invoke_dobjmethod(func, decl) \
	dobjmethod_vcallf(func, format, args, decl, attr)
#ifndef DKWOBJMETHOD_VCALLF_DEFINED
#define DKWOBJMETHOD_VCALLF_DEFINED
PRIVATE NONNULL((1, 2, 4, 5)) DREF DeeObject *DCALL
dkwobjmethod_vcallf(dkwobjmethod_t self,
                    char const *__restrict format,
                    va_list args,
                    DeeTypeObject *__restrict decl,
                    char const *__restrict attr) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	if unlikely(DeeTuple_SIZE(args_tuple) == 0)
		goto err_noargs;
	if unlikely(!(decl->tp_flags & TP_FABSTRACT)) {
		if (DeeObject_AssertType(DeeTuple_GET(args_tuple, 0), decl))
			goto err_args_tuple;
	}
	result = (*self)(DeeTuple_GET(args_tuple, 0),
	                 DeeTuple_SIZE(args_tuple) - 1,
	                 DeeTuple_ELEM(args_tuple) + 1,
	                 NULL);
	Dee_Decref(args_tuple);
	return result;
err_noargs:
	err_classmethod_requires_at_least_1_argument(decl, attr);
err_args_tuple:
	Dee_Decref(args_tuple);
err:
	return NULL;
}
#endif /* !DKWOBJMETHOD_VCALLF_DEFINED */
#ifndef DOBJMETHOD_VCALLF_DEFINED
#define DOBJMETHOD_VCALLF_DEFINED
PRIVATE NONNULL((1, 2, 4, 5)) DREF DeeObject *DCALL
dobjmethod_vcallf(dobjmethod_t self,
                  char const *__restrict format,
                  va_list args,
                  DeeTypeObject *__restrict decl,
                  char const *__restrict attr) {
	DREF DeeObject *result, *args_tuple;
	args_tuple = DeeTuple_VNewf(format, args);
	if unlikely(!args_tuple)
		goto err;
	if unlikely(DeeTuple_SIZE(args_tuple) == 0)
		goto err_noargs;
	if unlikely(!(decl->tp_flags & TP_FABSTRACT)) {
		if (DeeObject_AssertType(DeeTuple_GET(args_tuple, 0), decl))
			goto err_args_tuple;
	}
	result = (*self)(DeeTuple_GET(args_tuple, 0),
	                 DeeTuple_SIZE(args_tuple) - 1,
	                 DeeTuple_ELEM(args_tuple) + 1);
	Dee_Decref(args_tuple);
	return result;
err_noargs:
	err_classmethod_requires_at_least_1_argument(decl, attr);
err_args_tuple:
	Dee_Decref(args_tuple);
err:
	return NULL;
}
#endif /* !DOBJMETHOD_VCALLF_DEFINED */
#endif /* !LOCAL_HAS_len */

#endif /* ... */

#ifdef LOCAL_HAS_kw
#define LOCAL_assert_kw_empty(decl)                 \
	do {                                            \
		if (kw) {                                   \
			if (DeeKwds_Check(kw)) {                \
				if (DeeKwds_SIZE(kw) != 0) {        \
					tp_self = (decl);               \
					goto err_no_keywords;           \
				}                                   \
			} else {                                \
				size_t temp = DeeObject_Size(kw);   \
				if (temp != 0) {                    \
					if unlikely(temp == (size_t)-1) \
						goto err;                   \
					tp_self = (decl);               \
					goto err_no_keywords;           \
				}                                   \
			}                                       \
		}                                           \
	}	__WHILE0
#else /* LOCAL_HAS_kw */
#define LOCAL_assert_kw_empty_IS_NOOP
#define LOCAL_assert_kw_empty(decl) (void)0
#endif /* !LOCAL_HAS_kw */



#ifdef LOCAL_IS_GET
/* Lookup an attribute from cache.
 * @return: * :        The attribute value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTERN WUNUSED LOCAL_ATTR_NONNULL DREF DeeObject *DCALL
#elif defined(LOCAL_IS_BOUND)
/* @return: 1 : Attribute is bound.
 * @return: 0 : Attribute isn't bound.
 * @return: -1: An error occurred.
 * @return: -2: The attribute doesn't exist. */
INTERN WUNUSED LOCAL_ATTR_NONNULL int DCALL
#elif defined(LOCAL_IS_HAS)
/* @return: true : The attribute exists.
 * @return: false: The attribute doesn't exist. */
INTERN WUNUSED LOCAL_ATTR_NONNULL bool DCALL
#elif defined(LOCAL_IS_DEL)
/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the delete-operator on the attribute.
 * @return: -1: An error occurred. */
INTERN WUNUSED LOCAL_ATTR_NONNULL int DCALL
#elif defined(LOCAL_IS_SET)
/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the set-operator on the attribute.
 * @return: -1: An error occurred. */
INTERN WUNUSED LOCAL_ATTR_NONNULL int DCALL
#elif defined(LOCAL_IS_SET_BASIC)
/* @return:  2: The attribute is non-basic.
 * @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the set-operator on the attribute.
 * @return: -1: An error occurred. */
INTERN WUNUSED LOCAL_ATTR_NONNULL int DCALL
#elif (defined(LOCAL_IS_CALL) || defined(LOCAL_IS_CALL_KW) ||             \
       defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW) || \
       defined(LOCAL_IS_VCALLF))
/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTERN WUNUSED LOCAL_ATTR_NONNULL DREF DeeObject *DCALL
#elif defined(LOCAL_IS_FIND)
INTERN WUNUSED LOCAL_ATTR_NONNULL int DCALL
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
LOCAL_DeeType_AccessCachedAttr(DeeTypeObject *tp_self,
#ifdef LOCAL_IS_FIND
#ifndef LOCAL_IS_CLASS
                               DeeObject *instance,
#endif /* !LOCAL_IS_CLASS */
                               struct attribute_info *__restrict result,
                               struct attribute_lookup_rules const *__restrict rules
#else /* LOCAL_IS_FIND */
#ifdef LOCAL_HAS_self
                               DeeObject *self,
#endif /* LOCAL_HAS_self */
                               char const *__restrict attr,
#ifdef LOCAL_HAS_len
                               size_t attrlen,
#endif /* LOCAL_HAS_len */
                               dhash_t hash
#if defined(LOCAL_IS_SET) || defined(LOCAL_IS_SET_BASIC)
                               , DeeObject *value
#elif defined(LOCAL_IS_CALL) || defined(LOCAL_IS_CALL_KW)
                               , size_t argc, DeeObject *const *argv
#elif defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW)
                               , DeeObject *args
#elif defined(LOCAL_IS_VCALLF)
                               , char const *__restrict format, va_list args
#endif /* ... */
#ifdef LOCAL_HAS_kw
                               , DeeObject *kw
#endif /* LOCAL_HAS_kw */
#endif /* !LOCAL_IS_FIND */
                               ) {
#if defined(LOCAL_IS_CLASS) && !defined(LOCAL_IS_HAS)
#define LOCAL_HAS_self
#define LOCAL_self ((DeeObject *)tp_self)
#elif defined(LOCAL_HAS_self)
#define LOCAL_self self
#endif /* LOCAL_IS_CLASS */

#ifdef LOCAL_IS_FIND
#define LOCAL_attr rules->alr_name
#define LOCAL_hash rules->alr_hash
#else /* LOCAL_IS_FIND */
#define LOCAL_attr attr
#define LOCAL_hash hash
#endif /* !LOCAL_IS_FIND */

#ifdef LOCAL_HAS_len
#define LOCAL_Dee_membercache_slot_matches(item)             streq_len(item->mcs_name, LOCAL_attr, attrlen)
#define LOCAL_err_cant_access_attribute(tp, access)          err_cant_access_attribute_len(tp, LOCAL_attr, attrlen, access)
#define LOCAL_err_classmember_requires_1_argument()          err_classmember_requires_1_argument_len(tp_self, LOCAL_attr, attrlen)
#define LOCAL_err_classproperty_requires_1_argument()        err_classproperty_requires_1_argument_len(tp_self, LOCAL_attr, attrlen)
#define LOCAL_err_classmethod_requires_at_least_1_argument() err_classmethod_requires_at_least_1_argument_len(tp_self, LOCAL_attr, attrlen)
#else /* LOCAL_HAS_len */
#define LOCAL_Dee_membercache_slot_matches(item)             streq(item->mcs_name, LOCAL_attr)
#define LOCAL_err_cant_access_attribute(tp, access)          err_cant_access_attribute(tp, LOCAL_attr, access)
#define LOCAL_err_classmember_requires_1_argument()          err_classmember_requires_1_argument(tp_self, LOCAL_attr)
#define LOCAL_err_classproperty_requires_1_argument()        err_classproperty_requires_1_argument(tp_self, LOCAL_attr)
#define LOCAL_err_classmethod_requires_at_least_1_argument() err_classmethod_requires_at_least_1_argument(tp_self, LOCAL_attr)
#endif /* !LOCAL_HAS_len */
#ifdef LOCAL_IS_CLASS
#define LOCAL_DeeInstance_DESC(desc) class_desc_as_instance(desc)
#else /* LOCAL_IS_CLASS */
#define LOCAL_DeeInstance_DESC(desc) DeeInstance_DESC(desc, LOCAL_self)
#endif /* !LOCAL_IS_CLASS */
#if (defined(LOCAL_IS_GET) || defined(LOCAL_IS_HAS) || defined(LOCAL_IS_BOUND) || \
     defined(LOCAL_IS_CALL) || defined(LOCAL_IS_CALL_KW) ||                       \
     defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW) ||           \
     defined(LOCAL_IS_VCALLF))
#define LOCAL_ATTR_ACCESS ATTR_ACCESS_GET
#elif defined(LOCAL_IS_DEL)
#define LOCAL_ATTR_ACCESS ATTR_ACCESS_DEL
#elif defined(LOCAL_IS_SET) || defined(LOCAL_IS_SET_BASIC)
#define LOCAL_ATTR_ACCESS ATTR_ACCESS_SET
#endif /* !LOCAL_IS_CLASS */
#if (defined(LOCAL_HAS_self) &&                              \
     (defined(LOCAL_IS_CALL) || defined(LOCAL_IS_CALL_KW) || \
      defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW) || defined(LOCAL_IS_VCALLF)))
	DREF DeeObject *callback;
#endif /* ... */

	DREF struct Dee_membercache_table *table;
	dhash_t i, perturb;
	if unlikely(!Dee_membercache_acquiretable(&tp_self->LOCAL_tp_cache, &table))
		goto cache_miss;
	perturb = i = Dee_membercache_table_hashst(table, LOCAL_hash);
	for (;; Dee_membercache_table_hashnx(i, perturb)) {
#ifdef LOCAL_IS_FIND
		DREF DeeTypeObject *attr_type;
		char const *attr_doc;
		uint16_t attr_perm;
#endif /* LOCAL_IS_FIND */
		struct Dee_membercache_slot *item;
		uint16_t type;
		item = Dee_membercache_table_hashit(table, i);
		type = atomic_read(&item->mcs_type);
		if (type == MEMBERCACHE_UNUSED)
			break;
		if (item->mcs_hash != LOCAL_hash)
			continue;
		if unlikely(type == MEMBERCACHE_UNINITIALIZED)
			continue; /* Don't dereference uninitialized items! */
		if (!LOCAL_Dee_membercache_slot_matches(item))
			continue;

#ifdef LOCAL_IS_FIND
		if (rules->alr_decl && (DeeObject *)item->mcs_decl != result->a_decl)
			break; /* Attribute isn't declared by the requested declarator. */
#endif /* LOCAL_IS_FIND */

		/* Referenced attribute found! */
#ifdef LOCAL_IS_HAS
		Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
		return true;
#else /* LOCAL_IS_HAS */
		switch (type) {

#if (defined(LOCAL_IS_GET) || defined(LOCAL_IS_CALL) || defined(LOCAL_IS_CALL_KW) || \
     defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW) || defined(LOCAL_IS_VCALLF))
		/* Get or Call... */

		case MEMBERCACHE_METHOD: {
			dobjmethod_t func = item->mcs_method.m_func;
#ifdef LOCAL_IS_GET
#ifdef LOCAL_HAS_self
			if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
				Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
				return DeeKwObjMethod_New((dkwobjmethod_t)func, LOCAL_self);
			}
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			return DeeObjMethod_New(func, LOCAL_self);
#else /* LOCAL_HAS_self */
			DeeTypeObject *decl = item->mcs_decl;
			if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
				Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
				return DeeKwClsMethod_New(decl, (dkwobjmethod_t)func);
			}
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			return DeeClsMethod_New(decl, func);
#endif /* !LOCAL_HAS_self */
#else /* LOCAL_IS_GET */
#ifdef LOCAL_HAS_self
#ifndef LOCAL_assert_kw_empty_IS_NOOP
			DeeTypeObject *decl;
#endif /* !LOCAL_assert_kw_empty_IS_NOOP */
			if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
				Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
				return LOCAL_invoke_dkwobjmethod_thisarg((dkwobjmethod_t)func, LOCAL_self);
			}
#ifndef LOCAL_assert_kw_empty_IS_NOOP
			decl = item->mcs_decl;
#endif /* !LOCAL_assert_kw_empty_IS_NOOP */
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			LOCAL_assert_kw_empty(decl);
			return LOCAL_invoke_dobjmethod_thisarg(func, LOCAL_self);
#else /* LOCAL_HAS_self */
			DeeTypeObject *decl = item->mcs_decl;
#ifdef LOCAL_MUST_ASSERT_ARGC
			if unlikely(LOCAL_argc == 0) {
				Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
				LOCAL_err_classmethod_requires_at_least_1_argument();
				goto err;
#define NEED_err
			}
#endif /* LOCAL_MUST_ASSERT_ARGC */
			if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
				Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
#ifdef LOCAL_MUST_ASSERT_ARGC
				if unlikely(!(decl->tp_flags & TP_FABSTRACT)) {
					if (DeeObject_AssertType(LOCAL_argv[0], decl))
						goto err;
#define NEED_err
				}
#endif /* LOCAL_MUST_ASSERT_ARGC */
				return LOCAL_invoke_dkwobjmethod((dkwobjmethod_t)func, decl);
			}
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
#ifdef LOCAL_MUST_ASSERT_ARGC
			if unlikely(!(decl->tp_flags & TP_FABSTRACT)) {
				if (DeeObject_AssertType(LOCAL_argv[0], decl))
					goto err;
#define NEED_err
			}
#endif /* LOCAL_MUST_ASSERT_ARGC */
			LOCAL_assert_kw_empty(decl);
			return LOCAL_invoke_dobjmethod(func, decl);
#endif /* !LOCAL_HAS_self */
#endif /* !LOCAL_IS_GET */
		}	break;

		case MEMBERCACHE_GETSET: {
#ifdef LOCAL_IS_GET
			dgetmethod_t get = item->mcs_getset.gs_get;
#ifndef LOCAL_HAS_self
			ddelmethod_t del = item->mcs_getset.gs_del;
			dsetmethod_t set = item->mcs_getset.gs_set;
			DeeTypeObject *decl = item->mcs_decl;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			return DeeClsProperty_New(decl, get, del, set);
#else /* !LOCAL_HAS_self */
			if likely(get) {
				Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
				return (*get)(LOCAL_self);
			}
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			LOCAL_err_cant_access_attribute(tp_self, LOCAL_ATTR_ACCESS);
			goto err;
#define NEED_err
#endif /* LOCAL_HAS_self */
#else /* LOCAL_IS_GET */
			dgetmethod_t getter;
			getter = item->mcs_getset.gs_get;
			if likely(getter) {
#ifdef LOCAL_HAS_self
				DREF DeeObject *result;
				Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
				callback = (*getter)(LOCAL_self);
check_and_invoke_callback:
				if unlikely(!callback)
					goto err;
#define NEED_err
				result = LOCAL_invoke_object(callback);
				Dee_Decref(callback);
				return result;
#else /* LOCAL_HAS_self */
				DREF DeeObject *result;
				/*maybe:DREF*/ DeeObject *thisarg;
				DeeTypeObject *decl = item->mcs_decl;
				Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
				if unlikely(LOCAL_unpack_one_for_getter(&thisarg))
					goto err;
				if unlikely(!(decl->tp_flags & TP_FABSTRACT) &&
				            DeeObject_AssertType(thisarg, decl)) {
					result = NULL;
				} else {
					result = (*getter)(thisarg);
				}
				LOCAL_unpack_one_for_getter_cleanup(thisarg);
				return result;
#endif /* !LOCAL_HAS_self */
			}
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			LOCAL_err_cant_access_attribute(tp_self, LOCAL_ATTR_ACCESS);
			goto err;
#define NEED_err
#endif /* !LOCAL_IS_GET */
		}	break;

		case MEMBERCACHE_MEMBER: {
#ifdef LOCAL_IS_GET
#ifdef LOCAL_HAS_self
			struct buffer {
				uint8_t dat[offsetof(struct type_member, m_doc)];
			} buf;
			buf = *(struct buffer *)&item->mcs_member;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			return type_member_get((struct type_member const *)&buf, LOCAL_self);
#else /* LOCAL_HAS_self */
			struct type_member member;
			DeeTypeObject *decl;
			member = item->mcs_member;
			decl   = item->mcs_decl;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			return DeeClsMember_New(decl, &member);
#endif /* !LOCAL_HAS_self */
#else /* LOCAL_IS_GET */
#ifdef LOCAL_HAS_self
			struct buffer {
				uint8_t dat[offsetof(struct type_member, m_doc)];
			} buf;
			buf = *(struct buffer *)&item->mcs_member;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			callback = type_member_get((struct type_member const *)&buf, LOCAL_self);
			goto check_and_invoke_callback;
#else /* LOCAL_HAS_self */
			struct buffer {
				uint8_t dat[offsetof(struct type_member, m_doc)];
			} buf;
			DREF DeeObject *result;
			/*maybe:DREF*/ DeeObject *thisarg;
			DeeTypeObject *decl;
			buf  = *(struct buffer *)&item->mcs_member;
			decl = item->mcs_decl;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			if unlikely(LOCAL_unpack_one_for_getter(&thisarg))
				goto err;
#define NEED_err
			if unlikely(!(decl->tp_flags & TP_FABSTRACT) &&
			            DeeObject_AssertType(LOCAL_argv[0], decl)) {
				result = NULL;
			} else {
				result = type_member_get((struct type_member const *)&buf, thisarg);
			}
			LOCAL_unpack_one_for_getter_cleanup(thisarg);
			return result;
#endif /* !LOCAL_HAS_self */
#endif /* !LOCAL_IS_GET */
		}	break;

		case MEMBERCACHE_ATTRIB: {
#ifdef LOCAL_IS_GET
			struct class_attribute *catt = item->mcs_attrib.a_attr;
#ifdef LOCAL_HAS_self
			struct class_desc *desc = item->mcs_attrib.a_desc;
#else /* LOCAL_HAS_self */
			DeeTypeObject *decl = item->mcs_decl;
#endif /* !LOCAL_HAS_self */
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
#ifdef LOCAL_HAS_self
			return DeeInstance_GetAttribute(desc,
			                                LOCAL_DeeInstance_DESC(desc),
			                                LOCAL_self, catt);
#else /* LOCAL_HAS_self */
			return DeeClass_GetInstanceAttribute(decl, catt);
#endif /* !LOCAL_HAS_self */
#else /* LOCAL_IS_GET */
#ifdef LOCAL_HAS_self
			struct class_attribute *catt;
			struct class_desc *desc;
			catt = item->mcs_attrib.a_attr;
			desc = item->mcs_attrib.a_desc;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			return LOCAL_invoke_attribute(desc, LOCAL_DeeInstance_DESC(desc),
			                              LOCAL_self, catt);
#else /* LOCAL_HAS_self */
			struct class_attribute *catt;
			DeeTypeObject *decl;
			catt = item->mcs_attrib.a_attr;
			decl = item->mcs_decl;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			return LOCAL_invoke_instance_attribute(decl, catt);
#endif /* !LOCAL_HAS_self */
#endif /* !LOCAL_IS_GET */
		}	break;

#ifdef LOCAL_IS_CLASS
		case MEMBERCACHE_INSTANCE_METHOD: {
			dobjmethod_t func;
			DeeTypeObject *decl;
			func = item->mcs_method.m_func;
			decl = item->mcs_decl;
#if !defined(LOCAL_IS_GET) && defined(LOCAL_MUST_ASSERT_ARGC)
			if unlikely(LOCAL_argc == 0) {
				Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
				LOCAL_err_classmethod_requires_at_least_1_argument();
				goto err;
#define NEED_err
			}
#endif /* !LOCAL_IS_GET && LOCAL_MUST_ASSERT_ARGC */
			if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
				Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
#ifdef LOCAL_IS_GET
				return DeeKwClsMethod_New(decl, (dkwobjmethod_t)func);
#else /* LOCAL_IS_GET */
#ifdef LOCAL_MUST_ASSERT_ARGC
				if unlikely(!(decl->tp_flags & TP_FABSTRACT)) {
					if (DeeObject_AssertType(LOCAL_argv[0], decl))
						goto err;
#define NEED_err
				}
#endif /* LOCAL_MUST_ASSERT_ARGC */
				return LOCAL_invoke_dkwobjmethod((dkwobjmethod_t)func, decl);
#endif /* !LOCAL_IS_GET */
			}
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
#ifdef LOCAL_IS_GET
			return DeeClsMethod_New(decl, func);
#else /* LOCAL_IS_GET */
#ifdef LOCAL_MUST_ASSERT_ARGC
			if unlikely(!(decl->tp_flags & TP_FABSTRACT)) {
				if (DeeObject_AssertType(LOCAL_argv[0], decl))
					goto err;
#define NEED_err
			}
#endif /* LOCAL_MUST_ASSERT_ARGC */
			LOCAL_assert_kw_empty(decl);
			return LOCAL_invoke_dobjmethod(func, decl);
#endif /* !LOCAL_IS_GET */
		}	break;

		case MEMBERCACHE_INSTANCE_GETSET: {
#ifdef LOCAL_IS_GET
			dgetmethod_t get;
			ddelmethod_t del;
			dsetmethod_t set;
			DeeTypeObject *decl;
			get  = item->mcs_getset.gs_get;
			del  = item->mcs_getset.gs_del;
			set  = item->mcs_getset.gs_set;
			decl = item->mcs_decl;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			return DeeClsProperty_New(decl, get, del, set);
#else /* LOCAL_IS_GET */
			dgetmethod_t getter;
			getter = item->mcs_getset.gs_get;
			if likely(getter) {
				DREF DeeObject *result;
				/*maybe:DREF*/ DeeObject *thisarg;
				DeeTypeObject *decl = item->mcs_decl;
				Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
				if unlikely(LOCAL_unpack_one_for_getter(&thisarg))
					goto err;
				if unlikely(!(decl->tp_flags & TP_FABSTRACT) &&
				            DeeObject_AssertType(thisarg, decl)) {
					result = NULL;
				} else {
					result = (*getter)(thisarg);
				}
				LOCAL_unpack_one_for_getter_cleanup(thisarg);
				return result;
			}
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			LOCAL_err_cant_access_attribute(tp_self, LOCAL_ATTR_ACCESS);
			goto err;
#define NEED_err
#endif /* !LOCAL_IS_GET */
		}	break;

		case MEMBERCACHE_INSTANCE_MEMBER: {
#ifdef LOCAL_IS_GET
			struct type_member member;
			DeeTypeObject *decl;
			member = item->mcs_member;
			decl   = item->mcs_decl;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			return DeeClsMember_New(decl, &member);
#else /* LOCAL_IS_GET */
			struct buffer {
				uint8_t dat[offsetof(struct type_member, m_doc)];
			} buf;
			DREF DeeObject *result;
			/*maybe:DREF*/ DeeObject *thisarg;
			DeeTypeObject *decl;
			buf  = *(struct buffer *)&item->mcs_member;
			decl = item->mcs_decl;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			if unlikely(LOCAL_unpack_one_for_getter(&thisarg))
				goto err;
#define NEED_err
			if unlikely(!(decl->tp_flags & TP_FABSTRACT) &&
			            DeeObject_AssertType(thisarg, decl)) {
				result = NULL;
			} else {
				result = type_member_get((struct type_member const *)&buf, thisarg);
			}
			LOCAL_unpack_one_for_getter_cleanup(thisarg);
			return result;
#endif /* !LOCAL_IS_GET */
		}	break;

		case MEMBERCACHE_INSTANCE_ATTRIB: {
#ifdef LOCAL_IS_GET
			struct class_attribute *catt;
			DeeTypeObject *decl;
			catt = item->mcs_attrib.a_attr;
			decl = item->mcs_decl;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			return DeeClass_GetInstanceAttribute(decl, catt);
#else /* LOCAL_IS_GET */
			struct class_attribute *catt;
			DeeTypeObject *decl;
			catt = item->mcs_attrib.a_attr;
			decl = item->mcs_decl;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			return LOCAL_invoke_instance_attribute(decl, catt);
#endif /* !LOCAL_IS_GET */
		}	break;
#endif /* LOCAL_IS_CLASS */

#elif defined(LOCAL_IS_BOUND) || defined(LOCAL_IS_DEL) || defined(LOCAL_IS_SET) || defined(LOCAL_IS_SET_BASIC)
		/* Bound, Delete, or Set */

#ifndef LOCAL_IS_SET_BASIC
		case MEMBERCACHE_METHOD:
#ifdef LOCAL_IS_CLASS
		case MEMBERCACHE_INSTANCE_METHOD:
		case MEMBERCACHE_INSTANCE_GETSET:
		case MEMBERCACHE_INSTANCE_MEMBER:
#endif /* LOCAL_IS_CLASS */
#ifndef LOCAL_HAS_self
		case MEMBERCACHE_GETSET:
		case MEMBERCACHE_MEMBER:
#endif /* !LOCAL_HAS_self */
		{
#ifdef LOCAL_IS_BOUND
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			return 1;
#elif defined(LOCAL_IS_DEL) || defined(LOCAL_IS_SET) || defined(LOCAL_IS_SET_BASIC)
			DeeTypeObject *decl = item->mcs_decl;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			return LOCAL_err_cant_access_attribute(decl, LOCAL_ATTR_ACCESS);
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
		}	break;

#ifdef LOCAL_HAS_self
		case MEMBERCACHE_GETSET: {
#ifdef LOCAL_IS_BOUND
			dboundmethod_t bound;
			dgetmethod_t getter;
			DREF DeeObject *temp;
			bound  = item->mcs_getset.gs_bound;
			getter = item->mcs_getset.gs_get;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			if (bound)
				return (*bound)(LOCAL_self);
			if unlikely(!getter)
				return 0;
			temp = (*getter)(LOCAL_self);
			if unlikely(!temp) {
				if (CATCH_ATTRIBUTE_ERROR())
					return -3;
				if (DeeError_Catch(&DeeError_UnboundAttribute))
					return 0;
				goto err;
#define NEED_err
			}
			Dee_Decref(temp);
			return 1;
#elif defined(LOCAL_IS_DEL)
			ddelmethod_t del;
			DeeTypeObject *decl;
			del = item->mcs_getset.gs_del;
			if likely(del) {
				Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
				return (*del)(LOCAL_self);
			}
			decl = item->mcs_decl;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			LOCAL_err_cant_access_attribute(decl, LOCAL_ATTR_ACCESS);
			goto err;
#define NEED_err
#elif defined(LOCAL_IS_SET) || defined(LOCAL_IS_SET_BASIC)
			dsetmethod_t set;
			DeeTypeObject *decl;
			set = item->mcs_getset.gs_set;
			if likely(set) {
				Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
				return (*set)(LOCAL_self, value);
			}
			decl = item->mcs_decl;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			LOCAL_err_cant_access_attribute(decl, LOCAL_ATTR_ACCESS);
			goto err;
#define NEED_err
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
		}	break;
#endif /* LOCAL_HAS_self */
#endif /* !LOCAL_IS_SET_BASIC */

#ifdef LOCAL_HAS_self
		case MEMBERCACHE_MEMBER: {
			struct buffer {
				uint8_t dat[offsetof(struct type_member, m_doc)];
			} buf;
			buf = *(struct buffer *)&item->mcs_member;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
#ifdef LOCAL_IS_BOUND
			return type_member_bound((struct type_member const *)&buf, LOCAL_self);
#elif defined(LOCAL_IS_DEL)
			return type_member_del((struct type_member const *)&buf, LOCAL_self);
#elif defined(LOCAL_IS_SET) || defined(LOCAL_IS_SET_BASIC)
			return type_member_set((struct type_member const *)&buf, LOCAL_self, value);
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
		}	break;
#endif /* LOCAL_HAS_self */

		case MEMBERCACHE_ATTRIB: {
#ifdef LOCAL_IS_INSTANCE
			struct class_attribute *catt;
			DeeTypeObject *decl;
			catt = item->mcs_attrib.a_attr;
			decl = item->mcs_decl;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
#ifdef LOCAL_IS_BOUND
			return DeeClass_BoundInstanceAttribute(decl, catt);
#elif defined(LOCAL_IS_DEL)
			return DeeClass_DelInstanceAttribute(decl, catt);
#elif defined(LOCAL_IS_SET) || defined(LOCAL_IS_SET_BASIC)
			return DeeClass_SetInstanceAttribute(decl, catt, value);
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
#else /* LOCAL_IS_INSTANCE */
			struct class_attribute *catt;
			struct class_desc *desc;
			catt = item->mcs_attrib.a_attr;
			desc = item->mcs_attrib.a_desc;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
#ifdef LOCAL_IS_BOUND
			return DeeInstance_BoundAttribute(desc, LOCAL_DeeInstance_DESC(desc), LOCAL_self, catt);
#elif defined(LOCAL_IS_DEL)
			return DeeInstance_DelAttribute(desc, LOCAL_DeeInstance_DESC(desc), LOCAL_self, catt);
#elif defined(LOCAL_IS_SET) || defined(LOCAL_IS_SET_BASIC)
			return DeeInstance_SetAttribute(desc, LOCAL_DeeInstance_DESC(desc), LOCAL_self, catt, value);
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
#endif /* !LOCAL_IS_INSTANCE */
		}	break;

#ifdef LOCAL_IS_CLASS
		case MEMBERCACHE_INSTANCE_ATTRIB: {
			struct class_attribute *catt;
			DeeTypeObject *decl;
			catt = item->mcs_attrib.a_attr;
			decl = item->mcs_decl;
			Dee_membercache_releasetable(&tp_self->tp_class_cache, table);
#ifdef LOCAL_IS_BOUND
			return DeeClass_BoundInstanceAttribute(decl, catt);
#elif defined(LOCAL_IS_DEL)
			return DeeClass_DelInstanceAttribute(decl, catt);
#elif defined(LOCAL_IS_SET) || defined(LOCAL_IS_SET_BASIC)
			return DeeClass_SetInstanceAttribute(decl, catt, value);
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
		}	break;
#endif /* LOCAL_IS_CLASS */
#elif defined(LOCAL_IS_FIND)

#ifdef LOCAL_IS_CLASS
#define LOCAL_ATTR_xMEMBER ATTR_CMEMBER
#else /* LOCAL_IS_CLASS */
#define LOCAL_ATTR_xMEMBER ATTR_IMEMBER
#endif /* !LOCAL_IS_CLASS */

		case MEMBERCACHE_METHOD:
			attr_perm = LOCAL_ATTR_xMEMBER | ATTR_PERMGET | ATTR_PERMCALL;
			attr_doc  = item->mcs_method.m_doc;
			attr_type = &DeeObjMethod_Type;
			if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS)
				attr_type = &DeeKwObjMethod_Type;
			Dee_Incref(attr_type);
			break;

		case MEMBERCACHE_GETSET:
			attr_perm = LOCAL_ATTR_xMEMBER | ATTR_PROPERTY;
			attr_doc  = item->mcs_getset.gs_doc;
			attr_type = NULL;
			if (item->mcs_getset.gs_get)
				attr_perm |= ATTR_PERMGET;
			if (item->mcs_getset.gs_del)
				attr_perm |= ATTR_PERMDEL;
			if (item->mcs_getset.gs_set)
				attr_perm |= ATTR_PERMSET;
			break;

		case MEMBERCACHE_MEMBER:
			attr_perm = LOCAL_ATTR_xMEMBER | ATTR_PERMGET;
			attr_doc  = item->mcs_member.m_doc;
			if (TYPE_MEMBER_ISCONST(&item->mcs_member)) {
				attr_type = Dee_TYPE(item->mcs_member.m_const);
				Dee_Incref(attr_type);
			} else {
#ifdef LOCAL_IS_CLASS
				/* TODO: Use `type_member_get(&item->mcs_member, (DeeObject *)tp_self)' to determine the proper attribute type! */
#else /* LOCAL_IS_CLASS */
				/* TODO: Use `type_member_get(&item->mcs_member, instance)' to determine the proper attribute type! */
#endif /* !LOCAL_IS_CLASS */
				attr_type = type_member_typefor(&item->mcs_member);
				Dee_XIncref(attr_type);
				if (!(item->mcs_member.m_field.m_type & STRUCT_CONST))
					attr_perm |= ATTR_PERMDEL | ATTR_PERMSET;
			}
			break;

		case MEMBERCACHE_ATTRIB: {
			struct class_attribute *catt;
			struct instance_desc *inst;
			attr_doc = NULL;
			catt     = item->mcs_attrib.a_attr;
			attr_perm = LOCAL_ATTR_xMEMBER | ATTR_PERMGET | ATTR_PERMDEL | ATTR_PERMSET;
			if (catt->ca_doc) {
				attr_doc = DeeString_STR(catt->ca_doc);
				attr_perm |= ATTR_DOCOBJ;
				Dee_Incref(catt->ca_doc);
			}
			if (catt->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
				attr_perm |= ATTR_PRIVATE;
			if (catt->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				attr_perm |= ATTR_PROPERTY;
				attr_type = NULL;
			} else if (catt->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
				attr_perm |= ATTR_PERMCALL;
				attr_type = &DeeInstanceMethod_Type;
				Dee_Incref(attr_type);
			} else {
				attr_type = NULL;
			}

#ifdef LOCAL_IS_CLASS
			inst = class_desc_as_instance(item->mcs_attrib.a_desc);
#else /* LOCAL_IS_CLASS */
			inst = NULL;
			if (catt->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
				inst = class_desc_as_instance(item->mcs_attrib.a_desc);
			} else if (instance) {
				inst = DeeInstance_DESC(item->mcs_attrib.a_desc, instance);
			}
			if (inst != NULL)
#endif /* !LOCAL_IS_CLASS */
			{
				Dee_instance_desc_lock_read(inst);
				if (catt->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
					if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_GET])
						attr_perm &= ~ATTR_PERMGET;
					if (!(catt->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
						if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_DEL])
							attr_perm &= ~ATTR_PERMDEL;
						if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_SET])
							attr_perm &= ~ATTR_PERMSET;
					}
				} else if (!(catt->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
					ASSERT(!attr_type);
					attr_type = (DREF DeeTypeObject *)inst->id_vtab[catt->ca_addr + CLASS_GETSET_GET];
					if (attr_type) {
						attr_type = Dee_TYPE(attr_type);
						Dee_Incref(attr_type);
					}
				}
				Dee_instance_desc_lock_endread(inst);
			}
			if (catt->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
				attr_perm &= ~(ATTR_PERMDEL | ATTR_PERMSET);
		}	break;

#ifdef LOCAL_IS_CLASS
		case MEMBERCACHE_INSTANCE_METHOD:
			attr_perm = ATTR_CMEMBER | ATTR_IMEMBER | ATTR_WRAPPER | ATTR_PERMGET | ATTR_PERMCALL;
			attr_doc  = item->mcs_method.m_doc;
			attr_type = (item->mcs_method.m_flag & TYPE_METHOD_FKWDS)
			              ? &DeeKwClsMethod_Type
			              : &DeeClsMethod_Type;
			Dee_Incref(attr_type);
			break;

		case MEMBERCACHE_INSTANCE_GETSET:
			attr_perm = ATTR_CMEMBER | ATTR_IMEMBER | ATTR_WRAPPER | ATTR_PROPERTY;
			attr_doc  = item->mcs_getset.gs_doc;
			attr_type = NULL /*&DeeClsProperty_Type*/;
			if (item->mcs_getset.gs_get)
				attr_perm |= ATTR_PERMGET;
			if (item->mcs_getset.gs_del)
				attr_perm |= ATTR_PERMDEL;
			if (item->mcs_getset.gs_set)
				attr_perm |= ATTR_PERMSET;
			break;

		case MEMBERCACHE_INSTANCE_MEMBER:
			attr_perm = ATTR_CMEMBER | ATTR_IMEMBER | ATTR_WRAPPER;
			attr_doc  = item->mcs_member.m_doc;
			/*attr_type = &DeeClsMember_Type*/;
			if (TYPE_MEMBER_ISCONST(&item->mcs_member)) {
				attr_type = Dee_TYPE(item->mcs_member.m_const);
				Dee_Incref(attr_type);
			} else {
				attr_type = type_member_typefor(&item->mcs_member);
				Dee_XIncref(attr_type);
				if (!(item->mcs_member.m_field.m_type & STRUCT_CONST))
					attr_perm |= ATTR_PERMDEL | ATTR_PERMSET;
			}
			break;

		case MEMBERCACHE_INSTANCE_ATTRIB: {
			struct class_attribute *catt;
			attr_doc  = NULL;
			catt = item->mcs_attrib.a_attr;
			attr_perm = ATTR_CMEMBER | ATTR_IMEMBER | ATTR_WRAPPER | ATTR_PERMGET | ATTR_PERMDEL | ATTR_PERMSET;
			if (catt->ca_doc) {
				attr_doc = DeeString_STR(catt->ca_doc);
				attr_perm |= ATTR_DOCOBJ;
				Dee_Incref(catt->ca_doc);
			}
			if (catt->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
				attr_perm |= ATTR_PRIVATE;
			if (catt->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
				attr_perm |= ATTR_PROPERTY;
				attr_type = NULL;
			} else if (catt->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
				attr_perm |= ATTR_PERMCALL;
				attr_type = &DeeInstanceMethod_Type;
				Dee_Incref(attr_type);
			} else {
				attr_type = NULL;
			}
			if (catt->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
				struct instance_desc *inst;
				inst = class_desc_as_instance(item->mcs_attrib.a_desc);
				Dee_instance_desc_lock_read(inst);
				if (catt->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
					if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_GET])
						attr_perm &= ~ATTR_PERMGET;
					if (!(catt->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
						if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_DEL])
							attr_perm &= ~ATTR_PERMDEL;
						if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_SET])
							attr_perm &= ~ATTR_PERMSET;
					}
				} else if (!(catt->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
					ASSERT(!attr_type);
					attr_type = (DREF DeeTypeObject *)inst->id_vtab[catt->ca_addr + CLASS_GETSET_GET];
					if (attr_type) {
						attr_type = Dee_TYPE(attr_type);
						Dee_Incref(attr_type);
					}
				}
				Dee_instance_desc_lock_endread(inst);
			}
			if (catt->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
				attr_perm &= ~(ATTR_PERMDEL | ATTR_PERMSET);
		}	break;
#endif /* LOCAL_IS_CLASS */

#undef LOCAL_ATTR_xMEMBER

#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#ifdef LOCAL_IS_SET_BASIC
		default:
			/* Non-basic attribute */
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			return 2;
#else /* LOCAL_IS_SET_BASIC */
		default: __builtin_unreachable();
#endif /* !LOCAL_IS_SET_BASIC */
		}
#endif /* !LOCAL_IS_HAS */

#ifdef LOCAL_IS_FIND
		/* If the caller is looking for an attribute with
		 * specific permissions, check if we match those. */
		if ((attr_perm & rules->alr_perm_mask) != rules->alr_perm_value) {
			if (attr_perm & ATTR_DOCOBJ)
				Dee_Decref(COMPILER_CONTAINER_OF(attr_doc, DeeStringObject, s_str));
			Dee_XDecref_unlikely(attr_type);
			break;
		}

		/* Found it! */
		result->a_decl = (DREF DeeObject *)item->mcs_decl;
		Dee_Incref(result->a_decl);
		Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
		result->a_doc      = attr_doc;
		result->a_perm     = attr_perm;
		result->a_attrtype = attr_type; /* Inherit reference. */
		return 0;
#endif /* LOCAL_IS_FIND */
	} /* for (;; Dee_membercache_table_hashnx(i, perturb)) */

	/* Cache miss handler. */
	Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
cache_miss:
#if (defined(LOCAL_IS_GET) || defined(LOCAL_IS_CALL) || defined(LOCAL_IS_CALL_KW) || \
     defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW) || defined(LOCAL_IS_VCALLF))
	return ITER_DONE;
#elif defined(LOCAL_IS_BOUND)
	return -2;
#elif defined(LOCAL_IS_HAS)
	return false;
#elif defined(LOCAL_IS_DEL) || defined(LOCAL_IS_SET) || defined(LOCAL_IS_SET_BASIC)
	return 1;
#elif defined(LOCAL_IS_FIND)
	return 1;
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#ifdef LOCAL_HAS_kw
err_no_keywords:
#ifdef LOCAL_HAS_len
	err_keywords_func_not_accepted_len(tp_self, LOCAL_attr, attrlen, kw);
#else /* LOCAL_HAS_len */
	err_keywords_func_not_accepted(tp_self, LOCAL_attr, kw);
#endif /* !LOCAL_HAS_len */
	/* Fallthru to `err' below. */
#define NEED_err
#endif /* LOCAL_HAS_kw */

	/* Error handler. */
#ifdef NEED_err
#undef NEED_err
err:
#if (defined(LOCAL_IS_GET) || defined(LOCAL_IS_CALL) || defined(LOCAL_IS_CALL_KW) || \
     defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW) ||              \
     defined(LOCAL_IS_VCALLF))
	return NULL;
#elif defined(LOCAL_IS_BOUND) || defined(LOCAL_IS_DEL) || defined(LOCAL_IS_SET) || defined(LOCAL_IS_SET_BASIC)
	return -1;
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
#endif /* NEED_err */

#undef LOCAL_ATTR_ACCESS
#undef LOCAL_DeeInstance_DESC
#undef LOCAL_Dee_membercache_slot_matches
#undef LOCAL_err_cant_access_attribute
#undef LOCAL_err_classmember_requires_1_argument
#undef LOCAL_err_classproperty_requires_1_argument
#undef LOCAL_err_classmethod_requires_at_least_1_argument
#undef LOCAL_self
#undef LOCAL_attr
#undef LOCAL_hash
}

#undef LOCAL_argc
#undef LOCAL_argv
#undef LOCAL_kw
#undef LOCAL_HAS_argv
#undef LOCAL_HAS_args
#undef LOCAL_HAS_format
#undef LOCAL_invoke_dkwobjmethod_thisarg
#undef LOCAL_invoke_dobjmethod_thisarg
#undef LOCAL_invoke_dkwobjmethod
#undef LOCAL_invoke_dobjmethod
#undef LOCAL_invoke_type_member_get
#undef LOCAL_invoke_object_thisarg
#undef LOCAL_invoke_object
#undef LOCAL_invoke_attribute
#undef LOCAL_invoke_instance_attribute
#undef LOCAL_unpack_one_for_getter
#undef LOCAL_unpack_one_for_getter_cleanup
#undef LOCAL_MUST_ASSERT_ARGC
#undef LOCAL_assert_kw_empty_IS_NOOP
#undef LOCAL_assert_kw_empty
#undef LOCAL_ATTR_NONNULL
#undef LOCAL_DeeType_AccessCachedAttr
#undef LOCAL_IS_GET
#undef LOCAL_IS_BOUND
#undef LOCAL_IS_HAS
#undef LOCAL_IS_DEL
#undef LOCAL_IS_SET
#undef LOCAL_IS_SET_BASIC
#undef LOCAL_IS_CALL
#undef LOCAL_IS_CALL_KW
#undef LOCAL_IS_CALL_TUPLE
#undef LOCAL_IS_CALL_TUPLE_KW
#undef LOCAL_IS_VCALLF
#undef LOCAL_HAS_kw
#undef LOCAL_HAS_len
#undef LOCAL_IS_CLASS
#undef LOCAL_IS_INSTANCE
#undef LOCAL_HAS_self
#undef LOCAL_tp_cache

DECL_END

#undef DEFINE_DeeType_GetCachedAttr
#undef DEFINE_DeeType_GetCachedAttrLen
#undef DEFINE_DeeType_GetCachedClassAttr
#undef DEFINE_DeeType_GetCachedClassAttrLen
#undef DEFINE_DeeType_GetCachedInstanceAttr
#undef DEFINE_DeeType_GetCachedInstanceAttrLen
#undef DEFINE_DeeType_BoundCachedAttr
#undef DEFINE_DeeType_BoundCachedAttrLen
#undef DEFINE_DeeType_BoundCachedClassAttr
#undef DEFINE_DeeType_BoundCachedClassAttrLen
#undef DEFINE_DeeType_BoundCachedInstanceAttr
#undef DEFINE_DeeType_BoundCachedInstanceAttrLen
#undef DEFINE_DeeType_HasCachedAttr
#undef DEFINE_DeeType_HasCachedAttrLen
#undef DEFINE_DeeType_HasCachedClassAttr
#undef DEFINE_DeeType_HasCachedClassAttrLen
#undef DEFINE_DeeType_DelCachedAttr
#undef DEFINE_DeeType_DelCachedAttrLen
#undef DEFINE_DeeType_DelCachedClassAttr
#undef DEFINE_DeeType_DelCachedClassAttrLen
#undef DEFINE_DeeType_DelCachedInstanceAttr
#undef DEFINE_DeeType_DelCachedInstanceAttrLen
#undef DEFINE_DeeType_SetCachedAttr
#undef DEFINE_DeeType_SetCachedAttrLen
#undef DEFINE_DeeType_SetCachedClassAttr
#undef DEFINE_DeeType_SetCachedClassAttrLen
#undef DEFINE_DeeType_SetCachedInstanceAttr
#undef DEFINE_DeeType_SetCachedInstanceAttrLen
#undef DEFINE_DeeType_SetBasicCachedAttr
#undef DEFINE_DeeType_SetBasicCachedAttrLen
#undef DEFINE_DeeType_SetBasicCachedClassAttr
#undef DEFINE_DeeType_SetBasicCachedClassAttrLen
#undef DEFINE_DeeType_SetBasicCachedInstanceAttr
#undef DEFINE_DeeType_SetBasicCachedInstanceAttrLen
#undef DEFINE_DeeType_CallCachedAttr
#undef DEFINE_DeeType_CallCachedAttrLen
#undef DEFINE_DeeType_CallCachedClassAttr
#undef DEFINE_DeeType_CallCachedClassAttrLen
#undef DEFINE_DeeType_CallCachedInstanceAttr
#undef DEFINE_DeeType_CallCachedInstanceAttrLen
#undef DEFINE_DeeType_CallCachedAttrKw
#undef DEFINE_DeeType_CallCachedAttrLenKw
#undef DEFINE_DeeType_CallCachedClassAttrKw
#undef DEFINE_DeeType_CallCachedClassAttrLenKw
#undef DEFINE_DeeType_CallCachedInstanceAttrKw
#undef DEFINE_DeeType_CallCachedInstanceAttrLenKw
#undef DEFINE_DeeType_CallCachedAttrTuple
#undef DEFINE_DeeType_CallCachedAttrLenTuple
#undef DEFINE_DeeType_CallCachedClassAttrTuple
#undef DEFINE_DeeType_CallCachedClassAttrLenTuple
#undef DEFINE_DeeType_CallCachedInstanceAttrTuple
#undef DEFINE_DeeType_CallCachedInstanceAttrLenTuple
#undef DEFINE_DeeType_CallCachedAttrTupleKw
#undef DEFINE_DeeType_CallCachedAttrLenTupleKw
#undef DEFINE_DeeType_CallCachedClassAttrTupleKw
#undef DEFINE_DeeType_CallCachedClassAttrLenTupleKw
#undef DEFINE_DeeType_CallCachedInstanceAttrTupleKw
#undef DEFINE_DeeType_CallCachedInstanceAttrLenTupleKw
#undef DEFINE_DeeType_VCallCachedAttrf
#undef DEFINE_DeeType_VCallCachedAttrLenf
#undef DEFINE_DeeType_VCallCachedClassAttrf
#undef DEFINE_DeeType_VCallCachedClassAttrLenf
#undef DEFINE_DeeType_VCallCachedInstanceAttrf
#undef DEFINE_DeeType_VCallCachedInstanceAttrLenf
#undef DEFINE_DeeType_FindCachedAttr
#undef DEFINE_DeeType_FindCachedClassAttr
