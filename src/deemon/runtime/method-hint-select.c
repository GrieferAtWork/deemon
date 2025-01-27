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
	DeeMH_seq_operator_iter_t seq_operator_iter;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_RequireForeach(self))
		return self->tp_seq->tp_foreach;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_iter = (DeeMH_seq_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty)
		return &default__seq_operator_foreach__empty;
	if (seq_operator_iter)
		return &default__seq_operator_foreach__with__seq_operator_iter;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_foreach_pair_t DCALL
mh_select_seq_operator_foreach_pair(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_iter_t seq_operator_iter;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_RequireForeachPair(self))
		return self->tp_seq->tp_foreach_pair;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_iter = (DeeMH_seq_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty)
		return &default__seq_operator_foreach_pair__empty;
	if (seq_operator_iter)
		return &default__seq_operator_foreach_pair__with__seq_operator_iter;
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
	DeeMH_seq_operator_size_t seq_operator_size;
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerate(self))
		return self->tp_seq->tp_enumerate;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		if (self->tp_seq &&
		    self->tp_seq->tp_getitem_index_fast &&
		    self->tp_seq->tp_size)
			return &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndexFast;
	}
	seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_getitem_t seq_operator_getitem;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_operator_enumerate__empty;
		seq_operator_getitem = (DeeMH_seq_operator_getitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem);
		if (seq_operator_getitem == &default__seq_operator_getitem__with__seq_operator_getitem_index)
			return &default__seq_operator_enumerate__with__seq_operator_size_and_seq_operator_getitem_index;
		if (seq_operator_getitem != NULL)
			return &default__seq_operator_enumerate__with__seq_operator_sizeob_and_seq_operator_getitem;
	}
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_operator_enumerate__empty;
	if (seq_operator_foreach)
		return &default__seq_operator_enumerate__with__counter_and_seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_enumerate_index_t DCALL
mh_select_seq_operator_enumerate_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size;
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEnumerateIndex(self))
		return self->tp_seq->tp_enumerate_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		if (self->tp_seq &&
		    self->tp_seq->tp_getitem_index_fast &&
		    self->tp_seq->tp_size)
			return &DeeSeq_DefaultEnumerateIndexWithSizeAndGetItemIndexFast;
	}
	seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_operator_enumerate_index__empty;
		if ((DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index))
			return &default__seq_operator_enumerate_index__with__seq_operator_size_and_seq_operator_getitem_index;
	}
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_operator_enumerate_index__empty;
	if (seq_operator_foreach)
		return &default__seq_operator_enumerate_index__with__counter_and_seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getitem_t DCALL
mh_select_seq_operator_getitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetItem(self))
		return self->tp_seq->tp_getitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &default__seq_operator_getitem__empty;
	if (seq_operator_getitem_index)
		return &default__seq_operator_getitem__with__seq_operator_getitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getitem_index_t DCALL
mh_select_seq_operator_getitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetItemIndex(self))
		return self->tp_seq->tp_getitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_operator_getitem_index__empty;
	if (seq_operator_foreach)
		return &default__seq_operator_getitem_index__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_trygetitem_index_t DCALL
mh_select_seq_operator_trygetitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireTryGetItemIndex(self))
		return self->tp_seq->tp_trygetitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &default__seq_operator_trygetitem_index__empty;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_foreach)
		return &default__seq_operator_trygetitem_index__with__seq_operator_foreach;
	if (seq_operator_getitem_index)
		return &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_trygetitem_t DCALL
mh_select_seq_operator_trygetitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireTryGetItem(self))
		return self->tp_seq->tp_trygetitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &default__seq_operator_trygetitem__empty;
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
		return &default__seq_operator_trygetitem__with__seq_operator_getitem;
	if (seq_operator_trygetitem_index)
		return &default__seq_operator_trygetitem__with__seq_operator_trygetitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_hasitem_t DCALL
