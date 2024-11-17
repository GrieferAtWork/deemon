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
#include "default-api-methods.c"
//#define DEFINE_DeeType_SeqCache_RequireAny
//#define DEFINE_DeeType_SeqCache_RequireAnyWithKey
//#define DEFINE_DeeType_SeqCache_RequireAnyWithRange
//#define DEFINE_DeeType_SeqCache_RequireAnyWithRangeAndKey
//#define DEFINE_DeeType_SeqCache_RequireAll
//#define DEFINE_DeeType_SeqCache_RequireAllWithKey
//#define DEFINE_DeeType_SeqCache_RequireAllWithRange
//#define DEFINE_DeeType_SeqCache_RequireAllWithRangeAndKey
//#define DEFINE_DeeType_SeqCache_RequireParity
//#define DEFINE_DeeType_SeqCache_RequireParityWithKey
//#define DEFINE_DeeType_SeqCache_RequireParityWithRange
//#define DEFINE_DeeType_SeqCache_RequireParityWithRangeAndKey
//#define DEFINE_DeeType_SeqCache_RequireReduce
//#define DEFINE_DeeType_SeqCache_RequireReduceWithInit
//#define DEFINE_DeeType_SeqCache_RequireReduceWithRange
//#define DEFINE_DeeType_SeqCache_RequireReduceWithRangeAndInit
//#define DEFINE_DeeType_SeqCache_RequireMin
//#define DEFINE_DeeType_SeqCache_RequireMinWithKey
//#define DEFINE_DeeType_SeqCache_RequireMinWithRange
//#define DEFINE_DeeType_SeqCache_RequireMinWithRangeAndKey
//#define DEFINE_DeeType_SeqCache_RequireMax
//#define DEFINE_DeeType_SeqCache_RequireMaxWithKey
//#define DEFINE_DeeType_SeqCache_RequireMaxWithRange
//#define DEFINE_DeeType_SeqCache_RequireMaxWithRangeAndKey
//#define DEFINE_DeeType_SeqCache_RequireSum
//#define DEFINE_DeeType_SeqCache_RequireSumWithRange
//#define DEFINE_DeeType_SeqCache_RequireCount
//#define DEFINE_DeeType_SeqCache_RequireCountWithKey
//#define DEFINE_DeeType_SeqCache_RequireCountWithRange
//#define DEFINE_DeeType_SeqCache_RequireCountWithRangeAndKey
//#define DEFINE_DeeType_SeqCache_RequireContains
//#define DEFINE_DeeType_SeqCache_RequireContainsWithKey
//#define DEFINE_DeeType_SeqCache_RequireContainsWithRange
//#define DEFINE_DeeType_SeqCache_RequireContainsWithRangeAndKey
//#define DEFINE_DeeType_SeqCache_RequireLocate
//#define DEFINE_DeeType_SeqCache_RequireLocateWithKey
//#define DEFINE_DeeType_SeqCache_RequireLocateWithRange
//#define DEFINE_DeeType_SeqCache_RequireLocateWithRangeAndKey
//#define DEFINE_DeeType_SeqCache_RequireRLocateWithRange
//#define DEFINE_DeeType_SeqCache_RequireRLocateWithRangeAndKey
//#define DEFINE_DeeType_SeqCache_RequireStartsWith
//#define DEFINE_DeeType_SeqCache_RequireStartsWithWithKey
//#define DEFINE_DeeType_SeqCache_RequireStartsWithWithRange
//#define DEFINE_DeeType_SeqCache_RequireStartsWithWithRangeAndKey
//#define DEFINE_DeeType_SeqCache_RequireEndsWith
//#define DEFINE_DeeType_SeqCache_RequireEndsWithWithKey
//#define DEFINE_DeeType_SeqCache_RequireEndsWithWithRange
//#define DEFINE_DeeType_SeqCache_RequireEndsWithWithRangeAndKey
//#define DEFINE_DeeType_SeqCache_RequireFind
//#define DEFINE_DeeType_SeqCache_RequireFindWithKey
//#define DEFINE_DeeType_SeqCache_RequireRFind
//#define DEFINE_DeeType_SeqCache_RequireRFindWithKey
//#define DEFINE_DeeType_SeqCache_RequireErase
//#define DEFINE_DeeType_SeqCache_RequireInsert
//#define DEFINE_DeeType_SeqCache_RequireInsertAll
//#define DEFINE_DeeType_SeqCache_RequirePushFront
//#define DEFINE_DeeType_SeqCache_RequireAppend
//#define DEFINE_DeeType_SeqCache_RequireExtend
//#define DEFINE_DeeType_SeqCache_RequireXchItemIndex
//#define DEFINE_DeeType_SeqCache_RequireClear
//#define DEFINE_DeeType_SeqCache_RequirePop
//#define DEFINE_DeeType_SeqCache_RequireRemove
//#define DEFINE_DeeType_SeqCache_RequireRemoveWithKey
//#define DEFINE_DeeType_SeqCache_RequireRRemove
//#define DEFINE_DeeType_SeqCache_RequireRRemoveWithKey
//#define DEFINE_DeeType_SeqCache_RequireRemoveAll
//#define DEFINE_DeeType_SeqCache_RequireRemoveAllWithKey
//#define DEFINE_DeeType_SeqCache_RequireRemoveIf
//#define DEFINE_DeeType_SeqCache_RequireResize
//#define DEFINE_DeeType_SeqCache_RequireFill
//#define DEFINE_DeeType_SeqCache_RequireReverse
//#define DEFINE_DeeType_SeqCache_RequireReversed
//#define DEFINE_DeeType_SeqCache_RequireSort
//#define DEFINE_DeeType_SeqCache_RequireSortWithKey
//#define DEFINE_DeeType_SeqCache_RequireSorted
//#define DEFINE_DeeType_SeqCache_RequireSortedWithKey
//#define DEFINE_DeeType_SeqCache_RequireBFind
//#define DEFINE_DeeType_SeqCache_RequireBFindWithKey
//#define DEFINE_DeeType_SeqCache_RequireBPosition
//#define DEFINE_DeeType_SeqCache_RequireBPositionWithKey
//#define DEFINE_DeeType_SeqCache_RequireBRange
//#define DEFINE_DeeType_SeqCache_RequireBRangeWithKey
//#define DEFINE_DeeType_SeqCache_RequireBLocate
//#define DEFINE_DeeType_SeqCache_RequireBLocateWithKey
//#define DEFINE_DeeType_SeqCache_RequireSetInsert
//#define DEFINE_DeeType_SeqCache_RequireSetRemove
//#define DEFINE_DeeType_SeqCache_RequireSetUnify
//#define DEFINE_DeeType_SeqCache_RequireSetInsertAll
//#define DEFINE_DeeType_SeqCache_RequireSetRemoveAll
//#define DEFINE_DeeType_SeqCache_RequireSetPop
//#define DEFINE_DeeType_SeqCache_RequireSetPopWithDefault
//#define DEFINE_DeeType_SeqCache_RequireMapSetOld
//#define DEFINE_DeeType_SeqCache_RequireMapSetOldEx
//#define DEFINE_DeeType_SeqCache_RequireMapSetNew
//#define DEFINE_DeeType_SeqCache_RequireMapSetNewEx
//#define DEFINE_DeeType_SeqCache_RequireMapSetDefault
//#define DEFINE_DeeType_SeqCache_RequireMapUpdate
//#define DEFINE_DeeType_SeqCache_RequireMapRemove
//#define DEFINE_DeeType_SeqCache_RequireMapRemoveKeys
//#define DEFINE_DeeType_SeqCache_RequireMapPop
//#define DEFINE_DeeType_SeqCache_RequireMapPopWithDefault
//#define DEFINE_DeeType_SeqCache_RequireMapPopItem
#define DEFINE_DeeType_SeqCache_RequireMapKeys
//#define DEFINE_DeeType_SeqCache_RequireMapValues
//#define DEFINE_DeeType_SeqCache_RequireMapIterKeys
//#define DEFINE_DeeType_SeqCache_RequireMapIterValues
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeType_SeqCache_RequireAny) +                       \
     defined(DEFINE_DeeType_SeqCache_RequireAnyWithKey) +                \
     defined(DEFINE_DeeType_SeqCache_RequireAnyWithRange) +              \
     defined(DEFINE_DeeType_SeqCache_RequireAnyWithRangeAndKey) +        \
     defined(DEFINE_DeeType_SeqCache_RequireAll) +                       \
     defined(DEFINE_DeeType_SeqCache_RequireAllWithKey) +                \
     defined(DEFINE_DeeType_SeqCache_RequireAllWithRange) +              \
     defined(DEFINE_DeeType_SeqCache_RequireAllWithRangeAndKey) +        \
     defined(DEFINE_DeeType_SeqCache_RequireParity) +                    \
     defined(DEFINE_DeeType_SeqCache_RequireParityWithKey) +             \
     defined(DEFINE_DeeType_SeqCache_RequireParityWithRange) +           \
     defined(DEFINE_DeeType_SeqCache_RequireParityWithRangeAndKey) +     \
     defined(DEFINE_DeeType_SeqCache_RequireReduce) +                    \
     defined(DEFINE_DeeType_SeqCache_RequireReduceWithInit) +            \
     defined(DEFINE_DeeType_SeqCache_RequireReduceWithRange) +           \
     defined(DEFINE_DeeType_SeqCache_RequireReduceWithRangeAndInit) +    \
     defined(DEFINE_DeeType_SeqCache_RequireMin) +                       \
     defined(DEFINE_DeeType_SeqCache_RequireMinWithKey) +                \
     defined(DEFINE_DeeType_SeqCache_RequireMinWithRange) +              \
     defined(DEFINE_DeeType_SeqCache_RequireMinWithRangeAndKey) +        \
     defined(DEFINE_DeeType_SeqCache_RequireMax) +                       \
     defined(DEFINE_DeeType_SeqCache_RequireMaxWithKey) +                \
     defined(DEFINE_DeeType_SeqCache_RequireMaxWithRange) +              \
     defined(DEFINE_DeeType_SeqCache_RequireMaxWithRangeAndKey) +        \
     defined(DEFINE_DeeType_SeqCache_RequireSum) +                       \
     defined(DEFINE_DeeType_SeqCache_RequireSumWithRange) +              \
     defined(DEFINE_DeeType_SeqCache_RequireCount) +                     \
     defined(DEFINE_DeeType_SeqCache_RequireCountWithKey) +              \
     defined(DEFINE_DeeType_SeqCache_RequireCountWithRange) +            \
     defined(DEFINE_DeeType_SeqCache_RequireCountWithRangeAndKey) +      \
     defined(DEFINE_DeeType_SeqCache_RequireContains) +                  \
     defined(DEFINE_DeeType_SeqCache_RequireContainsWithKey) +           \
     defined(DEFINE_DeeType_SeqCache_RequireContainsWithRange) +         \
     defined(DEFINE_DeeType_SeqCache_RequireContainsWithRangeAndKey) +   \
     defined(DEFINE_DeeType_SeqCache_RequireLocate) +                    \
     defined(DEFINE_DeeType_SeqCache_RequireLocateWithKey) +             \
     defined(DEFINE_DeeType_SeqCache_RequireLocateWithRange) +           \
     defined(DEFINE_DeeType_SeqCache_RequireLocateWithRangeAndKey) +     \
     defined(DEFINE_DeeType_SeqCache_RequireRLocateWithRange) +          \
     defined(DEFINE_DeeType_SeqCache_RequireRLocateWithRangeAndKey) +    \
     defined(DEFINE_DeeType_SeqCache_RequireStartsWith) +                \
     defined(DEFINE_DeeType_SeqCache_RequireStartsWithWithKey) +         \
     defined(DEFINE_DeeType_SeqCache_RequireStartsWithWithRange) +       \
     defined(DEFINE_DeeType_SeqCache_RequireStartsWithWithRangeAndKey) + \
     defined(DEFINE_DeeType_SeqCache_RequireEndsWith) +                  \
     defined(DEFINE_DeeType_SeqCache_RequireEndsWithWithKey) +           \
     defined(DEFINE_DeeType_SeqCache_RequireEndsWithWithRange) +         \
     defined(DEFINE_DeeType_SeqCache_RequireEndsWithWithRangeAndKey) +   \
     defined(DEFINE_DeeType_SeqCache_RequireFind) +                      \
     defined(DEFINE_DeeType_SeqCache_RequireFindWithKey) +               \
     defined(DEFINE_DeeType_SeqCache_RequireRFind) +                     \
     defined(DEFINE_DeeType_SeqCache_RequireRFindWithKey) +              \
     defined(DEFINE_DeeType_SeqCache_RequireErase) +                     \
     defined(DEFINE_DeeType_SeqCache_RequireInsert) +                    \
     defined(DEFINE_DeeType_SeqCache_RequireInsertAll) +                 \
     defined(DEFINE_DeeType_SeqCache_RequirePushFront) +                 \
     defined(DEFINE_DeeType_SeqCache_RequireAppend) +                    \
     defined(DEFINE_DeeType_SeqCache_RequireExtend) +                    \
     defined(DEFINE_DeeType_SeqCache_RequireXchItemIndex) +              \
     defined(DEFINE_DeeType_SeqCache_RequireClear) +                     \
     defined(DEFINE_DeeType_SeqCache_RequirePop) +                       \
     defined(DEFINE_DeeType_SeqCache_RequireRemove) +                    \
     defined(DEFINE_DeeType_SeqCache_RequireRemoveWithKey) +             \
     defined(DEFINE_DeeType_SeqCache_RequireRRemove) +                   \
     defined(DEFINE_DeeType_SeqCache_RequireRRemoveWithKey) +            \
     defined(DEFINE_DeeType_SeqCache_RequireRemoveAll) +                 \
     defined(DEFINE_DeeType_SeqCache_RequireRemoveAllWithKey) +          \
     defined(DEFINE_DeeType_SeqCache_RequireRemoveIf) +                  \
     defined(DEFINE_DeeType_SeqCache_RequireResize) +                    \
     defined(DEFINE_DeeType_SeqCache_RequireFill) +                      \
     defined(DEFINE_DeeType_SeqCache_RequireReverse) +                   \
     defined(DEFINE_DeeType_SeqCache_RequireReversed) +                  \
     defined(DEFINE_DeeType_SeqCache_RequireSort) +                      \
     defined(DEFINE_DeeType_SeqCache_RequireSortWithKey) +               \
     defined(DEFINE_DeeType_SeqCache_RequireSorted) +                    \
     defined(DEFINE_DeeType_SeqCache_RequireSortedWithKey) +             \
     defined(DEFINE_DeeType_SeqCache_RequireBFind) +                     \
     defined(DEFINE_DeeType_SeqCache_RequireBFindWithKey) +              \
     defined(DEFINE_DeeType_SeqCache_RequireBPosition) +                 \
     defined(DEFINE_DeeType_SeqCache_RequireBPositionWithKey) +          \
     defined(DEFINE_DeeType_SeqCache_RequireBRange) +                    \
     defined(DEFINE_DeeType_SeqCache_RequireBRangeWithKey) +             \
     defined(DEFINE_DeeType_SeqCache_RequireBLocate) +                   \
     defined(DEFINE_DeeType_SeqCache_RequireBLocateWithKey) +            \
     defined(DEFINE_DeeType_SeqCache_RequireSetInsert) +                 \
     defined(DEFINE_DeeType_SeqCache_RequireSetRemove) +                 \
     defined(DEFINE_DeeType_SeqCache_RequireSetUnify) +                  \
     defined(DEFINE_DeeType_SeqCache_RequireSetInsertAll) +              \
     defined(DEFINE_DeeType_SeqCache_RequireSetRemoveAll) +              \
     defined(DEFINE_DeeType_SeqCache_RequireSetPop) +                    \
     defined(DEFINE_DeeType_SeqCache_RequireSetPopWithDefault) +         \
     defined(DEFINE_DeeType_SeqCache_RequireMapSetOld) +                 \
     defined(DEFINE_DeeType_SeqCache_RequireMapSetOldEx) +               \
     defined(DEFINE_DeeType_SeqCache_RequireMapSetNew) +                 \
     defined(DEFINE_DeeType_SeqCache_RequireMapSetNewEx) +               \
     defined(DEFINE_DeeType_SeqCache_RequireMapSetDefault) +             \
     defined(DEFINE_DeeType_SeqCache_RequireMapUpdate) +                 \
     defined(DEFINE_DeeType_SeqCache_RequireMapRemove) +                 \
     defined(DEFINE_DeeType_SeqCache_RequireMapRemoveKeys) +             \
     defined(DEFINE_DeeType_SeqCache_RequireMapPop) +                    \
     defined(DEFINE_DeeType_SeqCache_RequireMapPopWithDefault) +         \
     defined(DEFINE_DeeType_SeqCache_RequireMapPopItem) +                \
     defined(DEFINE_DeeType_SeqCache_RequireMapKeys) +                   \
     defined(DEFINE_DeeType_SeqCache_RequireMapValues) +                 \
     defined(DEFINE_DeeType_SeqCache_RequireMapIterKeys) +               \
     defined(DEFINE_DeeType_SeqCache_RequireMapIterValues)) != 1
