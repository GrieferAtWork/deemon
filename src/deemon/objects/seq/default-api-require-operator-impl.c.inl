/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#include "default-api.c"
//#define DEFINE_DeeType_RequireSeqOperatorBool
//#define DEFINE_DeeType_RequireSeqOperatorIter
//#define DEFINE_DeeType_RequireSeqOperatorSizeOb
//#define DEFINE_DeeType_RequireSeqOperatorContains
//#define DEFINE_DeeType_RequireSeqOperatorGetItem
//#define DEFINE_DeeType_RequireSeqOperatorDelItem
//#define DEFINE_DeeType_RequireSeqOperatorSetItem
//#define DEFINE_DeeType_RequireSeqOperatorGetRange
//#define DEFINE_DeeType_RequireSeqOperatorDelRange
//#define DEFINE_DeeType_RequireSeqOperatorSetRange
//#define DEFINE_DeeType_RequireSeqOperatorForeach
//#define DEFINE_DeeType_RequireSeqOperatorForeachPair
//#define DEFINE_DeeType_RequireSeqOperatorEnumerate
//#define DEFINE_DeeType_RequireSeqOperatorEnumerateIndex
//#define DEFINE_DeeType_RequireSeqOperatorIterKeys
//#define DEFINE_DeeType_RequireSeqOperatorBoundItem
//#define DEFINE_DeeType_RequireSeqOperatorHasItem
//#define DEFINE_DeeType_RequireSeqOperatorSize
//#define DEFINE_DeeType_RequireSeqOperatorSizeFast
//#define DEFINE_DeeType_RequireSeqOperatorGetItemIndex
//#define DEFINE_DeeType_RequireSeqOperatorDelItemIndex
//#define DEFINE_DeeType_RequireSeqOperatorSetItemIndex
//#define DEFINE_DeeType_RequireSeqOperatorBoundItemIndex
//#define DEFINE_DeeType_RequireSeqOperatorHasItemIndex
//#define DEFINE_DeeType_RequireSeqOperatorGetRangeIndex
//#define DEFINE_DeeType_RequireSeqOperatorDelRangeIndex
//#define DEFINE_DeeType_RequireSeqOperatorSetRangeIndex
//#define DEFINE_DeeType_RequireSeqOperatorGetRangeIndexN
//#define DEFINE_DeeType_RequireSeqOperatorDelRangeIndexN
//#define DEFINE_DeeType_RequireSeqOperatorSetRangeIndexN
//#define DEFINE_DeeType_RequireSeqOperatorTryGetItem
//#define DEFINE_DeeType_RequireSeqOperatorTryGetItemIndex
//#define DEFINE_DeeType_RequireSeqOperatorHash
//#define DEFINE_DeeType_RequireSeqOperatorCompareEq
//#define DEFINE_DeeType_RequireSeqOperatorCompare
//#define DEFINE_DeeType_RequireSeqOperatorTryCompareEq
//#define DEFINE_DeeType_RequireSeqOperatorEq
//#define DEFINE_DeeType_RequireSeqOperatorNe
//#define DEFINE_DeeType_RequireSeqOperatorLo
//#define DEFINE_DeeType_RequireSeqOperatorLe
//#define DEFINE_DeeType_RequireSeqOperatorGr
//#define DEFINE_DeeType_RequireSeqOperatorGe
//#define DEFINE_DeeType_RequireSeqOperatorInplaceAdd
//#define DEFINE_DeeType_RequireSeqOperatorInplaceMul

#define DEFINE_DeeType_RequireSetOperatorIter
//#define DEFINE_DeeType_RequireSetOperatorForeach
//#define DEFINE_DeeType_RequireSetOperatorSize
//#define DEFINE_DeeType_RequireSetOperatorSizeOb
//#define DEFINE_DeeType_RequireSetOperatorHash
//#define DEFINE_DeeType_RequireSetOperatorCompareEq
//#define DEFINE_DeeType_RequireSetOperatorTryCompareEq
//#define DEFINE_DeeType_RequireSetOperatorEq
//#define DEFINE_DeeType_RequireSetOperatorNe
//#define DEFINE_DeeType_RequireSetOperatorLo
//#define DEFINE_DeeType_RequireSetOperatorLe
//#define DEFINE_DeeType_RequireSetOperatorGr
//#define DEFINE_DeeType_RequireSetOperatorGe
//#define DEFINE_DeeType_RequireSetOperatorInv
//#define DEFINE_DeeType_RequireSetOperatorAdd
//#define DEFINE_DeeType_RequireSetOperatorSub
//#define DEFINE_DeeType_RequireSetOperatorAnd
//#define DEFINE_DeeType_RequireSetOperatorXor
//#define DEFINE_DeeType_RequireSetOperatorInplaceAdd
//#define DEFINE_DeeType_RequireSetOperatorInplaceSub
//#define DEFINE_DeeType_RequireSetOperatorInplaceAnd
//#define DEFINE_DeeType_RequireSetOperatorInplaceXor

//#define DEFINE_DeeType_RequireMapOperatorContains
//#define DEFINE_DeeType_RequireMapOperatorGetItem
//#define DEFINE_DeeType_RequireMapOperatorDelItem
//#define DEFINE_DeeType_RequireMapOperatorSetItem
//#define DEFINE_DeeType_RequireMapOperatorEnumerate
//#define DEFINE_DeeType_RequireMapOperatorEnumerateIndex
//#define DEFINE_DeeType_RequireMapOperatorBoundItem
//#define DEFINE_DeeType_RequireMapOperatorHasItem
//#define DEFINE_DeeType_RequireMapOperatorGetItemIndex
//#define DEFINE_DeeType_RequireMapOperatorDelItemIndex
//#define DEFINE_DeeType_RequireMapOperatorSetItemIndex
//#define DEFINE_DeeType_RequireMapOperatorBoundItemIndex
//#define DEFINE_DeeType_RequireMapOperatorHasItemIndex
//#define DEFINE_DeeType_RequireMapOperatorTryGetItem
//#define DEFINE_DeeType_RequireMapOperatorTryGetItemIndex
//#define DEFINE_DeeType_RequireMapOperatorTryGetItemStringHash
//#define DEFINE_DeeType_RequireMapOperatorGetItemStringHash
//#define DEFINE_DeeType_RequireMapOperatorDelItemStringHash
//#define DEFINE_DeeType_RequireMapOperatorSetItemStringHash
//#define DEFINE_DeeType_RequireMapOperatorBoundItemStringHash
//#define DEFINE_DeeType_RequireMapOperatorHasItemStringHash
//#define DEFINE_DeeType_RequireMapOperatorTryGetItemStringLenHash
//#define DEFINE_DeeType_RequireMapOperatorGetItemStringLenHash
//#define DEFINE_DeeType_RequireMapOperatorDelItemStringLenHash
//#define DEFINE_DeeType_RequireMapOperatorSetItemStringLenHash
//#define DEFINE_DeeType_RequireMapOperatorBoundItemStringLenHash
//#define DEFINE_DeeType_RequireMapOperatorHasItemStringLenHash
//#define DEFINE_DeeType_RequireMapOperatorCompareEq
//#define DEFINE_DeeType_RequireMapOperatorTryCompareEq
//#define DEFINE_DeeType_RequireMapOperatorEq
//#define DEFINE_DeeType_RequireMapOperatorNe
//#define DEFINE_DeeType_RequireMapOperatorLo
//#define DEFINE_DeeType_RequireMapOperatorLe
//#define DEFINE_DeeType_RequireMapOperatorGr
//#define DEFINE_DeeType_RequireMapOperatorGe
//#define DEFINE_DeeType_RequireMapOperatorAdd
//#define DEFINE_DeeType_RequireMapOperatorSub
//#define DEFINE_DeeType_RequireMapOperatorAnd
//#define DEFINE_DeeType_RequireMapOperatorXor
//#define DEFINE_DeeType_RequireMapOperatorInplaceAdd
//#define DEFINE_DeeType_RequireMapOperatorInplaceSub
//#define DEFINE_DeeType_RequireMapOperatorInplaceAnd
//#define DEFINE_DeeType_RequireMapOperatorInplaceXor

