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
//#define DEFINE_DeeType_RequireSeqTryGetFirst
//#define DEFINE_DeeType_RequireSeqGetFirst
//#define DEFINE_DeeType_RequireSeqBoundFirst
//#define DEFINE_DeeType_RequireSeqDelFirst
//#define DEFINE_DeeType_RequireSetFirst
//#define DEFINE_DeeType_RequireSeqTryGetLast
//#define DEFINE_DeeType_RequireSeqGetLast
#define DEFINE_DeeType_RequireSeqBoundLast
//#define DEFINE_DeeType_RequireSeqDelLast
//#define DEFINE_DeeType_RequireSetLast
//#define DEFINE_DeeType_RequireSeqAny
//#define DEFINE_DeeType_RequireSeqAnyWithKey
//#define DEFINE_DeeType_RequireSeqAnyWithRange
//#define DEFINE_DeeType_RequireSeqAnyWithRangeAndKey
//#define DEFINE_DeeType_RequireSeqAll
//#define DEFINE_DeeType_RequireSeqAllWithKey
//#define DEFINE_DeeType_RequireSeqAllWithRange
//#define DEFINE_DeeType_RequireSeqAllWithRangeAndKey
//#define DEFINE_DeeType_RequireSeqParity
//#define DEFINE_DeeType_RequireSeqParityWithKey
//#define DEFINE_DeeType_RequireSeqParityWithRange
//#define DEFINE_DeeType_RequireSeqParityWithRangeAndKey
//#define DEFINE_DeeType_RequireSeqReduce
//#define DEFINE_DeeType_RequireSeqReduceWithInit
//#define DEFINE_DeeType_RequireSeqReduceWithRange
//#define DEFINE_DeeType_RequireSeqReduceWithRangeAndInit
//#define DEFINE_DeeType_RequireSeqMin
//#define DEFINE_DeeType_RequireSeqMinWithKey
//#define DEFINE_DeeType_RequireSeqMinWithRange
//#define DEFINE_DeeType_RequireSeqMinWithRangeAndKey
//#define DEFINE_DeeType_RequireSeqMax
//#define DEFINE_DeeType_RequireSeqMaxWithKey
//#define DEFINE_DeeType_RequireSeqMaxWithRange
//#define DEFINE_DeeType_RequireSeqMaxWithRangeAndKey
//#define DEFINE_DeeType_RequireSeqSum
//#define DEFINE_DeeType_RequireSeqSumWithRange
//#define DEFINE_DeeType_RequireSeqCount
//#define DEFINE_DeeType_RequireSeqCountWithKey
//#define DEFINE_DeeType_RequireSeqCountWithRange
//#define DEFINE_DeeType_RequireSeqCountWithRangeAndKey
//#define DEFINE_DeeType_RequireSeqContains
//#define DEFINE_DeeType_RequireSeqContainsWithKey
//#define DEFINE_DeeType_RequireSeqContainsWithRange
//#define DEFINE_DeeType_RequireSeqContainsWithRangeAndKey
//#define DEFINE_DeeType_RequireSeqLocate
//#define DEFINE_DeeType_RequireSeqLocateWithKey
//#define DEFINE_DeeType_RequireSeqLocateWithRange
//#define DEFINE_DeeType_RequireSeqLocateWithRangeAndKey
//#define DEFINE_DeeType_RequireSeqRLocateWithRange
//#define DEFINE_DeeType_RequireSeqRLocateWithRangeAndKey
//#define DEFINE_DeeType_RequireSeqStartsWith
//#define DEFINE_DeeType_RequireSeqStartsWithWithKey
//#define DEFINE_DeeType_RequireSeqStartsWithWithRange
//#define DEFINE_DeeType_RequireSeqStartsWithWithRangeAndKey
//#define DEFINE_DeeType_RequireSeqEndsWith
//#define DEFINE_DeeType_RequireSeqEndsWithWithKey
//#define DEFINE_DeeType_RequireSeqEndsWithWithRange
//#define DEFINE_DeeType_RequireSeqEndsWithWithRangeAndKey
//#define DEFINE_DeeType_RequireSeqFind
//#define DEFINE_DeeType_RequireSeqFindWithKey
//#define DEFINE_DeeType_RequireSeqRFind
//#define DEFINE_DeeType_RequireSeqRFindWithKey
//#define DEFINE_DeeType_RequireSeqErase
//#define DEFINE_DeeType_RequireSeqInsert
//#define DEFINE_DeeType_RequireSeqInsertAll
//#define DEFINE_DeeType_RequireSeqPushFront
//#define DEFINE_DeeType_RequireSeqAppend
//#define DEFINE_DeeType_RequireSeqExtend
//#define DEFINE_DeeType_RequireSeqXchItemIndex
//#define DEFINE_DeeType_RequireSeqClear
//#define DEFINE_DeeType_RequireSeqPop
//#define DEFINE_DeeType_RequireSeqRemove
//#define DEFINE_DeeType_RequireSeqRemoveWithKey
//#define DEFINE_DeeType_RequireSeqRRemove
//#define DEFINE_DeeType_RequireSeqRRemoveWithKey
//#define DEFINE_DeeType_RequireSeqRemoveAll
//#define DEFINE_DeeType_RequireSeqRemoveAllWithKey
//#define DEFINE_DeeType_RequireSeqRemoveIf
//#define DEFINE_DeeType_RequireSeqResize
//#define DEFINE_DeeType_RequireSeqFill
//#define DEFINE_DeeType_RequireSeqReverse
//#define DEFINE_DeeType_RequireSeqReversed
//#define DEFINE_DeeType_RequireSeqSort
//#define DEFINE_DeeType_RequireSeqSortWithKey
//#define DEFINE_DeeType_RequireSeqSorted
//#define DEFINE_DeeType_RequireSeqSortedWithKey
//#define DEFINE_DeeType_RequireSeqBFind
//#define DEFINE_DeeType_RequireSeqBFindWithKey
//#define DEFINE_DeeType_RequireSeqBPosition
//#define DEFINE_DeeType_RequireSeqBPositionWithKey
//#define DEFINE_DeeType_RequireSeqBRange
//#define DEFINE_DeeType_RequireSeqBRangeWithKey
//#define DEFINE_DeeType_RequireSeqBLocate
//#define DEFINE_DeeType_RequireSeqBLocateWithKey
//#define DEFINE_DeeType_RequireSetInsert
//#define DEFINE_DeeType_RequireSetRemove
//#define DEFINE_DeeType_RequireSetUnify
//#define DEFINE_DeeType_RequireSetInsertAll
//#define DEFINE_DeeType_RequireSetRemoveAll
//#define DEFINE_DeeType_RequireSetPop
//#define DEFINE_DeeType_RequireSetPopWithDefault
//#define DEFINE_DeeType_RequireMapSetOld
//#define DEFINE_DeeType_RequireMapSetOldEx
//#define DEFINE_DeeType_RequireMapSetNew
//#define DEFINE_DeeType_RequireMapSetNewEx
//#define DEFINE_DeeType_RequireMapSetDefault
//#define DEFINE_DeeType_RequireMapUpdate
//#define DEFINE_DeeType_RequireMapRemove
//#define DEFINE_DeeType_RequireMapRemoveKeys
//#define DEFINE_DeeType_RequireMapPop
//#define DEFINE_DeeType_RequireMapPopWithDefault
//#define DEFINE_DeeType_RequireMapPopItem
//#define DEFINE_DeeType_RequireMapKeys
//#define DEFINE_DeeType_RequireMapValues
//#define DEFINE_DeeType_RequireMapIterKeys
//#define DEFINE_DeeType_RequireMapIterValues
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeType_RequireSeqTryGetFirst) +               \
     defined(DEFINE_DeeType_RequireSeqGetFirst) +                  \
     defined(DEFINE_DeeType_RequireSeqBoundFirst) +                \
     defined(DEFINE_DeeType_RequireSeqDelFirst) +                  \
     defined(DEFINE_DeeType_RequireSetFirst) +                  \
     defined(DEFINE_DeeType_RequireSeqTryGetLast) +                \
     defined(DEFINE_DeeType_RequireSeqGetLast) +                   \
     defined(DEFINE_DeeType_RequireSeqBoundLast) +                 \
     defined(DEFINE_DeeType_RequireSeqDelLast) +                   \
     defined(DEFINE_DeeType_RequireSetLast) +                   \
     defined(DEFINE_DeeType_RequireSeqAny) +                       \
     defined(DEFINE_DeeType_RequireSeqAnyWithKey) +                \
     defined(DEFINE_DeeType_RequireSeqAnyWithRange) +              \
     defined(DEFINE_DeeType_RequireSeqAnyWithRangeAndKey) +        \
     defined(DEFINE_DeeType_RequireSeqAll) +                       \
     defined(DEFINE_DeeType_RequireSeqAllWithKey) +                \
     defined(DEFINE_DeeType_RequireSeqAllWithRange) +              \
     defined(DEFINE_DeeType_RequireSeqAllWithRangeAndKey) +        \
     defined(DEFINE_DeeType_RequireSeqParity) +                    \
     defined(DEFINE_DeeType_RequireSeqParityWithKey) +             \
     defined(DEFINE_DeeType_RequireSeqParityWithRange) +           \
     defined(DEFINE_DeeType_RequireSeqParityWithRangeAndKey) +     \
     defined(DEFINE_DeeType_RequireSeqReduce) +                    \
     defined(DEFINE_DeeType_RequireSeqReduceWithInit) +            \
     defined(DEFINE_DeeType_RequireSeqReduceWithRange) +           \
     defined(DEFINE_DeeType_RequireSeqReduceWithRangeAndInit) +    \
     defined(DEFINE_DeeType_RequireSeqMin) +                       \
     defined(DEFINE_DeeType_RequireSeqMinWithKey) +                \
     defined(DEFINE_DeeType_RequireSeqMinWithRange) +              \
     defined(DEFINE_DeeType_RequireSeqMinWithRangeAndKey) +        \
     defined(DEFINE_DeeType_RequireSeqMax) +                       \
     defined(DEFINE_DeeType_RequireSeqMaxWithKey) +                \
     defined(DEFINE_DeeType_RequireSeqMaxWithRange) +              \
     defined(DEFINE_DeeType_RequireSeqMaxWithRangeAndKey) +        \
     defined(DEFINE_DeeType_RequireSeqSum) +                       \
     defined(DEFINE_DeeType_RequireSeqSumWithRange) +              \
     defined(DEFINE_DeeType_RequireSeqCount) +                     \
     defined(DEFINE_DeeType_RequireSeqCountWithKey) +              \
     defined(DEFINE_DeeType_RequireSeqCountWithRange) +            \
     defined(DEFINE_DeeType_RequireSeqCountWithRangeAndKey) +      \
     defined(DEFINE_DeeType_RequireSeqContains) +                  \
     defined(DEFINE_DeeType_RequireSeqContainsWithKey) +           \
     defined(DEFINE_DeeType_RequireSeqContainsWithRange) +         \
     defined(DEFINE_DeeType_RequireSeqContainsWithRangeAndKey) +   \
     defined(DEFINE_DeeType_RequireSeqLocate) +                    \
     defined(DEFINE_DeeType_RequireSeqLocateWithKey) +             \
     defined(DEFINE_DeeType_RequireSeqLocateWithRange) +           \
     defined(DEFINE_DeeType_RequireSeqLocateWithRangeAndKey) +     \
     defined(DEFINE_DeeType_RequireSeqRLocateWithRange) +          \
     defined(DEFINE_DeeType_RequireSeqRLocateWithRangeAndKey) +    \
     defined(DEFINE_DeeType_RequireSeqStartsWith) +                \
     defined(DEFINE_DeeType_RequireSeqStartsWithWithKey) +         \
     defined(DEFINE_DeeType_RequireSeqStartsWithWithRange) +       \
     defined(DEFINE_DeeType_RequireSeqStartsWithWithRangeAndKey) + \
     defined(DEFINE_DeeType_RequireSeqEndsWith) +                  \
     defined(DEFINE_DeeType_RequireSeqEndsWithWithKey) +           \
     defined(DEFINE_DeeType_RequireSeqEndsWithWithRange) +         \
     defined(DEFINE_DeeType_RequireSeqEndsWithWithRangeAndKey) +   \
     defined(DEFINE_DeeType_RequireSeqFind) +                      \
     defined(DEFINE_DeeType_RequireSeqFindWithKey) +               \
     defined(DEFINE_DeeType_RequireSeqRFind) +                     \
     defined(DEFINE_DeeType_RequireSeqRFindWithKey) +              \
     defined(DEFINE_DeeType_RequireSeqErase) +                     \
     defined(DEFINE_DeeType_RequireSeqInsert) +                    \
     defined(DEFINE_DeeType_RequireSeqInsertAll) +                 \
     defined(DEFINE_DeeType_RequireSeqPushFront) +                 \
     defined(DEFINE_DeeType_RequireSeqAppend) +                    \
     defined(DEFINE_DeeType_RequireSeqExtend) +                    \
     defined(DEFINE_DeeType_RequireSeqXchItemIndex) +              \
     defined(DEFINE_DeeType_RequireSeqClear) +                     \
     defined(DEFINE_DeeType_RequireSeqPop) +                       \
     defined(DEFINE_DeeType_RequireSeqRemove) +                    \
     defined(DEFINE_DeeType_RequireSeqRemoveWithKey) +             \
     defined(DEFINE_DeeType_RequireSeqRRemove) +                   \
     defined(DEFINE_DeeType_RequireSeqRRemoveWithKey) +            \
     defined(DEFINE_DeeType_RequireSeqRemoveAll) +                 \
     defined(DEFINE_DeeType_RequireSeqRemoveAllWithKey) +          \
     defined(DEFINE_DeeType_RequireSeqRemoveIf) +                  \
     defined(DEFINE_DeeType_RequireSeqResize) +                    \
     defined(DEFINE_DeeType_RequireSeqFill) +                      \
     defined(DEFINE_DeeType_RequireSeqReverse) +                   \
     defined(DEFINE_DeeType_RequireSeqReversed) +                  \
     defined(DEFINE_DeeType_RequireSeqSort) +                      \
     defined(DEFINE_DeeType_RequireSeqSortWithKey) +               \
     defined(DEFINE_DeeType_RequireSeqSorted) +                    \
     defined(DEFINE_DeeType_RequireSeqSortedWithKey) +             \
     defined(DEFINE_DeeType_RequireSeqBFind) +                     \
     defined(DEFINE_DeeType_RequireSeqBFindWithKey) +              \
     defined(DEFINE_DeeType_RequireSeqBPosition) +                 \
     defined(DEFINE_DeeType_RequireSeqBPositionWithKey) +          \
     defined(DEFINE_DeeType_RequireSeqBRange) +                    \
     defined(DEFINE_DeeType_RequireSeqBRangeWithKey) +             \
     defined(DEFINE_DeeType_RequireSeqBLocate) +                   \
     defined(DEFINE_DeeType_RequireSeqBLocateWithKey) +            \
     defined(DEFINE_DeeType_RequireSetInsert) +                 \
     defined(DEFINE_DeeType_RequireSetRemove) +                 \
     defined(DEFINE_DeeType_RequireSetUnify) +                  \
     defined(DEFINE_DeeType_RequireSetInsertAll) +              \
     defined(DEFINE_DeeType_RequireSetRemoveAll) +              \
     defined(DEFINE_DeeType_RequireSetPop) +                    \
     defined(DEFINE_DeeType_RequireSetPopWithDefault) +         \
     defined(DEFINE_DeeType_RequireMapSetOld) +                 \
     defined(DEFINE_DeeType_RequireMapSetOldEx) +               \
     defined(DEFINE_DeeType_RequireMapSetNew) +                 \
     defined(DEFINE_DeeType_RequireMapSetNewEx) +               \
     defined(DEFINE_DeeType_RequireMapSetDefault) +             \
     defined(DEFINE_DeeType_RequireMapUpdate) +                 \
     defined(DEFINE_DeeType_RequireMapRemove) +                 \
     defined(DEFINE_DeeType_RequireMapRemoveKeys) +             \
     defined(DEFINE_DeeType_RequireMapPop) +                    \
     defined(DEFINE_DeeType_RequireMapPopWithDefault) +         \
     defined(DEFINE_DeeType_RequireMapPopItem) +                \
     defined(DEFINE_DeeType_RequireMapKeys) +                   \
     defined(DEFINE_DeeType_RequireMapValues) +                 \
     defined(DEFINE_DeeType_RequireMapIterKeys) +               \
     defined(DEFINE_DeeType_RequireMapIterValues)) != 1
