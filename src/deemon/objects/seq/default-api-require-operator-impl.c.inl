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
//#define DEFINE_DeeType_RequireSeqOperatorEnumerate
//#define DEFINE_DeeType_RequireSeqOperatorEnumerateIndex
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
#define DEFINE_DeeType_RequireSetOperatorHash
//#define DEFINE_DeeType_RequireSetOperatorCompareEq
//#define DEFINE_DeeType_RequireSetOperatorTryCompareEq
//#define DEFINE_DeeType_RequireSetOperatorEq
//#define DEFINE_DeeType_RequireSetOperatorNe
//#define DEFINE_DeeType_RequireSetOperatorLo
//#define DEFINE_DeeType_RequireSetOperatorLe
//#define DEFINE_DeeType_RequireSetOperatorGr
//#define DEFINE_DeeType_RequireSetOperatorGe
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeType_RequireSeqOperatorBool) +            \
     defined(DEFINE_DeeType_RequireSeqOperatorIter) +            \
     defined(DEFINE_DeeType_RequireSeqOperatorSizeOb) +          \
     defined(DEFINE_DeeType_RequireSeqOperatorContains) +        \
     defined(DEFINE_DeeType_RequireSeqOperatorGetItem) +         \
     defined(DEFINE_DeeType_RequireSeqOperatorDelItem) +         \
     defined(DEFINE_DeeType_RequireSeqOperatorSetItem) +         \
     defined(DEFINE_DeeType_RequireSeqOperatorGetRange) +        \
     defined(DEFINE_DeeType_RequireSeqOperatorDelRange) +        \
     defined(DEFINE_DeeType_RequireSeqOperatorSetRange) +        \
     defined(DEFINE_DeeType_RequireSeqOperatorForeach) +         \
     defined(DEFINE_DeeType_RequireSeqOperatorEnumerate) +       \
     defined(DEFINE_DeeType_RequireSeqOperatorEnumerateIndex) +  \
     defined(DEFINE_DeeType_RequireSeqOperatorBoundItem) +       \
     defined(DEFINE_DeeType_RequireSeqOperatorHasItem) +         \
     defined(DEFINE_DeeType_RequireSeqOperatorSize) +            \
     defined(DEFINE_DeeType_RequireSeqOperatorSizeFast) +        \
     defined(DEFINE_DeeType_RequireSeqOperatorGetItemIndex) +    \
     defined(DEFINE_DeeType_RequireSeqOperatorDelItemIndex) +    \
     defined(DEFINE_DeeType_RequireSeqOperatorSetItemIndex) +    \
     defined(DEFINE_DeeType_RequireSeqOperatorBoundItemIndex) +  \
     defined(DEFINE_DeeType_RequireSeqOperatorHasItemIndex) +    \
     defined(DEFINE_DeeType_RequireSeqOperatorGetRangeIndex) +   \
     defined(DEFINE_DeeType_RequireSeqOperatorDelRangeIndex) +   \
     defined(DEFINE_DeeType_RequireSeqOperatorSetRangeIndex) +   \
     defined(DEFINE_DeeType_RequireSeqOperatorGetRangeIndexN) +  \
     defined(DEFINE_DeeType_RequireSeqOperatorDelRangeIndexN) +  \
     defined(DEFINE_DeeType_RequireSeqOperatorSetRangeIndexN) +  \
     defined(DEFINE_DeeType_RequireSeqOperatorTryGetItem) +      \
     defined(DEFINE_DeeType_RequireSeqOperatorTryGetItemIndex) + \
     defined(DEFINE_DeeType_RequireSeqOperatorHash) +            \
     defined(DEFINE_DeeType_RequireSeqOperatorCompareEq) +       \
     defined(DEFINE_DeeType_RequireSeqOperatorCompare) +         \
     defined(DEFINE_DeeType_RequireSeqOperatorTryCompareEq) +    \
     defined(DEFINE_DeeType_RequireSeqOperatorEq) +              \
     defined(DEFINE_DeeType_RequireSeqOperatorNe) +              \
     defined(DEFINE_DeeType_RequireSeqOperatorLo) +              \
     defined(DEFINE_DeeType_RequireSeqOperatorLe) +              \
     defined(DEFINE_DeeType_RequireSeqOperatorGr) +              \
     defined(DEFINE_DeeType_RequireSeqOperatorGe) +              \
     defined(DEFINE_DeeType_RequireSeqOperatorInplaceAdd) +      \
     defined(DEFINE_DeeType_RequireSeqOperatorInplaceMul) +      \
     defined(DEFINE_DeeType_RequireSetOperatorHash) +            \
     defined(DEFINE_DeeType_RequireSetOperatorCompareEq) +       \
     defined(DEFINE_DeeType_RequireSetOperatorTryCompareEq) +    \
     defined(DEFINE_DeeType_RequireSetOperatorEq) +              \
     defined(DEFINE_DeeType_RequireSetOperatorNe) +              \
     defined(DEFINE_DeeType_RequireSetOperatorLo) +              \
     defined(DEFINE_DeeType_RequireSetOperatorLe) +              \
     defined(DEFINE_DeeType_RequireSetOperatorGr) +              \
     defined(DEFINE_DeeType_RequireSetOperatorGe)) != 1