#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeType_RequireSeqOperatorBool) +                    \
     defined(DEFINE_DeeType_RequireSeqOperatorIter) +                    \
     defined(DEFINE_DeeType_RequireSeqOperatorSizeOb) +                  \
     defined(DEFINE_DeeType_RequireSeqOperatorContains) +                \
     defined(DEFINE_DeeType_RequireSeqOperatorGetItem) +                 \
     defined(DEFINE_DeeType_RequireSeqOperatorDelItem) +                 \
     defined(DEFINE_DeeType_RequireSeqOperatorSetItem) +                 \
     defined(DEFINE_DeeType_RequireSeqOperatorGetRange) +                \
     defined(DEFINE_DeeType_RequireSeqOperatorDelRange) +                \
     defined(DEFINE_DeeType_RequireSeqOperatorSetRange) +                \
     defined(DEFINE_DeeType_RequireSeqOperatorForeach) +                 \
     defined(DEFINE_DeeType_RequireSeqOperatorForeachPair) +             \
     defined(DEFINE_DeeType_RequireSeqOperatorEnumerate) +               \
     defined(DEFINE_DeeType_RequireSeqOperatorEnumerateIndex) +          \
     defined(DEFINE_DeeType_RequireSeqOperatorIterKeys) +                \
     defined(DEFINE_DeeType_RequireSeqOperatorBoundItem) +               \
     defined(DEFINE_DeeType_RequireSeqOperatorHasItem) +                 \
     defined(DEFINE_DeeType_RequireSeqOperatorSize) +                    \
     defined(DEFINE_DeeType_RequireSeqOperatorSizeFast) +                \
     defined(DEFINE_DeeType_RequireSeqOperatorGetItemIndex) +            \
     defined(DEFINE_DeeType_RequireSeqOperatorDelItemIndex) +            \
     defined(DEFINE_DeeType_RequireSeqOperatorSetItemIndex) +            \
     defined(DEFINE_DeeType_RequireSeqOperatorBoundItemIndex) +          \
     defined(DEFINE_DeeType_RequireSeqOperatorHasItemIndex) +            \
     defined(DEFINE_DeeType_RequireSeqOperatorGetRangeIndex) +           \
     defined(DEFINE_DeeType_RequireSeqOperatorDelRangeIndex) +           \
     defined(DEFINE_DeeType_RequireSeqOperatorSetRangeIndex) +           \
     defined(DEFINE_DeeType_RequireSeqOperatorGetRangeIndexN) +          \
     defined(DEFINE_DeeType_RequireSeqOperatorDelRangeIndexN) +          \
     defined(DEFINE_DeeType_RequireSeqOperatorSetRangeIndexN) +          \
     defined(DEFINE_DeeType_RequireSeqOperatorTryGetItem) +              \
     defined(DEFINE_DeeType_RequireSeqOperatorTryGetItemIndex) +         \
     defined(DEFINE_DeeType_RequireSeqOperatorHash) +                    \
     defined(DEFINE_DeeType_RequireSeqOperatorCompareEq) +               \
     defined(DEFINE_DeeType_RequireSeqOperatorCompare) +                 \
     defined(DEFINE_DeeType_RequireSeqOperatorTryCompareEq) +            \
     defined(DEFINE_DeeType_RequireSeqOperatorEq) +                      \
     defined(DEFINE_DeeType_RequireSeqOperatorNe) +                      \
     defined(DEFINE_DeeType_RequireSeqOperatorLo) +                      \
     defined(DEFINE_DeeType_RequireSeqOperatorLe) +                      \
     defined(DEFINE_DeeType_RequireSeqOperatorGr) +                      \
     defined(DEFINE_DeeType_RequireSeqOperatorGe) +                      \
     defined(DEFINE_DeeType_RequireSeqOperatorInplaceAdd) +              \
     defined(DEFINE_DeeType_RequireSeqOperatorInplaceMul) +              \
     defined(DEFINE_DeeType_RequireSetOperatorIter) +                    \
     defined(DEFINE_DeeType_RequireSetOperatorForeach) +                 \
     defined(DEFINE_DeeType_RequireSetOperatorSize) +                    \
     defined(DEFINE_DeeType_RequireSetOperatorSizeOb) +                  \
     defined(DEFINE_DeeType_RequireSetOperatorHash) +                    \
     defined(DEFINE_DeeType_RequireSetOperatorCompareEq) +               \
     defined(DEFINE_DeeType_RequireSetOperatorTryCompareEq) +            \
     defined(DEFINE_DeeType_RequireSetOperatorEq) +                      \
     defined(DEFINE_DeeType_RequireSetOperatorNe) +                      \
     defined(DEFINE_DeeType_RequireSetOperatorLo) +                      \
     defined(DEFINE_DeeType_RequireSetOperatorLe) +                      \
     defined(DEFINE_DeeType_RequireSetOperatorGr) +                      \
     defined(DEFINE_DeeType_RequireSetOperatorGe) +                      \
     defined(DEFINE_DeeType_RequireSetOperatorInv) +                     \
     defined(DEFINE_DeeType_RequireSetOperatorAdd) +                     \
     defined(DEFINE_DeeType_RequireSetOperatorSub) +                     \
     defined(DEFINE_DeeType_RequireSetOperatorAnd) +                     \
     defined(DEFINE_DeeType_RequireSetOperatorXor) +                     \
     defined(DEFINE_DeeType_RequireSetOperatorInplaceAdd) +              \
     defined(DEFINE_DeeType_RequireSetOperatorInplaceSub) +              \
     defined(DEFINE_DeeType_RequireSetOperatorInplaceAnd) +              \
     defined(DEFINE_DeeType_RequireSetOperatorInplaceXor) +              \
     defined(DEFINE_DeeType_RequireMapOperatorContains) +                \
     defined(DEFINE_DeeType_RequireMapOperatorGetItem) +                 \
     defined(DEFINE_DeeType_RequireMapOperatorDelItem) +                 \
     defined(DEFINE_DeeType_RequireMapOperatorSetItem) +                 \
     defined(DEFINE_DeeType_RequireMapOperatorEnumerate) +               \
     defined(DEFINE_DeeType_RequireMapOperatorEnumerateIndex) +          \
     defined(DEFINE_DeeType_RequireMapOperatorBoundItem) +               \
     defined(DEFINE_DeeType_RequireMapOperatorHasItem) +                 \
     defined(DEFINE_DeeType_RequireMapOperatorGetItemIndex) +            \
     defined(DEFINE_DeeType_RequireMapOperatorDelItemIndex) +            \
     defined(DEFINE_DeeType_RequireMapOperatorSetItemIndex) +            \
     defined(DEFINE_DeeType_RequireMapOperatorBoundItemIndex) +          \
     defined(DEFINE_DeeType_RequireMapOperatorHasItemIndex) +            \
     defined(DEFINE_DeeType_RequireMapOperatorTryGetItem) +              \
     defined(DEFINE_DeeType_RequireMapOperatorTryGetItemIndex) +         \
     defined(DEFINE_DeeType_RequireMapOperatorTryGetItemStringHash) +    \
     defined(DEFINE_DeeType_RequireMapOperatorGetItemStringHash) +       \
     defined(DEFINE_DeeType_RequireMapOperatorDelItemStringHash) +       \
     defined(DEFINE_DeeType_RequireMapOperatorSetItemStringHash) +       \
     defined(DEFINE_DeeType_RequireMapOperatorBoundItemStringHash) +     \
     defined(DEFINE_DeeType_RequireMapOperatorHasItemStringHash) +       \
     defined(DEFINE_DeeType_RequireMapOperatorTryGetItemStringLenHash) + \
     defined(DEFINE_DeeType_RequireMapOperatorGetItemStringLenHash) +    \
     defined(DEFINE_DeeType_RequireMapOperatorDelItemStringLenHash) +    \
     defined(DEFINE_DeeType_RequireMapOperatorSetItemStringLenHash) +    \
     defined(DEFINE_DeeType_RequireMapOperatorBoundItemStringLenHash) +  \
     defined(DEFINE_DeeType_RequireMapOperatorHasItemStringLenHash) +    \
     defined(DEFINE_DeeType_RequireMapOperatorCompareEq) +               \
     defined(DEFINE_DeeType_RequireMapOperatorTryCompareEq) +            \
     defined(DEFINE_DeeType_RequireMapOperatorEq) +                      \
     defined(DEFINE_DeeType_RequireMapOperatorNe) +                      \
     defined(DEFINE_DeeType_RequireMapOperatorLo) +                      \
     defined(DEFINE_DeeType_RequireMapOperatorLe) +                      \
     defined(DEFINE_DeeType_RequireMapOperatorGr) +                      \
     defined(DEFINE_DeeType_RequireMapOperatorGe) +                      \
     defined(DEFINE_DeeType_RequireMapOperatorAdd) +                     \
     defined(DEFINE_DeeType_RequireMapOperatorSub) +                     \
     defined(DEFINE_DeeType_RequireMapOperatorAnd) +                     \
     defined(DEFINE_DeeType_RequireMapOperatorXor) +                     \
     defined(DEFINE_DeeType_RequireMapOperatorInplaceAdd) +              \
     defined(DEFINE_DeeType_RequireMapOperatorInplaceSub) +              \
     defined(DEFINE_DeeType_RequireMapOperatorInplaceAnd) +              \
     defined(DEFINE_DeeType_RequireMapOperatorInplaceXor)) != 1
#error "Must #define exactly one of these macros"
#endif /* DEFINE_DeeType_RequireSeq... */

#ifdef LOCAL_FOR_VARIANTS
#define PP_PRIVATE_CAT5(a, b, c, d, e) a##b##c##d##e
#define PP_PRIVATE_CAT4(a, b, c, d) a##b##c##d
#define PP_PRIVATE_CAT3(a, b, c) a##b##c
#define PP_PRIVATE_CAT2(a, b) a##b
#define PP_CAT5(a, b, c, d, e) PP_PRIVATE_CAT5(a, b, c, d, e)
#define PP_CAT4(a, b, c, d) PP_PRIVATE_CAT4(a, b, c, d)
#define PP_CAT3(a, b, c) PP_PRIVATE_CAT3(a, b, c)
#define PP_CAT2(a, b) PP_PRIVATE_CAT2(a, b)
#define LOCAL_FOR_OPTIMIZE
#include "default-api.h"
#endif /* LOCAL_FOR_VARIANTS */


DECL_BEGIN

