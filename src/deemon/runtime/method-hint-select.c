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
#include <deemon/method-hints.h>
#include <deemon/object.h>
#include <deemon/seq.h>

#include "operator-require.h"

/**/
#include "../objects/seq/default-enumerate.h"

/**/
#include "method-hint-defaults.h"
#include "method-hint-select.h"

DECL_BEGIN

/* clang-format off */
/*[[[deemon (printMhInitSelectImpls from "..method-hints.method-hints")();]]]*/
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_bool_t DCALL
mh_select_seq_operator_bool(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size;
	DeeMH_seq_operator_compare_eq_t seq_operator_compare_eq = (DeeMH_seq_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare_eq);
	if (seq_operator_compare_eq) {
		if (seq_operator_compare_eq == &default__seq_operator_compare_eq__empty)
			return &default__seq_operator_bool__empty;
		if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_foreach)
			goto use_size; /* return &$with__seq_operator_foreach; */
		if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_size__and__operator_getitem_index_fast ||
		    seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index ||
		    seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_getitem_index)
			goto use_size; /* return &$with__seq_operator_size; */
		if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_sizeob__and__seq_operator_getitem)
			goto use_size; /* return &$with__seq_operator_sizeob; */
		return &default__seq_operator_bool__with__seq_operator_compare_eq;
	}
use_size:
	seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &default__seq_operator_bool__empty;
	if (seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach)
		return &default__seq_operator_bool__with__seq_operator_foreach;
	if (seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob)
		return &default__seq_operator_bool__with__seq_operator_sizeob;
	if (seq_operator_size)
		return &default__seq_operator_bool__with__seq_operator_size;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_sizeob_t DCALL
mh_select_seq_operator_sizeob(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &default__seq_operator_sizeob__empty;
	if (seq_operator_size)
		return &default__seq_operator_sizeob__with__seq_operator_size;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_size_t DCALL
mh_select_seq_operator_size(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if ((DeeMH_seq_operator_sizeob_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_sizeob))
		return &default__seq_operator_size__with__seq_operator_sizeob;
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_operator_size__empty;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_foreach_pair)
		return &default__seq_operator_size__with__seq_operator_foreach_pair;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__map_enumerate)
		return &default__seq_operator_size__with__map_enumerate;
	if (seq_operator_foreach)
		return default__seq_operator_size__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_iter_t DCALL
mh_select_seq_operator_iter(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_operator_iter__empty;
		if (self->tp_seq && self->tp_seq->tp_getitem_index_fast)
			return &default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast;
		if ((DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_trygetitem_index))
			return &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index;
		if ((DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_getitem_index))
			return &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index;
		if ((DeeMH_seq_operator_getitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_getitem) || (DeeMH_seq_operator_trygetitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_trygetitem))
			return &default__seq_operator_iter__with__seq_operator_sizeob__and__seq_operator_getitem;
	} else {
		if ((DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_getitem_index) ||
		    (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_trygetitem_index))
			return &default__seq_operator_iter__with__seq_operator_getitem_index;
		if ((DeeMH_seq_operator_getitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_getitem) ||
		    (DeeMH_seq_operator_trygetitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_trygetitem))
			return &default__seq_operator_iter__with__seq_operator_getitem;
	}
	if ((DeeMH_map_enumerate_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_enumerate))
		return &default__seq_operator_iter__with__map_enumerate;
	if ((DeeMH_map_iterkeys_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_iterkeys)) {
		if ((DeeMH_map_operator_trygetitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_trygetitem))
			return &default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem;
		if ((DeeMH_map_operator_getitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_getitem))
			return &default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_foreach_t DCALL
mh_select_seq_operator_foreach(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_iter_t seq_operator_iter;
	if ((DeeMH_seq_operator_foreach_pair_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_foreach_pair))
		return &default__seq_operator_foreach__with__seq_operator_foreach_pair;
	seq_operator_iter = (DeeMH_seq_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty)
		return &default__seq_operator_foreach__empty;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast)
		return &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index)
		return &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_operator_iter == &default__seq_operator_iter__with__map_enumerate)
		return &default__seq_operator_foreach__with__map_enumerate;
	if (seq_operator_iter)
		return &default__seq_operator_foreach__with__seq_operator_iter;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_foreach_pair_t DCALL
mh_select_seq_operator_foreach_pair(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_operator_foreach_pair__empty;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_iter)
		return &default__seq_operator_foreach_pair__with__seq_operator_iter;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__map_enumerate)
		return &default__seq_operator_foreach_pair__with__map_enumerate;
	if (seq_operator_foreach)
		return &default__seq_operator_foreach_pair__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getitem_t DCALL
mh_select_seq_operator_getitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &default__seq_operator_getitem__empty;
	if (seq_operator_getitem_index)
		return &default__seq_operator_getitem__with__seq_operator_getitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getitem_index_t DCALL
mh_select_seq_operator_getitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__with__map_enumerate)
		return &default__seq_operator_getitem_index__with__map_enumerate;
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_operator_getitem_index__empty;
	if (seq_operator_foreach)
		return &default__seq_operator_getitem_index__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_trygetitem_t DCALL
mh_select_seq_operator_trygetitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &default__seq_operator_trygetitem__empty;
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
		return &default__seq_operator_trygetitem__with__seq_operator_getitem;
	if (seq_operator_trygetitem_index)
		return &default__seq_operator_trygetitem__with__seq_operator_trygetitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_trygetitem_index_t DCALL
mh_select_seq_operator_trygetitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &default__seq_operator_trygetitem_index__empty;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_foreach)
		return &default__seq_operator_trygetitem_index__with__seq_operator_foreach;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__map_enumerate)
		return &default__seq_operator_trygetitem_index__with__map_enumerate;
	if (seq_operator_getitem_index)
		return &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_hasitem_t DCALL
mh_select_seq_operator_hasitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_hasitem_index_t seq_operator_hasitem_index = (DeeMH_seq_operator_hasitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_hasitem_index);
	if (seq_operator_hasitem_index == &default__seq_operator_hasitem_index__empty)
		return &default__seq_operator_hasitem__empty;
	if (seq_operator_hasitem_index == &default__seq_operator_hasitem_index__with__seq_operator_getitem_index)
		return &default__seq_operator_hasitem__with__seq_operator_getitem;
	if (seq_operator_hasitem_index == &default__seq_operator_hasitem_index__with__seq_operator_size)
		return default__seq_operator_hasitem__with__seq_operator_sizeob;
	if (seq_operator_hasitem_index)
		return &default__seq_operator_hasitem__with__seq_operator_hasitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_hasitem_index_t DCALL
mh_select_seq_operator_hasitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_size);
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
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
	DeeMH_seq_operator_bounditem_index_t seq_operator_bounditem_index = (DeeMH_seq_operator_bounditem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_bounditem_index);
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
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &default__seq_operator_bounditem_index__empty;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__map_enumerate)
		return &default__seq_operator_bounditem_index__with__map_enumerate;
	if (seq_operator_getitem_index)
		return &default__seq_operator_bounditem_index__with__seq_operator_getitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_delitem_t DCALL