#error "Must #define exactly one of these macros"
#endif /* DEFINE_DeeType_SeqCache_Require... */

DECL_BEGIN

#ifdef DEFINE_DeeType_SeqCache_RequireAny
#define LOCAL_CANONICAL_NAME             any
#define LOCAL_default_seq_foo            default_seq_any
#define LOCAL_tsc_foo                    tsc_any
#define LOCAL_DeeSeq_AttrFoo             Any
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAnyWithForeach
#elif defined(DEFINE_DeeType_SeqCache_RequireAnyWithKey)
#define LOCAL_CANONICAL_NAME             any
#define LOCAL_default_seq_foo            default_seq_any
#define LOCAL_tsc_foo                    tsc_any_with_key
#define LOCAL_DeeSeq_AttrFoo             AnyWithKey
#define LOCAL_DeeSeq_AttrBar             Any
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAnyWithKeyWithForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_SeqCache_RequireAnyWithRange)
#define LOCAL_CANONICAL_NAME             any
#define LOCAL_default_seq_foo            default_seq_any
#define LOCAL_tsc_foo                    tsc_any_with_range
#define LOCAL_DeeSeq_AttrFoo             AnyWithRange
#define LOCAL_DeeSeq_AttrBar             Any
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAnyWithRangeWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireAnyWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             any
#define LOCAL_default_seq_foo            default_seq_any
#define LOCAL_tsc_foo                    tsc_any_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             AnyWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             Any
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAnyWithRangeAndKeyWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireAll)
#define LOCAL_CANONICAL_NAME             all
#define LOCAL_default_seq_foo            default_seq_all
#define LOCAL_tsc_foo                    tsc_all
#define LOCAL_DeeSeq_AttrFoo             All
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAllWithForeach
#elif defined(DEFINE_DeeType_SeqCache_RequireAllWithKey)
#define LOCAL_CANONICAL_NAME             all
#define LOCAL_default_seq_foo            default_seq_all
#define LOCAL_tsc_foo                    tsc_all_with_key
#define LOCAL_DeeSeq_AttrFoo             AllWithKey
#define LOCAL_DeeSeq_AttrBar             All
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAllWithKeyWithForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_SeqCache_RequireAllWithRange)
#define LOCAL_CANONICAL_NAME             all
#define LOCAL_default_seq_foo            default_seq_all
#define LOCAL_tsc_foo                    tsc_all_with_range
#define LOCAL_DeeSeq_AttrFoo             AllWithRange
#define LOCAL_DeeSeq_AttrBar             All
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAllWithRangeWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireAllWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             all
#define LOCAL_default_seq_foo            default_seq_all
#define LOCAL_tsc_foo                    tsc_all_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             AllWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             All
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAllWithRangeAndKeyWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireParity)
#define LOCAL_CANONICAL_NAME             parity
#define LOCAL_default_seq_foo            default_seq_parity
#define LOCAL_tsc_foo                    tsc_parity
#define LOCAL_DeeSeq_AttrFoo             Parity
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultParityWithForeach
#elif defined(DEFINE_DeeType_SeqCache_RequireParityWithKey)
#define LOCAL_CANONICAL_NAME             parity
#define LOCAL_default_seq_foo            default_seq_parity
#define LOCAL_tsc_foo                    tsc_parity_with_key
#define LOCAL_DeeSeq_AttrFoo             ParityWithKey
#define LOCAL_DeeSeq_AttrBar             Parity
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultParityWithKeyWithForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_SeqCache_RequireParityWithRange)
#define LOCAL_CANONICAL_NAME             parity
#define LOCAL_default_seq_foo            default_seq_parity
#define LOCAL_tsc_foo                    tsc_parity_with_range
#define LOCAL_DeeSeq_AttrFoo             ParityWithRange
#define LOCAL_DeeSeq_AttrBar             Parity
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultParityWithRangeWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireParityWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             parity
#define LOCAL_default_seq_foo            default_seq_parity
#define LOCAL_tsc_foo                    tsc_parity_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             ParityWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             Parity
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultParityWithRangeAndKeyWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireReduce)
#define LOCAL_CANONICAL_NAME             reduce
#define LOCAL_default_seq_foo            default_seq_reduce
#define LOCAL_tsc_foo                    tsc_reduce
#define LOCAL_DeeSeq_AttrFoo             Reduce
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReduceWithForeach
#elif defined(DEFINE_DeeType_SeqCache_RequireReduceWithInit)
#define LOCAL_CANONICAL_NAME             reduce
#define LOCAL_default_seq_foo            default_seq_reduce
#define LOCAL_tsc_foo                    tsc_reduce_with_init
#define LOCAL_DeeSeq_AttrFoo             ReduceWithInit
#define LOCAL_DeeSeq_AttrBar             Reduce
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReduceWithInitWithForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_SeqCache_RequireReduceWithRange)
#define LOCAL_CANONICAL_NAME             reduce
#define LOCAL_default_seq_foo            default_seq_reduce
#define LOCAL_tsc_foo                    tsc_reduce_with_range
#define LOCAL_DeeSeq_AttrFoo             ReduceWithRange
#define LOCAL_DeeSeq_AttrBar             Reduce
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReduceWithRangeWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireReduceWithRangeAndInit)
#define LOCAL_CANONICAL_NAME             reduce
#define LOCAL_default_seq_foo            default_seq_reduce
#define LOCAL_tsc_foo                    tsc_reduce_with_range_and_init
#define LOCAL_DeeSeq_AttrFoo             ReduceWithRangeAndInit
#define LOCAL_DeeSeq_AttrBar             Reduce
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReduceWithRangeAndInitWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireMin)
#define LOCAL_CANONICAL_NAME             min
#define LOCAL_default_seq_foo            default_seq_min
#define LOCAL_tsc_foo                    tsc_min
#define LOCAL_DeeSeq_AttrFoo             Min
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMinWithForeach
#elif defined(DEFINE_DeeType_SeqCache_RequireMinWithKey)
#define LOCAL_CANONICAL_NAME             min
#define LOCAL_default_seq_foo            default_seq_min
#define LOCAL_tsc_foo                    tsc_min_with_key
#define LOCAL_DeeSeq_AttrFoo             MinWithKey
#define LOCAL_DeeSeq_AttrBar             Min
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMinWithKeyWithForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_SeqCache_RequireMinWithRange)
#define LOCAL_CANONICAL_NAME             min
#define LOCAL_default_seq_foo            default_seq_min
#define LOCAL_tsc_foo                    tsc_min_with_range
#define LOCAL_DeeSeq_AttrFoo             MinWithRange
#define LOCAL_DeeSeq_AttrBar             Min
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMinWithRangeWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireMinWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             min
#define LOCAL_default_seq_foo            default_seq_min
#define LOCAL_tsc_foo                    tsc_min_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             MinWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             Min
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMinWithRangeAndKeyWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireMax)
#define LOCAL_CANONICAL_NAME             max
#define LOCAL_default_seq_foo            default_seq_max
#define LOCAL_tsc_foo                    tsc_max
#define LOCAL_DeeSeq_AttrFoo             Max
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMaxWithForeach
#elif defined(DEFINE_DeeType_SeqCache_RequireMaxWithKey)
#define LOCAL_CANONICAL_NAME             max
#define LOCAL_default_seq_foo            default_seq_max
#define LOCAL_tsc_foo                    tsc_max_with_key
#define LOCAL_DeeSeq_AttrFoo             MaxWithKey
#define LOCAL_DeeSeq_AttrBar             Max
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMaxWithKeyWithForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_SeqCache_RequireMaxWithRange)
#define LOCAL_CANONICAL_NAME             max
#define LOCAL_default_seq_foo            default_seq_max
#define LOCAL_tsc_foo                    tsc_max_with_range
#define LOCAL_DeeSeq_AttrFoo             MaxWithRange
#define LOCAL_DeeSeq_AttrBar             Max
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMaxWithRangeWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireMaxWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             max
#define LOCAL_default_seq_foo            default_seq_max
#define LOCAL_tsc_foo                    tsc_max_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             MaxWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             Max
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMaxWithRangeAndKeyWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireSum)
#define LOCAL_CANONICAL_NAME             sum
#define LOCAL_default_seq_foo            default_seq_sum
#define LOCAL_tsc_foo                    tsc_sum
#define LOCAL_DeeSeq_AttrFoo             Sum
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultSumWithForeach
#elif defined(DEFINE_DeeType_SeqCache_RequireSumWithRange)
#define LOCAL_CANONICAL_NAME             sum
#define LOCAL_default_seq_foo            default_seq_sum
#define LOCAL_tsc_foo                    tsc_sum_with_range
#define LOCAL_DeeSeq_AttrFoo             SumWithRange
#define LOCAL_DeeSeq_AttrBar             Sum
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultSumWithRangeWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireCount)
#define LOCAL_CANONICAL_NAME             count
#define LOCAL_default_seq_foo            default_seq_count
#define LOCAL_tsc_foo                    tsc_count
#define LOCAL_DeeSeq_AttrFoo             Count
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultCountWithForeach
#elif defined(DEFINE_DeeType_SeqCache_RequireCountWithKey)
#define LOCAL_CANONICAL_NAME             count
#define LOCAL_default_seq_foo            default_seq_count
#define LOCAL_tsc_foo                    tsc_count_with_key
#define LOCAL_DeeSeq_AttrFoo             CountWithKey
#define LOCAL_DeeSeq_AttrBar             Count
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultCountWithKeyWithForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_SeqCache_RequireCountWithRange)
#define LOCAL_CANONICAL_NAME             count
#define LOCAL_default_seq_foo            default_seq_count
#define LOCAL_tsc_foo                    tsc_count_with_range
#define LOCAL_DeeSeq_AttrFoo             CountWithRange
#define LOCAL_DeeSeq_AttrBar             Count
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultCountWithRangeWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireCountWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             count
#define LOCAL_default_seq_foo            default_seq_count
#define LOCAL_tsc_foo                    tsc_count_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             CountWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             Count
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultCountWithRangeAndKeyWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireContains)
#define LOCAL_CANONICAL_NAME             contains
#define LOCAL_default_seq_foo            default_seq_contains
#define LOCAL_tsc_foo                    tsc_contains
#define LOCAL_DeeSeq_AttrFoo             Contains
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultContainsWithForeach
#elif defined(DEFINE_DeeType_SeqCache_RequireContainsWithKey)
#define LOCAL_CANONICAL_NAME             contains
#define LOCAL_default_seq_foo            default_seq_contains
#define LOCAL_tsc_foo                    tsc_contains_with_key
#define LOCAL_DeeSeq_AttrFoo             ContainsWithKey
#define LOCAL_DeeSeq_AttrBar             Contains
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultContainsWithKeyWithForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_SeqCache_RequireContainsWithRange)
#define LOCAL_CANONICAL_NAME             contains
#define LOCAL_default_seq_foo            default_seq_contains
#define LOCAL_tsc_foo                    tsc_contains_with_range
#define LOCAL_DeeSeq_AttrFoo             ContainsWithRange
#define LOCAL_DeeSeq_AttrBar             Contains
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultContainsWithRangeWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireContainsWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             contains
#define LOCAL_default_seq_foo            default_seq_contains
#define LOCAL_tsc_foo                    tsc_contains_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             ContainsWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             Contains
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultContainsWithRangeAndKeyWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireLocate)
#define LOCAL_CANONICAL_NAME             locate
#define LOCAL_default_seq_foo            default_seq_locate
#define LOCAL_tsc_foo                    tsc_locate
#define LOCAL_DeeSeq_AttrFoo             Locate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultLocateWithForeach
#elif defined(DEFINE_DeeType_SeqCache_RequireLocateWithKey)
#define LOCAL_CANONICAL_NAME             locate
#define LOCAL_default_seq_foo            default_seq_locate
#define LOCAL_tsc_foo                    tsc_locate_with_key
#define LOCAL_DeeSeq_AttrFoo             LocateWithKey
#define LOCAL_DeeSeq_AttrBar             Locate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultLocateWithKeyWithForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_SeqCache_RequireLocateWithRange)
#define LOCAL_CANONICAL_NAME             locate
#define LOCAL_default_seq_foo            default_seq_locate
#define LOCAL_tsc_foo                    tsc_locate_with_range
#define LOCAL_DeeSeq_AttrFoo             LocateWithRange
#define LOCAL_DeeSeq_AttrBar             Locate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultLocateWithRangeWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireLocateWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             locate
#define LOCAL_default_seq_foo            default_seq_locate
#define LOCAL_tsc_foo                    tsc_locate_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             LocateWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             Locate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultLocateWithRangeAndKeyWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireRLocateWithRange)
#define LOCAL_CANONICAL_NAME             rlocate
#define LOCAL_default_seq_foo            default_seq_rlocate
#define LOCAL_tsc_foo                    tsc_rlocate_with_range
#define LOCAL_DeeSeq_AttrFoo             RLocateWithRange
#define LOCAL_DeeSeq_AttrBar             RLocate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultRLocateWithRangeWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireRLocateWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             rlocate
#define LOCAL_default_seq_foo            default_seq_rlocate
#define LOCAL_tsc_foo                    tsc_rlocate_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             RLocateWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             RLocate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultRLocateWithRangeAndKeyWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireStartsWith)
#define LOCAL_CANONICAL_NAME             startswith
#define LOCAL_default_seq_foo            default_seq_startswith
#define LOCAL_tsc_foo                    tsc_startswith
#define LOCAL_DeeSeq_AttrFoo             StartsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultStartsWithWithTSCFirst
#elif defined(DEFINE_DeeType_SeqCache_RequireStartsWithWithKey)
#define LOCAL_CANONICAL_NAME             startswith
#define LOCAL_default_seq_foo            default_seq_startswith
#define LOCAL_tsc_foo                    tsc_startswith_with_key
#define LOCAL_DeeSeq_AttrFoo             StartsWithWithKey
#define LOCAL_DeeSeq_AttrBar             StartsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultStartsWithWithKeyWithTSCFirst
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_SeqCache_RequireStartsWithWithRange)
#define LOCAL_CANONICAL_NAME             startswith
#define LOCAL_default_seq_foo            default_seq_startswith
#define LOCAL_tsc_foo                    tsc_startswith_with_range
#define LOCAL_DeeSeq_AttrFoo             StartsWithWithRange
#define LOCAL_DeeSeq_AttrBar             StartsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultStartsWithWithRangeWithTryGetItemIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireStartsWithWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             startswith
#define LOCAL_default_seq_foo            default_seq_startswith
#define LOCAL_tsc_foo                    tsc_startswith_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             StartsWithWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             StartsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultStartsWithWithRangeAndKeyWithTryGetItemIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireEndsWith)
#define LOCAL_CANONICAL_NAME             endswith
#define LOCAL_default_seq_foo            default_seq_endswith
#define LOCAL_tsc_foo                    tsc_endswith
#define LOCAL_DeeSeq_AttrFoo             EndsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultEndsWithWithTSCLast
#elif defined(DEFINE_DeeType_SeqCache_RequireEndsWithWithKey)
#define LOCAL_CANONICAL_NAME             endswith
#define LOCAL_default_seq_foo            default_seq_endswith
#define LOCAL_tsc_foo                    tsc_endswith_with_key
#define LOCAL_DeeSeq_AttrFoo             EndsWithWithKey
#define LOCAL_DeeSeq_AttrBar             EndsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultEndsWithWithKeyWithTSCLast
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_SeqCache_RequireEndsWithWithRange)
#define LOCAL_CANONICAL_NAME             endswith
#define LOCAL_default_seq_foo            default_seq_endswith
#define LOCAL_tsc_foo                    tsc_endswith_with_range
#define LOCAL_DeeSeq_AttrFoo             EndsWithWithRange
#define LOCAL_DeeSeq_AttrBar             EndsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultEndsWithWithRangeWithSizeAndTryGetItemIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireEndsWithWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             endswith
#define LOCAL_default_seq_foo            default_seq_endswith
#define LOCAL_tsc_foo                    tsc_endswith_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             EndsWithWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             EndsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultEndsWithWithRangeAndKeyWithSizeAndTryGetItemIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireFind)
#define LOCAL_CANONICAL_NAME             find
#define LOCAL_default_seq_foo            default_seq_find
#define LOCAL_tsc_foo                    tsc_find
#define LOCAL_DeeSeq_AttrFoo             Find
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultFindWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireFindWithKey)
#define LOCAL_CANONICAL_NAME             find
#define LOCAL_default_seq_foo            default_seq_find
#define LOCAL_tsc_foo                    tsc_find_with_key
#define LOCAL_DeeSeq_AttrFoo             FindWithKey
#define LOCAL_DeeSeq_AttrBar             Find
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultFindWithKeyWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireRFind)
#define LOCAL_CANONICAL_NAME             rfind
#define LOCAL_default_seq_foo            default_seq_rfind
#define LOCAL_tsc_foo                    tsc_rfind
#define LOCAL_DeeSeq_AttrFoo             RFind
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultRFindWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireRFindWithKey)
#define LOCAL_CANONICAL_NAME             rfind
#define LOCAL_default_seq_foo            default_seq_rfind
#define LOCAL_tsc_foo                    tsc_rfind_with_key
#define LOCAL_DeeSeq_AttrFoo             RFindWithKey
#define LOCAL_DeeSeq_AttrBar             RFind
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultRFindWithKeyWithEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireErase)
#define LOCAL_CANONICAL_NAME         erase
#define LOCAL_default_seq_foo        default_seq_erase
#define LOCAL_tsc_foo                tsc_erase
#define LOCAL_DeeSeq_AttrFoo         Erase
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireInsert)
#define LOCAL_CANONICAL_NAME         insert
#define LOCAL_default_seq_foo        default_seq_insert
#define LOCAL_tsc_foo                tsc_insert
#define LOCAL_DeeSeq_AttrFoo         Insert
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireInsertAll)
#define LOCAL_CANONICAL_NAME         insertall
#define LOCAL_default_seq_foo        default_seq_insertall
#define LOCAL_tsc_foo                tsc_insertall
#define LOCAL_DeeSeq_AttrFoo         InsertAll
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequirePushFront)
#define LOCAL_CANONICAL_NAME             pushfront
#define LOCAL_default_seq_foo            default_seq_pushfront
#define LOCAL_tsc_foo                    tsc_pushfront
#define LOCAL_DeeSeq_AttrFoo             PushFront
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultPushFrontWithTSCInsert /* Use insert() by default */
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireAppend)
#define LOCAL_CANONICAL_NAME         append
#define LOCAL_default_seq_foo        default_seq_append
#define LOCAL_tsc_foo                tsc_append
#define LOCAL_DeeSeq_AttrFoo         Append
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireExtend)
#define LOCAL_CANONICAL_NAME         extend
#define LOCAL_default_seq_foo        default_seq_extend
#define LOCAL_tsc_foo                tsc_extend
#define LOCAL_DeeSeq_AttrFoo         Extend
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireXchItemIndex)
#define LOCAL_CANONICAL_NAME         xchitem
#define LOCAL_default_seq_foo        default_seq_xchitem
#define LOCAL_tsc_foo                tsc_xchitem_index
#define LOCAL_tsc_foo_data           tsc_xchitem_data
#define LOCAL_DeeSeq_AttrFoo         XchItemIndex
#define LOCAL_DeeSeq_AttrBar         XchItem
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireClear)
#define LOCAL_CANONICAL_NAME         clear
#define LOCAL_default_seq_foo        default_seq_clear
#define LOCAL_tsc_foo                tsc_clear
#define LOCAL_DeeSeq_AttrFoo         Clear
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequirePop)
#define LOCAL_CANONICAL_NAME         pop
#define LOCAL_default_seq_foo        default_seq_pop
#define LOCAL_tsc_foo                tsc_pop
#define LOCAL_DeeSeq_AttrFoo         Pop
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireRemove)
#define LOCAL_CANONICAL_NAME         remove
#define LOCAL_default_seq_foo        default_seq_remove
#define LOCAL_tsc_foo                tsc_remove
#define LOCAL_DeeSeq_AttrFoo         Remove
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveWithKey)
#define LOCAL_CANONICAL_NAME         remove
#define LOCAL_default_seq_foo        default_seq_remove
#define LOCAL_tsc_foo                tsc_remove_with_key
#define LOCAL_DeeSeq_AttrFoo         RemoveWithKey
#define LOCAL_DeeSeq_AttrBar         Remove
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireRRemove)
#define LOCAL_CANONICAL_NAME         rremove
#define LOCAL_default_seq_foo        default_seq_rremove
#define LOCAL_tsc_foo                tsc_rremove
#define LOCAL_DeeSeq_AttrFoo         RRemove
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireRRemoveWithKey)
#define LOCAL_CANONICAL_NAME         rremove
#define LOCAL_default_seq_foo        default_seq_rremove
#define LOCAL_tsc_foo                tsc_rremove_with_key
#define LOCAL_DeeSeq_AttrFoo         RRemoveWithKey
#define LOCAL_DeeSeq_AttrBar         RRemove
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveAll)
#define LOCAL_CANONICAL_NAME         removeall
#define LOCAL_default_seq_foo        default_seq_removeall
#define LOCAL_tsc_foo                tsc_removeall
#define LOCAL_DeeSeq_AttrFoo         RemoveAll
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveAllWithKey)
#define LOCAL_CANONICAL_NAME         removeall
#define LOCAL_default_seq_foo        default_seq_removeall
#define LOCAL_tsc_foo                tsc_removeall_with_key
#define LOCAL_DeeSeq_AttrFoo         RemoveAllWithKey
#define LOCAL_DeeSeq_AttrBar         RemoveAll
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveIf)
#define LOCAL_CANONICAL_NAME         removeif
#define LOCAL_default_seq_foo        default_seq_removeif
#define LOCAL_tsc_foo                tsc_removeif
#define LOCAL_DeeSeq_AttrFoo         RemoveIf
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireResize)
#define LOCAL_CANONICAL_NAME             resize
#define LOCAL_default_seq_foo            default_seq_resize
#define LOCAL_tsc_foo                    tsc_resize
#define LOCAL_DeeSeq_AttrFoo             Resize
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultResizeWithSizeAndTSCEraseAndTSCExtend /* Use erase() and extend() by default */
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireFill)
#define LOCAL_CANONICAL_NAME         fill
#define LOCAL_default_seq_foo        default_seq_fill
#define LOCAL_tsc_foo                tsc_fill
#define LOCAL_DeeSeq_AttrFoo         Fill
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireReverse)
#define LOCAL_CANONICAL_NAME         reverse
#define LOCAL_default_seq_foo        default_seq_reverse
#define LOCAL_tsc_foo                tsc_reverse
#define LOCAL_DeeSeq_AttrFoo         Reverse
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireReversed)
#define LOCAL_CANONICAL_NAME             reversed
#define LOCAL_default_seq_foo            default_seq_reversed
#define LOCAL_tsc_foo                    tsc_reversed
#define LOCAL_DeeSeq_AttrFoo             Reversed
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReversedWithCopyForeachDefault
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireSort)
#define LOCAL_CANONICAL_NAME  sort
#define LOCAL_default_seq_foo default_seq_sort
#define LOCAL_tsc_foo         tsc_sort
#define LOCAL_DeeSeq_AttrFoo  Sort
#elif defined(DEFINE_DeeType_SeqCache_RequireSortWithKey)
#define LOCAL_CANONICAL_NAME  sort
#define LOCAL_default_seq_foo default_seq_sort
#define LOCAL_tsc_foo         tsc_sort_with_key
#define LOCAL_DeeSeq_AttrFoo  SortWithKey
#define LOCAL_DeeSeq_AttrBar  Sort
#elif defined(DEFINE_DeeType_SeqCache_RequireSorted)
#define LOCAL_CANONICAL_NAME             sorted
#define LOCAL_default_seq_foo            default_seq_sorted
#define LOCAL_tsc_foo                    tsc_sorted
#define LOCAL_DeeSeq_AttrFoo             Sorted
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultSortedWithCopyForeachDefault
#elif defined(DEFINE_DeeType_SeqCache_RequireSortedWithKey)
#define LOCAL_CANONICAL_NAME             sorted
#define LOCAL_default_seq_foo            default_seq_sorted
#define LOCAL_tsc_foo                    tsc_sorted_with_key
#define LOCAL_DeeSeq_AttrFoo             SortedWithKey
#define LOCAL_DeeSeq_AttrBar             Sorted
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultSortedWithKeyWithCopyForeachDefault
#elif defined(DEFINE_DeeType_SeqCache_RequireBFind)
#define LOCAL_CANONICAL_NAME         bfind
#define LOCAL_default_seq_foo        default_seq_bfind
#define LOCAL_tsc_foo                tsc_bfind
#define LOCAL_DeeSeq_AttrFoo         BFind
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireBFindWithKey)
#define LOCAL_CANONICAL_NAME         bfind
#define LOCAL_default_seq_foo        default_seq_bfind
#define LOCAL_tsc_foo                tsc_bfind_with_key
#define LOCAL_DeeSeq_AttrFoo         BFindWithKey
#define LOCAL_DeeSeq_AttrBar         BFind
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireBPosition)
#define LOCAL_CANONICAL_NAME         bposition
#define LOCAL_default_seq_foo        default_seq_bposition
#define LOCAL_tsc_foo                tsc_bposition
#define LOCAL_DeeSeq_AttrFoo         BPosition
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireBPositionWithKey)
#define LOCAL_CANONICAL_NAME         bposition
#define LOCAL_default_seq_foo        default_seq_bposition
#define LOCAL_tsc_foo                tsc_bposition_with_key
#define LOCAL_DeeSeq_AttrFoo         BPositionWithKey
#define LOCAL_DeeSeq_AttrBar         BPosition
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireBRange)
#define LOCAL_CANONICAL_NAME         brange
#define LOCAL_default_seq_foo        default_seq_brange
#define LOCAL_tsc_foo                tsc_brange
#define LOCAL_DeeSeq_AttrFoo         BRange
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireBRangeWithKey)
#define LOCAL_CANONICAL_NAME         brange
#define LOCAL_default_seq_foo        default_seq_brange
#define LOCAL_tsc_foo                tsc_brange_with_key
#define LOCAL_DeeSeq_AttrFoo         BRangeWithKey
#define LOCAL_DeeSeq_AttrBar         BRange
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireBLocate)
#define LOCAL_CANONICAL_NAME         blocate
#define LOCAL_default_seq_foo        default_seq_blocate
#define LOCAL_tsc_foo                tsc_blocate
#define LOCAL_DeeSeq_AttrFoo         BLocate
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireBLocateWithKey)
#define LOCAL_CANONICAL_NAME         blocate
#define LOCAL_default_seq_foo        default_seq_blocate
#define LOCAL_tsc_foo                tsc_blocate_with_key
#define LOCAL_DeeSeq_AttrFoo         BLocateWithKey
#define LOCAL_DeeSeq_AttrBar         BLocate
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_SeqCache_RequireSetInsert)
#define LOCAL_CANONICAL_NAME         insert
#define LOCAL_default_seq_foo        default_set_insert
#define LOCAL_tsc_foo                tsc_set_insert
#define LOCAL_tsc_foo_data           tsc_set_insert_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default         DeeSet_Default
#define LOCAL_DeeSeq_RequireFoo      SetInsert
#define LOCAL_DeeSeq_AttrFoo         Insert
#elif defined(DEFINE_DeeType_SeqCache_RequireSetRemove)
#define LOCAL_CANONICAL_NAME         remove
#define LOCAL_default_seq_foo        default_set_remove
#define LOCAL_tsc_foo                tsc_set_remove
#define LOCAL_tsc_foo_data           tsc_set_remove_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default         DeeSet_Default
#define LOCAL_DeeSeq_RequireFoo      SetRemove
#define LOCAL_DeeSeq_AttrFoo         Remove
#elif defined(DEFINE_DeeType_SeqCache_RequireSetUnify)
#define LOCAL_CANONICAL_NAME         unify
#define LOCAL_default_seq_foo        default_set_unify
#define LOCAL_tsc_foo                tsc_set_unify
#define LOCAL_tsc_foo_data           tsc_set_unify_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default         DeeSet_Default
#define LOCAL_DeeSeq_RequireFoo      SetUnify
#define LOCAL_DeeSeq_AttrFoo         Unify
#elif defined(DEFINE_DeeType_SeqCache_RequireSetInsertAll)
#define LOCAL_CANONICAL_NAME         insertall
#define LOCAL_default_seq_foo        default_set_insertall
#define LOCAL_tsc_foo                tsc_set_insertall
#define LOCAL_tsc_foo_data           tsc_set_insertall_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default         DeeSet_Default
#define LOCAL_DeeSeq_RequireFoo      SetInsertAll
#define LOCAL_DeeSeq_AttrFoo         InsertAll
#elif defined(DEFINE_DeeType_SeqCache_RequireSetRemoveAll)
#define LOCAL_CANONICAL_NAME         removeall
#define LOCAL_default_seq_foo        default_set_removeall
#define LOCAL_tsc_foo                tsc_set_removeall
#define LOCAL_tsc_foo_data           tsc_set_removeall_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default         DeeSet_Default
#define LOCAL_DeeSeq_RequireFoo      SetRemoveAll
#define LOCAL_DeeSeq_AttrFoo         RemoveAll
#elif defined(DEFINE_DeeType_SeqCache_RequireSetPop)
#define LOCAL_CANONICAL_NAME         pop
#define LOCAL_default_seq_foo        default_set_pop
#define LOCAL_tsc_foo                tsc_set_pop
#define LOCAL_tsc_foo_data           tsc_set_pop_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default         DeeSet_Default
#define LOCAL_DeeSeq_RequireFoo      SetPop
#define LOCAL_DeeSeq_AttrFoo         Pop
#elif defined(DEFINE_DeeType_SeqCache_RequireSetPopWithDefault)
#define LOCAL_CANONICAL_NAME         pop
#define LOCAL_default_seq_foo        default_set_pop
#define LOCAL_tsc_foo                tsc_set_pop_with_default
#define LOCAL_tsc_foo_data           tsc_set_pop_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default         DeeSet_Default
#define LOCAL_DeeSeq_RequireFoo      SetPopWithDefault
#define LOCAL_DeeSeq_AttrFoo         PopWithDefault
#define LOCAL_DeeSeq_AttrBar         Pop
#elif defined(DEFINE_DeeType_SeqCache_RequireMapSetOld)
#define LOCAL_CANONICAL_NAME         setold
#define LOCAL_default_seq_foo        default_map_setold
#define LOCAL_tsc_foo                tsc_map_setold
#define LOCAL_tsc_foo_data           tsc_map_setold_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapSetOld
#define LOCAL_DeeSeq_AttrFoo         SetOld
#elif defined(DEFINE_DeeType_SeqCache_RequireMapSetOldEx)
#define LOCAL_CANONICAL_NAME         setold_ex
#define LOCAL_default_seq_foo        default_map_setold_ex
#define LOCAL_tsc_foo                tsc_map_setold_ex
#define LOCAL_tsc_foo_data           tsc_map_setold_ex_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapSetOldEx
#define LOCAL_DeeSeq_AttrFoo         SetOldEx
#elif defined(DEFINE_DeeType_SeqCache_RequireMapSetNew)
#define LOCAL_CANONICAL_NAME         setnew
#define LOCAL_default_seq_foo        default_map_setnew
#define LOCAL_tsc_foo                tsc_map_setnew
#define LOCAL_tsc_foo_data           tsc_map_setnew_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapSetNew
#define LOCAL_DeeSeq_AttrFoo         SetNew
#elif defined(DEFINE_DeeType_SeqCache_RequireMapSetNewEx)
#define LOCAL_CANONICAL_NAME         setnew_ex
#define LOCAL_default_seq_foo        default_map_setnew_ex
#define LOCAL_tsc_foo                tsc_map_setnew_ex
#define LOCAL_tsc_foo_data           tsc_map_setnew_ex_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapSetNewEx
#define LOCAL_DeeSeq_AttrFoo         SetNewEx
#elif defined(DEFINE_DeeType_SeqCache_RequireMapSetDefault)
#define LOCAL_CANONICAL_NAME         setdefault
#define LOCAL_default_seq_foo        default_map_setdefault
#define LOCAL_tsc_foo                tsc_map_setdefault
#define LOCAL_tsc_foo_data           tsc_map_setdefault_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapSetDefault
#define LOCAL_DeeSeq_AttrFoo         SetDefault
#elif defined(DEFINE_DeeType_SeqCache_RequireMapUpdate)
#define LOCAL_CANONICAL_NAME         update
#define LOCAL_default_seq_foo        default_map_update
#define LOCAL_tsc_foo                tsc_map_update
#define LOCAL_tsc_foo_data           tsc_map_update_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapUpdate
#define LOCAL_DeeSeq_AttrFoo         Update
#elif defined(DEFINE_DeeType_SeqCache_RequireMapRemove)
#define LOCAL_CANONICAL_NAME         remove
#define LOCAL_default_seq_foo        default_map_remove
#define LOCAL_tsc_foo                tsc_map_remove
#define LOCAL_tsc_foo_data           tsc_map_remove_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapRemove
#define LOCAL_DeeSeq_AttrFoo         Remove
#elif defined(DEFINE_DeeType_SeqCache_RequireMapRemoveKeys)
#define LOCAL_CANONICAL_NAME         removekeys
#define LOCAL_default_seq_foo        default_map_removekeys
#define LOCAL_tsc_foo                tsc_map_removekeys
#define LOCAL_tsc_foo_data           tsc_map_removekeys_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapRemoveKeys
#define LOCAL_DeeSeq_AttrFoo         RemoveKeys
#elif defined(DEFINE_DeeType_SeqCache_RequireMapPop)
#define LOCAL_CANONICAL_NAME         pop
#define LOCAL_default_seq_foo        default_map_pop
#define LOCAL_tsc_foo                tsc_map_pop
#define LOCAL_tsc_foo_data           tsc_map_pop_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapPop
#define LOCAL_DeeSeq_AttrFoo         Pop
#elif defined(DEFINE_DeeType_SeqCache_RequireMapPopWithDefault)
#define LOCAL_CANONICAL_NAME         pop
#define LOCAL_default_seq_foo        default_map_pop
#define LOCAL_tsc_foo                tsc_map_pop_with_default
#define LOCAL_tsc_foo_data           tsc_map_pop_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapPopWithDefault
#define LOCAL_DeeSeq_AttrFoo         PopWithDefault
#define LOCAL_DeeSeq_AttrBar         Pop
#elif defined(DEFINE_DeeType_SeqCache_RequireMapPopItem)
#define LOCAL_CANONICAL_NAME         popitem
#define LOCAL_default_seq_foo        default_map_popitem
#define LOCAL_tsc_foo                tsc_map_popitem
#define LOCAL_tsc_foo_data           tsc_map_popitem_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapPopItem
#define LOCAL_DeeSeq_AttrFoo         PopItem
#elif defined(DEFINE_DeeType_SeqCache_RequireMapKeys)
#define LOCAL_CANONICAL_NAME         keys
#define LOCAL_default_seq_foo        default_map_keys
#define LOCAL_tsc_foo                tsc_map_keys
#define LOCAL_tsc_foo_data           tsc_map_keys_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapKeys
#define LOCAL_DeeSeq_AttrFoo         Keys
#define LOCAL_IS_GETSET
#elif defined(DEFINE_DeeType_SeqCache_RequireMapValues)
#define LOCAL_CANONICAL_NAME         values
#define LOCAL_default_seq_foo        default_map_values
#define LOCAL_tsc_foo                tsc_map_values
#define LOCAL_tsc_foo_data           tsc_map_values_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapValues
#define LOCAL_DeeSeq_AttrFoo         Values
#define LOCAL_IS_GETSET
#elif defined(DEFINE_DeeType_SeqCache_RequireMapIterKeys)
#define LOCAL_CANONICAL_NAME         iterkeys
#define LOCAL_default_seq_foo        default_map_iterkeys
#define LOCAL_tsc_foo                tsc_map_iterkeys
#define LOCAL_tsc_foo_data           tsc_map_iterkeys_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapIterKeys
#define LOCAL_DeeSeq_AttrFoo         IterKeys
#define LOCAL_IS_GETSET
#elif defined(DEFINE_DeeType_SeqCache_RequireMapIterValues)
#define LOCAL_CANONICAL_NAME         itervalues
#define LOCAL_default_seq_foo        default_map_itervalues
#define LOCAL_tsc_foo                tsc_map_itervalues
#define LOCAL_tsc_foo_data           tsc_map_itervalues_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapIterValues
#define LOCAL_DeeSeq_AttrFoo         IterValues
#define LOCAL_IS_GETSET
#else /* DEFINE_DeeType_SeqCache_Require... */
#error "Invalid configuration"
#endif /* !DEFINE_DeeType_SeqCache_Require... */