#ifdef DEFINE_DeeType_RequireSeqOperatorBool
#define LOCAL_operator_foo     operator_bool
#define LOCAL_OperatorFoo      OperatorBool
#elif defined(DEFINE_DeeType_RequireSeqOperatorIter)
#define LOCAL_operator_foo     operator_iter
#define LOCAL_OperatorFoo      OperatorIter
#elif defined(DEFINE_DeeType_RequireSeqOperatorSizeOb)
#define LOCAL_operator_foo     operator_sizeob
#define LOCAL_OperatorFoo      OperatorSizeOb
#elif defined(DEFINE_DeeType_RequireSeqOperatorContains)
#define LOCAL_operator_foo     operator_contains
#define LOCAL_OperatorFoo      OperatorContains
#elif defined(DEFINE_DeeType_RequireSeqOperatorGetItem)
#define LOCAL_operator_foo     operator_getitem
#define LOCAL_OperatorFoo      OperatorGetItem
#elif defined(DEFINE_DeeType_RequireSeqOperatorDelItem)
#define LOCAL_operator_foo     operator_delitem
#define LOCAL_OperatorFoo      OperatorDelItem
#elif defined(DEFINE_DeeType_RequireSeqOperatorSetItem)
#define LOCAL_operator_foo     operator_setitem
#define LOCAL_OperatorFoo      OperatorSetItem
#elif defined(DEFINE_DeeType_RequireSeqOperatorGetRange)
#define LOCAL_operator_foo     operator_getrange
#define LOCAL_OperatorFoo      OperatorGetRange
#elif defined(DEFINE_DeeType_RequireSeqOperatorDelRange)
#define LOCAL_operator_foo     operator_delrange
#define LOCAL_OperatorFoo      OperatorDelRange
#elif defined(DEFINE_DeeType_RequireSeqOperatorSetRange)
#define LOCAL_operator_foo     operator_setrange
#define LOCAL_OperatorFoo      OperatorSetRange
#elif defined(DEFINE_DeeType_RequireSeqOperatorForeach)
#define LOCAL_operator_foo     operator_foreach
#define LOCAL_OperatorFoo      OperatorForeach
#elif defined(DEFINE_DeeType_RequireSeqOperatorForeachPair)
#define LOCAL_operator_foo     operator_foreach_pair
#define LOCAL_OperatorFoo      OperatorForeachPair
#elif defined(DEFINE_DeeType_RequireSeqOperatorEnumerate)
#define LOCAL_operator_foo     operator_enumerate
#define LOCAL_OperatorFoo      OperatorEnumerate
#elif defined(DEFINE_DeeType_RequireSeqOperatorEnumerateIndex)
#define LOCAL_operator_foo     operator_enumerate_index
#define LOCAL_OperatorFoo      OperatorEnumerateIndex
#elif defined(DEFINE_DeeType_RequireSeqOperatorIterKeys)
#define LOCAL_operator_foo     operator_iterkeys
#define LOCAL_OperatorFoo      OperatorIterKeys
#elif defined(DEFINE_DeeType_RequireSeqOperatorBoundItem)
#define LOCAL_operator_foo     operator_bounditem
#define LOCAL_OperatorFoo      OperatorBoundItem
#elif defined(DEFINE_DeeType_RequireSeqOperatorHasItem)
#define LOCAL_operator_foo     operator_hasitem
#define LOCAL_OperatorFoo      OperatorHasItem
#elif defined(DEFINE_DeeType_RequireSeqOperatorSize)
#define LOCAL_operator_foo     operator_size
#define LOCAL_OperatorFoo      OperatorSize
#elif defined(DEFINE_DeeType_RequireSeqOperatorSizeFast)
#define LOCAL_operator_foo     operator_size_fast
#define LOCAL_OperatorFoo      OperatorSizeFast
#elif defined(DEFINE_DeeType_RequireSeqOperatorGetItemIndex)
#define LOCAL_operator_foo     operator_getitem_index
#define LOCAL_OperatorFoo      OperatorGetItemIndex
#elif defined(DEFINE_DeeType_RequireSeqOperatorDelItemIndex)
#define LOCAL_operator_foo     operator_delitem_index
#define LOCAL_OperatorFoo      OperatorDelItemIndex
#elif defined(DEFINE_DeeType_RequireSeqOperatorSetItemIndex)
#define LOCAL_operator_foo     operator_setitem_index
#define LOCAL_OperatorFoo      OperatorSetItemIndex
#elif defined(DEFINE_DeeType_RequireSeqOperatorBoundItemIndex)
#define LOCAL_operator_foo     operator_bounditem_index
#define LOCAL_OperatorFoo      OperatorBoundItemIndex
#elif defined(DEFINE_DeeType_RequireSeqOperatorHasItemIndex)
#define LOCAL_operator_foo     operator_hasitem_index
#define LOCAL_OperatorFoo      OperatorHasItemIndex
#elif defined(DEFINE_DeeType_RequireSeqOperatorGetRangeIndex)
#define LOCAL_operator_foo     operator_getrange_index
#define LOCAL_OperatorFoo      OperatorGetRangeIndex
#elif defined(DEFINE_DeeType_RequireSeqOperatorDelRangeIndex)
#define LOCAL_operator_foo     operator_delrange_index
#define LOCAL_OperatorFoo      OperatorDelRangeIndex
#elif defined(DEFINE_DeeType_RequireSeqOperatorSetRangeIndex)
#define LOCAL_operator_foo     operator_setrange_index
#define LOCAL_OperatorFoo      OperatorSetRangeIndex
#elif defined(DEFINE_DeeType_RequireSeqOperatorGetRangeIndexN)
#define LOCAL_operator_foo     operator_getrange_index_n
#define LOCAL_OperatorFoo      OperatorGetRangeIndexN
#elif defined(DEFINE_DeeType_RequireSeqOperatorDelRangeIndexN)
#define LOCAL_operator_foo     operator_delrange_index_n
#define LOCAL_OperatorFoo      OperatorDelRangeIndexN
#elif defined(DEFINE_DeeType_RequireSeqOperatorSetRangeIndexN)
#define LOCAL_operator_foo     operator_setrange_index_n
#define LOCAL_OperatorFoo      OperatorSetRangeIndexN
#elif defined(DEFINE_DeeType_RequireSeqOperatorTryGetItem)
#define LOCAL_operator_foo     operator_trygetitem
#define LOCAL_OperatorFoo      OperatorTryGetItem
#elif defined(DEFINE_DeeType_RequireSeqOperatorTryGetItemIndex)
#define LOCAL_operator_foo     operator_trygetitem_index
#define LOCAL_OperatorFoo      OperatorTryGetItemIndex
#elif defined(DEFINE_DeeType_RequireSeqOperatorHash)
#define LOCAL_operator_foo     operator_hash
#define LOCAL_OperatorFoo      OperatorHash
#elif defined(DEFINE_DeeType_RequireSeqOperatorCompareEq)
#define LOCAL_operator_foo     operator_compare_eq
#define LOCAL_OperatorFoo      OperatorCompareEq
#elif defined(DEFINE_DeeType_RequireSeqOperatorCompare)
#define LOCAL_operator_foo     operator_compare
#define LOCAL_OperatorFoo      OperatorCompare
#elif defined(DEFINE_DeeType_RequireSeqOperatorTryCompareEq)
#define LOCAL_operator_foo     operator_trycompare_eq
#define LOCAL_OperatorFoo      OperatorTryCompareEq
#elif defined(DEFINE_DeeType_RequireSeqOperatorEq)
#define LOCAL_operator_foo     operator_eq
#define LOCAL_OperatorFoo      OperatorEq
#elif defined(DEFINE_DeeType_RequireSeqOperatorNe)
#define LOCAL_operator_foo     operator_ne
#define LOCAL_OperatorFoo      OperatorNe
#elif defined(DEFINE_DeeType_RequireSeqOperatorLo)
#define LOCAL_operator_foo     operator_lo
#define LOCAL_OperatorFoo      OperatorLo
#elif defined(DEFINE_DeeType_RequireSeqOperatorLe)
#define LOCAL_operator_foo     operator_le
#define LOCAL_OperatorFoo      OperatorLe
#elif defined(DEFINE_DeeType_RequireSeqOperatorGr)
#define LOCAL_operator_foo     operator_gr
#define LOCAL_OperatorFoo      OperatorGr
#elif defined(DEFINE_DeeType_RequireSeqOperatorGe)
#define LOCAL_operator_foo     operator_ge
#define LOCAL_OperatorFoo      OperatorGe
#elif defined(DEFINE_DeeType_RequireSeqOperatorInplaceAdd)
#define LOCAL_operator_foo     operator_inplace_add
#define LOCAL_OperatorFoo      OperatorInplaceAdd
#elif defined(DEFINE_DeeType_RequireSeqOperatorInplaceMul)
#define LOCAL_operator_foo     operator_inplace_mul
#define LOCAL_OperatorFoo      OperatorInplaceMul
#elif defined(DEFINE_DeeType_RequireSetOperatorIter)
#define LOCAL_operator_foo     operator_iter
#define LOCAL_OperatorFoo      OperatorIter
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorForeach)
#define LOCAL_operator_foo     operator_foreach
#define LOCAL_OperatorFoo      OperatorForeach
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorSize)
#define LOCAL_operator_foo     operator_size
#define LOCAL_OperatorFoo      OperatorSize
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorSizeOb)
#define LOCAL_operator_foo     operator_sizeob
#define LOCAL_OperatorFoo      OperatorSizeOb
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorHash)
#define LOCAL_operator_foo     operator_hash
#define LOCAL_OperatorFoo      OperatorHash
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorCompareEq)
#define LOCAL_operator_foo     operator_compare_eq
#define LOCAL_OperatorFoo      OperatorCompareEq
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorTryCompareEq)
#define LOCAL_operator_foo     operator_trycompare_eq
#define LOCAL_OperatorFoo      OperatorTryCompareEq
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorEq)
#define LOCAL_operator_foo     operator_eq
#define LOCAL_OperatorFoo      OperatorEq
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorNe)
#define LOCAL_operator_foo     operator_ne
#define LOCAL_OperatorFoo      OperatorNe
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorLo)
#define LOCAL_operator_foo     operator_lo
#define LOCAL_OperatorFoo      OperatorLo
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorLe)
#define LOCAL_operator_foo     operator_le
#define LOCAL_OperatorFoo      OperatorLe
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorGr)
#define LOCAL_operator_foo     operator_gr
#define LOCAL_OperatorFoo      OperatorGr
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorGe)
#define LOCAL_operator_foo     operator_ge
#define LOCAL_OperatorFoo      OperatorGe
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorInv)
#define LOCAL_operator_foo     operator_inv
#define LOCAL_OperatorFoo      OperatorInv
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorAdd)
#define LOCAL_operator_foo     operator_add
#define LOCAL_OperatorFoo      OperatorAdd
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorSub)
#define LOCAL_operator_foo     operator_sub
#define LOCAL_OperatorFoo      OperatorSub
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorAnd)
#define LOCAL_operator_foo     operator_and
#define LOCAL_OperatorFoo      OperatorAnd
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorXor)
#define LOCAL_operator_foo     operator_xor
#define LOCAL_OperatorFoo      OperatorXor
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorInplaceAdd)
#define LOCAL_operator_foo     operator_inplace_add
#define LOCAL_OperatorFoo      OperatorInplaceAdd
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorInplaceSub)
#define LOCAL_operator_foo     operator_inplace_sub
#define LOCAL_OperatorFoo      OperatorInplaceSub
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorInplaceAnd)
#define LOCAL_operator_foo     operator_inplace_and
#define LOCAL_OperatorFoo      OperatorInplaceAnd
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetOperatorInplaceXor)
#define LOCAL_operator_foo     operator_inplace_xor
#define LOCAL_OperatorFoo      OperatorInplaceXor
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireMapOperatorContains)
#define LOCAL_operator_foo     operator_contains
#define LOCAL_OperatorFoo      OperatorContains
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorGetItem)
#define LOCAL_operator_foo     operator_getitem
#define LOCAL_OperatorFoo      OperatorGetItem
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorDelItem)
#define LOCAL_operator_foo     operator_delitem
#define LOCAL_OperatorFoo      OperatorDelItem
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorSetItem)
#define LOCAL_operator_foo     operator_setitem
#define LOCAL_OperatorFoo      OperatorSetItem
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorEnumerate)
#define LOCAL_operator_foo     operator_enumerate
#define LOCAL_OperatorFoo      OperatorEnumerate
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorEnumerateIndex)
#define LOCAL_operator_foo     operator_enumerate_index
#define LOCAL_OperatorFoo      OperatorEnumerateIndex
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorBoundItem)
#define LOCAL_operator_foo     operator_bounditem
#define LOCAL_OperatorFoo      OperatorBoundItem
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorHasItem)
#define LOCAL_operator_foo     operator_hasitem
#define LOCAL_OperatorFoo      OperatorHasItem
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorGetItemIndex)
#define LOCAL_operator_foo     operator_getitem_index
#define LOCAL_OperatorFoo      OperatorGetItemIndex
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorDelItemIndex)
#define LOCAL_operator_foo     operator_delitem_index
#define LOCAL_OperatorFoo      OperatorDelItemIndex
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorSetItemIndex)
#define LOCAL_operator_foo     operator_setitem_index
#define LOCAL_OperatorFoo      OperatorSetItemIndex
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorBoundItemIndex)
#define LOCAL_operator_foo     operator_bounditem_index
#define LOCAL_OperatorFoo      OperatorBoundItemIndex
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorHasItemIndex)
#define LOCAL_operator_foo     operator_hasitem_index
#define LOCAL_OperatorFoo      OperatorHasItemIndex
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorTryGetItem)
#define LOCAL_operator_foo     operator_trygetitem
#define LOCAL_OperatorFoo      OperatorTryGetItem
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorTryGetItemIndex)
#define LOCAL_operator_foo     operator_trygetitem_index
#define LOCAL_OperatorFoo      OperatorTryGetItemIndex
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorTryGetItemStringHash)
#define LOCAL_operator_foo     operator_trygetitem_string_hash
#define LOCAL_OperatorFoo      OperatorTryGetItemStringHash
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorGetItemStringHash)
#define LOCAL_operator_foo     operator_getitem_string_hash
#define LOCAL_OperatorFoo      OperatorGetItemStringHash
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorDelItemStringHash)
#define LOCAL_operator_foo     operator_delitem_string_hash
#define LOCAL_OperatorFoo      OperatorDelItemStringHash
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorSetItemStringHash)
#define LOCAL_operator_foo     operator_setitem_string_hash
#define LOCAL_OperatorFoo      OperatorSetItemStringHash
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorBoundItemStringHash)
#define LOCAL_operator_foo     operator_bounditem_string_hash
#define LOCAL_OperatorFoo      OperatorBoundItemStringHash
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorHasItemStringHash)
#define LOCAL_operator_foo     operator_hasitem_string_hash
#define LOCAL_OperatorFoo      OperatorHasItemStringHash
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorTryGetItemStringLenHash)
#define LOCAL_operator_foo     operator_trygetitem_string_len_hash
#define LOCAL_OperatorFoo      OperatorTryGetItemStringLenHash
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorGetItemStringLenHash)
#define LOCAL_operator_foo     operator_getitem_string_len_hash
#define LOCAL_OperatorFoo      OperatorGetItemStringLenHash
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorDelItemStringLenHash)
#define LOCAL_operator_foo     operator_delitem_string_len_hash
#define LOCAL_OperatorFoo      OperatorDelItemStringLenHash
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorSetItemStringLenHash)
#define LOCAL_operator_foo     operator_setitem_string_len_hash
#define LOCAL_OperatorFoo      OperatorSetItemStringLenHash
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorBoundItemStringLenHash)
#define LOCAL_operator_foo     operator_bounditem_string_len_hash
#define LOCAL_OperatorFoo      OperatorBoundItemStringLenHash
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorHasItemStringLenHash)
#define LOCAL_operator_foo     operator_hasitem_string_len_hash
#define LOCAL_OperatorFoo      OperatorHasItemStringLenHash
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorCompareEq)
#define LOCAL_operator_foo     operator_compare_eq
#define LOCAL_OperatorFoo      OperatorCompareEq
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorTryCompareEq)
#define LOCAL_operator_foo     operator_trycompare_eq
#define LOCAL_OperatorFoo      OperatorTryCompareEq
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorEq)
#define LOCAL_operator_foo     operator_eq
#define LOCAL_OperatorFoo      OperatorEq
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorNe)
#define LOCAL_operator_foo     operator_ne
#define LOCAL_OperatorFoo      OperatorNe
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorLo)
#define LOCAL_operator_foo     operator_lo
#define LOCAL_OperatorFoo      OperatorLo
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorLe)
#define LOCAL_operator_foo     operator_le
#define LOCAL_OperatorFoo      OperatorLe
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorGr)
#define LOCAL_operator_foo     operator_gr
#define LOCAL_OperatorFoo      OperatorGr
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorGe)
#define LOCAL_operator_foo     operator_ge
#define LOCAL_OperatorFoo      OperatorGe
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorAdd)
#define LOCAL_operator_foo     operator_add
#define LOCAL_OperatorFoo      OperatorAdd
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorSub)
#define LOCAL_operator_foo     operator_sub
#define LOCAL_OperatorFoo      OperatorSub
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorAnd)
#define LOCAL_operator_foo     operator_and
#define LOCAL_OperatorFoo      OperatorAnd
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorXor)
#define LOCAL_operator_foo     operator_xor
#define LOCAL_OperatorFoo      OperatorXor
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorInplaceAdd)
#define LOCAL_operator_foo     operator_inplace_add
#define LOCAL_OperatorFoo      OperatorInplaceAdd
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorInplaceSub)
#define LOCAL_operator_foo     operator_inplace_sub
#define LOCAL_OperatorFoo      OperatorInplaceSub
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorInplaceAnd)
#define LOCAL_operator_foo     operator_inplace_and
#define LOCAL_OperatorFoo      OperatorInplaceAnd
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapOperatorInplaceXor)
#define LOCAL_operator_foo     operator_inplace_xor
#define LOCAL_OperatorFoo      OperatorInplaceXor
#define LOCAL_Dee_SEQCLASS     Dee_SEQCLASS_MAP
#else /* DEFINE_DeeType_RequireSeq... */
#error "Invalid configuration"
#endif /* !DEFINE_DeeType_RequireSeq... */

