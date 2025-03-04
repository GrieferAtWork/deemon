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
#include "mro.c"

#undef DeeType_GetCachedAttrStringHash
#undef DeeType_GetCachedAttrStringLenHash
#undef DeeType_GetCachedClassAttrStringHash
#undef DeeType_GetCachedClassAttrStringLenHash
#undef DeeType_GetCachedInstanceAttrStringHash
#undef DeeType_GetCachedInstanceAttrStringLenHash
#undef DeeType_BoundCachedAttrStringHash
#undef DeeType_BoundCachedAttrStringLenHash
#undef DeeType_BoundCachedClassAttrStringHash
#undef DeeType_BoundCachedClassAttrStringLenHash
#undef DeeType_BoundCachedInstanceAttrStringHash
#undef DeeType_BoundCachedInstanceAttrStringLenHash
#undef DeeType_HasCachedAttrStringHash
#undef DeeType_HasCachedAttrStringLenHash
#undef DeeType_HasCachedClassAttrStringHash
#undef DeeType_HasCachedClassAttrStringLenHash
#undef DeeType_HasCachedInstanceAttrStringHash
#undef DeeType_HasCachedInstanceAttrStringLenHash
#undef DeeType_DelCachedAttrStringHash
#undef DeeType_DelCachedAttrStringLenHash
#undef DeeType_DelCachedClassAttrStringHash
#undef DeeType_DelCachedClassAttrStringLenHash
#undef DeeType_DelCachedInstanceAttrStringHash
#undef DeeType_DelCachedInstanceAttrStringLenHash
#undef DeeType_SetCachedAttrStringHash
#undef DeeType_SetCachedAttrStringLenHash
#undef DeeType_SetCachedClassAttrStringHash
#undef DeeType_SetCachedClassAttrStringLenHash
#undef DeeType_SetCachedInstanceAttrStringHash
#undef DeeType_SetCachedInstanceAttrStringLenHash
#undef DeeType_SetBasicCachedAttrStringHash
#undef DeeType_SetBasicCachedAttrStringLenHash
#undef DeeType_SetBasicCachedClassAttrStringHash
#undef DeeType_SetBasicCachedClassAttrStringLenHash
#undef DeeType_SetBasicCachedInstanceAttrStringHash
#undef DeeType_SetBasicCachedInstanceAttrStringLenHash
#undef DeeType_CallCachedAttrStringHash
#undef DeeType_CallCachedAttrStringLenHash
#undef DeeType_CallCachedClassAttrStringHash
#undef DeeType_CallCachedClassAttrStringLenHash
#undef DeeType_CallCachedInstanceAttrStringHash
#undef DeeType_CallCachedInstanceAttrStringLenHash
#undef DeeType_CallCachedAttrStringHash
#undef DeeType_CallCachedAttrStringLenHash
#undef DeeType_CallCachedClassAttrStringHash
#undef DeeType_CallCachedClassAttrStringLenHash
#undef DeeType_CallCachedInstanceAttrStringHash
#undef DeeType_CallCachedInstanceAttrStringLenHash
#undef DeeType_CallCachedAttrStringHashTuple
#undef DeeType_CallCachedAttrStringLenHashTuple
#undef DeeType_CallCachedClassAttrStringHashTuple
#undef DeeType_CallCachedClassAttrStringLenHashTuple
#undef DeeType_CallCachedInstanceAttrStringHashTuple
#undef DeeType_CallCachedInstanceAttrStringLenHashTuple
#undef DeeType_CallCachedAttrStringHash
#undef DeeType_CallCachedAttrStringLenHash
#undef DeeType_CallCachedClassAttrStringHash
#undef DeeType_CallCachedClassAttrStringLenHash
#undef DeeType_CallCachedInstanceAttrStringHash
#undef DeeType_CallCachedInstanceAttrStringLenHash
#undef DeeType_VCallCachedAttrStringHashf
#undef DeeType_VCallCachedAttrStringLenHashf
#undef DeeType_VCallCachedClassAttrStringHashf
#undef DeeType_VCallCachedClassAttrStringLenHashf
#undef DeeType_VCallCachedInstanceAttrStringHashf
#undef DeeType_VCallCachedInstanceAttrStringLenHashf
#undef DeeType_FindCachedAttrInfoStringHash
#undef DeeType_FindCachedClassAttrInfoStringHash
#undef DeeType_FindCachedAttrInfoStringLenHash
#undef DeeType_FindCachedClassAttrInfoStringLenHash
#undef DeeType_FindCachedAttr
#undef DeeType_FindCachedClassAttr

