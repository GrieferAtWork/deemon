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
//#define DEFINE_DeeType_RequireSeqBoundLast
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
#define DEFINE_DeeType_RequireSeqXchItemIndex
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
     defined(DEFINE_DeeType_RequireSetFirst) +                     \
     defined(DEFINE_DeeType_RequireSeqTryGetLast) +                \
     defined(DEFINE_DeeType_RequireSeqGetLast) +                   \
     defined(DEFINE_DeeType_RequireSeqBoundLast) +                 \
     defined(DEFINE_DeeType_RequireSeqDelLast) +                   \
     defined(DEFINE_DeeType_RequireSetLast) +                      \
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
#define LOCAL_foo      trygetfirst
#define LOCAL_Foo      TryGetFirst
#define LOCAL_Attr_foo first
#define LOCAL_WITHOUT_ATTRIBUTE
#elif defined(DEFINE_DeeType_RequireSeqGetFirst)
#define LOCAL_foo      getfirst
#define LOCAL_Foo      GetFirst
#define LOCAL_Attr_foo first
#define LOCAL_IS_GETSET_GET
#define LOCAL_default_seq_foo default_seq_getfirst
#define LOCAL_NO_TMH
#elif defined(DEFINE_DeeType_RequireSeqBoundFirst)
#define LOCAL_foo              boundfirst
#define LOCAL_Foo              BoundFirst
#define LOCAL_Bar              GetFirst
#define LOCAL_Attr_foo         first
#define LOCAL_tsc_seq_foo_data tsc_seq_getfirst_data
#define LOCAL_IS_GETSET_BOUND
#define LOCAL_default_seq_foo default_seq_boundfirst
#define LOCAL_NO_TMH
#elif defined(DEFINE_DeeType_RequireSeqDelFirst)
#define LOCAL_foo      delfirst
#define LOCAL_Foo      DelFirst
#define LOCAL_Attr_foo first
#define LOCAL_IS_GETSET_DEL
#define LOCAL_default_seq_foo default_seq_delfirst
#define LOCAL_NO_TMH
#elif defined(DEFINE_DeeType_RequireSetFirst)
#define LOCAL_foo      setfirst
#define LOCAL_Foo      SetFirst
#define LOCAL_Attr_foo first
#define LOCAL_IS_GETSET_SET
#define LOCAL_default_seq_foo default_seq_setfirst
#define LOCAL_NO_TMH
#elif defined(DEFINE_DeeType_RequireSeqTryGetLast)
#define LOCAL_foo      trygetlast
#define LOCAL_Foo      TryGetLast
#define LOCAL_Attr_foo last
#define LOCAL_WITHOUT_ATTRIBUTE
#elif defined(DEFINE_DeeType_RequireSeqGetLast)
#define LOCAL_foo      getlast
#define LOCAL_Foo      GetLast
#define LOCAL_Attr_foo last
#define LOCAL_IS_GETSET_GET
#define LOCAL_default_seq_foo default_seq_getlast
#define LOCAL_NO_TMH
#elif defined(DEFINE_DeeType_RequireSeqBoundLast)
#define LOCAL_foo              boundlast
#define LOCAL_Foo              BoundLast
#define LOCAL_Bar              GetLast
#define LOCAL_Attr_foo         last
#define LOCAL_tsc_seq_foo_data tsc_seq_getlast_data
#define LOCAL_IS_GETSET_BOUND
#define LOCAL_default_seq_foo default_seq_boundlast
#define LOCAL_NO_TMH
#elif defined(DEFINE_DeeType_RequireSeqDelLast)
#define LOCAL_foo      dellast
#define LOCAL_Foo      DelLast
#define LOCAL_Attr_foo last
#define LOCAL_IS_GETSET_DEL
#define LOCAL_default_seq_foo default_seq_dellast
#define LOCAL_NO_TMH
#elif defined(DEFINE_DeeType_RequireSetLast)
#define LOCAL_foo      setlast
#define LOCAL_Foo      SetLast
#define LOCAL_Attr_foo last
#define LOCAL_IS_GETSET_SET
#define LOCAL_default_seq_foo default_seq_setlast
#define LOCAL_NO_TMH
#elif defined(DEFINE_DeeType_RequireSeqAny)
#define LOCAL_foo                        any
#define LOCAL_Foo                        Any
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAnyWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqAnyWithKey)
#define LOCAL_foo                        any_with_key
#define LOCAL_Foo                        AnyWithKey
#define LOCAL_Bar                        Any
#define LOCAL_Attr_foo                   any
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAnyWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqAnyWithRange)
#define LOCAL_foo                        any_with_range
#define LOCAL_Foo                        AnyWithRange
#define LOCAL_Attr_foo                   any
#define LOCAL_Bar                        Any
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAnyWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqAnyWithRangeAndKey)
#define LOCAL_foo                        any_with_range_and_key
#define LOCAL_Foo                        AnyWithRangeAndKey
#define LOCAL_Attr_foo                   any
#define LOCAL_Bar                        Any
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAnyWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqAll)
#define LOCAL_foo                        all
#define LOCAL_Foo                        All
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAllWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqAllWithKey)
#define LOCAL_foo                        all_with_key
#define LOCAL_Foo                        AllWithKey
#define LOCAL_Attr_foo                   all
#define LOCAL_Bar                        All
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAllWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqAllWithRange)
#define LOCAL_foo                        all_with_range
#define LOCAL_Foo                        AllWithRange
#define LOCAL_Attr_foo                   all
#define LOCAL_Bar                        All
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAllWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqAllWithRangeAndKey)
#define LOCAL_foo                        all_with_range_and_key
#define LOCAL_Foo                        AllWithRangeAndKey
#define LOCAL_Attr_foo                   all
#define LOCAL_Bar                        All
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultAllWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqParity)
#define LOCAL_foo                        parity
#define LOCAL_Foo                        Parity
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultParityWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqParityWithKey)
#define LOCAL_foo                        parity_with_key
#define LOCAL_Foo                        ParityWithKey
#define LOCAL_Attr_foo                   parity
#define LOCAL_Bar                        Parity
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultParityWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqParityWithRange)
#define LOCAL_foo                        parity_with_range
#define LOCAL_Foo                        ParityWithRange
#define LOCAL_Attr_foo                   parity
#define LOCAL_Bar                        Parity
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultParityWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqParityWithRangeAndKey)
#define LOCAL_foo                        parity_with_range_and_key
#define LOCAL_Foo                        ParityWithRangeAndKey
#define LOCAL_Attr_foo                   parity
#define LOCAL_Bar                        Parity
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultParityWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqReduce)
#define LOCAL_foo                        reduce
#define LOCAL_Foo                        Reduce
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReduceWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqReduceWithInit)
#define LOCAL_foo                        reduce_with_init
#define LOCAL_Foo                        ReduceWithInit
#define LOCAL_Attr_foo                   reduce
#define LOCAL_Bar                        Reduce
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReduceWithInitWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqReduceWithRange)
#define LOCAL_foo                        reduce_with_range
#define LOCAL_Foo                        ReduceWithRange
#define LOCAL_Attr_foo                   reduce
#define LOCAL_Bar                        Reduce
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReduceWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqReduceWithRangeAndInit)
#define LOCAL_foo                        reduce_with_range_and_init
#define LOCAL_Foo                        ReduceWithRangeAndInit
#define LOCAL_Attr_foo                   reduce
#define LOCAL_Bar                        Reduce
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReduceWithRangeAndInitWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqMin)
#define LOCAL_foo                        min
#define LOCAL_Foo                        Min
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMinWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqMinWithKey)
#define LOCAL_foo                        min_with_key
#define LOCAL_Foo                        MinWithKey
#define LOCAL_Attr_foo                   min
#define LOCAL_Bar                        Min
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMinWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqMinWithRange)
#define LOCAL_foo                        min_with_range
#define LOCAL_Foo                        MinWithRange
#define LOCAL_Attr_foo                   min
#define LOCAL_Bar                        Min
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMinWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqMinWithRangeAndKey)
#define LOCAL_foo                        min_with_range_and_key
#define LOCAL_Foo                        MinWithRangeAndKey
#define LOCAL_Attr_foo                   min
#define LOCAL_Bar                        Min
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMinWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqMax)
#define LOCAL_foo                        max
#define LOCAL_Foo                        Max
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMaxWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqMaxWithKey)
#define LOCAL_foo                        max_with_key
#define LOCAL_Foo                        MaxWithKey
#define LOCAL_Attr_foo                   max
#define LOCAL_Bar                        Max
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMaxWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqMaxWithRange)
#define LOCAL_foo                        max_with_range
#define LOCAL_Foo                        MaxWithRange
#define LOCAL_Attr_foo                   max
#define LOCAL_Bar                        Max
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMaxWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqMaxWithRangeAndKey)
#define LOCAL_foo                        max_with_range_and_key
#define LOCAL_Foo                        MaxWithRangeAndKey
#define LOCAL_Attr_foo                   max
#define LOCAL_Bar                        Max
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultMaxWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqSum)
#define LOCAL_foo                        sum
#define LOCAL_Foo                        Sum
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultSumWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqSumWithRange)
#define LOCAL_foo                        sum_with_range
#define LOCAL_Foo                        SumWithRange
#define LOCAL_Attr_foo                   sum
#define LOCAL_Bar                        Sum
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultSumWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqCount)
#define LOCAL_foo                        count
#define LOCAL_Foo                        Count
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultCountWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqCountWithKey)
#define LOCAL_foo                        count_with_key
#define LOCAL_Foo                        CountWithKey
#define LOCAL_Attr_foo                   count
#define LOCAL_Bar                        Count
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultCountWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqCountWithRange)
#define LOCAL_foo                        count_with_range
#define LOCAL_Foo                        CountWithRange
#define LOCAL_Attr_foo                   count
#define LOCAL_Bar                        Count
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultCountWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqCountWithRangeAndKey)
#define LOCAL_foo                        count_with_range_and_key
#define LOCAL_Foo                        CountWithRangeAndKey
#define LOCAL_Attr_foo                   count
#define LOCAL_Bar                        Count
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultCountWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqContains)
#define LOCAL_foo                        contains
#define LOCAL_Foo                        Contains
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultContainsWithForeach
#elif defined(DEFINE_DeeType_RequireSeqContainsWithKey)
#define LOCAL_foo                        contains_with_key
#define LOCAL_Foo                        ContainsWithKey
#define LOCAL_Attr_foo                   contains
#define LOCAL_Bar                        Contains
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultContainsWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqContainsWithRange)
#define LOCAL_foo                        contains_with_range
#define LOCAL_Foo                        ContainsWithRange
#define LOCAL_Attr_foo                   contains
#define LOCAL_Bar                        Contains
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultContainsWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqContainsWithRangeAndKey)
#define LOCAL_foo                        contains_with_range_and_key
#define LOCAL_Foo                        ContainsWithRangeAndKey
#define LOCAL_Attr_foo                   contains
#define LOCAL_Bar                        Contains
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultContainsWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqLocate)
#define LOCAL_foo                        locate
#define LOCAL_Foo                        Locate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultLocateWithSeqForeach
#elif defined(DEFINE_DeeType_RequireSeqLocateWithKey)
#define LOCAL_foo                        locate_with_key
#define LOCAL_Foo                        LocateWithKey
#define LOCAL_Attr_foo                   locate
#define LOCAL_Bar                        Locate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultLocateWithKeyWithSeqForeach
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqLocateWithRange)
#define LOCAL_foo                        locate_with_range
#define LOCAL_Foo                        LocateWithRange
#define LOCAL_Attr_foo                   locate
#define LOCAL_Bar                        Locate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultLocateWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqLocateWithRangeAndKey)
#define LOCAL_foo                        locate_with_range_and_key
#define LOCAL_Foo                        LocateWithRangeAndKey
#define LOCAL_Attr_foo                   locate
#define LOCAL_Bar                        Locate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultLocateWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRLocateWithRange)
#define LOCAL_foo                        rlocate_with_range
#define LOCAL_Foo                        RLocateWithRange
#define LOCAL_Attr_foo                   rlocate
#define LOCAL_Bar                        RLocate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultRLocateWithRangeWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRLocateWithRangeAndKey)
#define LOCAL_foo                        rlocate_with_range_and_key
#define LOCAL_Foo                        RLocateWithRangeAndKey
#define LOCAL_Attr_foo                   rlocate
#define LOCAL_Bar                        RLocate
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultRLocateWithRangeAndKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqStartsWith)
#define LOCAL_foo                        startswith
#define LOCAL_Foo                        StartsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultStartsWithWithSeqTryGetFirst
#elif defined(DEFINE_DeeType_RequireSeqStartsWithWithKey)
#define LOCAL_foo                        startswith_with_key
#define LOCAL_Foo                        StartsWithWithKey
#define LOCAL_Attr_foo                   startswith
#define LOCAL_Bar                        StartsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultStartsWithWithKeyWithSeqTryGetFirst
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqStartsWithWithRange)
#define LOCAL_foo                        startswith_with_range
#define LOCAL_Foo                        StartsWithWithRange
#define LOCAL_Attr_foo                   startswith
#define LOCAL_Bar                        StartsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultStartsWithWithRangeWithSeqTryGetItemIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqStartsWithWithRangeAndKey)
#define LOCAL_foo                        startswith_with_range_and_key
#define LOCAL_Foo                        StartsWithWithRangeAndKey
#define LOCAL_Attr_foo                   startswith
#define LOCAL_Bar                        StartsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultStartsWithWithRangeAndKeyWithSeqTryGetItemIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqEndsWith)
#define LOCAL_foo                        endswith
#define LOCAL_Foo                        EndsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultEndsWithWithSeqTryGetLast
#elif defined(DEFINE_DeeType_RequireSeqEndsWithWithKey)
#define LOCAL_foo                        endswith_with_key
#define LOCAL_Foo                        EndsWithWithKey
#define LOCAL_Attr_foo                   endswith
#define LOCAL_Bar                        EndsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultEndsWithWithKeyWithSeqTryGetLast
#define LOCAL_HAS_FOR_SEQ_SUFFIX
#elif defined(DEFINE_DeeType_RequireSeqEndsWithWithRange)
#define LOCAL_foo                        endswith_with_range
#define LOCAL_Foo                        EndsWithWithRange
#define LOCAL_Attr_foo                   endswith
#define LOCAL_Bar                        EndsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultEndsWithWithRangeWithSeqSizeAndSeqTryGetItemIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqEndsWithWithRangeAndKey)
#define LOCAL_foo                        endswith_with_range_and_key
#define LOCAL_Foo                        EndsWithWithRangeAndKey
#define LOCAL_Attr_foo                   endswith
#define LOCAL_Bar                        EndsWith
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultEndsWithWithRangeAndKeyWithSeqSizeAndSeqTryGetItemIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqFind)
#define LOCAL_foo                        find
#define LOCAL_Foo                        Find
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultFindWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqFindWithKey)
#define LOCAL_foo                        find_with_key
#define LOCAL_Foo                        FindWithKey
#define LOCAL_Attr_foo                   find
#define LOCAL_Bar                        Find
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultFindWithKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRFind)
#define LOCAL_foo                        rfind
#define LOCAL_Foo                        RFind
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultRFindWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRFindWithKey)
#define LOCAL_foo                        rfind_with_key
#define LOCAL_Foo                        RFindWithKey
#define LOCAL_Attr_foo                   rfind
#define LOCAL_Bar                        RFind
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndex
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqErase)
#define LOCAL_foo                    erase
#define LOCAL_Foo                    Erase
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqInsert)
#define LOCAL_foo                    insert
#define LOCAL_Foo                    Insert
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqInsertAll)
#define LOCAL_foo                    insertall
#define LOCAL_Foo                    InsertAll
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqPushFront)
#define LOCAL_foo                        pushfront
#define LOCAL_Foo                        PushFront
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultPushFrontWithSeqInsert /* Use insert() by default */
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqAppend)
#define LOCAL_foo                    append
#define LOCAL_Foo                    Append
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqExtend)
#define LOCAL_foo                    extend
#define LOCAL_Foo                    Extend
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqXchItemIndex)
#define LOCAL_foo                    xchitem_index
#define LOCAL_Foo                    XchItemIndex
#define LOCAL_tsc_seq_foo_data       tsc_seq_xchitem_data
#define LOCAL_Attr_foo               xchitem
#define LOCAL_Bar                    XchItem
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqClear)
#define LOCAL_foo                    clear
#define LOCAL_Foo                    Clear
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqPop)
#define LOCAL_foo                    pop
#define LOCAL_Foo                    Pop
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRemove)
#define LOCAL_foo                    remove
#define LOCAL_Foo                    Remove
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRemoveWithKey)
#define LOCAL_foo                    remove_with_key
#define LOCAL_Foo                    RemoveWithKey
#define LOCAL_Attr_foo               remove
#define LOCAL_Bar                    Remove
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRRemove)
#define LOCAL_foo                    rremove
#define LOCAL_Foo                    RRemove
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRRemoveWithKey)
#define LOCAL_foo                    rremove_with_key
#define LOCAL_Foo                    RRemoveWithKey
#define LOCAL_Attr_foo               rremove
#define LOCAL_Bar                    RRemove
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRemoveAll)
#define LOCAL_foo                    removeall
#define LOCAL_Foo                    RemoveAll
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRemoveAllWithKey)
#define LOCAL_foo                    removeall_with_key
#define LOCAL_Foo                    RemoveAllWithKey
#define LOCAL_Attr_foo               removeall
#define LOCAL_Bar                    RemoveAll
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqRemoveIf)
#define LOCAL_foo                    removeif
#define LOCAL_Foo                    RemoveIf
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqResize)
#define LOCAL_foo                        resize
#define LOCAL_Foo                        Resize
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultResizeWithSizeAndSeqEraseAndSeqExtend /* Use erase() and extend() by default */
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqFill)
#define LOCAL_foo                    fill
#define LOCAL_Foo                    Fill
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqReverse)
#define LOCAL_foo                    reverse
#define LOCAL_Foo                    Reverse
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqReversed)
#define LOCAL_foo                        reversed
#define LOCAL_Foo                        Reversed
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultReversedWithCopyForeachDefault
#define LOCAL_ATTR_REQUIRED_SEQCLASS     Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqSort)
#define LOCAL_foo      sort
#define LOCAL_Foo      Sort
#elif defined(DEFINE_DeeType_RequireSeqSortWithKey)
#define LOCAL_foo      sort_with_key
#define LOCAL_Foo      SortWithKey
#define LOCAL_Attr_foo sort
#define LOCAL_Bar      Sort
#elif defined(DEFINE_DeeType_RequireSeqSorted)
#define LOCAL_foo                        sorted
#define LOCAL_Foo                        Sorted
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultSortedWithCopyForeachDefault
#elif defined(DEFINE_DeeType_RequireSeqSortedWithKey)
#define LOCAL_foo                        sorted_with_key
#define LOCAL_Foo                        SortedWithKey
#define LOCAL_Attr_foo                   sorted
#define LOCAL_Bar                        Sorted
#define LOCAL_DeeSeq_DefaultFooWithError DeeSeq_DefaultSortedWithKeyWithCopyForeachDefault
#elif defined(DEFINE_DeeType_RequireSeqBFind)
#define LOCAL_foo                    bfind
#define LOCAL_Foo                    BFind
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqBFindWithKey)
#define LOCAL_foo                    bfind_with_key
#define LOCAL_Foo                    BFindWithKey
#define LOCAL_Attr_foo               bfind
#define LOCAL_Bar                    BFind
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqBPosition)
#define LOCAL_foo                    bposition
#define LOCAL_Foo                    BPosition
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqBPositionWithKey)
#define LOCAL_foo                    bposition_with_key
#define LOCAL_Foo                    BPositionWithKey
#define LOCAL_Attr_foo               bposition
#define LOCAL_Bar                    BPosition
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqBRange)
#define LOCAL_foo                    brange
#define LOCAL_Foo                    BRange
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqBRangeWithKey)
#define LOCAL_foo                    brange_with_key
#define LOCAL_Foo                    BRangeWithKey
#define LOCAL_Attr_foo               brange
#define LOCAL_Bar                    BRange
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqBLocate)
#define LOCAL_foo                    blocate
#define LOCAL_Foo                    BLocate
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSeqBLocateWithKey)
#define LOCAL_foo                    blocate_with_key
#define LOCAL_Foo                    BLocateWithKey
#define LOCAL_Attr_foo               blocate
#define LOCAL_Bar                    BLocate
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SEQ
#elif defined(DEFINE_DeeType_RequireSetInsert)
#define LOCAL_foo                    insert
#define LOCAL_Foo                    Insert
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_SET
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetRemove)
#define LOCAL_foo                    remove
#define LOCAL_Foo                    Remove
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_SET
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetUnify)
#define LOCAL_foo                    unify
#define LOCAL_Foo                    Unify
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_SET
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetInsertAll)
#define LOCAL_foo                    insertall
#define LOCAL_Foo                    InsertAll
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_SET
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetRemoveAll)
#define LOCAL_foo                    removeall
#define LOCAL_Foo                    RemoveAll
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_SET
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetPop)
#define LOCAL_foo                    pop
#define LOCAL_Foo                    Pop
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_SET
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireSetPopWithDefault)
#define LOCAL_foo                    pop_with_default
#define LOCAL_Foo                    PopWithDefault
#define LOCAL_Attr_foo               pop
#define LOCAL_Bar                    Pop
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_SET
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_SET
#elif defined(DEFINE_DeeType_RequireMapSetOld)
#define LOCAL_foo                    setold
#define LOCAL_Foo                    SetOld
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapSetOldEx)
#define LOCAL_foo                    setold_ex
#define LOCAL_Foo                    SetOldEx
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapSetNew)
#define LOCAL_foo                    setnew
#define LOCAL_Foo                    SetNew
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapSetNewEx)
#define LOCAL_foo                    setnew_ex
#define LOCAL_Foo                    SetNewEx
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapSetDefault)
#define LOCAL_foo                    setdefault
#define LOCAL_Foo                    SetDefault
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapUpdate)
#define LOCAL_foo                    update
#define LOCAL_Foo                    Update
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapRemove)
#define LOCAL_foo                    remove
#define LOCAL_Foo                    Remove
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapRemoveKeys)
#define LOCAL_foo                    removekeys
#define LOCAL_Foo                    RemoveKeys
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapPop)
#define LOCAL_foo                    pop
#define LOCAL_Foo                    Pop
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapPopWithDefault)
#define LOCAL_foo                    pop_with_default
#define LOCAL_Foo                    PopWithDefault
#define LOCAL_Attr_foo               pop
#define LOCAL_Bar                    Pop
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapPopItem)
#define LOCAL_foo                    popitem
#define LOCAL_Foo                    PopItem
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#elif defined(DEFINE_DeeType_RequireMapKeys)
#define LOCAL_foo                    keys
#define LOCAL_Foo                    Keys
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_IS_GETSET_GET
#define LOCAL_default_seq_foo default_map_keys
#elif defined(DEFINE_DeeType_RequireMapValues)
#define LOCAL_foo                    values
#define LOCAL_Foo                    Values
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_IS_GETSET_GET
#define LOCAL_default_seq_foo default_map_values
#elif defined(DEFINE_DeeType_RequireMapIterKeys)
#define LOCAL_foo                    iterkeys
#define LOCAL_Foo                    IterKeys
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_IS_GETSET_GET
#define LOCAL_default_seq_foo default_map_iterkeys
#elif defined(DEFINE_DeeType_RequireMapIterValues)
#define LOCAL_foo                    itervalues
#define LOCAL_Foo                    IterValues
#define LOCAL_Dee_SEQCLASS           Dee_SEQCLASS_MAP
#define LOCAL_ATTR_REQUIRED_SEQCLASS Dee_SEQCLASS_MAP
#define LOCAL_IS_GETSET_GET
#define LOCAL_default_seq_foo default_map_itervalues
#else /* DEFINE_DeeType_RequireSeq... */
#error "Invalid configuration"
#endif /* !DEFINE_DeeType_RequireSeq... */