#ifndef LOCAL_DeeSeq_AttrBar
#define LOCAL_DeeSeq_AttrBar LOCAL_DeeSeq_AttrFoo
#endif /* !LOCAL_DeeSeq_AttrBar */

#ifndef LOCAL_default_seq_foo
#define LOCAL_default_seq_foo PP_CAT2(default_seq_, LOCAL_CANONICAL_NAME)
#endif /* !LOCAL_default_seq_foo */

#ifndef LOCAL_tsc_foo
#define LOCAL_tsc_foo PP_CAT2(tsc_, LOCAL_CANONICAL_NAME)
#endif /* !LOCAL_tsc_foo */

#ifndef LOCAL_DeeSeq_Default
#define LOCAL_DeeSeq_Default DeeSeq_Default
#endif /* !LOCAL_DeeSeq_Default */

#ifndef LOCAL_DeeSeq_RequireFoo
#define LOCAL_DeeSeq_RequireFoo LOCAL_DeeSeq_AttrFoo
#endif /* !LOCAL_DeeSeq_RequireFoo */

#ifndef LOCAL_DeeType_SeqCache_RequireFoo
#define LOCAL_DeeType_SeqCache_RequireFoo_private_uncached PP_CAT3(DeeType_SeqCache_Require, LOCAL_DeeSeq_RequireFoo, _private_uncached)
#define LOCAL_DeeType_SeqCache_RequireFoo_uncached         PP_CAT3(DeeType_SeqCache_Require, LOCAL_DeeSeq_RequireFoo, _uncached)
#define LOCAL_DeeType_SeqCache_RequireFoo                  PP_CAT2(DeeType_SeqCache_Require, LOCAL_DeeSeq_RequireFoo)
#else /* !LOCAL_DeeType_SeqCache_RequireFoo */
#ifndef LOCAL_DeeType_SeqCache_RequireFoo_private_uncached
#define LOCAL_DeeType_SeqCache_RequireFoo_private_uncached PP_CAT2(LOCAL_DeeType_SeqCache_RequireFoo, _private_uncached)
#endif /* !LOCAL_DeeType_SeqCache_RequireFoo_private_uncached */
#ifndef LOCAL_DeeType_SeqCache_RequireFoo_uncached
#define LOCAL_DeeType_SeqCache_RequireFoo_uncached PP_CAT2(LOCAL_DeeType_SeqCache_RequireFoo, _uncached)
#endif /* !LOCAL_DeeType_SeqCache_RequireFoo_uncached */
#endif /* LOCAL_DeeType_SeqCache_RequireFoo */