#error "Must #define exactly one of these macros"
#endif /* DEFINE_DeeType_RequireSeq... */

DECL_BEGIN



#ifdef DEFINE_DeeType_RequireSeqTryGetFirst
#define LOCAL_CANONICAL_NAME             first
#define LOCAL_tsc_foo                    tsc_trygetfirst
#define LOCAL_DeeSeq_AttrFoo             TryGetFirst
#define LOCAL_IS_OPERATOR
#elif defined(DEFINE_DeeType_RequireSeqGetFirst)
#define LOCAL_CANONICAL_NAME             first
#define LOCAL_default_seq_foo            default_seq_getfirst
#define LOCAL_tsc_foo                    tsc_getfirst
#define LOCAL_tsc_foo_data               tsc_getfirst_data
#define LOCAL_DeeSeq_AttrFoo             GetFirst
#define LOCAL_IS_GETSET_GET
#elif defined(DEFINE_DeeType_RequireSeqBoundFirst)
#define LOCAL_CANONICAL_NAME             first
#define LOCAL_default_seq_foo            default_seq_boundfirst
#define LOCAL_tsc_foo                    tsc_boundfirst
#define LOCAL_tsc_foo_data               tsc_getfirst_data
#define LOCAL_DeeSeq_AttrFoo             BoundFirst
#define LOCAL_DeeSeq_AttrBar             GetFirst
#define LOCAL_IS_GETSET_BOUND
#elif defined(DEFINE_DeeType_RequireSeqDelFirst)
#define LOCAL_CANONICAL_NAME             first
#define LOCAL_default_seq_foo            default_seq_delfirst
#define LOCAL_tsc_foo                    tsc_delfirst
#define LOCAL_tsc_foo_data               tsc_delfirst_data
#define LOCAL_DeeSeq_AttrFoo             DelFirst
#define LOCAL_IS_GETSET_DEL
#elif defined(DEFINE_DeeType_RequireSetFirst)
#define LOCAL_CANONICAL_NAME             first
#define LOCAL_default_seq_foo            default_seq_setfirst
#define LOCAL_tsc_foo                    tsc_setfirst
#define LOCAL_tsc_foo_data               tsc_setfirst_data
#define LOCAL_DeeSeq_AttrFoo             SetFirst
#define LOCAL_IS_GETSET_SET
#elif defined(DEFINE_DeeType_RequireSeqTryGetLast)
#define LOCAL_CANONICAL_NAME             last
#define LOCAL_tsc_foo                    tsc_trygetlast
#define LOCAL_DeeSeq_AttrFoo             TryGetLast
#define LOCAL_IS_OPERATOR
#elif defined(DEFINE_DeeType_RequireSeqGetLast)
#define LOCAL_CANONICAL_NAME             last
#define LOCAL_default_seq_foo            default_seq_getlast
#define LOCAL_tsc_foo                    tsc_getlast
#define LOCAL_tsc_foo_data               tsc_getlast_data
#define LOCAL_DeeSeq_AttrFoo             GetLast
#define LOCAL_IS_GETSET_GET
#elif defined(DEFINE_DeeType_RequireSeqBoundLast)
#define LOCAL_CANONICAL_NAME             last
#define LOCAL_default_seq_foo            default_seq_boundlast
#define LOCAL_tsc_foo                    tsc_boundlast
#define LOCAL_tsc_foo_data               tsc_getlast_data
#define LOCAL_DeeSeq_AttrFoo             BoundLast
#define LOCAL_DeeSeq_AttrBar             GetLast
#define LOCAL_IS_GETSET_BOUND
#elif defined(DEFINE_DeeType_RequireSeqDelLast)
#define LOCAL_CANONICAL_NAME             last
#define LOCAL_default_seq_foo            default_seq_dellast
#define LOCAL_tsc_foo                    tsc_dellast
#define LOCAL_tsc_foo_data               tsc_dellast_data
#define LOCAL_DeeSeq_AttrFoo             DelLast
#define LOCAL_IS_GETSET_DEL
#elif defined(DEFINE_DeeType_RequireSetLast)
#define LOCAL_CANONICAL_NAME             last
#define LOCAL_default_seq_foo            default_seq_setlast
#define LOCAL_tsc_foo                    tsc_setlast
#define LOCAL_tsc_foo_data               tsc_setlast_data
#define LOCAL_DeeSeq_AttrFoo             SetLast
#define LOCAL_IS_GETSET_SET
#elif defined(DEFINE_DeeType_RequireSeqAny)
#define LOCAL_CANONICAL_NAME             any
#define LOCAL_default_seq_foo            default_seq_any
#define LOCAL_tsc_foo                    tsc_any
#define LOCAL_DeeSeq_AttrFoo             Any
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAnyWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqAnyWithKey)
#define LOCAL_CANONICAL_NAME             any
#define LOCAL_default_seq_foo            default_seq_any
#define LOCAL_tsc_foo                    tsc_any_with_key
#define LOCAL_DeeSeq_AttrFoo             AnyWithKey
#define LOCAL_DeeSeq_AttrBar             Any
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAnyWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqAnyWithRange)
#define LOCAL_CANONICAL_NAME             any
#define LOCAL_default_seq_foo            default_seq_any
#define LOCAL_tsc_foo                    tsc_any_with_range
#define LOCAL_DeeSeq_AttrFoo             AnyWithRange
#define LOCAL_DeeSeq_AttrBar             Any
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAnyWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqAnyWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             any
#define LOCAL_default_seq_foo            default_seq_any
#define LOCAL_tsc_foo                    tsc_any_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             AnyWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             Any
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAnyWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqAll)
#define LOCAL_CANONICAL_NAME             all
#define LOCAL_default_seq_foo            default_seq_all
#define LOCAL_tsc_foo                    tsc_all
#define LOCAL_DeeSeq_AttrFoo             All
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAllWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqAllWithKey)
#define LOCAL_CANONICAL_NAME             all
#define LOCAL_default_seq_foo            default_seq_all
#define LOCAL_tsc_foo                    tsc_all_with_key
#define LOCAL_DeeSeq_AttrFoo             AllWithKey
#define LOCAL_DeeSeq_AttrBar             All
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAllWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqAllWithRange)
#define LOCAL_CANONICAL_NAME             all
#define LOCAL_default_seq_foo            default_seq_all
#define LOCAL_tsc_foo                    tsc_all_with_range
#define LOCAL_DeeSeq_AttrFoo             AllWithRange
#define LOCAL_DeeSeq_AttrBar             All
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAllWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqAllWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             all
#define LOCAL_default_seq_foo            default_seq_all
#define LOCAL_tsc_foo                    tsc_all_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             AllWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             All
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAllWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqParity)
#define LOCAL_CANONICAL_NAME             parity
#define LOCAL_default_seq_foo            default_seq_parity
#define LOCAL_tsc_foo                    tsc_parity
#define LOCAL_DeeSeq_AttrFoo             Parity
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultParityWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqParityWithKey)
#define LOCAL_CANONICAL_NAME             parity
#define LOCAL_default_seq_foo            default_seq_parity
#define LOCAL_tsc_foo                    tsc_parity_with_key
#define LOCAL_DeeSeq_AttrFoo             ParityWithKey
#define LOCAL_DeeSeq_AttrBar             Parity
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultParityWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqParityWithRange)
#define LOCAL_CANONICAL_NAME             parity
#define LOCAL_default_seq_foo            default_seq_parity
#define LOCAL_tsc_foo                    tsc_parity_with_range
#define LOCAL_DeeSeq_AttrFoo             ParityWithRange
#define LOCAL_DeeSeq_AttrBar             Parity
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultParityWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqParityWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             parity
#define LOCAL_default_seq_foo            default_seq_parity
#define LOCAL_tsc_foo                    tsc_parity_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             ParityWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             Parity
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultParityWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqReduce)
#define LOCAL_CANONICAL_NAME             reduce
#define LOCAL_default_seq_foo            default_seq_reduce
#define LOCAL_tsc_foo                    tsc_reduce
#define LOCAL_DeeSeq_AttrFoo             Reduce
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReduceWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqReduceWithInit)
#define LOCAL_CANONICAL_NAME             reduce
#define LOCAL_default_seq_foo            default_seq_reduce
#define LOCAL_tsc_foo                    tsc_reduce_with_init
#define LOCAL_DeeSeq_AttrFoo             ReduceWithInit
#define LOCAL_DeeSeq_AttrBar             Reduce
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReduceWithInitWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqReduceWithRange)
#define LOCAL_CANONICAL_NAME             reduce
#define LOCAL_default_seq_foo            default_seq_reduce
#define LOCAL_tsc_foo                    tsc_reduce_with_range
#define LOCAL_DeeSeq_AttrFoo             ReduceWithRange
#define LOCAL_DeeSeq_AttrBar             Reduce
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReduceWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqReduceWithRangeAndInit)
#define LOCAL_CANONICAL_NAME             reduce
#define LOCAL_default_seq_foo            default_seq_reduce
#define LOCAL_tsc_foo                    tsc_reduce_with_range_and_init
#define LOCAL_DeeSeq_AttrFoo             ReduceWithRangeAndInit
#define LOCAL_DeeSeq_AttrBar             Reduce
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReduceWithRangeAndInitWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqMin)
#define LOCAL_CANONICAL_NAME             min
#define LOCAL_default_seq_foo            default_seq_min
#define LOCAL_tsc_foo                    tsc_min
#define LOCAL_DeeSeq_AttrFoo             Min
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMinWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqMinWithKey)
#define LOCAL_CANONICAL_NAME             min
#define LOCAL_default_seq_foo            default_seq_min
#define LOCAL_tsc_foo                    tsc_min_with_key
#define LOCAL_DeeSeq_AttrFoo             MinWithKey
#define LOCAL_DeeSeq_AttrBar             Min
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMinWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqMinWithRange)
#define LOCAL_CANONICAL_NAME             min
#define LOCAL_default_seq_foo            default_seq_min
#define LOCAL_tsc_foo                    tsc_min_with_range
#define LOCAL_DeeSeq_AttrFoo             MinWithRange
#define LOCAL_DeeSeq_AttrBar             Min
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMinWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqMinWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             min
#define LOCAL_default_seq_foo            default_seq_min
#define LOCAL_tsc_foo                    tsc_min_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             MinWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             Min
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMinWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqMax)
#define LOCAL_CANONICAL_NAME             max
#define LOCAL_default_seq_foo            default_seq_max
#define LOCAL_tsc_foo                    tsc_max
#define LOCAL_DeeSeq_AttrFoo             Max
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMaxWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqMaxWithKey)
#define LOCAL_CANONICAL_NAME             max
#define LOCAL_default_seq_foo            default_seq_max
#define LOCAL_tsc_foo                    tsc_max_with_key
#define LOCAL_DeeSeq_AttrFoo             MaxWithKey
#define LOCAL_DeeSeq_AttrBar             Max
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMaxWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqMaxWithRange)
#define LOCAL_CANONICAL_NAME             max
#define LOCAL_default_seq_foo            default_seq_max
#define LOCAL_tsc_foo                    tsc_max_with_range
#define LOCAL_DeeSeq_AttrFoo             MaxWithRange
#define LOCAL_DeeSeq_AttrBar             Max
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMaxWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqMaxWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             max
#define LOCAL_default_seq_foo            default_seq_max
#define LOCAL_tsc_foo                    tsc_max_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             MaxWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             Max
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMaxWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqSum)
#define LOCAL_CANONICAL_NAME             sum
#define LOCAL_default_seq_foo            default_seq_sum
#define LOCAL_tsc_foo                    tsc_sum
#define LOCAL_DeeSeq_AttrFoo             Sum
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultSumWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqSumWithRange)
#define LOCAL_CANONICAL_NAME             sum
#define LOCAL_default_seq_foo            default_seq_sum
#define LOCAL_tsc_foo                    tsc_sum_with_range
#define LOCAL_DeeSeq_AttrFoo             SumWithRange
#define LOCAL_DeeSeq_AttrBar             Sum
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultSumWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqCount)
#define LOCAL_CANONICAL_NAME             count
#define LOCAL_default_seq_foo            default_seq_count
#define LOCAL_tsc_foo                    tsc_count
#define LOCAL_DeeSeq_AttrFoo             Count
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultCountWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqCountWithKey)
#define LOCAL_CANONICAL_NAME             count
#define LOCAL_default_seq_foo            default_seq_count
#define LOCAL_tsc_foo                    tsc_count_with_key
#define LOCAL_DeeSeq_AttrFoo             CountWithKey
#define LOCAL_DeeSeq_AttrBar             Count
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultCountWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqCountWithRange)
#define LOCAL_CANONICAL_NAME             count
#define LOCAL_default_seq_foo            default_seq_count
#define LOCAL_tsc_foo                    tsc_count_with_range
#define LOCAL_DeeSeq_AttrFoo             CountWithRange
#define LOCAL_DeeSeq_AttrBar             Count
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultCountWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqCountWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             count
#define LOCAL_default_seq_foo            default_seq_count
#define LOCAL_tsc_foo                    tsc_count_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             CountWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             Count
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultCountWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqContains)
#define LOCAL_CANONICAL_NAME             contains
#define LOCAL_default_seq_foo            default_seq_contains
#define LOCAL_tsc_foo                    tsc_contains
#define LOCAL_DeeSeq_AttrFoo             Contains
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultContainsWithForeach
#elif defined(DEFINE_DeeType_RequireSeqContainsWithKey)
#define LOCAL_CANONICAL_NAME             contains
#define LOCAL_default_seq_foo            default_seq_contains
#define LOCAL_tsc_foo                    tsc_contains_with_key
#define LOCAL_DeeSeq_AttrFoo             ContainsWithKey
#define LOCAL_DeeSeq_AttrBar             Contains
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultContainsWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqContainsWithRange)
#define LOCAL_CANONICAL_NAME             contains
#define LOCAL_default_seq_foo            default_seq_contains
#define LOCAL_tsc_foo                    tsc_contains_with_range
#define LOCAL_DeeSeq_AttrFoo             ContainsWithRange
#define LOCAL_DeeSeq_AttrBar             Contains
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultContainsWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqContainsWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             contains
#define LOCAL_default_seq_foo            default_seq_contains
#define LOCAL_tsc_foo                    tsc_contains_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             ContainsWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             Contains
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultContainsWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqLocate)
#define LOCAL_CANONICAL_NAME             locate
#define LOCAL_default_seq_foo            default_seq_locate
#define LOCAL_tsc_foo                    tsc_locate
#define LOCAL_DeeSeq_AttrFoo             Locate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultLocateWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqLocateWithKey)
#define LOCAL_CANONICAL_NAME             locate
#define LOCAL_default_seq_foo            default_seq_locate
#define LOCAL_tsc_foo                    tsc_locate_with_key
#define LOCAL_DeeSeq_AttrFoo             LocateWithKey
#define LOCAL_DeeSeq_AttrBar             Locate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultLocateWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqLocateWithRange)
#define LOCAL_CANONICAL_NAME             locate
#define LOCAL_default_seq_foo            default_seq_locate
#define LOCAL_tsc_foo                    tsc_locate_with_range
#define LOCAL_DeeSeq_AttrFoo             LocateWithRange
#define LOCAL_DeeSeq_AttrBar             Locate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultLocateWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqLocateWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             locate
#define LOCAL_default_seq_foo            default_seq_locate
#define LOCAL_tsc_foo                    tsc_locate_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             LocateWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             Locate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultLocateWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRLocateWithRange)
#define LOCAL_CANONICAL_NAME             rlocate
#define LOCAL_default_seq_foo            default_seq_rlocate
#define LOCAL_tsc_foo                    tsc_rlocate_with_range
#define LOCAL_DeeSeq_AttrFoo             RLocateWithRange
#define LOCAL_DeeSeq_AttrBar             RLocate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultRLocateWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRLocateWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             rlocate
#define LOCAL_default_seq_foo            default_seq_rlocate
#define LOCAL_tsc_foo                    tsc_rlocate_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             RLocateWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             RLocate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultRLocateWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqStartsWith)
#define LOCAL_CANONICAL_NAME             startswith
#define LOCAL_default_seq_foo            default_seq_startswith
#define LOCAL_tsc_foo                    tsc_startswith
#define LOCAL_DeeSeq_AttrFoo             StartsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultStartsWithWithSeqTryGetFirst
#elif defined(DEFINE_DeeType_RequireSeqStartsWithWithKey)
#define LOCAL_CANONICAL_NAME             startswith
#define LOCAL_default_seq_foo            default_seq_startswith
#define LOCAL_tsc_foo                    tsc_startswith_with_key
#define LOCAL_DeeSeq_AttrFoo             StartsWithWithKey
#define LOCAL_DeeSeq_AttrBar             StartsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultStartsWithWithKeyWithSeqTryGetFirst
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqStartsWithWithRange)
#define LOCAL_CANONICAL_NAME             startswith
#define LOCAL_default_seq_foo            default_seq_startswith
#define LOCAL_tsc_foo                    tsc_startswith_with_range
#define LOCAL_DeeSeq_AttrFoo             StartsWithWithRange
#define LOCAL_DeeSeq_AttrBar             StartsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultStartsWithWithRangeWithSeqTryGetItemIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqStartsWithWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             startswith
#define LOCAL_default_seq_foo            default_seq_startswith
#define LOCAL_tsc_foo                    tsc_startswith_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             StartsWithWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             StartsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultStartsWithWithRangeAndKeyWithSeqTryGetItemIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqEndsWith)
#define LOCAL_CANONICAL_NAME             endswith
#define LOCAL_default_seq_foo            default_seq_endswith
#define LOCAL_tsc_foo                    tsc_endswith
#define LOCAL_DeeSeq_AttrFoo             EndsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultEndsWithWithSeqTryGetLast
#elif defined(DEFINE_DeeType_RequireSeqEndsWithWithKey)
#define LOCAL_CANONICAL_NAME             endswith
#define LOCAL_default_seq_foo            default_seq_endswith
#define LOCAL_tsc_foo                    tsc_endswith_with_key
#define LOCAL_DeeSeq_AttrFoo             EndsWithWithKey
#define LOCAL_DeeSeq_AttrBar             EndsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultEndsWithWithKeyWithSeqTryGetLast
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqEndsWithWithRange)
#define LOCAL_CANONICAL_NAME             endswith
#define LOCAL_default_seq_foo            default_seq_endswith
#define LOCAL_tsc_foo                    tsc_endswith_with_range
#define LOCAL_DeeSeq_AttrFoo             EndsWithWithRange
#define LOCAL_DeeSeq_AttrBar             EndsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultEndsWithWithRangeWithSeqSizeAndSeqTryGetItemIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqEndsWithWithRangeAndKey)
#define LOCAL_CANONICAL_NAME             endswith
#define LOCAL_default_seq_foo            default_seq_endswith
#define LOCAL_tsc_foo                    tsc_endswith_with_range_and_key
#define LOCAL_DeeSeq_AttrFoo             EndsWithWithRangeAndKey
#define LOCAL_DeeSeq_AttrBar             EndsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultEndsWithWithRangeAndKeyWithSeqSizeAndSeqTryGetItemIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqFind)
#define LOCAL_CANONICAL_NAME             find
#define LOCAL_default_seq_foo            default_seq_find
#define LOCAL_tsc_foo                    tsc_find
#define LOCAL_DeeSeq_AttrFoo             Find
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultFindWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqFindWithKey)
#define LOCAL_CANONICAL_NAME             find
#define LOCAL_default_seq_foo            default_seq_find
#define LOCAL_tsc_foo                    tsc_find_with_key
#define LOCAL_DeeSeq_AttrFoo             FindWithKey
#define LOCAL_DeeSeq_AttrBar             Find
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultFindWithKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRFind)
#define LOCAL_CANONICAL_NAME             rfind
#define LOCAL_default_seq_foo            default_seq_rfind
#define LOCAL_tsc_foo                    tsc_rfind
#define LOCAL_DeeSeq_AttrFoo             RFind
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultRFindWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRFindWithKey)
#define LOCAL_CANONICAL_NAME             rfind
#define LOCAL_default_seq_foo            default_seq_rfind
#define LOCAL_tsc_foo                    tsc_rfind_with_key
#define LOCAL_DeeSeq_AttrFoo             RFindWithKey
#define LOCAL_DeeSeq_AttrBar             RFind
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqErase)
#define LOCAL_CANONICAL_NAME         erase
#define LOCAL_default_seq_foo        default_seq_erase
#define LOCAL_tsc_foo                tsc_erase
#define LOCAL_DeeSeq_AttrFoo         Erase
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqInsert)
#define LOCAL_CANONICAL_NAME         insert
#define LOCAL_default_seq_foo        default_seq_insert
#define LOCAL_tsc_foo                tsc_insert
#define LOCAL_DeeSeq_AttrFoo         Insert
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqInsertAll)
#define LOCAL_CANONICAL_NAME         insertall
#define LOCAL_default_seq_foo        default_seq_insertall
#define LOCAL_tsc_foo                tsc_insertall
#define LOCAL_DeeSeq_AttrFoo         InsertAll
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqPushFront)
#define LOCAL_CANONICAL_NAME             pushfront
#define LOCAL_default_seq_foo            default_seq_pushfront
#define LOCAL_tsc_foo                    tsc_pushfront
#define LOCAL_DeeSeq_AttrFoo             PushFront
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultPushFrontWithSeqInsert /* Use insert() by default */
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqAppend)
#define LOCAL_CANONICAL_NAME         append
#define LOCAL_default_seq_foo        default_seq_append
#define LOCAL_tsc_foo                tsc_append
#define LOCAL_DeeSeq_AttrFoo         Append
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqExtend)
#define LOCAL_CANONICAL_NAME         extend
#define LOCAL_default_seq_foo        default_seq_extend
#define LOCAL_tsc_foo                tsc_extend
#define LOCAL_DeeSeq_AttrFoo         Extend
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqXchItemIndex)
#define LOCAL_CANONICAL_NAME         xchitem
#define LOCAL_default_seq_foo        default_seq_xchitem
#define LOCAL_tsc_foo                tsc_xchitem_index
#define LOCAL_tsc_foo_data           tsc_xchitem_data
#define LOCAL_DeeSeq_AttrFoo         XchItemIndex
#define LOCAL_DeeSeq_AttrBar         XchItem
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqClear)
#define LOCAL_CANONICAL_NAME         clear
#define LOCAL_default_seq_foo        default_seq_clear
#define LOCAL_tsc_foo                tsc_clear
#define LOCAL_DeeSeq_AttrFoo         Clear
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqPop)
#define LOCAL_CANONICAL_NAME         pop
#define LOCAL_default_seq_foo        default_seq_pop
#define LOCAL_tsc_foo                tsc_pop
#define LOCAL_DeeSeq_AttrFoo         Pop
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRemove)
#define LOCAL_CANONICAL_NAME         remove
#define LOCAL_default_seq_foo        default_seq_remove
#define LOCAL_tsc_foo                tsc_remove
#define LOCAL_DeeSeq_AttrFoo         Remove
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRemoveWithKey)
#define LOCAL_CANONICAL_NAME         remove
#define LOCAL_default_seq_foo        default_seq_remove
#define LOCAL_tsc_foo                tsc_remove_with_key
#define LOCAL_DeeSeq_AttrFoo         RemoveWithKey
#define LOCAL_DeeSeq_AttrBar         Remove
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRRemove)
#define LOCAL_CANONICAL_NAME         rremove
#define LOCAL_default_seq_foo        default_seq_rremove
#define LOCAL_tsc_foo                tsc_rremove
#define LOCAL_DeeSeq_AttrFoo         RRemove
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRRemoveWithKey)
#define LOCAL_CANONICAL_NAME         rremove
#define LOCAL_default_seq_foo        default_seq_rremove
#define LOCAL_tsc_foo                tsc_rremove_with_key
#define LOCAL_DeeSeq_AttrFoo         RRemoveWithKey
#define LOCAL_DeeSeq_AttrBar         RRemove
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRemoveAll)
#define LOCAL_CANONICAL_NAME         removeall
#define LOCAL_default_seq_foo        default_seq_removeall
#define LOCAL_tsc_foo                tsc_removeall
#define LOCAL_DeeSeq_AttrFoo         RemoveAll
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRemoveAllWithKey)
#define LOCAL_CANONICAL_NAME         removeall
#define LOCAL_default_seq_foo        default_seq_removeall
#define LOCAL_tsc_foo                tsc_removeall_with_key
#define LOCAL_DeeSeq_AttrFoo         RemoveAllWithKey
#define LOCAL_DeeSeq_AttrBar         RemoveAll
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRemoveIf)
#define LOCAL_CANONICAL_NAME         removeif
#define LOCAL_default_seq_foo        default_seq_removeif
#define LOCAL_tsc_foo                tsc_removeif
#define LOCAL_DeeSeq_AttrFoo         RemoveIf
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqResize)
#define LOCAL_CANONICAL_NAME             resize
#define LOCAL_default_seq_foo            default_seq_resize
#define LOCAL_tsc_foo                    tsc_resize
#define LOCAL_DeeSeq_AttrFoo             Resize
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultResizeWithSizeAndSeqEraseAndSeqExtend /* Use erase() and extend() by default */
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqFill)
#define LOCAL_CANONICAL_NAME         fill
#define LOCAL_default_seq_foo        default_seq_fill
#define LOCAL_tsc_foo                tsc_fill
#define LOCAL_DeeSeq_AttrFoo         Fill
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqReverse)
#define LOCAL_CANONICAL_NAME         reverse
#define LOCAL_default_seq_foo        default_seq_reverse
#define LOCAL_tsc_foo                tsc_reverse
#define LOCAL_DeeSeq_AttrFoo         Reverse
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqReversed)
#define LOCAL_CANONICAL_NAME             reversed
#define LOCAL_default_seq_foo            default_seq_reversed
#define LOCAL_tsc_foo                    tsc_reversed
#define LOCAL_DeeSeq_AttrFoo             Reversed
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReversedWithCopyForeachDefault
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqSort)
#define LOCAL_CANONICAL_NAME  sort
#define LOCAL_default_seq_foo default_seq_sort
#define LOCAL_tsc_foo         tsc_sort
#define LOCAL_DeeSeq_AttrFoo  Sort
#elif defined(DEFINE_DeeType_RequireSeqSortWithKey)
#define LOCAL_CANONICAL_NAME  sort
#define LOCAL_default_seq_foo default_seq_sort
#define LOCAL_tsc_foo         tsc_sort_with_key
#define LOCAL_DeeSeq_AttrFoo  SortWithKey
#define LOCAL_DeeSeq_AttrBar  Sort
#elif defined(DEFINE_DeeType_RequireSeqSorted)
#define LOCAL_CANONICAL_NAME             sorted
#define LOCAL_default_seq_foo            default_seq_sorted
#define LOCAL_tsc_foo                    tsc_sorted
#define LOCAL_DeeSeq_AttrFoo             Sorted
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultSortedWithCopyForeachDefault
#elif defined(DEFINE_DeeType_RequireSeqSortedWithKey)
#define LOCAL_CANONICAL_NAME             sorted
#define LOCAL_default_seq_foo            default_seq_sorted
#define LOCAL_tsc_foo                    tsc_sorted_with_key
#define LOCAL_DeeSeq_AttrFoo             SortedWithKey
#define LOCAL_DeeSeq_AttrBar             Sorted
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultSortedWithKeyWithCopyForeachDefault
#elif defined(DEFINE_DeeType_RequireSeqBFind)
#define LOCAL_CANONICAL_NAME         bfind
#define LOCAL_default_seq_foo        default_seq_bfind
#define LOCAL_tsc_foo                tsc_bfind
#define LOCAL_DeeSeq_AttrFoo         BFind
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqBFindWithKey)
#define LOCAL_CANONICAL_NAME         bfind
#define LOCAL_default_seq_foo        default_seq_bfind
#define LOCAL_tsc_foo                tsc_bfind_with_key
#define LOCAL_DeeSeq_AttrFoo         BFindWithKey
#define LOCAL_DeeSeq_AttrBar         BFind
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqBPosition)
#define LOCAL_CANONICAL_NAME         bposition
#define LOCAL_default_seq_foo        default_seq_bposition
#define LOCAL_tsc_foo                tsc_bposition
#define LOCAL_DeeSeq_AttrFoo         BPosition
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqBPositionWithKey)
#define LOCAL_CANONICAL_NAME         bposition
#define LOCAL_default_seq_foo        default_seq_bposition
#define LOCAL_tsc_foo                tsc_bposition_with_key
#define LOCAL_DeeSeq_AttrFoo         BPositionWithKey
#define LOCAL_DeeSeq_AttrBar         BPosition
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqBRange)
#define LOCAL_CANONICAL_NAME         brange
#define LOCAL_default_seq_foo        default_seq_brange
#define LOCAL_tsc_foo                tsc_brange
#define LOCAL_DeeSeq_AttrFoo         BRange
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqBRangeWithKey)
#define LOCAL_CANONICAL_NAME         brange
#define LOCAL_default_seq_foo        default_seq_brange
#define LOCAL_tsc_foo                tsc_brange_with_key
#define LOCAL_DeeSeq_AttrFoo         BRangeWithKey
#define LOCAL_DeeSeq_AttrBar         BRange
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqBLocate)
#define LOCAL_CANONICAL_NAME         blocate
#define LOCAL_default_seq_foo        default_seq_blocate
#define LOCAL_tsc_foo                tsc_blocate
#define LOCAL_DeeSeq_AttrFoo         BLocate
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqBLocateWithKey)
#define LOCAL_CANONICAL_NAME         blocate
#define LOCAL_default_seq_foo        default_seq_blocate
#define LOCAL_tsc_foo                tsc_blocate_with_key
#define LOCAL_DeeSeq_AttrFoo         BLocateWithKey
#define LOCAL_DeeSeq_AttrBar         BLocate
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSetInsert)
#define LOCAL_CANONICAL_NAME         insert
#define LOCAL_default_seq_foo        default_set_insert
#define LOCAL_tsc_foo                tsc_set_insert
#define LOCAL_tsc_foo_data           tsc_set_insert_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default         DeeSet_Default
#define LOCAL_DeeSeq_RequireFoo      SetInsert
#define LOCAL_DeeSeq_AttrFoo         Insert
#elif defined(DEFINE_DeeType_RequireSetRemove)
#define LOCAL_CANONICAL_NAME         remove
#define LOCAL_default_seq_foo        default_set_remove
#define LOCAL_tsc_foo                tsc_set_remove
#define LOCAL_tsc_foo_data           tsc_set_remove_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default         DeeSet_Default
#define LOCAL_DeeSeq_RequireFoo      SetRemove
#define LOCAL_DeeSeq_AttrFoo         Remove
#elif defined(DEFINE_DeeType_RequireSetUnify)
#define LOCAL_CANONICAL_NAME         unify
#define LOCAL_default_seq_foo        default_set_unify
#define LOCAL_tsc_foo                tsc_set_unify
#define LOCAL_tsc_foo_data           tsc_set_unify_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default         DeeSet_Default
#define LOCAL_DeeSeq_RequireFoo      SetUnify
#define LOCAL_DeeSeq_AttrFoo         Unify
#elif defined(DEFINE_DeeType_RequireSetInsertAll)
#define LOCAL_CANONICAL_NAME         insertall
#define LOCAL_default_seq_foo        default_set_insertall
#define LOCAL_tsc_foo                tsc_set_insertall
#define LOCAL_tsc_foo_data           tsc_set_insertall_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default         DeeSet_Default
#define LOCAL_DeeSeq_RequireFoo      SetInsertAll
#define LOCAL_DeeSeq_AttrFoo         InsertAll
#elif defined(DEFINE_DeeType_RequireSetRemoveAll)
#define LOCAL_CANONICAL_NAME         removeall
#define LOCAL_default_seq_foo        default_set_removeall
#define LOCAL_tsc_foo                tsc_set_removeall
#define LOCAL_tsc_foo_data           tsc_set_removeall_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default         DeeSet_Default
#define LOCAL_DeeSeq_RequireFoo      SetRemoveAll
#define LOCAL_DeeSeq_AttrFoo         RemoveAll
#elif defined(DEFINE_DeeType_RequireSetPop)
#define LOCAL_CANONICAL_NAME         pop
#define LOCAL_default_seq_foo        default_set_pop
#define LOCAL_tsc_foo                tsc_set_pop
#define LOCAL_tsc_foo_data           tsc_set_pop_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default         DeeSet_Default
#define LOCAL_DeeSeq_RequireFoo      SetPop
#define LOCAL_DeeSeq_AttrFoo         Pop
#elif defined(DEFINE_DeeType_RequireSetPopWithDefault)
#define LOCAL_CANONICAL_NAME         pop
#define LOCAL_default_seq_foo        default_set_pop
#define LOCAL_tsc_foo                tsc_set_pop_with_default
#define LOCAL_tsc_foo_data           tsc_set_pop_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default         DeeSet_Default
#define LOCAL_DeeSeq_RequireFoo      SetPopWithDefault
#define LOCAL_DeeSeq_AttrFoo         PopWithDefault
#define LOCAL_DeeSeq_AttrBar         Pop
#elif defined(DEFINE_DeeType_RequireMapSetOld)
#define LOCAL_CANONICAL_NAME         setold
#define LOCAL_default_seq_foo        default_map_setold
#define LOCAL_tsc_foo                tsc_map_setold
#define LOCAL_tsc_foo_data           tsc_map_setold_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapSetOld
#define LOCAL_DeeSeq_AttrFoo         SetOld
#elif defined(DEFINE_DeeType_RequireMapSetOldEx)
#define LOCAL_CANONICAL_NAME         setold_ex
#define LOCAL_default_seq_foo        default_map_setold_ex
#define LOCAL_tsc_foo                tsc_map_setold_ex
#define LOCAL_tsc_foo_data           tsc_map_setold_ex_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapSetOldEx
#define LOCAL_DeeSeq_AttrFoo         SetOldEx
#elif defined(DEFINE_DeeType_RequireMapSetNew)
#define LOCAL_CANONICAL_NAME         setnew
#define LOCAL_default_seq_foo        default_map_setnew
#define LOCAL_tsc_foo                tsc_map_setnew
#define LOCAL_tsc_foo_data           tsc_map_setnew_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapSetNew
#define LOCAL_DeeSeq_AttrFoo         SetNew
#elif defined(DEFINE_DeeType_RequireMapSetNewEx)
#define LOCAL_CANONICAL_NAME         setnew_ex
#define LOCAL_default_seq_foo        default_map_setnew_ex
#define LOCAL_tsc_foo                tsc_map_setnew_ex
#define LOCAL_tsc_foo_data           tsc_map_setnew_ex_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapSetNewEx
#define LOCAL_DeeSeq_AttrFoo         SetNewEx
#elif defined(DEFINE_DeeType_RequireMapSetDefault)
#define LOCAL_CANONICAL_NAME         setdefault
#define LOCAL_default_seq_foo        default_map_setdefault
#define LOCAL_tsc_foo                tsc_map_setdefault
#define LOCAL_tsc_foo_data           tsc_map_setdefault_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapSetDefault
#define LOCAL_DeeSeq_AttrFoo         SetDefault
#elif defined(DEFINE_DeeType_RequireMapUpdate)
#define LOCAL_CANONICAL_NAME         update
#define LOCAL_default_seq_foo        default_map_update
#define LOCAL_tsc_foo                tsc_map_update
#define LOCAL_tsc_foo_data           tsc_map_update_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapUpdate
#define LOCAL_DeeSeq_AttrFoo         Update
#elif defined(DEFINE_DeeType_RequireMapRemove)
#define LOCAL_CANONICAL_NAME         remove
#define LOCAL_default_seq_foo        default_map_remove
#define LOCAL_tsc_foo                tsc_map_remove
#define LOCAL_tsc_foo_data           tsc_map_remove_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapRemove
#define LOCAL_DeeSeq_AttrFoo         Remove
#elif defined(DEFINE_DeeType_RequireMapRemoveKeys)
#define LOCAL_CANONICAL_NAME         removekeys
#define LOCAL_default_seq_foo        default_map_removekeys
#define LOCAL_tsc_foo                tsc_map_removekeys
#define LOCAL_tsc_foo_data           tsc_map_removekeys_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapRemoveKeys
#define LOCAL_DeeSeq_AttrFoo         RemoveKeys
#elif defined(DEFINE_DeeType_RequireMapPop)
#define LOCAL_CANONICAL_NAME         pop
#define LOCAL_default_seq_foo        default_map_pop
#define LOCAL_tsc_foo                tsc_map_pop
#define LOCAL_tsc_foo_data           tsc_map_pop_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapPop
#define LOCAL_DeeSeq_AttrFoo         Pop
#elif defined(DEFINE_DeeType_RequireMapPopWithDefault)
#define LOCAL_CANONICAL_NAME         pop
#define LOCAL_default_seq_foo        default_map_pop
#define LOCAL_tsc_foo                tsc_map_pop_with_default
#define LOCAL_tsc_foo_data           tsc_map_pop_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapPopWithDefault
#define LOCAL_DeeSeq_AttrFoo         PopWithDefault
#define LOCAL_DeeSeq_AttrBar         Pop
#elif defined(DEFINE_DeeType_RequireMapPopItem)
#define LOCAL_CANONICAL_NAME         popitem
#define LOCAL_default_seq_foo        default_map_popitem
#define LOCAL_tsc_foo                tsc_map_popitem
#define LOCAL_tsc_foo_data           tsc_map_popitem_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapPopItem
#define LOCAL_DeeSeq_AttrFoo         PopItem
#elif defined(DEFINE_DeeType_RequireMapKeys)
#define LOCAL_CANONICAL_NAME         keys
#define LOCAL_default_seq_foo        default_map_keys
#define LOCAL_tsc_foo                tsc_map_keys
#define LOCAL_tsc_foo_data           tsc_map_keys_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapKeys
#define LOCAL_DeeSeq_AttrFoo         Keys
#define LOCAL_IS_GETSET_GET
#elif defined(DEFINE_DeeType_RequireMapValues)
#define LOCAL_CANONICAL_NAME         values
#define LOCAL_default_seq_foo        default_map_values
#define LOCAL_tsc_foo                tsc_map_values
#define LOCAL_tsc_foo_data           tsc_map_values_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapValues
#define LOCAL_DeeSeq_AttrFoo         Values
#define LOCAL_IS_GETSET_GET
#elif defined(DEFINE_DeeType_RequireMapIterKeys)
#define LOCAL_CANONICAL_NAME         iterkeys
#define LOCAL_default_seq_foo        default_map_iterkeys
#define LOCAL_tsc_foo                tsc_map_iterkeys
#define LOCAL_tsc_foo_data           tsc_map_iterkeys_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapIterKeys
#define LOCAL_DeeSeq_AttrFoo         IterKeys
#define LOCAL_IS_GETSET_GET
#elif defined(DEFINE_DeeType_RequireMapIterValues)
#define LOCAL_CANONICAL_NAME         itervalues
#define LOCAL_default_seq_foo        default_map_itervalues
#define LOCAL_tsc_foo                tsc_map_itervalues
#define LOCAL_tsc_foo_data           tsc_map_itervalues_data
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default         DeeMap_Default
#define LOCAL_DeeSeq_RequireFoo      MapIterValues
#define LOCAL_DeeSeq_AttrFoo         IterValues
#define LOCAL_IS_GETSET_GET
#else /* DEFINE_DeeType_RequireSeq... */
#error "Invalid configuration"
#endif /* !DEFINE_DeeType_RequireSeq... */

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
#define LOCAL_DeeSeq_RequireFoo PP_CAT2(Seq, LOCAL_DeeSeq_AttrFoo)
#endif /* !LOCAL_DeeSeq_RequireFoo */