//#define DEFINE_DeeType_GetCachedAttrStringHash
//#define DEFINE_DeeType_GetCachedAttrStringLenHash
//#define DEFINE_DeeType_GetCachedClassAttrStringHash
//#define DEFINE_DeeType_GetCachedClassAttrStringLenHash
//#define DEFINE_DeeType_GetCachedInstanceAttrStringHash
//#define DEFINE_DeeType_GetCachedInstanceAttrStringLenHash
//#define DEFINE_DeeType_BoundCachedAttrStringHash
//#define DEFINE_DeeType_BoundCachedAttrStringLenHash
//#define DEFINE_DeeType_BoundCachedClassAttrStringHash
//#define DEFINE_DeeType_BoundCachedClassAttrStringLenHash
//#define DEFINE_DeeType_BoundCachedInstanceAttrStringHash
//#define DEFINE_DeeType_BoundCachedInstanceAttrStringLenHash
//#define DEFINE_DeeType_HasCachedAttrStringHash
//#define DEFINE_DeeType_HasCachedAttrStringLenHash
//#define DEFINE_DeeType_HasCachedClassAttrStringHash
//#define DEFINE_DeeType_HasCachedClassAttrStringLenHash
//#define DEFINE_DeeType_DelCachedAttrStringHash
//#define DEFINE_DeeType_DelCachedAttrStringLenHash
//#define DEFINE_DeeType_DelCachedClassAttrStringHash
//#define DEFINE_DeeType_DelCachedClassAttrStringLenHash
//#define DEFINE_DeeType_DelCachedInstanceAttrStringHash
//#define DEFINE_DeeType_DelCachedInstanceAttrStringLenHash
//#define DEFINE_DeeType_SetCachedAttrStringHash
//#define DEFINE_DeeType_SetCachedAttrStringLenHash
//#define DEFINE_DeeType_SetCachedClassAttrStringHash
//#define DEFINE_DeeType_SetCachedClassAttrStringLenHash
//#define DEFINE_DeeType_SetCachedInstanceAttrStringHash
//#define DEFINE_DeeType_SetCachedInstanceAttrStringLenHash
//#define DEFINE_DeeType_SetBasicCachedAttrStringHash
//#define DEFINE_DeeType_SetBasicCachedAttrStringLenHash
//#define DEFINE_DeeType_SetBasicCachedClassAttrStringHash
//#define DEFINE_DeeType_SetBasicCachedClassAttrStringLenHash
//#define DEFINE_DeeType_SetBasicCachedInstanceAttrStringHash
//#define DEFINE_DeeType_SetBasicCachedInstanceAttrStringLenHash
//#define DEFINE_DeeType_CallCachedAttrStringHash
//#define DEFINE_DeeType_CallCachedAttrStringLenHash
//#define DEFINE_DeeType_CallCachedClassAttrStringHash
//#define DEFINE_DeeType_CallCachedClassAttrStringLenHash
//#define DEFINE_DeeType_CallCachedInstanceAttrStringHash
//#define DEFINE_DeeType_CallCachedInstanceAttrStringLenHash
//#define DEFINE_DeeType_CallCachedAttrStringHashKw
//#define DEFINE_DeeType_CallCachedAttrStringLenHashKw
//#define DEFINE_DeeType_CallCachedClassAttrStringHashKw
//#define DEFINE_DeeType_CallCachedClassAttrStringLenHashKw
//#define DEFINE_DeeType_CallCachedInstanceAttrStringHashKw
//#define DEFINE_DeeType_CallCachedInstanceAttrStringLenHashKw
//#define DEFINE_DeeType_CallCachedAttrStringHashTuple
//#define DEFINE_DeeType_CallCachedAttrStringLenHashTuple
//#define DEFINE_DeeType_CallCachedClassAttrStringHashTuple
//#define DEFINE_DeeType_CallCachedClassAttrStringLenHashTuple
//#define DEFINE_DeeType_CallCachedInstanceAttrStringHashTuple
//#define DEFINE_DeeType_CallCachedInstanceAttrStringLenHashTuple
//#define DEFINE_DeeType_CallCachedAttrStringHashTupleKw
//#define DEFINE_DeeType_CallCachedAttrStringLenHashTupleKw
//#define DEFINE_DeeType_CallCachedClassAttrStringHashTupleKw
//#define DEFINE_DeeType_CallCachedClassAttrStringLenHashTupleKw
//#define DEFINE_DeeType_CallCachedInstanceAttrStringHashTupleKw
//#define DEFINE_DeeType_CallCachedInstanceAttrStringLenHashTupleKw
//#define DEFINE_DeeType_VCallCachedAttrStringHashf
//#define DEFINE_DeeType_VCallCachedAttrStringLenHashf
//#define DEFINE_DeeType_VCallCachedClassAttrStringHashf
//#define DEFINE_DeeType_VCallCachedClassAttrStringLenHashf
//#define DEFINE_DeeType_VCallCachedInstanceAttrStringHashf
//#define DEFINE_DeeType_VCallCachedInstanceAttrStringLenHashf
//#define DEFINE_DeeType_FindCachedAttrInfoStringHash
//#define DEFINE_DeeType_FindCachedClassAttrInfoStringHash
#define DEFINE_DeeType_FindCachedAttrInfoStringLenHash
//#define DEFINE_DeeType_FindCachedClassAttrInfoStringLenHash
//#define DEFINE_DeeType_FindCachedAttr
//#define DEFINE_DeeType_FindCachedClassAttr
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/instancemethod.h>
#include <deemon/kwds.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/system-features.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
/**/

#include "kwlist.h" /* kwlist__thisarg */
#include "runtime_error.h"
/**/

#include <stdint.h>


