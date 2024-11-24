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
#define DEFINE_DeeType_SeqCache_RequireOperatorBool
//#define DEFINE_DeeType_SeqCache_RequireOperatorIter
//#define DEFINE_DeeType_SeqCache_RequireOperatorSizeOb
//#define DEFINE_DeeType_SeqCache_RequireOperatorContains
//#define DEFINE_DeeType_SeqCache_RequireOperatorGetItem
//#define DEFINE_DeeType_SeqCache_RequireOperatorDelItem
//#define DEFINE_DeeType_SeqCache_RequireOperatorSetItem
//#define DEFINE_DeeType_SeqCache_RequireOperatorGetRange
//#define DEFINE_DeeType_SeqCache_RequireOperatorDelRange
//#define DEFINE_DeeType_SeqCache_RequireOperatorSetRange
//#define DEFINE_DeeType_SeqCache_RequireOperatorForeach
//#define DEFINE_DeeType_SeqCache_RequireOperatorEnumerate
//#define DEFINE_DeeType_SeqCache_RequireOperatorEnumerateIndex
//#define DEFINE_DeeType_SeqCache_RequireOperatorBoundItem
//#define DEFINE_DeeType_SeqCache_RequireOperatorHasItem
//#define DEFINE_DeeType_SeqCache_RequireOperatorSize
//#define DEFINE_DeeType_SeqCache_RequireOperatorSizeFast
//#define DEFINE_DeeType_SeqCache_RequireOperatorGetItemIndex
//#define DEFINE_DeeType_SeqCache_RequireOperatorDelItemIndex
//#define DEFINE_DeeType_SeqCache_RequireOperatorSetItemIndex
//#define DEFINE_DeeType_SeqCache_RequireOperatorBoundItemIndex
//#define DEFINE_DeeType_SeqCache_RequireOperatorHasItemIndex
//#define DEFINE_DeeType_SeqCache_RequireOperatorGetRangeIndex
//#define DEFINE_DeeType_SeqCache_RequireOperatorDelRangeIndex
//#define DEFINE_DeeType_SeqCache_RequireOperatorSetRangeIndex
//#define DEFINE_DeeType_SeqCache_RequireOperatorGetRangeIndexN
//#define DEFINE_DeeType_SeqCache_RequireOperatorDelRangeIndexN
//#define DEFINE_DeeType_SeqCache_RequireOperatorSetRangeIndexN
//#define DEFINE_DeeType_SeqCache_RequireOperatorTryGetItem
//#define DEFINE_DeeType_SeqCache_RequireOperatorTryGetItemIndex
//#define DEFINE_DeeType_SeqCache_RequireOperatorHash
//#define DEFINE_DeeType_SeqCache_RequireOperatorCompareEq
//#define DEFINE_DeeType_SeqCache_RequireOperatorCompare
//#define DEFINE_DeeType_SeqCache_RequireOperatorTryCompareEq
//#define DEFINE_DeeType_SeqCache_RequireOperatorEq
//#define DEFINE_DeeType_SeqCache_RequireOperatorNe
//#define DEFINE_DeeType_SeqCache_RequireOperatorLo
//#define DEFINE_DeeType_SeqCache_RequireOperatorLe
//#define DEFINE_DeeType_SeqCache_RequireOperatorGr
//#define DEFINE_DeeType_SeqCache_RequireOperatorGe
//#define DEFINE_DeeType_SeqCache_RequireOperatorInplaceAdd
//#define DEFINE_DeeType_SeqCache_RequireOperatorInplaceMul
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeType_SeqCache_RequireOperatorBool) +            \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorIter) +            \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorSizeOb) +          \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorContains) +        \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorGetItem) +         \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorDelItem) +         \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorSetItem) +         \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorGetRange) +        \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorDelRange) +        \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorSetRange) +        \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorForeach) +         \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorEnumerate) +       \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorEnumerateIndex) +  \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorBoundItem) +       \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorHasItem) +         \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorSize) +            \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorSizeFast) +        \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorGetItemIndex) +    \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorDelItemIndex) +    \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorSetItemIndex) +    \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorBoundItemIndex) +  \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorHasItemIndex) +    \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorGetRangeIndex) +   \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorDelRangeIndex) +   \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorSetRangeIndex) +   \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorGetRangeIndexN) +  \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorDelRangeIndexN) +  \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorSetRangeIndexN) +  \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorTryGetItem) +      \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorTryGetItemIndex) + \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorHash) +            \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorCompareEq) +       \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorCompare) +         \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorTryCompareEq) +    \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorEq) +              \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorNe) +              \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorLo) +              \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorLe) +              \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorGr) +              \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorGe) +              \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorInplaceAdd) +      \
     defined(DEFINE_DeeType_SeqCache_RequireOperatorInplaceMul)) != 1
#error "Must #define exactly one of these macros"
#endif /* DEFINE_DeeType_SeqCache_Require... */