#ifdef LOCAL_HAS_FOR_SEQ_SUFFIX
#ifndef LOCAL_DeeSeq_DefaultFooWithCallAttrBarForSeq
#define LOCAL_DeeSeq_DefaultFooWithCallAttrBarForSeq \
	PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCallAttr, LOCAL_DeeSeq_AttrBar, ForSeq)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallAttrBarForSeq */
#ifndef LOCAL_DeeSeq_DefaultFooWithCallBarDataFunctionForSeq
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataFunctionForSeq \
	PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCall, LOCAL_DeeSeq_AttrBar, DataFunctionForSeq)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallBarDataFunctionForSeq */
#ifndef LOCAL_DeeSeq_DefaultFooWithCallBarDataMethodForSeq
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataMethodForSeq \
	PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCall, LOCAL_DeeSeq_AttrBar, DataMethodForSeq)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallBarDataMethodForSeq */
#ifndef LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethodForSeq
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethodForSeq \
	PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCall, LOCAL_DeeSeq_AttrBar, DataKwMethodForSeq)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethodForSeq */
#ifndef LOCAL_DeeSeq_DefaultFooWithCallAttrBarForSetOrMap
#define LOCAL_DeeSeq_DefaultFooWithCallAttrBarForSetOrMap \
	PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCallAttr, LOCAL_DeeSeq_AttrBar, ForSetOrMap)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallAttrBarForSetOrMap */
#ifndef LOCAL_DeeSeq_DefaultFooWithCallBarDataFunctionForSetOrMap
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataFunctionForSetOrMap \
	PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCall, LOCAL_DeeSeq_AttrBar, DataFunctionForSetOrMap)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallBarDataFunctionForSetOrMap */
#ifndef LOCAL_DeeSeq_DefaultFooWithCallBarDataMethodForSetOrMap
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataMethodForSetOrMap \
	PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCall, LOCAL_DeeSeq_AttrBar, DataMethodForSetOrMap)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallBarDataMethodForSetOrMap */
#ifndef LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethodForSetOrMap
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethodForSetOrMap \
	PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCall, LOCAL_DeeSeq_AttrBar, DataKwMethodForSetOrMap)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethodForSetOrMap */
#else /* LOCAL_HAS_FOR_SEQ_SUFFIX */
#ifndef LOCAL_DeeSeq_DefaultFooWithCallAttrBar
#define LOCAL_DeeSeq_DefaultFooWithCallAttrBar \
	PP_CAT4(LOCAL_DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCallAttr, LOCAL_DeeSeq_AttrBar)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallAttrBar */
#ifndef LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction \
	PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCall, LOCAL_DeeSeq_AttrBar, DataFunction)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction */
#ifndef LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod \
	PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCall, LOCAL_DeeSeq_AttrBar, DataMethod)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod */
#ifndef LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod \
	PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithCall, LOCAL_DeeSeq_AttrBar, DataKwMethod)
#endif /* !LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod */
#endif /* !LOCAL_HAS_FOR_SEQ_SUFFIX */
#ifndef LOCAL_DeeSeq_DefaultFooWithError
#define LOCAL_DeeSeq_DefaultFooWithError \
	PP_CAT3(LOCAL_DeeSeq_Default, LOCAL_DeeSeq_AttrFoo, WithError)
#endif /* !LOCAL_DeeSeq_DefaultFooWithError */


#ifndef LOCAL_Dee_tsc_foo_t
#define LOCAL_Dee_tsc_foo_t PP_CAT3(Dee_, LOCAL_tsc_foo, _t)
#endif /* !LOCAL_Dee_tsc_foo_t */
#ifndef LOCAL_tsc_foo_data
#define LOCAL_tsc_foo_data PP_CAT3(tsc_, LOCAL_CANONICAL_NAME, _data)
#endif /* !LOCAL_tsc_foo_data */