#if (defined(DEFINE_DeeType_GetCachedAttrStringHash) +                    \
     defined(DEFINE_DeeType_GetCachedAttrStringLenHash) +                 \
     defined(DEFINE_DeeType_GetCachedClassAttrStringHash) +               \
     defined(DEFINE_DeeType_GetCachedClassAttrStringLenHash) +            \
     defined(DEFINE_DeeType_GetCachedInstanceAttrStringHash) +            \
     defined(DEFINE_DeeType_GetCachedInstanceAttrStringLenHash) +         \
     defined(DEFINE_DeeType_BoundCachedAttrStringHash) +                  \
     defined(DEFINE_DeeType_BoundCachedAttrStringLenHash) +               \
     defined(DEFINE_DeeType_BoundCachedClassAttrStringHash) +             \
     defined(DEFINE_DeeType_BoundCachedClassAttrStringLenHash) +          \
     defined(DEFINE_DeeType_BoundCachedInstanceAttrStringHash) +          \
     defined(DEFINE_DeeType_BoundCachedInstanceAttrStringLenHash) +       \
     defined(DEFINE_DeeType_HasCachedAttrStringHash) +                    \
     defined(DEFINE_DeeType_HasCachedAttrStringLenHash) +                 \
     defined(DEFINE_DeeType_HasCachedClassAttrStringHash) +               \
     defined(DEFINE_DeeType_HasCachedClassAttrStringLenHash) +            \
     defined(DEFINE_DeeType_DelCachedAttrStringHash) +                    \
     defined(DEFINE_DeeType_DelCachedAttrStringLenHash) +                 \
     defined(DEFINE_DeeType_DelCachedClassAttrStringHash) +               \
     defined(DEFINE_DeeType_DelCachedClassAttrStringLenHash) +            \
     defined(DEFINE_DeeType_DelCachedInstanceAttrStringHash) +            \
     defined(DEFINE_DeeType_DelCachedInstanceAttrStringLenHash) +         \
     defined(DEFINE_DeeType_SetCachedAttrStringHash) +                    \
     defined(DEFINE_DeeType_SetCachedAttrStringLenHash) +                 \
     defined(DEFINE_DeeType_SetCachedClassAttrStringHash) +               \
     defined(DEFINE_DeeType_SetCachedClassAttrStringLenHash) +            \
     defined(DEFINE_DeeType_SetCachedInstanceAttrStringHash) +            \
     defined(DEFINE_DeeType_SetCachedInstanceAttrStringLenHash) +         \
     defined(DEFINE_DeeType_SetBasicCachedAttrStringHash) +               \
     defined(DEFINE_DeeType_SetBasicCachedAttrStringLenHash) +            \
     defined(DEFINE_DeeType_SetBasicCachedClassAttrStringHash) +          \
     defined(DEFINE_DeeType_SetBasicCachedClassAttrStringLenHash) +       \
     defined(DEFINE_DeeType_SetBasicCachedInstanceAttrStringHash) +       \
     defined(DEFINE_DeeType_SetBasicCachedInstanceAttrStringLenHash) +    \
     defined(DEFINE_DeeType_CallCachedAttrStringHash) +                   \
     defined(DEFINE_DeeType_CallCachedAttrStringLenHash) +                \
     defined(DEFINE_DeeType_CallCachedClassAttrStringHash) +              \
     defined(DEFINE_DeeType_CallCachedClassAttrStringLenHash) +           \
     defined(DEFINE_DeeType_CallCachedInstanceAttrStringHash) +           \
     defined(DEFINE_DeeType_CallCachedInstanceAttrStringLenHash) +        \
     defined(DEFINE_DeeType_CallCachedAttrStringHashKw) +                 \
     defined(DEFINE_DeeType_CallCachedAttrStringLenHashKw) +              \
     defined(DEFINE_DeeType_CallCachedClassAttrStringHashKw) +            \
     defined(DEFINE_DeeType_CallCachedClassAttrStringLenHashKw) +         \
     defined(DEFINE_DeeType_CallCachedInstanceAttrStringHashKw) +         \
     defined(DEFINE_DeeType_CallCachedInstanceAttrStringLenHashKw) +      \
     defined(DEFINE_DeeType_CallCachedAttrStringHashTuple) +              \
     defined(DEFINE_DeeType_CallCachedAttrStringLenHashTuple) +           \
     defined(DEFINE_DeeType_CallCachedClassAttrStringHashTuple) +         \
     defined(DEFINE_DeeType_CallCachedClassAttrStringLenHashTuple) +      \
     defined(DEFINE_DeeType_CallCachedInstanceAttrStringHashTuple) +      \
     defined(DEFINE_DeeType_CallCachedInstanceAttrStringLenHashTuple) +   \
     defined(DEFINE_DeeType_CallCachedAttrStringHashTupleKw) +            \
     defined(DEFINE_DeeType_CallCachedAttrStringLenHashTupleKw) +         \
     defined(DEFINE_DeeType_CallCachedClassAttrStringHashTupleKw) +       \
     defined(DEFINE_DeeType_CallCachedClassAttrStringLenHashTupleKw) +    \
     defined(DEFINE_DeeType_CallCachedInstanceAttrStringHashTupleKw) +    \
     defined(DEFINE_DeeType_CallCachedInstanceAttrStringLenHashTupleKw) + \
     defined(DEFINE_DeeType_VCallCachedAttrStringHashf) +                 \
     defined(DEFINE_DeeType_VCallCachedAttrStringLenHashf) +              \
     defined(DEFINE_DeeType_VCallCachedClassAttrStringHashf) +            \
     defined(DEFINE_DeeType_VCallCachedClassAttrStringLenHashf) +         \
     defined(DEFINE_DeeType_VCallCachedInstanceAttrStringHashf) +         \
     defined(DEFINE_DeeType_VCallCachedInstanceAttrStringLenHashf) +      \
     defined(DEFINE_DeeType_FindCachedAttrInfoStringHash) +               \
     defined(DEFINE_DeeType_FindCachedClassAttrInfoStringHash) +          \
     defined(DEFINE_DeeType_FindCachedAttrInfoStringLenHash) +            \
     defined(DEFINE_DeeType_FindCachedClassAttrInfoStringLenHash) +      \
     defined(DEFINE_DeeType_FindCachedAttr) +                            \
     defined(DEFINE_DeeType_FindCachedClassAttr)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */


DECL_BEGIN

