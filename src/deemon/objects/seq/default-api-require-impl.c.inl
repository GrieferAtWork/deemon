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
//#define DEFINE_DeeType_SeqCache_RequireGetX
//#define DEFINE_DeeType_SeqCache_RequireBoundX
#define DEFINE_DeeType_SeqCache_RequireDelX
//#define DEFINE_DeeType_SeqCache_RequireSetX
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_DeeType_SeqCache_RequireGetX) +   \
     defined(DEFINE_DeeType_SeqCache_RequireBoundX) + \
     defined(DEFINE_DeeType_SeqCache_RequireDelX) +   \
     defined(DEFINE_DeeType_SeqCache_RequireSetX)) != 1
#error "Must #define exactly one of these"
#endif /* DEFINE_* */

DECL_BEGIN

#if defined(DEFINE_DeeType_SeqCache_RequireGetX)
#define LOCAL_IS_GET
#define LOCAL_DeeType_SeqCache_RequireXFirst DeeType_SeqCache_RequireGetFirst
#define LOCAL_DeeType_SeqCache_RequireXLast  DeeType_SeqCache_RequireGetLast
#elif defined(DEFINE_DeeType_SeqCache_RequireBoundX)
#define LOCAL_IS_BOUND
#define LOCAL_DeeType_SeqCache_RequireXFirst DeeType_SeqCache_RequireBoundFirst
#define LOCAL_DeeType_SeqCache_RequireXLast  DeeType_SeqCache_RequireBoundLast
#elif defined(DEFINE_DeeType_SeqCache_RequireDelX)
#define LOCAL_IS_DEL
#define LOCAL_DeeType_SeqCache_RequireXFirst DeeType_SeqCache_RequireDelFirst
#define LOCAL_DeeType_SeqCache_RequireXLast  DeeType_SeqCache_RequireDelLast
#elif defined(DEFINE_DeeType_SeqCache_RequireSetX)
#define LOCAL_IS_SET
#define LOCAL_DeeType_SeqCache_RequireXFirst DeeType_SeqCache_RequireSetFirst
#define LOCAL_DeeType_SeqCache_RequireXLast  DeeType_SeqCache_RequireSetLast
#else /* ... */
#error "Unsupported configuration"
#endif /* !... */