#define LOCAL_CANONICAL_NAME_LENGTHOF    COMPILER_STRLEN(PP_STR(LOCAL_CANONICAL_NAME))
#define LOCAL_CANONICAL_NAME_str         PP_CAT2(str_, LOCAL_CANONICAL_NAME)
#define LOCAL_CANONICAL_NAME_STR         PP_CAT2(STR_, LOCAL_CANONICAL_NAME)
#define LOCAL_CANONICAL_NAME_Dee_HashStr PP_CAT2(Dee_HashStr__, LOCAL_CANONICAL_NAME)


/* Mutable sequence functions */
PRIVATE WUNUSED NONNULL((1, 2)) LOCAL_Dee_tsc_foo_t DCALL
LOCAL_DeeType_SeqCache_RequireFoo_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self) {
	int seqclass = DeeType_GetSeqClass(self);
	struct Dee_attrinfo attrinfo;
	(void)orig_type;
	(void)seqclass;
#ifdef LOCAL_HAS_FOR_SEQ_SUFFIX
#define LOCAL_DeeSeq_DefaultFooWithCallAttrBar                                      \
	(*(seqclass == Dee_SEQCLASS_SEQ ? &LOCAL_DeeSeq_DefaultFooWithCallAttrBarForSeq \
	                                : &LOCAL_DeeSeq_DefaultFooWithCallAttrBarForSetOrMap))
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction                                      \
	(*(seqclass == Dee_SEQCLASS_SEQ ? &LOCAL_DeeSeq_DefaultFooWithCallBarDataFunctionForSeq \
	                                : &LOCAL_DeeSeq_DefaultFooWithCallBarDataFunctionForSetOrMap))
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod                                      \
	(*(seqclass == Dee_SEQCLASS_SEQ ? &LOCAL_DeeSeq_DefaultFooWithCallBarDataMethodForSeq \
	                                : &LOCAL_DeeSeq_DefaultFooWithCallBarDataMethodForSetOrMap))
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod                                      \
	(*(seqclass == Dee_SEQCLASS_SEQ ? &LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethodForSeq \
	                                : &LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethodForSetOrMap))
#endif /* LOCAL_HAS_FOR_SEQ_SUFFIX */

	/* Check if the type defines an attribute matching the canonical name of this function. */
#ifdef LOCAL_ATTR_REQUIRED_SEQCLASS
	if (seqclass == LOCAL_ATTR_REQUIRED_SEQCLASS)
#else /* LOCAL_ATTR_REQUIRED_SEQCLASS */
	if (seqclass != Dee_SEQCLASS_NONE)
#endif /* !LOCAL_ATTR_REQUIRED_SEQCLASS */
	{
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL,
		                                                LOCAL_CANONICAL_NAME_STR,
		                                                LOCAL_CANONICAL_NAME_LENGTHOF,
		                                                LOCAL_CANONICAL_NAME_Dee_HashStr,
		                                                &attrinfo)) {
			struct Dee_type_seq_cache *sc = DeeType_TryRequireSeqCache(self);
			if likely(sc) {
				switch (attrinfo.ai_type) {
#ifdef LOCAL_IS_GETSET
				case Dee_ATTRINFO_GETSET:
					if (attrinfo.ai_value.v_getset->gs_get == &LOCAL_default_seq_foo)
						return &LOCAL_DeeSeq_DefaultFooWithError;
					atomic_write(&sc->LOCAL_tsc_foo_data.d_getter, attrinfo.ai_value.v_getset->gs_get);
					return &LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod;
				case Dee_ATTRINFO_ATTR:
					if ((attrinfo.ai_value.v_attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FGETSET | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) ==
					    /*                                */ (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FGETSET | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) {
						struct class_desc *desc = DeeClass_DESC(self);
						uint16_t id = attrinfo.ai_value.v_attr->ca_addr;
						DREF DeeObject *callback;
						Dee_class_desc_lock_read(desc);
						callback = desc->cd_members[id + Dee_CLASS_GETSET_GET];
						Dee_XIncref(callback);
						Dee_class_desc_lock_endread(desc);
						if likely(callback) {
							if unlikely(atomic_cmpxch(&sc->LOCAL_tsc_foo_data.d_function, NULL, callback))
								Dee_Decref(callback);
							return &LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction;
						}
					}
					break;
#else /* LOCAL_IS_GETSET */
				case Dee_ATTRINFO_METHOD:
					if ((Dee_funptr_t)attrinfo.ai_value.v_method->m_func == (Dee_funptr_t)&LOCAL_default_seq_foo)
						return &LOCAL_DeeSeq_DefaultFooWithError;
					atomic_write(&sc->LOCAL_tsc_foo_data.d_method, attrinfo.ai_value.v_method->m_func);
					if (attrinfo.ai_value.v_method->m_flag & Dee_TYPE_METHOD_FKWDS)
						return &LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod;
					return &LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod;
				case Dee_ATTRINFO_ATTR:
					if ((attrinfo.ai_value.v_attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) ==
					    /*                                */ (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) {
						struct class_desc *desc = DeeClass_DESC(self);
						uint16_t id = attrinfo.ai_value.v_attr->ca_addr;
						DREF DeeObject *callback;
						Dee_class_desc_lock_read(desc);
						callback = desc->cd_members[id];
						Dee_XIncref(callback);
						Dee_class_desc_lock_endread(desc);
						if likely(callback) {
							if unlikely(atomic_cmpxch(&sc->LOCAL_tsc_foo_data.d_function, NULL, callback))
								Dee_Decref(callback);
							return &LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction;
						}
					}
					break;
#endif /* !LOCAL_IS_GETSET */
				default: break;
				}
			}
			return &LOCAL_DeeSeq_DefaultFooWithCallAttrBar;
		}
	}
#ifdef DEFINE_DeeType_SeqCache_RequireAny
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireAnyWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireAnyWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireAnyWithRangeAndKey)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireAll)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireAllWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireAllWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireAllWithRangeAndKey)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireParity)
	/* TODO: Parity with count */
#elif defined(DEFINE_DeeType_SeqCache_RequireParityWithKey)
	/* TODO: Parity with count */
#elif defined(DEFINE_DeeType_SeqCache_RequireParityWithRange)
	/* TODO: Parity with count */
#elif defined(DEFINE_DeeType_SeqCache_RequireParityWithRangeAndKey)
	/* TODO: Parity with count */
#elif defined(DEFINE_DeeType_SeqCache_RequireReduce)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireReduceWithInit)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireReduceWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireReduceWithRangeAndInit)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireMin)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireMinWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireMinWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireMinWithRangeAndKey)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireMax)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireMaxWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireMaxWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireMaxWithRangeAndKey)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_Requiresum)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequiresumWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireCount)
	/* ... */
	/* TODO: Count with Find (repeated) */
#elif defined(DEFINE_DeeType_SeqCache_RequireCountWithKey)
	/* ... */
	/* TODO: Count with Find (repeated) */
#elif defined(DEFINE_DeeType_SeqCache_RequireCountWithRange)
	/* ... */
	/* TODO: Count with Find (repeated) */
#elif defined(DEFINE_DeeType_SeqCache_RequireCountWithRangeAndKey)
	/* ... */
	/* TODO: Count with Find (repeated) */
#elif defined(DEFINE_DeeType_SeqCache_RequireContains)
	if (seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_CONTAINS))
			return &DeeSeq_DefaultContainsWithContains;
	}
	/* TODO: Contains with Find */
	if (DeeType_HasPrivateOperator(self, OPERATOR_ITER))
		return &DeeSeq_DefaultContainsWithForeach;
#elif defined(DEFINE_DeeType_SeqCache_RequireContainsWithKey)
	/* TODO: Contains with Find */
#elif defined(DEFINE_DeeType_SeqCache_RequireContainsWithRange)
	/* TODO: Contains with Find */
#elif defined(DEFINE_DeeType_SeqCache_RequireContainsWithRangeAndKey)
	/* TODO: Contains with Find */
#elif defined(DEFINE_DeeType_SeqCache_RequireLocate)
	/* TODO: Locate with Find */
#elif defined(DEFINE_DeeType_SeqCache_RequireLocateWithKey)
	/* TODO: Locate with Find */
#elif defined(DEFINE_DeeType_SeqCache_RequireLocateWithRange)
	/* TODO: Locate with Find */
#elif defined(DEFINE_DeeType_SeqCache_RequireLocateWithRangeAndKey)
	/* TODO: Locate with Find */
#elif defined(DEFINE_DeeType_SeqCache_RequireRLocateWithRange)
	if (DeeType_SeqCache_HasPrivateEnumerateIndexReverse(orig_type, self))
		return &DeeSeq_DefaultRLocateWithRangeWithTSCEnumerateIndexReverse;
#elif defined(DEFINE_DeeType_SeqCache_RequireRLocateWithRangeAndKey)
	if (DeeType_SeqCache_HasPrivateEnumerateIndexReverse(orig_type, self))
		return &DeeSeq_DefaultRLocateWithRangeAndKeyWithTSCEnumerateIndexReverse;
#elif defined(DEFINE_DeeType_SeqCache_RequireStartsWith)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireStartsWithWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireStartsWithWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireStartsWithWithRangeAndKey)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireEndsWith)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireEndsWithWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireEndsWithWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireEndsWithWithRangeAndKey)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireFind)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireFindWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireRFind)
	if (DeeType_SeqCache_HasPrivateEnumerateIndexReverse(orig_type, self))
		return &DeeSeq_DefaultRFindWithTSCEnumerateIndexReverse;
#elif defined(DEFINE_DeeType_SeqCache_RequireRFindWithKey)
	if (DeeType_SeqCache_HasPrivateEnumerateIndexReverse(orig_type, self))
		return &DeeSeq_DefaultRFindWithKeyWithTSCEnumerateIndexReverse;
#elif defined(DEFINE_DeeType_SeqCache_RequireErase)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_DELRANGE))
			return &DeeSeq_DefaultEraseWithDelRangeIndex;
		{
			Dee_tsc_pop_t tsc_pop;
			tsc_pop = DeeType_SeqCache_RequirePop_private_uncached(orig_type, self);
			if (tsc_pop != NULL &&
			    tsc_pop != &DeeSeq_DefaultPopWithError)
				return &DeeSeq_DefaultEraseWithPop;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireInsert)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		Dee_tsc_insertall_t tsc_insertall;
		tsc_insertall = DeeType_SeqCache_RequireInsertAll_private_uncached(orig_type, self);
		if (tsc_insertall != NULL &&
		    tsc_insertall != &DeeSeq_DefaultInsertAllWithError)
			return &DeeSeq_DefaultInsertWithTSCInsertAll;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireInsertAll)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_SETRANGE) &&
		    self->tp_seq->tp_setrange_index != &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndTSCEraseAndTSCInsertAll)
			return &DeeSeq_DefaultInsertAllWithSetRangeIndex;
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_insert, 6, Dee_HashStr__insert, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_insert))
			return &DeeSeq_DefaultInsertAllWithTSCInsertForeach;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequirePushFront)
	/* ... */