#if (defined(LOCAL_IS_GETSET_GET) || defined(LOCAL_IS_GETSET_BOUND) || \
     defined(LOCAL_IS_GETSET_DEL) || defined(LOCAL_IS_GETSET_SET))
#define LOCAL_IS_GETSET
#endif /* LOCAL_IS_GETSET_* */

#ifndef LOCAL_method_foo
#if defined(LOCAL_WITHOUT_ATTRIBUTE) || defined(LOCAL_IS_GETSET)
#define LOCAL_method_foo LOCAL_foo
#else /* LOCAL_WITHOUT_ATTRIBUTE || LOCAL_IS_GETSET */
#define LOCAL_method_foo LOCAL_Attr_foo
#endif /* !LOCAL_WITHOUT_ATTRIBUTE && !LOCAL_IS_GETSET */
#endif /* !LOCAL_ */

#ifndef LOCAL_Dee_SEQCLASS
#define LOCAL_Dee_SEQCLASS Dee_SEQCLASS_SEQ
#endif /* !LOCAL_Dee_SEQCLASS */

#ifndef LOCAL_NO_TMH
#ifndef LOCAL_TMH
#if LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SEQ
#define LOCAL_TMH PP_CAT2(Dee_TMH_seq_, LOCAL_foo)
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SET
#define LOCAL_TMH PP_CAT2(Dee_TMH_set_, LOCAL_foo)
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_MAP
#define LOCAL_TMH PP_CAT2(Dee_TMH_map_, LOCAL_foo)
#else /* LOCAL_Dee_SEQCLASS == ... */
#define LOCAL_TMH PP_CAT2(Dee_TMH_INVALID_LOCAL_Dee_SEQCLASS_, LOCAL_foo)
#endif /* LOCAL_Dee_SEQCLASS != ... */
#endif /* !LOCAL_TMH */
#endif /* !LOCAL_NO_TMH */