#ifdef DEFINE_DeeType_GetCachedAttrStringHash
#define LOCAL_DeeType_AccessCachedAttr DeeType_GetCachedAttrStringHash
#define LOCAL_IS_GET
#elif defined(DEFINE_DeeType_GetCachedAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_GetCachedAttrStringLenHash
#define LOCAL_IS_GET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_GetCachedClassAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_GetCachedClassAttrStringHash
#define LOCAL_IS_GET
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_GetCachedClassAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_GetCachedClassAttrStringLenHash
#define LOCAL_IS_GET
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_GetCachedInstanceAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_GetCachedInstanceAttrStringHash
#define LOCAL_IS_GET
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_GetCachedInstanceAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_GetCachedInstanceAttrStringLenHash
#define LOCAL_IS_GET
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_BoundCachedAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_BoundCachedAttrStringHash
#define LOCAL_IS_BOUND
#elif defined(DEFINE_DeeType_BoundCachedAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_BoundCachedAttrStringLenHash
#define LOCAL_IS_BOUND
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_BoundCachedClassAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_BoundCachedClassAttrStringHash
#define LOCAL_IS_BOUND
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_BoundCachedClassAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_BoundCachedClassAttrStringLenHash
#define LOCAL_IS_BOUND
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_BoundCachedInstanceAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_BoundCachedInstanceAttrStringHash
#define LOCAL_IS_BOUND
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_BoundCachedInstanceAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_BoundCachedInstanceAttrStringLenHash
#define LOCAL_IS_BOUND
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_HasCachedAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_HasCachedAttrStringHash
#define LOCAL_IS_HAS
#elif defined(DEFINE_DeeType_HasCachedAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_HasCachedAttrStringLenHash
#define LOCAL_IS_HAS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_HasCachedClassAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_HasCachedClassAttrStringHash
#define LOCAL_IS_HAS
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_HasCachedClassAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_HasCachedClassAttrStringLenHash
#define LOCAL_IS_HAS
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_HasCachedInstanceAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_HasCachedInstanceAttrStringHash
#define LOCAL_IS_HAS
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_HasCachedInstanceAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_HasCachedInstanceAttrStringLenHash
#define LOCAL_IS_HAS
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_DelCachedAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_DelCachedAttrStringHash
#define LOCAL_IS_DEL
#elif defined(DEFINE_DeeType_DelCachedAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_DelCachedAttrStringLenHash
#define LOCAL_IS_DEL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_DelCachedClassAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_DelCachedClassAttrStringHash
#define LOCAL_IS_DEL
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_DelCachedClassAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_DelCachedClassAttrStringLenHash
#define LOCAL_IS_DEL
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_DelCachedInstanceAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_DelCachedInstanceAttrStringHash
#define LOCAL_IS_DEL
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_DelCachedInstanceAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_DelCachedInstanceAttrStringLenHash
#define LOCAL_IS_DEL
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetCachedAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetCachedAttrStringHash
#define LOCAL_IS_SET
#elif defined(DEFINE_DeeType_SetCachedAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetCachedAttrStringLenHash
#define LOCAL_IS_SET
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetCachedClassAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetCachedClassAttrStringHash
#define LOCAL_IS_SET
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_SetCachedClassAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetCachedClassAttrStringLenHash
#define LOCAL_IS_SET
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetCachedInstanceAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetCachedInstanceAttrStringHash
#define LOCAL_IS_SET
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_SetCachedInstanceAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetCachedInstanceAttrStringLenHash
#define LOCAL_IS_SET
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetBasicCachedAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetBasicCachedAttrStringHash
#define LOCAL_IS_SET_BASIC
#elif defined(DEFINE_DeeType_SetBasicCachedAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetBasicCachedAttrStringLenHash
#define LOCAL_IS_SET_BASIC
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetBasicCachedClassAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetBasicCachedClassAttrStringHash
#define LOCAL_IS_SET_BASIC
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_SetBasicCachedClassAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetBasicCachedClassAttrStringLenHash
#define LOCAL_IS_SET_BASIC
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_SetBasicCachedInstanceAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetBasicCachedInstanceAttrStringHash
#define LOCAL_IS_SET_BASIC
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_SetBasicCachedInstanceAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_SetBasicCachedInstanceAttrStringLenHash
#define LOCAL_IS_SET_BASIC
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrStringHash
#define LOCAL_IS_CALL
#elif defined(DEFINE_DeeType_CallCachedAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrStringLenHash
#define LOCAL_IS_CALL
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedClassAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrStringHash
#define LOCAL_IS_CALL
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_CallCachedClassAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrStringLenHash
#define LOCAL_IS_CALL
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrStringHash
#define LOCAL_IS_CALL
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrStringLenHash
#define LOCAL_IS_CALL
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedAttrStringHashKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrStringHashKw
#define LOCAL_IS_CALL_KW
#elif defined(DEFINE_DeeType_CallCachedAttrStringLenHashKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrStringLenHashKw
#define LOCAL_IS_CALL_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedClassAttrStringHashKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrStringHashKw
#define LOCAL_IS_CALL_KW
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_CallCachedClassAttrStringLenHashKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrStringLenHashKw
#define LOCAL_IS_CALL_KW
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrStringHashKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrStringHashKw
#define LOCAL_IS_CALL_KW
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrStringLenHashKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrStringLenHashKw
#define LOCAL_IS_CALL_KW
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedAttrStringHashTuple)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrStringHashTuple
#define LOCAL_IS_CALL_TUPLE
#elif defined(DEFINE_DeeType_CallCachedAttrStringLenHashTuple)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrStringLenHashTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedClassAttrStringHashTuple)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrStringHashTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_CallCachedClassAttrStringLenHashTuple)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrStringLenHashTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrStringHashTuple)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrStringHashTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrStringLenHashTuple)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrStringLenHashTuple
#define LOCAL_IS_CALL_TUPLE
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedAttrStringHashTupleKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrStringHashTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#elif defined(DEFINE_DeeType_CallCachedAttrStringLenHashTupleKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedAttrStringLenHashTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedClassAttrStringHashTupleKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrStringHashTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_CallCachedClassAttrStringLenHashTupleKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedClassAttrStringLenHashTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrStringHashTupleKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrStringHashTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_CallCachedInstanceAttrStringLenHashTupleKw)
#define LOCAL_DeeType_AccessCachedAttr DeeType_CallCachedInstanceAttrStringLenHashTupleKw
#define LOCAL_IS_CALL_TUPLE_KW
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_VCallCachedAttrStringHashf)
#define LOCAL_DeeType_AccessCachedAttr DeeType_VCallCachedAttrStringHashf
#define LOCAL_IS_VCALLF
#elif defined(DEFINE_DeeType_VCallCachedAttrStringLenHashf)
#define LOCAL_DeeType_AccessCachedAttr DeeType_VCallCachedAttrStringLenHashf
#define LOCAL_IS_VCALLF
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_VCallCachedClassAttrStringHashf)
#define LOCAL_DeeType_AccessCachedAttr DeeType_VCallCachedClassAttrStringHashf
#define LOCAL_IS_VCALLF
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_VCallCachedClassAttrStringLenHashf)
#define LOCAL_DeeType_AccessCachedAttr DeeType_VCallCachedClassAttrStringLenHashf
#define LOCAL_IS_VCALLF
#define LOCAL_IS_CLASS
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_VCallCachedInstanceAttrStringHashf)
#define LOCAL_DeeType_AccessCachedAttr DeeType_VCallCachedInstanceAttrStringHashf
#define LOCAL_IS_VCALLF
#define LOCAL_IS_INSTANCE
#elif defined(DEFINE_DeeType_VCallCachedInstanceAttrStringLenHashf)
#define LOCAL_DeeType_AccessCachedAttr DeeType_VCallCachedInstanceAttrStringLenHashf
#define LOCAL_IS_VCALLF
#define LOCAL_IS_INSTANCE
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_FindCachedAttrInfoStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_FindCachedAttrInfoStringHash
#define LOCAL_IS_FINDINFO
#elif defined(DEFINE_DeeType_FindCachedClassAttrInfoStringHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_FindCachedClassAttrInfoStringHash
#define LOCAL_IS_FINDINFO
#define LOCAL_IS_CLASS
#elif defined(DEFINE_DeeType_FindCachedAttrInfoStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_FindCachedAttrInfoStringLenHash
#define LOCAL_IS_FINDINFO
#define LOCAL_HAS_len
#elif defined(DEFINE_DeeType_FindCachedClassAttrInfoStringLenHash)
#define LOCAL_DeeType_AccessCachedAttr DeeType_FindCachedClassAttrInfoStringLenHash
#define LOCAL_IS_FINDINFO
#define LOCAL_IS_CLASS
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

#if !defined(LOCAL_IS_CLASS) && !defined(LOCAL_IS_INSTANCE) && !defined(LOCAL_IS_HAS) && !defined(LOCAL_IS_FINDINFO)
#define LOCAL_HAS_self
#endif /* !LOCAL_IS_CLASS && !LOCAL_IS_INSTANCE && !LOCAL_IS_HAS && !LOCAL_IS_FINDINFO */

#ifdef LOCAL_IS_CLASS
#define LOCAL_tp_cache tp_class_cache
#else /* LOCAL_IS_CLASS */
#define LOCAL_tp_cache tp_cache
#endif /* !LOCAL_IS_CLASS */