mh_select_seq_operator_delitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_delitem_index_t seq_operator_delitem_index = (DeeMH_seq_operator_delitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delitem_index);
	if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
		return &default__seq_operator_delitem__empty;
	if (seq_operator_delitem_index)
		return &default__seq_operator_delitem__with__seq_operator_delitem_index;
	/* TODO: CALL_DEPENDENCY(set_remove, self, CALL_DEPENDENCY(seq_operator_getitem, self, index)) */
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_delitem_index_t DCALL
mh_select_seq_operator_delitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_erase_t seq_erase;
	if ((DeeMH_seq_operator_delrange_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_delrange_index) ||
	    (DeeMH_seq_operator_setrange_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_setrange_index))
		return &default__seq_operator_delitem_index__with__seq_operator_delrange_index;
	seq_erase = (DeeMH_seq_erase_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_erase);
	if (seq_erase == &default__seq_erase__empty)
		return &default__seq_operator_delitem_index__empty;
	if (seq_erase && (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported)
		return &default__seq_operator_delitem_index__with__seq_operator_size__and__seq_erase;
	/* TODO: CALL_DEPENDENCY(set_remove, self, CALL_DEPENDENCY(seq_operator_getitem_index, self, index)) */
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_setitem_t DCALL
mh_select_seq_operator_setitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_setitem_index_t seq_operator_setitem_index = (DeeMH_seq_operator_setitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setitem_index);
	if (seq_operator_setitem_index == &default__seq_operator_setitem_index__empty)
		return &default__seq_operator_setitem__empty;
	if (seq_operator_setitem_index)
		return &default__seq_operator_setitem__with__seq_operator_setitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_setitem_index_t DCALL
mh_select_seq_operator_setitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if ((DeeMH_seq_operator_setrange_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_setrange_index))
		return &default__seq_operator_setitem_index__with__seq_operator_setrange_index;
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_operator_setitem_index__empty;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getrange_t DCALL
mh_select_seq_operator_getrange(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_getrange_index_t seq_operator_getrange_index = (DeeMH_seq_operator_getrange_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getrange_index);
	if (seq_operator_getrange_index == &default__seq_operator_getrange_index__empty)
		return &default__seq_operator_getrange__empty;
	if ((seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index &&
	     (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index) == &default__seq_operator_getitem_index__with__seq_operator_getitem) ||
	    (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index &&
	     (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index) == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index &&
	     (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index) == &default__seq_operator_getitem_index__with__seq_operator_getitem) ||
	    (seq_operator_getrange_index == default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem))
		return &default__seq_operator_getrange__with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_operator_getrange_index && (DeeMH_seq_operator_getrange_index_n_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getrange_index_n))
		return &default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getrange_index_t DCALL
mh_select_seq_operator_getrange_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return default__seq_operator_getrange_index__empty;
		if (self->tp_seq->tp_getitem_index_fast)
			return &default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
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
		if (seq_operator_trygetitem_index)
			return default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getrange_index_n_t DCALL
mh_select_seq_operator_getrange_index_n(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_getrange_index_t seq_operator_getrange_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return default__seq_operator_getrange_index_n__empty;
		seq_operator_getrange_index = (DeeMH_seq_operator_getrange_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getrange_index);
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast)
			return &default__seq_operator_getrange_index_n__with__seq_operator_size__and__operator_getitem_index_fast;
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
	DeeMH_seq_operator_setrange_index_t seq_operator_setrange_index = (DeeMH_seq_operator_setrange_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setrange_index);
	if (seq_operator_setrange_index == &default__seq_operator_setrange_index__empty)
		return &default__seq_operator_setrange__empty;
	if (seq_operator_setrange_index && (DeeMH_seq_operator_setrange_index_n_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setrange_index_n))
		return &default__seq_operator_setrange__with__seq_operator_setrange_index__and__seq_operator_setrange_index_n;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_setrange_index_t DCALL
mh_select_seq_operator_setrange_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
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
	DeeMH_seq_operator_setrange_index_t seq_operator_setrange_index = (DeeMH_seq_operator_setrange_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setrange_index);
	if (seq_operator_setrange_index == &default__seq_operator_setrange_index__empty)
		return default__seq_operator_setrange_index_n__empty;
	if (seq_operator_setrange_index == &default__seq_operator_setrange_index__with__seq_operator_setrange)
		return default__seq_operator_setrange_index_n__with__seq_operator_setrange;
	if (seq_operator_setrange_index)
		return default__seq_operator_setrange_index_n__with__seq_operator_setrange_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_assign_t DCALL
mh_select_seq_operator_assign(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_setrange_t seq_operator_setrange;
	seq_operator_setrange = (DeeMH_seq_operator_setrange_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setrange);
	if (seq_operator_setrange)
		return &default__seq_operator_assign__with__seq_operator_setrange;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_hash_t DCALL
mh_select_seq_operator_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_operator_hash__empty;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast)
		return &default__seq_operator_hash__with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &default__seq_operator_hash__with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index)
		return &default__seq_operator_hash__with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &default__seq_operator_hash__with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_operator_foreach)
		return default__seq_operator_hash__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_compare_t DCALL
mh_select_seq_operator_compare(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	DeeMH_seq_operator_size_t seq_operator_size;
	if ((DeeMH_seq_operator_eq_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_eq)) {
		if ((DeeMH_seq_operator_lo_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_lo))
			return &default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__lo;
		if ((DeeMH_seq_operator_le_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_le))
			return &default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__le;
		if ((DeeMH_seq_operator_gr_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_gr))
			return &default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__gr;
		if ((DeeMH_seq_operator_ge_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_ge))
			return &default__seq_operator_compare__with__seq_operator_eq__and__seq_operator__ge;
	} else if ((DeeMH_seq_operator_ne_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_ne)) {
		if ((DeeMH_seq_operator_lo_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_lo))
			return &default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__lo;
		if ((DeeMH_seq_operator_le_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_le))
			return &default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__le;
		if ((DeeMH_seq_operator_gr_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_gr))
			return &default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__gr;
		if ((DeeMH_seq_operator_ge_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_ge))
			return &default__seq_operator_compare__with__seq_operator_ne__and__seq_operator__ge;
	} else {
		if ((DeeMH_seq_operator_lo_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_lo) && (DeeMH_seq_operator_gr_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_gr))
			return &default__seq_operator_compare__with__seq_operator_lo__and__seq_operator__gr;
		if ((DeeMH_seq_operator_le_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_le) && (DeeMH_seq_operator_ge_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_ge))
			return &default__seq_operator_compare__with__seq_operator_le__and__seq_operator__ge;
	}

	seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_operator_compare__empty;
		if (self->tp_seq && self->tp_seq->tp_getitem_index_fast)
			return &default__seq_operator_compare__with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index) {
			if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
				return &default__seq_operator_compare__empty;
			if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach)
				return &default__seq_operator_compare__with__seq_operator_foreach;
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
	if ((DeeMH_seq_operator_eq_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_eq))
		return &default__seq_operator_compare_eq__with__seq_operator_eq;
	if ((DeeMH_seq_operator_ne_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_ne))
		return &default__seq_operator_compare_eq__with__seq_operator_ne;
	seq_operator_compare = (DeeMH_seq_operator_compare_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare);
	/*if (seq_operator_compare == &default__seq_operator_compare__empty)
		return &$empty;*/ /* Already optimally handled by fallback below */
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_foreach)
		return &default__seq_operator_compare_eq__with__seq_operator_foreach;
	if (seq_operator_compare == &default__seq_operator_compare__with__seq_operator_size__and__operator_getitem_index_fast)
		return &default__seq_operator_compare_eq__with__seq_operator_size__and__operator_getitem_index_fast;
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
	DeeMH_seq_operator_compare_eq_t seq_operator_compare_eq = (DeeMH_seq_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare_eq);
	if (seq_operator_compare_eq == &default__seq_operator_compare_eq__empty)
		return &default__seq_operator_trycompare_eq__empty;
	if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_foreach)
		return &default__seq_operator_trycompare_eq__with__seq_operator_foreach;
	if (seq_operator_compare_eq == &default__seq_operator_compare_eq__with__seq_operator_size__and__operator_getitem_index_fast)
		return &default__seq_operator_trycompare_eq__with__seq_operator_size__and__operator_getitem_index_fast;
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
	if ((DeeMH_seq_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare_eq))
		return &default__seq_operator_eq__with__seq_operator_compare_eq;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_ne_t DCALL
mh_select_seq_operator_ne(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare_eq))
		return &default__seq_operator_ne__with__seq_operator_compare_eq;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_lo_t DCALL
mh_select_seq_operator_lo(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_operator_compare_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare))
		return &default__seq_operator_lo__with__seq_operator_compare;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_le_t DCALL
mh_select_seq_operator_le(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_operator_compare_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare))
		return &default__seq_operator_le__with__seq_operator_compare;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_gr_t DCALL
mh_select_seq_operator_gr(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_operator_compare_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare))
		return &default__seq_operator_gr__with__seq_operator_compare;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_ge_t DCALL
mh_select_seq_operator_ge(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_operator_compare_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare))
		return &default__seq_operator_ge__with__seq_operator_compare;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_inplace_add_t DCALL