#ifndef LOCAL_default_seq_foo
#if LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SEQ
#define LOCAL_default_seq_foo PP_CAT2(DeeMH_seq_, LOCAL_method_foo)
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SET
#define LOCAL_default_seq_foo PP_CAT2(DeeMH_set_, LOCAL_method_foo)
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_MAP
#define LOCAL_default_seq_foo PP_CAT2(DeeMH_map_, LOCAL_method_foo)
#else /* LOCAL_Dee_SEQCLASS == ... */
#define LOCAL_default_seq_foo PP_CAT2(default_INVALID_LOCAL_Dee_SEQCLASS_, LOCAL_method_foo)
#endif /* LOCAL_Dee_SEQCLASS != ... */
#endif /* !LOCAL_default_seq_foo */

#ifndef LOCAL_Attr_foo
#define LOCAL_Attr_foo LOCAL_foo
#endif /* !LOCAL_Attr_foo */

#ifndef LOCAL_tsc_seq_foo
#if LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SEQ
#define LOCAL_tsc_seq_foo PP_CAT2(tsc_seq_, LOCAL_foo)
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SET
#define LOCAL_tsc_seq_foo PP_CAT2(tsc_set_, LOCAL_foo)
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_MAP
#define LOCAL_tsc_seq_foo PP_CAT2(tsc_map_, LOCAL_foo)
#else /* LOCAL_Dee_SEQCLASS == ... */
#define LOCAL_tsc_seq_foo PP_CAT2(tsc_INVALID_LOCAL_Dee_SEQCLASS_, LOCAL_foo)
#endif /* LOCAL_Dee_SEQCLASS != ... */
#endif /* !LOCAL_tsc_seq_foo */

#ifndef LOCAL_tsc_seq_method_foo
#if LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SEQ
#define LOCAL_tsc_seq_method_foo PP_CAT2(tsc_seq_, LOCAL_method_foo)
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SET
#define LOCAL_tsc_seq_method_foo PP_CAT2(tsc_set_, LOCAL_method_foo)
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_MAP
#define LOCAL_tsc_seq_method_foo PP_CAT2(tsc_map_, LOCAL_method_foo)
#else /* LOCAL_Dee_SEQCLASS == ... */
#define LOCAL_tsc_seq_method_foo PP_CAT2(tsc_INVALID_LOCAL_Dee_SEQCLASS_, LOCAL_method_foo)
#endif /* LOCAL_Dee_SEQCLASS != ... */
#endif /* !LOCAL_tsc_seq_method_foo */

#ifndef LOCAL_Dee_mh_seq_foo_t
#if LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SEQ
#define LOCAL_Dee_mh_seq_foo_t PP_CAT3(Dee_mh_seq_, LOCAL_foo, _t)
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SET
#define LOCAL_Dee_mh_seq_foo_t PP_CAT3(Dee_mh_set_, LOCAL_foo, _t)
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_MAP
#define LOCAL_Dee_mh_seq_foo_t PP_CAT3(Dee_mh_map_, LOCAL_foo, _t)
#else /* LOCAL_Dee_SEQCLASS == ... */
#define LOCAL_Dee_mh_seq_foo_t PP_CAT3(Dee_tsc_INVALID_LOCAL_Dee_SEQCLASS_, LOCAL_foo, _t)
#endif /* LOCAL_Dee_SEQCLASS != ... */
#endif /* !LOCAL_Dee_mh_seq_foo_t */
#ifndef LOCAL_tsc_seq_foo_data
#define LOCAL_tsc_seq_foo_data PP_CAT2(LOCAL_tsc_seq_method_foo, _data)
#endif /* !LOCAL_tsc_seq_foo_data */