#if (defined(LOCAL_IS_SET) || defined(LOCAL_IS_FINDINFO) || \
     defined(LOCAL_IS_SET_BASIC) || defined(LOCAL_IS_CALL_TUPLE) || \
     defined(LOCAL_IS_CALL_TUPLE_KW) || defined(LOCAL_IS_VCALLF))
#if defined(LOCAL_HAS_self) && defined(LOCAL_HAS_len)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3, 6))
#elif defined(LOCAL_HAS_self)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 3, 5))
#elif defined(LOCAL_HAS_len)
#define LOCAL_ATTR_NONNULL NONNULL((1, 2, 5))
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
	DeeArg_UnpackKw(LOCAL_argc, LOCAL_argv, LOCAL_kw, kwlist__thisarg, "o:get", p_thisarg)
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
	DeeArg_UnpackKw(LOCAL_argc, LOCAL_argv, LOCAL_kw, kwlist__thisarg, "o:get", p_thisarg)
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
#define LOCAL_invoke_dkwobjmethod_thisarg(func, thisarg) DeeKwObjMethod_VCallFuncf(func, thisarg, format, args)
#define LOCAL_invoke_dobjmethod_thisarg(func, thisarg)   DeeObjMethod_VCallFuncf(func, thisarg, format, args)
#ifdef LOCAL_HAS_len
#define LOCAL_invoke_dkwobjmethod(func, decl) \
	dkwobjmethod_vcallf_len(func, decl, attr, attrlen, format, args)
#define LOCAL_invoke_dobjmethod(func, decl) \
	dobjmethod_vcallf_len(func, decl, attr, attrlen, format, args)
#ifndef DKWOBJMETHOD_VCALLF_LEN_DEFINED
#define DKWOBJMETHOD_VCALLF_LEN_DEFINED
PRIVATE NONNULL((1, 2, 3, 5)) DREF DeeObject *DCALL
dkwobjmethod_vcallf_len(dkwobjmethod_t self,
                        DeeTypeObject *__restrict cls_type,
                        char const *__restrict attr, size_t attrlen,
                        char const *__restrict format, va_list args) {
	DREF DeeObject *result, *thisarg;
	if unlikely(*format == '\0')
		goto err_noargs;

	/* Use the first argument as the this-argument. */
	thisarg = Dee_VPPackf((char const **)&format, (struct va_list_struct *)VALIST_ADDR(args));
	if unlikely(!thisarg) {
		Dee_VPPackf_Cleanup(format, ((struct va_list_struct *)VALIST_ADDR(args))->vl_ap);
		goto err;
	} 
	if (DeeObject_AssertTypeOrAbstract(thisarg, cls_type))
		goto err_thisarg;

	/* Invoke the function. */
	result = DeeKwObjMethod_VCallFuncf(self, thisarg, format, args);
	Dee_Decref(thisarg);
	return result;
err_thisarg:
	Dee_Decref(thisarg);
err:
	return NULL;
err_noargs:
	err_classmethod_requires_at_least_1_argument_string_len(cls_type, attr, attrlen);
	goto err;
}
#endif /* !DKWOBJMETHOD_VCALLF_LEN_DEFINED */
#ifndef DOBJMETHOD_VCALLF_LEN_DEFINED
#define DOBJMETHOD_VCALLF_LEN_DEFINED
PRIVATE NONNULL((1, 2, 3, 5)) DREF DeeObject *DCALL
dobjmethod_vcallf_len(dobjmethod_t self,
                      DeeTypeObject *__restrict cls_type,
                      char const *__restrict attr, size_t attrlen,
                      char const *__restrict format, va_list args) {
	DREF DeeObject *result, *thisarg;
	if unlikely(*format == '\0')
		goto err_noargs;

	/* Use the first argument as the this-argument. */
	thisarg = Dee_VPPackf((char const **)&format, (struct va_list_struct *)VALIST_ADDR(args));
	if unlikely(!thisarg) {
		Dee_VPPackf_Cleanup(format, ((struct va_list_struct *)VALIST_ADDR(args))->vl_ap);
		goto err;
	} 
	if (DeeObject_AssertTypeOrAbstract(thisarg, cls_type))
		goto err_thisarg;

	/* Invoke the function. */
	result = DeeObjMethod_VCallFuncf(self, thisarg, format, args);
	Dee_Decref(thisarg);
	return result;
err_thisarg:
	Dee_Decref(thisarg);
err:
	return NULL;
err_noargs:
	err_classmethod_requires_at_least_1_argument_string_len(cls_type, attr, attrlen);
	goto err;
}
#endif /* !DOBJMETHOD_VCALLF_LEN_DEFINED */
#else /* LOCAL_HAS_len */
#define LOCAL_invoke_dkwobjmethod(func, decl) \
	dkwobjmethod_vcallf(func, decl, attr, format, args)
#define LOCAL_invoke_dobjmethod(func, decl) \
	dobjmethod_vcallf(func, decl, attr, format, args)