mh_select_seq_operator_hasitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_hasitem_index_t seq_operator_hasitem_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireHasItem(self))
		return self->tp_seq->tp_hasitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_hasitem_index = (DeeMH_seq_operator_hasitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_hasitem_index);
	if (seq_operator_hasitem_index == &default__seq_operator_hasitem_index__empty)
		return &default__seq_operator_hasitem__empty;
	if (seq_operator_hasitem_index == &default__seq_operator_hasitem_index__with__seq_operator_getitem_index)
		return &default__seq_operator_hasitem__with__seq_operator_getitem;
	if (seq_operator_hasitem_index)
		return &default__seq_operator_hasitem__with__seq_operator_hasitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_hasitem_index_t DCALL
mh_select_seq_operator_hasitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size;
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireHasItem(self))
		return self->tp_seq->tp_hasitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &default__seq_operator_hasitem_index__empty;
	if (seq_operator_size != &default__seq_operator_size__with__seq_operator_foreach)
		return &default__seq_operator_hasitem_index__with__seq_operator_size;
	seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &default__seq_operator_hasitem_index__empty;
	if (seq_operator_getitem_index)
		return &default__seq_operator_hasitem_index__with__seq_operator_getitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_bounditem_t DCALL
mh_select_seq_operator_bounditem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_bounditem_index_t seq_operator_bounditem_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireBoundItem(self))
		return self->tp_seq->tp_bounditem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_bounditem_index = (DeeMH_seq_operator_bounditem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_bounditem_index);
	if (seq_operator_bounditem_index == &default__seq_operator_bounditem_index__empty)
		return &default__seq_operator_bounditem__empty;
	if (seq_operator_bounditem_index == &default__seq_operator_bounditem_index__with__seq_operator_getitem_index)
		return &default__seq_operator_bounditem__with__seq_operator_getitem;
	if (seq_operator_bounditem_index)
		return &default__seq_operator_bounditem__with__seq_operator_bounditem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_bounditem_index_t DCALL
mh_select_seq_operator_bounditem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireHasItem(self))
		return self->tp_seq->tp_bounditem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &default__seq_operator_bounditem_index__empty;
	if (seq_operator_getitem_index)
		return &default__seq_operator_bounditem_index__with__seq_operator_getitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_delitem_t DCALL
mh_select_seq_operator_delitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_delitem_index_t seq_operator_delitem_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireDelItem(self))
		return self->tp_seq->tp_delitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_delitem_index = (DeeMH_seq_operator_delitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delitem_index);
	if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
		return &default__seq_operator_delitem__empty;
	if (seq_operator_delitem_index)
		return &default__seq_operator_delitem__with__seq_operator_delitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_delitem_index_t DCALL
mh_select_seq_operator_delitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_erase_t seq_erase;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireDelItemIndex(self))
		return self->tp_seq->tp_delitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_erase = (DeeMH_seq_erase_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_erase);
	if (seq_erase == &default__seq_erase__empty)
		return &default__seq_operator_delitem_index__empty;
	if (seq_erase && (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported)
		return &default__seq_operator_delitem_index__with__seq_operator_size__and__seq_erase;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_setitem_t DCALL
mh_select_seq_operator_setitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_setitem_index_t seq_operator_setitem_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetItem(self))
		return self->tp_seq->tp_setitem;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_setitem_index = (DeeMH_seq_operator_setitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setitem_index);
	if (seq_operator_setitem_index == &default__seq_operator_setitem_index__empty)
		return &default__seq_operator_setitem__empty;
	if (seq_operator_setitem_index)
		return &default__seq_operator_setitem__with__seq_operator_setitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_setitem_index_t DCALL
mh_select_seq_operator_setitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetItemIndex(self))
		return self->tp_seq->tp_setitem_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_operator_setitem_index__empty;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getrange_t DCALL
mh_select_seq_operator_getrange(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_getrange_index_t seq_operator_getrange_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetRange(self))
		return self->tp_seq->tp_getrange;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_getrange_index = (DeeMH_seq_operator_getrange_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getrange_index);
	if (seq_operator_getrange_index == &default__seq_operator_getrange_index__empty)
		return &default__seq_operator_getrange__empty;
	if (seq_operator_getrange_index && (DeeMH_seq_operator_getrange_index_n_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getrange_index_n))
		return &default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getrange_index_t DCALL