#error "Must #define exactly one of these macros"
#endif /* DEFINE_DeeType_RequireSeq... */

#ifdef LOCAL_FOR_VARIANTS
#define PP_PRIVATE_CAT3(a, b, c) a##b##c
#define PP_PRIVATE_CAT2(a, b)    a##b
#define PP_CAT3(a, b, c) PP_PRIVATE_CAT3(a, b, c)
#define PP_CAT2(a, b, c) PP_PRIVATE_CAT2(a, b, c)
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
#elif defined(DEFINE_DeeType_RequireSeqOperatorEnumerate)
#define LOCAL_operator_foo     operator_enumerate
#define LOCAL_OperatorFoo      OperatorEnumerate
#elif defined(DEFINE_DeeType_RequireSeqOperatorEnumerateIndex)
#define LOCAL_operator_foo     operator_enumerate_index
#define LOCAL_OperatorFoo      OperatorEnumerateIndex
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

#ifndef LOCAL_seq_class
#if LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SEQ
#define LOCAL_seq_class seq
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_SET
#define LOCAL_seq_class set
#elif LOCAL_Dee_SEQCLASS == Dee_SEQCLASS_MAP
#define LOCAL_seq_class map
#else /* LOCAL_Dee_SEQCLASS == ... */
#define LOCAL_seq_class INVALID_LOCAL_Dee_SEQCLASS
#endif /* LOCAL_Dee_SEQCLASS != ... */
#endif /* !LOCAL_seq_class */

#ifndef LOCAL_tsc_seq_operator_foo
#define LOCAL_tsc_seq_operator_foo PP_CAT4(tsc_, LOCAL_seq_class, _, LOCAL_operator_foo)
#endif /* !LOCAL_tsc_seq_operator_foo */

#ifndef LOCAL_Dee_tsc_operator_foo_t
#define LOCAL_Dee_tsc_operator_foo_t PP_CAT3(Dee_tsc_, LOCAL_operator_foo, _t)
#endif /* !LOCAL_Dee_tsc_operator_foo_t */

#ifndef LOCAL_DeeSeq_OperatorFoo
#define LOCAL_DeeSeq_OperatorFoo PP_CAT2(DeeSeq_, LOCAL_OperatorFoo)
#endif /* !LOCAL_DeeSeq_OperatorFoo */

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


#ifdef LOCAL_FOR_OPTIMIZE
#define LOCAL_DeeType_RequireSeqOperatorSize            DeeType_RequireSeqOperatorSize_for_optimize
#define LOCAL_DeeType_RequireSeqOperatorForeach         DeeType_RequireSeqOperatorForeach_for_optimize
#define LOCAL_DeeType_RequireSeqOperatorGetItemIndex    DeeType_RequireSeqOperatorGetItemIndex_for_optimize
#define LOCAL_DeeType_RequireSeqOperatorIter            DeeType_RequireSeqOperatorIter_for_optimize
#define LOCAL_DeeType_RequireSeqOperatorBoundItemIndex  DeeType_RequireSeqOperatorBoundItemIndex_for_optimize
#define LOCAL_DeeType_RequireSeqOperatorHasItemIndex    DeeType_RequireSeqOperatorHasItemIndex_for_optimize
#define LOCAL_DeeType_RequireSeqOperatorTryGetItemIndex DeeType_RequireSeqOperatorTryGetItemIndex_for_optimize
#define LOCAL_DeeType_RequireSeqOperatorCompareEq       DeeType_RequireSeqOperatorCompareEq_for_optimize
#define LOCAL_DeeType_RequireSeqOperatorCompare         DeeType_RequireSeqOperatorCompare_for_optimize
#define LOCAL_DeeType_RequireSetOperatorCompareEq       DeeType_RequireSetOperatorCompareEq_for_optimize
#else /* LOCAL_FOR_OPTIMIZE */
#define LOCAL_DeeType_RequireSeqOperatorSize            DeeType_RequireSeqOperatorSize
#define LOCAL_DeeType_RequireSeqOperatorForeach         DeeType_RequireSeqOperatorForeach
#define LOCAL_DeeType_RequireSeqOperatorGetItemIndex    DeeType_RequireSeqOperatorGetItemIndex
#define LOCAL_DeeType_RequireSeqOperatorIter            DeeType_RequireSeqOperatorIter
#define LOCAL_DeeType_RequireSeqOperatorBoundItemIndex  DeeType_RequireSeqOperatorBoundItemIndex
#define LOCAL_DeeType_RequireSeqOperatorHasItemIndex    DeeType_RequireSeqOperatorHasItemIndex
#define LOCAL_DeeType_RequireSeqOperatorTryGetItemIndex DeeType_RequireSeqOperatorTryGetItemIndex
#define LOCAL_DeeType_RequireSeqOperatorCompareEq       DeeType_RequireSeqOperatorCompareEq
#define LOCAL_DeeType_RequireSeqOperatorCompare         DeeType_RequireSeqOperatorCompare
#define LOCAL_DeeType_RequireSetOperatorCompareEq       DeeType_RequireSetOperatorCompareEq
#endif /* !LOCAL_FOR_OPTIMIZE */