#ifdef LOCAL_FOR_VARIANTS
#define PP_PRIVATE_CAT3(a, b, c) a##b##c
#define PP_PRIVATE_CAT2(a, b)    a##b
#define PP_CAT3(a, b, c) PP_PRIVATE_CAT3(a, b, c)
#define PP_CAT2(a, b, c) PP_PRIVATE_CAT2(a, b, c)
#define LOCAL_FOR_OPTIMIZE
#include "default-api.h"
#endif /* LOCAL_FOR_VARIANTS */


DECL_BEGIN

#ifdef DEFINE_DeeType_SeqCache_RequireOperatorBool
#define LOCAL_tsc_operator_foo tsc_operator_bool
#define LOCAL_OperatorFoo      OperatorBool
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorIter)
#define LOCAL_tsc_operator_foo tsc_operator_iter
#define LOCAL_OperatorFoo      OperatorIter
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSizeOb)
#define LOCAL_tsc_operator_foo tsc_operator_sizeob
#define LOCAL_OperatorFoo      OperatorSizeOb
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorContains)
#define LOCAL_tsc_operator_foo tsc_operator_contains
#define LOCAL_OperatorFoo      OperatorContains
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorGetItem)
#define LOCAL_tsc_operator_foo tsc_operator_getitem
#define LOCAL_OperatorFoo      OperatorGetItem
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorDelItem)
#define LOCAL_tsc_operator_foo tsc_operator_delitem
#define LOCAL_OperatorFoo      OperatorDelItem
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSetItem)
#define LOCAL_tsc_operator_foo tsc_operator_setitem
#define LOCAL_OperatorFoo      OperatorSetItem
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorGetRange)
#define LOCAL_tsc_operator_foo tsc_operator_getrange
#define LOCAL_OperatorFoo      OperatorGetRange
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorDelRange)
#define LOCAL_tsc_operator_foo tsc_operator_delrange
#define LOCAL_OperatorFoo      OperatorDelRange
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSetRange)
#define LOCAL_tsc_operator_foo tsc_operator_setrange
#define LOCAL_OperatorFoo      OperatorSetRange
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorForeach)
#define LOCAL_tsc_operator_foo tsc_operator_foreach
#define LOCAL_OperatorFoo      OperatorForeach
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorEnumerate)
#define LOCAL_tsc_operator_foo tsc_operator_enumerate
#define LOCAL_OperatorFoo      OperatorEnumerate
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorEnumerateIndex)
#define LOCAL_tsc_operator_foo tsc_operator_enumerate_index
#define LOCAL_OperatorFoo      OperatorEnumerateIndex
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorBoundItem)
#define LOCAL_tsc_operator_foo tsc_operator_bounditem
#define LOCAL_OperatorFoo      OperatorBoundItem
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorHasItem)
#define LOCAL_tsc_operator_foo tsc_operator_hasitem
#define LOCAL_OperatorFoo      OperatorHasItem
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSize)
#define LOCAL_tsc_operator_foo tsc_operator_size
#define LOCAL_OperatorFoo      OperatorSize
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSizeFast)
#define LOCAL_tsc_operator_foo tsc_operator_size_fast
#define LOCAL_OperatorFoo      OperatorSizeFast
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorGetItemIndex)
#define LOCAL_tsc_operator_foo tsc_operator_getitem_index
#define LOCAL_OperatorFoo      OperatorGetItemIndex
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorDelItemIndex)
#define LOCAL_tsc_operator_foo tsc_operator_delitem_index
#define LOCAL_OperatorFoo      OperatorDelItemIndex
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSetItemIndex)
#define LOCAL_tsc_operator_foo tsc_operator_setitem_index
#define LOCAL_OperatorFoo      OperatorSetItemIndex
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorBoundItemIndex)
#define LOCAL_tsc_operator_foo tsc_operator_bounditem_index
#define LOCAL_OperatorFoo      OperatorBoundItemIndex
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorHasItemIndex)
#define LOCAL_tsc_operator_foo tsc_operator_hasitem_index
#define LOCAL_OperatorFoo      OperatorHasItemIndex
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorGetRangeIndex)
#define LOCAL_tsc_operator_foo tsc_operator_getrange_index
#define LOCAL_OperatorFoo      OperatorGetRangeIndex
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorDelRangeIndex)
#define LOCAL_tsc_operator_foo tsc_operator_delrange_index
#define LOCAL_OperatorFoo      OperatorDelRangeIndex
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSetRangeIndex)
#define LOCAL_tsc_operator_foo tsc_operator_setrange_index
#define LOCAL_OperatorFoo      OperatorSetRangeIndex
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorGetRangeIndexN)
#define LOCAL_tsc_operator_foo tsc_operator_getrange_index_n
#define LOCAL_OperatorFoo      OperatorGetRangeIndexN
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorDelRangeIndexN)
#define LOCAL_tsc_operator_foo tsc_operator_delrange_index_n
#define LOCAL_OperatorFoo      OperatorDelRangeIndexN
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSetRangeIndexN)
#define LOCAL_tsc_operator_foo tsc_operator_setrange_index_n
#define LOCAL_OperatorFoo      OperatorSetRangeIndexN
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorTryGetItem)
#define LOCAL_tsc_operator_foo tsc_operator_trygetitem
#define LOCAL_OperatorFoo      OperatorTryGetItem
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorTryGetItemIndex)
#define LOCAL_tsc_operator_foo tsc_operator_trygetitem_index
#define LOCAL_OperatorFoo      OperatorTryGetItemIndex
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorHash)
#define LOCAL_tsc_operator_foo tsc_operator_hash
#define LOCAL_OperatorFoo      OperatorHash
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorCompareEq)
#define LOCAL_tsc_operator_foo tsc_operator_compare_eq
#define LOCAL_OperatorFoo      OperatorCompareEq
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorCompare)
#define LOCAL_tsc_operator_foo tsc_operator_compare
#define LOCAL_OperatorFoo      OperatorCompare
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorTryCompareEq)
#define LOCAL_tsc_operator_foo tsc_operator_trycompare_eq
#define LOCAL_OperatorFoo      OperatorTryCompareEq
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorEq)
#define LOCAL_tsc_operator_foo tsc_operator_eq
#define LOCAL_OperatorFoo      OperatorEq
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorNe)
#define LOCAL_tsc_operator_foo tsc_operator_ne
#define LOCAL_OperatorFoo      OperatorNe
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorLo)
#define LOCAL_tsc_operator_foo tsc_operator_lo
#define LOCAL_OperatorFoo      OperatorLo
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorLe)
#define LOCAL_tsc_operator_foo tsc_operator_le
#define LOCAL_OperatorFoo      OperatorLe
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorGr)
#define LOCAL_tsc_operator_foo tsc_operator_gr
#define LOCAL_OperatorFoo      OperatorGr
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorGe)
#define LOCAL_tsc_operator_foo tsc_operator_ge
#define LOCAL_OperatorFoo      OperatorGe
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorInplaceAdd)
#define LOCAL_tsc_operator_foo tsc_operator_inplace_add
#define LOCAL_OperatorFoo      OperatorInplaceAdd
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorInplaceMul)
#define LOCAL_tsc_operator_foo tsc_operator_inplace_mul
#define LOCAL_OperatorFoo      OperatorInplaceMul
#else /* DEFINE_DeeType_SeqCache_Require... */
#error "Invalid configuration"
#endif /* !DEFINE_DeeType_SeqCache_Require... */