mh_select_seq_operator_inplace_add(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_extend_t seq_extend = (DeeMH_seq_extend_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_extend);
	if (seq_extend)
		return &default__seq_operator_inplace_add__with__seq_extend;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_inplace_mul_t DCALL
mh_select_seq_operator_inplace_mul(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_extend_t seq_extend = (DeeMH_seq_extend_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_extend);
	if (seq_extend) {
		DeeMH_seq_clear_t seq_clear = (DeeMH_seq_clear_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_clear);
		if (seq_clear == &default__seq_clear__empty)
			return &default__seq_operator_inplace_mul__empty;
		if (seq_clear != &default__seq_clear__unsupported)
			return &default__seq_operator_inplace_mul__with__seq_clear__and__seq_extend;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_enumerate_t DCALL
mh_select_seq_enumerate(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_getitem_t seq_operator_getitem;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_enumerate__empty;
		if (self->tp_seq && self->tp_seq->tp_getitem_index_fast)
			return &default__seq_enumerate__with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_getitem = (DeeMH_seq_operator_getitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem);
		if (seq_operator_getitem == &default__seq_operator_getitem__with__seq_operator_getitem_index)
			return &default__seq_enumerate__with__seq_operator_size__and__seq_operator_getitem_index;
		if (seq_operator_getitem)
			return &default__seq_enumerate__with__seq_operator_sizeob__and__seq_operator_getitem;
	}
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_enumerate__empty;
	if (seq_operator_foreach)
		return &default__seq_enumerate__with__counter__and__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_enumerate_index_t DCALL
mh_select_seq_enumerate_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (self->tp_seq && self->tp_seq->tp_getitem_index_fast)
			return &default__seq_enumerate_index__with__seq_operator_size__and__operator_getitem_index_fast;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_enumerate_index__empty;
		if ((DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index))
			return &default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_getitem_index;
	}
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_enumerate_index__empty;
	if (seq_operator_foreach)
		return &default__seq_enumerate_index__with__counter__and__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_makeenumeration_t DCALL
mh_select_seq_makeenumeration(DeeTypeObject *self, DeeTypeObject *orig_type) {
	/* TODO: All of this stuff shouldn't be using generic operators, but *specifically* sequence operators! */
	/* TODO: Also: the implementation variants here should be defined by magic! */
	if (DeeType_HasPrivateOperator(self, OPERATOR_ITER)) {
		unsigned int seqclass;
		if (self->tp_seq->tp_enumerate == &DeeObject_DefaultEnumerateWithIterKeysAndGetItem) {
			return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem;
		} else if (self->tp_seq->tp_enumerate == &DeeObject_DefaultEnumerateWithIterKeysAndTryGetItem ||
		           self->tp_seq->tp_enumerate == &DeeObject_DefaultEnumerateWithIterKeysAndTryGetItemDefault) {
			return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem;
		} else if (self->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndexFast) {
			return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast;
		} else if (self->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeAndGetItemIndex ||
		           self->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeDefaultAndGetItemIndexDefault) {
			return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex;
		} else if (self->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeAndTryGetItemIndex) {
			return &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex;
		} else if (self->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithSizeObAndGetItem) {
			return &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem;
		} else if (self->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithCounterAndForeach ||
		           self->tp_seq->tp_enumerate == &DeeSeq_DefaultEnumerateWithCounterAndIter) {
			if (self->tp_seq->tp_iter == &DeeObject_DefaultIterWithIterKeysAndGetItem ||
			    self->tp_seq->tp_iter == &DeeMap_DefaultIterWithIterKeysAndGetItem)
				return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem;
			if (self->tp_seq->tp_iter == &DeeObject_DefaultIterWithIterKeysAndTryGetItem ||
			    self->tp_seq->tp_iter == &DeeObject_DefaultIterWithIterKeysAndTryGetItemDefault ||
			    self->tp_seq->tp_iter == &DeeMap_DefaultIterWithIterKeysAndTryGetItem ||
			    self->tp_seq->tp_iter == &DeeMap_DefaultIterWithIterKeysAndTryGetItemDefault)
				return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem;
			if (self->tp_seq->tp_iter == &DeeSeq_DefaultIterWithSizeAndGetItemIndexFast)
				return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast;
			if (self->tp_seq->tp_iter == &DeeSeq_DefaultIterWithSizeAndGetItemIndex)
				return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex;
			if (self->tp_seq->tp_iter == &DeeSeq_DefaultIterWithSizeAndTryGetItemIndex)
				return &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex;
			if (self->tp_seq->tp_iter == &DeeSeq_DefaultIterWithSizeObAndGetItem)
				return &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem;
			if (self->tp_seq->tp_iter == &DeeSeq_DefaultIterWithGetItemIndex ||
			    self->tp_seq->tp_iter == &DeeSeq_DefaultIterWithGetItem)
				return &DeeSeq_DefaultMakeEnumerationWithGetItemIndex;
			return &DeeSeq_DefaultMakeEnumerationWithIterAndCounter;
		} else if (self->tp_seq->tp_enumerate == &DeeMap_DefaultEnumerateWithIter) {
			return &DeeMap_DefaultMakeEnumerationWithIterAndUnpack;
		}
		if (self->tp_seq->tp_iterkeys && !DeeType_IsDefaultIterKeys(self->tp_seq->tp_iterkeys)) {
			if (self->tp_seq->tp_iterkeys == &DeeMap_DefaultIterKeysWithIter ||
			    self->tp_seq->tp_iterkeys == &DeeMap_DefaultIterKeysWithIterDefault)
				return &DeeMap_DefaultMakeEnumerationWithIterAndUnpack;
			if (self->tp_seq->tp_iterkeys == &DeeObject_DefaultIterKeysWithEnumerate ||
			    self->tp_seq->tp_iterkeys == &DeeObject_DefaultIterKeysWithEnumerateIndex)
				return &DeeSeq_DefaultMakeEnumerationWithEnumerate;
			if (self->tp_seq->tp_iterkeys == &DeeSeq_DefaultIterKeysWithSizeOb)
				return &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem;
			if (self->tp_seq->tp_iterkeys == &DeeSeq_DefaultIterKeysWithSize ||
			    self->tp_seq->tp_iterkeys == &DeeSeq_DefaultIterKeysWithSizeDefault) {
				if (self->tp_seq->tp_trygetitem && (!DeeType_IsDefaultTryGetItem(self->tp_seq->tp_trygetitem) ||
				                                    !DeeType_IsDefaultTryGetItemIndex(self->tp_seq->tp_trygetitem_index)))
					return &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex;
				if (self->tp_seq->tp_getitem && (!DeeType_IsDefaultGetItem(self->tp_seq->tp_getitem) ||
				                                 !DeeType_IsDefaultGetItemIndex(self->tp_seq->tp_getitem_index)))
					return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex;
				if (self->tp_seq->tp_trygetitem)
					return &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex;
				if (self->tp_seq->tp_getitem)
					return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex;
			} else {
				if (self->tp_seq->tp_trygetitem && !DeeType_IsDefaultTryGetItem(self->tp_seq->tp_trygetitem))
					return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem;
				if (self->tp_seq->tp_getitem && !DeeType_IsDefaultGetItem(self->tp_seq->tp_getitem))
					return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem;
				if (self->tp_seq->tp_trygetitem)
					return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem;
				if (self->tp_seq->tp_getitem)
					return &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem;
			}
		}
		seqclass = DeeType_GetSeqClass(self);
		if (seqclass == Dee_SEQCLASS_SEQ) {
			if (DeeType_HasOperator(self, OPERATOR_SIZE)) {
				if (self->tp_seq->tp_getitem_index_fast)
					return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast;
				if (DeeType_HasOperator(self, OPERATOR_GETITEM)) {
					if (!DeeType_IsDefaultGetItemIndex(self->tp_seq->tp_getitem_index))
						return &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex;
					if (!DeeType_IsDefaultTryGetItemIndex(self->tp_seq->tp_trygetitem_index))
						return &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex;
					if (!DeeType_IsDefaultGetItem(self->tp_seq->tp_getitem))
						return &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem;
				}
			} else if (DeeType_HasOperator(self, OPERATOR_GETITEM)) {
				if (!DeeType_IsDefaultGetItemIndex(self->tp_seq->tp_getitem_index) ||
				    !DeeType_IsDefaultTryGetItemIndex(self->tp_seq->tp_trygetitem_index) ||
				    !DeeType_IsDefaultGetItem(self->tp_seq->tp_getitem))
					return &DeeSeq_DefaultMakeEnumerationWithGetItemIndex;
			}
			if (!DeeType_IsDefaultIter(self->tp_seq->tp_iter))
				return &DeeSeq_DefaultMakeEnumerationWithIterAndCounter;
		}
		if (self->tp_seq->tp_enumerate != NULL &&
		    self->tp_seq->tp_enumerate != self->tp_seq->tp_foreach_pair &&
		    !DeeType_IsDefaultEnumerate(self->tp_seq->tp_enumerate)) {
			return &DeeSeq_DefaultMakeEnumerationWithEnumerate;
		} else if (self->tp_seq->tp_enumerate_index != NULL &&
		           !DeeType_IsDefaultEnumerateIndex(self->tp_seq->tp_enumerate_index)) {
			return &DeeSeq_DefaultMakeEnumerationWithEnumerate;
		}
		if (seqclass == Dee_SEQCLASS_SEQ) {
			return &DeeSeq_DefaultMakeEnumerationWithIterAndCounter;
		} else if (seqclass == Dee_SEQCLASS_MAP) {
			return &DeeMap_DefaultMakeEnumerationWithIterAndUnpack;
		} else if (seqclass == Dee_SEQCLASS_NONE) {
			return &DeeSeq_DefaultMakeEnumerationWithIterAndCounter;
		}
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_makeenumeration_with_int_range_t DCALL
mh_select_seq_makeenumeration_with_int_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	/* TODO: All of this stuff shouldn't be using generic operators, but *specifically* sequence operators! */
	/* TODO: Also: the implementation variants here should be defined by magic! */
	DeeMH_seq_makeenumeration_t seq_makeenumeration = (DeeMH_seq_makeenumeration_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_makeenumeration);
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndGetItemIndexFastAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndTryGetItemIndexAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeAndGetItemIndexAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithSizeObAndGetItemAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithGetItemIndex)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithGetItemIndexAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterKeysAndTryGetItemAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterKeysAndGetItemAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithIterAndCounter)
		return &DeeSeq_DefaultMakeEnumerationWithIntRangeWithIterAndCounterAndFilter;
	if (seq_makeenumeration == &DeeMap_DefaultMakeEnumerationWithIterAndUnpack)
		return &DeeMap_DefaultMakeEnumerationWithIntRangeWithIterAndUnpackAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithEnumerate)
		return &DeeMap_DefaultMakeEnumerationWithIntRangeWithEnumerateIndex;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_makeenumeration_with_range_t DCALL
mh_select_seq_makeenumeration_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	/* TODO: All of this stuff shouldn't be using generic operators, but *specifically* sequence operators! */
	/* TODO: Also: the implementation variants here should be defined by magic! */
	DeeMH_seq_makeenumeration_t seq_makeenumeration = (DeeMH_seq_makeenumeration_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_makeenumeration);
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndexFast ||
	    seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeAndTryGetItemIndex ||
	    seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeAndGetItemIndex ||
	    seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithSizeObAndGetItem)
		return &DeeSeq_DefaultMakeEnumerationWithRangeWithSizeObAndGetItemAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithGetItemIndex)
		return &DeeSeq_DefaultMakeEnumerationWithRangeWithGetItemAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithIterKeysAndTryGetItem)
		return &DeeSeq_DefaultMakeEnumerationWithRangeWithIterKeysAndTryGetItemAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithIterKeysAndGetItem)
		return &DeeSeq_DefaultMakeEnumerationWithRangeWithIterKeysAndGetItemAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithIterAndCounter)
		return &DeeSeq_DefaultMakeEnumerationWithRangeWithIterAndCounterAndFilter;
	if (seq_makeenumeration == &DeeMap_DefaultMakeEnumerationWithIterAndUnpack)
		return &DeeMap_DefaultMakeEnumerationWithRangeWithIterAndUnpackAndFilter;
	if (seq_makeenumeration == &DeeSeq_DefaultMakeEnumerationWithEnumerate)
		return &DeeMap_DefaultMakeEnumerationWithRangeWithEnumerateAndFilter;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_foreach_reverse_t DCALL