#ifndef LOCAL_DeeType_RequireSeqFoo
#define LOCAL_DeeType_RequireSeqFoo_private_uncached PP_CAT3(DeeType_Require, LOCAL_DeeSeq_RequireFoo, _private_uncached)
#define LOCAL_DeeType_RequireSeqFoo_uncached         PP_CAT3(DeeType_Require, LOCAL_DeeSeq_RequireFoo, _uncached)
#define LOCAL_DeeType_RequireSeqFoo                  PP_CAT2(DeeType_Require, LOCAL_DeeSeq_RequireFoo)
#else /* !LOCAL_DeeType_RequireSeqFoo */
#ifndef LOCAL_DeeType_RequireSeqFoo_private_uncached
#define LOCAL_DeeType_RequireSeqFoo_private_uncached PP_CAT2(LOCAL_DeeType_RequireSeqFoo, _private_uncached)
#endif /* !LOCAL_DeeType_RequireSeqFoo_private_uncached */
#ifndef LOCAL_DeeType_RequireSeqFoo_uncached
#define LOCAL_DeeType_RequireSeqFoo_uncached PP_CAT2(LOCAL_DeeType_RequireSeqFoo, _uncached)
#endif /* !LOCAL_DeeType_RequireSeqFoo_uncached */
#endif /* LOCAL_DeeType_RequireSeqFoo */

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
#ifndef LOCAL_DeeSeq_DefaultFooForEmpty
#define LOCAL_DeeSeq_DefaultFooForEmpty LOCAL_DeeSeq_DefaultFooWithError
#endif /* !LOCAL_DeeSeq_DefaultFooForEmpty */


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