#ifndef DKWOBJMETHOD_VCALLF_DEFINED
#define DKWOBJMETHOD_VCALLF_DEFINED
PRIVATE NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
dkwobjmethod_vcallf(dkwobjmethod_t self,
                    DeeTypeObject *__restrict cls_type,
                    char const *__restrict attr,
                    char const *__restrict format, va_list args) {
	DREF DeeObject *result, *thisarg;
	if unlikely(*format == '\0')
		goto err_noargs;

	/* Use the first argument as the this-argument. */
	thisarg = Dee_VPPackf((char const **)&format, (struct va_list_struct *)VALIST_ADDR(args));
	if unlikely(!thisarg) {
		Dee_VPPackf_Cleanup(format, ((struct va_list_struct *)VALIST_ADDR(args))->vl_ap);
		goto err;
	} 
	if (DeeObject_AssertTypeOrAbstract(thisarg, cls_type))
		goto err_thisarg;

	/* Invoke the function. */
	result = DeeKwObjMethod_VCallFuncf(self, thisarg, format, args);
	Dee_Decref(thisarg);
	return result;
err_thisarg:
	Dee_Decref(thisarg);
err:
	return NULL;
err_noargs:
	err_classmethod_requires_at_least_1_argument_string(cls_type, attr);
	goto err;
}
#endif /* !DKWOBJMETHOD_VCALLF_DEFINED */
#ifndef DOBJMETHOD_VCALLF_DEFINED
#define DOBJMETHOD_VCALLF_DEFINED
PRIVATE NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL
dobjmethod_vcallf(dobjmethod_t self,
                  DeeTypeObject *__restrict cls_type,
                  char const *__restrict attr,
                  char const *__restrict format, va_list args) {
	DREF DeeObject *result, *thisarg;
	if unlikely(*format == '\0')
		goto err_noargs;

	/* Use the first argument as the this-argument. */
	thisarg = Dee_VPPackf((char const **)&format, (struct va_list_struct *)VALIST_ADDR(args));
	if unlikely(!thisarg) {
		Dee_VPPackf_Cleanup(format, ((struct va_list_struct *)VALIST_ADDR(args))->vl_ap);
		goto err;
	} 
	if (DeeObject_AssertTypeOrAbstract(thisarg, cls_type))
		goto err_thisarg;

	/* Invoke the function. */
	result = DeeObjMethod_VCallFuncf(self, thisarg, format, args);
	Dee_Decref(thisarg);
	return result;
err_thisarg:
	Dee_Decref(thisarg);
err:
	return NULL;
err_noargs:
	err_classmethod_requires_at_least_1_argument_string(cls_type, attr);
	goto err;
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
INTERN WUNUSED LOCAL_ATTR_NONNULL DREF DeeObject *
#elif defined(LOCAL_IS_BOUND)
/* @return: Dee_BOUND_YES:     Attribute is bound.
 * @return: Dee_BOUND_NO:      Attribute isn't bound.
 * @return: Dee_BOUND_ERR:     An error occurred.
 * @return: Dee_BOUND_MISSING: The attribute doesn't exist. */
INTERN WUNUSED LOCAL_ATTR_NONNULL int
#elif defined(LOCAL_IS_HAS)
/* @return: true : The attribute exists.
 * @return: false: The attribute doesn't exist. */
INTERN WUNUSED LOCAL_ATTR_NONNULL bool
#elif defined(LOCAL_IS_DEL)
/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the delete-operator on the attribute.
 * @return: -1: An error occurred. */
INTERN WUNUSED LOCAL_ATTR_NONNULL int
#elif defined(LOCAL_IS_SET)
/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the set-operator on the attribute.
 * @return: -1: An error occurred. */
INTERN WUNUSED LOCAL_ATTR_NONNULL int
#elif defined(LOCAL_IS_SET_BASIC)
/* @return:  2: The attribute is non-basic.
 * @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the set-operator on the attribute.
 * @return: -1: An error occurred. */
INTERN WUNUSED LOCAL_ATTR_NONNULL int
#elif (defined(LOCAL_IS_CALL) || defined(LOCAL_IS_CALL_KW) ||             \
       defined(LOCAL_IS_CALL_TUPLE) || defined(LOCAL_IS_CALL_TUPLE_KW) || \
       defined(LOCAL_IS_VCALLF))
/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTERN WUNUSED LOCAL_ATTR_NONNULL DREF DeeObject *
#elif defined(LOCAL_IS_FINDINFO)
/* @return: true:  Attribute was found.
 * @return: false: Attribute wasn't found. */