#ifdef LOCAL_IS_GET
#define LOCAL_gs_X                                           gs_get
#define LOCAL_tsc_Xfirst                                     tsc_getfirst
#define LOCAL_tsc_Xlast                                      tsc_getlast
#define LOCAL_Dee_tsc_Xfirst_t                               Dee_tsc_getfirst_t
#define LOCAL_Dee_tsc_Xlast_t                                Dee_tsc_getlast_t
#define LOCAL_default_seq_Xfirst                             default_seq_getfirst
#define LOCAL_default_seq_Xlast                              default_seq_getlast
#define LOCAL_DeeSeq_DefaultXFirstWithXAttr                  DeeSeq_DefaultGetFirstWithGetAttr
#define LOCAL_DeeSeq_DefaultXFirstWithXItem                  DeeSeq_DefaultGetFirstWithGetItem
#define LOCAL_DeeSeq_DefaultXFirstWithXItemIndex             DeeSeq_DefaultGetFirstWithGetItemIndex
#define LOCAL_DeeSeq_DefaultXFirstWithForeachDefault         DeeSeq_DefaultGetFirstWithForeachDefault
#define LOCAL_DeeSeq_DefaultXLastWithXAttr                   DeeSeq_DefaultGetLastWithGetAttr
#define LOCAL_DeeSeq_DefaultXLastWithSizeObAndXItem          DeeSeq_DefaultGetLastWithSizeObAndGetItem
#define LOCAL_DeeSeq_DefaultXLastWithSizeAndGetItemIndexFast DeeSeq_DefaultGetLastWithSizeAndGetItemIndexFast
#define LOCAL_DeeSeq_DefaultXLastWithSizeAndXItemIndex       DeeSeq_DefaultGetLastWithSizeAndGetItemIndex
#define LOCAL_DeeSeq_DefaultXLastWithForeachDefault          DeeSeq_DefaultGetLastWithForeachDefault
#define LOCAL_Dee_type_seq_has_custom_tp_Xitem_index         Dee_type_seq_has_custom_tp_getitem_index
#define LOCAL_DeeType_RequireXItem                           DeeType_RequireGetItem
#elif defined(LOCAL_IS_BOUND)
#define LOCAL_gs_X                                           gs_bound
#define LOCAL_tsc_Xfirst                                     tsc_boundfirst
#define LOCAL_tsc_Xlast                                      tsc_boundlast
#define LOCAL_Dee_tsc_Xfirst_t                               Dee_tsc_boundfirst_t
#define LOCAL_Dee_tsc_Xlast_t                                Dee_tsc_boundlast_t
#define LOCAL_default_seq_Xfirst                             default_seq_boundfirst
#define LOCAL_default_seq_Xlast                              default_seq_boundlast
#define LOCAL_DeeSeq_DefaultXFirstWithXAttr                  DeeSeq_DefaultBoundFirstWithBoundAttr
#define LOCAL_DeeSeq_DefaultXFirstWithXItem                  DeeSeq_DefaultBoundFirstWithBoundItem
#define LOCAL_DeeSeq_DefaultXFirstWithXItemIndex             DeeSeq_DefaultBoundFirstWithBoundItemIndex
#define LOCAL_DeeSeq_DefaultXFirstWithForeachDefault         DeeSeq_DefaultBoundFirstWithForeachDefault
#define LOCAL_DeeSeq_DefaultXLastWithXAttr                   DeeSeq_DefaultBoundLastWithBoundAttr
#define LOCAL_DeeSeq_DefaultXLastWithSizeObAndXItem          DeeSeq_DefaultBoundLastWithSizeObAndBoundItem
#define LOCAL_DeeSeq_DefaultXLastWithSizeAndGetItemIndexFast DeeSeq_DefaultBoundLastWithSizeAndGetItemIndexFast
#define LOCAL_DeeSeq_DefaultXLastWithSizeAndXItemIndex       DeeSeq_DefaultBoundLastWithSizeAndBoundItemIndex
#define LOCAL_DeeSeq_DefaultXLastWithForeachDefault          DeeSeq_DefaultBoundLastWithForeachDefault
#define LOCAL_Dee_type_seq_has_custom_tp_Xitem_index         Dee_type_seq_has_custom_tp_bounditem_index
#define LOCAL_DeeType_RequireXItem                           DeeType_RequireBoundItem
#elif defined(LOCAL_IS_DEL)
#define LOCAL_gs_X                                     gs_del
#define LOCAL_tsc_Xfirst                               tsc_delfirst
#define LOCAL_tsc_Xlast                                tsc_dellast
#define LOCAL_Dee_tsc_Xfirst_t                         Dee_tsc_delfirst_t
#define LOCAL_Dee_tsc_Xlast_t                          Dee_tsc_dellast_t
#define LOCAL_default_seq_Xfirst                       default_seq_delfirst
#define LOCAL_default_seq_Xlast                        default_seq_dellast
#define LOCAL_DeeSeq_DefaultXFirstWithXAttr            DeeSeq_DefaultDelFirstWithDelAttr
#define LOCAL_DeeSeq_DefaultXFirstWithXItem            DeeSeq_DefaultDelFirstWithDelItem
#define LOCAL_DeeSeq_DefaultXFirstWithXItemIndex       DeeSeq_DefaultDelFirstWithDelItemIndex
#define LOCAL_DeeSeq_DefaultXFirstWithForeachDefault   DeeSeq_DefaultDelFirstWithError
#define LOCAL_DeeSeq_DefaultXLastWithXAttr             DeeSeq_DefaultDelLastWithDelAttr
#define LOCAL_DeeSeq_DefaultXLastWithSizeObAndXItem    DeeSeq_DefaultDelLastWithSizeObAndDelItem
#define LOCAL_DeeSeq_DefaultXLastWithSizeAndXItemIndex DeeSeq_DefaultDelLastWithSizeAndDelItemIndex
#define LOCAL_DeeSeq_DefaultXLastWithForeachDefault    DeeSeq_DefaultDelLastWithError
#define LOCAL_Dee_type_seq_has_custom_tp_Xitem_index   Dee_type_seq_has_custom_tp_delitem_index
#define LOCAL_DeeType_RequireXItem                     DeeType_RequireDelItem
#elif defined(LOCAL_IS_SET)
#define LOCAL_gs_X                                     gs_set
#define LOCAL_tsc_Xfirst                               tsc_setfirst
#define LOCAL_tsc_Xlast                                tsc_setlast
#define LOCAL_Dee_tsc_Xfirst_t                         Dee_tsc_setfirst_t
#define LOCAL_Dee_tsc_Xlast_t                          Dee_tsc_setlast_t
#define LOCAL_default_seq_Xfirst                       default_seq_setfirst
#define LOCAL_default_seq_Xlast                        default_seq_setlast
#define LOCAL_DeeSeq_DefaultXFirstWithXAttr            DeeSeq_DefaultSetFirstWithSetAttr
#define LOCAL_DeeSeq_DefaultXFirstWithXItem            DeeSeq_DefaultSetFirstWithSetItem
#define LOCAL_DeeSeq_DefaultXFirstWithXItemIndex       DeeSeq_DefaultSetFirstWithSetItemIndex
#define LOCAL_DeeSeq_DefaultXFirstWithForeachDefault   DeeSeq_DefaultSetFirstWithError
#define LOCAL_DeeSeq_DefaultXLastWithXAttr             DeeSeq_DefaultSetLastWithSetAttr
#define LOCAL_DeeSeq_DefaultXLastWithSizeObAndXItem    DeeSeq_DefaultSetLastWithSizeObAndSetItem
#define LOCAL_DeeSeq_DefaultXLastWithSizeAndXItemIndex DeeSeq_DefaultSetLastWithSizeAndSetItemIndex
#define LOCAL_DeeSeq_DefaultXLastWithForeachDefault    DeeSeq_DefaultSetLastWithError
#define LOCAL_Dee_type_seq_has_custom_tp_Xitem_index   Dee_type_seq_has_custom_tp_setitem_index
#define LOCAL_DeeType_RequireXItem                     DeeType_RequireSetItem
#endif /* !... */