#ifndef LOCAL_DeeSeq_Default
#if LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SEQ
#define LOCAL_DeeSeq_Default DeeSeq_Default
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_Default DeeSet_Default
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_Default DeeMap_Default
#else /* LOCAL_Dee_SEQCLASS == ... */
#define LOCAL_DeeSeq_Default DeeINVALID_LOCAL_Dee_SEQCLASS_Default
#endif /* LOCAL_Dee_SEQCLASS != ... */
#endif /* !LOCAL_DeeSeq_Default */

#ifndef LOCAL_DeeSeq_RequireFoo
#if LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SEQ
#define LOCAL_DeeSeq_RequireFoo PP_CAT2(Seq, LOCAL_Foo)
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SET
#define LOCAL_DeeSeq_RequireFoo PP_CAT2(Set, LOCAL_Foo)
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_MAP
#define LOCAL_DeeSeq_RequireFoo PP_CAT2(Map, LOCAL_Foo)
#else /* LOCAL_Dee_SEQCLASS == ... */
#define LOCAL_DeeSeq_RequireFoo PP_CAT2(INVALID_LOCAL_DeeSeq_RequireFoo, LOCAL_Foo)
#endif /* LOCAL_Dee_SEQCLASS != ... */
#endif /* !LOCAL_DeeSeq_RequireFoo */

#define LOCAL_DeeType_RequireSeqFoo_private_uncached PP_CAT3(DeeType_Require, LOCAL_DeeSeq_RequireFoo, _private_uncached)
#define LOCAL_DeeType_RequireSeqFoo_uncached         PP_CAT3(DeeType_Require, LOCAL_DeeSeq_RequireFoo, _uncached)
#define LOCAL_DeeType_RequireSeqFoo                  PP_CAT2(DeeType_Require, LOCAL_DeeSeq_RequireFoo)

#ifndef LOCAL_Bar
#define LOCAL_Bar LOCAL_Foo
#endif /* !LOCAL_Bar */

#ifdef LOCAL_HAS_FOR_SEQ_SUFFIX
#define LOCAL_DeeSeq_DefaultFooWithCallAttrBarForSeq              PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_Foo, WithCallAttr, LOCAL_Bar, ForSeq)
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataFunctionForSeq      PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_Foo, WithCall, LOCAL_Bar, DataFunctionForSeq)
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataMethodForSeq        PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_Foo, WithCall, LOCAL_Bar, DataMethodForSeq)
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethodForSeq      PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_Foo, WithCall, LOCAL_Bar, DataKwMethodForSeq)
#define LOCAL_DeeSeq_DefaultFooWithCallAttrBarForSetOrMap         PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_Foo, WithCallAttr, LOCAL_Bar, ForSetOrMap)
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataFunctionForSetOrMap PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_Foo, WithCall, LOCAL_Bar, DataFunctionForSetOrMap)
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataMethodForSetOrMap   PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_Foo, WithCall, LOCAL_Bar, DataMethodForSetOrMap)
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethodForSetOrMap PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_Foo, WithCall, LOCAL_Bar, DataKwMethodForSetOrMap)
#else /* LOCAL_HAS_FOR_SEQ_SUFFIX */
#define LOCAL_DeeSeq_DefaultFooWithCallAttrBar         PP_CAT4(LOCAL_DeeSeq_Default, LOCAL_Foo, WithCallAttr, LOCAL_Bar)
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_Foo, WithCall, LOCAL_Bar, DataFunction)
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod   PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_Foo, WithCall, LOCAL_Bar, DataMethod)
#define LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod PP_CAT5(LOCAL_DeeSeq_Default, LOCAL_Foo, WithCall, LOCAL_Bar, DataKwMethod)
#endif /* !LOCAL_HAS_FOR_SEQ_SUFFIX */
#ifndef LOCAL_DeeSeq_DefaultFooWithError
#define LOCAL_DeeSeq_DefaultFooWithError \
	PP_CAT3(LOCAL_DeeSeq_Default, LOCAL_Foo, WithError)
#endif /* !LOCAL_DeeSeq_DefaultFooWithError */
#ifndef LOCAL_DeeSeq_DefaultFooForEmpty
#define LOCAL_DeeSeq_DefaultFooForEmpty LOCAL_DeeSeq_DefaultFooWithError
#endif /* !LOCAL_DeeSeq_DefaultFooForEmpty */


#define LOCAL_CANONICAL_NAME_LENGTHOF    COMPILER_STRLEN(PP_STR(LOCAL_Attr_foo))
#define LOCAL_CANONICAL_NAME_str         PP_CAT2(str_, LOCAL_Attr_foo)
#define LOCAL_CANONICAL_NAME_STR         PP_CAT2(STR_, LOCAL_Attr_foo)
#define LOCAL_CANONICAL_NAME_Dee_HashStr PP_CAT2(Dee_HashStr__, LOCAL_Attr_foo)