#ifndef LOCAL_Dee_SEQCLASS
#define LOCAL_Dee_SEQCLASS Dee_SEQCLASS_SEQ
#endif /* !LOCAL_Dee_SEQCLASS */

#ifndef LOCAL_SeqClass
#if LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SEQ
#define LOCAL_SeqClass Seq
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SET
#define LOCAL_SeqClass Set
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_MAP
#define LOCAL_SeqClass Map
#else /* LOCAL_Dee_SEQCLASS == ... */
#define LOCAL_SeqClass INVALID_LOCAL_Dee_SEQCLASS
#endif /* LOCAL_Dee_SEQCLASS != ... */
#endif /* !LOCAL_SeqClass */

#ifndef LOCAL_Seq
#if LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SEQ
#define LOCAL_Seq Seq
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SET
#define LOCAL_Seq Set
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_MAP
#define LOCAL_Seq Map
#else /* LOCAL_Dee_SEQCLASS == ... */
#define LOCAL_Seq INVALID_LOCAL_Dee_SEQCLASS
#endif /* LOCAL_Dee_SEQCLASS != ... */
#endif /* !LOCAL_Seq */

#ifndef LOCAL_seq
#if LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SEQ
#define LOCAL_seq seq
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SET
#define LOCAL_seq set
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_MAP
#define LOCAL_seq map
#else /* LOCAL_Dee_SEQCLASS == ... */
#define LOCAL_seq INVALID_LOCAL_Dee_SEQCLASS
#endif /* LOCAL_Dee_SEQCLASS != ... */
#endif /* !LOCAL_seq */

#ifndef LOCAL_tsc_seq_operator_foo
#define LOCAL_tsc_seq_operator_foo PP_CAT4(tsc_, LOCAL_seq, _, LOCAL_operator_foo)
#endif /* !LOCAL_tsc_seq_operator_foo */

#ifndef LOCAL_TMH
#if LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SEQ
#define LOCAL_TMH PP_CAT2(Dee_TMH_seq_, LOCAL_operator_foo)
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SET
#define LOCAL_TMH PP_CAT2(Dee_TMH_set_, LOCAL_operator_foo)
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_MAP
#define LOCAL_TMH PP_CAT2(Dee_TMH_map_, LOCAL_operator_foo)
#else /* LOCAL_Dee_SEQCLASS == ... */
#define LOCAL_TMH PP_CAT2(Dee_TMH_INVALID_LOCAL_Dee_SEQCLASS_, LOCAL_operator_foo)
#endif /* LOCAL_Dee_SEQCLASS != ... */
#endif /* !LOCAL_TMH */

#ifndef LOCAL_Dee_mh_seq_operator_foo_t
#define LOCAL_Dee_mh_seq_operator_foo_t PP_CAT5(Dee_mh_, LOCAL_seq, _, LOCAL_operator_foo, _t)
#endif /* !LOCAL_Dee_mh_seq_operator_foo_t */

#ifndef LOCAL_DeeSeq_OperatorFoo
#define LOCAL_DeeSeq_OperatorFoo PP_CAT4(Dee, LOCAL_Seq, _, LOCAL_OperatorFoo)
#endif /* !LOCAL_DeeSeq_OperatorFoo */

#ifndef LOCAL_DeeType_RequireSeqOperatorFoo_uncached
#ifdef LOCAL_FOR_OPTIMIZE
#define LOCAL_DeeType_RequireSeqOperatorFoo_uncached PP_CAT4(DeeType_Require, LOCAL_Seq, LOCAL_OperatorFoo, _for_optimize)
#else /* LOCAL_FOR_OPTIMIZE */
#define LOCAL_DeeType_RequireSeqOperatorFoo_uncached PP_CAT4(DeeType_Require, LOCAL_Seq, LOCAL_OperatorFoo, _uncached)
#endif /* !LOCAL_FOR_OPTIMIZE */
#endif /* !LOCAL_DeeType_RequireSeqOperatorFoo_uncached */

#ifndef LOCAL_DeeType_RequireSeqOperatorFoo
#define LOCAL_DeeType_RequireSeqOperatorFoo PP_CAT3(DeeType_Require, LOCAL_Seq, LOCAL_OperatorFoo)
#endif /* !LOCAL_DeeType_RequireSeqOperatorFoo */

#ifndef LOCAL_DeeSeq_DefaultOperatorFooWithEmpty
#define LOCAL_DeeSeq_DefaultOperatorFooWithEmpty PP_CAT5(Dee, LOCAL_Seq, _Default, LOCAL_OperatorFoo, WithEmpty)
#endif /* !LOCAL_DeeSeq_DefaultOperatorFooWithEmpty */

#ifndef LOCAL_DeeSeq_DefaultOperatorFooWithError
#define LOCAL_DeeSeq_DefaultOperatorFooWithError PP_CAT5(Dee, LOCAL_Seq, _Default, LOCAL_OperatorFoo, WithError)
#endif /* !LOCAL_DeeSeq_DefaultOperatorFooWithError */


PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_mh_seq_operator_foo_t DCALL
LOCAL_DeeType_RequireSeqOperatorFoo_uncached(DeeTypeObject *__restrict self) {
	(void)self;
#ifdef LOCAL_TMH
	{
		/* Check if the type defines a custom hint for this operator. */
		LOCAL_Dee_mh_seq_operator_foo_t hint;
		hint = (LOCAL_Dee_mh_seq_operator_foo_t)DeeType_GetPrivateMethodHint(self, LOCAL_TMH);
		if (hint != NULL)
			return hint;
	}
#endif /* LOCAL_TMH */
#ifdef DEFINE_DeeType_RequireSeqOperatorBool
	if (DeeType_GetSeqClass(self) != Dee_SEQCLASS_NONE) {
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
		if (DeeType_RequireBool(self))
			return self->tp_cast.tp_bool;
#endif /* !LOCAL_FOR_OPTIMIZE */
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorBoolWithEmpty;
		if (self->tp_seq && Dee_type_seq_has_custom_tp_size(self->tp_seq))
			return &DeeSeq_DefaultBoolWithSize;
		if (self->tp_seq && Dee_type_seq_has_custom_tp_sizeob(self->tp_seq))
			return &DeeSeq_DefaultBoolWithSizeOb;
		if (self->tp_seq && Dee_type_seq_has_custom_tp_foreach(self->tp_seq))
			return &DeeSeq_DefaultBoolWithForeach;
		if (self->tp_cmp && self->tp_cmp->tp_compare_eq &&
		    !DeeType_IsDefaultCompareEq(self->tp_cmp->tp_compare_eq) &&
		    !DeeType_IsDefaultCompare(self->tp_cmp->tp_compare_eq))
			return &DeeSeq_DefaultBoolWithCompareEq;
		if (self->tp_cmp && self->tp_cmp->tp_eq && !DeeType_IsDefaultEq(self->tp_cmp->tp_eq))
			return &DeeSeq_DefaultBoolWithEq;
		if (self->tp_cmp && self->tp_cmp->tp_ne && !DeeType_IsDefaultNe(self->tp_cmp->tp_ne))
			return &DeeSeq_DefaultBoolWithNe;
		if ((self->tp_seq && self->tp_seq->tp_foreach) || DeeType_InheritIter(self))
			return &DeeSeq_DefaultBoolWithForeachDefault;
	} else {
		if (DeeType_RequireForeach(self))
			return &DeeSeq_DefaultBoolWithForeach;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorIter)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_RequireIter(self))
		return self->tp_seq->tp_iter;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_RequireSeqOperatorSizeOb)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) != Dee_SEQCLASS_NONE && DeeType_RequireSizeOb(self))
		return self->tp_seq->tp_sizeob;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_size_t tsc_seq_operator_size;
		tsc_seq_operator_size = DeeType_RequireSeqOperatorSize(self);
		if (tsc_seq_operator_size == &DeeSeq_DefaultOperatorSizeWithEmpty)
			return &DeeSeq_DefaultOperatorSizeObWithEmpty;
		if (tsc_seq_operator_size != &DeeSeq_DefaultOperatorSizeWithError)
			return &DeeSeq_DefaultOperatorSizeObWithSeqOperatorSize;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorContains)
	switch (DeeType_GetSeqClass(self)) {
	case Dee_SEQCLASS_SEQ:
	case Dee_SEQCLASS_SET:
#ifndef LOCAL_FOR_OPTIMIZE
		if (DeeType_RequireContains(self))
			return self->tp_seq->tp_contains;
#endif /* !LOCAL_FOR_OPTIMIZE */
		break;
	case Dee_SEQCLASS_MAP:
		if ((self->tp_seq && self->tp_seq->tp_trygetitem) || DeeType_InheritGetItem(self))
			return &DeeSeq_DefaultOperatorContainsWithMapTryGetItem;
		break;
	default: break;
	}
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorContainsWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultContainsWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorGetItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetItem(self))
		return self->tp_seq->tp_getitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_getitem_index_t tsc_seq_operator_getitem_index;
		tsc_seq_operator_getitem_index = DeeType_RequireSeqOperatorGetItemIndex(self);
		if (tsc_seq_operator_getitem_index == &DeeSeq_DefaultOperatorGetItemIndexWithEmpty)
			return &DeeSeq_DefaultOperatorGetItemWithEmpty;
		if (tsc_seq_operator_getitem_index != &DeeSeq_DefaultOperatorGetItemIndexWithError)
			return &DeeSeq_DefaultOperatorGetItemWithSeqGetItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorDelItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireDelItem(self))
		return self->tp_seq->tp_delitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_RequireSeqOperatorSetItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetItem(self))
		return self->tp_seq->tp_setitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_RequireSeqOperatorGetRange)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetRange(self))
		return self->tp_seq->tp_getrange;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_iter_t tsc_seq_operator_iter;
		tsc_seq_operator_iter = DeeType_RequireSeqOperatorIter(self);
		if (tsc_seq_operator_iter == &DeeSeq_DefaultOperatorIterWithEmpty)
			return &DeeSeq_DefaultOperatorGetRangeWithEmpty;
		if (tsc_seq_operator_iter != &DeeSeq_DefaultOperatorIterWithError)
			return &DeeSeq_DefaultOperatorGetRangeWithSeqGetRangeIndexAndGetRangeIndexN;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorDelRange)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireDelRange(self))
		return self->tp_seq->tp_delrange;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_RequireSeqOperatorSetRange)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetRange(self))
		return self->tp_seq->tp_setrange;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_RequireSeqOperatorForeach)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_RequireForeach(self))
		return self->tp_seq->tp_foreach;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_RequireSeqOperatorForeachPair)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_RequireForeachPair(self))
		return self->tp_seq->tp_foreach_pair;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_RequireSeqOperatorEnumerate)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerate(self))
		return self->tp_seq->tp_enumerate;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorEnumerateWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultEnumerateWithCounterAndForeach;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorIterKeys)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireIterKeys(self))
		return self->tp_seq->tp_iterkeys;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorIterKeysWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultOperatorIterKeysWithSeqSize;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorEnumerateIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerateIndex(self))
		return self->tp_seq->tp_enumerate_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorEnumerateIndexWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError) {
			if (tsc_seq_operator_foreach == &DeeObject_DefaultForeachWithIter)
				return &DeeSeq_DefaultEnumerateIndexWithCounterAndIter;
			return &DeeSeq_DefaultEnumerateIndexWithCounterAndForeachDefault;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorBoundItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireBoundItem(self))
		return self->tp_seq->tp_bounditem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_bounditem_index_t tsc_seq_operator_bounditem_index;
		tsc_seq_operator_bounditem_index = DeeType_RequireSeqOperatorBoundItemIndex(self);
		if (tsc_seq_operator_bounditem_index == &DeeSeq_DefaultOperatorBoundItemIndexWithEmpty)
			return &DeeSeq_DefaultOperatorBoundItemWithEmpty;
		if (tsc_seq_operator_bounditem_index != &DeeSeq_DefaultOperatorBoundItemIndexWithError)
			return &DeeSeq_DefaultOperatorBoundItemWithSeqBoundItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorHasItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireHasItem(self))
		return self->tp_seq->tp_hasitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_hasitem_index_t tsc_seq_operator_hasitem_index;
		tsc_seq_operator_hasitem_index = DeeType_RequireSeqOperatorHasItemIndex(self);
		if (tsc_seq_operator_hasitem_index == &DeeSeq_DefaultOperatorHasItemIndexWithEmpty)
			return &DeeSeq_DefaultOperatorHasItemWithEmpty;
		if (tsc_seq_operator_hasitem_index != &DeeSeq_DefaultOperatorHasItemIndexWithError)
			return &DeeSeq_DefaultOperatorHasItemWithSeqHasItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorSize)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) != Dee_SEQCLASS_NONE && DeeType_RequireSize(self))
		return self->tp_seq->tp_size;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorSizeWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError) {
			ASSERT(self->tp_seq);
			if (self->tp_seq->tp_foreach && !DeeType_IsDefaultForeach(self->tp_seq->tp_foreach))
				return &DeeSeq_DefaultSizeWithForeach;
			if (self->tp_seq->tp_foreach_pair && !DeeType_IsDefaultForeachPair(self->tp_seq->tp_foreach_pair))
				return &DeeSeq_DefaultSizeWithForeachPair;
			return &DeeSeq_DefaultSizeWithForeach;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorSizeFast)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSizeFast(self))
		return self->tp_seq->tp_size_fast;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_RequireSeqOperatorGetItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetItemIndex(self))
		return self->tp_seq->tp_getitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorGetItemIndexWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultGetItemIndexWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorDelItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireDelItemIndex(self))
		return self->tp_seq->tp_delitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_RequireSeqOperatorSetItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetItemIndex(self))
		return self->tp_seq->tp_setitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_RequireSeqOperatorBoundItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireBoundItemIndex(self))
		return self->tp_seq->tp_bounditem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_size_t tsc_seq_operator_size;
		tsc_seq_operator_size = DeeType_RequireSeqOperatorSize(self);
		if (tsc_seq_operator_size == &DeeSeq_DefaultOperatorSizeWithEmpty)
			return &DeeSeq_DefaultOperatorBoundItemIndexWithEmpty;
		if (tsc_seq_operator_size != &DeeSeq_DefaultOperatorSizeWithError)
			return &DeeSeq_DefaultOperatorBoundItemIndexWithSeqSize;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorHasItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireHasItemIndex(self))
		return self->tp_seq->tp_hasitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_size_t tsc_seq_operator_size;
		tsc_seq_operator_size = DeeType_RequireSeqOperatorSize(self);
		if (tsc_seq_operator_size == &DeeSeq_DefaultOperatorSizeWithEmpty)
			return &DeeSeq_DefaultOperatorHasItemIndexWithEmpty;
		if (tsc_seq_operator_size != &DeeSeq_DefaultOperatorSizeWithError)
			return &DeeSeq_DefaultOperatorHasItemIndexWithSeqSize;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorGetRangeIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetRangeIndex(self))
		return self->tp_seq->tp_getrange_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_iter_t tsc_seq_operator_iter;
		tsc_seq_operator_iter = DeeType_RequireSeqOperatorIter(self);
		if (tsc_seq_operator_iter == &DeeSeq_DefaultOperatorIterWithEmpty)
			return &DeeSeq_DefaultOperatorGetRangeIndexWithEmpty;
		if (tsc_seq_operator_iter != &DeeSeq_DefaultOperatorIterWithError)
			return &DeeSeq_DefaultOperatorGetRangeIndexWithIterAndSeqSize;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorDelRangeIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireDelRangeIndex(self))
		return self->tp_seq->tp_delrange_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_RequireSeqOperatorSetRangeIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetRangeIndex(self))
		return self->tp_seq->tp_setrange_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_RequireSeqOperatorGetRangeIndexN)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetRangeIndexN(self))
		return self->tp_seq->tp_getrange_index_n;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_iter_t tsc_seq_operator_iter;
		tsc_seq_operator_iter = DeeType_RequireSeqOperatorIter(self);
		if (tsc_seq_operator_iter == &DeeSeq_DefaultOperatorIterWithEmpty)
			return &DeeSeq_DefaultOperatorGetRangeIndexNWithEmpty;
		if (tsc_seq_operator_iter != &DeeSeq_DefaultOperatorIterWithError)
			return &DeeSeq_DefaultOperatorGetRangeIndexNWithIterAndSeqSize;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorDelRangeIndexN)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireDelRangeIndexN(self))
		return self->tp_seq->tp_delrange_index_n;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_RequireSeqOperatorSetRangeIndexN)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetRangeIndexN(self))
		return self->tp_seq->tp_setrange_index_n;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_RequireSeqOperatorTryGetItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireTryGetItem(self))
		return self->tp_seq->tp_trygetitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_trygetitem_index_t tsc_seq_operator_trygetitem_index;
		tsc_seq_operator_trygetitem_index = DeeType_RequireSeqOperatorTryGetItemIndex(self);
		if (tsc_seq_operator_trygetitem_index == &DeeSeq_DefaultOperatorTryGetItemIndexWithEmpty)
			return &DeeSeq_DefaultOperatorTryGetItemWithEmpty;
		if (tsc_seq_operator_trygetitem_index != &DeeSeq_DefaultOperatorTryGetItemIndexWithError)
			return &DeeSeq_DefaultOperatorTryGetItemWithSeqTryGetItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorTryGetItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireTryGetItemIndex(self))
		return self->tp_seq->tp_trygetitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorTryGetItemIndexWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultTryGetItemIndexWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorHash)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireHash(self))
		return self->tp_cmp->tp_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorHashWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultHashWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorCompareEq)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireCompareEq(self))
		return self->tp_cmp->tp_compare_eq;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorCompareEqWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultCompareEqWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorCompare)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireCompare(self))
		return self->tp_cmp->tp_compare;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorCompareWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultCompareWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorTryCompareEq)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireTryCompareEq(self))
		return self->tp_cmp->tp_trycompare_eq;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorTryCompareEqWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultTryCompareEqWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorEq)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEq(self))
		return self->tp_cmp->tp_eq;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_compare_eq_t tsc_seq_operator_compare_eq;
		tsc_seq_operator_compare_eq = DeeType_RequireSeqOperatorCompareEq(self);
		if (tsc_seq_operator_compare_eq == &DeeSeq_DefaultOperatorCompareEqWithEmpty)
			return &DeeSeq_DefaultOperatorEqWithEmpty;
		if (tsc_seq_operator_compare_eq != &DeeSeq_DefaultOperatorCompareEqWithError)
			return &DeeSeq_DefaultOperatorEqWithSeqCompareEq;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorNe)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireNe(self))
		return self->tp_cmp->tp_ne;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_compare_eq_t tsc_seq_operator_compare_eq;
		tsc_seq_operator_compare_eq = DeeType_RequireSeqOperatorCompareEq(self);
		if (tsc_seq_operator_compare_eq == &DeeSeq_DefaultOperatorCompareEqWithEmpty)
			return &DeeSeq_DefaultOperatorNeWithEmpty;
		if (tsc_seq_operator_compare_eq != &DeeSeq_DefaultOperatorCompareEqWithError)
			return &DeeSeq_DefaultOperatorNeWithSeqCompareEq;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorLo)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireLo(self))
		return self->tp_cmp->tp_lo;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_compare_t tsc_seq_operator_compare;
		tsc_seq_operator_compare = DeeType_RequireSeqOperatorCompare(self);
		if (tsc_seq_operator_compare == &DeeSeq_DefaultOperatorCompareWithEmpty)
			return &DeeSeq_DefaultOperatorLoWithEmpty;
		if (tsc_seq_operator_compare != &DeeSeq_DefaultOperatorCompareWithError)
			return &DeeSeq_DefaultOperatorLoWithSeqCompare;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorLe)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireLe(self))
		return self->tp_cmp->tp_le;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_compare_t tsc_seq_operator_compare;
		tsc_seq_operator_compare = DeeType_RequireSeqOperatorCompare(self);
		if (tsc_seq_operator_compare == &DeeSeq_DefaultOperatorCompareWithEmpty)
			return &DeeSeq_DefaultOperatorLeWithEmpty;
		if (tsc_seq_operator_compare != &DeeSeq_DefaultOperatorCompareWithError)
			return &DeeSeq_DefaultOperatorLeWithSeqCompare;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorGr)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGr(self))
		return self->tp_cmp->tp_gr;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_compare_t tsc_seq_operator_compare;
		tsc_seq_operator_compare = DeeType_RequireSeqOperatorCompare(self);
		if (tsc_seq_operator_compare == &DeeSeq_DefaultOperatorCompareWithEmpty)
			return &DeeSeq_DefaultOperatorGrWithEmpty;
		if (tsc_seq_operator_compare != &DeeSeq_DefaultOperatorCompareWithError)
			return &DeeSeq_DefaultOperatorGrWithSeqCompare;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorGe)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGe(self))
		return self->tp_cmp->tp_ge;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_compare_t tsc_seq_operator_compare;
		tsc_seq_operator_compare = DeeType_RequireSeqOperatorCompare(self);
		if (tsc_seq_operator_compare == &DeeSeq_DefaultOperatorCompareWithEmpty)
			return &DeeSeq_DefaultOperatorGeWithEmpty;
		if (tsc_seq_operator_compare != &DeeSeq_DefaultOperatorCompareWithError)
			return &DeeSeq_DefaultOperatorGeWithSeqCompare;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorInplaceAdd)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireInplaceAdd(self))
		return self->tp_math->tp_inplace_add;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_extend_t tsc_seq_extend;
		tsc_seq_extend = DeeType_RequireSeqExtend(self);
		if (tsc_seq_extend != &DeeSeq_DefaultExtendWithError)
			return &DeeSeq_DefaultOperatorInplaceAddWithTSCExtend;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorInplaceMul)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireInplaceMul(self))
		return self->tp_math->tp_inplace_mul;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_extend_t tsc_seq_extend;
		Dee_mh_seq_clear_t tsc_seq_clear;
		if ((tsc_seq_extend = DeeType_RequireSeqExtend(self)) != &DeeSeq_DefaultExtendWithError &&
		    (tsc_seq_clear = DeeType_RequireSeqClear(self)) != &DeeSeq_DefaultClearWithError)
			return &DeeSeq_DefaultOperatorInplaceMulWithTSCClearAndTSCExtend;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorIter)