#ifndef LOCAL_Dee_tsc_operator_foo_t
#define LOCAL_Dee_tsc_operator_foo_t PP_CAT3(Dee_, LOCAL_tsc_operator_foo, _t)
#endif /* !LOCAL_Dee_tsc_operator_foo_t */

#ifndef LOCAL_DeeSeq_OperatorFoo
#define LOCAL_DeeSeq_OperatorFoo PP_CAT2(DeeSeq_, LOCAL_OperatorFoo)
#endif /* !LOCAL_DeeSeq_OperatorFoo */

#ifndef LOCAL_DeeType_SeqCache_RequireOperatorFoo_uncached
#ifdef LOCAL_FOR_OPTIMIZE
#define LOCAL_DeeType_SeqCache_RequireOperatorFoo_uncached PP_CAT3(DeeType_SeqCache_Require, LOCAL_OperatorFoo, _for_optimize)
#else /* LOCAL_FOR_OPTIMIZE */
#define LOCAL_DeeType_SeqCache_RequireOperatorFoo_uncached PP_CAT3(DeeType_SeqCache_Require, LOCAL_OperatorFoo, _uncached)
#endif /* !LOCAL_FOR_OPTIMIZE */
#endif /* !LOCAL_DeeType_SeqCache_RequireOperatorFoo_uncached */

#ifndef LOCAL_DeeType_SeqCache_RequireOperatorFoo
#define LOCAL_DeeType_SeqCache_RequireOperatorFoo PP_CAT2(DeeType_SeqCache_Require, LOCAL_OperatorFoo)
#endif /* !LOCAL_DeeType_SeqCache_RequireOperatorFoo */

#ifndef LOCAL_DeeSeq_DefaultOperatorFooWithEmpty
#define LOCAL_DeeSeq_DefaultOperatorFooWithEmpty PP_CAT3(DeeSeq_Default, LOCAL_OperatorFoo, WithEmpty)
#endif /* !LOCAL_DeeSeq_DefaultOperatorFooWithEmpty */

#ifndef LOCAL_DeeSeq_DefaultOperatorFooWithError
#define LOCAL_DeeSeq_DefaultOperatorFooWithError PP_CAT3(DeeSeq_Default, LOCAL_OperatorFoo, WithError)
#endif /* !LOCAL_DeeSeq_DefaultOperatorFooWithError */