/* Mutable sequence functions */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) LOCAL_Dee_mh_seq_foo_t DCALL
LOCAL_DeeType_RequireSeqFoo_private_uncached(DeeTypeObject *orig_type, DeeTypeObject *self) {
	unsigned int seqclass = DeeType_GetSeqClass(self);
#ifndef LOCAL_WITHOUT_ATTRIBUTE
	struct Dee_attrinfo info;
#endif /* !LOCAL_WITHOUT_ATTRIBUTE */
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
#ifndef LOCAL_WITHOUT_ATTRIBUTE
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
		                                                &info)) {
			struct Dee_type_seq_cache *sc = DeeType_TryRequireSeqCache(self);
			if likely(sc) {
				switch (info.ai_type) {

#ifdef LOCAL_IS_GETSET
				case Dee_ATTRINFO_GETSET: {
#ifdef LOCAL_IS_GETSET_GET
					if (info.ai_value.v_getset->gs_get != &LOCAL_default_seq_foo)
						return info.ai_value.v_getset->gs_get;
#elif defined(LOCAL_IS_GETSET_BOUND)
					if (info.ai_value.v_getset->gs_bound != &LOCAL_default_seq_foo) 
						return info.ai_value.v_getset->gs_bound;
#elif defined(LOCAL_IS_GETSET_DEL)
					if (info.ai_value.v_getset->gs_del != &LOCAL_default_seq_foo)
						return info.ai_value.v_getset->gs_del;
#elif defined(LOCAL_IS_GETSET_SET)
					if (info.ai_value.v_getset->gs_set != &LOCAL_default_seq_foo)
						return info.ai_value.v_getset->gs_set;
#endif /* ... */
					return &LOCAL_DeeSeq_DefaultFooForEmpty;
				}	break;

				case Dee_ATTRINFO_ATTR:
#if defined(LOCAL_IS_GETSET_GET) || defined(LOCAL_IS_GETSET_BOUND)
					if ((info.ai_value.v_attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FGETSET | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) ==
					    /*                                */ (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FGETSET | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM))
#else /* LOCAL_IS_GETSET_GET || LOCAL_IS_GETSET_BOUND */
					if ((info.ai_value.v_attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FGETSET | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM | Dee_CLASS_ATTRIBUTE_FREADONLY)) ==
					    /*                                */ (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FGETSET | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM))
#endif /* 1LOCAL_IS_GETSET_GET && !LOCAL_IS_GETSET_BOUND */
					{
						struct class_desc *desc = DeeClass_DESC(self);
						uint16_t id = info.ai_value.v_attr->ca_addr;
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
							if unlikely(atomic_cmpxch(&sc->LOCAL_tsc_seq_foo_data.d_function, NULL, callback))
								Dee_Decref(callback);
							return &LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction;
						}
					}
					break;
#else /* LOCAL_IS_GETSET */

				case Dee_ATTRINFO_METHOD:
					if ((Dee_funptr_t)info.ai_value.v_method->m_func == (Dee_funptr_t)&LOCAL_default_seq_foo) {
#ifdef LOCAL_TMH
						LOCAL_Dee_mh_seq_foo_t hint;
						hint = (LOCAL_Dee_mh_seq_foo_t)DeeType_GetPrivateMethodHint((DeeTypeObject *)info.ai_decl, LOCAL_TMH);
						if (hint)
							return hint;
#endif /* LOCAL_TMH */
						return &LOCAL_DeeSeq_DefaultFooForEmpty;
					}
					atomic_write(&sc->LOCAL_tsc_seq_foo_data.d_method, info.ai_value.v_method->m_func);
					if (info.ai_value.v_method->m_flag & Dee_TYPE_METHOD_FKWDS)
						return &LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod;
					return &LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod;

				case Dee_ATTRINFO_ATTR:
					if ((info.ai_value.v_attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) ==
					    /*                                */ (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) {
						struct class_desc *desc = DeeClass_DESC(self);
						uint16_t id = info.ai_value.v_attr->ca_addr;
						DREF DeeObject *callback;
						Dee_class_desc_lock_read(desc);
						callback = desc->cd_members[id];
						Dee_XIncref(callback);
						Dee_class_desc_lock_endread(desc);
						if likely(callback) {
							if unlikely(atomic_cmpxch(&sc->LOCAL_tsc_seq_foo_data.d_function, NULL, callback))
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
#endif /* !LOCAL_WITHOUT_ATTRIBUTE */

#ifdef DEFINE_DeeType_RequireSeqTryGetFirst
	{
		Dee_mh_seq_getfirst_t tsc_seq_getfirst;
		tsc_seq_getfirst = DeeType_RequireSeqGetFirst_private_uncached(orig_type, self);
		if (tsc_seq_getfirst == &DeeSeq_DefaultGetFirstWithGetItemIndex) {
			if (!DeeType_IsDefaultTryGetItemIndex(self->tp_seq->tp_trygetitem_index))
				return &DeeSeq_DefaultTryGetFirstWithTryGetItemIndex;
			if (DeeType_HasOperator(self, OPERATOR_SIZE)) {
				if (self->tp_seq->tp_getitem_index_fast)
					return &DeeSeq_DefaultTryGetFirstWithSizeAndGetItemIndexFast;
				return &DeeSeq_DefaultTryGetFirstWithSizeAndGetItemIndex;
			}
			return &DeeSeq_DefaultTryGetFirstWithTryGetItemIndex;
		} else if (tsc_seq_getfirst == &DeeSeq_DefaultGetFirstWithGetItem) {
			if (!DeeType_IsDefaultTryGetItem(self->tp_seq->tp_trygetitem))
				return &DeeSeq_DefaultTryGetFirstWithTryGetItem;
			if (DeeType_HasOperator(self, OPERATOR_SIZE))
				return &DeeSeq_DefaultTryGetFirstWithSizeAndGetItem;
			return &DeeSeq_DefaultTryGetFirstWithTryGetItem;
		} else if (tsc_seq_getfirst == &DeeSeq_DefaultGetFirstWithForeach) {
			return &DeeSeq_DefaultTryGetFirstWithForeach;
		} else if (tsc_seq_getfirst == &DeeSeq_DefaultGetFirstWithError) {
			return &DeeSeq_DefaultTryGetFirstWithError;
		} else if (tsc_seq_getfirst != NULL) {
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
		Dee_mh_set_remove_t tsc_set_remove;
		tsc_set_remove = DeeType_RequireSetRemove_private_uncached(orig_type, self);
		if (tsc_set_remove != NULL &&
		    tsc_set_remove != &DeeSet_DefaultRemoveWithError) {
			Dee_mh_seq_trygetfirst_t tsc_seq_trygetfirst;
			tsc_seq_trygetfirst = DeeType_RequireSeqTryGetFirst_private_uncached(orig_type, self);
			if (tsc_seq_trygetfirst != NULL &&
			    tsc_seq_trygetfirst != &DeeSeq_DefaultTryGetFirstWithError)
				return &DeeSeq_DefaultDelFirstWithSeqGetFirstAndSetRemove;
		}
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM)) {
			Dee_mh_seq_trygetfirst_t tsc_seq_trygetfirst;
			tsc_seq_trygetfirst = DeeType_RequireSeqTryGetFirst_private_uncached(orig_type, self);
			if (tsc_seq_trygetfirst != NULL &&
			    tsc_seq_trygetfirst != &DeeSeq_DefaultTryGetFirstWithError)
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
		Dee_mh_seq_getlast_t tsc_seq_getlast;
		tsc_seq_getlast = DeeType_RequireSeqGetLast_private_uncached(orig_type, self);
		if (tsc_seq_getlast == &DeeSeq_DefaultGetLastWithSizeAndGetItemIndexFast) {
			return &DeeSeq_DefaultTryGetLastWithSizeAndGetItemIndexFast;
		} else if (tsc_seq_getlast == &DeeSeq_DefaultGetLastWithSizeAndGetItemIndex) {
			return &DeeSeq_DefaultTryGetLastWithSizeAndGetItemIndex;
		} else if (tsc_seq_getlast == &DeeSeq_DefaultGetLastWithSizeAndGetItem) {
			return &DeeSeq_DefaultTryGetLastWithSizeAndGetItem;
		} else if (tsc_seq_getlast == &DeeSeq_DefaultGetLastWithForeach) {
			return &DeeSeq_DefaultTryGetLastWithForeach;
		} else if (tsc_seq_getlast == &DeeSeq_DefaultGetLastWithError) {
			return &DeeSeq_DefaultTryGetLastWithError;
		} else if (tsc_seq_getlast != NULL) {
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
		Dee_mh_set_remove_t tsc_set_remove;
		tsc_set_remove = DeeType_RequireSetRemove_private_uncached(orig_type, self);
		if (tsc_set_remove != NULL &&
		    tsc_set_remove != &DeeSet_DefaultRemoveWithError) {
			Dee_mh_seq_trygetlast_t tsc_seq_trygetlast;
			tsc_seq_trygetlast = DeeType_RequireSeqTryGetLast_private_uncached(orig_type, self);
			if (tsc_seq_trygetlast != NULL &&
			    tsc_seq_trygetlast != &DeeSeq_DefaultTryGetLastWithError)
				return &DeeSeq_DefaultDelLastWithSeqGetLastAndSetRemove;
		}
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM)) {
			Dee_mh_seq_trygetlast_t tsc_seq_trygetlast;
			tsc_seq_trygetlast = DeeType_RequireSeqTryGetLast_private_uncached(orig_type, self);
			if (tsc_seq_trygetlast != NULL &&
			    tsc_seq_trygetlast != &DeeSeq_DefaultTryGetLastWithError)
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
	{
		Dee_mh_seq_count_t mh_seq_count;
		mh_seq_count = DeeType_RequireSeqCount_private_uncached(orig_type, self);
		if (mh_seq_count != NULL &&
		    mh_seq_count != &DeeSeq_DefaultCountWithSeqForeach)
			return &DeeSeq_DefaultParityWithSeqCount;
	}
#elif defined(DEFINE_DeeType_RequireSeqParityWithKey)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqParityWithRange)
	{
		Dee_mh_seq_count_with_range_t mh_seq_count_with_range;
		mh_seq_count_with_range = DeeType_RequireSeqCountWithRange_private_uncached(orig_type, self);
		if (mh_seq_count_with_range != NULL &&
		    mh_seq_count_with_range != &DeeSeq_DefaultCountWithRangeWithSeqEnumerateIndex)
			return &DeeSeq_DefaultParityWithRangeWithSeqCountWithRange;
	}
#elif defined(DEFINE_DeeType_RequireSeqParityWithRangeAndKey)
	/* ... */
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
	{
		Dee_mh_seq_find_t mh_seq_find;
		mh_seq_find = DeeType_RequireSeqFind_private_uncached(orig_type, self);
		if (mh_seq_find != NULL &&
		    mh_seq_find != &DeeSeq_DefaultFindWithSeqEnumerateIndex)
			return &DeeSeq_DefaultCountWithSeqFind;
	}
#elif defined(DEFINE_DeeType_RequireSeqCountWithKey)
	{
		Dee_mh_seq_find_with_key_t mh_seq_find_with_key;
		mh_seq_find_with_key = DeeType_RequireSeqFindWithKey_private_uncached(orig_type, self);
		if (mh_seq_find_with_key != NULL &&
		    mh_seq_find_with_key != &DeeSeq_DefaultFindWithKeyWithSeqEnumerateIndex)
			return &DeeSeq_DefaultCountWithKeyWithSeqFindWithKey;
	}
#elif defined(DEFINE_DeeType_RequireSeqCountWithRange)
	{
		Dee_mh_seq_find_t mh_seq_find;
		mh_seq_find = DeeType_RequireSeqFind_private_uncached(orig_type, self);
		if (mh_seq_find != NULL &&
		    mh_seq_find != &DeeSeq_DefaultFindWithSeqEnumerateIndex)
			return &DeeSeq_DefaultCountWithRangeWithSeqFind;
	}
#elif defined(DEFINE_DeeType_RequireSeqCountWithRangeAndKey)
	{
		Dee_mh_seq_find_with_key_t mh_seq_find_with_key;
		mh_seq_find_with_key = DeeType_RequireSeqFindWithKey_private_uncached(orig_type, self);
		if (mh_seq_find_with_key != NULL &&
		    mh_seq_find_with_key != &DeeSeq_DefaultFindWithKeyWithSeqEnumerateIndex)
			return &DeeSeq_DefaultCountWithRangeAndKeyWithSeqFindWithKey;
	}
#elif defined(DEFINE_DeeType_RequireSeqContains)
	if (seqclass == Dee_SEQCLASS_SEQ || seqclass == Dee_SEQCLASS_SET) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_CONTAINS))
			return &DeeSeq_DefaultContainsWithContains;
	}
	{
		Dee_mh_seq_find_t mh_seq_find;
		mh_seq_find = DeeType_RequireSeqFind_private_uncached(orig_type, self);
		if (mh_seq_find != NULL &&
		    mh_seq_find != &DeeSeq_DefaultFindWithSeqEnumerateIndex)
			return &DeeSeq_DefaultContainsWithSeqFind;
	}
	if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_ITER))
		return &DeeSeq_DefaultContainsWithForeach;
#elif defined(DEFINE_DeeType_RequireSeqContainsWithKey)
	{
		Dee_mh_seq_find_with_key_t mh_seq_find_with_key;
		mh_seq_find_with_key = DeeType_RequireSeqFindWithKey_private_uncached(orig_type, self);
		if (mh_seq_find_with_key != NULL &&
		    mh_seq_find_with_key != &DeeSeq_DefaultFindWithKeyWithSeqEnumerateIndex)
			return &DeeSeq_DefaultContainsWithKeyWithSeqFindWithKey;
	}
#elif defined(DEFINE_DeeType_RequireSeqContainsWithRange)
	{
		Dee_mh_seq_find_t mh_seq_find;
		mh_seq_find = DeeType_RequireSeqFind_private_uncached(orig_type, self);
		if (mh_seq_find != NULL &&
		    mh_seq_find != &DeeSeq_DefaultFindWithSeqEnumerateIndex)
			return &DeeSeq_DefaultContainsWithRangeWithSeqFind;
	}
#elif defined(DEFINE_DeeType_RequireSeqContainsWithRangeAndKey)
	{
		Dee_mh_seq_find_with_key_t mh_seq_find_with_key;
		mh_seq_find_with_key = DeeType_RequireSeqFindWithKey_private_uncached(orig_type, self);
		if (mh_seq_find_with_key != NULL &&
		    mh_seq_find_with_key != &DeeSeq_DefaultFindWithKeyWithSeqEnumerateIndex)
			return &DeeSeq_DefaultContainsWithRangeAndKeyWithSeqFindWithKey;
	}