#ifndef LOCAL_FOR_OPTIMIZE
	{
		unsigned int sc = DeeType_GetSeqClass(self);
		if ((sc == Dee_SEQCLASS_SET || sc == Dee_SEQCLASS_MAP) && DeeType_RequireIter(self))
			return self->tp_seq->tp_iter;
	}
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_iter_t tsc_seq_operator_iter;
		tsc_seq_operator_iter = DeeType_RequireSeqOperatorIter(self);
		if (tsc_seq_operator_iter == &DeeSeq_DefaultOperatorIterWithEmpty)
			return &DeeSet_DefaultOperatorIterWithEmpty;
		if (tsc_seq_operator_iter != &DeeSeq_DefaultOperatorIterWithError)
			return &DeeSet_DefaultOperatorIterWithDistinctIter;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorForeach)
#ifndef LOCAL_FOR_OPTIMIZE
	{
		unsigned int sc = DeeType_GetSeqClass(self);
		if ((sc == Dee_SEQCLASS_SET || sc == Dee_SEQCLASS_MAP) && DeeType_RequireForeach(self))
			return self->tp_seq->tp_foreach;
	}
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorForeachWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultOperatorForeachWithDistinctForeach;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorSize)
#ifndef LOCAL_FOR_OPTIMIZE
	{
		unsigned int sc = DeeType_GetSeqClass(self);
		if ((sc == Dee_SEQCLASS_SET || sc == Dee_SEQCLASS_MAP) && DeeType_RequireSize(self))
			return self->tp_seq->tp_size;
	}
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorSizeWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultOperatorSizeWithSetForeach;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorSizeOb)
#ifndef LOCAL_FOR_OPTIMIZE
	{
		unsigned int sc = DeeType_GetSeqClass(self);
		if ((sc == Dee_SEQCLASS_SET || sc == Dee_SEQCLASS_MAP) && DeeType_RequireSizeOb(self))
			return self->tp_seq->tp_sizeob;
	}
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_set_operator_size_t tsc_set_operator_size;
		tsc_set_operator_size = DeeType_RequireSetOperatorSize(self);
		if (tsc_set_operator_size == &DeeSet_DefaultOperatorSizeWithEmpty)
			return &DeeSet_DefaultOperatorSizeObWithEmpty;
		if (tsc_set_operator_size != &DeeSet_DefaultOperatorSizeWithError)
			return &DeeSet_DefaultOperatorSizeObWithSetSize;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorHash)
#ifndef LOCAL_FOR_OPTIMIZE
	{
		unsigned int sc = DeeType_GetSeqClass(self);
		if ((sc == Dee_SEQCLASS_SET || sc == Dee_SEQCLASS_MAP) && DeeType_RequireHash(self))
			return self->tp_cmp->tp_hash;
	}
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorHashWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError) {
			ASSERTF(self->tp_seq, "Should exist because `tsc_seq_operator_foreach' isn't the error-impl");
			if (self->tp_seq->tp_foreach && !DeeType_IsDefaultForeach(self->tp_seq->tp_foreach))
				return &DeeSet_DefaultHashWithForeachDefault;
			if (self->tp_seq->tp_foreach_pair && !DeeType_IsDefaultForeachPair(self->tp_seq->tp_foreach_pair))
				return &DeeMap_DefaultHashWithForeachPairDefault;
			return &DeeSet_DefaultHashWithForeachDefault;
		}
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorCompareEq)
#ifndef LOCAL_FOR_OPTIMIZE
	{
		unsigned int sc = DeeType_GetSeqClass(self);
		if ((sc == Dee_SEQCLASS_SET || sc == Dee_SEQCLASS_MAP) && DeeType_RequireCompareEq(self))
			return self->tp_cmp->tp_compare_eq;
	}
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorCompareEqWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultCompareEqWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorTryCompareEq)
#ifndef LOCAL_FOR_OPTIMIZE
	{
		unsigned int sc = DeeType_GetSeqClass(self);
		if ((sc == Dee_SEQCLASS_SET || sc == Dee_SEQCLASS_MAP) && DeeType_RequireTryCompareEq(self))
			return self->tp_cmp->tp_trycompare_eq;
	}
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorTryCompareEqWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultTryCompareEqWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorEq)
#ifndef LOCAL_FOR_OPTIMIZE
	{
		unsigned int sc = DeeType_GetSeqClass(self);
		if ((sc == Dee_SEQCLASS_SET || sc == Dee_SEQCLASS_MAP) && DeeType_RequireEq(self))
			return self->tp_cmp->tp_eq;
	}
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_compare_eq_t tsc_seq_operator_compare_eq;
		tsc_seq_operator_compare_eq = DeeType_RequireSetOperatorCompareEq(self);
		if (tsc_seq_operator_compare_eq == &DeeSet_DefaultOperatorCompareEqWithEmpty)
			return &DeeSet_DefaultOperatorEqWithEmpty;
		if (tsc_seq_operator_compare_eq != &DeeSet_DefaultOperatorCompareEqWithError)
			return &DeeSet_DefaultOperatorEqWithSetCompareEq;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorNe)
#ifndef LOCAL_FOR_OPTIMIZE
	{
		unsigned int sc = DeeType_GetSeqClass(self);
		if ((sc == Dee_SEQCLASS_SET || sc == Dee_SEQCLASS_MAP) && DeeType_RequireNe(self))
			return self->tp_cmp->tp_ne;
	}
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_compare_eq_t tsc_seq_operator_compare_eq;
		tsc_seq_operator_compare_eq = DeeType_RequireSetOperatorCompareEq(self);
		if (tsc_seq_operator_compare_eq == &DeeSet_DefaultOperatorCompareEqWithEmpty)
			return &DeeSet_DefaultOperatorNeWithEmpty;
		if (tsc_seq_operator_compare_eq != &DeeSet_DefaultOperatorCompareEqWithError)
			return &DeeSet_DefaultOperatorNeWithSetCompareEq;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorLo)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SET && DeeType_RequireLo(self))
		return self->tp_cmp->tp_lo;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorLoWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultLoWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorLe)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SET && DeeType_RequireLe(self))
		return self->tp_cmp->tp_le;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorLeWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultLeWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorGr)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SET && DeeType_RequireGr(self))
		return self->tp_cmp->tp_gr;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorGrWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultGrWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorGe)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SET && DeeType_RequireGe(self))
		return self->tp_cmp->tp_ge;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorGeWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultGeWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorInv)
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorInvWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultOperatorInvWithForeach;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorAdd)
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorAddWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultOperatorAddWithForeach;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorSub)
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorSubWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultOperatorSubWithForeach;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorAnd)
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorAndWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultOperatorAndWithForeach;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorXor)
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorXorWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultOperatorXorWithForeach;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorInplaceAdd)
	{
		Dee_mh_set_insertall_t tsc_set_insertall;
		tsc_set_insertall = DeeType_RequireSetInsertAll(self);
		if (tsc_set_insertall != &DeeSet_DefaultInsertAllWithError)
			return &DeeSet_DefaultOperatorInplaceAddWithSetInsertAll;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorInplaceSub)
	{
		Dee_mh_set_removeall_t tsc_set_removeall;
		tsc_set_removeall = DeeType_RequireSetRemoveAll(self);
		if (tsc_set_removeall != &DeeSet_DefaultRemoveAllWithError)
			return &DeeSet_DefaultOperatorInplaceSubWithSetRemoveAll;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorInplaceAnd)
	{
		Dee_mh_set_removeall_t tsc_set_removeall;
		tsc_set_removeall = DeeType_RequireSetRemoveAll(self);
		if (tsc_set_removeall != &DeeSet_DefaultRemoveAllWithError)
			return &DeeSet_DefaultOperatorInplaceAndWithForeachAndSetRemoveAll;
	}