INTDEF WUNUSED LOCAL_ATTR_NONNULL bool
#elif defined(LOCAL_IS_FIND)
INTERN WUNUSED LOCAL_ATTR_NONNULL int
#else /* ... */
#error "Invalid configuration"
#endif /* !... */
(DCALL LOCAL_DeeType_AccessCachedAttr)(DeeTypeObject *tp_self,
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
                                       Dee_hash_t hash
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
#ifdef LOCAL_IS_FINDINFO
                                       , struct attrinfo *__restrict retinfo
#endif /* LOCAL_IS_FINDINFO */
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
#define LOCAL_err_cant_access_attribute(tp, access)          err_cant_access_attribute_string_len(tp, LOCAL_attr, attrlen, access)
#define LOCAL_err_classmember_requires_1_argument()          err_classmember_requires_1_argument_string_len(tp_self, LOCAL_attr, attrlen)
#define LOCAL_err_classproperty_requires_1_argument()        err_classproperty_requires_1_argument_string_len(tp_self, LOCAL_attr, attrlen)
#define LOCAL_err_classmethod_requires_at_least_1_argument() err_classmethod_requires_at_least_1_argument_string_len(tp_self, LOCAL_attr, attrlen)
#else /* LOCAL_HAS_len */
#define LOCAL_Dee_membercache_slot_matches(item)             streq(item->mcs_name, LOCAL_attr)
#define LOCAL_err_cant_access_attribute(tp, access)          err_cant_access_attribute_string(tp, LOCAL_attr, access)
#define LOCAL_err_classmember_requires_1_argument()          err_classmember_requires_1_argument_string(tp_self, LOCAL_attr)
#define LOCAL_err_classproperty_requires_1_argument()        err_classproperty_requires_1_argument_string(tp_self, LOCAL_attr)
#define LOCAL_err_classmethod_requires_at_least_1_argument() err_classmethod_requires_at_least_1_argument_string(tp_self, LOCAL_attr)
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
	Dee_hash_t i, perturb;
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
		if (rules->alr_decl != NULL &&
		    rules->alr_decl != (DeeObject *)item->mcs_decl)
			break; /* Attribute isn't declared by the requested declarator. */
#endif /* LOCAL_IS_FIND */

		/* Referenced attribute found! */
#ifdef LOCAL_IS_HAS
		Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
		return true;
#elif defined(LOCAL_IS_FINDINFO)
		STATIC_ASSERT(Dee_ATTRINFO_METHOD == MEMBERCACHE_METHOD);
		STATIC_ASSERT(Dee_ATTRINFO_GETSET == MEMBERCACHE_GETSET);
		STATIC_ASSERT(Dee_ATTRINFO_MEMBER == MEMBERCACHE_MEMBER);
		STATIC_ASSERT(Dee_ATTRINFO_ATTR == MEMBERCACHE_ATTRIB);
		STATIC_ASSERT(Dee_ATTRINFO_INSTANCE_METHOD == MEMBERCACHE_INSTANCE_METHOD);
		STATIC_ASSERT(Dee_ATTRINFO_INSTANCE_GETSET == MEMBERCACHE_INSTANCE_GETSET);
		STATIC_ASSERT(Dee_ATTRINFO_INSTANCE_MEMBER == MEMBERCACHE_INSTANCE_MEMBER);
		STATIC_ASSERT(Dee_ATTRINFO_INSTANCE_ATTR == MEMBERCACHE_INSTANCE_ATTRIB);
		retinfo->ai_type = type;
		retinfo->ai_decl = (DeeObject *)item->mcs_decl;
		switch (type) {

#ifdef LOCAL_IS_CLASS
		case MEMBERCACHE_INSTANCE_ATTRIB:
#endif /* LOCAL_IS_CLASS */
		case MEMBERCACHE_ATTRIB:
			retinfo->ai_value.v_attr = item->mcs_attrib.a_attr;
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			break;

			/* Because the cache only stores copies of method/getset/member items,
			 * we have to find the address of the original declaration, since the
			 * cache's only remains valid until the lock is released.
			 *
			 * This overhead is OK because FindAttrInfo isn't called during normal
			 * code execution, but is instead used by _hostasm and AST optimization,
			 * and overall, doing it this was is still faster that not using the
			 * cache at all! */
#ifdef LOCAL_IS_CLASS
		case MEMBERCACHE_METHOD: {
			struct Dee_type_method memb;
			struct Dee_type_method const *iter;
			memcpy(&memb, &item->mcs_method, sizeof(memb));
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			iter = ((DeeTypeObject *)retinfo->ai_decl)->tp_class_methods;
			ASSERT(iter);
			ASSERT(iter->m_name);
			while (memcmp(iter, &memb, sizeof(memb)) != 0) {
				++iter;
				ASSERT(iter->m_name);
			}
			retinfo->ai_value.v_method = iter;
		}	break;

		case MEMBERCACHE_GETSET: {
			struct Dee_type_getset memb;
			struct Dee_type_getset const *iter;
			memcpy(&memb, &item->mcs_getset, sizeof(memb));
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			iter = ((DeeTypeObject *)retinfo->ai_decl)->tp_class_getsets;
			ASSERT(iter);
			ASSERT(iter->gs_name);
			while (memcmp(iter, &memb, sizeof(memb)) != 0) {
				++iter;
				ASSERT(iter->gs_name);
			}
			retinfo->ai_value.v_getset = iter;
		}	break;

		case MEMBERCACHE_MEMBER: {
			struct Dee_type_member memb;
			struct Dee_type_member const *iter;
			memcpy(&memb, &item->mcs_member, sizeof(memb));
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			iter = ((DeeTypeObject *)retinfo->ai_decl)->tp_class_members;
			ASSERT(iter);
			ASSERT(iter->m_name);
			while (memcmp(iter, &memb, sizeof(memb)) != 0) {
				++iter;
				ASSERT(iter->m_name);
			}
			retinfo->ai_value.v_member = iter;
		}	break;
#endif /* LOCAL_IS_CLASS */

#ifdef LOCAL_IS_CLASS
		case MEMBERCACHE_INSTANCE_METHOD:
#else /* LOCAL_IS_CLASS */
		case MEMBERCACHE_METHOD:
#endif /* !LOCAL_IS_CLASS */
		{
			struct Dee_type_method memb;
			struct Dee_type_method const *iter;
			memcpy(&memb, &item->mcs_method, sizeof(memb));
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			iter = ((DeeTypeObject *)retinfo->ai_decl)->tp_methods;
			ASSERT(iter);
			ASSERT(iter->m_name);
			while (memcmp(iter, &memb, sizeof(memb)) != 0) {
				++iter;
				ASSERT(iter->m_name);
			}
			retinfo->ai_value.v_method = iter;
		}	break;

#ifdef LOCAL_IS_CLASS
		case MEMBERCACHE_INSTANCE_GETSET:
#else /* LOCAL_IS_CLASS */
		case MEMBERCACHE_GETSET:
#endif /* !LOCAL_IS_CLASS */
		{
			struct Dee_type_getset memb;
			struct Dee_type_getset const *iter;
			memcpy(&memb, &item->mcs_getset, sizeof(memb));
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			iter = ((DeeTypeObject *)retinfo->ai_decl)->tp_getsets;
			ASSERT(iter);
			ASSERT(iter->gs_name);
			while (memcmp(iter, &memb, sizeof(memb)) != 0) {
				++iter;
				ASSERT(iter->gs_name);
			}
			retinfo->ai_value.v_getset = iter;
		}	break;

#ifdef LOCAL_IS_CLASS
		case MEMBERCACHE_INSTANCE_MEMBER:
#else /* LOCAL_IS_CLASS */
		case MEMBERCACHE_MEMBER:
#endif /* !LOCAL_IS_CLASS */
		{
			struct Dee_type_member memb;
			struct Dee_type_member const *iter;
			memcpy(&memb, &item->mcs_member, sizeof(memb));
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
			iter = ((DeeTypeObject *)retinfo->ai_decl)->tp_members;
			ASSERT(iter);
			ASSERT(iter->m_name);
			while (memcmp(iter, &memb, sizeof(memb)) != 0) {
				++iter;
				ASSERT(iter->m_name);
			}
			retinfo->ai_value.v_member = iter;
		}	break;

		default: __builtin_unreachable();
		}
		return true;
#else /* ... */
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
				if (DeeObject_AssertTypeOrAbstract(LOCAL_argv[0], decl))
					goto err;
#define NEED_err
#endif /* LOCAL_MUST_ASSERT_ARGC */
				return LOCAL_invoke_dkwobjmethod((dkwobjmethod_t)func, decl);
			}
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
#ifdef LOCAL_MUST_ASSERT_ARGC
			if (DeeObject_AssertTypeOrAbstract(LOCAL_argv[0], decl))
				goto err;