#if (defined(LOCAL_IS_GETSET_GET) || defined(LOCAL_IS_GETSET_BOUND) || \
     defined(LOCAL_IS_GETSET_DEL) || defined(LOCAL_IS_GETSET_SET))
#define LOCAL_IS_GETSET
#endif /* LOCAL_IS_GETSET_* */

/* Mutable sequence functions */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) LOCAL_Dee_tsc_foo_t DCALL
LOCAL_DeeType_RequireSeqFoo_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self) {
	int seqclass = DeeType_GetSeqClass(self);
#ifndef LOCAL_IS_OPERATOR
	struct Dee_attrinfo attrinfo;
#endif /* !LOCAL_IS_OPERATOR */
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
#ifndef LOCAL_IS_OPERATOR
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
#ifdef LOCAL_IS_GETSET_GET
					if (attrinfo.ai_value.v_getset->gs_get == &LOCAL_default_seq_foo)
						return &LOCAL_DeeSeq_DefaultFooForEmpty;
					return attrinfo.ai_value.v_getset->gs_get;
#elif defined(LOCAL_IS_GETSET_BOUND)
					if (attrinfo.ai_value.v_getset->gs_bound == &LOCAL_default_seq_foo)
						return &LOCAL_DeeSeq_DefaultFooForEmpty;
					return attrinfo.ai_value.v_getset->gs_bound;
#elif defined(LOCAL_IS_GETSET_DEL)
					if (attrinfo.ai_value.v_getset->gs_del == &LOCAL_default_seq_foo)
						return &LOCAL_DeeSeq_DefaultFooForEmpty;
					return attrinfo.ai_value.v_getset->gs_del;
#elif defined(LOCAL_IS_GETSET_SET)
					if (attrinfo.ai_value.v_getset->gs_set == &LOCAL_default_seq_foo)
						return &LOCAL_DeeSeq_DefaultFooForEmpty;
					return attrinfo.ai_value.v_getset->gs_set;
#endif /* ... */
				case Dee_ATTRINFO_ATTR:
#if defined(LOCAL_IS_GETSET_GET) || defined(LOCAL_IS_GETSET_BOUND)
					if ((attrinfo.ai_value.v_attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FGETSET | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) ==
					    /*                                */ (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FGETSET | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM))
#else /* LOCAL_IS_GETSET_GET || LOCAL_IS_GETSET_BOUND */
					if ((attrinfo.ai_value.v_attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FGETSET | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM | Dee_CLASS_ATTRIBUTE_FREADONLY)) ==
					    /*                                */ (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FGETSET | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM))