#elif defined(DEFINE_DeeType_SeqCache_RequireAppend)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (seqclass == Dee_SEQCLASS_SEQ) {
			/* Check for "pushback()" */
			if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_pushback, 8,
			                                                Dee_HashStr__pushback, &attrinfo)) {
				struct Dee_type_seq_cache *sc = DeeType_TryRequireSeqCache(self);
				if likely(sc) {
					switch (attrinfo.ai_type) {
					case Dee_ATTRINFO_METHOD:
						if ((Dee_funptr_t)attrinfo.ai_value.v_method->m_func == (Dee_funptr_t)&LOCAL_default_seq_foo)
							return &LOCAL_DeeSeq_DefaultFooWithError;
						atomic_write(&sc->LOCAL_tsc_foo_data.d_method, attrinfo.ai_value.v_method->m_func);
						if (attrinfo.ai_value.v_method->m_flag & Dee_TYPE_METHOD_FKWDS)
							return &LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod;
						return &LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod;
					case Dee_ATTRINFO_ATTR:
						ASSERT(attrinfo.ai_type == Dee_ATTRINFO_ATTR);
						if ((attrinfo.ai_value.v_attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) ==
						    /*                                */ (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) {
							struct class_desc *desc = DeeClass_DESC(self);
							uint16_t id = attrinfo.ai_value.v_attr->ca_addr;
							DREF DeeObject *callback;
							Dee_class_desc_lock_read(desc);
							callback = desc->cd_members[id];
							Dee_XIncref(callback);
							Dee_class_desc_lock_endread(desc);
							if likely(callback) {
								if unlikely(atomic_cmpxch(&sc->LOCAL_tsc_foo_data.d_function, NULL, callback))
									Dee_Decref(callback);
								return &LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction;
							}
						}
						break;
					default: break;
					}
				}
				return &DeeSeq_DefaultAppendWithCallAttrPushBack;
			}
		}
		{
			Dee_tsc_extend_t tsc_extend;
			tsc_extend = DeeType_SeqCache_RequireExtend_private_uncached(orig_type, self);
			if (tsc_extend != NULL &&
			    tsc_extend != &DeeSeq_DefaultExtendWithError) {
				if (tsc_extend == &DeeSeq_DefaultExtendWithSizeAndTSCInsertAll)
					return &DeeSeq_DefaultAppendWithSizeAndTSCInsert;;
				return &DeeSeq_DefaultAppendWithTSCExtend;
			}
		}
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
			Dee_tsc_insert_t tsc_insert;
			tsc_insert = DeeType_SeqCache_RequireInsert_private_uncached(orig_type, self);
			if (tsc_insert != NULL &&
			    tsc_insert != &DeeSeq_DefaultInsertWithError)
				return &DeeSeq_DefaultAppendWithSizeAndTSCInsert;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireExtend)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasOperator(orig_type, OPERATOR_ITER)) {
			if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_append, 6, Dee_HashStr__append, &attrinfo) &&
			    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_append))
				return &DeeSeq_DefaultExtendWithTSCAppendForeach;
			if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_pushback, 8, Dee_HashStr__pushback, &attrinfo) &&
			    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_append))
				return &DeeSeq_DefaultExtendWithTSCAppendForeach;
		}
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
			Dee_tsc_insertall_t tsc_insertall;
			tsc_insertall = DeeType_SeqCache_RequireInsertAll_private_uncached(orig_type, self);
			if (tsc_insertall != NULL &&
			    tsc_insertall != &DeeSeq_DefaultInsertAllWithError)
				return &DeeSeq_DefaultExtendWithSizeAndTSCInsertAll;
		}
		if (DeeType_HasOperator(orig_type, OPERATOR_ITER)) {
			Dee_tsc_insert_t tsc_insert;
			tsc_insert = DeeType_SeqCache_RequireInsert_private_uncached(orig_type, self);
			if (tsc_insert != NULL &&
				tsc_insert != &DeeSeq_DefaultInsertWithError)
				return &DeeSeq_DefaultExtendWithTSCAppendForeach;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireXchItemIndex)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_SETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM))
			return &DeeSeq_DefaultXchItemIndexWithGetItemIndexAndSetItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireClear)
	switch (seqclass) {

	case Dee_SEQCLASS_SET:{
		Dee_tsc_set_removeall_t tsc_set_removeall;
		tsc_set_removeall = DeeType_SeqCache_RequireSetRemoveAll_private_uncached(orig_type, self);
		if (tsc_set_removeall != NULL &&
		    tsc_set_removeall != &DeeSet_DefaultRemoveAllWithError)
			return &DeeSeq_DefaultClearWithTSCRemoveAllForSet;
	}	break;

	case Dee_SEQCLASS_MAP: {
		Dee_tsc_map_removekeys_t tsc_map_removekeys;
		tsc_map_removekeys = DeeType_SeqCache_RequireMapRemoveKeys_private_uncached(orig_type, self);
		if (tsc_map_removekeys != NULL &&
		    tsc_map_removekeys != &DeeMap_DefaultRemoveKeysWithError)
			return &DeeSeq_DefaultClearWithTSCRemoveKeysForMap;
	}	break;

	default:
		if (DeeType_HasPrivateOperator(self, OPERATOR_DELRANGE)) {
			if (self->tp_seq->tp_delrange_index_n == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNone ||
			    self->tp_seq->tp_delrange_index_n == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNoneDefault)
				return &DeeSeq_DefaultClearWithSetRangeIndexN;
			return &DeeSeq_DefaultClearWithDelRangeIndexN;
		}
		if (DeeType_HasPrivateOperator(self, OPERATOR_SETRANGE))
			return &DeeSeq_DefaultClearWithSetRangeIndexN;
		{
			Dee_tsc_erase_t tsc_erase;
			tsc_erase = DeeType_SeqCache_RequireErase_private_uncached(orig_type, self);
			if (tsc_erase != NULL &&
			    tsc_erase != &DeeSeq_DefaultEraseWithError)
				return &DeeSeq_DefaultClearWithTSCErase;
		}
		break;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequirePop)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_erase, 5, Dee_HashStr__erase, &attrinfo) &&
			    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_erase))
				return &DeeSeq_DefaultPopWithSizeAndGetItemIndexAndTSCErase;
		}
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator(self, OPERATOR_DELITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultPopWithSizeAndGetItemIndexAndDelItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireRemove)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		{
			Dee_tsc_removeall_t tsc_removeall;
			tsc_removeall = DeeType_SeqCache_RequireRemoveAll_private_uncached(orig_type, self);
			if (tsc_removeall != NULL &&
			    tsc_removeall != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveWithTSCRemoveAll;
		}
		{
			Dee_tsc_removeif_t tsc_removeif;
			tsc_removeif = DeeType_SeqCache_RequireRemoveIf_private_uncached(orig_type, self);
			if (tsc_removeif != NULL &&
			    tsc_removeif != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveWithTSCRemoveIf;
		}
		if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM)) {
			Dee_tsc_find_t tsc_find;
			tsc_find = DeeType_SeqCache_RequireFind_private_uncached(orig_type, self);
			if (tsc_find == &DeeSeq_DefaultFindWithEnumerateIndex)
				return &DeeSeq_DefaultRemoveWithEnumerateIndexAndDelItemIndex;
			if (tsc_find)
				return &DeeSeq_DefaultRemoveWithTSCFindAndDelItemIndex;
			if (DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
				return &DeeSeq_DefaultRemoveWithEnumerateIndexAndDelItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveWithKey)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		{
			Dee_tsc_removeall_with_key_t tsc_removeall_with_key;
			tsc_removeall_with_key = DeeType_SeqCache_RequireRemoveAllWithKey_private_uncached(orig_type, self);
			if (tsc_removeall_with_key != NULL &&
			    tsc_removeall_with_key != &DeeSeq_DefaultRemoveAllWithKeyWithError)
				return &DeeSeq_DefaultRemoveWithKeyWithTSCRemoveAllWithKey;
		}
		{
			Dee_tsc_removeif_t tsc_removeif;
			tsc_removeif = DeeType_SeqCache_RequireRemoveIf_private_uncached(orig_type, self);
			if (tsc_removeif != NULL &&
			    tsc_removeif != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveWithKeyWithTSCRemoveIf;
		}
		if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM)) {
			Dee_tsc_find_with_key_t tsc_find_with_key;
			tsc_find_with_key = DeeType_SeqCache_RequireFindWithKey_private_uncached(orig_type, self);
			if (tsc_find_with_key == &DeeSeq_DefaultFindWithKeyWithEnumerateIndex)
				return &DeeSeq_DefaultRemoveWithKeyWithEnumerateIndexAndDelItemIndex;
			if (tsc_find_with_key)
				return &DeeSeq_DefaultRemoveWithKeyWithTSCFindWithKeyAndDelItemIndex;
			if (DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
				return &DeeSeq_DefaultRemoveWithKeyWithEnumerateIndexAndDelItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireRRemove)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM)) {
			Dee_tsc_rfind_t tsc_rfind;
			tsc_rfind = DeeType_SeqCache_RequireRFind_private_uncached(orig_type, self);
			if (tsc_rfind == &DeeSeq_DefaultRFindWithTSCEnumerateIndexReverse)
				return &DeeSeq_DefaultRRemoveWithTSCEnumerateIndexReverseAndDelItemIndex;
			if (tsc_rfind == &DeeSeq_DefaultRFindWithEnumerateIndex)
				return &DeeSeq_DefaultRRemoveWithEnumerateIndexAndDelItemIndex;
			if (tsc_rfind)
				return &DeeSeq_DefaultRRemoveWithTSCRFindAndDelItemIndex;
			if (DeeType_SeqCache_TryRequireEnumerateIndexReverse(orig_type))
				return &DeeSeq_DefaultRRemoveWithTSCEnumerateIndexReverseAndDelItemIndex;
			if (DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
				return &DeeSeq_DefaultRRemoveWithEnumerateIndexAndDelItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireRRemoveWithKey)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM)) {
			Dee_tsc_rfind_with_key_t tsc_rfind_with_key;
			tsc_rfind_with_key = DeeType_SeqCache_RequireRFindWithKey_private_uncached(orig_type, self);
			if (tsc_rfind_with_key == &DeeSeq_DefaultRFindWithKeyWithTSCEnumerateIndexReverse)
				return &DeeSeq_DefaultRRemoveWithKeyWithTSCEnumerateIndexReverseAndDelItemIndex;
			if (tsc_rfind_with_key == &DeeSeq_DefaultRFindWithKeyWithEnumerateIndex)
				return &DeeSeq_DefaultRRemoveWithKeyWithEnumerateIndexAndDelItemIndex;
			if (tsc_rfind_with_key)
				return &DeeSeq_DefaultRRemoveWithKeyWithTSCRFindWithKeyAndDelItemIndex;
			if (DeeType_SeqCache_TryRequireEnumerateIndexReverse(orig_type))
				return &DeeSeq_DefaultRRemoveWithKeyWithTSCEnumerateIndexReverseAndDelItemIndex;
			if (DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
				return &DeeSeq_DefaultRRemoveWithKeyWithEnumerateIndexAndDelItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveAll)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		{
			Dee_tsc_removeif_t tsc_removeif;
			tsc_removeif = DeeType_SeqCache_RequireRemoveIf_private_uncached(orig_type, self);
			if (tsc_removeif != NULL &&
			    tsc_removeif != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveAllWithTSCRemoveIf;
		}
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_remove, 6, Dee_HashStr__remove, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_remove))
			return &DeeSeq_DefaultRemoveAllWithTSCRemove;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator(self, OPERATOR_DELITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultRemoveAllWithSizeAndGetItemIndexAndDelItemIndex;
		if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
			return &DeeSeq_DefaultRemoveAllWithTSCRemove;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveAllWithKey)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		{
			Dee_tsc_removeif_t tsc_removeif;
			tsc_removeif = DeeType_SeqCache_RequireRemoveIf_private_uncached(orig_type, self);
			if (tsc_removeif != NULL &&
			    tsc_removeif != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveIf;
		}
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_remove, 6, Dee_HashStr__remove, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_remove))
			return &DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveWithKey;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator(self, OPERATOR_DELITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultRemoveAllWithKeyWithSizeAndGetItemIndexAndDelItemIndex;
		if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
			return &DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveWithKey;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireRemoveIf)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_removeall, 9, Dee_HashStr__removeall, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_removeall))
			return &DeeSeq_DefaultRemoveIfWithTSCRemoveAllWithKey;
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_remove, 6, Dee_HashStr__remove, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_remove))
			return &DeeSeq_DefaultRemoveIfWithTSCRemoveAllWithKey;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator(self, OPERATOR_DELITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultRemoveIfWithSizeAndGetItemIndexAndDelItemIndex;
		if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
			return &DeeSeq_DefaultRemoveIfWithTSCRemoveAllWithKey;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireResize)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator(self, OPERATOR_SETRANGE) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
			if (DeeType_HasOperator(orig_type, OPERATOR_DELRANGE) &&
			    orig_type->tp_seq->tp_delrange_index != &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNone &&
			    orig_type->tp_seq->tp_delrange_index != &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNoneDefault)
				return &DeeSeq_DefaultResizeWithSizeAndSetRangeIndexAndDelRangeIndex;
			return &DeeSeq_DefaultResizeWithSizeAndSetRangeIndex;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireFill)
	if (seqclass == Dee_SEQCLASS_SEQ) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_SETRANGE) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultFillWithSizeAndSetRangeIndex;
		if (DeeType_HasPrivateOperator(self, OPERATOR_SETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_ITER))
			return &DeeSeq_DefaultFillWithEnumerateIndexAndSetItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireReverse)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_SETRANGE))
			return &DeeSeq_DefaultReverseWithTSCReversedAndSetRangeIndex;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator(self, OPERATOR_SETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
			if (DeeType_HasOperator(orig_type, OPERATOR_DELITEM))
				return &DeeSeq_DefaultReverseWithSizeAndGetItemIndexAndSetItemIndexAndDelItemIndex;
			return &DeeSeq_DefaultReverseWithSizeAndGetItemIndexAndSetItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireReversed)
	if (seqclass == Dee_SEQCLASS_SEQ &&
	    DeeType_HasPrivateOperator(self, OPERATOR_GETITEM) &&
	    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
		struct type_seq *seq = self->tp_seq;
		ASSERT(seq->tp_getitem_index);
		ASSERT(seq->tp_trygetitem_index);
		if (seq->tp_getitem_index_fast)
			return &DeeSeq_DefaultReversedWithProxySizeAndGetItemIndexFast;
		if (!DeeType_IsDefaultGetItemIndex(seq->tp_getitem_index))
			return &DeeSeq_DefaultReversedWithProxySizeAndGetItemIndex;
		if (!DeeType_IsDefaultTryGetItemIndex(seq->tp_trygetitem_index))
			return &DeeSeq_DefaultReversedWithProxySizeAndTryGetItemIndex;
		return &DeeSeq_DefaultReversedWithProxySizeAndGetItemIndex;
	}
	if (DeeType_HasPrivateOperator(self, OPERATOR_ITER))
		return &DeeSeq_DefaultReversedWithCopyForeachDefault; /* non-Default would also be OK */
#elif defined(DEFINE_DeeType_SeqCache_RequireSort)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_SETRANGE))
			return &DeeSeq_DefaultSortWithTSCSortedAndSetRangeIndex;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator(self, OPERATOR_SETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultSortWithSizeAndGetItemIndexAndSetItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireSortWithKey)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_SETRANGE))
			return &DeeSeq_DefaultSortWithKeyWithTSCSortedAndSetRangeIndex;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator(self, OPERATOR_SETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultSortWithKeyWithSizeAndGetItemIndexAndSetItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireSorted)
	if (DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
		if (orig_type->tp_seq->tp_getitem_index_fast)
			return &DeeSeq_DefaultSortedWithCopySizeAndGetItemIndexFast;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator(self, OPERATOR_GETITEM))
			return &DeeSeq_DefaultSortedWithCopySizeAndTryGetItemIndex;
	}
	if (DeeType_HasPrivateOperator(self, OPERATOR_ITER))
		return &DeeSeq_DefaultSortedWithCopyForeachDefault; /* non-Default would also be OK */
#elif defined(DEFINE_DeeType_SeqCache_RequireSortedWithKey)
	if (DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
		if (orig_type->tp_seq->tp_getitem_index_fast)
			return &DeeSeq_DefaultSortedWithKeyWithCopySizeAndGetItemIndexFast;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator(self, OPERATOR_GETITEM))
			return &DeeSeq_DefaultSortedWithKeyWithCopySizeAndTryGetItemIndex;
	}
	if (DeeType_HasPrivateOperator(self, OPERATOR_ITER))
		return &DeeSeq_DefaultSortedWithKeyWithCopyForeachDefault; /* non-Default would also be OK */