#ifdef LOCAL_FOR_OPTIMIZE
#define LOCAL_DeeType_SeqCache_RequireOperatorSize            DeeType_SeqCache_RequireOperatorSize_for_optimize
#define LOCAL_DeeType_SeqCache_RequireOperatorForeach         DeeType_SeqCache_RequireOperatorForeach_for_optimize
#define LOCAL_DeeType_SeqCache_RequireOperatorGetItemIndex    DeeType_SeqCache_RequireOperatorGetItemIndex_for_optimize
#define LOCAL_DeeType_SeqCache_RequireOperatorIter            DeeType_SeqCache_RequireOperatorIter_for_optimize
#define LOCAL_DeeType_SeqCache_RequireOperatorBoundItemIndex  DeeType_SeqCache_RequireOperatorBoundItemIndex_for_optimize
#define LOCAL_DeeType_SeqCache_RequireOperatorHasItemIndex    DeeType_SeqCache_RequireOperatorHasItemIndex_for_optimize
#define LOCAL_DeeType_SeqCache_RequireOperatorTryGetItemIndex DeeType_SeqCache_RequireOperatorTryGetItemIndex_for_optimize
#define LOCAL_DeeType_SeqCache_RequireOperatorCompareEq       DeeType_SeqCache_RequireOperatorCompareEq_for_optimize
#define LOCAL_DeeType_SeqCache_RequireOperatorCompare         DeeType_SeqCache_RequireOperatorCompare_for_optimize
#else /* LOCAL_FOR_OPTIMIZE */
#define LOCAL_DeeType_SeqCache_RequireOperatorSize            DeeType_SeqCache_RequireOperatorSize
#define LOCAL_DeeType_SeqCache_RequireOperatorForeach         DeeType_SeqCache_RequireOperatorForeach
#define LOCAL_DeeType_SeqCache_RequireOperatorGetItemIndex    DeeType_SeqCache_RequireOperatorGetItemIndex
#define LOCAL_DeeType_SeqCache_RequireOperatorIter            DeeType_SeqCache_RequireOperatorIter
#define LOCAL_DeeType_SeqCache_RequireOperatorBoundItemIndex  DeeType_SeqCache_RequireOperatorBoundItemIndex
#define LOCAL_DeeType_SeqCache_RequireOperatorHasItemIndex    DeeType_SeqCache_RequireOperatorHasItemIndex
#define LOCAL_DeeType_SeqCache_RequireOperatorTryGetItemIndex DeeType_SeqCache_RequireOperatorTryGetItemIndex
#define LOCAL_DeeType_SeqCache_RequireOperatorCompareEq       DeeType_SeqCache_RequireOperatorCompareEq
#define LOCAL_DeeType_SeqCache_RequireOperatorCompare         DeeType_SeqCache_RequireOperatorCompare
#endif /* !LOCAL_FOR_OPTIMIZE */