#endif /* 1LOCAL_IS_GETSET_GET && !LOCAL_IS_GETSET_BOUND */
					{
						struct class_desc *desc = DeeClass_DESC(self);
						uint16_t id = attrinfo.ai_value.v_attr->ca_addr;
						DREF DeeObject *callback;
						Dee_class_desc_lock_read(desc);
#if defined(LOCAL_IS_GETSET_GET) || defined(LOCAL_IS_GETSET_BOUND)
						callback = desc->cd_members[id + Dee_CLASS_GETSET_GET];
#elif defined(LOCAL_IS_GETSET_DEL)
						callback = desc->cd_members[id + Dee_CLASS_GETSET_DEL];
#elif defined(LOCAL_IS_GETSET_SET)
						callback = desc->cd_members[id + Dee_CLASS_GETSET_SET];
#endif /* ... */
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
						return &LOCAL_DeeSeq_DefaultFooForEmpty;
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
#endif /* !LOCAL_IS_OPERATOR */

#ifdef DEFINE_DeeType_RequireSeqTryGetFirst
	{
		Dee_tsc_getfirst_t tsc_getfirst;
		tsc_getfirst = DeeType_RequireSeqGetFirst_private_uncached(orig_type, self);
		if (tsc_getfirst == &DeeSeq_DefaultGetFirstWithGetItemIndex) {
			if (!DeeType_IsDefaultTryGetItemIndex(self->tp_seq->tp_trygetitem_index))
				return &DeeSeq_DefaultTryGetFirstWithTryGetItemIndex;
			if (DeeType_HasOperator(self, OPERATOR_SIZE)) {
				if (self->tp_seq->tp_getitem_index_fast)
					return &DeeSeq_DefaultTryGetFirstWithSizeAndGetItemIndexFast;
				return &DeeSeq_DefaultTryGetFirstWithSizeAndGetItemIndex;
			}
			return &DeeSeq_DefaultTryGetFirstWithTryGetItemIndex;
		} else if (tsc_getfirst == &DeeSeq_DefaultGetFirstWithGetItem) {
			if (!DeeType_IsDefaultTryGetItem(self->tp_seq->tp_trygetitem))
				return &DeeSeq_DefaultTryGetFirstWithTryGetItem;
			if (DeeType_HasOperator(self, OPERATOR_SIZE))
				return &DeeSeq_DefaultTryGetFirstWithSizeAndGetItem;
			return &DeeSeq_DefaultTryGetFirstWithTryGetItem;
		} else if (tsc_getfirst == &DeeSeq_DefaultGetFirstWithForeach) {
			return &DeeSeq_DefaultTryGetFirstWithForeach;
		} else if (tsc_getfirst == &DeeSeq_DefaultGetFirstWithError) {
			return &DeeSeq_DefaultTryGetFirstWithError;
		} else if (tsc_getfirst != NULL) {
			return &DeeSeq_DefaultTryGetFirstWithSeqGetFirst;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqGetFirst)
	if (seqclass == Dee_SEQCLASS_SEQ) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_GETITEM)) {
			if (!DeeType_IsDefaultTryGetItemIndex(self->tp_seq->tp_trygetitem_index))
				return &DeeSeq_DefaultGetFirstWithGetItemIndex;
			return &DeeSeq_DefaultGetFirstWithGetItem;
		}
	}
	if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_ITER))
		return &DeeSeq_DefaultGetFirstWithForeach;
#elif defined(DEFINE_DeeType_RequireSeqBoundFirst)
	if (seqclass == Dee_SEQCLASS_SEQ) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_GETITEM)) {
			if (!DeeType_IsDefaultBoundItemIndex(self->tp_seq->tp_bounditem_index))
				return &DeeSeq_DefaultBoundFirstWithBoundItemIndex;
			return &DeeSeq_DefaultBoundFirstWithBoundItem;
		}
	}
	if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_ITER))
		return &DeeSeq_DefaultBoundFirstWithForeach;
#elif defined(DEFINE_DeeType_RequireSeqDelFirst)
	if (seqclass == Dee_SEQCLASS_SEQ) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM)) {
			if (!DeeType_IsDefaultDelItemIndex(self->tp_seq->tp_delitem_index))
				return &DeeSeq_DefaultDelFirstWithDelItemIndex;
			return &DeeSeq_DefaultDelFirstWithDelItem;
		}
	} else if (seqclass == Dee_SEQCLASS_SET) {
		Dee_tsc_set_remove_t tsc_set_remove;
		tsc_set_remove = DeeType_RequireSetRemove_private_uncached(orig_type, self);
		if (tsc_set_remove != NULL &&
		    tsc_set_remove != &DeeSet_DefaultRemoveWithError) {
			Dee_tsc_trygetfirst_t tsc_trygetfirst;
			tsc_trygetfirst = DeeType_RequireSeqTryGetFirst_private_uncached(orig_type, self);
			if (tsc_trygetfirst != NULL &&
			    tsc_trygetfirst != &DeeSeq_DefaultTryGetFirstWithError)
				return &DeeSeq_DefaultDelFirstWithSeqGetFirstAndSetRemove;
		}
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM)) {
			Dee_tsc_trygetfirst_t tsc_trygetfirst;
			tsc_trygetfirst = DeeType_RequireSeqTryGetFirst_private_uncached(orig_type, self);
			if (tsc_trygetfirst != NULL &&
			    tsc_trygetfirst != &DeeSeq_DefaultTryGetFirstWithError)
				return &DeeSeq_DefaultDelFirstWithSeqGetFirstAndMaplikeDelItem;
		}
	}
#elif defined(DEFINE_DeeType_RequireSetFirst)
	if (seqclass == Dee_SEQCLASS_SEQ) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETITEM)) {
			if (!DeeType_IsDefaultSetItemIndex(self->tp_seq->tp_setitem_index))
				return &DeeSeq_DefaultSetFirstWithSetItemIndex;
			return &DeeSeq_DefaultSetFirstWithSetItem;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqTryGetLast)
	{
		Dee_tsc_getlast_t tsc_getlast;
		tsc_getlast = DeeType_RequireSeqGetLast_private_uncached(orig_type, self);
		if (tsc_getlast == &DeeSeq_DefaultGetLastWithSizeAndGetItemIndexFast) {
			return &DeeSeq_DefaultTryGetLastWithSizeAndGetItemIndexFast;
		} else if (tsc_getlast == &DeeSeq_DefaultGetLastWithSizeAndGetItemIndex) {
			return &DeeSeq_DefaultTryGetLastWithSizeAndGetItemIndex;
		} else if (tsc_getlast == &DeeSeq_DefaultGetLastWithSizeAndGetItem) {
			return &DeeSeq_DefaultTryGetLastWithSizeAndGetItem;
		} else if (tsc_getlast == &DeeSeq_DefaultGetLastWithForeach) {
			return &DeeSeq_DefaultTryGetLastWithForeach;
		} else if (tsc_getlast == &DeeSeq_DefaultGetLastWithError) {
			return &DeeSeq_DefaultTryGetLastWithError;
		} else if (tsc_getlast != NULL) {
			return &DeeSeq_DefaultTryGetLastWithSeqGetLast;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqGetLast)
	if (seqclass == Dee_SEQCLASS_SEQ) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_GETITEM)) {
			if (DeeType_HasOperator(self, OPERATOR_SIZE)) {
				if (self->tp_seq->tp_getitem_index_fast)
					return &DeeSeq_DefaultGetLastWithSizeAndGetItemIndexFast;
				if (!DeeType_IsDefaultGetItemIndex(self->tp_seq->tp_getitem_index))
					return &DeeSeq_DefaultGetLastWithSizeAndGetItemIndex;
				return &DeeSeq_DefaultGetLastWithSizeAndGetItem;
			}
		}
	}
	if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_ITER))
		return &DeeSeq_DefaultGetLastWithForeach;
#elif defined(DEFINE_DeeType_RequireSeqBoundLast)
	if (seqclass == Dee_SEQCLASS_SEQ) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_GETITEM)) {
			if (DeeType_HasOperator(self, OPERATOR_SIZE)) {
				if (!DeeType_IsDefaultBoundItemIndex(self->tp_seq->tp_bounditem_index))
					return &DeeSeq_DefaultBoundLastWithSizeAndBoundItemIndex;
				return &DeeSeq_DefaultBoundLastWithSizeAndBoundItem;
			}
		}
	}
	if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_ITER))
		return &DeeSeq_DefaultBoundLastWithForeach;
#elif defined(DEFINE_DeeType_RequireSeqDelLast)
	if (seqclass == Dee_SEQCLASS_SEQ) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM)) {
			if (DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
				if (!DeeType_IsDefaultDelItemIndex(self->tp_seq->tp_delitem_index))
					return &DeeSeq_DefaultDelLastWithSizeAndDelItemIndex;
				return &DeeSeq_DefaultDelLastWithSizeAndDelItem;
			}
		}
	} else if (seqclass == Dee_SEQCLASS_SET) {
		Dee_tsc_set_remove_t tsc_set_remove;
		tsc_set_remove = DeeType_RequireSetRemove_private_uncached(orig_type, self);
		if (tsc_set_remove != NULL &&
		    tsc_set_remove != &DeeSet_DefaultRemoveWithError) {
			Dee_tsc_trygetlast_t tsc_trygetlast;
			tsc_trygetlast = DeeType_RequireSeqTryGetLast_private_uncached(orig_type, self);
			if (tsc_trygetlast != NULL &&
			    tsc_trygetlast != &DeeSeq_DefaultTryGetLastWithError)
				return &DeeSeq_DefaultDelLastWithSeqGetLastAndSetRemove;
		}
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM)) {
			Dee_tsc_trygetlast_t tsc_trygetlast;
			tsc_trygetlast = DeeType_RequireSeqTryGetLast_private_uncached(orig_type, self);
			if (tsc_trygetlast != NULL &&
			    tsc_trygetlast != &DeeSeq_DefaultTryGetLastWithError)
				return &DeeSeq_DefaultDelLastWithSeqGetLastAndMaplikeDelItem;
		}
	}
#elif defined(DEFINE_DeeType_RequireSetLast)
	if (seqclass == Dee_SEQCLASS_SEQ) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETITEM)) {
			if (DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
				if (!DeeType_IsDefaultSetItemIndex(self->tp_seq->tp_setitem_index))
					return &DeeSeq_DefaultSetLastWithSizeAndSetItemIndex;
				return &DeeSeq_DefaultSetLastWithSizeAndSetItem;
			}
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqAny)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqAnyWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqAnyWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqAnyWithRangeAndKey)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqAll)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqAllWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqAllWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqAllWithRangeAndKey)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqParity)
	/* TODO: Parity with count */
#elif defined(DEFINE_DeeType_RequireSeqParityWithKey)
	/* TODO: Parity with count */
#elif defined(DEFINE_DeeType_RequireSeqParityWithRange)
	/* TODO: Parity with count */
#elif defined(DEFINE_DeeType_RequireSeqParityWithRangeAndKey)
	/* TODO: Parity with count */
#elif defined(DEFINE_DeeType_RequireSeqReduce)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqReduceWithInit)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqReduceWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqReduceWithRangeAndInit)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqMin)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqMinWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqMinWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqMinWithRangeAndKey)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqMax)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqMaxWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqMaxWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqMaxWithRangeAndKey)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqsum)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqsumWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqCount)
	/* ... */
	/* TODO: Count with Find (repeated) */
#elif defined(DEFINE_DeeType_RequireSeqCountWithKey)
	/* ... */
	/* TODO: Count with Find (repeated) */
#elif defined(DEFINE_DeeType_RequireSeqCountWithRange)
	/* ... */
	/* TODO: Count with Find (repeated) */
#elif defined(DEFINE_DeeType_RequireSeqCountWithRangeAndKey)
	/* ... */
	/* TODO: Count with Find (repeated) */
#elif defined(DEFINE_DeeType_RequireSeqContains)
	if (seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_CONTAINS))
			return &DeeSeq_DefaultContainsWithContains;
	}
	/* TODO: Contains with Find */
	if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_ITER))
		return &DeeSeq_DefaultContainsWithForeach;
#elif defined(DEFINE_DeeType_RequireSeqContainsWithKey)
	/* TODO: Contains with Find */
#elif defined(DEFINE_DeeType_RequireSeqContainsWithRange)
	/* TODO: Contains with Find */
#elif defined(DEFINE_DeeType_RequireSeqContainsWithRangeAndKey)
	/* TODO: Contains with Find */
#elif defined(DEFINE_DeeType_RequireSeqLocate)
	/* TODO: Locate with Find */
#elif defined(DEFINE_DeeType_RequireSeqLocateWithKey)
	/* TODO: Locate with Find */
#elif defined(DEFINE_DeeType_RequireSeqLocateWithRange)
	/* TODO: Locate with Find */
#elif defined(DEFINE_DeeType_RequireSeqLocateWithRangeAndKey)
	/* TODO: Locate with Find */
#elif defined(DEFINE_DeeType_RequireSeqRLocateWithRange)
	if (DeeType_HasPrivateSeqEnumerateIndexReverse(orig_type, self))
		return &DeeSeq_DefaultRLocateWithRangeWithSeqEnumerateIndexReverse;
#elif defined(DEFINE_DeeType_RequireSeqRLocateWithRangeAndKey)
	if (DeeType_HasPrivateSeqEnumerateIndexReverse(orig_type, self))
		return &DeeSeq_DefaultRLocateWithRangeAndKeyWithSeqEnumerateIndexReverse;