PRIVATE ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_tsc_operator_foo_t DCALL
LOCAL_DeeType_RequireSeqOperatorFoo_uncached(DeeTypeObject *__restrict self) {
	(void)self;
#ifdef DEFINE_DeeType_RequireSeqOperatorBool
	if (DeeType_GetSeqClass(self) != Dee_SEQCLASS_NONE) {
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
		if (DeeType_RequireBool(self))
			return self->tp_cast.tp_bool;
#endif /* !LOCAL_FOR_OPTIMIZE */
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
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
		Dee_tsc_operator_size_t tsc_seq_operator_size;
		tsc_seq_operator_size = LOCAL_DeeType_RequireSeqOperatorSize(self);
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
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
		Dee_tsc_operator_getitem_index_t tsc_seq_operator_getitem_index;
		tsc_seq_operator_getitem_index = LOCAL_DeeType_RequireSeqOperatorGetItemIndex(self);
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
		Dee_tsc_operator_iter_t tsc_seq_operator_iter;
		tsc_seq_operator_iter = LOCAL_DeeType_RequireSeqOperatorIter(self);
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
#elif defined(DEFINE_DeeType_RequireSeqOperatorEnumerate)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerate(self))
		return self->tp_seq->tp_enumerate;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorEnumerateWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultEnumerateWithCounterAndForeach;
	}
#elif defined(DEFINE_DeeType_RequireSeqOperatorEnumerateIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerateIndex(self))
		return self->tp_seq->tp_enumerate_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
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
		Dee_tsc_operator_bounditem_index_t tsc_seq_operator_bounditem_index;
		tsc_seq_operator_bounditem_index = LOCAL_DeeType_RequireSeqOperatorBoundItemIndex(self);
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
		Dee_tsc_operator_hasitem_index_t tsc_seq_operator_hasitem_index;
		tsc_seq_operator_hasitem_index = LOCAL_DeeType_RequireSeqOperatorHasItemIndex(self);
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorSizeWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError) {
			if (!DeeType_IsDefaultForeachPair(self->tp_seq->tp_foreach_pair))
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
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
		Dee_tsc_operator_size_t tsc_seq_operator_size;
		tsc_seq_operator_size = LOCAL_DeeType_RequireSeqOperatorSize(self);
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
		Dee_tsc_operator_size_t tsc_seq_operator_size;
		tsc_seq_operator_size = LOCAL_DeeType_RequireSeqOperatorSize(self);
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
		Dee_tsc_operator_iter_t tsc_seq_operator_iter;
		tsc_seq_operator_iter = LOCAL_DeeType_RequireSeqOperatorIter(self);
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
		Dee_tsc_operator_iter_t tsc_seq_operator_iter;
		tsc_seq_operator_iter = LOCAL_DeeType_RequireSeqOperatorIter(self);
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
		Dee_tsc_operator_trygetitem_index_t tsc_seq_operator_trygetitem_index;
		tsc_seq_operator_trygetitem_index = LOCAL_DeeType_RequireSeqOperatorTryGetItemIndex(self);
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
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
		Dee_tsc_operator_compare_eq_t tsc_seq_operator_compare_eq;
		tsc_seq_operator_compare_eq = LOCAL_DeeType_RequireSeqOperatorCompareEq(self);
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
		Dee_tsc_operator_compare_eq_t tsc_seq_operator_compare_eq;
		tsc_seq_operator_compare_eq = LOCAL_DeeType_RequireSeqOperatorCompareEq(self);
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
		Dee_tsc_operator_compare_t tsc_seq_operator_compare;
		tsc_seq_operator_compare = LOCAL_DeeType_RequireSeqOperatorCompare(self);
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
		Dee_tsc_operator_compare_t tsc_seq_operator_compare;
		tsc_seq_operator_compare = LOCAL_DeeType_RequireSeqOperatorCompare(self);
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
		Dee_tsc_operator_compare_t tsc_seq_operator_compare;
		tsc_seq_operator_compare = LOCAL_DeeType_RequireSeqOperatorCompare(self);
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
		Dee_tsc_operator_compare_t tsc_seq_operator_compare;
		tsc_seq_operator_compare = LOCAL_DeeType_RequireSeqOperatorCompare(self);
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
		Dee_tsc_seq_extend_t tsc_seq_extend;
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
		Dee_tsc_seq_extend_t tsc_seq_extend;
		Dee_tsc_seq_clear_t tsc_seq_clear;
		if ((tsc_seq_extend = DeeType_RequireSeqExtend(self)) != &DeeSeq_DefaultExtendWithError &&
		    (tsc_seq_clear = DeeType_RequireSeqClear(self)) != &DeeSeq_DefaultClearWithError)
			return &DeeSeq_DefaultOperatorInplaceMulWithTSCClearAndTSCExtend;
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorHashWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultHashWithForeachDefault;
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
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
		Dee_tsc_operator_compare_eq_t tsc_seq_operator_compare_eq;
		tsc_seq_operator_compare_eq = LOCAL_DeeType_RequireSetOperatorCompareEq(self);
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
		Dee_tsc_operator_compare_eq_t tsc_seq_operator_compare_eq;
		tsc_seq_operator_compare_eq = LOCAL_DeeType_RequireSetOperatorCompareEq(self);
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
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
		Dee_tsc_operator_foreach_t tsc_seq_operator_foreach;
		tsc_seq_operator_foreach = LOCAL_DeeType_RequireSeqOperatorForeach(self);
		if (tsc_seq_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSet_DefaultOperatorGeWithEmpty;
		if (tsc_seq_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSet_DefaultGeWithForeachDefault;
	}
#endif /* ... */
	return &LOCAL_DeeSeq_DefaultOperatorFooWithError;
}

#undef LOCAL_DeeType_RequireSeqOperatorSize
#undef LOCAL_DeeType_RequireSeqOperatorForeach
#undef LOCAL_DeeType_RequireSeqOperatorGetItemIndex
#undef LOCAL_DeeType_RequireSeqOperatorIter
#undef LOCAL_DeeType_RequireSeqOperatorBoundItemIndex
#undef LOCAL_DeeType_RequireSeqOperatorHasItemIndex
#undef LOCAL_DeeType_RequireSeqOperatorTryGetItemIndex
#undef LOCAL_DeeType_RequireSeqOperatorCompareEq
#undef LOCAL_DeeType_RequireSeqOperatorCompare
#undef LOCAL_DeeType_RequireSetOperatorCompareEq


#ifndef LOCAL_FOR_OPTIMIZE
INTERN ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_tsc_operator_foo_t DCALL
LOCAL_DeeType_RequireSeqOperatorFoo(DeeTypeObject *__restrict self) {
	LOCAL_Dee_tsc_operator_foo_t result;
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
#undef LOCAL_Dee_tsc_operator_foo_t
#undef LOCAL_tsc_seq_operator_foo
#undef LOCAL_seq_class
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
#undef DEFINE_DeeType_RequireSeqOperatorEnumerate
#undef DEFINE_DeeType_RequireSeqOperatorEnumerateIndex
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
#undef DEFINE_DeeType_RequireSetOperatorHash
#undef DEFINE_DeeType_RequireSetOperatorCompareEq
#undef DEFINE_DeeType_RequireSetOperatorTryCompareEq
#undef DEFINE_DeeType_RequireSetOperatorEq
#undef DEFINE_DeeType_RequireSetOperatorNe
#undef DEFINE_DeeType_RequireSetOperatorLo
#undef DEFINE_DeeType_RequireSetOperatorLe
#undef DEFINE_DeeType_RequireSetOperatorGr
#undef DEFINE_DeeType_RequireSetOperatorGe
