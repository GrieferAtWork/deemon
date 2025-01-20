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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINT_SELECT_C
#define GUARD_DEEMON_RUNTIME_METHOD_HINT_SELECT_C 1

#include <deemon/api.h>

#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/alloc.h>
#include <deemon/class.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/seq.h>

#include "operator-require.h"

/**/
#include "method-hint-defaults.h"
#include "method-hint-select.h"

DECL_BEGIN

/* clang-format off */
/*[[[deemon (printMhInitSelectImpls from "..method-hints.method-hints")();]]]*/
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_bool_t DCALL
mh_select_seq_operator_bool(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size;
	if (DeeType_GetSeqClass(self) != Dee_SEQCLASS_NONE) {
		struct type_seq *tp_seq;
#ifndef LOCAL_FOR_OPTIMIZE
		if (DeeType_RequireBool(self))
			return self->tp_cast.tp_bool;
#endif /* !LOCAL_FOR_OPTIMIZE */
		tp_seq = self->tp_seq;
		if (tp_seq) {
			if (tp_seq->tp_size && !DeeType_IsDefaultSize(tp_seq->tp_size))
				return &DeeSeq_DefaultBoolWithSize;
			if (tp_seq->tp_sizeob && !DeeType_IsDefaultSizeOb(tp_seq->tp_sizeob))
				return &DeeSeq_DefaultBoolWithSizeOb;
			if (tp_seq->tp_foreach && !DeeType_IsDefaultForeach(tp_seq->tp_foreach))
				return &DeeSeq_DefaultBoolWithForeach;
			if (self->tp_cmp && self->tp_cmp->tp_compare_eq &&
			    !DeeType_IsDefaultCompareEq(self->tp_cmp->tp_compare_eq) &&
			    !DeeType_IsDefaultCompare(self->tp_cmp->tp_compare_eq))
				return &DeeSeq_DefaultBoolWithCompareEq;
			if (self->tp_cmp && self->tp_cmp->tp_eq && !DeeType_IsDefaultEq(self->tp_cmp->tp_eq))
				return &DeeSeq_DefaultBoolWithEq;
			if (self->tp_cmp && self->tp_cmp->tp_ne && !DeeType_IsDefaultNe(self->tp_cmp->tp_ne))
				return &DeeSeq_DefaultBoolWithNe;
			if (tp_seq->tp_foreach || DeeType_InheritIter(self))
				return &DeeSeq_DefaultBoolWithForeachDefault;
		}
	}
	seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &default__seq_operator_bool__empty;
	if (seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach)
		return &default__seq_operator_bool__with__seq_operator_foreach;
	if (seq_operator_size)
		return &default__seq_operator_bool__with__seq_operator_size;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_sizeob_t DCALL
mh_select_seq_operator_sizeob(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) != Dee_SEQCLASS_NONE && DeeType_RequireSizeOb(self))
		return self->tp_seq->tp_sizeob;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &default__seq_operator_sizeob__empty;
	if (seq_operator_size)
		return &default__seq_operator_sizeob__with__seq_operator_size;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_size_t DCALL
mh_select_seq_operator_size(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) != Dee_SEQCLASS_NONE && DeeType_RequireSize(self))
		return self->tp_seq->tp_size;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_operator_size__empty;
	if (seq_operator_foreach)
		return default__seq_operator_size__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_iter_t DCALL
mh_select_seq_operator_iter(DeeTypeObject *self, DeeTypeObject *orig_type) {
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_RequireIter(self))
		return self->tp_seq->tp_iter;
#endif /* !LOCAL_FOR_OPTIMIZE */
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_foreach_t DCALL
mh_select_seq_operator_foreach(DeeTypeObject *self, DeeTypeObject *orig_type) {
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_RequireForeach(self))
		return self->tp_seq->tp_foreach;
#endif /* !LOCAL_FOR_OPTIMIZE */
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_foreach_pair_t DCALL
mh_select_seq_operator_foreach_pair(DeeTypeObject *self, DeeTypeObject *orig_type) {
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_RequireForeachPair(self))
		return self->tp_seq->tp_foreach_pair;
#endif /* !LOCAL_FOR_OPTIMIZE */
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_iterkeys_t DCALL
mh_select_seq_operator_iterkeys(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireIterKeys(self))
		return self->tp_seq->tp_iterkeys;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_operator_iterkeys__empty;
	if (seq_operator_foreach)
		return &default__seq_operator_iterkeys__with__seq_size;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_enumerate_t DCALL