PRIVATE ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_tsc_operator_foo_t DCALL
LOCAL_DeeType_SeqCache_RequireOperatorFoo_uncached(DeeTypeObject *__restrict self) {
	(void)self;
#ifdef DEFINE_DeeType_SeqCache_RequireOperatorBool
	if (DeeType_GetSeqClass(self) != Dee_SEQCLASS_NONE) {
		Dee_tsc_operator_foreach_t tsc_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
		if (DeeType_RequireBool(self))
			return self->tp_cast.tp_bool;
#endif /* !LOCAL_FOR_OPTIMIZE */
		tsc_operator_foreach = LOCAL_DeeType_SeqCache_RequireOperatorForeach(self);
		if (tsc_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
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
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorIter)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_RequireIter(self))
		return self->tp_seq->tp_iter;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSizeOb)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) != Dee_SEQCLASS_NONE && DeeType_RequireSizeOb(self))
		return self->tp_seq->tp_sizeob;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_size_t tsc_operator_size;
		tsc_operator_size = LOCAL_DeeType_SeqCache_RequireOperatorSize(self);
		if (tsc_operator_size == &DeeSeq_DefaultOperatorSizeWithEmpty)
			return &DeeSeq_DefaultOperatorSizeObWithEmpty;
		if (tsc_operator_size != &DeeSeq_DefaultOperatorSizeWithError)
			return &DeeSeq_DefaultOperatorSizeObWithSeqOperatorSize;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorContains)
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
		Dee_tsc_operator_foreach_t tsc_operator_foreach;
		tsc_operator_foreach = LOCAL_DeeType_SeqCache_RequireOperatorForeach(self);
		if (tsc_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorContainsWithEmpty;
		if (tsc_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultContainsWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorGetItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetItem(self))
		return self->tp_seq->tp_getitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_getitem_index_t tsc_operator_getitem_index;
		tsc_operator_getitem_index = LOCAL_DeeType_SeqCache_RequireOperatorGetItemIndex(self);
		if (tsc_operator_getitem_index == &DeeSeq_DefaultOperatorGetItemIndexWithEmpty)
			return &DeeSeq_DefaultOperatorGetItemWithEmpty;
		if (tsc_operator_getitem_index != &DeeSeq_DefaultOperatorGetItemIndexWithError)
			return &DeeSeq_DefaultOperatorGetItemWithSeqGetItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorDelItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireDelItem(self))
		return self->tp_seq->tp_delitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSetItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetItem(self))
		return self->tp_seq->tp_setitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorGetRange)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetRange(self))
		return self->tp_seq->tp_getrange;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_iter_t tsc_operator_iter;
		tsc_operator_iter = LOCAL_DeeType_SeqCache_RequireOperatorIter(self);
		if (tsc_operator_iter == &DeeSeq_DefaultOperatorIterWithEmpty)
			return &DeeSeq_DefaultOperatorGetRangeWithEmpty;
		if (tsc_operator_iter != &DeeSeq_DefaultOperatorIterWithError)
			return &DeeSeq_DefaultOperatorGetRangeWithSeqGetRangeIndexAndGetRangeIndexN;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorDelRange)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireDelRange(self))
		return self->tp_seq->tp_delrange;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSetRange)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetRange(self))
		return self->tp_seq->tp_setrange;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorForeach)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_RequireForeach(self))
		return self->tp_seq->tp_foreach;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorEnumerate)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerate(self))
		return self->tp_seq->tp_enumerate;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_foreach_t tsc_operator_foreach;
		tsc_operator_foreach = LOCAL_DeeType_SeqCache_RequireOperatorForeach(self);
		if (tsc_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorEnumerateWithEmpty;
		if (tsc_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultEnumerateWithCounterAndForeach;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorEnumerateIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerateIndex(self))
		return self->tp_seq->tp_enumerate_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_foreach_t tsc_operator_foreach;
		tsc_operator_foreach = LOCAL_DeeType_SeqCache_RequireOperatorForeach(self);
		if (tsc_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorEnumerateIndexWithEmpty;
		if (tsc_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultEnumerateIndexWithCounterAndForeach;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorBoundItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireBoundItem(self))
		return self->tp_seq->tp_bounditem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_bounditem_index_t tsc_operator_bounditem_index;
		tsc_operator_bounditem_index = LOCAL_DeeType_SeqCache_RequireOperatorBoundItemIndex(self);
		if (tsc_operator_bounditem_index == &DeeSeq_DefaultOperatorBoundItemIndexWithEmpty)
			return &DeeSeq_DefaultOperatorBoundItemWithEmpty;
		if (tsc_operator_bounditem_index != &DeeSeq_DefaultOperatorBoundItemIndexWithError)
			return &DeeSeq_DefaultOperatorBoundItemWithSeqBoundItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorHasItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireHasItem(self))
		return self->tp_seq->tp_hasitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_hasitem_index_t tsc_operator_hasitem_index;
		tsc_operator_hasitem_index = LOCAL_DeeType_SeqCache_RequireOperatorHasItemIndex(self);
		if (tsc_operator_hasitem_index == &DeeSeq_DefaultOperatorHasItemIndexWithEmpty)
			return &DeeSeq_DefaultOperatorHasItemWithEmpty;
		if (tsc_operator_hasitem_index != &DeeSeq_DefaultOperatorHasItemIndexWithError)
			return &DeeSeq_DefaultOperatorHasItemWithSeqHasItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSize)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) != Dee_SEQCLASS_NONE && DeeType_RequireSize(self))
		return self->tp_seq->tp_size;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_foreach_t tsc_operator_foreach;
		tsc_operator_foreach = LOCAL_DeeType_SeqCache_RequireOperatorForeach(self);
		if (tsc_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorSizeWithEmpty;
		if (tsc_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError) {
			if (!DeeType_IsDefaultForeachPair(self->tp_seq->tp_foreach_pair))
				return &DeeSeq_DefaultSizeWithForeachPair;
			return &DeeSeq_DefaultSizeWithForeach;
		}
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSizeFast)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSizeFast(self))
		return self->tp_seq->tp_size_fast;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorGetItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetItemIndex(self))
		return self->tp_seq->tp_getitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_foreach_t tsc_operator_foreach;
		tsc_operator_foreach = LOCAL_DeeType_SeqCache_RequireOperatorForeach(self);
		if (tsc_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorGetItemIndexWithEmpty;
		if (tsc_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultGetItemIndexWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorDelItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireDelItemIndex(self))
		return self->tp_seq->tp_delitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSetItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetItemIndex(self))
		return self->tp_seq->tp_setitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorBoundItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireBoundItemIndex(self))
		return self->tp_seq->tp_bounditem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_size_t tsc_operator_size;
		tsc_operator_size = LOCAL_DeeType_SeqCache_RequireOperatorSize(self);
		if (tsc_operator_size == &DeeSeq_DefaultOperatorSizeWithEmpty)
			return &DeeSeq_DefaultOperatorBoundItemIndexWithEmpty;
		if (tsc_operator_size != &DeeSeq_DefaultOperatorSizeWithError)
			return &DeeSeq_DefaultOperatorBoundItemIndexWithSeqSize;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorHasItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireHasItemIndex(self))
		return self->tp_seq->tp_hasitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_size_t tsc_operator_size;
		tsc_operator_size = LOCAL_DeeType_SeqCache_RequireOperatorSize(self);
		if (tsc_operator_size == &DeeSeq_DefaultOperatorSizeWithEmpty)
			return &DeeSeq_DefaultOperatorHasItemIndexWithEmpty;
		if (tsc_operator_size != &DeeSeq_DefaultOperatorSizeWithError)
			return &DeeSeq_DefaultOperatorHasItemIndexWithSeqSize;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorGetRangeIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetRangeIndex(self))
		return self->tp_seq->tp_getrange_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_iter_t tsc_operator_iter;
		tsc_operator_iter = LOCAL_DeeType_SeqCache_RequireOperatorIter(self);
		if (tsc_operator_iter == &DeeSeq_DefaultOperatorIterWithEmpty)
			return &DeeSeq_DefaultOperatorGetRangeIndexWithEmpty;
		if (tsc_operator_iter != &DeeSeq_DefaultOperatorIterWithError)
			return &DeeSeq_DefaultOperatorGetRangeIndexWithIterAndSeqSize;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorDelRangeIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireDelRangeIndex(self))
		return self->tp_seq->tp_delrange_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSetRangeIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetRangeIndex(self))
		return self->tp_seq->tp_setrange_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorGetRangeIndexN)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetRangeIndexN(self))
		return self->tp_seq->tp_getrange_index_n;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_iter_t tsc_operator_iter;
		tsc_operator_iter = LOCAL_DeeType_SeqCache_RequireOperatorIter(self);
		if (tsc_operator_iter == &DeeSeq_DefaultOperatorIterWithEmpty)
			return &DeeSeq_DefaultOperatorGetRangeIndexNWithEmpty;
		if (tsc_operator_iter != &DeeSeq_DefaultOperatorIterWithError)
			return &DeeSeq_DefaultOperatorGetRangeIndexNWithIterAndSeqSize;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorDelRangeIndexN)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireDelRangeIndexN(self))
		return self->tp_seq->tp_delrange_index_n;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorSetRangeIndexN)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetRangeIndexN(self))
		return self->tp_seq->tp_setrange_index_n;