/* Type sequence operator definition functions. */
INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_tsc_Xfirst_t DCALL
LOCAL_DeeType_SeqCache_RequireXFirst(DeeTypeObject *__restrict self) {
	struct Dee_type_seq *seq = self->tp_seq;
	if likely(seq) {
		struct Dee_type_seq_cache *sc;
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->LOCAL_tsc_Xfirst)
			return sc->LOCAL_tsc_Xfirst;
	}
	switch (DeeType_GetSeqClass(self)) {

	case Dee_SEQCLASS_SEQ: {
		struct Dee_type_seq_cache *sc;
		LOCAL_Dee_tsc_Xfirst_t result = &LOCAL_DeeSeq_DefaultXFirstWithForeachDefault;
		struct Dee_attrinfo attr;
		if (DeeObject_TFindAttrInfo(self, NULL, (DeeObject *)&str_first, &attr)) {
			if (attr.ai_type == Dee_ATTRINFO_GETSET) {
				if (attr.ai_value.v_getset->LOCAL_gs_X == &LOCAL_default_seq_Xfirst ||
				    attr.ai_value.v_getset->LOCAL_gs_X == &LOCAL_DeeSeq_DefaultXFirstWithXAttr ||
				    attr.ai_value.v_getset->LOCAL_gs_X == &LOCAL_DeeSeq_DefaultXFirstWithXItem ||
				    attr.ai_value.v_getset->LOCAL_gs_X == &LOCAL_DeeSeq_DefaultXFirstWithXItemIndex ||
				    attr.ai_value.v_getset->LOCAL_gs_X == &LOCAL_DeeSeq_DefaultXFirstWithForeachDefault)
					goto detect_for_sequence_type;
				result = attr.ai_value.v_getset->LOCAL_gs_X; /* Directly forward attribute */
			} else {
				result = &LOCAL_DeeSeq_DefaultXFirstWithXAttr;
			}
		} else {
detect_for_sequence_type:
			if (DeeType_RequireSize(self) && LOCAL_DeeType_RequireXItem(self)) {
				if (Dee_type_seq_has_custom_tp_size(seq) &&
				    LOCAL_Dee_type_seq_has_custom_tp_Xitem_index(seq)) {
					result = &LOCAL_DeeSeq_DefaultXFirstWithXItemIndex;
				} else {
					result = &LOCAL_DeeSeq_DefaultXFirstWithXItem;
				}
			}
		}
		sc = DeeType_TryRequireSeqCache(self);
		if likely(sc)
			atomic_write(&sc->LOCAL_tsc_Xfirst, result);
		return result;
	}	break;

#ifdef DEFINE_DeeType_SeqCache_RequireDelX
	case Dee_SEQCLASS_SET:
		if (DeeType_SeqCache_RequireSetRemove(self) != &DeeSet_DefaultRemoveWithError)
			return &DeeSeq_DefaultDelFirstWithTSCRemoveForSet;
		break;

	case Dee_SEQCLASS_MAP:
		if (DeeType_HasOperator(self, OPERATOR_DELITEM))
			return &DeeSeq_DefaultDelFirstWithDelItemForMap;
		break;
#endif /* DEFINE_DeeType_SeqCache_RequireDelX */

	default: break;
	}
	return &LOCAL_DeeSeq_DefaultXFirstWithForeachDefault;
}