mh_select_seq_operator_getrange_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetRangeIndex(self))
		return self->tp_seq->tp_getrange_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (self->tp_seq &&
	    self->tp_seq->tp_getitem_index_fast &&
	    self->tp_seq->tp_size &&
	    !DeeType_IsDefaultSize(self->tp_seq->tp_size))
		return &DeeSeq_DefaultGetRangeIndexWithSizeAndGetItemIndexFast;
	seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return default__seq_operator_getrange_index__empty;
	if (seq_operator_trygetitem_index) {
		DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
		if (seq_operator_size != &default__seq_operator_size__unsupported) {
			if (seq_operator_size != &default__seq_operator_size__empty)
				return default__seq_operator_getrange_index__empty;
			if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach) {
				if ((DeeMH_seq_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_iter))
					return default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter;
			}
			if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index) {
				DeeMH_seq_operator_getitem_t seq_operator_getitem = (DeeMH_seq_operator_getitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem);
				if (seq_operator_getitem == &default__seq_operator_getitem__with__seq_operator_getitem_index)
					return default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index;
				if (seq_operator_getitem == &default__seq_operator_getitem__empty)
					return default__seq_operator_getrange_index__empty;
				return default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem;
			}
			return default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index;
		}
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getrange_index_n_t DCALL
mh_select_seq_operator_getrange_index_n(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireGetRangeIndexN(self))
		return self->tp_seq->tp_getrange_index_n;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (self->tp_seq &&
	    self->tp_seq->tp_getitem_index_fast &&
	    self->tp_seq->tp_size &&
	    !DeeType_IsDefaultSize(self->tp_seq->tp_size))
		return &DeeSeq_DefaultGetRangeIndexNWithSizeAndGetItemIndexFast;
	seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_getrange_index_t seq_operator_getrange_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return default__seq_operator_getrange_index_n__empty;
		seq_operator_getrange_index = (DeeMH_seq_operator_getrange_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getrange_index);
		if (seq_operator_getrange_index == &DeeSeq_DefaultGetRangeIndexWithSizeAndGetItemIndexFast)
			return &DeeSeq_DefaultGetRangeIndexNWithSizeAndGetItemIndexFast;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__empty)
			return &default__seq_operator_getrange_index_n__empty;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index)
			return default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index)
			return default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem)
			return default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter)
			return default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter;
		if (seq_operator_getrange_index)
			return default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getrange_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_delrange_t DCALL
mh_select_seq_operator_delrange(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_delrange_index_t seq_operator_delrange_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		if (DeeType_RequireDelRange(self))
			return self->tp_seq->tp_delrange;
		if (DeeType_RequireSetRange(self))
			return &default__seq_operator_delrange__with__seq_operator_setrange;
	}
#endif /* !LOCAL_FOR_OPTIMIZE */
	if ((DeeMH_seq_operator_setrange_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_setrange))
		return &default__seq_operator_delrange__with__seq_operator_setrange;
	seq_operator_delrange_index = (DeeMH_seq_operator_delrange_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delrange_index);
	if (seq_operator_delrange_index == &default__seq_operator_delrange_index__empty)
		return &default__seq_operator_delrange__empty;
	if (seq_operator_delrange_index && (DeeMH_seq_operator_delrange_index_n_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delrange_index_n))
		return &default__seq_operator_delrange__with__seq_operator_delrange_index__and__seq_operator_delrange_index_n;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_delrange_index_t DCALL
mh_select_seq_operator_delrange_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		if (DeeType_RequireDelRangeIndex(self))
			return self->tp_seq->tp_delrange_index;
		if (DeeType_RequireSetRangeIndex(self))
			return &default__seq_operator_delrange_index__with__seq_operator_setrange_index;
	}