#endif /* !LOCAL_FOR_OPTIMIZE */
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorTryGetItem)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireTryGetItem(self))
		return self->tp_seq->tp_trygetitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_trygetitem_index_t tsc_operator_trygetitem_index;
		tsc_operator_trygetitem_index = LOCAL_DeeType_SeqCache_RequireOperatorTryGetItemIndex(self);
		if (tsc_operator_trygetitem_index == &DeeSeq_DefaultOperatorTryGetItemIndexWithEmpty)
			return &DeeSeq_DefaultOperatorTryGetItemWithEmpty;
		if (tsc_operator_trygetitem_index != &DeeSeq_DefaultOperatorTryGetItemIndexWithError)
			return &DeeSeq_DefaultOperatorTryGetItemWithSeqTryGetItemIndex;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorTryGetItemIndex)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireTryGetItemIndex(self))
		return self->tp_seq->tp_trygetitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_foreach_t tsc_operator_foreach;
		tsc_operator_foreach = LOCAL_DeeType_SeqCache_RequireOperatorForeach(self);
		if (tsc_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorTryGetItemIndexWithEmpty;
		if (tsc_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultTryGetItemIndexWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorHash)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireHash(self))
		return self->tp_cmp->tp_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_foreach_t tsc_operator_foreach;
		tsc_operator_foreach = LOCAL_DeeType_SeqCache_RequireOperatorForeach(self);
		if (tsc_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorHashWithEmpty;
		if (tsc_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultHashWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorCompareEq)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireCompareEq(self))
		return self->tp_cmp->tp_compare_eq;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_foreach_t tsc_operator_foreach;
		tsc_operator_foreach = LOCAL_DeeType_SeqCache_RequireOperatorForeach(self);
		if (tsc_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorCompareEqWithEmpty;
		if (tsc_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultCompareEqWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorCompare)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireCompare(self))
		return self->tp_cmp->tp_compare;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_foreach_t tsc_operator_foreach;
		tsc_operator_foreach = LOCAL_DeeType_SeqCache_RequireOperatorForeach(self);
		if (tsc_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorCompareWithEmpty;
		if (tsc_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultCompareWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorTryCompareEq)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireTryCompareEq(self))
		return self->tp_cmp->tp_trycompare_eq;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_foreach_t tsc_operator_foreach;
		tsc_operator_foreach = LOCAL_DeeType_SeqCache_RequireOperatorForeach(self);
		if (tsc_operator_foreach == &DeeSeq_DefaultOperatorForeachWithEmpty)
			return &DeeSeq_DefaultOperatorTryCompareEqWithEmpty;
		if (tsc_operator_foreach != &DeeSeq_DefaultOperatorForeachWithError)
			return &DeeSeq_DefaultTryCompareEqWithForeachDefault;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorEq)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEq(self))
		return self->tp_cmp->tp_eq;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_compare_eq_t tsc_operator_compare_eq;
		tsc_operator_compare_eq = LOCAL_DeeType_SeqCache_RequireOperatorCompareEq(self);
		if (tsc_operator_compare_eq == &DeeSeq_DefaultOperatorCompareEqWithEmpty)
			return &DeeSeq_DefaultOperatorEqWithEmpty;
		if (tsc_operator_compare_eq != &DeeSeq_DefaultOperatorCompareEqWithError)
			return &DeeSeq_DefaultOperatorEqWithSeqCompareEq;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorNe)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireNe(self))
		return self->tp_cmp->tp_ne;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_compare_eq_t tsc_operator_compare_eq;
		tsc_operator_compare_eq = LOCAL_DeeType_SeqCache_RequireOperatorCompareEq(self);
		if (tsc_operator_compare_eq == &DeeSeq_DefaultOperatorCompareEqWithEmpty)
			return &DeeSeq_DefaultOperatorNeWithEmpty;
		if (tsc_operator_compare_eq != &DeeSeq_DefaultOperatorCompareEqWithError)
			return &DeeSeq_DefaultOperatorNeWithSeqCompareEq;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorLo)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireLo(self))
		return self->tp_cmp->tp_lo;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_compare_t tsc_operator_compare;
		tsc_operator_compare = LOCAL_DeeType_SeqCache_RequireOperatorCompare(self);
		if (tsc_operator_compare == &DeeSeq_DefaultOperatorCompareWithEmpty)
			return &DeeSeq_DefaultOperatorLoWithEmpty;
		if (tsc_operator_compare != &DeeSeq_DefaultOperatorCompareWithError)
			return &DeeSeq_DefaultOperatorLoWithSeqCompare;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorLe)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireLe(self))
		return self->tp_cmp->tp_le;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_compare_t tsc_operator_compare;
		tsc_operator_compare = LOCAL_DeeType_SeqCache_RequireOperatorCompare(self);
		if (tsc_operator_compare == &DeeSeq_DefaultOperatorCompareWithEmpty)
			return &DeeSeq_DefaultOperatorLeWithEmpty;
		if (tsc_operator_compare != &DeeSeq_DefaultOperatorCompareWithError)
			return &DeeSeq_DefaultOperatorLeWithSeqCompare;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorGr)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGr(self))
		return self->tp_cmp->tp_gr;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_compare_t tsc_operator_compare;
		tsc_operator_compare = LOCAL_DeeType_SeqCache_RequireOperatorCompare(self);
		if (tsc_operator_compare == &DeeSeq_DefaultOperatorCompareWithEmpty)
			return &DeeSeq_DefaultOperatorGrWithEmpty;
		if (tsc_operator_compare != &DeeSeq_DefaultOperatorCompareWithError)
			return &DeeSeq_DefaultOperatorGrWithSeqCompare;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorGe)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGe(self))
		return self->tp_cmp->tp_ge;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_operator_compare_t tsc_operator_compare;
		tsc_operator_compare = LOCAL_DeeType_SeqCache_RequireOperatorCompare(self);
		if (tsc_operator_compare == &DeeSeq_DefaultOperatorCompareWithEmpty)
			return &DeeSeq_DefaultOperatorGeWithEmpty;
		if (tsc_operator_compare != &DeeSeq_DefaultOperatorCompareWithError)
			return &DeeSeq_DefaultOperatorGeWithSeqCompare;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorInplaceAdd)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireInplaceAdd(self))
		return self->tp_math->tp_inplace_add;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_extend_t tsc_extend;
		tsc_extend = DeeType_SeqCache_RequireExtend(self);
		if (tsc_extend != &DeeSeq_DefaultExtendWithError)
			return &DeeSeq_DefaultOperatorInplaceAddWithTSCExtend;
	}