#elif defined(DEFINE_DeeType_RequireSetOperatorInplaceXor)
	{
		Dee_mh_set_insertall_t tsc_set_insertall;
		Dee_mh_set_removeall_t tsc_set_removeall;
		if ((tsc_set_insertall = DeeType_RequireSetInsertAll(self)) != &DeeSet_DefaultInsertAllWithError &&
		    (tsc_set_removeall = DeeType_RequireSetRemoveAll(self)) != &DeeSet_DefaultRemoveAllWithError)
			return &DeeSet_DefaultOperatorInplaceXorWithForeachAndSetInsertAllAndSetRemoveAll;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorContains)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireContains(self))
		return self->tp_seq->tp_contains;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorContainsWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultContainsWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorGetItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireGetItem(self))
		return self->tp_seq->tp_getitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorGetItemWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultGetItemWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorDelItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireDelItem(self))
		return self->tp_seq->tp_delitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	/* TODO: Treat as ?S?T2?O?O and implement as (if used functions are available):
	 * >> operator del[](key) {
	 * >>     Sequence.remove(key: x -> key == x.first);
	 * >> } */
#elif defined(DEFINE_DeeType_RequireMapOperatorSetItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireSetItem(self))
		return self->tp_seq->tp_setitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	/* TODO: Treat as ?S?T2?O?O and implement as (if used functions are available):
	 * >> operator []= (key, value) {
	 * >>     local index = Sequence.find(this, key: x -> key == x.first);
	 * >>     if (index == -1) {
	 * >>         Sequence.append(this, (key, value));
	 * >>     } else {
	 * >>         Sequence.__delitem__(this, index);
	 * >>     }
	 * >> } */
#elif defined(DEFINE_DeeType_RequireMapOperatorEnumerate)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireEnumerate(self))
		return self->tp_seq->tp_enumerate;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorEnumerateWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultEnumerateWithForeachPairDefault;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorEnumerateIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireEnumerateIndex(self))
		return self->tp_seq->tp_enumerate_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorEnumerateIndexWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultEnumerateIndexWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorBoundItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireBoundItem(self))
		return self->tp_seq->tp_bounditem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorBoundItemWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultBoundItemWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorHasItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireHasItem(self))
		return self->tp_seq->tp_hasitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorHasItemWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultHasItemWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorGetItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireGetItemIndex(self))
		return self->tp_seq->tp_getitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorGetItemIndexWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultGetItemIndexWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorDelItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireDelItemIndex(self))
		return self->tp_seq->tp_delitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	/* TODO: See `DEFINE_DeeType_RequireMapOperatorDelItem' */
#elif defined(DEFINE_DeeType_RequireMapOperatorSetItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireSetItemIndex(self))
		return self->tp_seq->tp_setitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	/* TODO: See `DEFINE_DeeType_RequireMapOperatorSetItem' */
#elif defined(DEFINE_DeeType_RequireMapOperatorBoundItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireBoundItemIndex(self))
		return self->tp_seq->tp_bounditem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorBoundItemIndexWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultBoundItemIndexWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorHasItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireHasItemIndex(self))
		return self->tp_seq->tp_hasitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorHasItemIndexWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultHasItemIndexWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorTryGetItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireTryGetItem(self))
		return self->tp_seq->tp_trygetitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorTryGetItemWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultTryGetItemWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorTryGetItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireTryGetItemIndex(self))
		return self->tp_seq->tp_trygetitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorTryGetItemIndexWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultTryGetItemIndexWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorTryGetItemStringHash)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireTryGetItemStringHash(self))
		return self->tp_seq->tp_trygetitem_string_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorTryGetItemStringHashWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultTryGetItemStringHashWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorGetItemStringHash)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireGetItemStringHash(self))
		return self->tp_seq->tp_getitem_string_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorGetItemStringHashWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultGetItemStringHashWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorDelItemStringHash)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireDelItemStringHash(self))
		return self->tp_seq->tp_delitem_string_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	/* TODO: See `DEFINE_DeeType_RequireMapOperatorDelItem' */
#elif defined(DEFINE_DeeType_RequireMapOperatorSetItemStringHash)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireSetItemStringHash(self))
		return self->tp_seq->tp_setitem_string_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	/* TODO: See `DEFINE_DeeType_RequireMapOperatorSetItem' */
#elif defined(DEFINE_DeeType_RequireMapOperatorBoundItemStringHash)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireBoundItemStringHash(self))
		return self->tp_seq->tp_bounditem_string_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorBoundItemStringHashWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultBoundItemStringHashWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorHasItemStringHash)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireHasItemStringHash(self))
		return self->tp_seq->tp_hasitem_string_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorHasItemStringHashWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultHasItemStringHashWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorTryGetItemStringLenHash)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireTryGetItemStringLenHash(self))
		return self->tp_seq->tp_trygetitem_string_len_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorTryGetItemStringLenHashWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultTryGetItemStringLenHashWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorGetItemStringLenHash)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireGetItemStringLenHash(self))
		return self->tp_seq->tp_getitem_string_len_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorGetItemStringLenHashWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultGetItemStringLenHashWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorDelItemStringLenHash)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireDelItemStringLenHash(self))
		return self->tp_seq->tp_delitem_string_len_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	/* TODO: See `DEFINE_DeeType_RequireMapOperatorDelItem' */
#elif defined(DEFINE_DeeType_RequireMapOperatorSetItemStringLenHash)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireSetItemStringLenHash(self))
		return self->tp_seq->tp_setitem_string_len_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	/* TODO: See `DEFINE_DeeType_RequireMapOperatorSetItem' */
#elif defined(DEFINE_DeeType_RequireMapOperatorBoundItemStringLenHash)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireBoundItemStringLenHash(self))
		return self->tp_seq->tp_bounditem_string_len_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorBoundItemStringLenHashWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultBoundItemStringLenHashWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorHasItemStringLenHash)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireHasItemStringLenHash(self))
		return self->tp_seq->tp_hasitem_string_len_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorHasItemStringLenHashWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultHasItemStringLenHashWithForeachPair;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorCompareEq)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireCompareEq(self))
		return self->tp_cmp->tp_compare_eq;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorCompareEqWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultCompareEqWithForeachPairDefault;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorTryCompareEq)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireTryCompareEq(self))
		return self->tp_cmp->tp_trycompare_eq;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorTryCompareEqWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultTryCompareEqWithForeachPairDefault;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorEq)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireEq(self))
		return self->tp_cmp->tp_eq;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_compare_eq_t tsc_map_operator_compare_eq;
		tsc_map_operator_compare_eq = DeeType_RequireMapOperatorCompareEq(self);
		if (tsc_map_operator_compare_eq == &DeeMap_DefaultOperatorCompareEqWithEmpty)
			return &DeeMap_DefaultOperatorEqWithEmpty;
		if (tsc_map_operator_compare_eq != &DeeMap_DefaultOperatorCompareEqWithError)
			return &DeeMap_DefaultOperatorEqWithMapCompareEq;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorNe)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireNe(self))
		return self->tp_cmp->tp_ne;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_compare_eq_t tsc_map_operator_compare_eq;
		tsc_map_operator_compare_eq = DeeType_RequireMapOperatorCompareEq(self);
		if (tsc_map_operator_compare_eq == &DeeMap_DefaultOperatorCompareEqWithEmpty)
			return &DeeMap_DefaultOperatorNeWithEmpty;
		if (tsc_map_operator_compare_eq != &DeeMap_DefaultOperatorCompareEqWithError)
			return &DeeMap_DefaultOperatorNeWithMapCompareEq;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorLo)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireLo(self))
		return self->tp_cmp->tp_lo;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorLoWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultLoWithForeachPairDefault;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorLe)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireLe(self))
		return self->tp_cmp->tp_le;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorLeWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultLeWithForeachPairDefault;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorGr)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireGr(self))
		return self->tp_cmp->tp_gr;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorGrWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultGrWithForeachPairDefault;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorGe)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP && DeeType_RequireGe(self))
		return self->tp_cmp->tp_ge;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_mh_seq_operator_foreach_pair_t tsc_seq_operator_foreach_pair;
		tsc_seq_operator_foreach_pair = DeeType_RequireSeqOperatorForeachPair(self);
		if (tsc_seq_operator_foreach_pair == &DeeSeq_DefaultOperatorForeachPairWithEmpty)
			return &DeeMap_DefaultOperatorGeWithEmpty;
		if (tsc_seq_operator_foreach_pair != &DeeSeq_DefaultOperatorForeachPairWithError)
			return &DeeMap_DefaultGeWithForeachPairDefault;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorAdd)
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeMap_DefaultOperatorAddWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeMap_DefaultOperatorAddWithForeach;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorSub)
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeMap_DefaultOperatorSubWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeMap_DefaultOperatorSubWithForeach;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorAnd)
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeMap_DefaultOperatorAndWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeMap_DefaultOperatorAndWithForeach;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorXor)
	{
		Dee_mh_seq_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeMap_DefaultOperatorXorWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeMap_DefaultOperatorXorWithForeach;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorInplaceAdd)
	{
		Dee_mh_map_update_t tsc_map_update;
		tsc_map_update = DeeType_RequireMapUpdate(self);
		if (tsc_map_update != &DeeMap_DefaultUpdateWithError)
			return &DeeMap_DefaultOperatorInplaceAddWithMapUpdate;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorInplaceSub)
	{
		Dee_mh_map_removekeys_t tsc_map_removekeys;
		tsc_map_removekeys = DeeType_RequireMapRemoveKeys(self);
		if (tsc_map_removekeys != &DeeMap_DefaultRemoveKeysWithError)
			return &DeeMap_DefaultOperatorInplaceSubWithMapRemoveKeys;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorInplaceAnd)
	{
		Dee_mh_map_removekeys_t tsc_map_removekeys;
		tsc_map_removekeys = DeeType_RequireMapRemoveKeys(self);
		if (tsc_map_removekeys != &DeeMap_DefaultRemoveKeysWithError)
			return &DeeMap_DefaultOperatorInplaceAndWithForeachAndMapRemoveKeys;
	}