#endif /* !LOCAL_FOR_OPTIMIZE */
	if ((DeeMH_seq_operator_setrange_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_setrange_index))
		return &default__seq_operator_delrange_index__with__seq_operator_setrange_index;
	seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_operator_delrange_index__empty;
		if ((DeeMH_seq_erase_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_erase))
			return &default__seq_operator_delrange_index__with__seq_operator_size__and__seq_erase;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_delrange_index_n_t DCALL
mh_select_seq_operator_delrange_index_n(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_delrange_index_t seq_operator_delrange_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		if (DeeType_RequireDelRangeIndexN(self))
			return self->tp_seq->tp_delrange_index_n;
		if (DeeType_RequireSetRangeIndexN(self))
			return &default__seq_operator_delrange_index_n__with__seq_operator_setrange_index_n;
	}
#endif /* !LOCAL_FOR_OPTIMIZE */
	if ((DeeMH_seq_operator_setrange_index_n_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_setrange_index_n))
		return &default__seq_operator_delrange_index_n__with__seq_operator_setrange_index_n;
	seq_operator_delrange_index = (DeeMH_seq_operator_delrange_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delrange_index);
	if (seq_operator_delrange_index == &default__seq_operator_delrange_index__empty)
		return default__seq_operator_delrange_index_n__empty;
	if (seq_operator_delrange_index == &default__seq_operator_delrange_index__with__seq_operator_delrange)
		return default__seq_operator_delrange_index_n__with__seq_operator_delrange;
	if (seq_operator_delrange_index)
		return default__seq_operator_delrange_index_n__with__seq_operator_delrange_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_setrange_t DCALL
mh_select_seq_operator_setrange(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_setrange_index_t seq_operator_setrange_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetRange(self))
		return self->tp_seq->tp_setrange;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_setrange_index = (DeeMH_seq_operator_setrange_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setrange_index);
	if (seq_operator_setrange_index == &default__seq_operator_setrange_index__empty)
		return &default__seq_operator_setrange__empty;
	if (seq_operator_setrange_index && (DeeMH_seq_operator_setrange_index_n_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setrange_index_n))
		return &default__seq_operator_setrange__with__seq_operator_setrange_index__and__seq_operator_setrange_index_n;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_setrange_index_t DCALL
mh_select_seq_operator_setrange_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetRangeIndex(self))
		return self->tp_seq->tp_setrange_index;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_operator_setrange_index__empty;
		if ((DeeMH_seq_erase_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_erase) &&
		    (DeeMH_seq_insertall_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_insertall))
			return &default__seq_operator_setrange_index__with__seq_operator_size__and__seq_erase__and__seq_insertall;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_setrange_index_n_t DCALL
mh_select_seq_operator_setrange_index_n(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_setrange_index_t seq_operator_setrange_index;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetRangeIndexN(self))
		return self->tp_seq->tp_setrange_index_n;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_setrange_index = (DeeMH_seq_operator_setrange_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setrange_index);
	if (seq_operator_setrange_index == &default__seq_operator_setrange_index__empty)
		return default__seq_operator_setrange_index_n__empty;
	if (seq_operator_setrange_index == &default__seq_operator_setrange_index__with__seq_operator_setrange)
		return default__seq_operator_setrange_index_n__with__seq_operator_setrange;
	if (seq_operator_setrange_index)
		return default__seq_operator_setrange_index_n__with__seq_operator_setrange_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_hash_t DCALL
mh_select_seq_operator_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireHash(self))
		return self->tp_cmp->tp_hash;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_operator_hash__empty;
	if (seq_operator_foreach)
		return default__seq_operator_hash__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_compare_t DCALL