#elif defined(DEFINE_DeeType_SeqCache_RequireOperatorInplaceMul)
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireInplaceMul(self))
		return self->tp_math->tp_inplace_mul;
#endif /* !LOCAL_FOR_OPTIMIZE */
	{
		Dee_tsc_extend_t tsc_extend;
		Dee_tsc_clear_t tsc_clear;
		if ((tsc_extend = DeeType_SeqCache_RequireExtend(self)) != &DeeSeq_DefaultExtendWithError &&
		    (tsc_clear = DeeType_SeqCache_RequireClear(self)) != &DeeSeq_DefaultClearWithError)
			return &DeeSeq_DefaultOperatorInplaceMulWithTSCClearAndTSCExtend;
	}
#endif /* ... */
	return &LOCAL_DeeSeq_DefaultOperatorFooWithError;
}

#undef LOCAL_DeeType_SeqCache_RequireOperatorSize
#undef LOCAL_DeeType_SeqCache_RequireOperatorForeach
#undef LOCAL_DeeType_SeqCache_RequireOperatorGetItemIndex
#undef LOCAL_DeeType_SeqCache_RequireOperatorIter
#undef LOCAL_DeeType_SeqCache_RequireOperatorBoundItemIndex
#undef LOCAL_DeeType_SeqCache_RequireOperatorHasItemIndex
#undef LOCAL_DeeType_SeqCache_RequireOperatorTryGetItemIndex
#undef LOCAL_DeeType_SeqCache_RequireOperatorCompareEq
#undef LOCAL_DeeType_SeqCache_RequireOperatorCompare