INTERN ATTR_RETNONNULL WUNUSED NONNULL((1)) LOCAL_Dee_tsc_Xlast_t DCALL
LOCAL_DeeType_SeqCache_RequireXLast(DeeTypeObject *__restrict self) {
	struct Dee_type_seq *seq = self->tp_seq;
	if likely(seq) {
		struct Dee_type_seq_cache *sc;
		sc = self->tp_seq->_tp_seqcache;
		if likely(sc && sc->LOCAL_tsc_Xlast)
			return sc->LOCAL_tsc_Xlast;
	}
	switch (DeeType_GetSeqClass(self)) {

	case Dee_SEQCLASS_SEQ: {
		struct Dee_type_seq_cache *sc;
		LOCAL_Dee_tsc_Xlast_t result = &LOCAL_DeeSeq_DefaultXLastWithForeachDefault;
		struct Dee_attrinfo attr;
		if (DeeObject_TFindAttrInfo(self, NULL, (DeeObject *)&str_last, &attr)) {
			if (attr.ai_type == Dee_ATTRINFO_GETSET) {
				if (attr.ai_value.v_getset->LOCAL_gs_X == &LOCAL_default_seq_Xlast ||
				    attr.ai_value.v_getset->LOCAL_gs_X == &LOCAL_DeeSeq_DefaultXLastWithXAttr ||
				    attr.ai_value.v_getset->LOCAL_gs_X == &LOCAL_DeeSeq_DefaultXLastWithSizeObAndXItem ||
				    attr.ai_value.v_getset->LOCAL_gs_X == &LOCAL_DeeSeq_DefaultXLastWithSizeAndXItemIndex ||
#ifdef LOCAL_DeeSeq_DefaultXLastWithSizeAndGetItemIndexFast
				    attr.ai_value.v_getset->LOCAL_gs_X == &LOCAL_DeeSeq_DefaultXLastWithSizeAndGetItemIndexFast ||
#endif /* LOCAL_DeeSeq_DefaultXLastWithSizeAndGetItemIndexFast */
				    attr.ai_value.v_getset->LOCAL_gs_X == &LOCAL_DeeSeq_DefaultXLastWithForeachDefault)
					goto detect_for_sequence_type;
				result = attr.ai_value.v_getset->LOCAL_gs_X; /* Directly forward attribute */
			} else {
				result = &LOCAL_DeeSeq_DefaultXLastWithXAttr;
			}
		} else {
detect_for_sequence_type:
			if (DeeType_RequireSize(self) && LOCAL_DeeType_RequireXItem(self)) {
				bool has_tp_size = Dee_type_seq_has_custom_tp_size(seq);
#ifdef LOCAL_DeeSeq_DefaultXLastWithSizeAndGetItemIndexFast
				if (has_tp_size && seq->tp_getitem_index_fast) {
					result = &LOCAL_DeeSeq_DefaultXLastWithSizeAndGetItemIndexFast;
				} else
#endif /* LOCAL_DeeSeq_DefaultXLastWithSizeAndGetItemIndexFast */
				{
					if (has_tp_size && LOCAL_Dee_type_seq_has_custom_tp_Xitem_index(seq)) {
						result = &LOCAL_DeeSeq_DefaultXLastWithSizeAndXItemIndex;
					} else {
						result = &LOCAL_DeeSeq_DefaultXLastWithSizeObAndXItem;
					}
				}
			}
		}
		sc = DeeType_TryRequireSeqCache(self);
		if likely(sc)
			atomic_write(&sc->LOCAL_tsc_Xlast, result);
		return result;
	}	break;

#ifdef DEFINE_DeeType_SeqCache_RequireDelX
	case Dee_SEQCLASS_SET:
		if (DeeType_SeqCache_RequireSetRemove(self) != &DeeSet_DefaultRemoveWithError)
			return &DeeSeq_DefaultDelLastWithTSCRemoveForSet;
		break;

	case Dee_SEQCLASS_MAP:
		if (DeeType_HasOperator(self, OPERATOR_DELITEM))
			return &DeeSeq_DefaultDelLastWithDelItemForMap;
		break;
#endif /* DEFINE_DeeType_SeqCache_RequireDelX */

	default: break;
	}
	return &LOCAL_DeeSeq_DefaultXLastWithForeachDefault;
}