#elif defined(DEFINE_DeeType_RequireSeqStartsWith)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqStartsWithWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqStartsWithWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqStartsWithWithRangeAndKey)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqEndsWith)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqEndsWithWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqEndsWithWithRange)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqEndsWithWithRangeAndKey)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqFind)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqFindWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqRFind)
	if (DeeType_HasPrivateSeqEnumerateIndexReverse(orig_type, self))
		return &DeeSeq_DefaultRFindWithSeqEnumerateIndexReverse;
#elif defined(DEFINE_DeeType_RequireSeqRFindWithKey)
	if (DeeType_HasPrivateSeqEnumerateIndexReverse(orig_type, self))
		return &DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndexReverse;
#elif defined(DEFINE_DeeType_RequireSeqErase)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELRANGE))
			return &DeeSeq_DefaultEraseWithDelRangeIndex;
		{
			Dee_tsc_pop_t tsc_pop;
			tsc_pop = DeeType_RequireSeqPop_private_uncached(orig_type, self);
			if (tsc_pop != NULL &&
			    tsc_pop != &DeeSeq_DefaultPopWithError)
				return &DeeSeq_DefaultEraseWithPop;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqInsert)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		Dee_tsc_insertall_t tsc_insertall;
		tsc_insertall = DeeType_RequireSeqInsertAll_private_uncached(orig_type, self);
		if (tsc_insertall != NULL &&
		    tsc_insertall != &DeeSeq_DefaultInsertAllWithError)
			return &DeeSeq_DefaultInsertWithSeqInsertAll;
	}
#elif defined(DEFINE_DeeType_RequireSeqInsertAll)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETRANGE) &&
		    self->tp_seq->tp_setrange_index != &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndTSCEraseAndTSCInsertAll)
			return &DeeSeq_DefaultInsertAllWithSetRangeIndex;
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_insert, 6, Dee_HashStr__insert, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_insert))
			return &DeeSeq_DefaultInsertAllWithSeqInsert;
	}
#elif defined(DEFINE_DeeType_RequireSeqPushFront)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqAppend)
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
							return &LOCAL_DeeSeq_DefaultFooForEmpty;
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
			tsc_extend = DeeType_RequireSeqExtend_private_uncached(orig_type, self);
			if (tsc_extend != NULL &&
			    tsc_extend != &DeeSeq_DefaultExtendWithError) {
				if (tsc_extend == &DeeSeq_DefaultExtendWithSizeAndSeqInsertAll)
					return &DeeSeq_DefaultAppendWithSizeAndSeqInsert;;
				return &DeeSeq_DefaultAppendWithSeqExtend;
			}
		}
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
			Dee_tsc_insert_t tsc_insert;
			tsc_insert = DeeType_RequireSeqInsert_private_uncached(orig_type, self);
			if (tsc_insert != NULL &&
			    tsc_insert != &DeeSeq_DefaultInsertWithError)
				return &DeeSeq_DefaultAppendWithSizeAndSeqInsert;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqExtend)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasOperator(orig_type, OPERATOR_ITER)) {
			if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_append, 6, Dee_HashStr__append, &attrinfo) &&
			    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_append))
				return &DeeSeq_DefaultExtendWithSeqAppend;
			if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_pushback, 8, Dee_HashStr__pushback, &attrinfo) &&
			    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_append))
				return &DeeSeq_DefaultExtendWithSeqAppend;
		}
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
			Dee_tsc_insertall_t tsc_insertall;
			tsc_insertall = DeeType_RequireSeqInsertAll_private_uncached(orig_type, self);
			if (tsc_insertall != NULL &&
			    tsc_insertall != &DeeSeq_DefaultInsertAllWithError)
				return &DeeSeq_DefaultExtendWithSizeAndSeqInsertAll;
		}
		if (DeeType_HasOperator(orig_type, OPERATOR_ITER)) {
			Dee_tsc_insert_t tsc_insert;
			tsc_insert = DeeType_RequireSeqInsert_private_uncached(orig_type, self);
			if (tsc_insert != NULL &&
				tsc_insert != &DeeSeq_DefaultInsertWithError)
				return &DeeSeq_DefaultExtendWithSeqAppend;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqXchItemIndex)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM))
			return &DeeSeq_DefaultXchItemIndexWithGetItemIndexAndSetItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqClear)
	switch (seqclass) {

	case Dee_SEQCLASS_SET:{
		Dee_tsc_set_removeall_t tsc_set_removeall;
		tsc_set_removeall = DeeType_RequireSetRemoveAll_private_uncached(orig_type, self);
		if (tsc_set_removeall != NULL &&
		    tsc_set_removeall != &DeeSet_DefaultRemoveAllWithError)
			return &DeeSeq_DefaultClearWithSeqRemoveAll;
	}	break;

	case Dee_SEQCLASS_MAP: {
		Dee_tsc_map_removekeys_t tsc_map_removekeys;
		tsc_map_removekeys = DeeType_RequireMapRemoveKeys_private_uncached(orig_type, self);
		if (tsc_map_removekeys != NULL &&
		    tsc_map_removekeys != &DeeMap_DefaultRemoveKeysWithError)
			return &DeeSeq_DefaultClearWithMapRemoveKeys;
	}	break;

	default:
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELRANGE)) {
			if (self->tp_seq->tp_delrange_index_n == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNone ||
			    self->tp_seq->tp_delrange_index_n == &DeeSeq_DefaultDelRangeIndexNWithSetRangeIndexNNoneDefault)
				return &DeeSeq_DefaultClearWithSetRangeIndexN;
			return &DeeSeq_DefaultClearWithDelRangeIndexN;
		}
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETRANGE))
			return &DeeSeq_DefaultClearWithSetRangeIndexN;
		{
			Dee_tsc_erase_t tsc_erase;
			tsc_erase = DeeType_RequireSeqErase_private_uncached(orig_type, self);
			if (tsc_erase != NULL &&
			    tsc_erase != &DeeSeq_DefaultEraseWithError)
				return &DeeSeq_DefaultClearWithSeqErase;
		}
		break;
	}
#elif defined(DEFINE_DeeType_RequireSeqPop)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_erase, 5, Dee_HashStr__erase, &attrinfo) &&
			    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_erase))
				return &DeeSeq_DefaultPopWithSizeAndGetItemIndexAndSeqErase;
		}
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultPopWithSizeAndGetItemIndexAndDelItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqRemove)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		{
			Dee_tsc_removeall_t tsc_removeall;
			tsc_removeall = DeeType_RequireSeqRemoveAll_private_uncached(orig_type, self);
			if (tsc_removeall != NULL &&
			    tsc_removeall != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveWithSeqRemoveAll;
		}
		{
			Dee_tsc_removeif_t tsc_removeif;
			tsc_removeif = DeeType_RequireSeqRemoveIf_private_uncached(orig_type, self);
			if (tsc_removeif != NULL &&
			    tsc_removeif != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveWithSeqRemoveIf;
		}
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM)) {
			Dee_tsc_find_t tsc_find;
			tsc_find = DeeType_RequireSeqFind_private_uncached(orig_type, self);
			if (tsc_find == &DeeSeq_DefaultFindWithSeqEnumerateIndex)
				return &DeeSeq_DefaultRemoveWithSeqEnumerateIndexAndDelItemIndex;
			if (tsc_find)
				return &DeeSeq_DefaultRemoveWithSeqFindAndDelItemIndex;
			if (DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
				return &DeeSeq_DefaultRemoveWithSeqEnumerateIndexAndDelItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqRemoveWithKey)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		{
			Dee_tsc_removeall_with_key_t tsc_removeall_with_key;
			tsc_removeall_with_key = DeeType_RequireSeqRemoveAllWithKey_private_uncached(orig_type, self);
			if (tsc_removeall_with_key != NULL &&
			    tsc_removeall_with_key != &DeeSeq_DefaultRemoveAllWithKeyWithError)
				return &DeeSeq_DefaultRemoveWithKeyWithSeqRemoveAllWithKey;
		}
		{
			Dee_tsc_removeif_t tsc_removeif;
			tsc_removeif = DeeType_RequireSeqRemoveIf_private_uncached(orig_type, self);
			if (tsc_removeif != NULL &&
			    tsc_removeif != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveWithKeyWithSeqRemoveIf;
		}
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM)) {
			Dee_tsc_find_with_key_t tsc_find_with_key;
			tsc_find_with_key = DeeType_RequireSeqFindWithKey_private_uncached(orig_type, self);
			if (tsc_find_with_key == &DeeSeq_DefaultFindWithKeyWithSeqEnumerateIndex)
				return &DeeSeq_DefaultRemoveWithKeyWithSeqEnumerateIndexAndDelItemIndex;
			if (tsc_find_with_key)
				return &DeeSeq_DefaultRemoveWithKeyWithSeqFindWithKeyAndDelItemIndex;
			if (DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
				return &DeeSeq_DefaultRemoveWithKeyWithSeqEnumerateIndexAndDelItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqRRemove)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM)) {
			Dee_tsc_rfind_t tsc_rfind;
			tsc_rfind = DeeType_RequireSeqRFind_private_uncached(orig_type, self);
			if (tsc_rfind == &DeeSeq_DefaultRFindWithSeqEnumerateIndexReverse)
				return &DeeSeq_DefaultRRemoveWithSeqEnumerateIndexReverseAndDelItemIndex;
			if (tsc_rfind == &DeeSeq_DefaultRFindWithSeqEnumerateIndex)
				return &DeeSeq_DefaultRRemoveWithSeqEnumerateIndexAndDelItemIndex;
			if (tsc_rfind)
				return &DeeSeq_DefaultRRemoveWithTSeqFindAndDelItemIndex;
			if (DeeType_TryRequireSeqEnumerateIndexReverse(orig_type))
				return &DeeSeq_DefaultRRemoveWithSeqEnumerateIndexReverseAndDelItemIndex;
			if (DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
				return &DeeSeq_DefaultRRemoveWithSeqEnumerateIndexAndDelItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqRRemoveWithKey)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM)) {
			Dee_tsc_rfind_with_key_t tsc_rfind_with_key;
			tsc_rfind_with_key = DeeType_RequireSeqRFindWithKey_private_uncached(orig_type, self);
			if (tsc_rfind_with_key == &DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndexReverse)
				return &DeeSeq_DefaultRRemoveWithKeyWithSeqEnumerateIndexReverseAndDelItemIndex;
			if (tsc_rfind_with_key == &DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndex)
				return &DeeSeq_DefaultRRemoveWithKeyWithSeqEnumerateIndexAndDelItemIndex;
			if (tsc_rfind_with_key)
				return &DeeSeq_DefaultRRemoveWithKeyWithSeqRFindWithKeyAndDelItemIndex;
			if (DeeType_TryRequireSeqEnumerateIndexReverse(orig_type))
				return &DeeSeq_DefaultRRemoveWithKeyWithSeqEnumerateIndexReverseAndDelItemIndex;
			if (DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
				return &DeeSeq_DefaultRRemoveWithKeyWithSeqEnumerateIndexAndDelItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqRemoveAll)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		{
			Dee_tsc_removeif_t tsc_removeif;
			tsc_removeif = DeeType_RequireSeqRemoveIf_private_uncached(orig_type, self);
			if (tsc_removeif != NULL &&
			    tsc_removeif != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveAllWithSeqRemoveIf;
		}
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_remove, 6, Dee_HashStr__remove, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_remove))
			return &DeeSeq_DefaultRemoveAllWithSeqRemove;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultRemoveAllWithSizeAndGetItemIndexAndDelItemIndex;
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
			return &DeeSeq_DefaultRemoveAllWithSeqRemove;
	}
#elif defined(DEFINE_DeeType_RequireSeqRemoveAllWithKey)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		{
			Dee_tsc_removeif_t tsc_removeif;
			tsc_removeif = DeeType_RequireSeqRemoveIf_private_uncached(orig_type, self);
			if (tsc_removeif != NULL &&
			    tsc_removeif != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveAllWithKeyWithSeqRemoveIf;
		}
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_remove, 6, Dee_HashStr__remove, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_remove))
			return &DeeSeq_DefaultRemoveAllWithKeyWithSeqRemoveWithKey;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultRemoveAllWithKeyWithSizeAndGetItemIndexAndDelItemIndex;
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
			return &DeeSeq_DefaultRemoveAllWithKeyWithSeqRemoveWithKey;
	}
#elif defined(DEFINE_DeeType_RequireSeqRemoveIf)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO */
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* TODO */
	} else {
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_removeall, 9, Dee_HashStr__removeall, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_removeall))
			return &DeeSeq_DefaultRemoveIfWithSeqRemoveAllWithKey;
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_remove, 6, Dee_HashStr__remove, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_seq_remove))
			return &DeeSeq_DefaultRemoveIfWithSeqRemoveAllWithKey;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultRemoveIfWithSizeAndGetItemIndexAndDelItemIndex;
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_ITER) && orig_type->tp_seq->tp_enumerate_index)
			return &DeeSeq_DefaultRemoveIfWithSeqRemoveAllWithKey;
	}
#elif defined(DEFINE_DeeType_RequireSeqResize)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETRANGE) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
			if (DeeType_HasOperator(orig_type, OPERATOR_DELRANGE) &&
			    orig_type->tp_seq->tp_delrange_index != &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNone &&
			    orig_type->tp_seq->tp_delrange_index != &DeeSeq_DefaultDelRangeIndexWithSetRangeIndexNoneDefault)
				return &DeeSeq_DefaultResizeWithSizeAndSetRangeIndexAndDelRangeIndex;
			return &DeeSeq_DefaultResizeWithSizeAndSetRangeIndex;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqFill)
	if (seqclass == Dee_SEQCLASS_SEQ) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETRANGE) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultFillWithSizeAndSetRangeIndex;
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_ITER))
			return &DeeSeq_DefaultFillWithEnumerateIndexAndSetItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqReverse)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETRANGE))
			return &DeeSeq_DefaultReverseWithSeqReversedAndSetRangeIndex;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
			if (DeeType_HasOperator(orig_type, OPERATOR_DELITEM))
				return &DeeSeq_DefaultReverseWithSizeAndGetItemIndexAndSetItemIndexAndDelItemIndex;
			return &DeeSeq_DefaultReverseWithSizeAndGetItemIndexAndSetItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqReversed)
	if (seqclass == Dee_SEQCLASS_SEQ &&
	    DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_GETITEM) &&
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
	if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_ITER))
		return &DeeSeq_DefaultReversedWithCopyForeachDefault; /* non-Default would also be OK */