mh_select_seq_operator_compare(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireCompare(self))
		return self->tp_cmp->tp_compare;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (self->tp_seq &&
	    self->tp_seq->tp_getitem_index_fast &&
	    self->tp_seq->tp_size)
		return &DeeSeq_DefaultCompareWithSizeAndGetItemIndexFast;

	seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index) {
		DeeMH_seq_operator_size_t seq_operator_size;
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach)
			return &default__seq_operator_compare__with__seq_operator_foreach;
		seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
		if (seq_operator_size != &default__seq_operator_size__unsupported) {
			if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index) {
				if (seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob ||
				    (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index) == &default__seq_operator_getitem_index__with__seq_operator_getitem)
					return &default__seq_operator_compare__with__seq_operator_sizeob__and__seq_operator_getitem;
				return &default__seq_operator_compare__with__seq_operator_size__and__seq_operator_getitem_index;
			}
			return &default__seq_operator_compare__with__seq_operator_size__and__seq_operator_trygetitem_index;
		}
	}

	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_operator_compare__empty;
	if (seq_operator_foreach)
		return &default__seq_operator_compare__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_compare_eq_t DCALL
mh_select_seq_operator_compare_eq(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_compare_t seq_operator_compare;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireCompareEq(self))
		return self->tp_cmp->tp_compare_eq;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (self->tp_seq &&
	    self->tp_seq->tp_getitem_index_fast &&
	    self->tp_seq->tp_size)
		return &DeeSeq_DefaultCompareWithSizeAndGetItemIndexFast;

	seq_operator_compare = (DeeMH_seq_operator_compare_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare);
	if (seq_operator_compare == &default__seq_operator_compare__empty)
		return &default__seq_operator_compare_eq__empty;
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_foreach)
		return &default__seq_operator_compare_eq__with__seq_operator_foreach;
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_size__and__seq_operator_getitem_index)
		return &default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &default__seq_operator_compare_eq__with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_operator_compare)
		return seq_operator_compare; /* Binary-compatible! */
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_trycompare_eq_t DCALL
mh_select_seq_operator_trycompare_eq(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_compare_eq_t seq_operator_compare_eq;
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireTryCompareEq(self))
		return self->tp_cmp->tp_trycompare_eq;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if (self->tp_seq &&
	    self->tp_seq->tp_getitem_index_fast &&
	    self->tp_seq->tp_size)
		return &DeeSeq_DefaultTryCompareEqWithSizeAndGetItemIndexFast;
	seq_operator_compare_eq = (DeeMH_seq_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare_eq);
	if (seq_operator_compare_eq == &default__seq_operator_compare_eq__empty)
		return &default__seq_operator_trycompare_eq__empty;
	if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_foreach)
		return &default__seq_operator_trycompare_eq__with__seq_operator_foreach;
	if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &default__seq_operator_trycompare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_getitem_index)
		return &default__seq_operator_trycompare_eq__with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &default__seq_operator_trycompare_eq__with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_operator_compare_eq)
		return &default__seq_operator_trycompare_eq__with__seq_operator_compare_eq;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_eq_t DCALL
mh_select_seq_operator_eq(DeeTypeObject *self, DeeTypeObject *orig_type) {
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEq(self))
		return self->tp_cmp->tp_eq;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if ((DeeMH_seq_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare_eq))
		return &default__seq_operator_eq__with__seq_operator_compare_eq;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_ne_t DCALL
mh_select_seq_operator_ne(DeeTypeObject *self, DeeTypeObject *orig_type) {
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEq(self))
		return self->tp_cmp->tp_ne;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if ((DeeMH_seq_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare_eq))
		return &default__seq_operator_ne__with__seq_operator_compare_eq;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_lo_t DCALL
mh_select_seq_operator_lo(DeeTypeObject *self, DeeTypeObject *orig_type) {
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEq(self))
		return self->tp_cmp->tp_lo;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if ((DeeMH_seq_operator_compare_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare))
		return &default__seq_operator_lo__with__seq_operator_compare;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_le_t DCALL
mh_select_seq_operator_le(DeeTypeObject *self, DeeTypeObject *orig_type) {
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEq(self))
		return self->tp_cmp->tp_le;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if ((DeeMH_seq_operator_compare_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare))
		return &default__seq_operator_le__with__seq_operator_compare;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_gr_t DCALL