#elif defined(DEFINE_DeeType_RequireSeqLocate)
	if (seqclass == Dee_SEQCLASS_SEQ && DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
		Dee_mh_seq_find_t mh_seq_find;
		mh_seq_find = DeeType_RequireSeqFind_private_uncached(orig_type, self);
		if (mh_seq_find != NULL &&
		    mh_seq_find != &DeeSeq_DefaultFindWithSeqEnumerateIndex)
			return &DeeSeq_DefaultLocateWithSeqFindAndSeqGetItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqLocateWithKey)
	if (seqclass == Dee_SEQCLASS_SEQ && DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
		Dee_mh_seq_find_with_key_t mh_seq_find_with_key;
		mh_seq_find_with_key = DeeType_RequireSeqFindWithKey_private_uncached(orig_type, self);
		if (mh_seq_find_with_key != NULL &&
		    mh_seq_find_with_key != &DeeSeq_DefaultFindWithKeyWithSeqEnumerateIndex)
			return &DeeSeq_DefaultLocateWithKeyWithSeqFindWithKeyAndSeqGetItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqLocateWithRange)
	if (seqclass == Dee_SEQCLASS_SEQ && DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
		Dee_mh_seq_find_t mh_seq_find;
		mh_seq_find = DeeType_RequireSeqFind_private_uncached(orig_type, self);
		if (mh_seq_find != NULL &&
		    mh_seq_find != &DeeSeq_DefaultFindWithSeqEnumerateIndex)
			return &DeeSeq_DefaultLocateWithRangeWithSeqFindAndSeqGetItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqLocateWithRangeAndKey)
	if (seqclass == Dee_SEQCLASS_SEQ && DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
		Dee_mh_seq_find_with_key_t mh_seq_find_with_key;
		mh_seq_find_with_key = DeeType_RequireSeqFindWithKey_private_uncached(orig_type, self);
		if (mh_seq_find_with_key != NULL &&
		    mh_seq_find_with_key != &DeeSeq_DefaultFindWithKeyWithSeqEnumerateIndex)
			return &DeeSeq_DefaultLocateWithRangeAndKeyWithSeqFindWithKeyAndSeqGetItemIndex;
	}
#elif defined(DEFINE_DeeType_RequireSeqRLocateWithRange)
	if (seqclass == Dee_SEQCLASS_SEQ && DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
		Dee_mh_seq_rfind_t mh_seq_rfind;
		mh_seq_rfind = DeeType_RequireSeqRFind_private_uncached(orig_type, self);
		if (mh_seq_rfind != NULL &&
		    mh_seq_rfind != &DeeSeq_DefaultRFindWithSeqEnumerateIndex &&
		    mh_seq_rfind != &DeeSeq_DefaultRFindWithSeqEnumerateIndexReverse)
			return &DeeSeq_DefaultRLocateWithRangeWithSeqRFindAndSeqGetItemIndex;
	}
	if (DeeType_HasPrivateSeqEnumerateIndexReverse(orig_type, self))
		return &DeeSeq_DefaultRLocateWithRangeWithSeqEnumerateIndexReverse;
#elif defined(DEFINE_DeeType_RequireSeqRLocateWithRangeAndKey)
	if (seqclass == Dee_SEQCLASS_SEQ && DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
		Dee_mh_seq_rfind_with_key_t mh_seq_rfind_with_key;
		mh_seq_rfind_with_key = DeeType_RequireSeqFindWithKey_private_uncached(orig_type, self);
		if (mh_seq_rfind_with_key != NULL &&
		    mh_seq_rfind_with_key != &DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndex &&
		    mh_seq_rfind_with_key != &DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndexReverse)
			return &DeeSeq_DefaultRLocateWithRangeAndKeyWithSeqRFindWithKeyAndSeqGetItemIndex;
	}
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
			Dee_mh_seq_pop_t tsc_seq_pop;
			tsc_seq_pop = DeeType_RequireSeqPop_private_uncached(orig_type, self);
			if (tsc_seq_pop != NULL &&
			    tsc_seq_pop != &DeeSeq_DefaultPopWithError)
				return &DeeSeq_DefaultEraseWithPop;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqInsert)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		Dee_mh_seq_insertall_t tsc_seq_insertall;
		tsc_seq_insertall = DeeType_RequireSeqInsertAll_private_uncached(orig_type, self);
		if (tsc_seq_insertall != NULL &&
		    tsc_seq_insertall != &DeeSeq_DefaultInsertAllWithError)
			return &DeeSeq_DefaultInsertWithSeqInsertAll;
	}
#elif defined(DEFINE_DeeType_RequireSeqInsertAll)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETRANGE) &&
		    self->tp_seq->tp_setrange_index != &DeeSeq_DefaultSetRangeIndexWithSizeDefaultAndTSCEraseAndTSCInsertAll)
			return &DeeSeq_DefaultInsertAllWithSetRangeIndex;
		if (DeeType_HasPrivateCustomSeqInsert(self))
			return &DeeSeq_DefaultInsertAllWithSeqInsert;
	}
#elif defined(DEFINE_DeeType_RequireSeqPushFront)
	/* ... */