#elif defined(DEFINE_DeeType_RequireSeqSort)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETRANGE))
			return &DeeSeq_DefaultSortWithSeqSortedAndSetRangeIndex;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultSortWithSizeAndGetItemIndexAndSetItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqSortWithKey)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETRANGE))
			return &DeeSeq_DefaultSortWithKeyWithSeqSortedAndSetRangeIndex;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultSortWithKeyWithSizeAndGetItemIndexAndSetItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqSorted)
	if (DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
		if (orig_type->tp_seq->tp_getitem_index_fast)
			return &DeeSeq_DefaultSortedWithCopySizeAndGetItemIndexFast;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_GETITEM))
			return &DeeSeq_DefaultSortedWithCopySizeAndTryGetItemIndex;
	}
	if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_ITER))
		return &DeeSeq_DefaultSortedWithCopyForeachDefault; /* non-Default would also be OK */
#elif defined(DEFINE_DeeType_RequireSeqSortedWithKey)
	if (DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
		if (orig_type->tp_seq->tp_getitem_index_fast)
			return &DeeSeq_DefaultSortedWithKeyWithCopySizeAndGetItemIndexFast;
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_GETITEM))
			return &DeeSeq_DefaultSortedWithKeyWithCopySizeAndTryGetItemIndex;
	}
	if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_ITER))
		return &DeeSeq_DefaultSortedWithKeyWithCopyForeachDefault; /* non-Default would also be OK */
#elif defined(DEFINE_DeeType_RequireSeqBFind)
	{
		Dee_tsc_brange_t tsc_brange;
		tsc_brange = DeeType_RequireSeqBRange_private_uncached(orig_type, self);
		if (tsc_brange != NULL &&
		    tsc_brange != &DeeSeq_DefaultBRangeWithError) {
			if (tsc_brange == &DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBFindWithSeqBRange;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqBFindWithKey)
	{
		Dee_tsc_brange_with_key_t tsc_brange_with_key;
		tsc_brange_with_key = DeeType_RequireSeqBRangeWithKey_private_uncached(orig_type, self);
		if (tsc_brange_with_key != NULL &&
		    tsc_brange_with_key != &DeeSeq_DefaultBRangeWithKeyWithError) {
			if (tsc_brange_with_key == &DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBFindWithKeyWithSeqBRangeWithKey;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqBPosition)
	{
		Dee_tsc_brange_t tsc_brange;
		tsc_brange = DeeType_RequireSeqBRange_private_uncached(orig_type, self);
		if (tsc_brange != NULL &&
		    tsc_brange != &DeeSeq_DefaultBRangeWithError) {
			if (tsc_brange == &DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBPositionWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBPositionWithSeqBRange;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqBPositionWithKey)
	{
		Dee_tsc_brange_with_key_t tsc_brange_with_key;
		tsc_brange_with_key = DeeType_RequireSeqBRangeWithKey_private_uncached(orig_type, self);
		if (tsc_brange_with_key != NULL &&
		    tsc_brange_with_key != &DeeSeq_DefaultBRangeWithKeyWithError) {
			if (tsc_brange_with_key == &DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBPositionWithKeyWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBPositionWithKeyWithSeqBRangeWithKey;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqBRange)
	{
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_GETITEM) && DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqBRangeWithKey)
	{
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_GETITEM) && DeeType_HasOperator(orig_type, OPERATOR_SIZE))
			return &DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqBLocate)
	{
		Dee_tsc_bfind_t tsc_bfind;
		tsc_bfind = DeeType_RequireSeqBFind_private_uncached(orig_type, self);
		if (tsc_bfind != NULL &&
		    tsc_bfind != &DeeSeq_DefaultBFindWithError) {
			if (tsc_bfind == &DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBLocateWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBLocateWithSeqBFindAndGetItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqBLocateWithKey)
	{
		Dee_tsc_bfind_with_key_t tsc_bfind_with_key;
		tsc_bfind_with_key = DeeType_RequireSeqBFindWithKey_private_uncached(orig_type, self);
		if (tsc_bfind_with_key != NULL &&
		    tsc_bfind_with_key != &DeeSeq_DefaultBFindWithKeyWithError) {
			if (tsc_bfind_with_key == &DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBLocateWithKeyWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBLocateWithKeyWithSeqBFindWithKeyAndGetItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_RequireSetInsert)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* use insertall and "operator #" before/after */
		if (DeeType_HasOperator(orig_type, OPERATOR_SIZE) &&
		    DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_insertall, 9, Dee_HashStr__insertall, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_set_insertall))
			return &DeeSet_DefaultInsertWithSeqSizeAndSetInsertAll;
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* >> local x = Dict();
		 * >> (x as Set).insert(("foo", "bar")); // x.setnew("foo", "bar");
		 * >> print repr x; // {"foo":"bar"} */
		Dee_tsc_map_setnew_t tsc_map_setnew;
		tsc_map_setnew = DeeType_RequireMapSetNew_private_uncached(orig_type, self);
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
		tsc_append = DeeType_RequireSeqAppend_private_uncached(orig_type, self);
		if (tsc_append != NULL &&
		    tsc_append != &DeeSeq_DefaultAppendWithError)
			return &DeeSet_DefaultInsertWithSeqSeqContainsAndSeqAppend;
	}
#elif defined(DEFINE_DeeType_RequireSetRemove)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* use removeall and "operator #" before/after */
		if (DeeType_HasOperator(orig_type, OPERATOR_SIZE) &&
		    DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_removeall, 9, Dee_HashStr__removeall, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_set_removeall))
			return &DeeSet_DefaultRemoveWithSeqSizeAndSeqRemoveAll;
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* >> local x = Dict(("foo", "bar"));
		 * >> (x as Set).remove(("foo", "bar")); // if (equals(x["foo"], "bar")) del x["foo"];
		 * >> print repr x; // {} */
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM) &&
		    DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM))
			return &DeeSet_DefaultRemoveWithMapTryGetItemAndMapDelItem;
	} else {
		/* >> local x = [10, 20, 30];
		 * >> (x as Set).remove(30); // x.remove(30);  (using `DeeType_RequireSeqRemove')
		 * >> print repr x;          // [10, 20] */
		Dee_tsc_remove_t tsc_remove;
		tsc_remove = DeeType_RequireSeqRemove_private_uncached(orig_type, self);
		if (tsc_remove != NULL &&
		    tsc_remove != &DeeSeq_DefaultRemoveWithError)
			return &DeeSet_DefaultRemoveWithSeqRemove;
	}
#elif defined(DEFINE_DeeType_RequireSetUnify)
	if (seqclass == Dee_SEQCLASS_SET) {
		Dee_tsc_set_insert_t tsc_set_insert;
		tsc_set_insert = DeeType_RequireSetInsert_private_uncached(orig_type, self);
		if (tsc_set_insert != NULL &&
		    tsc_set_insert != &DeeSet_DefaultInsertWithError) {
			if (DeeType_HasOperator(orig_type, OPERATOR_ITER))
				return &DeeSet_DefaultUnifyWithSetInsertAndSeqForeach;
		}
	} else if (seqclass != Dee_SEQCLASS_MAP) {
		/* >> local x = [];
		 * >> (x as Set).unify("foo"); // try { x.locate("foo"); } catch (ValueError) { x.append("foo"); }
		 * >> (x as Set).unify("bar");
		 * >> (x as Set).unify("foo");
		 * >> print repr x; // ["foo", "bar"] */
		Dee_tsc_append_t tsc_append;
		tsc_append = DeeType_RequireSeqAppend_private_uncached(orig_type, self);
		if (tsc_append != NULL &&
		    tsc_append != &DeeSeq_DefaultAppendWithError)
			return &DeeSet_DefaultUnifyWithSeqLocateAndSeqAppend;
	}
#elif defined(DEFINE_DeeType_RequireSetInsertAll)
	if (seqclass == Dee_SEQCLASS_SET) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_INPLACE_ADD))
			return &DeeSet_DefaultInsertAllWithInplaceAdd;
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_INPLACE_OR))
			return &DeeSet_DefaultInsertAllWithInplaceOr;
	}
	{
		Dee_tsc_set_insert_t tsc_set_insert;
		tsc_set_insert = DeeType_RequireSetInsert_private_uncached(orig_type, self);
		if (tsc_set_insert != NULL &&
		    tsc_set_insert != &DeeSet_DefaultInsertWithError)
			return &DeeSet_DefaultInsertAllWithSetInsert;
	}
#elif defined(DEFINE_DeeType_RequireSetRemoveAll)
	if (seqclass == Dee_SEQCLASS_SET) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_INPLACE_SUB))
			return &DeeSet_DefaultRemoveAllWithInplaceSub;
	}
	{
		Dee_tsc_set_remove_t tsc_set_remove;
		tsc_set_remove = DeeType_RequireSetRemove_private_uncached(orig_type, self);
		if (tsc_set_remove != NULL &&
		    tsc_set_remove != &DeeSet_DefaultRemoveWithError)
			return &DeeSet_DefaultRemoveAllWithSetRemove;
	}
#elif defined(DEFINE_DeeType_RequireSetPop)
	if (seqclass == Dee_SEQCLASS_SET) {
		Dee_tsc_set_remove_t tsc_set_remove;
		tsc_set_remove = DeeType_RequireSetRemove_private_uncached(orig_type, self);
		if (tsc_set_remove != NULL &&
		    tsc_set_remove != &DeeSet_DefaultRemoveWithError)
			return &DeeSet_DefaultPopWithSetFirstAndSetRemove;
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* >> local x = Dict(("foo", "bar"));
		 * >> print repr((x as Set).pop()); // ("foo", "bar")
		 * >> print repr x; // {} */
		Dee_tsc_map_popitem_t tsc_map_popitem;
		tsc_map_popitem = DeeType_RequireMapPopItem_private_uncached(orig_type, self);
		if (tsc_map_popitem != NULL &&
		    tsc_map_popitem != &DeeMap_DefaultPopItemWithError)
			return &DeeSet_DefaultPopWithMapPopItem;
	} else {
		/* >> local x = [10, 20, 30];
		 * >> print repr((x as Set).pop()); // 30
		 * >> print repr x;                 // [10, 20] */
		Dee_tsc_pop_t tsc_pop;
		tsc_pop = DeeType_RequireSeqPop_private_uncached(orig_type, self);
		if (tsc_pop != NULL &&
		    tsc_pop != &DeeSeq_DefaultPopWithError)
			return &DeeSet_DefaultPopWithSeqPop;
	}
#elif defined(DEFINE_DeeType_RequireSetPopWithDefault)
	if (seqclass == Dee_SEQCLASS_SET) {
		Dee_tsc_set_remove_t tsc_set_remove;
		tsc_set_remove = DeeType_RequireSetRemove_private_uncached(orig_type, self);
		if (tsc_set_remove != NULL &&
		    tsc_set_remove != &DeeSet_DefaultRemoveWithError)
			return &DeeSet_DefaultPopWithDefaultWithSeqTryGetFirstAndSetRemove;
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* >> local x = Dict(("foo", "bar"));
		 * >> print repr((x as Set).pop(("foo", "ignored"))); // ("foo", "bar")
		 * >> print repr x; // {} */
		Dee_tsc_map_popitem_t tsc_map_popitem;
		tsc_map_popitem = DeeType_RequireMapPopItem_private_uncached(orig_type, self);
		if (tsc_map_popitem != NULL &&
		    tsc_map_popitem != &DeeMap_DefaultPopItemWithError)
			return &DeeSet_DefaultPopWithDefaultWithMapPopItem;
	} else {
		/* >> local x = [10, 20, 30];
		 * >> print repr((x as Set).pop()); // 30
		 * >> print repr x;                 // [10, 20] */
		Dee_tsc_pop_t tsc_pop;
		tsc_pop = DeeType_RequireSeqPop_private_uncached(orig_type, self);
		if (tsc_pop != NULL &&
		    tsc_pop != &DeeSeq_DefaultPopWithError)
			return &DeeSet_DefaultPopWithDefaultWithSeqPop;
	}
#elif defined(DEFINE_DeeType_RequireMapSetOld)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_setold_ex, 9, Dee_HashStr__setold_ex, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_setold_ex))
			return &DeeMap_DefaultSetOldWithMapSetOldEx;
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
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
#elif defined(DEFINE_DeeType_RequireMapSetOldEx)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETITEM) &&
		    DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
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
#elif defined(DEFINE_DeeType_RequireMapSetNew)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_setnew_ex, 9, Dee_HashStr__setnew_ex, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_setnew_ex))
			return &DeeMap_DefaultSetNewWithMapSetNewEx;
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			struct type_seq *seq = orig_type->tp_seq;
			ASSERT(seq->tp_bounditem);
			ASSERT(seq->tp_trygetitem);
			ASSERT(seq->tp_getitem);
			if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_setdefault, 10, Dee_HashStr__setdefault, &attrinfo) &&
			    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_setdefault)) {
				if (!DeeType_IsDefaultBoundItem(seq->tp_bounditem))
					return &DeeMap_DefaultSetNewWithBoundItemAndMapSetDefault;
				if (!DeeType_IsDefaultTryGetItem(seq->tp_trygetitem))
					return &DeeMap_DefaultSetNewWithTryGetItemAndMapSetDefault;
				return &DeeMap_DefaultSetNewWithGetItemAndMapSetDefault;
			} else if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETITEM)) {
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
#elif defined(DEFINE_DeeType_RequireMapSetNewEx)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			struct type_seq *seq = orig_type->tp_seq;
			ASSERT(seq->tp_bounditem);
			ASSERT(seq->tp_trygetitem);
			ASSERT(seq->tp_getitem);
			if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_setdefault, 10, Dee_HashStr__setdefault, &attrinfo) &&
			    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_setdefault)) {
				if (!DeeType_IsDefaultTryGetItem(seq->tp_trygetitem))
					return &DeeMap_DefaultSetNewExWithTryGetItemAndMapSetDefault;
				return &DeeMap_DefaultSetNewExWithGetItemAndMapSetDefault;
			} else if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETITEM)) {
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
#elif defined(DEFINE_DeeType_RequireMapSetDefault)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_setnew_ex, 9, Dee_HashStr__setnew_ex, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_setnew_ex))
			return &DeeMap_DefaultSetDefaultWithMapSetNewEx;
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			struct type_seq *seq = orig_type->tp_seq;
			ASSERT(seq->tp_bounditem);
			ASSERT(seq->tp_trygetitem);
			ASSERT(seq->tp_getitem);
			if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_setnew, 6, Dee_HashStr__setnew, &attrinfo) &&
			    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_setnew)) {
				return &DeeMap_DefaultSetDefaultWithMapSetNewAndGetItem;
			} else if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETITEM)) {
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
#elif defined(DEFINE_DeeType_RequireMapUpdate)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_INPLACE_ADD))
			return &DeeMap_DefaultUpdateWithInplaceAdd;
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_INPLACE_OR))
			return &DeeMap_DefaultUpdateWithInplaceOr;
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETITEM))
			return &DeeMap_DefaultUpdateWithSetItem;
	} else {
		/* TODO: Treat as ?S?T2?O?O and use `Set.insert' or `Sequence.append' */
	}