#define NEED_err
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
				if unlikely(DeeObject_AssertTypeOrAbstract(thisarg, decl)) {
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
			if unlikely(DeeObject_AssertTypeOrAbstract(LOCAL_argv[0], decl)) {
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
				if (DeeObject_AssertTypeOrAbstract(LOCAL_argv[0], decl))
					goto err;
#define NEED_err
#endif /* LOCAL_MUST_ASSERT_ARGC */
				return LOCAL_invoke_dkwobjmethod((dkwobjmethod_t)func, decl);
#endif /* !LOCAL_IS_GET */
			}
			Dee_membercache_releasetable(&tp_self->LOCAL_tp_cache, table);
#ifdef LOCAL_IS_GET
			return DeeClsMethod_New(decl, func);
#else /* LOCAL_IS_GET */
#ifdef LOCAL_MUST_ASSERT_ARGC
			if (DeeObject_AssertTypeOrAbstract(LOCAL_argv[0], decl))
				goto err;
#define NEED_err
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
				if unlikely(DeeObject_AssertTypeOrAbstract(thisarg, decl)) {
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
			if unlikely(DeeObject_AssertTypeOrAbstract(thisarg, decl)) {
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
				return Dee_BOUND_NO;
			temp = (*getter)(LOCAL_self);
			if unlikely(!temp) {
				if (DeeError_Catch(&DeeError_UnboundAttribute))
					return Dee_BOUND_NO;
				goto err;
#define NEED_err
			}
			Dee_Decref(temp);
			return Dee_BOUND_YES;
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
			{
				bool bound = type_member_bound((struct type_member const *)&buf, LOCAL_self);
				return Dee_BOUND_FROMBOOL(bound);
			}
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
#endif /* !... */

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
	return Dee_BOUND_MISSING;
#elif defined(LOCAL_IS_HAS) || defined(LOCAL_IS_FINDINFO)
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
	err_keywords_func_not_accepted_string_len(tp_self, LOCAL_attr, attrlen, kw);
#else /* LOCAL_HAS_len */
	err_keywords_func_not_accepted_string(tp_self, LOCAL_attr, kw);
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
#elif defined(LOCAL_IS_BOUND)
	return Dee_BOUND_ERR;
#elif defined(LOCAL_IS_DEL) || defined(LOCAL_IS_SET) || defined(LOCAL_IS_SET_BASIC)
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
#undef LOCAL_IS_FIND
#undef LOCAL_IS_FINDINFO
#undef LOCAL_HAS_self
#undef LOCAL_tp_cache

DECL_END

#undef DEFINE_DeeType_GetCachedAttrStringHash
#undef DEFINE_DeeType_GetCachedAttrStringLenHash
#undef DEFINE_DeeType_GetCachedClassAttrStringHash
#undef DEFINE_DeeType_GetCachedClassAttrStringLenHash
#undef DEFINE_DeeType_GetCachedInstanceAttrStringHash
#undef DEFINE_DeeType_GetCachedInstanceAttrStringLenHash
#undef DEFINE_DeeType_BoundCachedAttrStringHash
#undef DEFINE_DeeType_BoundCachedAttrStringLenHash
#undef DEFINE_DeeType_BoundCachedClassAttrStringHash
#undef DEFINE_DeeType_BoundCachedClassAttrStringLenHash
#undef DEFINE_DeeType_BoundCachedInstanceAttrStringHash
#undef DEFINE_DeeType_BoundCachedInstanceAttrStringLenHash
#undef DEFINE_DeeType_HasCachedAttrStringHash
#undef DEFINE_DeeType_HasCachedAttrStringLenHash
#undef DEFINE_DeeType_HasCachedClassAttrStringHash
#undef DEFINE_DeeType_HasCachedClassAttrStringLenHash
#undef DEFINE_DeeType_DelCachedAttrStringHash
#undef DEFINE_DeeType_DelCachedAttrStringLenHash
#undef DEFINE_DeeType_DelCachedClassAttrStringHash
#undef DEFINE_DeeType_DelCachedClassAttrStringLenHash
#undef DEFINE_DeeType_DelCachedInstanceAttrStringHash
#undef DEFINE_DeeType_DelCachedInstanceAttrStringLenHash
#undef DEFINE_DeeType_SetCachedAttrStringHash
#undef DEFINE_DeeType_SetCachedAttrStringLenHash
#undef DEFINE_DeeType_SetCachedClassAttrStringHash
#undef DEFINE_DeeType_SetCachedClassAttrStringLenHash
#undef DEFINE_DeeType_SetCachedInstanceAttrStringHash
#undef DEFINE_DeeType_SetCachedInstanceAttrStringLenHash
#undef DEFINE_DeeType_SetBasicCachedAttrStringHash
#undef DEFINE_DeeType_SetBasicCachedAttrStringLenHash
#undef DEFINE_DeeType_SetBasicCachedClassAttrStringHash
#undef DEFINE_DeeType_SetBasicCachedClassAttrStringLenHash
#undef DEFINE_DeeType_SetBasicCachedInstanceAttrStringHash
#undef DEFINE_DeeType_SetBasicCachedInstanceAttrStringLenHash
#undef DEFINE_DeeType_CallCachedAttrStringHash
#undef DEFINE_DeeType_CallCachedAttrStringLenHash
#undef DEFINE_DeeType_CallCachedClassAttrStringHash
#undef DEFINE_DeeType_CallCachedClassAttrStringLenHash
#undef DEFINE_DeeType_CallCachedInstanceAttrStringHash
#undef DEFINE_DeeType_CallCachedInstanceAttrStringLenHash
#undef DEFINE_DeeType_CallCachedAttrStringHashKw
#undef DEFINE_DeeType_CallCachedAttrStringLenHashKw
#undef DEFINE_DeeType_CallCachedClassAttrStringHashKw
#undef DEFINE_DeeType_CallCachedClassAttrStringLenHashKw
#undef DEFINE_DeeType_CallCachedInstanceAttrStringHashKw
#undef DEFINE_DeeType_CallCachedInstanceAttrStringLenHashKw
#undef DEFINE_DeeType_CallCachedAttrStringHashTuple
#undef DEFINE_DeeType_CallCachedAttrStringLenHashTuple
#undef DEFINE_DeeType_CallCachedClassAttrStringHashTuple
#undef DEFINE_DeeType_CallCachedClassAttrStringLenHashTuple
#undef DEFINE_DeeType_CallCachedInstanceAttrStringHashTuple
#undef DEFINE_DeeType_CallCachedInstanceAttrStringLenHashTuple
#undef DEFINE_DeeType_CallCachedAttrStringHashTupleKw
#undef DEFINE_DeeType_CallCachedAttrStringLenHashTupleKw
#undef DEFINE_DeeType_CallCachedClassAttrStringHashTupleKw
#undef DEFINE_DeeType_CallCachedClassAttrStringLenHashTupleKw
#undef DEFINE_DeeType_CallCachedInstanceAttrStringHashTupleKw
#undef DEFINE_DeeType_CallCachedInstanceAttrStringLenHashTupleKw
#undef DEFINE_DeeType_VCallCachedAttrStringHashf
#undef DEFINE_DeeType_VCallCachedAttrStringLenHashf
#undef DEFINE_DeeType_VCallCachedClassAttrStringHashf
#undef DEFINE_DeeType_VCallCachedClassAttrStringLenHashf
#undef DEFINE_DeeType_VCallCachedInstanceAttrStringHashf
#undef DEFINE_DeeType_VCallCachedInstanceAttrStringLenHashf
#undef DEFINE_DeeType_FindCachedAttrInfoStringHash
#undef DEFINE_DeeType_FindCachedClassAttrInfoStringHash
#undef DEFINE_DeeType_FindCachedAttrInfoStringLenHash
#undef DEFINE_DeeType_FindCachedClassAttrInfoStringLenHash
#undef DEFINE_DeeType_FindCachedAttr
#undef DEFINE_DeeType_FindCachedClassAttr