mh_select_seq_operator_gr(DeeTypeObject *self, DeeTypeObject *orig_type) {
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEq(self))
		return self->tp_cmp->tp_gr;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if ((DeeMH_seq_operator_compare_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare))
		return &default__seq_operator_gr__with__seq_operator_compare;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_ge_t DCALL
mh_select_seq_operator_ge(DeeTypeObject *self, DeeTypeObject *orig_type) {
#ifndef LOCAL_FOR_OPTIMIZE
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireEq(self))
		return self->tp_cmp->tp_ge;
#endif /* !LOCAL_FOR_OPTIMIZE */
	if ((DeeMH_seq_operator_compare_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare))
		return &default__seq_operator_ge__with__seq_operator_compare;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_trygetfirst_t DCALL
mh_select_seq_trygetfirst(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		if (self->tp_seq &&
		    self->tp_seq->tp_getitem_index_fast &&
		    self->tp_seq->tp_size)
			return &default__seq_trygetfirst__with__size__and__getitem_index_fast;
	}
	seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &default__seq_trygetfirst__empty;
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach)
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

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_trygetlast_t DCALL
mh_select_seq_trygetlast(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ) {
		if (self->tp_seq &&
		    self->tp_seq->tp_getitem_index_fast &&
		    self->tp_seq->tp_size)
			return &default__seq_trygetlast__with__size__and__getitem_index_fast;
	}
	seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &default__seq_trygetlast__empty;
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach)
		return &default__seq_trygetlast__with__seq_operator_foreach;
	if (seq_operator_trygetitem_index &&
	    (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported)
		return &default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_getlast_t DCALL
mh_select_seq_getlast(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_trygetlast_t seq_trygetlast = (DeeMH_seq_trygetlast_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_trygetlast);
	if (seq_trygetlast == &default__seq_trygetlast__empty)
		return &default__seq_getlast__empty;
	if (seq_trygetlast == &default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &default__seq_getlast__with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_trygetlast)
		return &default__seq_getlast__with__seq_trygetlast;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_boundlast_t DCALL
mh_select_seq_boundlast(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_trygetlast_t seq_trygetlast = (DeeMH_seq_trygetlast_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_trygetlast);
	if (seq_trygetlast == &default__seq_trygetlast__empty)
		return &default__seq_boundlast__empty;
	if (seq_trygetlast == &default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &default__seq_boundlast__with__seq_operator_size__and__seq_operator_bounditem_index;
	if (seq_trygetlast)
		return &default__seq_boundlast__with__seq_trygetlast;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_dellast_t DCALL
mh_select_seq_dellast(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_trygetlast_t seq_trygetlast = (DeeMH_seq_trygetlast_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_trygetlast);
	if (seq_trygetlast == &default__seq_trygetlast__empty)
		return &default__seq_dellast__empty;
	if (seq_trygetlast == &default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		if ((DeeMH_seq_operator_delitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delitem_index))
			return &default__seq_dellast__with__seq_operator_size__and__seq_operator_delitem_index;
	}
	if (seq_trygetlast)
		return &default__seq_dellast__unsupported;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_setlast_t DCALL
mh_select_seq_setlast(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_trygetlast_t seq_trygetlast = (DeeMH_seq_trygetlast_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_trygetlast);
	if (seq_trygetlast == &default__seq_trygetlast__empty)
		return &default__seq_setlast__empty;
	if (seq_trygetlast == &default__seq_trygetlast__with__seq_operator_size__and__seq_operator_trygetitem_index) {
		if ((DeeMH_seq_operator_setitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setitem_index))
			return &default__seq_setlast__with__seq_operator_size__and__seq_operator_setitem_index;
	}
	if (seq_trygetlast)
		return &default__seq_setlast__unsupported;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_t DCALL
mh_select_seq_any(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_any__empty;
	if (seq_operator_foreach)
		return &default__seq_any__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_with_key_t DCALL
mh_select_seq_any_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_any_with_key__empty;
	if (seq_operator_foreach)
		return &default__seq_any_with_key__with__seq_operator_foreach;
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
		return &default__seq_all__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_all_with_key_t DCALL
mh_select_seq_all_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_all_with_key__empty;
	if (seq_operator_foreach)
		return &default__seq_all_with_key__with__seq_operator_foreach;
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

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_find_t DCALL
mh_select_seq_find(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_enumerate_index_t seq_operator_enumerate_index = (DeeMH_seq_operator_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_enumerate_index);
	if (seq_operator_enumerate_index == &default__seq_operator_enumerate_index__empty)
		return &default__seq_find__empty;
	if (seq_operator_enumerate_index)
		return &default__seq_find__with__seq_operator_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_find_with_key_t DCALL
mh_select_seq_find_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_enumerate_index_t seq_operator_enumerate_index = (DeeMH_seq_operator_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_enumerate_index);
	if (seq_operator_enumerate_index == &default__seq_operator_enumerate_index__empty)
		return &default__seq_find_with_key__empty;
	if (seq_operator_enumerate_index)
		return &default__seq_find_with_key__with__seq_operator_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_erase_t DCALL
mh_select_seq_erase(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_pop_t seq_pop;
	if ((DeeMH_seq_operator_delrange_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_delrange_index))
		return &default__seq_erase__with__seq_operator_delrange_index;
	seq_pop = (DeeMH_seq_pop_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_pop);
	if (seq_pop == &default__seq_pop__empty)
		return &default__seq_erase__empty;
	if (seq_pop)
		return &default__seq_erase__with__seq_pop;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_insert_t DCALL
mh_select_seq_insert(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_insertall_t seq_insertall = (DeeMH_seq_insertall_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_insertall);
	if (seq_insertall == &default__seq_insertall__empty)
		return &default__seq_insert__empty;
	if (seq_insertall)
		return &default__seq_insert__with__seq_insertall;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_insertall_t DCALL
mh_select_seq_insertall(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_SEQ && DeeType_RequireSetRangeIndex(self))
		return &default__seq_insertall__with__seq_operator_setrange_index;
	if ((DeeMH_seq_operator_setrange_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_setrange_index))
		return &default__seq_insertall__with__seq_operator_setrange_index;
	if ((DeeMH_seq_insert_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_insert))
		return &default__seq_insertall__with__seq_insert;
	if ((DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach) == &default__seq_operator_foreach__empty)
		return &default__seq_insertall__empty;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_pushfront_t DCALL
mh_select_seq_pushfront(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_insert_t seq_insert = (DeeMH_seq_insert_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_insert);
	if (seq_insert == &default__seq_insert__empty)
		return &default__seq_pushfront__empty;
	if (seq_insert)
		return &default__seq_pushfront__with__seq_insert;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_append_t DCALL
mh_select_seq_append(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_extend_t seq_extend = (DeeMH_seq_extend_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_extend);
	if (seq_extend == &default__seq_extend__empty)
		return &default__seq_append__empty;
	if (seq_extend)
		return &default__seq_append__with__seq_extend;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_extend_t DCALL
mh_select_seq_extend(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size;
	DeeMH_seq_insertall_t seq_insertall;
	if ((DeeMH_seq_append_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_append))
		return &default__seq_extend__with__seq_append;
	seq_insertall = (DeeMH_seq_insertall_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_insertall);
	if (seq_insertall == &default__seq_insertall__empty)
		return &default__seq_extend__empty;
	seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &default__seq_extend__empty;
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_insertall == &default__seq_insertall__with__seq_operator_setrange_index)
			return &default__seq_extend__with__seq_operator_size__and__seq_operator_setrange_index;
		if (seq_insertall)
			return &default__seq_extend__with__seq_operator_size__and__seq_insertall;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_xchitem_index_t DCALL
mh_select_seq_xchitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_setitem_index_t seq_operator_setitem_index = (DeeMH_seq_operator_setitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setitem_index);
	if (seq_operator_setitem_index == &default__seq_operator_setitem_index__empty)
		return &default__seq_xchitem_index__empty;
	if (seq_operator_setitem_index) {
		if ((DeeMH_seq_operator_getitem_index_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_getitem_index) != &default__seq_operator_getitem_index__unsupported)
			return &default__seq_xchitem_index__with__seq_operator_getitem_index__and__seq_operator_setitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_clear_t DCALL
mh_select_seq_clear(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_erase_t seq_erase;
	DeeMH_seq_operator_delrange_index_n_t seq_operator_delrange_index_n;

	seq_erase = (DeeMH_seq_erase_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_erase);
	if (seq_erase == &default__seq_erase__empty)
		return &default__seq_clear__empty;
	if (seq_erase != &default__seq_erase__with__seq_operator_delrange_index &&
	    seq_erase != &default__seq_erase__with__seq_pop)
		return &default__seq_clear__with__seq_erase;

	seq_operator_delrange_index_n = (DeeMH_seq_operator_delrange_index_n_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delrange_index_n);
	if (seq_operator_delrange_index_n == &default__seq_operator_delrange_index_n__empty)
		return &default__seq_clear__empty;
	if (seq_operator_delrange_index_n == &default__seq_operator_delrange_index_n__with__seq_operator_delrange)
		return &default__seq_clear__with__seq_operator_delrange;
	if (seq_operator_delrange_index_n)
		return &default__seq_clear__with__seq_operator_delrange_index_n;
	if (seq_erase)
		return &default__seq_clear__with__seq_erase;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_pop_t DCALL
mh_select_seq_pop(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
	/* TODO:
	 * >> local item;
	 * >> do {
	 * >>     local used_index = index;
	 * >>     if (used_index < 0)
	 * >>         used_index = DeeSeqRange_Clamp_n(used_index, Sequence.__size__(this));
	 * >>     item = Sequence.__getitem__(this, used_index);
	 * >> } while (!Set.remove(this, item));
	 * >> return item;
	 *
	 * >> $with__seq_operator_size__and__seq_operator_getitem_index__and__set_remove
	 */
	seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &default__seq_pop__empty;
	if (seq_operator_getitem_index &&
	    (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_delitem_index_t seq_operator_delitem_index = (DeeMH_seq_operator_delitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delitem_index);
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
			return &default__seq_pop__empty;
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__with__seq_operator_size__and__seq_erase)
			return &default__seq_pop__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_erase;
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__with__seq_operator_size__and__seq_erase)
			return &default__seq_pop__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_delitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_iter_t DCALL
mh_select_set_operator_iter(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_iter_t seq_operator_iter;
#ifndef LOCAL_FOR_OPTIMIZE
	if (Dee_SEQCLASS_ISSETORMAP(DeeType_GetSeqClass(self)) && DeeType_RequireIter(self))
		return self->tp_seq->tp_iter;
#endif /* !LOCAL_FOR_OPTIMIZE */
	seq_operator_iter = (DeeMH_seq_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty)
		return &default__set_operator_iter__empty;
	if (seq_operator_iter)
		return &default__set_operator_iter__with__seq_operator_iter;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_foreach_t DCALL
mh_select_set_operator_foreach(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_iter_t set_operator_iter;
#ifndef LOCAL_FOR_OPTIMIZE
	if (Dee_SEQCLASS_ISSETORMAP(DeeType_GetSeqClass(self)) && DeeType_RequireForeach(self))
		return self->tp_seq->tp_foreach;
#endif /* !LOCAL_FOR_OPTIMIZE */
	set_operator_iter = (DeeMH_set_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_iter);
	if (set_operator_iter == &default__set_operator_iter__empty)
		return &default__set_operator_foreach__empty;
	if (set_operator_iter == &default__set_operator_iter__with__seq_operator_iter)
		return &default__set_operator_foreach__with__seq_operator_foreach;
	if (set_operator_iter)
		return &default__set_operator_foreach__with__set_operator_iter;
	return NULL;
}
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_SELECT_C */