mh_select_seq_foreach_reverse(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_foreach_reverse__empty;
		if (self->tp_seq && self->tp_seq->tp_getitem_index_fast)
			return &default__seq_foreach_reverse__with__seq_operator_size__and__getitem_index_fast;
		seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
			return &default__seq_foreach_reverse__empty;
		if (seq_operator_trygetitem_index == NULL)
			goto nope;
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach)
			goto nope;
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index) {
			DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
			seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
			ASSERT(seq_operator_getitem_index);
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
				return &default__seq_foreach_reverse__empty;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_foreach)
				goto nope;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_getitem ||
			    seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob)
				return &default__seq_foreach_reverse__with__seq_operator_sizeob__and__seq_operator_getitem;
			return &default__seq_foreach_reverse__with__seq_operator_size__and__seq_operator_getitem_index;
		}
		if (seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob)
			return &default__seq_foreach_reverse__with__seq_operator_sizeob__and__seq_operator_getitem;
		return &default__seq_foreach_reverse__with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
nope:;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_enumerate_index_reverse_t DCALL
mh_select_seq_enumerate_index_reverse(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_enumerate_index_reverse__empty;
		if (self->tp_seq && self->tp_seq->tp_getitem_index_fast)
			return &default__seq_enumerate_index_reverse__with__seq_operator_size__and__getitem_index_fast;
		seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
			return &default__seq_enumerate_index_reverse__empty;
		if (seq_operator_trygetitem_index == NULL)
			goto nope;
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach)
			goto nope;
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index) {
			DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
			seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
			ASSERT(seq_operator_getitem_index);
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
				return &default__seq_enumerate_index_reverse__empty;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_foreach)
				goto nope;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_getitem ||
			    seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob)
				return &default__seq_enumerate_index_reverse__with__seq_operator_sizeob__and__seq_operator_getitem;
			return &default__seq_enumerate_index_reverse__with__seq_operator_size__and__seq_operator_getitem_index;
		}
		if (seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob)
			return &default__seq_enumerate_index_reverse__with__seq_operator_sizeob__and__seq_operator_getitem;
		return &default__seq_enumerate_index_reverse__with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
nope:;
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
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_any_with_range__empty;
	if (seq_enumerate_index)
		return &default__seq_any_with_range__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_with_range_and_key_t DCALL
mh_select_seq_any_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_any_with_range_and_key__empty;
	if (seq_enumerate_index)
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
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_all_with_range__empty;
	if (seq_enumerate_index)
		return &default__seq_all_with_range__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_all_with_range_and_key_t DCALL
mh_select_seq_all_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_all_with_range_and_key__empty;
	if (seq_enumerate_index)
		return &default__seq_all_with_range_and_key__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_parity_t DCALL