#elif defined(DEFINE_DeeType_RequireSeqAppend)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (seqclass == Dee_SEQCLASS_SEQ) {
			/* Check for "pushback()" */
			if (DeeObject_TFindPrivateAttrInfoStringLenHash(self, NULL, STR_pushback, 8,
			                                                Dee_HashStr__pushback, &info)) {
				struct Dee_type_seq_cache *sc = DeeType_TryRequireSeqCache(self);
				if likely(sc) {
					switch (info.ai_type) {
					case Dee_ATTRINFO_METHOD:
						if ((Dee_funptr_t)info.ai_value.v_method->m_func == (Dee_funptr_t)&LOCAL_default_seq_foo)
							return &LOCAL_DeeSeq_DefaultFooForEmpty;
						atomic_write(&sc->LOCAL_tsc_seq_foo_data.d_method, info.ai_value.v_method->m_func);
						if (info.ai_value.v_method->m_flag & Dee_TYPE_METHOD_FKWDS)
							return &LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod;
						return &LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod;
					case Dee_ATTRINFO_ATTR:
						ASSERT(info.ai_type == Dee_ATTRINFO_ATTR);
						if ((info.ai_value.v_attr->ca_flag & (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) ==
						    /*                                */ (Dee_CLASS_ATTRIBUTE_FMETHOD | Dee_CLASS_ATTRIBUTE_FREADONLY | Dee_CLASS_ATTRIBUTE_FCLASSMEM)) {
							struct class_desc *desc = DeeClass_DESC(self);
							uint16_t id = info.ai_value.v_attr->ca_addr;
							DREF DeeObject *callback;
							Dee_class_desc_lock_read(desc);
							callback = desc->cd_members[id];
							Dee_XIncref(callback);
							Dee_class_desc_lock_endread(desc);
							if likely(callback) {
								if unlikely(atomic_cmpxch(&sc->LOCAL_tsc_seq_foo_data.d_function, NULL, callback))
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
			Dee_mh_seq_extend_t tsc_seq_extend;
			tsc_seq_extend = DeeType_RequireSeqExtend_private_uncached(orig_type, self);
			if (tsc_seq_extend != NULL &&
			    tsc_seq_extend != &DeeSeq_DefaultExtendWithError) {
				if (tsc_seq_extend == &DeeSeq_DefaultExtendWithSizeAndSeqInsertAll)
					return &DeeSeq_DefaultAppendWithSizeAndSeqInsert;;
				return &DeeSeq_DefaultAppendWithSeqExtend;
			}
		}
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
			Dee_mh_seq_insert_t tsc_seq_insert;
			tsc_seq_insert = DeeType_RequireSeqInsert_private_uncached(orig_type, self);
			if (tsc_seq_insert != NULL &&
			    tsc_seq_insert != &DeeSeq_DefaultInsertWithError)
				return &DeeSeq_DefaultAppendWithSizeAndSeqInsert;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqExtend)
	if (seqclass != Dee_SEQCLASS_SET && seqclass != Dee_SEQCLASS_MAP) {
		if (DeeType_HasOperator(orig_type, OPERATOR_ITER)) {
			if (DeeType_HasPrivateCustomSeqAppend(self))
				return &DeeSeq_DefaultExtendWithSeqAppend;
			if (DeeType_HasPrivateCustomSeqPushBack(self))
				return &DeeSeq_DefaultExtendWithSeqAppend;
		}
		if (seqclass == Dee_SEQCLASS_SEQ &&
		    DeeType_HasOperator(orig_type, OPERATOR_SIZE)) {
			Dee_mh_seq_insertall_t tsc_seq_insertall;
			tsc_seq_insertall = DeeType_RequireSeqInsertAll_private_uncached(orig_type, self);
			if (tsc_seq_insertall != NULL &&
			    tsc_seq_insertall != &DeeSeq_DefaultInsertAllWithError)
				return &DeeSeq_DefaultExtendWithSizeAndSeqInsertAll;
		}
		if (DeeType_HasOperator(orig_type, OPERATOR_ITER)) {
			Dee_mh_seq_insert_t tsc_seq_insert;
			tsc_seq_insert = DeeType_RequireSeqInsert_private_uncached(orig_type, self);
			if (tsc_seq_insert != NULL &&
				tsc_seq_insert != &DeeSeq_DefaultInsertWithError)
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
		Dee_mh_set_removeall_t tsc_set_removeall;
		tsc_set_removeall = DeeType_RequireSetRemoveAll_private_uncached(orig_type, self);
		if (tsc_set_removeall != NULL &&
		    tsc_set_removeall != &DeeSet_DefaultRemoveAllWithError)
			return &DeeSeq_DefaultClearWithSeqRemoveAll;
	}	break;

	case Dee_SEQCLASS_MAP: {
		Dee_mh_map_removekeys_t tsc_map_removekeys;
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
			Dee_mh_seq_erase_t tsc_seq_erase;
			tsc_seq_erase = DeeType_RequireSeqErase_private_uncached(orig_type, self);
			if (tsc_seq_erase != NULL &&
			    tsc_seq_erase != &DeeSeq_DefaultEraseWithError)
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
			if (DeeType_HasPrivateCustomSeqErase(self))
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
			Dee_mh_seq_removeall_t tsc_seq_removeall;
			tsc_seq_removeall = DeeType_RequireSeqRemoveAll_private_uncached(orig_type, self);
			if (tsc_seq_removeall != NULL &&
			    tsc_seq_removeall != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveWithSeqRemoveAll;
		}
		{
			Dee_mh_seq_removeif_t tsc_seq_removeif;
			tsc_seq_removeif = DeeType_RequireSeqRemoveIf_private_uncached(orig_type, self);
			if (tsc_seq_removeif != NULL &&
			    tsc_seq_removeif != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveWithSeqRemoveIf;
		}
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM)) {
			Dee_mh_seq_find_t tsc_seq_find;
			tsc_seq_find = DeeType_RequireSeqFind_private_uncached(orig_type, self);
			if (tsc_seq_find == &DeeSeq_DefaultFindWithSeqEnumerateIndex)
				return &DeeSeq_DefaultRemoveWithSeqEnumerateIndexAndDelItemIndex;
			if (tsc_seq_find)
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
			Dee_mh_seq_removeall_with_key_t tsc_seq_removeall_with_key;
			tsc_seq_removeall_with_key = DeeType_RequireSeqRemoveAllWithKey_private_uncached(orig_type, self);
			if (tsc_seq_removeall_with_key != NULL &&
			    tsc_seq_removeall_with_key != &DeeSeq_DefaultRemoveAllWithKeyWithError)
				return &DeeSeq_DefaultRemoveWithKeyWithSeqRemoveAllWithKey;
		}
		{
			Dee_mh_seq_removeif_t tsc_seq_removeif;
			tsc_seq_removeif = DeeType_RequireSeqRemoveIf_private_uncached(orig_type, self);
			if (tsc_seq_removeif != NULL &&
			    tsc_seq_removeif != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveWithKeyWithSeqRemoveIf;
		}
		if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_DELITEM)) {
			Dee_mh_seq_find_with_key_t tsc_seq_find_with_key;
			tsc_seq_find_with_key = DeeType_RequireSeqFindWithKey_private_uncached(orig_type, self);
			if (tsc_seq_find_with_key == &DeeSeq_DefaultFindWithKeyWithSeqEnumerateIndex)
				return &DeeSeq_DefaultRemoveWithKeyWithSeqEnumerateIndexAndDelItemIndex;
			if (tsc_seq_find_with_key)
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
			Dee_mh_seq_rfind_t tsc_seq_rfind;
			tsc_seq_rfind = DeeType_RequireSeqRFind_private_uncached(orig_type, self);
			if (tsc_seq_rfind == &DeeSeq_DefaultRFindWithSeqEnumerateIndexReverse)
				return &DeeSeq_DefaultRRemoveWithSeqEnumerateIndexReverseAndDelItemIndex;
			if (tsc_seq_rfind == &DeeSeq_DefaultRFindWithSeqEnumerateIndex)
				return &DeeSeq_DefaultRRemoveWithSeqEnumerateIndexAndDelItemIndex;
			if (tsc_seq_rfind)
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
			Dee_mh_seq_rfind_with_key_t tsc_seq_rfind_with_key;
			tsc_seq_rfind_with_key = DeeType_RequireSeqRFindWithKey_private_uncached(orig_type, self);
			if (tsc_seq_rfind_with_key == &DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndexReverse)
				return &DeeSeq_DefaultRRemoveWithKeyWithSeqEnumerateIndexReverseAndDelItemIndex;
			if (tsc_seq_rfind_with_key == &DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndex)
				return &DeeSeq_DefaultRRemoveWithKeyWithSeqEnumerateIndexAndDelItemIndex;
			if (tsc_seq_rfind_with_key)
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
			Dee_mh_seq_removeif_t tsc_seq_removeif;
			tsc_seq_removeif = DeeType_RequireSeqRemoveIf_private_uncached(orig_type, self);
			if (tsc_seq_removeif != NULL &&
			    tsc_seq_removeif != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveAllWithSeqRemoveIf;
		}
		if (DeeType_HasPrivateCustomSeqRemove(self))
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
			Dee_mh_seq_removeif_t tsc_seq_removeif;
			tsc_seq_removeif = DeeType_RequireSeqRemoveIf_private_uncached(orig_type, self);
			if (tsc_seq_removeif != NULL &&
			    tsc_seq_removeif != &DeeSeq_DefaultRemoveAllWithError)
				return &DeeSeq_DefaultRemoveAllWithKeyWithSeqRemoveIf;
		}
		if (DeeType_HasPrivateCustomSeqRemove(self))
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
		if (DeeType_HasPrivateCustomSeqRemoveAll(self))
			return &DeeSeq_DefaultRemoveIfWithSeqRemoveAllWithKey;
		if (DeeType_HasPrivateCustomSeqRemove(self))
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
		Dee_mh_seq_brange_t tsc_seq_brange;
		tsc_seq_brange = DeeType_RequireSeqBRange_private_uncached(orig_type, self);
		if (tsc_seq_brange != NULL &&
		    tsc_seq_brange != &DeeSeq_DefaultBRangeWithError) {
			if (tsc_seq_brange == &DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBFindWithSeqBRange;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqBFindWithKey)
	{
		Dee_mh_seq_brange_with_key_t tsc_seq_brange_with_key;
		tsc_seq_brange_with_key = DeeType_RequireSeqBRangeWithKey_private_uncached(orig_type, self);
		if (tsc_seq_brange_with_key != NULL &&
		    tsc_seq_brange_with_key != &DeeSeq_DefaultBRangeWithKeyWithError) {
			if (tsc_seq_brange_with_key == &DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBFindWithKeyWithSeqBRangeWithKey;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqBPosition)
	{
		Dee_mh_seq_brange_t tsc_seq_brange;
		tsc_seq_brange = DeeType_RequireSeqBRange_private_uncached(orig_type, self);
		if (tsc_seq_brange != NULL &&
		    tsc_seq_brange != &DeeSeq_DefaultBRangeWithError) {
			if (tsc_seq_brange == &DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBPositionWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBPositionWithSeqBRange;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqBPositionWithKey)
	{
		Dee_mh_seq_brange_with_key_t tsc_seq_brange_with_key;
		tsc_seq_brange_with_key = DeeType_RequireSeqBRangeWithKey_private_uncached(orig_type, self);
		if (tsc_seq_brange_with_key != NULL &&
		    tsc_seq_brange_with_key != &DeeSeq_DefaultBRangeWithKeyWithError) {
			if (tsc_seq_brange_with_key == &DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex)
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
		Dee_mh_seq_bfind_t tsc_seq_bfind;
		tsc_seq_bfind = DeeType_RequireSeqBFind_private_uncached(orig_type, self);
		if (tsc_seq_bfind != NULL &&
		    tsc_seq_bfind != &DeeSeq_DefaultBFindWithError) {
			if (tsc_seq_bfind == &DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBLocateWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBLocateWithSeqBFindAndGetItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_RequireSeqBLocateWithKey)
	{
		Dee_mh_seq_bfind_with_key_t tsc_seq_bfind_with_key;
		tsc_seq_bfind_with_key = DeeType_RequireSeqBFindWithKey_private_uncached(orig_type, self);
		if (tsc_seq_bfind_with_key != NULL &&
		    tsc_seq_bfind_with_key != &DeeSeq_DefaultBFindWithKeyWithError) {
			if (tsc_seq_bfind_with_key == &DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultBLocateWithKeyWithSizeAndTryGetItemIndex;
			return &DeeSeq_DefaultBLocateWithKeyWithSeqBFindWithKeyAndGetItemIndex;
		}
	}
#elif defined(DEFINE_DeeType_RequireSetInsert)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* use insertall and "operator #" before/after */
		if (DeeType_HasOperator(orig_type, OPERATOR_SIZE) &&
		    DeeType_HasPrivateCustomSetInsertAll(self))
			return &DeeSet_DefaultInsertWithSeqSizeAndSetInsertAll;
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* >> local x = Dict();
		 * >> (x as Set).insert(("foo", "bar")); // x.setnew("foo", "bar");
		 * >> print repr x; // {"foo":"bar"} */
		Dee_mh_map_setnew_t tsc_map_setnew;
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
		Dee_mh_seq_append_t tsc_append;
		tsc_append = DeeType_RequireSeqAppend_private_uncached(orig_type, self);
		if (tsc_append != NULL &&
		    tsc_append != &DeeSeq_DefaultAppendWithError)
			return &DeeSet_DefaultInsertWithSeqSeqContainsAndSeqAppend;
	}
#elif defined(DEFINE_DeeType_RequireSetRemove)
	if (seqclass == Dee_SEQCLASS_SET) {
		/* use removeall and "operator #" before/after */
		if (DeeType_HasOperator(orig_type, OPERATOR_SIZE) &&
		    DeeType_HasPrivateCustomSetRemoveAll(self))
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
		Dee_mh_seq_remove_t tsc_remove;
		tsc_remove = DeeType_RequireSeqRemove_private_uncached(orig_type, self);
		if (tsc_remove != NULL &&
		    tsc_remove != &DeeSeq_DefaultRemoveWithError)
			return &DeeSet_DefaultRemoveWithSeqRemove;
	}
#elif defined(DEFINE_DeeType_RequireSetUnify)
	if (seqclass == Dee_SEQCLASS_SET) {
		Dee_mh_set_insert_t tsc_set_insert;
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
		Dee_mh_seq_append_t tsc_append;
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
		Dee_mh_set_insert_t tsc_set_insert;
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
		Dee_mh_set_remove_t tsc_set_remove;
		tsc_set_remove = DeeType_RequireSetRemove_private_uncached(orig_type, self);
		if (tsc_set_remove != NULL &&
		    tsc_set_remove != &DeeSet_DefaultRemoveWithError)
			return &DeeSet_DefaultRemoveAllWithSetRemove;
	}
#elif defined(DEFINE_DeeType_RequireSetPop)
	if (seqclass == Dee_SEQCLASS_SET) {
		Dee_mh_set_remove_t tsc_set_remove;
		tsc_set_remove = DeeType_RequireSetRemove_private_uncached(orig_type, self);
		if (tsc_set_remove != NULL &&
		    tsc_set_remove != &DeeSet_DefaultRemoveWithError)
			return &DeeSet_DefaultPopWithSetFirstAndSetRemove;
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* >> local x = Dict(("foo", "bar"));
		 * >> print repr((x as Set).pop()); // ("foo", "bar")
		 * >> print repr x; // {} */
		Dee_mh_map_popitem_t tsc_map_popitem;
		tsc_map_popitem = DeeType_RequireMapPopItem_private_uncached(orig_type, self);
		if (tsc_map_popitem != NULL &&
		    tsc_map_popitem != &DeeMap_DefaultPopItemWithError)
			return &DeeSet_DefaultPopWithMapPopItem;
	} else {
		/* >> local x = [10, 20, 30];
		 * >> print repr((x as Set).pop()); // 30
		 * >> print repr x;                 // [10, 20] */
		Dee_mh_seq_pop_t tsc_seq_pop;
		tsc_seq_pop = DeeType_RequireSeqPop_private_uncached(orig_type, self);
		if (tsc_seq_pop != NULL &&
		    tsc_seq_pop != &DeeSeq_DefaultPopWithError)
			return &DeeSet_DefaultPopWithSeqPop;
	}
#elif defined(DEFINE_DeeType_RequireSetPopWithDefault)
	if (seqclass == Dee_SEQCLASS_SET) {
		Dee_mh_set_remove_t tsc_set_remove;
		tsc_set_remove = DeeType_RequireSetRemove_private_uncached(orig_type, self);
		if (tsc_set_remove != NULL &&
		    tsc_set_remove != &DeeSet_DefaultRemoveWithError)
			return &DeeSet_DefaultPopWithDefaultWithSeqTryGetFirstAndSetRemove;
	} else if (seqclass == Dee_SEQCLASS_MAP) {
		/* >> local x = Dict(("foo", "bar"));
		 * >> print repr((x as Set).pop(("foo", "ignored"))); // ("foo", "bar")
		 * >> print repr x; // {} */
		Dee_mh_map_popitem_t tsc_map_popitem;
		tsc_map_popitem = DeeType_RequireMapPopItem_private_uncached(orig_type, self);
		if (tsc_map_popitem != NULL &&
		    tsc_map_popitem != &DeeMap_DefaultPopItemWithError)
			return &DeeSet_DefaultPopWithDefaultWithMapPopItem;
	} else {
		/* >> local x = [10, 20, 30];
		 * >> print repr((x as Set).pop()); // 30
		 * >> print repr x;                 // [10, 20] */
		Dee_mh_seq_pop_t tsc_seq_pop;
		tsc_seq_pop = DeeType_RequireSeqPop_private_uncached(orig_type, self);
		if (tsc_seq_pop != NULL &&
		    tsc_seq_pop != &DeeSeq_DefaultPopWithError)
			return &DeeSet_DefaultPopWithDefaultWithSeqPop;
	}
#elif defined(DEFINE_DeeType_RequireMapSetOld)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateCustomMapSetOldEx(self))
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
		if (DeeType_HasPrivateCustomMapSetNewEx(self))
			return &DeeMap_DefaultSetNewWithMapSetNewEx;
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			struct type_seq *seq = orig_type->tp_seq;
			ASSERT(seq->tp_bounditem);
			ASSERT(seq->tp_trygetitem);
			ASSERT(seq->tp_getitem);
			if (DeeType_HasPrivateCustomMapSetDefault(self)) {
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
			if (DeeType_HasPrivateCustomMapSetDefault(self)) {
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
		if (DeeType_HasPrivateCustomMapSetNewEx(self))
			return &DeeMap_DefaultSetDefaultWithMapSetNewEx;
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			struct type_seq *seq = orig_type->tp_seq;
			ASSERT(seq->tp_bounditem);
			ASSERT(seq->tp_trygetitem);
			ASSERT(seq->tp_getitem);
			if (DeeType_HasPrivateCustomMapSetNew(self))
				return &DeeMap_DefaultSetDefaultWithMapSetNewAndGetItem;
			if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_SETITEM)) {
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
		if (DeeType_HasPrivateCustomMapRemoveKeys(self)) {
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
		if (DeeType_HasPrivateCustomMapRemove(self))
			return &DeeMap_DefaultRemoveKeysWithMapRemove;
	} else if (seqclass == Dee_SEQCLASS_SET) {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Set.remove' */
	} else {
		/* TODO: Treat as ?S?T2?O?O and use foreach + `Sequence.erase' */
	}
#elif defined(DEFINE_DeeType_RequireMapPop)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasOperator(orig_type, OPERATOR_GETITEM)) {
			Dee_mh_map_remove_t tsc_map_remove;
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
			Dee_mh_map_remove_t tsc_map_remove;
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
		Dee_mh_map_remove_t tsc_map_remove;
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
		Dee_mh_map_iterkeys_t tsc_map_iterkeys;
		tsc_map_iterkeys = DeeType_RequireMapIterKeys_private_uncached(orig_type, self);
		if (tsc_map_iterkeys != NULL &&
		    tsc_map_iterkeys != &DeeMap_DefaultIterKeysWithError)
			return &DeeMap_DefaultKeysWithMapIterKeys;
	}
#elif defined(DEFINE_DeeType_RequireMapValues)
	{
		Dee_mh_map_itervalues_t tsc_map_itervalues;
		tsc_map_itervalues = DeeType_RequireMapIterValues_private_uncached(orig_type, self);
		if (tsc_map_itervalues != NULL &&
		    tsc_map_itervalues != &DeeMap_DefaultIterValuesWithError)
			return &DeeMap_DefaultValuesWithMapIterValues;
	}
#elif defined(DEFINE_DeeType_RequireMapIterKeys)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateCustomMapKeys(self))
			return &DeeMap_DefaultIterKeysWithMapKeys;
		if (self->tp_seq && self->tp_seq->tp_iterkeys)
			return self->tp_seq->tp_iterkeys;
	}
	if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_ITER))
		return &DeeMap_DefaultIterKeysWithIter;
#elif defined(DEFINE_DeeType_RequireMapIterValues)
	if (seqclass == Dee_SEQCLASS_MAP) {
		if (DeeType_HasPrivateCustomMapValues(self))
			return &DeeMap_DefaultIterValuesWithMapValues;
	}
	if (DeeType_HasPrivateOperator_in(orig_type, self, OPERATOR_ITER))
		return &DeeMap_DefaultIterValuesWithIter;
#endif /* ... */
	return NULL;
}

PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_mh_seq_foo_t DCALL
LOCAL_DeeType_RequireSeqFoo_uncached(DeeTypeObject *__restrict self) {
	DeeTypeObject *iter;
	DeeTypeMRO mro;
	iter = DeeTypeMRO_Init(&mro, self);
	do {
		LOCAL_Dee_mh_seq_foo_t result;
		result = LOCAL_DeeType_RequireSeqFoo_private_uncached(self, iter);
		if (result)
			return result;
	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL);
	return &LOCAL_DeeSeq_DefaultFooWithError;
}

INTERN ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_mh_seq_foo_t DCALL
LOCAL_DeeType_RequireSeqFoo(DeeTypeObject *__restrict self) {
	LOCAL_Dee_mh_seq_foo_t result;
	struct Dee_type_seq_cache *sc;
	struct Dee_type_seq *seq = self->tp_seq;
	if likely(seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->LOCAL_tsc_seq_foo)
			return sc->LOCAL_tsc_seq_foo;
	}
	result = LOCAL_DeeType_RequireSeqFoo_uncached(self);
	sc     = DeeType_TryRequireSeqCache(self);
	if likely(sc)
		atomic_write(&sc->LOCAL_tsc_seq_foo, result);
	return result;
}


#undef LOCAL_Dee_SEQCLASS
#undef LOCAL_Attr_foo
#undef LOCAL_NO_TMH
#undef LOCAL_TMH
#undef LOCAL_Foo
#undef LOCAL_foo
#undef LOCAL_method_foo
#undef LOCAL_default_seq_foo
#undef LOCAL_tsc_seq_foo
#undef LOCAL_Dee_mh_seq_foo_t
#undef LOCAL_tsc_seq_method_foo
#undef LOCAL_tsc_seq_foo_data
#undef LOCAL_DeeSeq_Default
#undef LOCAL_DeeSeq_RequireFoo
#undef LOCAL_DeeType_RequireSeqFoo_private_uncached
#undef LOCAL_DeeType_RequireSeqFoo_uncached
#undef LOCAL_DeeType_RequireSeqFoo
#undef LOCAL_Bar
#undef LOCAL_DeeSeq_DefaultFooWithCallAttrBarForSeq
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataFunctionForSeq
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataMethodForSeq
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethodForSeq
#undef LOCAL_DeeSeq_DefaultFooWithCallAttrBarForSetOrMap
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataFunctionForSetOrMap
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataMethodForSetOrMap
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethodForSetOrMap
#undef LOCAL_DeeSeq_DefaultFooWithCallAttrBar
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataFunction
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataMethod
#undef LOCAL_DeeSeq_DefaultFooWithCallBarDataKwMethod
#undef LOCAL_DeeSeq_DefaultFooWithError
#undef LOCAL_DeeSeq_DefaultFooForEmpty
#undef LOCAL_CANONICAL_NAME_LENGTHOF
#undef LOCAL_CANONICAL_NAME_str
#undef LOCAL_CANONICAL_NAME_STR
#undef LOCAL_CANONICAL_NAME_Dee_HashStr
#undef LOCAL_IS_GETSET_GET
#undef LOCAL_IS_GETSET_BOUND
#undef LOCAL_IS_GETSET_DEL
#undef LOCAL_IS_GETSET_SET
#undef LOCAL_IS_GETSET
#undef LOCAL_HAS_FOR_SEQ_SUFFIX
#undef LOCAL_WITHOUT_ATTRIBUTE
#undef LOCAL_ATTR_REQUIRED_SEQCLASS

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