#elif defined(DEFINE_DeeType_RequireMapRemove)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM)) {
			if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM))
				return &DeeMap_DefaultRemoveWithBoundItemAndDelItem;
			if (DeeType_HasOperator(orig_type, OPERATOR_SIZE))
				return &DeeMap_DefaultRemoveWithSizeAndDelItem;
		}
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_removekeys, 10, Dee_HashStr__removekeys, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_removekeys)) {
			if (DeeType_HasOperator(orig_type, OPERATOR_SIZE))
				return &DeeMap_DefaultRemoveWithSizeAndMapRemoveKeys;
		}
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Set.remove' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Sequence.erase' */
	}
#elif defined(DEFINE_DeeType_RequireMapRemoveKeys)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM))
			return &DeeMap_DefaultRemoveKeysWithDelItem;
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_remove, 6, Dee_HashStr__remove, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_remove))
			return &DeeMap_DefaultRemoveKeysWithMapRemove;
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Set.remove' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Sequence.erase' */
	}
#elif defined(DEFINE_DeeType_RequireMapPop)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			Dee_tsc_map_remove_t tsc_map_remove;
			tsc_map_remove = DeeType_RequireMapRemove_private_uncached(orig_type, self);
			if (tsc_map_remove != NULL &&
			    tsc_map_remove != &DeeMap_DefaultRemoveWithError &&
			    tsc_map_remove != &DeeMap_DefaultRemoveWithSizeAndDelItem &&
			    tsc_map_remove != &DeeMap_DefaultRemoveWithBoundItemAndDelItem)
				return &DeeMap_DefaultPopWithGetItemAndMapRemove;
			if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM))
				return &DeeMap_DefaultPopWithGetItemAndDelItem;
		}
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Set.remove' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Sequence.erase' */
	}
#elif defined(DEFINE_DeeType_RequireMapPopWithDefault)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			Dee_tsc_map_remove_t tsc_map_remove;
			tsc_map_remove = DeeType_RequireMapRemove_private_uncached(orig_type, self);
			if (tsc_map_remove != NULL &&
			    tsc_map_remove != &DeeMap_DefaultRemoveWithError &&
			    tsc_map_remove != &DeeMap_DefaultRemoveWithSizeAndDelItem &&
			    tsc_map_remove != &DeeMap_DefaultRemoveWithBoundItemAndDelItem)
				return &DeeMap_DefaultPopWithDefaultWithTryGetItemAndMapRemove;
			if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM))
				return &DeeMap_DefaultPopWithDefaultWithTryGetItemAndDelItem;
		}
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Set.remove' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Sequence.erase' */
	}
#elif defined(DEFINE_DeeType_RequireMapPopItem)
	if (seqclass == Dee_SEQCLASS_MAP) {
		Dee_tsc_map_remove_t tsc_map_remove;
		tsc_map_remove = DeeType_RequireMapRemove_private_uncached(orig_type, self);
		if (tsc_map_remove != NULL &&
		    tsc_map_remove != &DeeMap_DefaultRemoveWithError &&
		    tsc_map_remove != &DeeMap_DefaultRemoveWithSizeAndDelItem &&
		    tsc_map_remove != &DeeMap_DefaultRemoveWithBoundItemAndDelItem)
			return &DeeMap_DefaultPopItemWithSeqTryGetFirstAndMapRemove;
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM))
			return &DeeMap_DefaultPopItemWithSeqTryGetFirstAndDelItem;
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use `Set.pop' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use `Sequence.pop' */
	}
#elif defined(DEFINE_DeeType_RequireMapKeys)
	{
		Dee_tsc_map_iterkeys_t tsc_map_iterkeys;
		tsc_map_iterkeys = DeeType_RequireMapIterKeys_private_uncached(orig_type, self);
		if (tsc_map_iterkeys != NULL &&
		    tsc_map_iterkeys != &DeeMap_DefaultIterKeysWithError)
			return &DeeMap_DefaultKeysWithMapIterKeys;
	}
#elif defined(DEFINE_DeeType_RequireMapValues)
	{
		Dee_tsc_map_itervalues_t tsc_map_itervalues;
		tsc_map_itervalues = DeeType_RequireMapIterValues_private_uncached(orig_type, self);
		if (tsc_map_itervalues != NULL &&
		    tsc_map_itervalues != &DeeMap_DefaultIterValuesWithError)
			return &DeeMap_DefaultValuesWithMapIterValues;
	}
#elif defined(DEFINE_DeeType_RequireMapIterKeys)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_keys, 4, Dee_HashStr__keys, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_keys))
			return &DeeMap_DefaultIterKeysWithMapKeys;
		if (self->tp_seq && self->tp_seq->tp_iterkeys)
			return self->tp_seq->tp_iterkeys;
	}
	if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_ITER))
		return &DeeMap_DefaultIterKeysWithIter;
#elif defined(DEFINE_DeeType_RequireMapIterValues)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_values, 6, Dee_HashStr__values, &attrinfo) &&
		    (attrinfo.ai_type != Dee_ATTRINFO_METHOD || (Dee_funptr_t)attrinfo.ai_value.v_method->m_func != (Dee_funptr_t)&default_map_values))
			return &DeeMap_DefaultIterValuesWithMapValues;
	}
	if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_ITER))
		return &DeeMap_DefaultIterValuesWithIter;
#endif /* ... */
	return NULL;
}

PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_tsc_foo_t DCALL
LOCAL_DeeType_RequireSeqFoo_uncached(DeeTypeObject *__restrict self) {
	DeeTypeObject *iter;
	DeeTypeMRO mro;
	iter = DeeTypeMRO_Init(&mro, self);
	do {
		LOCAL_Dee_tsc_foo_t result;
		result = LOCAL_DeeType_RequireSeqFoo_private_uncached(self, iter);
		if (result)
			return result;
	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL);
	return &LOCAL_DeeSeq_DefaultFooWithError;
}

INTERN ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_tsc_foo_t DCALL
LOCAL_DeeType_RequireSeqFoo(DeeTypeObject *__restrict self) {
	LOCAL_Dee_tsc_foo_t result;
	struct Dee_type_seq_cache *sc;
	struct Dee_type_seq *seq = self->tp_seq;
	if likely(seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->LOCAL_tsc_foo)
			return sc->LOCAL_tsc_foo;
	}
	result = LOCAL_DeeType_RequireSeqFoo_uncached(self);
	sc     = DeeType_TryRequireSeqCache(self);
	if likely(sc)
		atomic_write(&sc->LOCAL_tsc_foo, result);
	return result;
}


#undef LOCAL_DeeSeq_AttrBar
#undef LOCAL_DeeType_RequireSeqFoo_private_uncached
#undef LOCAL_DeeType_RequireSeqFoo_uncached
#undef LOCAL_DeeType_RequireSeqFoo
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
#undef LOCAL_DeeSeq_DefaultFooForEmpty
#undef LOCAL_Dee_tsc_foo_t
#undef LOCAL_tsc_foo_data
#undef LOCAL_CANONICAL_NAME_LENGTHOF
#undef LOCAL_CANONICAL_NAME_str
#undef LOCAL_CANONICAL_NAME_STR
#undef LOCAL_CANONICAL_NAME_Dee_HashStr
#undef LOCAL_IS_OPERATOR
#undef LOCAL_IS_GETSET
#undef LOCAL_IS_GETSET_GET
#undef LOCAL_IS_GETSET_BOUND
#undef LOCAL_IS_GETSET_DEL
#undef LOCAL_IS_GETSET_SET


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

#undef DEFINE_DeeType_RequireSeqTryGetFirst
#undef DEFINE_DeeType_RequireSeqGetFirst
#undef DEFINE_DeeType_RequireSeqBoundFirst
#undef DEFINE_DeeType_RequireSeqDelFirst
#undef DEFINE_DeeType_RequireSetFirst
#undef DEFINE_DeeType_RequireSeqTryGetLast
#undef DEFINE_DeeType_RequireSeqGetLast
#undef DEFINE_DeeType_RequireSeqBoundLast
#undef DEFINE_DeeType_RequireSeqDelLast
#undef DEFINE_DeeType_RequireSetLast
#undef DEFINE_DeeType_RequireSeqAny
#undef DEFINE_DeeType_RequireSeqAnyWithKey
#undef DEFINE_DeeType_RequireSeqAnyWithRange
#undef DEFINE_DeeType_RequireSeqAnyWithRangeAndKey
#undef DEFINE_DeeType_RequireSeqAll
#undef DEFINE_DeeType_RequireSeqAllWithKey
#undef DEFINE_DeeType_RequireSeqAllWithRange
#undef DEFINE_DeeType_RequireSeqAllWithRangeAndKey
#undef DEFINE_DeeType_RequireSeqParity
#undef DEFINE_DeeType_RequireSeqParityWithKey
#undef DEFINE_DeeType_RequireSeqParityWithRange
#undef DEFINE_DeeType_RequireSeqParityWithRangeAndKey
#undef DEFINE_DeeType_RequireSeqReduce
#undef DEFINE_DeeType_RequireSeqReduceWithInit
#undef DEFINE_DeeType_RequireSeqReduceWithRange
#undef DEFINE_DeeType_RequireSeqReduceWithRangeAndInit
#undef DEFINE_DeeType_RequireSeqMin
#undef DEFINE_DeeType_RequireSeqMinWithKey
#undef DEFINE_DeeType_RequireSeqMinWithRange
#undef DEFINE_DeeType_RequireSeqMinWithRangeAndKey
#undef DEFINE_DeeType_RequireSeqMax
#undef DEFINE_DeeType_RequireSeqMaxWithKey
#undef DEFINE_DeeType_RequireSeqMaxWithRange
#undef DEFINE_DeeType_RequireSeqMaxWithRangeAndKey
#undef DEFINE_DeeType_RequireSeqSum
#undef DEFINE_DeeType_RequireSeqSumWithRange
#undef DEFINE_DeeType_RequireSeqCount
#undef DEFINE_DeeType_RequireSeqCountWithKey
#undef DEFINE_DeeType_RequireSeqCountWithRange
#undef DEFINE_DeeType_RequireSeqCountWithRangeAndKey
#undef DEFINE_DeeType_RequireSeqContains
#undef DEFINE_DeeType_RequireSeqContainsWithKey
#undef DEFINE_DeeType_RequireSeqContainsWithRange
#undef DEFINE_DeeType_RequireSeqContainsWithRangeAndKey
#undef DEFINE_DeeType_RequireSeqLocate
#undef DEFINE_DeeType_RequireSeqLocateWithKey
#undef DEFINE_DeeType_RequireSeqLocateWithRange
#undef DEFINE_DeeType_RequireSeqLocateWithRangeAndKey
#undef DEFINE_DeeType_RequireSeqRLocateWithRange
#undef DEFINE_DeeType_RequireSeqRLocateWithRangeAndKey
#undef DEFINE_DeeType_RequireSeqStartsWith
#undef DEFINE_DeeType_RequireSeqStartsWithWithKey
#undef DEFINE_DeeType_RequireSeqStartsWithWithRange
#undef DEFINE_DeeType_RequireSeqStartsWithWithRangeAndKey
#undef DEFINE_DeeType_RequireSeqEndsWith
#undef DEFINE_DeeType_RequireSeqEndsWithWithKey
#undef DEFINE_DeeType_RequireSeqEndsWithWithRange
#undef DEFINE_DeeType_RequireSeqEndsWithWithRangeAndKey
#undef DEFINE_DeeType_RequireSeqFind
#undef DEFINE_DeeType_RequireSeqFindWithKey
#undef DEFINE_DeeType_RequireSeqRFind
#undef DEFINE_DeeType_RequireSeqRFindWithKey
#undef DEFINE_DeeType_RequireSeqErase
#undef DEFINE_DeeType_RequireSeqInsert
#undef DEFINE_DeeType_RequireSeqInsertAll
#undef DEFINE_DeeType_RequireSeqPushFront
#undef DEFINE_DeeType_RequireSeqAppend
#undef DEFINE_DeeType_RequireSeqExtend
#undef DEFINE_DeeType_RequireSeqXchItemIndex
#undef DEFINE_DeeType_RequireSeqClear
#undef DEFINE_DeeType_RequireSeqPop
#undef DEFINE_DeeType_RequireSeqRemove
#undef DEFINE_DeeType_RequireSeqRemoveWithKey
#undef DEFINE_DeeType_RequireSeqRRemove
#undef DEFINE_DeeType_RequireSeqRRemoveWithKey
#undef DEFINE_DeeType_RequireSeqRemoveAll
#undef DEFINE_DeeType_RequireSeqRemoveAllWithKey
#undef DEFINE_DeeType_RequireSeqRemoveIf
#undef DEFINE_DeeType_RequireSeqResize
#undef DEFINE_DeeType_RequireSeqFill
#undef DEFINE_DeeType_RequireSeqReverse
#undef DEFINE_DeeType_RequireSeqReversed
#undef DEFINE_DeeType_RequireSeqSort
#undef DEFINE_DeeType_RequireSeqSortWithKey
#undef DEFINE_DeeType_RequireSeqSorted
#undef DEFINE_DeeType_RequireSeqSortedWithKey
#undef DEFINE_DeeType_RequireSeqBFind
#undef DEFINE_DeeType_RequireSeqBFindWithKey
#undef DEFINE_DeeType_RequireSeqBPosition
#undef DEFINE_DeeType_RequireSeqBPositionWithKey
#undef DEFINE_DeeType_RequireSeqBRange
#undef DEFINE_DeeType_RequireSeqBRangeWithKey
#undef DEFINE_DeeType_RequireSeqBLocate
#undef DEFINE_DeeType_RequireSeqBLocateWithKey
#undef DEFINE_DeeType_RequireSetInsert
#undef DEFINE_DeeType_RequireSetRemove
#undef DEFINE_DeeType_RequireSetUnify
#undef DEFINE_DeeType_RequireSetInsertAll
#undef DEFINE_DeeType_RequireSetRemoveAll
#undef DEFINE_DeeType_RequireSetPop
#undef DEFINE_DeeType_RequireSetPopWithDefault
#undef DEFINE_DeeType_RequireMapSetOld
#undef DEFINE_DeeType_RequireMapSetOldEx
#undef DEFINE_DeeType_RequireMapSetNew
#undef DEFINE_DeeType_RequireMapSetNewEx
#undef DEFINE_DeeType_RequireMapSetDefault
#undef DEFINE_DeeType_RequireMapUpdate
#undef DEFINE_DeeType_RequireMapRemove
#undef DEFINE_DeeType_RequireMapRemoveKeys
#undef DEFINE_DeeType_RequireMapPop
#undef DEFINE_DeeType_RequireMapPopWithDefault
#undef DEFINE_DeeType_RequireMapPopItem
#undef DEFINE_DeeType_RequireMapKeys
#undef DEFINE_DeeType_RequireMapValues
#undef DEFINE_DeeType_RequireMapIterKeys
#undef DEFINE_DeeType_RequireMapIterValues