#elif defined(DEFINE_DeeType_RequireMapOperatorInplaceXor)
	{
		Dee_mh_map_update_t tsc_map_update;
		Dee_mh_map_removekeys_t tsc_map_removekeys;
		if ((tsc_map_update = DeeType_RequireMapUpdate(self)) != &DeeMap_DefaultUpdateWithError &&
		    (tsc_map_removekeys = DeeType_RequireMapRemoveKeys(self)) != &DeeMap_DefaultRemoveKeysWithError)
			return &DeeMap_DefaultOperatorInplaceXorWithForeachAndMapUpdatAndMapRemoveKeys;
	}
#endif /* ... */
	return &LOCAL_DeeSeq_DefaultOperatorFooWithError;
}


#ifndef LOCAL_FOR_OPTIMIZE
INTERN ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_mh_seq_operator_foo_t DCALL
LOCAL_DeeType_RequireSeqOperatorFoo(DeeTypeObject *__restrict self) {
	LOCAL_Dee_mh_seq_operator_foo_t result;
	struct Dee_type_seq_cache *sc;
	struct Dee_type_seq *seq = self->tp_seq;
	if likely(seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->LOCAL_tsc_seq_operator_foo)
			return sc->LOCAL_tsc_seq_operator_foo;
	}
	result = LOCAL_DeeType_RequireSeqOperatorFoo_uncached(self);
	if unlikely(result == &LOCAL_DeeSeq_OperatorFoo) {
		/* Prevent infinite loop. This can happen when the user explicitly
		 * inherits an operator from "Sequence" (which by itself behaves
		 * like an empty sequence), so that is the behavior that needs to
		 * be inherited here. */
		result = &LOCAL_DeeSeq_DefaultOperatorFooWithEmpty;
	}
	sc = DeeType_TryRequireSeqCache(self);
	if likely(sc)
		atomic_write(&sc->LOCAL_tsc_seq_operator_foo, result);
	return result;
}
#endif /* !LOCAL_FOR_OPTIMIZE */

#undef LOCAL_DeeSeq_OperatorFoo
#undef LOCAL_TMH
#undef LOCAL_Dee_mh_seq_operator_foo_t
#undef LOCAL_tsc_seq_operator_foo
#undef LOCAL_seq
#undef LOCAL_SeqClass
#undef LOCAL_Seq
#undef LOCAL_DeeType_RequireSeqOperatorFoo_uncached
#undef LOCAL_DeeType_RequireSeqOperatorFoo
#undef LOCAL_DeeSeq_DefaultOperatorFooWithEmpty
#undef LOCAL_DeeSeq_DefaultOperatorFooWithError
#undef LOCAL_operator_foo
#undef LOCAL_OperatorFoo
#undef LOCAL_Dee_SEQCLASS

DECL_END

#undef DEFINE_DeeType_RequireSeqOperatorBool
#undef DEFINE_DeeType_RequireSeqOperatorIter
#undef DEFINE_DeeType_RequireSeqOperatorSizeOb
#undef DEFINE_DeeType_RequireSeqOperatorContains
#undef DEFINE_DeeType_RequireSeqOperatorGetItem
#undef DEFINE_DeeType_RequireSeqOperatorDelItem
#undef DEFINE_DeeType_RequireSeqOperatorSetItem
#undef DEFINE_DeeType_RequireSeqOperatorGetRange
#undef DEFINE_DeeType_RequireSeqOperatorDelRange
#undef DEFINE_DeeType_RequireSeqOperatorSetRange
#undef DEFINE_DeeType_RequireSeqOperatorForeach
#undef DEFINE_DeeType_RequireSeqOperatorForeachPair
#undef DEFINE_DeeType_RequireSeqOperatorEnumerate
#undef DEFINE_DeeType_RequireSeqOperatorEnumerateIndex
#undef DEFINE_DeeType_RequireSeqOperatorIterKeys
#undef DEFINE_DeeType_RequireSeqOperatorBoundItem
#undef DEFINE_DeeType_RequireSeqOperatorHasItem
#undef DEFINE_DeeType_RequireSeqOperatorSize
#undef DEFINE_DeeType_RequireSeqOperatorSizeFast
#undef DEFINE_DeeType_RequireSeqOperatorGetItemIndex
#undef DEFINE_DeeType_RequireSeqOperatorDelItemIndex
#undef DEFINE_DeeType_RequireSeqOperatorSetItemIndex
#undef DEFINE_DeeType_RequireSeqOperatorBoundItemIndex
#undef DEFINE_DeeType_RequireSeqOperatorHasItemIndex
#undef DEFINE_DeeType_RequireSeqOperatorGetRangeIndex
#undef DEFINE_DeeType_RequireSeqOperatorDelRangeIndex
#undef DEFINE_DeeType_RequireSeqOperatorSetRangeIndex
#undef DEFINE_DeeType_RequireSeqOperatorGetRangeIndexN
#undef DEFINE_DeeType_RequireSeqOperatorDelRangeIndexN
#undef DEFINE_DeeType_RequireSeqOperatorSetRangeIndexN
#undef DEFINE_DeeType_RequireSeqOperatorTryGetItem
#undef DEFINE_DeeType_RequireSeqOperatorTryGetItemIndex
#undef DEFINE_DeeType_RequireSeqOperatorHash
#undef DEFINE_DeeType_RequireSeqOperatorCompareEq
#undef DEFINE_DeeType_RequireSeqOperatorCompare
#undef DEFINE_DeeType_RequireSeqOperatorTryCompareEq
#undef DEFINE_DeeType_RequireSeqOperatorEq
#undef DEFINE_DeeType_RequireSeqOperatorNe
#undef DEFINE_DeeType_RequireSeqOperatorLo
#undef DEFINE_DeeType_RequireSeqOperatorLe
#undef DEFINE_DeeType_RequireSeqOperatorGr
#undef DEFINE_DeeType_RequireSeqOperatorGe
#undef DEFINE_DeeType_RequireSeqOperatorInplaceAdd
#undef DEFINE_DeeType_RequireSeqOperatorInplaceMul
#undef DEFINE_DeeType_RequireSetOperatorIter
#undef DEFINE_DeeType_RequireSetOperatorForeach
#undef DEFINE_DeeType_RequireSetOperatorSize
#undef DEFINE_DeeType_RequireSetOperatorSizeOb
#undef DEFINE_DeeType_RequireSetOperatorHash
#undef DEFINE_DeeType_RequireSetOperatorCompareEq
#undef DEFINE_DeeType_RequireSetOperatorTryCompareEq
#undef DEFINE_DeeType_RequireSetOperatorEq
#undef DEFINE_DeeType_RequireSetOperatorNe
#undef DEFINE_DeeType_RequireSetOperatorLo
#undef DEFINE_DeeType_RequireSetOperatorLe
#undef DEFINE_DeeType_RequireSetOperatorGr
#undef DEFINE_DeeType_RequireSetOperatorGe
#undef DEFINE_DeeType_RequireSetOperatorInv
#undef DEFINE_DeeType_RequireSetOperatorAdd
#undef DEFINE_DeeType_RequireSetOperatorSub
#undef DEFINE_DeeType_RequireSetOperatorAnd
#undef DEFINE_DeeType_RequireSetOperatorXor
#undef DEFINE_DeeType_RequireSetOperatorInplaceAdd
#undef DEFINE_DeeType_RequireSetOperatorInplaceSub
#undef DEFINE_DeeType_RequireSetOperatorInplaceAnd
#undef DEFINE_DeeType_RequireSetOperatorInplaceXor
#undef DEFINE_DeeType_RequireMapOperatorContains
#undef DEFINE_DeeType_RequireMapOperatorGetItem
#undef DEFINE_DeeType_RequireMapOperatorDelItem
#undef DEFINE_DeeType_RequireMapOperatorSetItem
#undef DEFINE_DeeType_RequireMapOperatorEnumerate
#undef DEFINE_DeeType_RequireMapOperatorEnumerateIndex
#undef DEFINE_DeeType_RequireMapOperatorBoundItem
#undef DEFINE_DeeType_RequireMapOperatorHasItem
#undef DEFINE_DeeType_RequireMapOperatorGetItemIndex
#undef DEFINE_DeeType_RequireMapOperatorDelItemIndex
#undef DEFINE_DeeType_RequireMapOperatorSetItemIndex
#undef DEFINE_DeeType_RequireMapOperatorBoundItemIndex
#undef DEFINE_DeeType_RequireMapOperatorHasItemIndex
#undef DEFINE_DeeType_RequireMapOperatorTryGetItem
#undef DEFINE_DeeType_RequireMapOperatorTryGetItemIndex
#undef DEFINE_DeeType_RequireMapOperatorTryGetItemStringHash
#undef DEFINE_DeeType_RequireMapOperatorGetItemStringHash
#undef DEFINE_DeeType_RequireMapOperatorDelItemStringHash
#undef DEFINE_DeeType_RequireMapOperatorSetItemStringHash
#undef DEFINE_DeeType_RequireMapOperatorBoundItemStringHash
#undef DEFINE_DeeType_RequireMapOperatorHasItemStringHash
#undef DEFINE_DeeType_RequireMapOperatorTryGetItemStringLenHash
#undef DEFINE_DeeType_RequireMapOperatorGetItemStringLenHash
#undef DEFINE_DeeType_RequireMapOperatorDelItemStringLenHash
#undef DEFINE_DeeType_RequireMapOperatorSetItemStringLenHash
#undef DEFINE_DeeType_RequireMapOperatorBoundItemStringLenHash
#undef DEFINE_DeeType_RequireMapOperatorHasItemStringLenHash
#undef DEFINE_DeeType_RequireMapOperatorCompareEq
#undef DEFINE_DeeType_RequireMapOperatorTryCompareEq
#undef DEFINE_DeeType_RequireMapOperatorEq
#undef DEFINE_DeeType_RequireMapOperatorNe
#undef DEFINE_DeeType_RequireMapOperatorLo
#undef DEFINE_DeeType_RequireMapOperatorLe
#undef DEFINE_DeeType_RequireMapOperatorGr
#undef DEFINE_DeeType_RequireMapOperatorGe
#undef DEFINE_DeeType_RequireMapOperatorAdd
#undef DEFINE_DeeType_RequireMapOperatorSub
#undef DEFINE_DeeType_RequireMapOperatorAnd
#undef DEFINE_DeeType_RequireMapOperatorXor
#undef DEFINE_DeeType_RequireMapOperatorInplaceAdd
#undef DEFINE_DeeType_RequireMapOperatorInplaceSub
#undef DEFINE_DeeType_RequireMapOperatorInplaceAnd
#undef DEFINE_DeeType_RequireMapOperatorInplaceXor