#undef LOCAL_gs_X
#undef LOCAL_tsc_Xfirst
#undef LOCAL_tsc_Xlast
#undef LOCAL_Dee_tsc_Xfirst_t
#undef LOCAL_Dee_tsc_Xlast_t
#undef LOCAL_default_seq_Xfirst
#undef LOCAL_default_seq_Xlast
#undef LOCAL_DeeSeq_DefaultXFirstWithXAttr
#undef LOCAL_DeeSeq_DefaultXFirstWithXItem
#undef LOCAL_DeeSeq_DefaultXFirstWithXItemIndex
#undef LOCAL_DeeSeq_DefaultXFirstWithForeachDefault
#undef LOCAL_DeeSeq_DefaultXLastWithXAttr
#undef LOCAL_DeeSeq_DefaultXLastWithSizeObAndXItem
#undef LOCAL_DeeSeq_DefaultXLastWithSizeAndGetItemIndexFast
#undef LOCAL_DeeSeq_DefaultXLastWithSizeAndXItemIndex
#undef LOCAL_DeeSeq_DefaultXLastWithForeachDefault
#undef LOCAL_Dee_type_seq_has_custom_tp_Xitem_index
#undef LOCAL_DeeType_RequireXItem

#undef LOCAL_IS_GET
#undef LOCAL_IS_BOUND
#undef LOCAL_IS_DEL
#undef LOCAL_IS_SET
#undef LOCAL_DeeType_SeqCache_RequireXFirst
#undef LOCAL_DeeType_SeqCache_RequireXLast

DECL_END

#undef DEFINE_DeeType_SeqCache_RequireGetX
#undef DEFINE_DeeType_SeqCache_RequireBoundX
#undef DEFINE_DeeType_SeqCache_RequireDelX
#undef DEFINE_DeeType_SeqCache_RequireSetX