#ifndef LOCAL_FOR_OPTIMIZE
INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_tsc_operator_foo_t DCALL
LOCAL_DeeType_SeqCache_RequireOperatorFoo(DeeTypeObject *__restrict self) {
	LOCAL_Dee_tsc_operator_foo_t result;
	struct Dee_type_seq_cache *sc;
	struct Dee_type_seq *seq = self->tp_seq;
	if likely(seq) {
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->LOCAL_tsc_operator_foo)
			return sc->LOCAL_tsc_operator_foo;
	}
	result = LOCAL_DeeType_SeqCache_RequireOperatorFoo_uncached(self);
	if unlikely(result == &LOCAL_DeeSeq_OperatorFoo) {
		/* Prevent infinite loop. This can happen when the user explicitly
		 * inherits an operator from "Sequence" (which by itself behaves
		 * like an empty sequence), so that is the behavior that needs to
		 * be inherited here. */
		result = &LOCAL_DeeSeq_DefaultOperatorFooWithEmpty;
	}
	sc = DeeType_TryRequireSeqCache(self);
	if likely(sc)
		atomic_write(&sc->LOCAL_tsc_operator_foo, result);
	return result;
}
#endif /* !LOCAL_FOR_OPTIMIZE */

#undef LOCAL_DeeSeq_OperatorFoo
#undef LOCAL_Dee_tsc_operator_foo_t
#undef LOCAL_DeeType_SeqCache_RequireOperatorFoo_uncached
#undef LOCAL_DeeType_SeqCache_RequireOperatorFoo
#undef LOCAL_DeeSeq_DefaultOperatorFooWithEmpty
#undef LOCAL_DeeSeq_DefaultOperatorFooWithError
#undef LOCAL_tsc_operator_foo
#undef LOCAL_OperatorFoo

DECL_END

#undef DEFINE_DeeType_SeqCache_RequireOperatorBool
#undef DEFINE_DeeType_SeqCache_RequireOperatorIter
#undef DEFINE_DeeType_SeqCache_RequireOperatorSizeOb
#undef DEFINE_DeeType_SeqCache_RequireOperatorContains
#undef DEFINE_DeeType_SeqCache_RequireOperatorGetItem
#undef DEFINE_DeeType_SeqCache_RequireOperatorDelItem
#undef DEFINE_DeeType_SeqCache_RequireOperatorSetItem
#undef DEFINE_DeeType_SeqCache_RequireOperatorGetRange
#undef DEFINE_DeeType_SeqCache_RequireOperatorDelRange
#undef DEFINE_DeeType_SeqCache_RequireOperatorSetRange
#undef DEFINE_DeeType_SeqCache_RequireOperatorForeach
#undef DEFINE_DeeType_SeqCache_RequireOperatorEnumerate
#undef DEFINE_DeeType_SeqCache_RequireOperatorEnumerateIndex
#undef DEFINE_DeeType_SeqCache_RequireOperatorBoundItem
#undef DEFINE_DeeType_SeqCache_RequireOperatorHasItem
#undef DEFINE_DeeType_SeqCache_RequireOperatorSize
#undef DEFINE_DeeType_SeqCache_RequireOperatorSizeFast
#undef DEFINE_DeeType_SeqCache_RequireOperatorGetItemIndex
#undef DEFINE_DeeType_SeqCache_RequireOperatorDelItemIndex
#undef DEFINE_DeeType_SeqCache_RequireOperatorSetItemIndex
#undef DEFINE_DeeType_SeqCache_RequireOperatorBoundItemIndex
#undef DEFINE_DeeType_SeqCache_RequireOperatorHasItemIndex
#undef DEFINE_DeeType_SeqCache_RequireOperatorGetRangeIndex
#undef DEFINE_DeeType_SeqCache_RequireOperatorDelRangeIndex
#undef DEFINE_DeeType_SeqCache_RequireOperatorSetRangeIndex
#undef DEFINE_DeeType_SeqCache_RequireOperatorGetRangeIndexN
#undef DEFINE_DeeType_SeqCache_RequireOperatorDelRangeIndexN
#undef DEFINE_DeeType_SeqCache_RequireOperatorSetRangeIndexN
#undef DEFINE_DeeType_SeqCache_RequireOperatorTryGetItem
#undef DEFINE_DeeType_SeqCache_RequireOperatorTryGetItemIndex
#undef DEFINE_DeeType_SeqCache_RequireOperatorHash
#undef DEFINE_DeeType_SeqCache_RequireOperatorCompareEq
#undef DEFINE_DeeType_SeqCache_RequireOperatorCompare
#undef DEFINE_DeeType_SeqCache_RequireOperatorTryCompareEq
#undef DEFINE_DeeType_SeqCache_RequireOperatorEq
#undef DEFINE_DeeType_SeqCache_RequireOperatorNe
#undef DEFINE_DeeType_SeqCache_RequireOperatorLo
#undef DEFINE_DeeType_SeqCache_RequireOperatorLe
#undef DEFINE_DeeType_SeqCache_RequireOperatorGr
#undef DEFINE_DeeType_SeqCache_RequireOperatorGe
#undef DEFINE_DeeType_SeqCache_RequireOperatorInplaceAdd
#undef DEFINE_DeeType_SeqCache_RequireOperatorInplaceMul