#elif defined(DEFINE_DeeType_SeqCache_RequireBFind)
	{
		Dee_tsc_brange_t tsc_brange;
		tsc_brange = DeeType_SeqCache_RequireBRange_private_uncached(orig_type, self);
		if (tsc_brange != NULL &&
		    tsc_brange != &DeeSeq_DefaultBRangeWithError) {
			if (tsc_brange == &DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBFindWithTSCBRange;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireBFindWithKey)
	{
		Dee_tsc_brange_with_key_t tsc_brange_with_key;
		tsc_brange_with_key = DeeType_SeqCache_RequireBRangeWithKey_private_uncached(orig_type, self);
		if (tsc_brange_with_key != NULL &&
		    tsc_brange_with_key != &DeeSeq_DefaultBRangeWithKeyWithError) {
			if (tsc_brange_with_key == &DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBFindWithKeyWithTSCBRangeWithKey;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireBPosition)
	{
		Dee_tsc_brange_t tsc_brange;
		tsc_brange = DeeType_SeqCache_RequireBRange_private_uncached(orig_type, self);
		if (tsc_brange != NULL &&
		    tsc_brange != &DeeSeq_DefaultBRangeWithError) {
			if (tsc_brange == &DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBPositionWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBPositionWithTSCBRange;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireBPositionWithKey)
	{
		Dee_tsc_brange_with_key_t tsc_brange_with_key;
		tsc_brange_with_key = DeeType_SeqCache_RequireBRangeWithKey_private_uncached(orig_type, self);
		if (tsc_brange_with_key != NULL &&
		    tsc_brange_with_key != &DeeSeq_DefaultBRangeWithKeyWithError) {
			if (tsc_brange_with_key == &DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBPositionWithKeyWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBPositionWithKeyWithTSCBRangeWithKey;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireBRange)
	{
		if (DeeType_HasPrivateOperator(self, OPERATOR_GETITEM) && DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireBRangeWithKey)
	{
		if (DeeType_HasPrivateOperator(self, OPERATOR_GETITEM) && DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireBLocate)
	{
		Dee_tsc_bfind_t tsc_bfind;
		tsc_bfind = DeeType_SeqCache_RequireBFind_private_uncached(orig_type, self);
		if (tsc_bfind != NULL &&
		    tsc_bfind != &DeeSeq_DefaultBFindWithError) {
			if (tsc_bfind == &DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBLocateWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBLocateWithTSCBFindAndGetItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireBLocateWithKey)
	{
		Dee_tsc_bfind_with_key_t tsc_bfind_with_key;
		tsc_bfind_with_key = DeeType_SeqCache_RequireBFindWithKey_private_uncached(orig_type, self);
		if (tsc_bfind_with_key != NULL &&
		    tsc_bfind_with_key != &DeeSeq_DefaultBFindWithKeyWithError) {
			if (tsc_bfind_with_key == &DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBLocateWithKeyWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBLocateWithKeyWithTSCBFindWithKeyAndGetItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireSetInsert)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* use insertall and "operator #" before/after */
		if (DeeType_HasOperator(orig_type, OPERATOR_SIZE) &&
		    DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_insertall, 9, Dee_HashStr__insertall, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_set_insertall))
			return &DeeSet_DefaultInsertWithSizeAndTSCInsertAll;
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* >> local x = Dict();
		 * >> (x as Set).insert(("foo", "bar")); // x.setnew("foo", "bar");
		 * >> print repr x; // {"foo":"bar"} */
		Dee_tsc_map_setnew_t tsc_map_setnew;
		tsc_map_setnew = DeeType_SeqCache_RequireMapSetNew_private_uncached(orig_type, self);
		if (tsc_map_setnew != NULL &&
		    tsc_map_setnew != &DeeMap_DefaultSetNewWithError)
			return &DeeSet_DefaultInsertWithMapSetNew;
	} else {
		/* >> local x = [];
		 * >> (x as Set).insert(10); // if (!x.contains(10)) x.append(10);
		 * >> (x as Set).insert(20); // if (!x.contains(20)) x.append(20);
		 * >> (x as Set).insert(10); // if (!x.contains(10)) x.append(10);
		 * >> print repr x;          // [10, 20] */
		Dee_tsc_append_t tsc_append;
		tsc_append = DeeType_SeqCache_RequireAppend_private_uncached(orig_type, self);
		if (tsc_append != NULL &&
		    tsc_append != &DeeSeq_DefaultAppendWithError)
			return &DeeSet_DefaultInsertWithSeqTSCContainsAndTSCAppend;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireSetRemove)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* use removeall and "operator #" before/after */
		if (DeeType_HasOperator(orig_type, OPERATOR_SIZE) &&
		    DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_removeall, 9, Dee_HashStr__removeall, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_set_removeall))
			return &DeeSet_DefaultRemoveWithSizeAndTSCRemoveAll;
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* >> local x = Dict(("foo", "bar"));
		 * >> (x as Set).remove(("foo", "bar")); // if (equals(x["foo"], "bar")) del x["foo"];
		 * >> print repr x; // {} */
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasPrivateOperator(self, OPERATOR_DELITEM))
			return &DeeSet_DefaultRemoveWithMapGetItemAndDelItem;
	} else {
		/* >> local x = [10, 20, 30];
		 * >> (x as Set).remove(30); // x.remove(30);  (using `DeeType_SeqCache_RequireRemove')
		 * >> print repr x;          // [10, 20] */
		Dee_tsc_remove_t tsc_remove;
		tsc_remove = DeeType_SeqCache_RequireRemove_private_uncached(orig_type, self);
		if (tsc_remove != NULL &&
		    tsc_remove != &DeeSeq_DefaultRemoveWithError)
			return &DeeSet_DefaultRemoveWithSeqTSCRemove;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireSetUnify)
	if (seqclass == Dee_SEQCLASS_SET) {
		Dee_tsc_set_insert_t tsc_set_insert;
		tsc_set_insert = DeeType_SeqCache_RequireSetInsert_private_uncached(orig_type, self);
		if (tsc_set_insert != NULL &&
		    tsc_set_insert != &DeeSet_DefaultInsertWithError) {
			if (DeeType_HasOperator(orig_type, OPERATOR_ITER))
				return &DeeSet_DefaultUnifyWithTSCInsertAndForeach;
		}
	} else if (seqclass != Dee_SEQCLASS_MAP) {
		/* >> local x = [];
		 * >> (x as Set).unify("foo"); // try { x.locate("foo"); } catch (ValueError) { x.append("foo"); }
		 * >> (x as Set).unify("bar");
		 * >> (x as Set).unify("foo");
		 * >> print repr x; // ["foo", "bar"] */
		Dee_tsc_append_t tsc_append;
		tsc_append = DeeType_SeqCache_RequireAppend_private_uncached(orig_type, self);
		if (tsc_append != NULL &&
		    tsc_append != &DeeSeq_DefaultAppendWithError)
			return &DeeSet_DefaultUnifyWithTSCLocateAndTSCAppend;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireSetInsertAll)
	if (seqclass == Dee_SEQCLASS_SET) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_INPLACE_ADD))
			return &DeeSet_DefaultInsertAllWithInplaceAdd;
		if (DeeType_HasPrivateOperator(self, OPERATOR_INPLACE_OR))
			return &DeeSet_DefaultInsertAllWithInplaceOr;
	}
	{
		Dee_tsc_set_insert_t tsc_set_insert;
		tsc_set_insert = DeeType_SeqCache_RequireSetInsert_private_uncached(orig_type, self);
		if (tsc_set_insert != NULL &&
		    tsc_set_insert != &DeeSet_DefaultInsertWithError)
			return &DeeSet_DefaultInsertAllWithTSCInsert;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireSetRemoveAll)
	if (seqclass == Dee_SEQCLASS_SET) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_INPLACE_SUB))
			return &DeeSet_DefaultRemoveAllWithInplaceSub;
	}
	{
		Dee_tsc_set_remove_t tsc_set_remove;
		tsc_set_remove = DeeType_SeqCache_RequireSetRemove_private_uncached(orig_type, self);
		if (tsc_set_remove != NULL &&
		    tsc_set_remove != &DeeSet_DefaultRemoveWithError)
			return &DeeSet_DefaultRemoveAllWithTSCRemove;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireSetPop)
	if (seqclass == Dee_SEQCLASS_SET) {
		Dee_tsc_set_remove_t tsc_set_remove;
		tsc_set_remove = DeeType_SeqCache_RequireSetRemove_private_uncached(orig_type, self);
		if (tsc_set_remove != NULL &&
		    tsc_set_remove != &DeeSet_DefaultRemoveWithError)
			return &DeeSet_DefaultPopWithTSCFirstAndTSCRemove;
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* >> local x = Dict(("foo", "bar"));
		 * >> print repr((x as Set).pop()); // ("foo", "bar")
		 * >> print repr x; // {} */
		Dee_tsc_map_popitem_t tsc_map_popitem;
		tsc_map_popitem = DeeType_SeqCache_RequireMapPopItem_private_uncached(orig_type, self);
		if (tsc_map_popitem != NULL &&
		    tsc_map_popitem != &DeeMap_DefaultPopItemWithError)
			return &DeeSet_DefaultPopWithMapPopItem;
	} else {
		/* >> local x = [10, 20, 30];
		 * >> print repr((x as Set).pop()); // 30
		 * >> print repr x;                 // [10, 20] */
		Dee_tsc_pop_t tsc_pop;
		tsc_pop = DeeType_SeqCache_RequirePop_private_uncached(orig_type, self);
		if (tsc_pop != NULL &&
		    tsc_pop != &DeeSeq_DefaultPopWithError)
			return &DeeSet_DefaultPopWithSeqPop;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireSetPopWithDefault)
	if (seqclass == Dee_SEQCLASS_SET) {
		Dee_tsc_set_remove_t tsc_set_remove;
		tsc_set_remove = DeeType_SeqCache_RequireSetRemove_private_uncached(orig_type, self);
		if (tsc_set_remove != NULL &&
		    tsc_set_remove != &DeeSet_DefaultRemoveWithError)
			return &DeeSet_DefaultPopWithDefaultWithTSCFirstAndTSCRemove;
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* >> local x = Dict(("foo", "bar"));
		 * >> print repr((x as Set).pop(("foo", "ignored"))); // ("foo", "bar")
		 * >> print repr x; // {} */
		Dee_tsc_map_popitem_t tsc_map_popitem;
		tsc_map_popitem = DeeType_SeqCache_RequireMapPopItem_private_uncached(orig_type, self);
		if (tsc_map_popitem != NULL &&
		    tsc_map_popitem != &DeeMap_DefaultPopItemWithError)
			return &DeeSet_DefaultPopWithDefaultWithMapPopItem;
	} else {
		/* >> local x = [10, 20, 30];
		 * >> print repr((x as Set).pop()); // 30
		 * >> print repr x;                 // [10, 20] */
		Dee_tsc_pop_t tsc_pop;
		tsc_pop = DeeType_SeqCache_RequirePop_private_uncached(orig_type, self);
		if (tsc_pop != NULL &&
		    tsc_pop != &DeeSeq_DefaultPopWithError)
			return &DeeSet_DefaultPopWithDefaultWithSeqPop;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireMapSetOld)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_setold_ex, 9, Dee_HashStr__setold_ex, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_setold_ex))
			return &DeeMap_DefaultSetOldWithTSCSetOldEx;
		if (DeeType_HasPrivateOperator(self, OPERATOR_SETITEM) && DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			struct type_seq *seq = orig_type->tp_seq;
			ASSERT(seq->tp_bounditem);
			ASSERT(seq->tp_trygetitem);
			ASSERT(seq->tp_getitem);
			if (!DeeType_IsDefaultBoundItem(seq->tp_bounditem))
				return &DeeMap_DefaultSetOldWithBoundItemAndSetItem;
			if (!DeeType_IsDefaultTryGetItem(seq->tp_trygetitem))
				return &DeeMap_DefaultSetOldWithTryGetItemAndSetItem;
			return &DeeMap_DefaultSetOldWithGetItemAndSetItem;
		}
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Set.remove'+`Set.insert' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `SetItemIndex' */
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireMapSetOldEx)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_SETITEM) && DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			struct type_seq *seq = orig_type->tp_seq;
			ASSERT(seq->tp_trygetitem);
			ASSERT(seq->tp_getitem);
			if (!DeeType_IsDefaultTryGetItem(seq->tp_trygetitem))
				return &DeeMap_DefaultSetOldExWithTryGetItemAndSetItem;
			return &DeeMap_DefaultSetOldExWithGetItemAndSetItem;
		}
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Set.remove'+`Set.insert' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `SetItemIndex' */
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireMapSetNew)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_setnew_ex, 9, Dee_HashStr__setnew_ex, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_setnew_ex))
			return &DeeMap_DefaultSetNewWithTSCSetNewEx;
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			struct type_seq *seq = orig_type->tp_seq;
			ASSERT(seq->tp_bounditem);
			ASSERT(seq->tp_trygetitem);
			ASSERT(seq->tp_getitem);
			if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_setdefault, 10, Dee_HashStr__setdefault, &attrinfo) &&
			    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_setdefault)) {
				if (!DeeType_IsDefaultBoundItem(seq->tp_bounditem))
					return &DeeMap_DefaultSetNewWithBoundItemAndTSCSetDefault;
				if (!DeeType_IsDefaultTryGetItem(seq->tp_trygetitem))
					return &DeeMap_DefaultSetNewWithTryGetItemAndTSCSetDefault;
				return &DeeMap_DefaultSetNewWithGetItemAndTSCSetDefault;
			} else if (DeeType_HasPrivateOperator(self, OPERATOR_SETITEM)) {
				if (!DeeType_IsDefaultBoundItem(seq->tp_bounditem))
					return &DeeMap_DefaultSetNewWithBoundItemAndSetItem;
				if (!DeeType_IsDefaultTryGetItem(seq->tp_trygetitem))
					return &DeeMap_DefaultSetNewWithTryGetItemAndSetItem;
				return &DeeMap_DefaultSetNewWithGetItemAndSetItem;
			}
		}
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Set.insert' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Sequence.append' */
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireMapSetNewEx)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			struct type_seq *seq = orig_type->tp_seq;
			ASSERT(seq->tp_bounditem);
			ASSERT(seq->tp_trygetitem);
			ASSERT(seq->tp_getitem);
			if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_setdefault, 10, Dee_HashStr__setdefault, &attrinfo) &&
			    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_setdefault)) {
				if (!DeeType_IsDefaultTryGetItem(seq->tp_trygetitem))
					return &DeeMap_DefaultSetNewExWithTryGetItemAndTSCSetDefault;
				return &DeeMap_DefaultSetNewExWithGetItemAndTSCSetDefault;
			} else if (DeeType_HasPrivateOperator(self, OPERATOR_SETITEM)) {
				if (!DeeType_IsDefaultTryGetItem(seq->tp_trygetitem))
					return &DeeMap_DefaultSetNewExWithTryGetItemAndSetItem;
				return &DeeMap_DefaultSetNewExWithGetItemAndSetItem;
			}
		}
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Set.insert' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Sequence.append' */
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireMapSetDefault)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_setnew_ex, 9, Dee_HashStr__setnew_ex, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_setnew_ex))
			return &DeeMap_DefaultSetDefaultWithTSCSetNewEx;
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			struct type_seq *seq = orig_type->tp_seq;
			ASSERT(seq->tp_bounditem);
			ASSERT(seq->tp_trygetitem);
			ASSERT(seq->tp_getitem);
			if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_setnew, 6, Dee_HashStr__setnew, &attrinfo) &&
			    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_setnew)) {
				return &DeeMap_DefaultSetDefaultWithTSCSetNewAndGetItem;
			} else if (DeeType_HasPrivateOperator(self, OPERATOR_SETITEM)) {
				if (!DeeType_IsDefaultTryGetItem(seq->tp_trygetitem))
					return &DeeMap_DefaultSetDefaultWithTryGetItemAndSetItem;
				return &DeeMap_DefaultSetDefaultWithGetItemAndSetItem;
			}
		}
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Set.insert' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Sequence.append' */
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireMapUpdate)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_INPLACE_ADD))
			return &DeeMap_DefaultUpdateWithInplaceAdd;
		if (DeeType_HasPrivateOperator(self, OPERATOR_INPLACE_OR))
			return &DeeMap_DefaultUpdateWithInplaceOr;
		if (DeeType_HasPrivateOperator(self, OPERATOR_SETITEM))
			return &DeeMap_DefaultUpdateWithSetItem;
	} else {
		/* TODO: Treat as ?S?T2?O?O and use `Set.insert' or `Sequence.append' */
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireMapRemove)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM)) {
			if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM))
				return &DeeMap_DefaultRemoveWithBoundItemAndDelItem;
			if (DeeType_HasOperator(orig_type, OPERATOR_SIZE))
				return &DeeMap_DefaultRemoveWithSizeAndDelItem;
		}
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_removekeys, 10, Dee_HashStr__removekeys, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_removekeys)) {
			if (DeeType_HasOperator(orig_type, OPERATOR_SIZE))
				return &DeeMap_DefaultRemoveWithSizeAndTSCRemoveKeys;
		}
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Set.remove' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Sequence.erase' */
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireMapRemoveKeys)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM))
			return &DeeMap_DefaultRemoveKeysWithDelItem;
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_remove, 6, Dee_HashStr__remove, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_remove))
			return &DeeMap_DefaultRemoveKeysWithTSCRemove;
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Set.remove' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Sequence.erase' */
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireMapPop)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			Dee_tsc_map_remove_t tsc_map_remove;
			tsc_map_remove = DeeType_SeqCache_RequireMapRemove_private_uncached(orig_type, self);
			if (tsc_map_remove != NULL &&
			    tsc_map_remove != &DeeMap_DefaultRemoveWithError &&
			    tsc_map_remove != &DeeMap_DefaultRemoveWithSizeAndDelItem &&
			    tsc_map_remove != &DeeMap_DefaultRemoveWithBoundItemAndDelItem)
				return &DeeMap_DefaultPopWithGetItemAndTSCRemove;
			if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM))
				return &DeeMap_DefaultPopWithGetItemAndDelItem;
		}
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Set.remove' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Sequence.erase' */
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireMapPopWithDefault)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			Dee_tsc_map_remove_t tsc_map_remove;
			tsc_map_remove = DeeType_SeqCache_RequireMapRemove_private_uncached(orig_type, self);
			if (tsc_map_remove != NULL &&
			    tsc_map_remove != &DeeMap_DefaultRemoveWithError &&
			    tsc_map_remove != &DeeMap_DefaultRemoveWithSizeAndDelItem &&
			    tsc_map_remove != &DeeMap_DefaultRemoveWithBoundItemAndDelItem)
				return &DeeMap_DefaultPopWithDefaultWithTryGetItemAndTSCRemove;
			if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM))
				return &DeeMap_DefaultPopWithDefaultWithTryGetItemAndDelItem;
		}
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Set.remove' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Sequence.erase' */
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireMapPopItem)
	if (seqclass == Dee_SEQCLASS_MAP) {
		Dee_tsc_map_remove_t tsc_map_remove;
		tsc_map_remove = DeeType_SeqCache_RequireMapRemove_private_uncached(orig_type, self);
		if (tsc_map_remove != NULL &&
		    tsc_map_remove != &DeeMap_DefaultRemoveWithError &&
		    tsc_map_remove != &DeeMap_DefaultRemoveWithSizeAndDelItem &&
		    tsc_map_remove != &DeeMap_DefaultRemoveWithBoundItemAndDelItem)
			return &DeeMap_DefaultPopItemWithTSCFirstAndTSCRemove;
		if (DeeType_HasPrivateOperator(self, OPERATOR_DELITEM))
			return &DeeMap_DefaultPopItemWithTSCFirstAndDelItem;
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use `Set.pop' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use `Sequence.pop' */
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireMapKeys)
	{
		Dee_tsc_map_iterkeys_t tsc_map_iterkeys;
		tsc_map_iterkeys = DeeType_SeqCache_RequireMapIterKeys_private_uncached(orig_type, self);
		if (tsc_map_iterkeys != NULL &&
		    tsc_map_iterkeys != &DeeMap_DefaultIterKeysWithError)
			return &DeeMap_DefaultKeysWithTSCIterKeys;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireMapValues)
	{
		Dee_tsc_map_itervalues_t tsc_map_itervalues;
		tsc_map_itervalues = DeeType_SeqCache_RequireMapIterValues_private_uncached(orig_type, self);
		if (tsc_map_itervalues != NULL &&
		    tsc_map_itervalues != &DeeMap_DefaultIterValuesWithError)
			return &DeeMap_DefaultValuesWithTSCIterValues;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireMapIterKeys)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_keys, 4, Dee_HashStr__keys, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_keys))
			return &DeeMap_DefaultIterKeysWithTSCKeys;
		if (self->tp_seq && self->tp_seq->tp_iterkeys)
			return self->tp_seq->tp_iterkeys;
	}
	if (DeeType_HasPrivateOperator(self, OPERATOR_ITER))
		return &DeeMap_DefaultIterKeysWithIter;