mh_select_seq_operator_enumerate(DeeTypeObject *self, DeeTypeObject *orig_type) {
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerate(self))
		return self->tp_seq->tp_enumerate;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		if (self->tp_seq &&
		    self->tp_seq->tp_getitem_index_fast &&
		    self->tp_seq->tp_size)
			return &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndexFast
	}
	//TODO: $with__seq_operator_size_and_seq_operator_try_getitem_index
	//TODO: $with__seq_operator_sizeob_and_seq_operator_try_getitem
	//TODO: $with__counter_and_seq_operator_foreach
	//TODO: $with__counter_and_seq_operator_iter
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_enumerate_index_t DCALL
mh_select_seq_operator_enumerate_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerateIndex(self))
		return self->tp_seq->tp_enumerate_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		if (self->tp_seq &&
		    self->tp_seq->tp_getitem_index_fast &&
		    self->tp_seq->tp_size)
			return &DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndexFast
	}
	//TODO: $with__seq_operator_size_and_seq_operator_try_getitem_index
	//TODO: $with__counter_and_seq_operator_foreach
	//TODO: $with__counter_and_seq_operator_iter
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_t DCALL
mh_select_seq_any(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_any__empty;
	if (seq_operator_foreach)
		return &default__seq_any__with__seq_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_with_key_t DCALL
mh_select_seq_any_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_any_with_key__empty;
	if (seq_operator_foreach)
		return &default__seq_any_with_key__with__seq_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_with_range_t DCALL
mh_select_seq_any_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_enumerate_index_t seq_operator_enumerate_index = (DeeMH_seq_operator_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_enumerate_index);
	if (seq_operator_enumerate_index == &default__seq_operator_enumerate_index__empty)
		return &default__seq_any_with_range__empty;
	if (seq_operator_enumerate_index)
		return &default__seq_any_with_range__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_with_range_and_key_t DCALL
mh_select_seq_any_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_enumerate_index_t seq_operator_enumerate_index = (DeeMH_seq_operator_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_enumerate_index);
	if (seq_operator_enumerate_index == &default__seq_operator_enumerate_index__empty)
		return &default__seq_any_with_range_and_key__empty;
	if (seq_operator_enumerate_index)
		return &default__seq_any_with_range_and_key__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_all_t DCALL
mh_select_seq_all(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_all__empty;
	if (seq_operator_foreach)
		return &default__seq_all__with__seq_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_all_with_key_t DCALL
mh_select_seq_all_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_all_with_key__empty;
	if (seq_operator_foreach)
		return &default__seq_all_with_key__with__seq_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_all_with_range_t DCALL
mh_select_seq_all_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_enumerate_index_t seq_operator_enumerate_index = (DeeMH_seq_operator_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_enumerate_index);
	if (seq_operator_enumerate_index == &default__seq_operator_enumerate_index__empty)
		return &default__seq_all_with_range__empty;
	if (seq_operator_enumerate_index)
		return &default__seq_all_with_range__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_all_with_range_and_key_t DCALL
mh_select_seq_all_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_enumerate_index_t seq_operator_enumerate_index = (DeeMH_seq_operator_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_enumerate_index);
	if (seq_operator_enumerate_index == &default__seq_operator_enumerate_index__empty)
		return &default__seq_all_with_range_and_key__empty;
	if (seq_operator_enumerate_index)
		return &default__seq_all_with_range_and_key__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_trygetfirst_t DCALL
mh_select_seq_trygetfirst(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		if (self->tp_seq &&
		    self->tp_seq->tp_getitem_index_fast &&
		    self->tp_seq->tp_size)
			return &default__seq_trygetfirst__with__size_and_getitem_index_fast;
	}
	seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &default__seq_trygetfirst__empty;
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_foreach)
		return &default__seq_trygetfirst__with__seq_operator_foreach;
	if (seq_operator_trygetitem_index)
		return &default__seq_trygetfirst__with__seq_operator_trygetitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_getfirst_t DCALL
mh_select_seq_getfirst(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_trygetfirst_t seq_trygetfirst = (DeeMH_seq_trygetfirst_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_trygetfirst);
	if (seq_trygetfirst == &default__seq_trygetfirst__empty)
		return &default__seq_getfirst__empty;
	if (seq_trygetfirst == &default__seq_trygetfirst__with__seq_operator_trygetitem_index)
		return &default__seq_getfirst__with__seq_operator_getitem_index;
	if (seq_trygetfirst)
		return &default__seq_getfirst__with__seq_trygetfirst;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_boundfirst_t DCALL
mh_select_seq_boundfirst(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_trygetfirst_t seq_trygetfirst = (DeeMH_seq_trygetfirst_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_trygetfirst);
	if (seq_trygetfirst == &default__seq_trygetfirst__empty)
		return &default__seq_boundfirst__empty;
	if (seq_trygetfirst == &default__seq_trygetfirst__with__seq_operator_trygetitem_index)
		return &default__seq_boundfirst__with__seq_operator_bounditem_index;
	if (seq_trygetfirst)
		return &default__seq_boundfirst__with__seq_trygetfirst;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_delfirst_t DCALL
mh_select_seq_delfirst(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_trygetfirst_t seq_trygetfirst = (DeeMH_seq_trygetfirst_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_trygetfirst);
	if (seq_trygetfirst == &default__seq_trygetfirst__empty)
		return &default__seq_delfirst__empty;
	if (seq_trygetfirst == &default__seq_trygetfirst__with__seq_operator_trygetitem_index) {
		if ((DeeMH_seq_operator_delitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delitem_index))
			return &default__seq_delfirst__with__seq_operator_delitem_index;
	}
	if (seq_trygetfirst)
		return &default__seq_delfirst__unsupported;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_setfirst_t DCALL
mh_select_seq_setfirst(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_trygetfirst_t seq_trygetfirst = (DeeMH_seq_trygetfirst_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_trygetfirst);
	if (seq_trygetfirst == &default__seq_trygetfirst__empty)
		return &default__seq_setfirst__empty;
	if (seq_trygetfirst == &default__seq_trygetfirst__with__seq_operator_trygetitem_index) {
		if ((DeeMH_seq_operator_setitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setitem_index))
			return &default__seq_setfirst__with__seq_operator_setitem_index;
	}
	if (seq_trygetfirst)
		return &default__seq_setfirst__unsupported;
	return NULL;
}
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_SELECT_C */