mh_select_seq_parity(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if (Dee_SEQCLASS_ISSETORMAP(DeeType_GetSeqClass(self))) {
		DeeMH_seq_count_t seq_count = (DeeMH_seq_count_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_count);
		if (seq_count == &default__seq_count__empty)
			return &default__seq_parity__empty;
		if (seq_count == &default__seq_count__with__seq_operator_foreach)
			return &default__seq_parity__with__seq_operator_foreach;
		if (seq_count)
			return &default__seq_parity__with__seq_count;
	}
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_parity__empty;
	if (seq_operator_foreach)
		return &default__seq_parity__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_parity_with_key_t DCALL
mh_select_seq_parity_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_parity_with_key__empty;
	if (seq_operator_foreach)
		return &default__seq_parity_with_key__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_parity_with_range_t DCALL
mh_select_seq_parity_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index;
	if (Dee_SEQCLASS_ISSETORMAP(DeeType_GetSeqClass(self))) {
		DeeMH_seq_count_with_range_t seq_count_with_range = (DeeMH_seq_count_with_range_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_count_with_range);
		if (seq_count_with_range == &default__seq_count_with_range__empty)
			return &default__seq_parity_with_range__empty;
		if (seq_count_with_range == &default__seq_count_with_range__with__seq_enumerate_index)
			return &default__seq_parity_with_range__with__seq_enumerate_index;
		if (seq_count_with_range)
			return &default__seq_parity_with_range__with__seq_count_with_range;
	}
	seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_parity_with_range__empty;
	if (seq_enumerate_index)
		return &default__seq_parity_with_range__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_parity_with_range_and_key_t DCALL
mh_select_seq_parity_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_parity_with_range_and_key__empty;
	if (seq_enumerate_index)
		return &default__seq_parity_with_range_and_key__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_min_t DCALL
mh_select_seq_min(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_min__empty;
	if (seq_operator_foreach)
		return &default__seq_min__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_min_with_key_t DCALL
mh_select_seq_min_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_min_with_key__empty;
	if (seq_operator_foreach)
		return &default__seq_min_with_key__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_min_with_range_t DCALL
mh_select_seq_min_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_min_with_range__empty;
	if (seq_enumerate_index)
		return &default__seq_min_with_range__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_min_with_range_and_key_t DCALL
mh_select_seq_min_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_min_with_range_and_key__empty;
	if (seq_enumerate_index)
		return &default__seq_min_with_range_and_key__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_max_t DCALL
mh_select_seq_max(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_max__empty;
	if (seq_operator_foreach)
		return &default__seq_max__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_max_with_key_t DCALL
mh_select_seq_max_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_max_with_key__empty;
	if (seq_operator_foreach)
		return &default__seq_max_with_key__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_max_with_range_t DCALL
mh_select_seq_max_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_max_with_range__empty;
	if (seq_enumerate_index)
		return &default__seq_max_with_range__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_max_with_range_and_key_t DCALL
mh_select_seq_max_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_max_with_range_and_key__empty;
	if (seq_enumerate_index)
		return &default__seq_max_with_range_and_key__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_sum_t DCALL
mh_select_seq_sum(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_sum__empty;
	if (seq_operator_foreach)
		return &default__seq_sum__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_sum_with_range_t DCALL
mh_select_seq_sum_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_sum_with_range__empty;
	if (seq_enumerate_index)
		return &default__seq_sum_with_range__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_count_t DCALL
mh_select_seq_count(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_find_t seq_find;
	if (Dee_SEQCLASS_ISSETORMAP(DeeType_GetSeqClass(self))) {
		DeeMH_seq_operator_contains_t seq_operator_contains;
		seq_operator_contains = (DeeMH_seq_operator_contains_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_contains);
		if (seq_operator_contains)
			return &default__seq_count__with__set_operator_contains;
	}
	seq_find = (DeeMH_seq_find_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_find);
	if (seq_find == &default__seq_find__empty)
		return &default__seq_count__empty;
	if (seq_find == &default__seq_find__with__seq_enumerate_index)
		return &default__seq_count__with__seq_operator_foreach;
	if (seq_find)
		return &default__seq_count__with__seq_find;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_count_with_key_t DCALL
mh_select_seq_count_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_find_with_key_t seq_find_with_key;
	seq_find_with_key = (DeeMH_seq_find_with_key_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_find_with_key);
	if (seq_find_with_key == &default__seq_find_with_key__empty)
		return &default__seq_count_with_key__empty;
	if (seq_find_with_key == &default__seq_find_with_key__with__seq_enumerate_index)
		return &default__seq_count_with_key__with__seq_operator_foreach;
	if (seq_find_with_key)
		return &default__seq_count_with_key__with__seq_find_with_key;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_count_with_range_t DCALL
mh_select_seq_count_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_find_t seq_find;
	seq_find = (DeeMH_seq_find_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_find);
	if (seq_find == &default__seq_find__empty)
		return &default__seq_count_with_range__empty;
	if (seq_find == &default__seq_find__with__seq_enumerate_index)
		return &default__seq_count_with_range__with__seq_enumerate_index;
	if (seq_find)
		return &default__seq_count_with_range__with__seq_find;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_count_with_range_and_key_t DCALL
mh_select_seq_count_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_find_with_key_t seq_find_with_key;
	seq_find_with_key = (DeeMH_seq_find_with_key_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_find_with_key);
	if (seq_find_with_key == &default__seq_find_with_key__empty)
		return &default__seq_count_with_range_and_key__empty;
	if (seq_find_with_key == &default__seq_find_with_key__with__seq_enumerate_index)
		return &default__seq_count_with_range_and_key__with__seq_enumerate_index;
	if (seq_find_with_key)
		return &default__seq_count_with_range_and_key__with__seq_find_with_key;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_contains_t DCALL
mh_select_seq_contains(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_contains_t seq_operator_contains;
	DeeMH_seq_find_t seq_find;
	seq_operator_contains = (DeeMH_seq_operator_contains_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_contains);
	if (seq_operator_contains)
		return &default__seq_contains__with__seq_operator_contains;
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP) {
		DeeMH_map_operator_trygetitem_t map_operator_trygetitem;
		map_operator_trygetitem = (DeeMH_map_operator_trygetitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_trygetitem);
		if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
			return &default__seq_contains__empty;
		if (map_operator_trygetitem == &default__map_operator_trygetitem__with__map_enumerate)
			return &default__seq_contains__with__seq_operator_foreach;
		if (map_operator_trygetitem)
			return &default__seq_contains__with__map_operator_trygetitem;
	}
	seq_find = (DeeMH_seq_find_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_find);
	if (seq_find == &default__seq_find__empty)
		return &default__seq_contains__empty;
	if (seq_find == &default__seq_find__with__seq_enumerate_index)
		return &default__seq_contains__with__seq_operator_foreach;
	if (seq_find)
		return &default__seq_contains__with__seq_find;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_contains_with_key_t DCALL
mh_select_seq_contains_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_find_with_key_t seq_find_with_key = (DeeMH_seq_find_with_key_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_find_with_key);
	if (seq_find_with_key == &default__seq_find_with_key__empty)
		return &default__seq_contains_with_key__empty;
	if (seq_find_with_key == &default__seq_find_with_key__with__seq_enumerate_index)
		return &default__seq_contains_with_key__with__seq_operator_foreach;
	if (seq_find_with_key)
		return &default__seq_contains_with_key__with__seq_find_with_key;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_contains_with_range_t DCALL
mh_select_seq_contains_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_find_t seq_find = (DeeMH_seq_find_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_find);
	if (seq_find == &default__seq_find__empty)
		return &default__seq_contains_with_range__empty;
	if (seq_find == &default__seq_find__with__seq_enumerate_index)
		return &default__seq_contains_with_range__with__seq_enumerate_index;
	if (seq_find)
		return &default__seq_contains_with_range__with__seq_find;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_contains_with_range_and_key_t DCALL
mh_select_seq_contains_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_find_with_key_t seq_find_with_key = (DeeMH_seq_find_with_key_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_find_with_key);
	if (seq_find_with_key == &default__seq_find_with_key__empty)
		return &default__seq_contains_with_range_and_key__empty;
	if (seq_find_with_key == &default__seq_find_with_key__with__seq_enumerate_index)
		return &default__seq_contains_with_range_and_key__with__seq_enumerate_index;
	if (seq_find_with_key)
		return &default__seq_contains_with_range_and_key__with__seq_find_with_key;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_contains_t DCALL
mh_select_seq_operator_contains(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_contains_t seq_contains = (DeeMH_seq_contains_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_contains);
	if (seq_contains == &default__seq_contains__empty)
		return &default__seq_operator_contains__empty;
	if (seq_contains)
		return &default__seq_operator_contains__with__seq_contains;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_locate_t DCALL
mh_select_seq_locate(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_locate__empty;
	if (seq_operator_foreach)
		return &default__seq_locate__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_locate_with_range_t DCALL
mh_select_seq_locate_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_locate_with_range__empty;
	if (seq_enumerate_index)
		return &default__seq_locate_with_range__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_rlocate_t DCALL
mh_select_seq_rlocate(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if ((DeeMH_seq_foreach_reverse_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_foreach_reverse))
		return &default__seq_rlocate__with__seq_foreach_reverse;
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_rlocate__empty;
	if (seq_operator_foreach)
		return &default__seq_rlocate__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_rlocate_with_range_t DCALL
mh_select_seq_rlocate_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index;
	if ((DeeMH_seq_enumerate_index_reverse_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index_reverse))
		return &default__seq_rlocate_with_range__with__seq_enumerate_index_reverse;
	seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_rlocate_with_range__empty;
	if (seq_enumerate_index)
		return &default__seq_rlocate_with_range__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_startswith_t DCALL
mh_select_seq_startswith(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_trygetfirst_t seq_trygetfirst;
	seq_trygetfirst = (DeeMH_seq_trygetfirst_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_trygetfirst);
	if (seq_trygetfirst == &default__seq_trygetfirst__empty)
		return &default__seq_startswith__empty;
	if (seq_trygetfirst)
		return &default__seq_startswith__with__seq_trygetfirst;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_startswith_with_key_t DCALL
mh_select_seq_startswith_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_trygetfirst_t seq_trygetfirst;
	seq_trygetfirst = (DeeMH_seq_trygetfirst_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_trygetfirst);
	if (seq_trygetfirst == &default__seq_trygetfirst__empty)
		return &default__seq_startswith_with_key__empty;
	if (seq_trygetfirst)
		return &default__seq_startswith_with_key__with__seq_trygetfirst;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_startswith_with_range_t DCALL
mh_select_seq_startswith_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &default__seq_startswith_with_range__empty;
	if (seq_operator_trygetitem_index)
		return &default__seq_startswith_with_range__with__seq_operator_trygetitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_startswith_with_range_and_key_t DCALL
mh_select_seq_startswith_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &default__seq_startswith_with_range_and_key__empty;
	if (seq_operator_trygetitem_index)
		return &default__seq_startswith_with_range_and_key__with__seq_operator_trygetitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_endswith_t DCALL
mh_select_seq_endswith(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_trygetlast_t seq_trygetlast;
	seq_trygetlast = (DeeMH_seq_trygetlast_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_trygetlast);
	if (seq_trygetlast == &default__seq_trygetlast__empty)
		return &default__seq_endswith__empty;
	if (seq_trygetlast)
		return &default__seq_endswith__with__seq_trygetlast;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_endswith_with_key_t DCALL
mh_select_seq_endswith_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_trygetlast_t seq_trygetlast;
	seq_trygetlast = (DeeMH_seq_trygetlast_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_trygetlast);
	if (seq_trygetlast == &default__seq_trygetlast__empty)
		return &default__seq_endswith_with_key__empty;
	if (seq_trygetlast)
		return &default__seq_endswith_with_key__with__seq_trygetlast;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_endswith_with_range_t DCALL
mh_select_seq_endswith_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &default__seq_endswith_with_range__empty;
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
			return &default__seq_endswith_with_range__empty;
		if (seq_operator_trygetitem_index)
			return &default__seq_endswith_with_range__with__seq_operator_size__and__operator_trygetitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_endswith_with_range_and_key_t DCALL
mh_select_seq_endswith_with_range_and_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &default__seq_endswith_with_range_and_key__empty;
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
			return &default__seq_endswith_with_range_and_key__empty;
		if (seq_operator_trygetitem_index)
			return &default__seq_endswith_with_range_and_key__with__seq_operator_size__and__operator_trygetitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_find_t DCALL
mh_select_seq_find(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_find__empty;
	if (seq_enumerate_index)
		return &default__seq_find__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_find_with_key_t DCALL
mh_select_seq_find_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_find_with_key__empty;
	if (seq_enumerate_index)
		return &default__seq_find_with_key__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_rfind_t DCALL
mh_select_seq_rfind(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index;
	if ((DeeMH_seq_enumerate_index_reverse_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index_reverse))
		return &default__seq_rfind__with__seq_enumerate_index_reverse;
	seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_rfind__empty;
	if (seq_enumerate_index)
		return &default__seq_rfind__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_rfind_with_key_t DCALL
mh_select_seq_rfind_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index;
	if ((DeeMH_seq_enumerate_index_reverse_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index_reverse))
		return &default__seq_rfind_with_key__with__seq_enumerate_index_reverse;
	seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_rfind_with_key__empty;
	if (seq_enumerate_index)
		return &default__seq_rfind_with_key__with__seq_enumerate_index;
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
	DeeMH_seq_operator_iter_t seq_operator_iter = (DeeMH_seq_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty)
		return &default__set_operator_iter__empty;
	if (seq_operator_iter)
		return &default__set_operator_iter__with__seq_operator_iter;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_foreach_t DCALL
mh_select_set_operator_foreach(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_iter_t set_operator_iter = (DeeMH_set_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_iter);
	if (set_operator_iter == &default__set_operator_iter__empty)
		return &default__set_operator_foreach__empty;
	if (set_operator_iter == &default__set_operator_iter__with__seq_operator_iter)
		return &default__set_operator_foreach__with__seq_operator_foreach;
	if (set_operator_iter)
		return &default__set_operator_foreach__with__set_operator_iter;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_foreach_pair_t DCALL
mh_select_set_operator_foreach_pair(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_foreach_t set_operator_foreach = (DeeMH_set_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &default__set_operator_foreach_pair__empty;
	if (set_operator_foreach)
		return &default__set_operator_foreach_pair__with__set_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_sizeob_t DCALL
mh_select_set_operator_sizeob(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_size_t set_operator_size = (DeeMH_set_operator_size_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_size);
	if (set_operator_size == &default__set_operator_size__empty)
		return &default__set_operator_sizeob__empty;
	if (set_operator_size)
		return &default__set_operator_sizeob__with__set_operator_size;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_size_t DCALL
mh_select_set_operator_size(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_foreach_t set_operator_foreach;
	if ((DeeMH_set_operator_sizeob_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_sizeob))
		return &default__set_operator_size__with__set_operator_sizeob;
	set_operator_foreach = (DeeMH_set_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &default__set_operator_size__empty;
	if (set_operator_foreach)
		return default__set_operator_size__with__set_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_hash_t DCALL
mh_select_set_operator_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_foreach_t set_operator_foreach = (DeeMH_set_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &default__set_operator_hash__empty;
	if (set_operator_foreach)
		return default__set_operator_hash__with__set_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_getitem_t DCALL
mh_select_map_operator_getitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_enumerate_t map_enumerate;
	if ((DeeMH_map_operator_getitem_string_len_hash_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_getitem_string_len_hash)) {
		return (DeeMH_map_operator_getitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_getitem_index)
		       ? &default__map_operator_getitem__with__map_operator_getitem_index__and__map_operator_getitem_string_len_hash
		       : &default__map_operator_getitem__with__map_operator_getitem_string_len_hash;
	} else if ((DeeMH_map_operator_getitem_string_hash_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_getitem_string_hash)) {
		return (DeeMH_map_operator_getitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_getitem_index)
		       ? &default__map_operator_getitem__with__map_operator_getitem_index__and__map_operator_getitem_string_hash
		       : &default__map_operator_getitem__with__map_operator_getitem_string_hash;
	} else if ((DeeMH_map_operator_getitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_getitem_index)) {
		return &default__map_operator_getitem__with__map_operator_getitem_index;
	}
	map_enumerate = (DeeMH_map_enumerate_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_enumerate);
	if (map_enumerate == &default__map_enumerate__empty)
		return &default__map_operator_getitem__empty;
	if (map_enumerate)
		return &default__map_operator_getitem__with__map_enumerate;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_trygetitem_t DCALL
mh_select_map_operator_trygetitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_getitem_t map_operator_getitem;
	if ((DeeMH_map_operator_trygetitem_string_len_hash_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_trygetitem_string_len_hash)) {
		return (DeeMH_map_operator_trygetitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_trygetitem_index)
		       ? &default__map_operator_trygetitem__with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_len_hash
		       : &default__map_operator_trygetitem__with__map_operator_trygetitem_string_len_hash;
	} else if ((DeeMH_map_operator_trygetitem_string_hash_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_trygetitem_string_hash)) {
		return (DeeMH_map_operator_trygetitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_trygetitem_index)
		       ? &default__map_operator_trygetitem__with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_hash
		       : &default__map_operator_trygetitem__with__map_operator_trygetitem_string_hash;
	} else if ((DeeMH_map_operator_trygetitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_trygetitem_index)) {
		return &default__map_operator_trygetitem__with__map_operator_trygetitem_index;
	}
	map_operator_getitem = (DeeMH_map_operator_getitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &default__map_operator_trygetitem__empty;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_enumerate)
		return &default__map_operator_trygetitem__with__map_enumerate;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_getitem_index__and__map_operator_getitem_string_len_hash)
		return &default__map_operator_trygetitem__with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_len_hash;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_getitem_index__and__map_operator_getitem_string_hash)
		return &default__map_operator_trygetitem__with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_hash;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_getitem_string_len_hash)
		return &default__map_operator_trygetitem__with__map_operator_trygetitem_string_len_hash;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_getitem_string_hash)
		return &default__map_operator_trygetitem__with__map_operator_trygetitem_string_hash;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_operator_getitem_index)
		return &default__map_operator_trygetitem__with__map_operator_trygetitem_index;
	if (map_operator_getitem)
		return &default__map_operator_trygetitem__with__map_operator_getitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_getitem_index_t DCALL
mh_select_map_operator_getitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_getitem_t map_operator_getitem = (DeeMH_map_operator_getitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &default__map_operator_getitem_index__empty;
	if (map_operator_getitem)
		return &default__map_operator_getitem_index__with__map_operator_getitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_trygetitem_index_t DCALL
mh_select_map_operator_trygetitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_trygetitem_t map_operator_trygetitem = (DeeMH_map_operator_trygetitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_trygetitem);
	if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
		return &default__map_operator_trygetitem_index__empty;
	if (map_operator_trygetitem)
		return &default__map_operator_trygetitem_index__with__map_operator_trygetitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_getitem_string_hash_t DCALL
mh_select_map_operator_getitem_string_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_getitem_t map_operator_getitem = (DeeMH_map_operator_getitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &default__map_operator_getitem_string_hash__empty;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_enumerate)
		return &default__map_operator_getitem_string_hash__with__map_enumerate;
	if (map_operator_getitem)
		return &default__map_operator_getitem_string_hash__with__map_operator_getitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_trygetitem_string_hash_t DCALL
mh_select_map_operator_trygetitem_string_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_trygetitem_t map_operator_trygetitem = (DeeMH_map_operator_trygetitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_trygetitem);
	if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
		return &default__map_operator_trygetitem_string_hash__empty;
	if (map_operator_trygetitem == &default__map_operator_trygetitem__with__map_enumerate)
		return &default__map_operator_trygetitem_string_hash__with__map_enumerate;
	if (map_operator_trygetitem)
		return &default__map_operator_trygetitem_string_hash__with__map_operator_trygetitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_getitem_string_len_hash_t DCALL
mh_select_map_operator_getitem_string_len_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_getitem_t map_operator_getitem = (DeeMH_map_operator_getitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &default__map_operator_getitem_string_len_hash__empty;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_enumerate)
		return &default__map_operator_getitem_string_len_hash__with__map_enumerate;
	if (map_operator_getitem)
		return &default__map_operator_getitem_string_len_hash__with__map_operator_getitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_trygetitem_string_len_hash_t DCALL
mh_select_map_operator_trygetitem_string_len_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_trygetitem_t map_operator_trygetitem = (DeeMH_map_operator_trygetitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_trygetitem);
	if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
		return &default__map_operator_trygetitem_string_len_hash__empty;
	if (map_operator_trygetitem == &default__map_operator_trygetitem__with__map_enumerate)
		return &default__map_operator_trygetitem_string_len_hash__with__map_enumerate;
	if (map_operator_trygetitem)
		return &default__map_operator_trygetitem_string_len_hash__with__map_operator_trygetitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_bounditem_t DCALL
mh_select_map_operator_bounditem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_contains_t map_operator_contains;
	DeeMH_map_operator_getitem_t map_operator_getitem;
	map_operator_contains = (DeeMH_map_operator_contains_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_contains);
	if (map_operator_contains) {
		if (map_operator_contains == &default__map_operator_contains__empty)
			return &default__map_operator_bounditem__empty;
		return &default__map_operator_bounditem__with__map_operator_contains;
	}
	map_operator_getitem = (DeeMH_map_operator_getitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_getitem);
	if (map_operator_getitem == &default__map_operator_getitem__empty)
		return &default__map_operator_bounditem__empty;
	if (map_operator_getitem == &default__map_operator_getitem__with__map_enumerate)
		return &default__map_operator_bounditem__with__map_enumerate;
	if (map_operator_getitem)
		return &default__map_operator_bounditem__with__map_operator_getitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_bounditem_index_t DCALL
mh_select_map_operator_bounditem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_bounditem_t map_operator_bounditem = (DeeMH_map_operator_bounditem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_bounditem);
	if (map_operator_bounditem == &default__map_operator_bounditem__empty)
		return &default__map_operator_bounditem_index__empty;
	if (map_operator_bounditem == &default__map_operator_bounditem__with__map_operator_getitem)
		return &default__map_operator_bounditem_index__with__map_operator_getitem_index;
	if (map_operator_bounditem)
		return &default__map_operator_bounditem_index__with__map_operator_bounditem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_bounditem_string_hash_t DCALL
mh_select_map_operator_bounditem_string_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_bounditem_t map_operator_bounditem = (DeeMH_map_operator_bounditem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_bounditem);
	if (map_operator_bounditem == &default__map_operator_bounditem__empty)
		return &default__map_operator_bounditem_string_hash__empty;
	if (map_operator_bounditem == &default__map_operator_bounditem__with__map_operator_getitem)
		return &default__map_operator_bounditem_string_hash__with__map_operator_getitem_string_hash;
	if (map_operator_bounditem)
		return &default__map_operator_bounditem_string_hash__with__map_operator_bounditem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_bounditem_string_len_hash_t DCALL
mh_select_map_operator_bounditem_string_len_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_bounditem_t map_operator_bounditem = (DeeMH_map_operator_bounditem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_bounditem);
	if (map_operator_bounditem == &default__map_operator_bounditem__empty)
		return &default__map_operator_bounditem_string_len_hash__empty;
	if (map_operator_bounditem == &default__map_operator_bounditem__with__map_operator_getitem)
		return &default__map_operator_bounditem_string_len_hash__with__map_operator_getitem_string_len_hash;
	if (map_operator_bounditem)
		return &default__map_operator_bounditem_string_len_hash__with__map_operator_bounditem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_hasitem_t DCALL
mh_select_map_operator_hasitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_bounditem_t map_operator_bounditem = (DeeMH_map_operator_bounditem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_bounditem);
	if (map_operator_bounditem) {
#ifdef Dee_BOUND_MAYALIAS_HAS
		return map_operator_bounditem;
#else /* Dee_BOUND_MAYALIAS_HAS */
		return &default__map_operator_hasitem__with__map_operator_bounditem;
#endif /* !Dee_BOUND_MAYALIAS_HAS */
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_hasitem_index_t DCALL
mh_select_map_operator_hasitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_bounditem_index_t map_operator_bounditem_index = (DeeMH_map_operator_bounditem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_bounditem_index);
	if (map_operator_bounditem_index) {
#ifdef Dee_BOUND_MAYALIAS_HAS
		return map_operator_bounditem_index;
#else /* Dee_BOUND_MAYALIAS_HAS */
		return &default__map_operator_hasitem_index__with__map_operator_bounditem_index;
#endif /* !Dee_BOUND_MAYALIAS_HAS */
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_hasitem_string_hash_t DCALL
mh_select_map_operator_hasitem_string_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_bounditem_string_hash_t map_operator_bounditem_string_hash = (DeeMH_map_operator_bounditem_string_hash_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_bounditem_string_hash);
	if (map_operator_bounditem_string_hash) {
#ifdef Dee_BOUND_MAYALIAS_HAS
		return map_operator_bounditem_string_hash;
#else /* Dee_BOUND_MAYALIAS_HAS */
		return &default__map_operator_hasitem_string_hash__with__map_operator_bounditem_string_hash;
#endif /* !Dee_BOUND_MAYALIAS_HAS */
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_hasitem_string_len_hash_t DCALL
mh_select_map_operator_hasitem_string_len_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_bounditem_string_len_hash_t map_operator_bounditem_string_len_hash = (DeeMH_map_operator_bounditem_string_len_hash_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_bounditem_string_len_hash);
	if (map_operator_bounditem_string_len_hash) {
#ifdef Dee_BOUND_MAYALIAS_HAS
		return map_operator_bounditem_string_len_hash;
#else /* Dee_BOUND_MAYALIAS_HAS */
		return &default__map_operator_hasitem_string_len_hash__with__map_operator_bounditem_string_len_hash;
#endif /* !Dee_BOUND_MAYALIAS_HAS */
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_delitem_t DCALL
mh_select_map_operator_delitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_enumerate_t map_enumerate;
	if ((DeeMH_map_operator_delitem_string_len_hash_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_delitem_string_len_hash)) {
		return (DeeMH_map_operator_delitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_delitem_index)
		       ? &default__map_operator_delitem__with__map_operator_delitem_index__and__map_operator_delitem_string_len_hash
		       : &default__map_operator_delitem__with__map_operator_delitem_string_len_hash;
	} else if ((DeeMH_map_operator_delitem_string_hash_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_delitem_string_hash)) {
		return (DeeMH_map_operator_delitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_delitem_index)
		       ? &default__map_operator_delitem__with__map_operator_delitem_index__and__map_operator_delitem_string_hash
		       : &default__map_operator_delitem__with__map_operator_delitem_string_hash;
	} else if ((DeeMH_map_operator_delitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_delitem_index)) {
		return &default__map_operator_delitem__with__map_operator_delitem_index;
	}
	map_enumerate = (DeeMH_map_enumerate_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_enumerate);
	if (map_enumerate == &default__map_enumerate__empty)
		return &default__map_operator_delitem__empty;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_delitem_index_t DCALL
mh_select_map_operator_delitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_delitem_t map_operator_delitem = (DeeMH_map_operator_delitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_delitem);
	if (map_operator_delitem == &default__map_operator_delitem__empty)
		return &default__map_operator_delitem_index__empty;
	if (map_operator_delitem)
		return &default__map_operator_delitem_index__with__map_operator_delitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_delitem_string_hash_t DCALL
mh_select_map_operator_delitem_string_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_delitem_t map_operator_delitem = (DeeMH_map_operator_delitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_delitem);
	if (map_operator_delitem == &default__map_operator_delitem__empty)
		return &default__map_operator_delitem_string_hash__empty;
	if (map_operator_delitem)
		return &default__map_operator_delitem_string_hash__with__map_operator_delitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_delitem_string_len_hash_t DCALL
mh_select_map_operator_delitem_string_len_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_delitem_t map_operator_delitem = (DeeMH_map_operator_delitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_delitem);
	if (map_operator_delitem == &default__map_operator_delitem__empty)
		return &default__map_operator_delitem_string_len_hash__empty;
	if (map_operator_delitem)
		return &default__map_operator_delitem_string_len_hash__with__map_operator_delitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_setitem_t DCALL
mh_select_map_operator_setitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_enumerate_t map_enumerate;
	if ((DeeMH_map_operator_setitem_string_len_hash_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_setitem_string_len_hash)) {
		return (DeeMH_map_operator_setitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_setitem_index)
		       ? &default__map_operator_setitem__with__map_operator_setitem_index__and__map_operator_setitem_string_len_hash
		       : &default__map_operator_setitem__with__map_operator_setitem_string_len_hash;
	} else if ((DeeMH_map_operator_setitem_string_hash_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_setitem_string_hash)) {
		return (DeeMH_map_operator_setitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_setitem_index)
		       ? &default__map_operator_setitem__with__map_operator_setitem_index__and__map_operator_setitem_string_hash
		       : &default__map_operator_setitem__with__map_operator_setitem_string_hash;
	} else if ((DeeMH_map_operator_setitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_setitem_index)) {
		return &default__map_operator_setitem__with__map_operator_setitem_index;
	}
	map_enumerate = (DeeMH_map_enumerate_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_enumerate);
	if (map_enumerate == &default__map_enumerate__empty)
		return &default__map_operator_setitem__empty;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_setitem_index_t DCALL
mh_select_map_operator_setitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_setitem_t map_operator_setitem = (DeeMH_map_operator_setitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_setitem);
	if (map_operator_setitem == &default__map_operator_setitem__empty)
		return &default__map_operator_setitem_index__empty;
	if (map_operator_setitem)
		return &default__map_operator_setitem_index__with__map_operator_setitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_setitem_string_hash_t DCALL
mh_select_map_operator_setitem_string_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_setitem_t map_operator_setitem = (DeeMH_map_operator_setitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_setitem);
	if (map_operator_setitem == &default__map_operator_setitem__empty)
		return &default__map_operator_setitem_string_hash__empty;
	if (map_operator_setitem)
		return &default__map_operator_setitem_string_hash__with__map_operator_setitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_setitem_string_len_hash_t DCALL
mh_select_map_operator_setitem_string_len_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_setitem_t map_operator_setitem = (DeeMH_map_operator_setitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_setitem);
	if (map_operator_setitem == &default__map_operator_setitem__empty)
		return &default__map_operator_setitem_string_len_hash__empty;
	if (map_operator_setitem)
		return &default__map_operator_setitem_string_len_hash__with__map_operator_setitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_contains_t DCALL
mh_select_map_operator_contains(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_trygetitem_t map_operator_trygetitem;
	DeeMH_map_operator_bounditem_t map_operator_bounditem = (DeeMH_map_operator_bounditem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_bounditem);
	if (map_operator_bounditem) {
		if (map_operator_bounditem == &default__map_operator_bounditem__empty)
			return &default__map_operator_contains__empty;
		if (map_operator_bounditem != &default__map_operator_bounditem__with__map_operator_getitem)
			return &default__map_operator_contains__with__map_operator_bounditem;
	}
	map_operator_trygetitem = (DeeMH_map_operator_trygetitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_trygetitem);
	if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
		return &default__map_operator_contains__empty;
	if (map_operator_trygetitem)
		return &default__map_operator_contains__with__map_operator_trygetitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_iterkeys_t DCALL
mh_select_map_iterkeys(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_enumerate_t map_enumerate = (DeeMH_map_enumerate_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_enumerate);
	if (map_enumerate == &default__map_enumerate__empty)
		return &default__map_iterkeys__empty;
	if (map_enumerate != (DeeMH_set_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_foreach_pair)) {
		DeeMH_set_operator_iter_t set_operator_iter = (DeeMH_set_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_iter);
		if (set_operator_iter == &default__set_operator_iter__empty)
			return &default__map_iterkeys__empty;
		if (set_operator_iter)
			return &default__map_iterkeys__with__set_operator_iter;
	}
	if (map_enumerate)
		return &default__map_iterkeys__with__map_enumerate;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_enumerate_t DCALL
mh_select_map_enumerate(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_foreach_pair_t set_operator_foreach_pair;
	DeeMH_map_iterkeys_t map_iterkeys = (DeeMH_map_iterkeys_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_iterkeys);
	if (map_iterkeys) {
		if (map_iterkeys == &default__map_iterkeys__empty)
			return &default__map_enumerate__empty;
		if (map_iterkeys != &default__map_iterkeys__with__set_operator_iter) {
			DeeMH_map_operator_trygetitem_t map_operator_trygetitem = (DeeMH_map_operator_trygetitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_trygetitem);
			if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
				return &default__map_enumerate__empty;
			if (map_operator_trygetitem)
				return &default__map_enumerate__with__map_iterkeys__and__map_operator_trygetitem;
		}
	}
	set_operator_foreach_pair = (DeeMH_set_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_foreach_pair);
	if (set_operator_foreach_pair)
		return set_operator_foreach_pair; /* Binary-compatible */
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_enumerate_range_t DCALL
mh_select_map_enumerate_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_enumerate_t map_enumerate = (DeeMH_map_enumerate_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_enumerate);
	if (map_enumerate == &default__map_enumerate__empty)
		return &default__map_enumerate_range__empty;
	if (map_enumerate == &default__map_enumerate__with__map_iterkeys__and__map_operator_trygetitem)
		return &default__map_enumerate_range__with__map_iterkeys__and__map_operator_trygetitem;
	if (map_enumerate)
		return &default__map_enumerate_range__with__map_enumerate;
	return NULL;
}
/*[[[end]]]*/
/* clang-format on */

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_SELECT_C */