#elif defined(DEFINE_DeeType_SeqCache_RequireMapIterValues)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_values, 6, Dee_HashStr__values, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_values))
			return &DeeMap_DefaultIterValuesWithTSCValues;
	}
	if (DeeType_HasPrivateOperator(self, OPERATOR_ITER))
		return &DeeMap_DefaultIterValuesWithIter;
#endif /* ... */
	return NULL;
}

PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_tsc_foo_t DCALL
LOCAL_DeeType_SeqCache_RequireFoo_uncached(DeeTypeObject *__restrict self) {
	DeeTypeObject *iter;
	DeeTypeMRO mro;
	iter = DeeTypeMRO_Init(&mro, self);
	do {
		LOCAL_Dee_tsc_foo_t result;
		result = LOCAL_DeeType_SeqCache_RequireFoo_private_uncached(self, iter);
		if (result)
			return result;
	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL);
	return &LOCAL_DeeSeq_DefaultFooWithError;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_tsc_foo_t DCALL
LOCAL_DeeType_SeqCache_RequireFoo(DeeTypeObject *__restrict self) {
	LOCAL_Dee_tsc_foo_t result;
	struct Dee_type_seq_cache *sc;
	struct Dee_type_seq *seq = self->tp_seq;
	if likely(seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->LOCAL_tsc_foo)
			return sc->LOCAL_tsc_foo;
	}
	result = LOCAL_DeeType_SeqCache_RequireFoo_uncached(self);
	sc     = DeeType_TryRequireSeqCache(self);
	if likely(sc)
		atomic_write(&sc->LOCAL_tsc_foo, result);
	return result;
}


#undef LOCAL_DeeSeq_AttrBar
#undef LOCAL_DeeType_SeqCache_RequireFoo_private_uncached
#undef LOCAL_DeeType_SeqCache_RequireFoo_uncached
#undef LOCAL_DeeType_SeqCache_RequireFoo
#undef LOCAL_DeeSeq_DefaultFooWithCallAttrBar
#undef LOCAL_DeeSeq_DefaultFooWithCallAttrBarForSeq
#undef LOCAL_DeeSeq_DefaultFooWithCallAttrBarForSetOrMap
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataFunctionForSeq
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataFunctionForSetOrMap
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataMethodForSeq
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataMethodForSetOrMap
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethodForSeq
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethodForSetOrMap
#undef LOCAL_DeeSeq_DefaultFooWithError
#undef LOCAL_Dee_tsc_foo_t
#undef LOCAL_tsc_foo_data
#undef LOCAL_CANONICAL_NAME_LENGTHOF
#undef LOCAL_CANONICAL_NAME_str
#undef LOCAL_CANONICAL_NAME_STR
#undef LOCAL_CANONICAL_NAME_Dee_HashStr
#undef LOCAL_IS_GETSET


#undef LOCAL_HAS_FOR_SEQ_SUFFIX
#undef LOCAL_ATTR_REQUIRED_SEQCLASS
#undef LOCAL_CANONICAL_NAME
#undef LOCAL_default_seq_foo
#undef LOCAL_tsc_foo
#undef LOCAL_DeeSeq_Default
#undef LOCAL_DeeSeq_RequireFoo
#undef LOCAL_DeeSeq_AttrFoo
#undef LOCAL_DeeSeq_AttrBar

DECL_END

#undef DEFINE_DeeType_SeqCache_RequireAny
#undef DEFINE_DeeType_SeqCache_RequireAnyWithKey
#undef DEFINE_DeeType_SeqCache_RequireAnyWithRange
#undef DEFINE_DeeType_SeqCache_RequireAnyWithRangeAndKey
#undef DEFINE_DeeType_SeqCache_RequireAll
#undef DEFINE_DeeType_SeqCache_RequireAllWithKey
#undef DEFINE_DeeType_SeqCache_RequireAllWithRange
#undef DEFINE_DeeType_SeqCache_RequireAllWithRangeAndKey
#undef DEFINE_DeeType_SeqCache_RequireParity
#undef DEFINE_DeeType_SeqCache_RequireParityWithKey
#undef DEFINE_DeeType_SeqCache_RequireParityWithRange
#undef DEFINE_DeeType_SeqCache_RequireParityWithRangeAndKey
#undef DEFINE_DeeType_SeqCache_RequireReduce
#undef DEFINE_DeeType_SeqCache_RequireReduceWithInit
#undef DEFINE_DeeType_SeqCache_RequireReduceWithRange
#undef DEFINE_DeeType_SeqCache_RequireReduceWithRangeAndInit
#undef DEFINE_DeeType_SeqCache_RequireMin
#undef DEFINE_DeeType_SeqCache_RequireMinWithKey
#undef DEFINE_DeeType_SeqCache_RequireMinWithRange
#undef DEFINE_DeeType_SeqCache_RequireMinWithRangeAndKey
#undef DEFINE_DeeType_SeqCache_RequireMax
#undef DEFINE_DeeType_SeqCache_RequireMaxWithKey
#undef DEFINE_DeeType_SeqCache_RequireMaxWithRange
#undef DEFINE_DeeType_SeqCache_RequireMaxWithRangeAndKey
#undef DEFINE_DeeType_SeqCache_RequireSum
#undef DEFINE_DeeType_SeqCache_RequireSumWithRange
#undef DEFINE_DeeType_SeqCache_RequireCount
#undef DEFINE_DeeType_SeqCache_RequireCountWithKey
#undef DEFINE_DeeType_SeqCache_RequireCountWithRange
#undef DEFINE_DeeType_SeqCache_RequireCountWithRangeAndKey
#undef DEFINE_DeeType_SeqCache_RequireContains
#undef DEFINE_DeeType_SeqCache_RequireContainsWithKey
#undef DEFINE_DeeType_SeqCache_RequireContainsWithRange
#undef DEFINE_DeeType_SeqCache_RequireContainsWithRangeAndKey
#undef DEFINE_DeeType_SeqCache_RequireLocate
#undef DEFINE_DeeType_SeqCache_RequireLocateWithKey
#undef DEFINE_DeeType_SeqCache_RequireLocateWithRange
#undef DEFINE_DeeType_SeqCache_RequireLocateWithRangeAndKey
#undef DEFINE_DeeType_SeqCache_RequireRLocateWithRange
#undef DEFINE_DeeType_SeqCache_RequireRLocateWithRangeAndKey
#undef DEFINE_DeeType_SeqCache_RequireStartsWith
#undef DEFINE_DeeType_SeqCache_RequireStartsWithWithKey
#undef DEFINE_DeeType_SeqCache_RequireStartsWithWithRange
#undef DEFINE_DeeType_SeqCache_RequireStartsWithWithRangeAndKey
#undef DEFINE_DeeType_SeqCache_RequireEndsWith
#undef DEFINE_DeeType_SeqCache_RequireEndsWithWithKey
#undef DEFINE_DeeType_SeqCache_RequireEndsWithWithRange
#undef DEFINE_DeeType_SeqCache_RequireEndsWithWithRangeAndKey
#undef DEFINE_DeeType_SeqCache_RequireFind
#undef DEFINE_DeeType_SeqCache_RequireFindWithKey
#undef DEFINE_DeeType_SeqCache_RequireRFind
#undef DEFINE_DeeType_SeqCache_RequireRFindWithKey
#undef DEFINE_DeeType_SeqCache_RequireErase
#undef DEFINE_DeeType_SeqCache_RequireInsert
#undef DEFINE_DeeType_SeqCache_RequireInsertAll
#undef DEFINE_DeeType_SeqCache_RequirePushFront
#undef DEFINE_DeeType_SeqCache_RequireAppend
#undef DEFINE_DeeType_SeqCache_RequireExtend
#undef DEFINE_DeeType_SeqCache_RequireXchItemIndex
#undef DEFINE_DeeType_SeqCache_RequireClear
#undef DEFINE_DeeType_SeqCache_RequirePop
#undef DEFINE_DeeType_SeqCache_RequireRemove
#undef DEFINE_DeeType_SeqCache_RequireRemoveWithKey
#undef DEFINE_DeeType_SeqCache_RequireRRemove
#undef DEFINE_DeeType_SeqCache_RequireRRemoveWithKey
#undef DEFINE_DeeType_SeqCache_RequireRemoveAll
#undef DEFINE_DeeType_SeqCache_RequireRemoveAllWithKey
#undef DEFINE_DeeType_SeqCache_RequireRemoveIf
#undef DEFINE_DeeType_SeqCache_RequireResize
#undef DEFINE_DeeType_SeqCache_RequireFill
#undef DEFINE_DeeType_SeqCache_RequireReverse
#undef DEFINE_DeeType_SeqCache_RequireReversed
#undef DEFINE_DeeType_SeqCache_RequireSort
#undef DEFINE_DeeType_SeqCache_RequireSortWithKey
#undef DEFINE_DeeType_SeqCache_RequireSorted
#undef DEFINE_DeeType_SeqCache_RequireSortedWithKey
#undef DEFINE_DeeType_SeqCache_RequireBFind
#undef DEFINE_DeeType_SeqCache_RequireBFindWithKey
#undef DEFINE_DeeType_SeqCache_RequireBPosition
#undef DEFINE_DeeType_SeqCache_RequireBPositionWithKey
#undef DEFINE_DeeType_SeqCache_RequireBRange
#undef DEFINE_DeeType_SeqCache_RequireBRangeWithKey
#undef DEFINE_DeeType_SeqCache_RequireBLocate
#undef DEFINE_DeeType_SeqCache_RequireBLocateWithKey
#undef DEFINE_DeeType_SeqCache_RequireSetInsert
#undef DEFINE_DeeType_SeqCache_RequireSetRemove
#undef DEFINE_DeeType_SeqCache_RequireSetUnify
#undef DEFINE_DeeType_SeqCache_RequireSetInsertAll
#undef DEFINE_DeeType_SeqCache_RequireSetRemoveAll
#undef DEFINE_DeeType_SeqCache_RequireSetPop
#undef DEFINE_DeeType_SeqCache_RequireSetPopWithDefault
#undef DEFINE_DeeType_SeqCache_RequireMapSetOld
#undef DEFINE_DeeType_SeqCache_RequireMapSetOldEx
#undef DEFINE_DeeType_SeqCache_RequireMapSetNew
#undef DEFINE_DeeType_SeqCache_RequireMapSetNewEx
#undef DEFINE_DeeType_SeqCache_RequireMapSetDefault
#undef DEFINE_DeeType_SeqCache_RequireMapUpdate
#undef DEFINE_DeeType_SeqCache_RequireMapRemove
#undef DEFINE_DeeType_SeqCache_RequireMapRemoveKeys
#undef DEFINE_DeeType_SeqCache_RequireMapPop
#undef DEFINE_DeeType_SeqCache_RequireMapPopWithDefault
#undef DEFINE_DeeType_SeqCache_RequireMapPopItem
#undef DEFINE_DeeType_SeqCache_RequireMapKeys
#undef DEFINE_DeeType_SeqCache_RequireMapValues
#undef DEFINE_DeeType_SeqCache_RequireMapIterKeys
#undef DEFINE_DeeType_SeqCache_RequireMapIterValues
