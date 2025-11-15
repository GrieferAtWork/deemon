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
#include <deemon/method-hints.h>
#include <deemon/none-operator.h>
#include <deemon/object.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>

/**/
#include "method-hint-defaults.h"
#include "method-hint-select.h"
#include "method-hints.h"

DECL_BEGIN

/* clang-format off */
/*[[[deemon (printMhInitSelectImpls from "..method-hints.method-hints")();]]]*/
INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_bool_t DCALL
mh_select_seq_operator_bool(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size;
	DeeMH_seq_operator_compare_eq_t seq_operator_compare_eq;
	DeeMH_set_operator_compare_eq_t set_operator_compare_eq;
	DeeMH_map_operator_compare_eq_t map_operator_compare_eq;

	seq_operator_compare_eq = (DeeMH_seq_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare_eq);
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

	set_operator_compare_eq = (DeeMH_set_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_compare_eq);
	if (set_operator_compare_eq) {
		if (set_operator_compare_eq == &default__set_operator_compare_eq__empty)
			return &default__seq_operator_bool__empty;
		if (set_operator_compare_eq == &default__set_operator_compare_eq__with__set_operator_foreach)
			goto use_size; /* return &$with__seq_operator_foreach; */
		return &default__seq_operator_bool__with__set_operator_compare_eq;
	}

	map_operator_compare_eq = (DeeMH_map_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_compare_eq);
	if (map_operator_compare_eq) {
		if (map_operator_compare_eq == &default__map_operator_compare_eq__empty)
			return &default__seq_operator_bool__empty;
		if (map_operator_compare_eq == &default__map_operator_compare_eq__with__map_operator_foreach_pair)
			goto use_size; /* return &$with__seq_operator_foreach; */
		return &default__seq_operator_bool__with__map_operator_compare_eq;
	}

use_size:
	seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &default__seq_operator_bool__empty;
	if (seq_operator_size == &default__seq_operator_size__with__seq_operator_iter)
		return &default__seq_operator_bool__with__seq_operator_iter;
	if (seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach)
		return &default__seq_operator_bool__with__seq_operator_foreach;
	if (seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach_pair)
		return &default__seq_operator_bool__with__seq_operator_foreach_pair;
	if (seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob)
		return &default__seq_operator_bool__with__seq_operator_sizeob;
	if (seq_operator_size)
		return &default__seq_operator_bool__with__seq_operator_size;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_sizeob_t DCALL
mh_select_seq_operator_sizeob(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_operator_sizeob__empty;
		if (seq_operator_size == &default__seq_operator_size__with__set_operator_sizeob ||
		    seq_operator_size == (DeeMH_set_operator_size_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_size))
			return (DeeMH_set_operator_sizeob_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_sizeob);
		if (seq_operator_size == &default__seq_operator_size__with__map_operator_sizeob ||
		    seq_operator_size == (DeeMH_map_operator_size_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_size))
			return (DeeMH_map_operator_sizeob_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_sizeob);
		if (seq_operator_size == &default__seq_operator_size__with__seq_enumerate_index) {
			if ((DeeMH_seq_enumerate_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_enumerate))
				return &default__seq_operator_sizeob__with__seq_enumerate;
		}
		return &default__seq_operator_sizeob__with__seq_operator_size;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_size_t DCALL
mh_select_seq_operator_size(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	DeeMH_set_operator_size_t set_operator_size;
	DeeMH_map_operator_size_t map_operator_size;
	DeeMH_map_enumerate_t map_enumerate;
	if ((set_operator_size = (DeeMH_set_operator_size_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_size)) != NULL)
		return set_operator_size;
	if ((map_operator_size = (DeeMH_map_operator_size_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_size)) != NULL)
		return map_operator_size;
	if ((DeeMH_seq_operator_sizeob_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_sizeob))
		return &default__seq_operator_size__with__seq_operator_sizeob;
	if ((DeeMH_set_operator_sizeob_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_sizeob))
		return &default__seq_operator_size__with__set_operator_sizeob;
	if ((DeeMH_map_operator_sizeob_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_sizeob))
		return &default__seq_operator_size__with__map_operator_sizeob;

	/* Check for special case: is actually a map, and map items may be unbound */
	map_enumerate = (DeeMH_map_enumerate_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_enumerate);
	if (((map_enumerate && map_enumerate != (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_foreach_pair)) ||
	     (DeeMH_map_keys_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_keys) || (DeeMH_map_iterkeys_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_iterkeys)) &&
	    !DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___map_getitem_always_bound__))
		return &default__seq_operator_size__with__map_enumerate;

	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_operator_size__empty;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_foreach_pair) {
		DeeMH_seq_operator_foreach_pair_t seq_operator_foreach_pair = (DeeMH_seq_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach_pair);
		if (seq_operator_foreach_pair == &default__seq_operator_foreach_pair__with__seq_operator_iter)
			return &default__seq_operator_size__with__seq_operator_iter;
		return &default__seq_operator_size__with__seq_operator_foreach_pair;
	}
	if (seq_operator_foreach == &default__seq_operator_foreach__with__map_enumerate)
		return &default__seq_operator_size__with__map_enumerate;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_enumerate ||
	    seq_operator_foreach == &default__seq_operator_foreach__with__seq_enumerate_index)
		return &default__seq_operator_size__with__seq_enumerate_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_iter)
		return &default__seq_operator_size__with__seq_operator_iter;
	if (seq_operator_foreach)
		return &default__seq_operator_size__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_iter_t DCALL
mh_select_seq_operator_iter(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_iter_t map_operator_iter;
	DeeMH_set_operator_iter_t set_operator_iter;
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_operator_iter__empty;
with_seq_operator_size:
		if (self->tp_seq && self->tp_seq->tp_getitem_index_fast)
			return &default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index) {
			if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
				return &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index;
			return &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index;
		}
		if ((DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_getitem_index))
			return &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index;
		if ((DeeMH_seq_operator_getitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_getitem) || (DeeMH_seq_operator_trygetitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_trygetitem))
			return &default__seq_operator_iter__with__seq_operator_sizeob__and__seq_operator_getitem;
	} else {
		DeeMH_seq_operator_sizeob_t seq_operator_sizeob = (DeeMH_seq_operator_sizeob_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_sizeob);
		if (seq_operator_sizeob) {
			if (seq_operator_sizeob == &default__seq_operator_sizeob__empty)
				return &default__seq_operator_iter__empty;
			goto with_seq_operator_size;
		}
		if ((DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_getitem_index) ||
		    (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_trygetitem_index))
			return &default__seq_operator_iter__with__seq_operator_getitem_index;
		if ((DeeMH_seq_operator_getitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_getitem) ||
		    (DeeMH_seq_operator_trygetitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_trygetitem))
			return &default__seq_operator_iter__with__seq_operator_getitem;
	}

	/* If provided, can also use explicitly defined "Set.operator iter()" */
	set_operator_iter = (DeeMH_set_operator_iter_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_iter);
	if (set_operator_iter)
		return set_operator_iter;

	/* If provided, can also use explicitly defined "Mapping.operator iter()" */
	map_operator_iter = (DeeMH_map_operator_iter_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_iter);
	if (map_operator_iter)
		return map_operator_iter;

	if ((DeeMH_map_iterkeys_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_iterkeys) || (DeeMH_map_keys_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_keys)) {
		if ((DeeMH_map_operator_trygetitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_trygetitem))
			return &default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem;
		if ((DeeMH_map_operator_getitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_getitem))
			return &default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem;
	}
	if ((DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_enumerate_index) ||
	    (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_foreach) ||
	    (DeeMH_seq_operator_foreach_pair_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_foreach_pair))
		return &default__seq_operator_iter__with__seq_enumerate_index;
	if ((DeeMH_seq_enumerate_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_enumerate))
		return &default__seq_operator_iter__with__seq_enumerate;
	if ((DeeMH_map_enumerate_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_enumerate) || (DeeMH_map_enumerate_range_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_enumerate_range))
		return &default__seq_operator_iter__with__map_enumerate;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_foreach_t DCALL
mh_select_seq_operator_foreach(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_iter_t seq_operator_iter;
	/*if (REQUIRE_NODEFAULT(seq_operator_foreach_pair))
		return &$with__seq_operator_foreach_pair;*/
	if ((DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_foreach_pair))
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
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_getitem_index)
		return &default__seq_operator_foreach__with__seq_operator_getitem_index;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_getitem)
		return &default__seq_operator_foreach__with__seq_operator_getitem;
	if (seq_operator_iter == &default__seq_operator_iter__with__map_enumerate)
		return &default__seq_operator_foreach__with__map_enumerate;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_enumerate_index)
		return &default__seq_operator_foreach__with__seq_enumerate_index;
	if (seq_operator_iter == &default__seq_operator_iter__with__seq_enumerate)
		return &default__seq_operator_foreach__with__seq_enumerate;
	if (seq_operator_iter) {
		if (seq_operator_iter == (DeeMH_set_operator_iter_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_iter)) {
			DeeMH_set_operator_foreach_t set_operator_foreach = (DeeMH_set_operator_foreach_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_foreach);
			if (set_operator_foreach)
				return set_operator_foreach;
		}
		return &default__seq_operator_foreach__with__seq_operator_iter;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_foreach_pair_t DCALL
mh_select_seq_operator_foreach_pair(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair = (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_foreach_pair);
	if (map_operator_foreach_pair)
		return map_operator_foreach_pair;
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
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
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_enumerate_index) {
		if ((DeeMH_seq_enumerate_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_enumerate))
			return &default__seq_operator_getitem__with__seq_enumerate;
	}
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
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem ||
	    seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_getitem)
		return &default__seq_operator_getitem_index__with__seq_operator_getitem;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &default__seq_operator_getitem_index__with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_enumerate ||
	    seq_operator_foreach == &default__seq_operator_foreach__with__seq_enumerate_index)
		return &default__seq_operator_getitem_index__with__seq_enumerate_index;
	if (seq_operator_foreach) {
		if ((DeeMH_seq_operator_getitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_getitem))
			return &default__seq_operator_getitem_index__with__seq_operator_getitem;
		/* Check if "seq_operator_foreach" may possibly be skipping unbound items. */
		if ((DeeMH_seq_enumerate_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_enumerate) ||
		    (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_enumerate_index)) {
			if (!DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___seq_getitem_always_bound__))
				return &default__seq_operator_getitem_index__with__seq_enumerate_index;
		} else {
			DeeMH_map_enumerate_t map_enumerate = (DeeMH_map_enumerate_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_enumerate);
			if ((map_enumerate && map_enumerate != (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_foreach_pair)) ||
			    (DeeMH_map_keys_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_keys) ||
			    (DeeMH_map_iterkeys_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_iterkeys)) {
				if (!DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___map_getitem_always_bound__))
					return &default__seq_operator_getitem_index__with__map_enumerate;
			}
		}
		return &default__seq_operator_getitem_index__with__seq_operator_foreach;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_trygetitem_t DCALL
mh_select_seq_operator_trygetitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
		return &default__seq_operator_trygetitem__empty;
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
		return &default__seq_operator_trygetitem__with__seq_operator_getitem;
	if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_enumerate_index) {
		if ((DeeMH_seq_enumerate_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_enumerate))
			return &default__seq_operator_trygetitem__with__seq_enumerate;
	}
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
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_enumerate_index)
		return &default__seq_operator_trygetitem_index__with__seq_enumerate_index;
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
	if (seq_operator_hasitem_index == &default__seq_operator_hasitem_index__with__seq_operator_size) {
		DeeMH_seq_operator_sizeob_t seq_operator_sizeob = (DeeMH_seq_operator_sizeob_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_sizeob);
		if (seq_operator_sizeob == &default__seq_operator_sizeob__empty)
			return &default__seq_operator_hasitem__empty;
		if (seq_operator_sizeob == &default__seq_operator_sizeob__with__seq_operator_size)
			return &default__seq_operator_hasitem__with__seq_operator_hasitem_index; /* This way, sizeob isn't called, and no int-object gets created */
		return &default__seq_operator_hasitem__with__seq_operator_sizeob;
	}
	if (seq_operator_hasitem_index == &default__seq_operator_hasitem_index__with__seq_enumerate_index) {
		if ((DeeMH_seq_enumerate_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_enumerate))
			return &default__seq_operator_hasitem__with__seq_enumerate;
	}
	if (seq_operator_hasitem_index)
		return &default__seq_operator_hasitem__with__seq_operator_hasitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_hasitem_index_t DCALL
mh_select_seq_operator_hasitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_size);
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
	if (DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___seq_getitem_always_bound__)) {
		if ((DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported)
			return &default__seq_operator_hasitem_index__with__seq_operator_size;
	}
	if (seq_operator_size == &default__seq_operator_size__empty)
		return &default__seq_operator_hasitem_index__empty;
	if (seq_operator_size != &default__seq_operator_size__with__seq_operator_foreach &&
	    seq_operator_size != &default__seq_operator_size__with__seq_operator_iter)
		return &default__seq_operator_hasitem_index__with__seq_operator_size;
	if (seq_operator_size == &default__seq_operator_size__with__seq_enumerate_index)
		return &default__seq_operator_hasitem_index__with__seq_enumerate_index;
	seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &default__seq_operator_hasitem_index__empty;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_enumerate_index)
		return &default__seq_operator_hasitem_index__with__seq_enumerate_index;
	if (seq_operator_getitem_index)
		return &default__seq_operator_hasitem_index__with__seq_operator_getitem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_bounditem_t DCALL
mh_select_seq_operator_bounditem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_bounditem_index_t seq_operator_bounditem_index;
	if (DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___seq_getitem_always_bound__)) {
		/* TODO: Optimizations */
	}
	seq_operator_bounditem_index = (DeeMH_seq_operator_bounditem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_bounditem_index);
	if (seq_operator_bounditem_index == &default__seq_operator_bounditem_index__empty)
		return &default__seq_operator_bounditem__empty;
	if (seq_operator_bounditem_index == &default__seq_operator_bounditem_index__with__seq_operator_getitem_index)
		return &default__seq_operator_bounditem__with__seq_operator_getitem;
	if (seq_operator_bounditem_index == &default__seq_operator_bounditem_index__with__seq_enumerate_index) {
		if ((DeeMH_seq_enumerate_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_enumerate))
			return &default__seq_operator_bounditem__with__seq_enumerate;
	}
	if (seq_operator_bounditem_index)
		return &default__seq_operator_bounditem__with__seq_operator_bounditem_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_bounditem_index_t DCALL
mh_select_seq_operator_bounditem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
	if (DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___seq_getitem_always_bound__)) {
		/* TODO: Optimizations */
	}
	seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
		return &default__seq_operator_bounditem_index__empty;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__map_enumerate)
		return &default__seq_operator_bounditem_index__with__map_enumerate;
	if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_enumerate_index)
		return &default__seq_operator_bounditem_index__with__seq_enumerate_index;
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
			return &default__seq_operator_getrange_index__empty;
		if (self->tp_seq && self->tp_seq->tp_getitem_index_fast)
			return &default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
			return &default__seq_operator_getrange_index__empty;
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_foreach) {
			if ((DeeMH_seq_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_iter))
				return &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter;
		}
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index) {
			DeeMH_seq_operator_getitem_t seq_operator_getitem = (DeeMH_seq_operator_getitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem);
			if (seq_operator_getitem == &default__seq_operator_getitem__with__seq_operator_getitem_index)
				return &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index;
			if (seq_operator_getitem == &default__seq_operator_getitem__empty)
				return &default__seq_operator_getrange_index__empty;
			return &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem;
		}
		if (seq_operator_trygetitem_index)
			return &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_getrange_index_n_t DCALL
mh_select_seq_operator_getrange_index_n(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_getrange_index_t seq_operator_getrange_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_operator_getrange_index_n__empty;
		seq_operator_getrange_index = (DeeMH_seq_operator_getrange_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getrange_index);
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast)
			return &default__seq_operator_getrange_index_n__with__seq_operator_size__and__operator_getitem_index_fast;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__empty)
			return &default__seq_operator_getrange_index_n__empty;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem_index)
			return &default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem_index;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index)
			return &default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_getitem)
			return &default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getitem;
		if (seq_operator_getrange_index == &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter)
			return &default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter;
		if (seq_operator_getrange_index)
			return &default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_getrange_index;
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
		return &default__seq_operator_delrange_index_n__empty;
	if (seq_operator_delrange_index == &default__seq_operator_delrange_index__with__seq_operator_delrange)
		return &default__seq_operator_delrange_index_n__with__seq_operator_delrange;
	if (seq_operator_delrange_index &&
	    (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported)
		return &default__seq_operator_delrange_index_n__with__seq_operator_size__and__seq_operator_delrange_index;
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
		return &default__seq_operator_setrange_index_n__empty;
	if (seq_operator_setrange_index == &default__seq_operator_setrange_index__with__seq_operator_setrange)
		return &default__seq_operator_setrange_index_n__with__seq_operator_setrange;
	if (seq_operator_setrange_index &&
	    (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported)
		return &default__seq_operator_setrange_index_n__with__seq_operator_size__and__seq_operator_setrange_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_assign_t DCALL
mh_select_seq_operator_assign(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_setrange_t seq_operator_setrange;
	seq_operator_setrange = (DeeMH_seq_operator_setrange_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setrange);
	if (seq_operator_setrange == &default__seq_operator_setrange__empty)
		return &default__seq_operator_assign__empty;
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
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_foreach_pair)
		return &default__seq_operator_hash__with__seq_operator_foreach_pair;
	if (seq_operator_foreach) {
		if ((DeeMH_seq_operator_foreach_pair_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_foreach_pair) ||
		    (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_foreach_pair))
			return &default__seq_operator_hash__with__seq_operator_foreach_pair;
		return &default__seq_operator_hash__with__seq_operator_foreach;
	}
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
	if (seq_operator_compare_eq)
		return &default__seq_operator_trycompare_eq__with__seq_operator_compare_eq;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_eq_t DCALL
mh_select_seq_operator_eq(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_operator_ne_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_ne))
		return &default__seq_operator_eq__with__seq_operator_ne;
	if ((DeeMH_seq_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare_eq))
		return &default__seq_operator_eq__with__seq_operator_compare_eq;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_ne_t DCALL
mh_select_seq_operator_ne(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_operator_eq_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_eq))
		return &default__seq_operator_ne__with__seq_operator_eq;
	if ((DeeMH_seq_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare_eq))
		return &default__seq_operator_ne__with__seq_operator_compare_eq;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_lo_t DCALL
mh_select_seq_operator_lo(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_operator_ge_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_ge))
		return &default__seq_operator_lo__with__seq_operator_ge;
	if ((DeeMH_seq_operator_compare_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare))
		return &default__seq_operator_lo__with__seq_operator_compare;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_le_t DCALL
mh_select_seq_operator_le(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_operator_gr_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_gr))
		return &default__seq_operator_le__with__seq_operator_gr;
	if ((DeeMH_seq_operator_compare_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare))
		return &default__seq_operator_le__with__seq_operator_compare;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_gr_t DCALL
mh_select_seq_operator_gr(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_operator_le_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_le))
		return &default__seq_operator_gr__with__seq_operator_le;
	if ((DeeMH_seq_operator_compare_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare))
		return &default__seq_operator_gr__with__seq_operator_compare;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_ge_t DCALL
mh_select_seq_operator_ge(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_operator_lo_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_lo))
		return &default__seq_operator_ge__with__seq_operator_lo;
	if ((DeeMH_seq_operator_compare_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_compare))
		return &default__seq_operator_ge__with__seq_operator_compare;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_inplace_add_t DCALL
mh_select_seq_operator_inplace_add(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_extend_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_extend))
		return &default__seq_operator_inplace_add__with__seq_extend;
	if ((DeeMH_seq_operator_add_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_add))
		return &default__seq_operator_inplace_add__with__seq_operator_add;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_operator_inplace_mul_t DCALL
mh_select_seq_operator_inplace_mul(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_extend_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_extend)) {
		DeeMH_seq_clear_t seq_clear = (DeeMH_seq_clear_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_clear);
		if (seq_clear == &default__seq_clear__empty)
			return &default__seq_operator_inplace_mul__empty;
		if (seq_clear != &default__seq_clear__unsupported)
			return &default__seq_operator_inplace_mul__with__seq_clear__and__seq_extend;
	}
	if ((DeeMH_seq_operator_mul_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_mul))
		return &default__seq_operator_inplace_mul__with__seq_operator_mul;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_enumerate_t DCALL
mh_select_seq_enumerate(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	DeeMH_seq_operator_size_t seq_operator_size;
	/*if (REQUIRE_NODEFAULT(seq_enumerate_index))
		return &$with__seq_enumerate_index;*/
	seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_getitem_t seq_operator_getitem;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_enumerate__empty;
		if (self->tp_seq && self->tp_seq->tp_getitem_index_fast)
			return &default__seq_enumerate__with__seq_operator_size__and__operator_getitem_index_fast;
		if (seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach)
			goto use_seq_operator_foreach;
		if (seq_operator_size == &default__seq_operator_size__with__seq_operator_iter)
			goto use_seq_operator_foreach;
		if (seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach_pair)
			goto use_seq_operator_foreach;
		seq_operator_getitem = (DeeMH_seq_operator_getitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem);
		if (seq_operator_getitem == &default__seq_operator_getitem__with__seq_operator_getitem_index) {
			DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
			seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
				return &default__seq_enumerate__empty;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_foreach)
				goto use_seq_operator_foreach;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_size__and__seq_operator_trygetitem_index)
				return &default__seq_enumerate__with__seq_operator_size__and__seq_operator_trygetitem_index;
			return &default__seq_enumerate__with__seq_operator_size__and__seq_operator_getitem_index;
		} else if (seq_operator_getitem) {
			return &default__seq_enumerate__with__seq_operator_sizeob__and__seq_operator_getitem;
		}
	}
use_seq_operator_foreach:
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach) {
		if (seq_operator_foreach == &default__seq_operator_foreach__empty)
			return &default__seq_enumerate__empty;
		if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast)
			return &default__seq_enumerate__with__seq_operator_size__and__operator_getitem_index_fast;
		if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index)
			return &default__seq_enumerate__with__seq_operator_size__and__seq_operator_trygetitem_index;
		if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index)
			return &default__seq_enumerate__with__seq_operator_size__and__seq_operator_getitem_index;
		if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_sizeob__and__seq_operator_getitem)
			return &default__seq_enumerate__with__seq_operator_sizeob__and__seq_operator_getitem;
		if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_getitem_index)
			return &default__seq_enumerate__with__seq_operator_getitem_index;
		if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_getitem)
			return &default__seq_enumerate__with__seq_operator_getitem;
		return &default__seq_enumerate__with__seq_operator_foreach__and__counter;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_enumerate_index_t DCALL
mh_select_seq_enumerate_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_t seq_enumerate = (DeeMH_seq_enumerate_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate);
	if (seq_enumerate == &default__seq_enumerate__empty)
		return &default__seq_enumerate_index__empty;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__operator_getitem_index_fast)
		return &default__seq_enumerate_index__with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__seq_operator_getitem_index ||
	    seq_enumerate == &default__seq_enumerate__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_getitem_index ||
	    seq_enumerate == &default__seq_enumerate__with__seq_operator_getitem)
		return &default__seq_enumerate_index__with__seq_operator_getitem_index;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_foreach__and__counter) {
		DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
		if (seq_operator_foreach == &default__seq_operator_foreach__empty)
			return &default__seq_enumerate_index__empty;
		if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_iter)
			return &default__seq_enumerate_index__with__seq_operator_iter;
		return &default__seq_enumerate_index__with__seq_operator_foreach__and__counter;
	}
	if (seq_enumerate)
		return &default__seq_enumerate_index__with__seq_enumerate;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_makeenumeration_t DCALL
mh_select_seq_makeenumeration(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_t seq_enumerate = (DeeMH_seq_enumerate_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate);
	if (seq_enumerate == &default__seq_enumerate__empty)
		return &default__seq_makeenumeration__empty;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__operator_getitem_index_fast)
		return &default__seq_makeenumeration__with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__seq_operator_getitem_index)
		return &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &default__seq_makeenumeration__with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_getitem_index)
		return &default__seq_makeenumeration__with__seq_operator_getitem_index;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_getitem)
		return &default__seq_makeenumeration__with__seq_operator_getitem;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_foreach__and__counter)
		return &default__seq_makeenumeration__with__seq_operator_iter__and__counter;
	if (seq_enumerate) {
		/* The "$with__seq_enumerate" impl is super-inefficient
		 * See if we can *maybe* still use one of the "$with__*size*__and__*getitem*" impls. */
		DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_makeenumeration__empty;
		if (seq_operator_size != &default__seq_operator_size__unsupported &&
		    seq_operator_size != &default__seq_operator_size__with__seq_operator_iter &&
		    seq_operator_size != &default__seq_operator_size__with__seq_operator_foreach &&
		    seq_operator_size != &default__seq_operator_size__with__seq_operator_foreach_pair &&
		    seq_operator_size != &default__seq_operator_size__with__map_enumerate &&
		    seq_operator_size != &default__seq_operator_size__with__seq_enumerate_index) {
			DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
			if (seq_operator_getitem_index) {
				/* Yes! We *do* also have a getitem operator!
				 * Now just make sure that it's O(1), and then we can actually use it! */
				if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
					return &default__seq_makeenumeration__empty;
				if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_getitem) {
					DeeMH_seq_operator_getitem_t seq_operator_getitem = (DeeMH_seq_operator_getitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem);
					if (seq_operator_getitem == &default__seq_operator_getitem__empty)
						return &default__seq_makeenumeration__empty;
					return &default__seq_makeenumeration__with__seq_operator_sizeob__and__seq_operator_getitem;
				}
				if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_foreach)
					goto use__seq_enumerate;
				if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__map_enumerate)
					goto use__seq_enumerate;
				if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_size__and__seq_operator_trygetitem_index)
					return &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_trygetitem_index;
				if (self->tp_seq && self->tp_seq->tp_getitem_index_fast)
					return &default__seq_makeenumeration__with__seq_operator_size__and__operator_getitem_index_fast;
				return &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_getitem_index;
			}
		}
use__seq_enumerate:
		return &default__seq_makeenumeration__with__seq_enumerate;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_makeenumeration_with_range_t DCALL
mh_select_seq_makeenumeration_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_makeenumeration_t seq_makeenumeration;
	if ((DeeMH_seq_makeenumeration_with_intrange_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_makeenumeration_with_intrange))
		return &default__seq_makeenumeration_with_range__with__seq_makeenumeration_with_intrange;
	seq_makeenumeration = (DeeMH_seq_makeenumeration_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_makeenumeration);

	if (seq_makeenumeration == &default__seq_makeenumeration__empty)
		return &default__seq_makeenumeration_with_range__empty;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__operator_getitem_index_fast ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_trygetitem_index ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_getitem_index ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_getitem_index ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_iter__and__counter ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_enumerate)
		return &default__seq_makeenumeration_with_range__with__seq_makeenumeration_with_intrange;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &default__seq_makeenumeration_with_range__with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_getitem)
		return &default__seq_makeenumeration_with_range__with__seq_operator_getitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_makeenumeration_with_intrange_t DCALL
mh_select_seq_makeenumeration_with_intrange(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_makeenumeration_t seq_makeenumeration;
	if ((DeeMH_seq_makeenumeration_with_range_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_makeenumeration_with_range))
		return &default__seq_makeenumeration_with_intrange__with__seq_makeenumeration_with_range;

	seq_makeenumeration = (DeeMH_seq_makeenumeration_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_makeenumeration);
	if (seq_makeenumeration == &default__seq_makeenumeration__empty)
		return &default__seq_makeenumeration_with_intrange__empty;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__operator_getitem_index_fast)
		return &default__seq_makeenumeration_with_intrange__with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &default__seq_makeenumeration_with_intrange__with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_size__and__seq_operator_getitem_index)
		return &default__seq_makeenumeration_with_intrange__with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_sizeob__and__seq_operator_getitem ||
	    seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_getitem)
		return &default__seq_makeenumeration_with_intrange__with__seq_makeenumeration_with_range;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_getitem_index)
		return &default__seq_makeenumeration_with_intrange__with__seq_operator_getitem_index;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_operator_iter__and__counter)
		return &default__seq_makeenumeration_with_intrange__with__seq_operator_iter__and__counter;
	if (seq_makeenumeration == &default__seq_makeenumeration__with__seq_enumerate)
		return &default__seq_makeenumeration_with_intrange__with__seq_enumerate_index;
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
			return &default__seq_foreach_reverse__with__seq_operator_size__and__operator_getitem_index_fast;
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
			if (seq_operator_getitem_index == default__seq_operator_getitem_index__with__seq_operator_size__and__seq_operator_trygetitem_index)
				return &default__seq_foreach_reverse__with__seq_operator_size__and__seq_operator_trygetitem_index;
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
			return &default__seq_enumerate_index_reverse__with__seq_operator_size__and__operator_getitem_index_fast;
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
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_size__and__seq_operator_trygetitem_index)
				return &default__seq_enumerate_index_reverse__with__seq_operator_size__and__seq_operator_trygetitem_index;
			return &default__seq_enumerate_index_reverse__with__seq_operator_size__and__seq_operator_getitem_index;
		}
		if (seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob)
			return &default__seq_enumerate_index_reverse__with__seq_operator_sizeob__and__seq_operator_getitem;
		return &default__seq_enumerate_index_reverse__with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
nope:;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_unpack_t DCALL
mh_select_seq_unpack(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_unpack_ex_t seq_unpack_ex = (DeeMH_seq_unpack_ex_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_unpack_ex);
	if (seq_unpack_ex == &default__seq_unpack_ex__empty)
		return &default__seq_unpack__empty;
	if (seq_unpack_ex == &default__seq_unpack_ex__with__tp_asvector)
		return &default__seq_unpack__with__tp_asvector;
	if (seq_unpack_ex == &default__seq_unpack_ex__with__seq_operator_size__and__operator_getitem_index_fast)
		return &default__seq_unpack__with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_unpack_ex == &default__seq_unpack_ex__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &default__seq_unpack__with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_unpack_ex == &default__seq_unpack_ex__with__seq_operator_size__and__seq_operator_getitem_index)
		return &default__seq_unpack__with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_unpack_ex == &default__seq_unpack_ex__with__seq_operator_foreach)
		return &default__seq_unpack__with__seq_operator_foreach;
	if (seq_unpack_ex == &default__seq_unpack_ex__with__seq_operator_iter)
		return &default__seq_unpack__with__seq_operator_iter;
	if (seq_unpack_ex)
		return &default__seq_unpack__with__seq_unpack_ex;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_unpack_ex_t DCALL
mh_select_seq_unpack_ex(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if (self->tp_seq && self->tp_seq->tp_asvector)
		return &default__seq_unpack_ex__with__tp_asvector;

	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_unpack_ex__empty;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast &&
	    DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___seq_getitem_always_bound__))
		return &default__seq_unpack_ex__with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index &&
	    DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___seq_getitem_always_bound__))
		return &default__seq_unpack_ex__with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index &&
	    DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___seq_getitem_always_bound__))
		return &default__seq_unpack_ex__with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_iter)
		return &default__seq_unpack_ex__with__seq_operator_iter;
	if (seq_operator_foreach)
		return &default__seq_unpack_ex__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_unpack_ub_t DCALL
mh_select_seq_unpack_ub(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if (DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___seq_getitem_always_bound__))
		return (DeeMH_seq_unpack_ex_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_unpack_ex); /* Can just re-use the regular `seq_unpack_ex' */
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_unpack_ub__empty;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast)
		return &default__seq_unpack_ub__with__seq_operator_size__and__operator_getitem_index_fast;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &default__seq_unpack_ub__with__seq_operator_size__and__seq_operator_trygetitem_index;
	if (seq_operator_foreach == &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_getitem_index)
		return &default__seq_unpack_ub__with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_operator_foreach)
		return (DeeMH_seq_unpack_ex_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_unpack_ex);
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_trygetfirst_t DCALL
mh_select_seq_trygetfirst(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
	if ((DeeMH_seq_getfirst_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_getfirst))
		return &default__seq_trygetfirst__with__seq_getfirst;
	if (self->tp_seq && self->tp_seq->tp_getitem_index_fast &&
	    (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported)
		return &default__seq_trygetfirst__with__seq_operator_size__and__operator_getitem_index_fast;
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
	if ((DeeMH_seq_getlast_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_getlast))
		return &default__seq_trygetlast__with__seq_getlast;
	if (self->tp_seq &&
	    self->tp_seq->tp_getitem_index_fast &&
	    self->tp_seq->tp_size)
		return &default__seq_trygetlast__with__seq_operator_size__and__operator_getitem_index_fast;
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

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_cached_t DCALL
mh_select_seq_cached(DeeTypeObject *self, DeeTypeObject *orig_type) {
	/* TODO:
	 * if (REQUIRE_NODEFAULT(map_cached))
	 *     return <deemon>Mapping.cached(this) as Sequence</deemon>;
	 * if (REQUIRE_NODEFAULT(set_cached))
	 *     return <deemon>Set.cached(this) as Sequence</deemon>;
	 */
	DeeMH_seq_enumerate_t seq_enumerate = (DeeMH_seq_enumerate_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate);
	if (seq_enumerate == &default__seq_enumerate__empty)
		return &default__seq_cached__empty;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__operator_getitem_index_fast ||
	    seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__seq_operator_getitem_index ||
	    seq_enumerate == &default__seq_enumerate__with__seq_operator_size__and__seq_operator_trygetitem_index)
		return &default__seq_cached__with__seq_operator_size__and__seq_operator_getitem_index;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_sizeob__and__seq_operator_getitem)
		return &default__seq_cached__with__seq_operator_sizeob__and__seq_operator_getitem;
	if (seq_enumerate == &default__seq_enumerate__with__seq_operator_getitem_index ||
	    seq_enumerate == &default__seq_enumerate__with__seq_operator_getitem)
		return &default__seq_cached__with__seq_operator_getitem;
	if (seq_enumerate == &default__seq_enumerate__with__seq_enumerate_index) {
		DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
		if (seq_enumerate_index == &default__seq_enumerate_index__empty)
			return &default__seq_cached__empty;
		if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_size__and__operator_getitem_index_fast ||
		    seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_getitem_index ||
		    seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_size__and__seq_operator_trygetitem_index)
			return &default__seq_cached__with__seq_operator_size__and__seq_operator_getitem_index;
		if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_getitem_index)
			return &default__seq_cached__with__seq_operator_getitem;
		if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_iter ||
		    seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_foreach__and__counter)
			goto use_seq_operator_iter;
	} else if (seq_enumerate == &default__seq_enumerate__with__seq_operator_foreach__and__counter) {
		DeeMH_seq_operator_iter_t seq_operator_iter;
use_seq_operator_iter:
		seq_operator_iter = (DeeMH_seq_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_iter);
		if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast ||
		    seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index ||
		    seq_operator_iter == &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_getitem_index)
			return &default__seq_cached__with__seq_operator_size__and__seq_operator_getitem_index;
		if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_sizeob__and__seq_operator_getitem)
			return &default__seq_cached__with__seq_operator_sizeob__and__seq_operator_getitem;
		if (seq_operator_iter == &default__seq_operator_iter__with__seq_operator_getitem_index ||
		    seq_operator_iter == &default__seq_operator_iter__with__seq_operator_getitem)
			return &default__seq_cached__with__seq_operator_getitem;
		if (seq_operator_iter == &default__seq_operator_iter__with__map_enumerate ||
		    seq_operator_iter == &default__seq_operator_iter__with__seq_enumerate_index)
			return &default__seq_cached__with__seq_enumerate_index;
		if (seq_operator_iter == &default__seq_operator_iter__with__seq_enumerate)
			return &default__seq_cached__with__seq_enumerate;
		return &default__seq_cached__with__seq_operator_iter;
	}
	if (seq_enumerate) {
		DeeMH_seq_operator_size_t seq_operator_size;
		DeeMH_seq_operator_getitem_t seq_operator_getitem;
		if (DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___seq_getitem_always_bound__))
			goto use_seq_operator_iter;
		seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_size);
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_cached__empty;
		if (seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach ||
		    seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach_pair ||
		    seq_operator_size == &default__seq_operator_size__with__seq_operator_iter)
			goto use_seq_operator_iter;
		seq_operator_getitem = (DeeMH_seq_operator_getitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem);
		if (seq_operator_getitem == &default__seq_operator_getitem__empty)
			return &default__seq_cached__empty;
		if (seq_operator_getitem == &default__seq_operator_getitem__with__seq_operator_getitem_index) {
			DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_getitem_index);
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
				return &default__seq_cached__empty;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_foreach)
				goto use_seq_operator_iter;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_operator_size__and__seq_operator_trygetitem_index)
				return &default__seq_cached__with__seq_operator_size__and__seq_operator_getitem_index;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__with__map_enumerate ||
			    seq_operator_getitem_index == &default__seq_operator_getitem_index__with__seq_enumerate_index)
				return &default__seq_cached__with__seq_enumerate_index;
		}
		if (seq_operator_size == &default__seq_operator_size__with__seq_operator_sizeob) {
			DeeMH_seq_operator_sizeob_t seq_operator_sizeob = (DeeMH_seq_operator_sizeob_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_sizeob);
			if (seq_operator_sizeob == &default__seq_operator_sizeob__empty)
				return &default__seq_cached__empty;
			if (seq_operator_sizeob == &default__seq_operator_sizeob__with__seq_enumerate)
				return &default__seq_cached__with__seq_enumerate;
		}
		if (seq_operator_size == &default__seq_operator_size__with__seq_enumerate_index)
			return &default__seq_cached__with__seq_enumerate_index;
		if (seq_operator_getitem == &default__seq_operator_getitem__with__seq_enumerate)
			return &default__seq_cached__with__seq_enumerate;
		if (seq_operator_size && seq_operator_getitem)
			return &default__seq_cached__with__seq_operator_size__and__seq_operator_getitem_index;
		return &default__seq_cached__with__seq_enumerate;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_frozen_t DCALL
mh_select_seq_frozen(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index;
	if ((DeeMH_set_frozen_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_frozen))
		return &default__seq_frozen__with__set_frozen;
	if ((DeeMH_map_frozen_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_frozen))
		return &default__seq_frozen__with__map_frozen;
	seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_frozen__empty;
	if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_iter ||
	    seq_enumerate_index == &default__seq_enumerate_index__with__seq_operator_foreach__and__counter)
		return &default__seq_frozen__with__seq_operator_foreach;
	if (seq_enumerate_index == &default__seq_enumerate_index__with__seq_enumerate) {
		DeeMH_seq_enumerate_t seq_enumerate = (DeeMH_seq_enumerate_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate);
		if (seq_enumerate == &default__seq_enumerate__empty)
			return &default__seq_frozen__empty;
		if (seq_enumerate == &default__seq_enumerate__with__seq_operator_foreach__and__counter)
			return &default__seq_frozen__with__seq_operator_foreach;
	}
	if (seq_enumerate_index) {
		if (DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___seq_getitem_always_bound__))
			return &default__seq_frozen__with__seq_operator_foreach;
		return &default__seq_frozen__with__seq_enumerate_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_any_t DCALL
mh_select_seq_any(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP) {
		/* All sequence-like map items are "true" (because they
		 * are non-empty (2-element) tuples). As such, so-long as
		 * a mapping itself is non-empty, there will always exist
		 * an **item** (the 2-element tuple) that evaluations to
		 * true. */
		DeeMH_seq_operator_bool_t seq_operator_bool = (DeeMH_seq_operator_bool_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_bool);
		if (seq_operator_bool)
			return seq_operator_bool;
	}
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
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
	DeeMH_seq_enumerate_index_t seq_enumerate_index;
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP) {
		/* All sequence-like map items are "true" (because they
		 * are non-empty (2-element) tuples). As such, so-long as
		 * a mapping itself is non-empty, there will always exist
		 * an **item** (the 2-element tuple) that evaluations to
		 * true. */
		if ((DeeMH_seq_operator_bool_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_bool) || (DeeMH_map_operator_size_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_size))
			return &default__seq_any_with_range__with__seqclass_map__and__seq_operator_bool__and__map_operator_size;
	}
	seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
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
	DeeMH_seq_operator_foreach_t seq_operator_foreach;
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP) {
		/* Mappings are made up of non-empty (2-element) tuples, so they can never
		 * have items (the 2-element tuples) that evaluate to "false". As such, the
		 * Sequence.all() operator for mappings can return a constant "true". */
		return (DeeMH_seq_all_t)&_DeeNone_reti1_1;
	}
	seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
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
	DeeMH_seq_enumerate_index_t seq_enumerate_index;
	if (DeeType_GetSeqClass(self) == Dee_SEQCLASS_MAP) {
		/* Mappings are made up of non-empty (2-element) tuples, so they can never
		 * have items (the 2-element tuples) that evaluate to "false". As such, the
		 * Sequence.all() operator for mappings can return a constant "true". */
		return (DeeMH_seq_all_with_range_t)&_DeeNone_reti1_3;
	}
	seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
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

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_reduce_t DCALL
mh_select_seq_reduce(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_reduce__empty;
	if (seq_operator_foreach)
		return &default__seq_reduce__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_reduce_with_init_t DCALL
mh_select_seq_reduce_with_init(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__seq_reduce_with_init__empty;
	if (seq_operator_foreach)
		return &default__seq_reduce_with_init__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_reduce_with_range_t DCALL
mh_select_seq_reduce_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_reduce_with_range__empty;
	if (seq_enumerate_index)
		return &default__seq_reduce_with_range__with__seq_enumerate_index;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_reduce_with_range_and_init_t DCALL
mh_select_seq_reduce_with_range_and_init(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_enumerate_index_t seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_enumerate_index);
	if (seq_enumerate_index == &default__seq_enumerate_index__empty)
		return &default__seq_reduce_with_range_and_init__empty;
	if (seq_enumerate_index)
		return &default__seq_reduce_with_range_and_init__with__seq_enumerate_index;
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
	DeeMH_seq_erase_t seq_erase = (DeeMH_seq_erase_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_erase);
	if (seq_erase == &default__seq_erase__empty)
		return &default__seq_clear__empty;
	if (seq_erase == &default__seq_erase__with__seq_operator_delrange_index ||
	    seq_erase == &default__seq_erase__with__seq_pop) {
		DeeMH_seq_operator_delrange_index_n_t seq_operator_delrange_index_n = (DeeMH_seq_operator_delrange_index_n_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delrange_index_n);
		if (seq_operator_delrange_index_n == &default__seq_operator_delrange_index_n__empty)
			return &default__seq_clear__empty;
		if (seq_operator_delrange_index_n == &default__seq_operator_delrange_index_n__with__seq_operator_delrange)
			return &default__seq_clear__with__seq_operator_delrange;
		if (seq_operator_delrange_index_n)
			return &default__seq_clear__with__seq_operator_delrange_index_n;
	}
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

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_remove_t DCALL
mh_select_seq_remove(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_removeall_t seq_removeall;
	DeeMH_seq_removeif_t seq_removeif;
	DeeMH_seq_operator_delitem_index_t seq_operator_delitem_index;
	seq_removeall = (DeeMH_seq_removeall_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_removeall);
	if (seq_removeall) {
		if (seq_removeall == &default__seq_removeall__empty)
			return &default__seq_remove__empty;
		return &default__seq_remove__with__seq_removeall;
	}
	seq_removeif = (DeeMH_seq_removeif_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_removeif);
	if (seq_removeif) {
		if (seq_removeif == &default__seq_removeif__empty)
			return &default__seq_remove__empty;
		return &default__seq_remove__with__seq_removeall;
	}
	seq_operator_delitem_index = (DeeMH_seq_operator_delitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_delitem_index);
	if (seq_operator_delitem_index) {
		DeeMH_seq_find_t seq_find;
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
			return &default__seq_remove__empty;
		seq_find = (DeeMH_seq_find_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_find);
		if (seq_find != &default__seq_find__unsupported) {
			if (seq_find == &default__seq_find__with__seq_enumerate_index)
				return &default__seq_remove__with__seq_enumerate_index__and__seq_operator_delitem_index;
			return &default__seq_remove__with__seq_find__and__seq_operator_delitem_index;
		}
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_remove_with_key_t DCALL
mh_select_seq_remove_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_removeall_t seq_removeall;
	DeeMH_seq_removeif_t seq_removeif;
	DeeMH_seq_operator_delitem_index_t seq_operator_delitem_index;
	seq_removeall = (DeeMH_seq_removeall_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_removeall);
	if (seq_removeall) {
		if (seq_removeall == &default__seq_removeall__empty)
			return &default__seq_remove_with_key__empty;
		return &default__seq_remove_with_key__with__seq_removeall;
	}
	seq_removeif = (DeeMH_seq_removeif_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_removeif);
	if (seq_removeif) {
		if (seq_removeif == &default__seq_removeif__empty)
			return &default__seq_remove_with_key__empty;
		return &default__seq_remove_with_key__with__seq_removeall;
	}
	seq_operator_delitem_index = (DeeMH_seq_operator_delitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_delitem_index);
	if (seq_operator_delitem_index) {
		DeeMH_seq_find_with_key_t seq_find_with_key;
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
			return &default__seq_remove_with_key__empty;
		seq_find_with_key = (DeeMH_seq_find_with_key_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_find_with_key);
		if (seq_find_with_key != &default__seq_find_with_key__unsupported) {
			if (seq_find_with_key == &default__seq_find_with_key__with__seq_enumerate_index)
				return &default__seq_remove_with_key__with__seq_enumerate_index__and__seq_operator_delitem_index;
			return &default__seq_remove_with_key__with__seq_find_with_key__and__seq_operator_delitem_index;
		}
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_rremove_t DCALL
mh_select_seq_rremove(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_delitem_index_t seq_operator_delitem_index;
	seq_operator_delitem_index = (DeeMH_seq_operator_delitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_delitem_index);
	if (seq_operator_delitem_index) {
		DeeMH_seq_rfind_t seq_rfind;
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
			return &default__seq_rremove__empty;
		seq_rfind = (DeeMH_seq_rfind_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_rfind);
		if (seq_rfind != &default__seq_rfind__unsupported) {
			if (seq_rfind == &default__seq_rfind__with__seq_enumerate_index_reverse)
				return &default__seq_rremove__with__seq_enumerate_index_reverse__and__seq_operator_delitem_index;
			return &default__seq_rremove__with__seq_rfind__and__seq_operator_delitem_index;
		}
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_rremove_with_key_t DCALL
mh_select_seq_rremove_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_delitem_index_t seq_operator_delitem_index = (DeeMH_seq_operator_delitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_delitem_index);
	if (seq_operator_delitem_index) {
		DeeMH_seq_rfind_with_key_t seq_rfind_with_key;
		if (seq_operator_delitem_index == &default__seq_operator_delitem_index__empty)
			return &default__seq_rremove_with_key__empty;
		seq_rfind_with_key = (DeeMH_seq_rfind_with_key_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_rfind_with_key);
		if (seq_rfind_with_key != &default__seq_rfind_with_key__unsupported) {
			if (seq_rfind_with_key == &default__seq_rfind_with_key__with__seq_enumerate_index_reverse)
				return &default__seq_rremove_with_key__with__seq_enumerate_index_reverse__and__seq_operator_delitem_index;
			return &default__seq_rremove_with_key__with__seq_rfind_with_key__and__seq_operator_delitem_index;
		}
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_removeall_t DCALL
mh_select_seq_removeall(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_removeif_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_removeif))
		return &default__seq_removeall__with__seq_removeif;
	if ((DeeMH_seq_remove_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_remove)) {
		if (Dee_SEQCLASS_ISSETORMAP(DeeType_GetSeqClass(self)))
			return &default__seq_removeall__with__seq_remove__once;
		if ((DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported)
			return &default__seq_removeall__with__seq_operator_size__and__seq_remove;
	}
	if ((DeeMH_seq_operator_delitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delitem_index)) {
		if ((DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported &&
		    (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_trygetitem_index) != &default__seq_operator_trygetitem_index__unsupported)
			return &default__seq_removeall__with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_removeall_with_key_t DCALL
mh_select_seq_removeall_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_removeif_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_removeif))
		return &default__seq_removeall_with_key__with__seq_removeif;
	if ((DeeMH_seq_remove_with_key_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_remove_with_key)) {
		if (Dee_SEQCLASS_ISSETORMAP(DeeType_GetSeqClass(self)))
			return &default__seq_removeall_with_key__with__seq_remove_with_key__once;
		if ((DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported)
			return &default__seq_removeall_with_key__with__seq_operator_size__and__seq_remove_with_key;
	}
	if ((DeeMH_seq_operator_delitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delitem_index)) {
		if ((DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported &&
		    (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_trygetitem_index) != &default__seq_operator_trygetitem_index__unsupported)
			return &default__seq_removeall_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_removeif_t DCALL
mh_select_seq_removeif(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_seq_removeall_with_key_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_removeall_with_key))
		return &default__seq_removeif__with__seq_removeall_with_key;
	if ((DeeMH_seq_operator_delitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delitem_index)) {
		if ((DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported &&
		    (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_trygetitem_index) != &default__seq_operator_trygetitem_index__unsupported)
			return &default__seq_removeif__with__seq_operator_size__and__seq_operator_trygetitem_index__and__seq_operator_delitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_resize_t DCALL
mh_select_seq_resize(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_setrange_index_t seq_operator_setrange_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_resize__empty;
		if ((DeeMH_seq_erase_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_erase) && (DeeMH_seq_extend_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_extend))
			return &default__seq_resize__with__seq_operator_size__and__seq_erase__and__seq_extend;
		seq_operator_setrange_index = (DeeMH_seq_operator_setrange_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setrange_index);
		if (seq_operator_setrange_index) {
			DeeMH_seq_operator_delrange_index_t seq_operator_delrange_index;
			if (seq_operator_setrange_index == &default__seq_operator_setrange_index__with__seq_operator_size__and__seq_erase__and__seq_insertall)
				return &default__seq_resize__with__seq_operator_size__and__seq_erase__and__seq_extend;
			seq_operator_delrange_index = (DeeMH_seq_operator_delrange_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delrange_index);
			if (seq_operator_delrange_index != &default__seq_operator_delrange_index__with__seq_operator_setrange_index)
				return &default__seq_resize__with__seq_operator_size__and__seq_operator_setrange_index__and__seq_operator_delrange_index;
			return &default__seq_resize__with__seq_operator_size__and__seq_operator_setrange_index;
		}
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_fill_t DCALL
mh_select_seq_fill(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_setitem_index_t seq_operator_setitem_index;
	DeeMH_seq_operator_setrange_index_t seq_operator_setrange_index = (DeeMH_seq_operator_setrange_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setrange_index);
	if (seq_operator_setrange_index != NULL &&
	    seq_operator_setrange_index != &default__seq_operator_setrange_index__with__seq_operator_size__and__seq_erase__and__seq_insertall) {
		DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
		if (seq_operator_size != &default__seq_operator_size__unsupported) {
			if (seq_operator_size == &default__seq_operator_size__empty)
				return &default__seq_fill__empty;
			if (seq_operator_size == &default__seq_operator_size__with__seq_operator_iter ||
			    seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach ||
			    seq_operator_size == &default__seq_operator_size__with__seq_operator_foreach_pair ||
			    seq_operator_size == &default__seq_operator_size__with__map_enumerate ||
			    seq_operator_size == &default__seq_operator_size__with__seq_enumerate_index) {
				if ((DeeMH_seq_operator_setitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_setitem_index))
					return &default__seq_fill__with__seq_enumerate_index__and__seq_operator_setitem_index;
			}
			return &default__seq_fill__with__seq_operator_size__and__seq_operator_setrange_index;
		}
	}

	seq_operator_setitem_index = (DeeMH_seq_operator_setitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setitem_index);
	if (seq_operator_setitem_index) {
		DeeMH_seq_enumerate_index_t seq_enumerate_index;
		if (seq_operator_setitem_index == &default__seq_operator_setitem_index__empty)
			return &default__seq_fill__empty;
		seq_enumerate_index = (DeeMH_seq_enumerate_index_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_enumerate_index);
		if (seq_enumerate_index != &default__seq_enumerate_index__unsupported) {
			if (seq_enumerate_index == &default__seq_enumerate_index__empty)
				return &default__seq_fill__empty;
			return &default__seq_fill__with__seq_enumerate_index__and__seq_operator_setitem_index;
		}
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_reverse_t DCALL
mh_select_seq_reverse(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_setitem_index_t seq_operator_setitem_index;
	if ((DeeMH_seq_reversed_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_reversed) &&
	    (DeeMH_seq_operator_setrange_index_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_setrange_index) != &default__seq_operator_setrange_index__unsupported)
		return &default__seq_reverse__with__seq_reversed__and__seq_operator_setrange_index;
	seq_operator_setitem_index = (DeeMH_seq_operator_setitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setitem_index);
	if (seq_operator_setitem_index) {
		DeeMH_seq_operator_size_t seq_operator_size;
		DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
		if (seq_operator_setitem_index == &default__seq_operator_setitem_index__empty)
			return &default__seq_reverse__empty;
		if ((seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size)) != &default__seq_operator_size__unsupported &&
		    (seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_getitem_index)) != &default__seq_operator_getitem_index__unsupported) {
			if (seq_operator_size == &default__seq_operator_size__empty)
				return &default__seq_reverse__empty;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
				return &default__seq_reverse__empty;
			if ((DeeMH_seq_operator_delitem_index_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_operator_delitem_index))
				return &default__seq_reverse__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index__and__seq_operator_delitem_index;
			return &default__seq_reverse__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index;
		}
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_reversed_t DCALL
mh_select_seq_reversed(DeeTypeObject *self, DeeTypeObject *orig_type) {
	/* TODO: Implement using seq_enumerate_index! */
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_reversed__empty;
		if (self->tp_seq && self->tp_seq->tp_getitem_index_fast)
			return &default__seq_reversed__with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
			return &default__seq_reversed__empty;
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__with__seq_operator_getitem_index)
			return &default__seq_reversed__with__seq_operator_size__and__seq_operator_getitem_index;
		if (seq_operator_trygetitem_index)
			return &default__seq_reversed__with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
	if ((DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach))
		return &default__seq_reversed__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_sort_t DCALL
mh_select_seq_sort(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_setitem_index_t seq_operator_setitem_index;
	if ((DeeMH_seq_sorted_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_sorted) &&
	    (DeeMH_seq_operator_setrange_index_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_setrange_index) != &default__seq_operator_setrange_index__unsupported)
		return &default__seq_sort__with__seq_sorted__and__seq_operator_setrange_index;
	seq_operator_setitem_index = (DeeMH_seq_operator_setitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setitem_index);
	if (seq_operator_setitem_index) {
		DeeMH_seq_operator_size_t seq_operator_size;
		DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
		if (seq_operator_setitem_index == &default__seq_operator_setitem_index__empty)
			return &default__seq_sort__empty;
		if ((seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size)) != &default__seq_operator_size__unsupported &&
		    (seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_getitem_index)) != &default__seq_operator_getitem_index__unsupported) {
			if (seq_operator_size == &default__seq_operator_size__empty)
				return &default__seq_sort__empty;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
				return &default__seq_sort__empty;
			return &default__seq_sort__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index;
		}
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_sort_with_key_t DCALL
mh_select_seq_sort_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_setitem_index_t seq_operator_setitem_index;
	if ((DeeMH_seq_sorted_with_key_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_sorted_with_key) &&
	    (DeeMH_seq_operator_setrange_index_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_setrange_index) != &default__seq_operator_setrange_index__unsupported)
		return &default__seq_sort_with_key__with__seq_sorted_with_key__and__seq_operator_setrange_index;
	seq_operator_setitem_index = (DeeMH_seq_operator_setitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setitem_index);
	if (seq_operator_setitem_index) {
		DeeMH_seq_operator_size_t seq_operator_size;
		DeeMH_seq_operator_getitem_index_t seq_operator_getitem_index;
		if (seq_operator_setitem_index == &default__seq_operator_setitem_index__empty)
			return &default__seq_sort_with_key__empty;
		if ((seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size)) != &default__seq_operator_size__unsupported &&
		    (seq_operator_getitem_index = (DeeMH_seq_operator_getitem_index_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_getitem_index)) != &default__seq_operator_getitem_index__unsupported) {
			if (seq_operator_size == &default__seq_operator_size__empty)
				return &default__seq_sort_with_key__empty;
			if (seq_operator_getitem_index == &default__seq_operator_getitem_index__empty)
				return &default__seq_sort_with_key__empty;
			return &default__seq_sort_with_key__with__seq_operator_size__and__seq_operator_getitem_index__and__seq_operator_setitem_index;
		}
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_sorted_t DCALL
mh_select_seq_sorted(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_sorted__empty;
		if (self->tp_seq && self->tp_seq->tp_getitem_index_fast)
			return &default__seq_sorted__with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
			return &default__seq_sorted__empty;
		if (seq_operator_trygetitem_index)
			return &default__seq_sorted__with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
	if ((DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach))
		return &default__seq_sorted__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_sorted_with_key_t DCALL
mh_select_seq_sorted_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		DeeMH_seq_operator_trygetitem_index_t seq_operator_trygetitem_index;
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_sorted_with_key__empty;
		if (self->tp_seq && self->tp_seq->tp_getitem_index_fast)
			return &default__seq_sorted_with_key__with__seq_operator_size__and__operator_getitem_index_fast;
		seq_operator_trygetitem_index = (DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index);
		if (seq_operator_trygetitem_index == &default__seq_operator_trygetitem_index__empty)
			return &default__seq_sorted_with_key__empty;
		if (seq_operator_trygetitem_index)
			return &default__seq_sorted_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
	if ((DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach))
		return &default__seq_sorted_with_key__with__seq_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_bfind_t DCALL
mh_select_seq_bfind(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_bfind__empty;
		if ((DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index))
			return &default__seq_bfind__with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_bfind_with_key_t DCALL
mh_select_seq_bfind_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_bfind_with_key__empty;
		if ((DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index))
			return &default__seq_bfind_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_bposition_t DCALL
mh_select_seq_bposition(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_bposition__empty;
		if ((DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index))
			return &default__seq_bposition__with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_bposition_with_key_t DCALL
mh_select_seq_bposition_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_bposition_with_key__empty;
		if ((DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index))
			return &default__seq_bposition_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_brange_t DCALL
mh_select_seq_brange(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_brange__empty;
		if ((DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index))
			return &default__seq_brange__with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_seq_brange_with_key_t DCALL
mh_select_seq_brange_with_key(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_size_t seq_operator_size = (DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size);
	if (seq_operator_size != &default__seq_operator_size__unsupported) {
		if (seq_operator_size == &default__seq_operator_size__empty)
			return &default__seq_brange_with_key__empty;
		if ((DeeMH_seq_operator_trygetitem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_trygetitem_index))
			return &default__seq_brange_with_key__with__seq_operator_size__and__seq_operator_trygetitem_index;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_iter_t DCALL
mh_select_set_operator_iter(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_iter_t seq_operator_iter;
	DeeMH_map_operator_iter_t map_operator_iter = (DeeMH_map_operator_iter_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_iter);
	if (map_operator_iter)
		return map_operator_iter;
	seq_operator_iter = (DeeMH_seq_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty)
		return &default__set_operator_iter__empty;
	if (seq_operator_iter == &default__seq_operator_iter__with__map_enumerate ||
	    seq_operator_iter == &default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem ||
	    seq_operator_iter == &default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem)
		return seq_operator_iter;
	if (seq_operator_iter)
		return &default__set_operator_iter__with__seq_operator_iter;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_foreach_t DCALL
mh_select_set_operator_foreach(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_iter_t set_operator_iter;
	if ((DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_foreach_pair))
		return &default__set_operator_foreach__with__map_operator_foreach_pair;
	set_operator_iter = (DeeMH_set_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_iter);
	if (set_operator_iter == &default__set_operator_iter__empty)
		return &default__set_operator_foreach__empty;
	if (set_operator_iter == &default__set_operator_iter__with__seq_operator_iter)
		return &default__set_operator_foreach__with__seq_operator_foreach;
	if (set_operator_iter)
		return &default__set_operator_foreach__with__set_operator_iter;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_sizeob_t DCALL
mh_select_set_operator_sizeob(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_size_t set_operator_size;
	if (DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___map_getitem_always_bound__)) {
		/* Method hint "map_operator_sizeob" includes */
		DeeMH_map_operator_sizeob_t map_operator_sizeob = (DeeMH_map_operator_sizeob_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_sizeob);
		if (map_operator_sizeob) {
			if (map_operator_sizeob == &default__map_operator_sizeob__empty)
				return &default__set_operator_sizeob__empty;
			return map_operator_sizeob;
		}
	}
	set_operator_size = (DeeMH_set_operator_size_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_size);
	if (set_operator_size == &default__set_operator_size__empty)
		return &default__set_operator_sizeob__empty;
	if (set_operator_size)
		return &default__set_operator_sizeob__with__set_operator_size;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_size_t DCALL
mh_select_set_operator_size(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_foreach_t set_operator_foreach;
	/* Because "map_operator_size" counts unbound items,
	 * it can only be used if there can never be any. */
	DeeMH_map_operator_size_t map_operator_size = (DeeMH_map_operator_size_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_size);
	if (map_operator_size) {
		if (map_operator_size == &default__map_operator_size__empty ||
		    map_operator_size == &default__map_operator_size__with__map_operator_foreach_pair ||
		    map_operator_size == &default__map_operator_size__with__map_operator_iter ||
		    DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___map_getitem_always_bound__))
			return map_operator_size;
	}
	if ((DeeMH_set_operator_sizeob_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_sizeob))
		return &default__set_operator_size__with__set_operator_sizeob;
	set_operator_foreach = (DeeMH_set_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &default__set_operator_size__empty;
	if (set_operator_foreach == &default__set_operator_foreach__with__set_operator_iter)
		return &default__set_operator_size__with__set_operator_iter;
	if (set_operator_foreach)
		return &default__set_operator_size__with__set_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_hash_t DCALL
mh_select_set_operator_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	/* The mapping hash-operator is actually semantically compatible, so we
	 * can use *it* to determine how to implement the set hash-operator! */
	DeeMH_map_operator_hash_t map_operator_hash = (DeeMH_map_operator_hash_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_hash);
	if (map_operator_hash == &default__map_operator_hash__empty)
		return &default__set_operator_hash__empty;
	if (map_operator_hash == &default__map_operator_hash__with__map_operator_foreach_pair) {
		DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair = (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_foreach_pair);
		if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
			return &default__set_operator_hash__empty;
		if (map_operator_foreach_pair == &default__map_operator_foreach_pair__with__seq_operator_foreach_pair ||
		    map_operator_foreach_pair == &default__map_operator_foreach_pair__with__map_operator_iter ||
			map_operator_foreach_pair == &default__map_operator_foreach_pair__with__map_enumerate) {
			DeeMH_set_operator_foreach_t set_operator_foreach = (DeeMH_set_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_foreach);
			if (set_operator_foreach == &default__set_operator_foreach__empty)
				return &default__set_operator_hash__empty;
			if (set_operator_foreach == &default__set_operator_foreach__with__map_operator_foreach_pair)
				return &default__set_operator_hash__with__map_operator_foreach_pair;
			if (set_operator_foreach)
				return &default__set_operator_hash__with__set_operator_foreach;
		}
	}
	if (map_operator_hash)
		return map_operator_hash;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_compare_eq_t DCALL
mh_select_set_operator_compare_eq(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_foreach_t set_operator_foreach;
	if ((DeeMH_set_operator_eq_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_eq))
		return &default__set_operator_compare_eq__with__set_operator_eq;
	if ((DeeMH_set_operator_ne_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_ne))
		return &default__set_operator_compare_eq__with__set_operator_ne;
	set_operator_foreach = (DeeMH_set_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &default__set_operator_compare_eq__empty;
	if (set_operator_foreach)
		return &default__set_operator_compare_eq__with__set_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_trycompare_eq_t DCALL
mh_select_set_operator_trycompare_eq(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_compare_eq_t set_operator_compare_eq = (DeeMH_set_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_compare_eq);
	if (set_operator_compare_eq == &default__set_operator_compare_eq__empty)
		return &default__set_operator_trycompare_eq__empty;
	if (set_operator_compare_eq)
		return &default__set_operator_trycompare_eq__with__set_operator_compare_eq;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_eq_t DCALL
mh_select_set_operator_eq(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_set_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_compare_eq))
		return &default__set_operator_eq__with__set_operator_compare_eq;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_ne_t DCALL
mh_select_set_operator_ne(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_set_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_compare_eq))
		return &default__set_operator_ne__with__set_operator_compare_eq;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_lo_t DCALL
mh_select_set_operator_lo(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_foreach_t set_operator_foreach;
	if ((DeeMH_set_operator_ge_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_ge))
		return &default__set_operator_lo__with__set_operator_ge;
	set_operator_foreach = (DeeMH_set_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &default__set_operator_lo__empty;
	if (set_operator_foreach)
		return &default__set_operator_lo__with__set_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_le_t DCALL
mh_select_set_operator_le(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_foreach_t set_operator_foreach;
	if ((DeeMH_set_operator_gr_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_gr))
		return &default__set_operator_le__with__set_operator_gr;
	set_operator_foreach = (DeeMH_set_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &default__set_operator_le__empty;
	if (set_operator_foreach)
		return &default__set_operator_le__with__set_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_gr_t DCALL
mh_select_set_operator_gr(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_foreach_t set_operator_foreach;
	if ((DeeMH_set_operator_le_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_le))
		return &default__set_operator_gr__with__set_operator_le;
	set_operator_foreach = (DeeMH_set_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &default__set_operator_gr__empty;
	if (set_operator_foreach)
		return &default__set_operator_gr__with__set_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_ge_t DCALL
mh_select_set_operator_ge(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_foreach_t set_operator_foreach;
	if ((DeeMH_set_operator_lo_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_lo))
		return &default__set_operator_ge__with__set_operator_lo;
	set_operator_foreach = (DeeMH_set_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &default__set_operator_ge__empty;
	if (set_operator_foreach)
		return &default__set_operator_ge__with__set_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_inplace_add_t DCALL
mh_select_set_operator_inplace_add(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_insertall_t set_insertall = (DeeMH_set_insertall_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_insertall);
	if (set_insertall)
		return &default__set_operator_inplace_add__with__set_insertall;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_inplace_sub_t DCALL
mh_select_set_operator_inplace_sub(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_removeall_t set_removeall = (DeeMH_set_removeall_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_removeall);
	if (set_removeall && (DeeMH_set_operator_foreach_t)DeeType_GetMethodHint(orig_type, Dee_TMH_set_operator_foreach) != &default__set_operator_foreach__unsupported)
		return &default__set_operator_inplace_sub__with__set_operator_foreach__and__set_removeall;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_inplace_and_t DCALL
mh_select_set_operator_inplace_and(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_removeall_t set_removeall = (DeeMH_set_removeall_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_removeall);
	if (set_removeall && (DeeMH_set_operator_foreach_t)DeeType_GetMethodHint(orig_type, Dee_TMH_set_operator_foreach) != &default__set_operator_foreach__unsupported)
		return &default__set_operator_inplace_and__with__set_operator_foreach__and__set_removeall;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_operator_inplace_xor_t DCALL
mh_select_set_operator_inplace_xor(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if (((DeeMH_set_operator_foreach_t)DeeType_GetMethodHint(orig_type, Dee_TMH_set_operator_foreach) != &default__set_operator_foreach__unsupported) &&
	    (((DeeMH_set_removeall_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_removeall) && (DeeMH_set_insertall_t)DeeType_GetMethodHint(orig_type, Dee_TMH_set_insertall)) ||
	     ((DeeMH_set_insertall_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_insertall) && (DeeMH_set_removeall_t)DeeType_GetMethodHint(orig_type, Dee_TMH_set_removeall))))
		return &default__set_operator_inplace_xor__with__set_operator_foreach__and__set_insertall__and__set_removeall;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_frozen_t DCALL
mh_select_set_frozen(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_set_operator_foreach_t set_operator_foreach;
	if ((DeeMH_map_frozen_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_frozen))
		return &default__set_frozen__with__map_frozen;
	set_operator_foreach = (DeeMH_set_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_operator_foreach);
	if (set_operator_foreach == &default__set_operator_foreach__empty)
		return &default__set_frozen__empty;
	if (set_operator_foreach)
		return &default__set_frozen__with__set_operator_foreach;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_unify_t DCALL
mh_select_set_unify(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_foreach_t seq_operator_foreach = (DeeMH_seq_operator_foreach_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_foreach);
	if (seq_operator_foreach == &default__seq_operator_foreach__empty)
		return &default__set_unify__empty;
	if (seq_operator_foreach) {
		DeeMH_set_insert_t set_insert = (DeeMH_set_insert_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_insert);
		if (set_insert == &default__set_insert__empty)
			return &default__set_unify__empty;
		if (set_insert == &default__set_insert__with__seq_contains__and__seq_append)
			return &default__set_unify__with__seq_operator_foreach__and__seq_append;
		if (set_insert)
			return &default__set_unify__with__seq_operator_foreach__and__set_insert;
		if ((DeeMH_seq_append_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_append))
			return &default__set_unify__with__seq_operator_foreach__and__seq_append;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_insert_t DCALL
mh_select_set_insert(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_set_insertall_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_insertall)) {
		if ((DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported)
			return &default__set_insert__with__seq_operator_size__and__set_insertall;
	}
	if ((DeeMH_seq_append_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_append)) {
		if ((DeeMH_seq_contains_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_contains) != &default__seq_contains__unsupported)
			return &default__set_insert__with__seq_contains__and__seq_append;
	}
	if ((DeeMH_map_setnew_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_setnew))
		return &default__set_insert__with__map_setnew;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_insertall_t DCALL
mh_select_set_insertall(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_set_operator_inplace_add_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_inplace_add))
		return &default__set_insertall__with__set_operator_inplace_add;
	if ((DeeMH_set_insert_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_insert))
		return &default__set_insertall__with__set_insert;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_remove_t DCALL
mh_select_set_remove(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_set_removeall_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_removeall)) {
		if ((DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported)
			return &default__set_remove__with__seq_operator_size__and__set_removeall;
	}
	if ((DeeMH_seq_removeall_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_removeall))
		return &default__set_remove__with__seq_removeall;
	if ((DeeMH_map_operator_delitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_delitem) &&
	    (DeeMH_map_operator_trygetitem_t)DeeType_GetMethodHint(orig_type, Dee_TMH_map_operator_trygetitem) != &default__map_operator_trygetitem__unsupported)
		return &default__set_remove__with__map_operator_trygetitem__and__map_operator_delitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_removeall_t DCALL
mh_select_set_removeall(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_set_operator_inplace_sub_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_set_operator_inplace_sub))
		return &default__set_removeall__with__set_operator_inplace_sub;
	if ((DeeMH_set_remove_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_set_remove))
		return &default__set_removeall__with__set_remove;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_pop_t DCALL
mh_select_set_pop(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_pop_t seq_pop;
	if ((DeeMH_map_popitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_popitem))
		return &default__set_pop__with__map_popitem;
	seq_pop = (DeeMH_seq_pop_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_pop);
	if (seq_pop == &default__seq_pop__empty)
		return &default__set_pop__empty;
	if (seq_pop)
		return &default__set_pop__with__seq_pop;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_set_pop_with_default_t DCALL
mh_select_set_pop_with_default(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_pop_t seq_pop;
	if ((DeeMH_map_popitem_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_popitem))
		return &default__set_pop_with_default__with__map_popitem;
	seq_pop = (DeeMH_seq_pop_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_pop);
	if (seq_pop == &default__seq_pop__empty)
		return &default__set_pop_with_default__empty;
	if (seq_pop)
		return &default__set_pop_with_default__with__seq_pop;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_iter_t DCALL
mh_select_map_operator_iter(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_iter_t seq_operator_iter = (DeeMH_seq_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_iter);
	if (seq_operator_iter == &default__seq_operator_iter__empty)
		return &default__map_operator_iter__empty;
	if (seq_operator_iter == &default__seq_operator_iter__with__map_enumerate ||
	    seq_operator_iter == &default__seq_operator_iter__with__map_iterkeys__and__map_operator_trygetitem ||
	    seq_operator_iter == &default__seq_operator_iter__with__map_iterkeys__and__map_operator_getitem)
		return seq_operator_iter;
	if (seq_operator_iter)
		return &default__map_operator_iter__with__seq_operator_iter;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_foreach_pair_t DCALL
mh_select_map_operator_foreach_pair(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_iter_t map_operator_iter;
	if ((DeeMH_map_enumerate_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_enumerate) || (DeeMH_map_enumerate_range_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_enumerate_range))
		return &default__map_operator_foreach_pair__with__map_enumerate;
	map_operator_iter = (DeeMH_map_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_iter);
	if (map_operator_iter == &default__map_operator_iter__empty)
		return &default__map_operator_foreach_pair__empty;
	if (map_operator_iter == &default__map_operator_iter__with__seq_operator_iter)
		return &default__map_operator_foreach_pair__with__seq_operator_foreach_pair;
	if (map_operator_iter)
		return &default__map_operator_foreach_pair__with__map_operator_iter;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_sizeob_t DCALL
mh_select_map_operator_sizeob(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_size_t map_operator_size = (DeeMH_map_operator_size_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_size);
	if (map_operator_size == &default__map_operator_size__empty)
		return &default__map_operator_sizeob__empty;
	if (map_operator_size)
		return &default__map_operator_sizeob__with__map_operator_size;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_size_t DCALL
mh_select_map_operator_size(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;
	if ((DeeMH_map_operator_sizeob_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_sizeob))
		return &default__map_operator_size__with__map_operator_sizeob;
	map_operator_foreach_pair = (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_foreach_pair);
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
		return &default__map_operator_size__empty;
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__with__map_operator_iter)
		return &default__map_operator_size__with__map_operator_iter;
	if (map_operator_foreach_pair)
		return &default__map_operator_size__with__map_operator_foreach_pair;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_hash_t DCALL
mh_select_map_operator_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair = (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_foreach_pair);
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
		return &default__map_operator_hash__empty;
	if (map_operator_foreach_pair)
		return &default__map_operator_hash__with__map_operator_foreach_pair;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_getitem_t DCALL
mh_select_map_operator_getitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;
	/*if (REQUIRE_NODEFAULT(map_operator_getitem_string_len_hash)) {
		return REQUIRE_NODEFAULT(map_operator_getitem_index)
		       ? &$with__map_operator_getitem_index__and__map_operator_getitem_string_len_hash
		       : &$with__map_operator_getitem_string_len_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_getitem_string_hash)) {
		return REQUIRE_NODEFAULT(map_operator_getitem_index)
		       ? &$with__map_operator_getitem_index__and__map_operator_getitem_string_hash
		       : &$with__map_operator_getitem_string_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_getitem_index)) {
		return &$with__map_operator_getitem_index;
	}*/
	if ((DeeMH_map_enumerate_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_enumerate) || (DeeMH_map_enumerate_range_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_enumerate_range))
		return &default__map_operator_getitem__with__map_enumerate;
	map_operator_foreach_pair = (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_foreach_pair);
	if (map_operator_foreach_pair) {
		if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
			return &default__map_operator_getitem__empty;
		return &default__map_operator_getitem__with__map_enumerate;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_trygetitem_t DCALL
mh_select_map_operator_trygetitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_getitem_t map_operator_getitem;
	/*if (REQUIRE_NODEFAULT(map_operator_trygetitem_string_len_hash)) {
		return REQUIRE_NODEFAULT(map_operator_trygetitem_index)
		       ? &$with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_len_hash
		       : &$with__map_operator_trygetitem_string_len_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_trygetitem_string_hash)) {
		return REQUIRE_NODEFAULT(map_operator_trygetitem_index)
		       ? &$with__map_operator_trygetitem_index__and__map_operator_trygetitem_string_hash
		       : &$with__map_operator_trygetitem_string_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_trygetitem_index)) {
		return &$with__map_operator_trygetitem_index;
	}*/
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
	DeeMH_map_operator_getitem_t map_operator_getitem;
	DeeMH_map_operator_contains_t map_operator_contains = (DeeMH_map_operator_contains_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_contains);
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
#ifdef CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS
		return map_operator_bounditem;
#else /* CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
		if (map_operator_bounditem == &default__map_operator_bounditem__empty)
			return &default__map_operator_hasitem__empty;
		return &default__map_operator_hasitem__with__map_operator_bounditem;
#endif /* !CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_hasitem_index_t DCALL
mh_select_map_operator_hasitem_index(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_bounditem_index_t map_operator_bounditem_index = (DeeMH_map_operator_bounditem_index_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_bounditem_index);
	if (map_operator_bounditem_index) {
#ifdef CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS
		return map_operator_bounditem_index;
#else /* CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
		if (map_operator_bounditem_index == &default__map_operator_bounditem_index__empty)
			return &default__map_operator_hasitem_index__empty;
		return &default__map_operator_hasitem_index__with__map_operator_bounditem_index;
#endif /* !CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_hasitem_string_hash_t DCALL
mh_select_map_operator_hasitem_string_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_bounditem_string_hash_t map_operator_bounditem_string_hash = (DeeMH_map_operator_bounditem_string_hash_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_bounditem_string_hash);
	if (map_operator_bounditem_string_hash) {
#ifdef CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS
		return map_operator_bounditem_string_hash;
#else /* CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
		if (map_operator_bounditem_string_hash == &default__map_operator_bounditem_string_hash__empty)
			return &default__map_operator_hasitem_string_hash__empty;
		return &default__map_operator_hasitem_string_hash__with__map_operator_bounditem_string_hash;
#endif /* !CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_hasitem_string_len_hash_t DCALL
mh_select_map_operator_hasitem_string_len_hash(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_bounditem_string_len_hash_t map_operator_bounditem_string_len_hash = (DeeMH_map_operator_bounditem_string_len_hash_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_bounditem_string_len_hash);
	if (map_operator_bounditem_string_len_hash) {
#ifdef CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS
		return map_operator_bounditem_string_len_hash;
#else /* CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
		if (map_operator_bounditem_string_len_hash == &default__map_operator_bounditem_string_len_hash__empty)
			return &default__map_operator_hasitem_string_len_hash__empty;
		return &default__map_operator_hasitem_string_len_hash__with__map_operator_bounditem_string_len_hash;
#endif /* !CONFIG_EXPERIMENTAL_ALTERED_BOUND_CONSTANTS */
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_delitem_t DCALL
mh_select_map_operator_delitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_delitem_t seq_operator_delitem;
	DeeMH_map_enumerate_t map_enumerate;
	/*if (REQUIRE_NODEFAULT(map_operator_delitem_string_len_hash)) {
		return REQUIRE_NODEFAULT(map_operator_delitem_index)
		       ? &$with__map_operator_delitem_index__and__map_operator_delitem_string_len_hash
		       : &$with__map_operator_delitem_string_len_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_delitem_string_hash)) {
		return REQUIRE_NODEFAULT(map_operator_delitem_index)
		       ? &$with__map_operator_delitem_index__and__map_operator_delitem_string_hash
		       : &$with__map_operator_delitem_string_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_delitem_index)) {
		return &$with__map_operator_delitem_index;
	}*/
	if ((DeeMH_map_remove_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_remove))
		return &default__map_operator_delitem__with__map_remove;
	if ((DeeMH_map_pop_with_default_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_pop_with_default))
		return &default__map_operator_delitem__with__map_pop_with_default;
	if ((DeeMH_map_removekeys_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_removekeys))
		return &default__map_operator_delitem__with__map_removekeys;
	seq_operator_delitem = (DeeMH_seq_operator_delitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delitem);
	if (seq_operator_delitem) {
		if (seq_operator_delitem == &default__seq_operator_delitem__empty)
			return &default__map_operator_delitem__empty;
		if ((DeeMH_seq_enumerate_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_enumerate) != &default__seq_enumerate__unsupported)
			return &default__map_operator_delitem__with__map_remove; /* See selector in `map_remove' */
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
	DeeMH_seq_operator_setitem_t seq_operator_setitem;
	/*if (REQUIRE_NODEFAULT(map_operator_setitem_string_len_hash)) {
		return REQUIRE_NODEFAULT(map_operator_setitem_index)
		       ? &$with__map_operator_setitem_index__and__map_operator_setitem_string_len_hash
		       : &$with__map_operator_setitem_string_len_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_setitem_string_hash)) {
		return REQUIRE_NODEFAULT(map_operator_setitem_index)
		       ? &$with__map_operator_setitem_index__and__map_operator_setitem_string_hash
		       : &$with__map_operator_setitem_string_hash;
	} else if (REQUIRE_NODEFAULT(map_operator_setitem_index)) {
		return &$with__map_operator_setitem_index;
	}*/
	if (((seq_operator_setitem = (DeeMH_seq_operator_setitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setitem)) != NULL &&
	     ((DeeMH_seq_append_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_append) != &default__seq_append__unsupported)) ||
	    (((DeeMH_seq_append_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_append) != NULL) &&
	     (seq_operator_setitem = (DeeMH_seq_operator_setitem_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_setitem)) != &default__seq_operator_setitem__unsupported)) {
		if (seq_operator_setitem == &default__seq_operator_setitem__empty)
			return &default__map_operator_setitem__empty;
		if (seq_operator_setitem == &default__seq_operator_setitem__with__seq_operator_setitem_index &&
		    (DeeMH_seq_enumerate_index_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_enumerate_index) != &default__seq_enumerate_index__unsupported)
			return &default__map_operator_setitem__with__seq_enumerate_index__and__seq_operator_setitem_index__and__seq_append;
		if ((DeeMH_seq_enumerate_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_enumerate) != &default__seq_enumerate__unsupported)
			return &default__map_operator_setitem__with__seq_enumerate__and__seq_operator_setitem__and__seq_append;
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

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_keys_t DCALL
mh_select_map_keys(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_iterkeys_t map_iterkeys = (DeeMH_map_iterkeys_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_iterkeys);
	if (map_iterkeys == &default__map_iterkeys__empty)
		return &default__map_keys__empty;
	if (map_iterkeys)
		return &default__map_keys__with__map_iterkeys;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_iterkeys_t DCALL
mh_select_map_iterkeys(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_enumerate_t map_enumerate;
	if ((DeeMH_map_keys_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_keys))
		return &default__map_iterkeys__with__map_keys;
	map_enumerate = (DeeMH_map_enumerate_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_enumerate);
	if (map_enumerate == &default__map_enumerate__empty)
		return &default__map_iterkeys__empty;
	if (DeeType_HasPrivateTrait(self, orig_type, DeeType_TRAIT___map_getitem_always_bound__) ||
	    (map_enumerate && map_enumerate == (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_foreach_pair))) {
		DeeMH_map_operator_iter_t map_operator_iter = (DeeMH_map_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_iter);
		if (map_operator_iter == &default__map_operator_iter__empty)
			return &default__map_iterkeys__empty;
		if (map_operator_iter)
			return &default__map_iterkeys__with__map_operator_iter;
	}
	if (map_enumerate)
		return &default__map_iterkeys__with__map_enumerate;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_values_t DCALL
mh_select_map_values(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_itervalues_t map_itervalues = (DeeMH_map_itervalues_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_itervalues);
	if (map_itervalues == &default__map_itervalues__empty)
		return &default__map_values__empty;
	if (map_itervalues)
		return &default__map_values__with__map_itervalues;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_itervalues_t DCALL
mh_select_map_itervalues(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_iter_t map_operator_iter;
	if ((DeeMH_map_values_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_values))
		return &default__map_itervalues__with__map_values;
	map_operator_iter = (DeeMH_map_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_iter);
	if (map_operator_iter == &default__map_operator_iter__empty)
		return &default__map_itervalues__empty;
	if (map_operator_iter)
		return &default__map_itervalues__with__map_operator_iter;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_enumerate_t DCALL
mh_select_map_enumerate(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_keys_t map_keys;
	DeeMH_map_iterkeys_t map_iterkeys;
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;
	/*if (REQUIRE_NODEFAULT(map_enumerate_range))
		return &$with__map_enumerate_range;*/

	map_keys = (DeeMH_map_keys_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_keys);
	if (map_keys) {
		if (map_keys == &default__map_keys__empty)
			return &default__map_enumerate__empty;
		if (map_keys != &default__map_keys__with__map_iterkeys)
			goto check_with_iterkeys;
	}

	map_iterkeys = (DeeMH_map_iterkeys_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_iterkeys);
	if (map_iterkeys) {
		if (map_iterkeys == &default__map_iterkeys__empty)
			return &default__map_enumerate__empty;
		if (map_iterkeys != &default__map_iterkeys__with__map_operator_iter) {
			DeeMH_map_operator_trygetitem_t map_operator_trygetitem;
check_with_iterkeys:
			map_operator_trygetitem = (DeeMH_map_operator_trygetitem_t)DeeType_GetMethodHint(orig_type, Dee_TMH_map_operator_trygetitem);
			if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
				return &default__map_enumerate__empty;
			if (map_operator_trygetitem)
				return &default__map_enumerate__with__map_iterkeys__and__map_operator_trygetitem;
		}
	}

	map_operator_foreach_pair = (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_foreach_pair);
	if (map_operator_foreach_pair) {
		if (map_operator_foreach_pair == &default__map_operator_foreach_pair__with__map_operator_iter) {
			DeeMH_map_operator_iter_t map_operator_iter = (DeeMH_map_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_iter);
			if (map_operator_iter == &default__map_operator_iter__empty)
				return &default__map_enumerate__empty;
			if (map_operator_iter == &default__map_operator_iter__with__map_iterkeys__and__map_operator_trygetitem ||
			    map_operator_iter == &default__map_operator_iter__with__map_iterkeys__and__map_operator_getitem)
				return &default__map_enumerate__with__map_iterkeys__and__map_operator_trygetitem;
		}/* else if (map_operator_foreach_pair == &default__map_operator_foreach_pair__with__seq_operator_foreach_pair) {
			// ...
		}*/
		return map_operator_foreach_pair; /* Binary-compatible */
	}
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

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_makeenumeration_t DCALL
mh_select_map_makeenumeration(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_enumerate_t map_enumerate = (DeeMH_map_enumerate_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_enumerate);
	if (map_enumerate == &default__map_enumerate__empty)
		return &default__map_makeenumeration__empty;
	if (map_enumerate == &default__map_enumerate__with__map_iterkeys__and__map_operator_trygetitem) {
		DeeMH_map_operator_trygetitem_t map_operator_trygetitem;
		DeeMH_map_iterkeys_t map_iterkeys = (DeeMH_map_iterkeys_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_iterkeys);
		if (map_iterkeys == &default__map_iterkeys__empty)
			return &default__map_makeenumeration__empty;
		if (map_iterkeys == &default__map_iterkeys__with__map_operator_iter) {
return__with__map_operator_iter:
			if (DeeType_RequireSupportedNativeOperator(self, iter) == (DeeMH_map_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_iter))
				return &default__map_makeenumeration__with__operator_iter;
			return &default__map_makeenumeration__with__map_operator_iter;
		}

		map_operator_trygetitem = (DeeMH_map_operator_trygetitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_trygetitem);
		if (map_operator_trygetitem == &default__map_operator_trygetitem__empty)
			return &default__map_makeenumeration__empty;
		if (map_operator_trygetitem == &default__map_operator_trygetitem__with__map_operator_getitem)
			return &default__map_makeenumeration__with__map_iterkeys__and__map_operator_getitem;
		return &default__map_makeenumeration__with__map_iterkeys__and__map_operator_trygetitem;
	}
	if (map_enumerate) {
		/* The "$with__map_enumerate" impl is super-inefficient.
		 * See if we can use one of the others, even if that one would work. */
		DeeMH_map_operator_iter_t map_operator_iter = (DeeMH_map_operator_iter_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_iter);
		if (map_operator_iter == &default__map_operator_iter__empty)
			return &default__map_makeenumeration__empty;
		if (map_operator_iter == &default__map_operator_iter__with__map_iterkeys__and__map_operator_trygetitem)
			return &default__map_makeenumeration__with__map_iterkeys__and__map_operator_trygetitem;
		if (map_operator_iter == &default__map_operator_iter__with__map_iterkeys__and__map_operator_getitem)
			return &default__map_makeenumeration__with__map_iterkeys__and__map_operator_getitem;
		if (map_operator_iter != &default__map_operator_iter__with__map_enumerate)
			goto return__with__map_operator_iter;
		return &default__map_makeenumeration__with__map_enumerate;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_makeenumeration_with_range_t DCALL
mh_select_map_makeenumeration_with_range(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_makeenumeration_t map_makeenumeration = (DeeMH_map_makeenumeration_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_makeenumeration);
	if (map_makeenumeration == &default__map_makeenumeration__empty)
		return &default__map_makeenumeration_with_range__empty;
	if (map_makeenumeration == &default__map_makeenumeration__with__operator_iter ||
	    map_makeenumeration == &default__map_makeenumeration__with__map_operator_iter)
		return &default__map_makeenumeration_with_range__with__map_operator_iter;
	if (map_makeenumeration == &default__map_makeenumeration__with__map_iterkeys__and__map_operator_getitem)
		return &default__map_makeenumeration_with_range__with__map_iterkeys__and__map_operator_getitem;
	if (map_makeenumeration == &default__map_makeenumeration__with__map_iterkeys__and__map_operator_trygetitem)
		return &default__map_makeenumeration_with_range__with__map_iterkeys__and__map_operator_trygetitem;
	if (map_makeenumeration == &default__map_makeenumeration__with__map_enumerate)
		return &default__map_makeenumeration_with_range__with__map_enumerate_range;
	/*if (map_makeenumeration) // TODO
		return &$with__map_makeenumeration;*/
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_compare_eq_t DCALL
mh_select_map_operator_compare_eq(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;
	if ((DeeMH_map_operator_eq_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_eq))
		return &default__map_operator_compare_eq__with__map_operator_eq;
	if ((DeeMH_map_operator_ne_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_ne))
		return &default__map_operator_compare_eq__with__map_operator_ne;
	map_operator_foreach_pair = (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_foreach_pair);
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
		return &default__map_operator_compare_eq__empty;
	if (map_operator_foreach_pair)
		return &default__map_operator_compare_eq__with__map_operator_foreach_pair;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_trycompare_eq_t DCALL
mh_select_map_operator_trycompare_eq(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_compare_eq_t map_operator_compare_eq = (DeeMH_map_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_compare_eq);
	if (map_operator_compare_eq == &default__map_operator_compare_eq__empty)
		return &default__map_operator_trycompare_eq__empty;
	if (map_operator_compare_eq)
		return &default__map_operator_trycompare_eq__with__map_operator_compare_eq;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_eq_t DCALL
mh_select_map_operator_eq(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_map_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_compare_eq))
		return &default__map_operator_eq__with__map_operator_compare_eq;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_ne_t DCALL
mh_select_map_operator_ne(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_map_operator_compare_eq_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_compare_eq))
		return &default__map_operator_ne__with__map_operator_compare_eq;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_lo_t DCALL
mh_select_map_operator_lo(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;
	if ((DeeMH_map_operator_ge_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_ge))
		return &default__map_operator_lo__with__map_operator_ge;
	map_operator_foreach_pair = (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_foreach_pair);
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
		return &default__map_operator_lo__empty;
	if (map_operator_foreach_pair)
		return &default__map_operator_lo__with__map_operator_foreach_pair;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_le_t DCALL
mh_select_map_operator_le(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;
	if ((DeeMH_map_operator_gr_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_gr))
		return &default__map_operator_le__with__map_operator_gr;
	map_operator_foreach_pair = (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_foreach_pair);
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
		return &default__map_operator_le__empty;
	if (map_operator_foreach_pair)
		return &default__map_operator_le__with__map_operator_foreach_pair;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_gr_t DCALL
mh_select_map_operator_gr(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;
	if ((DeeMH_map_operator_le_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_le))
		return &default__map_operator_gr__with__map_operator_le;
	map_operator_foreach_pair = (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_foreach_pair);
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
		return &default__map_operator_gr__empty;
	if (map_operator_foreach_pair)
		return &default__map_operator_gr__with__map_operator_foreach_pair;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_ge_t DCALL
mh_select_map_operator_ge(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair;
	if ((DeeMH_map_operator_lo_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_lo))
		return &default__map_operator_ge__with__map_operator_lo;
	map_operator_foreach_pair = (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_foreach_pair);
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
		return &default__map_operator_ge__empty;
	if (map_operator_foreach_pair)
		return &default__map_operator_ge__with__map_operator_foreach_pair;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_inplace_add_t DCALL
mh_select_map_operator_inplace_add(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_update_t map_update = (DeeMH_map_update_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_update);
	if (map_update)
		return &default__map_operator_inplace_add__with__map_update;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_inplace_sub_t DCALL
mh_select_map_operator_inplace_sub(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_map_removekeys_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_removekeys))
		return &default__map_operator_inplace_sub__with__map_removekeys;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_inplace_and_t DCALL
mh_select_map_operator_inplace_and(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_removekeys_t map_removekeys = (DeeMH_map_removekeys_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_removekeys);
	if (map_removekeys && (DeeMH_map_operator_foreach_pair_t)DeeType_GetMethodHint(orig_type, Dee_TMH_map_operator_foreach_pair) != &default__map_operator_foreach_pair__unsupported)
		return &default__map_operator_inplace_and__with__map_operator_foreach_pair__and__map_removekeys;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_operator_inplace_xor_t DCALL
mh_select_map_operator_inplace_xor(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if (((DeeMH_map_operator_foreach_pair_t)DeeType_GetMethodHint(orig_type, Dee_TMH_map_operator_foreach_pair) != &default__map_operator_foreach_pair__unsupported) &&
	    (((DeeMH_map_removekeys_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_removekeys) && (DeeMH_map_update_t)DeeType_GetMethodHint(orig_type, Dee_TMH_map_update)) ||
	     ((DeeMH_map_update_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_update) && (DeeMH_map_removekeys_t)DeeType_GetMethodHint(orig_type, Dee_TMH_map_removekeys))))
		return &default__map_operator_inplace_xor__with__map_operator_foreach_pair__and__map_update__and__map_removekeys;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_frozen_t DCALL
mh_select_map_frozen(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_foreach_pair_t map_operator_foreach_pair = (DeeMH_map_operator_foreach_pair_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_foreach_pair);
	if (map_operator_foreach_pair == &default__map_operator_foreach_pair__empty)
		return &default__map_frozen__empty;
	if (map_operator_foreach_pair)
		return &default__map_frozen__with__map_operator_foreach_pair;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_setold_t DCALL
mh_select_map_setold(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_setold_ex_t map_setold_ex = (DeeMH_map_setold_ex_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_setold_ex);
	if (map_setold_ex == &default__map_setold_ex__empty)
		return &default__map_setold__empty;
	if (map_setold_ex == &default__map_setold_ex__with__map_operator_trygetitem__and__map_operator_setitem)
		return &default__map_setold__with__map_operator_trygetitem__and__map_operator_setitem;
	if (map_setold_ex)
		return &default__map_setold__with__map_setold_ex;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_setold_ex_t DCALL
mh_select_map_setold_ex(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_setitem_t map_operator_setitem;
	DeeMH_seq_operator_setitem_t seq_operator_setitem;
	if ((DeeMH_map_setold_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_setold) &&
	    (DeeMH_map_operator_trygetitem_t)DeeType_GetMethodHint(orig_type, Dee_TMH_map_operator_trygetitem) != &default__map_operator_trygetitem__unsupported)
		return &default__map_setold_ex__with__map_operator_trygetitem__and__map_setold;
	map_operator_setitem = (DeeMH_map_operator_setitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_setitem);
	if (map_operator_setitem) {
		if (map_operator_setitem == &default__map_operator_setitem__with__seq_enumerate_index__and__seq_operator_setitem_index__and__seq_append)
			return &default__map_setold_ex__with__seq_enumerate_index__and__seq_operator_setitem_index;
		if (map_operator_setitem == &default__map_operator_setitem__with__seq_enumerate__and__seq_operator_setitem__and__seq_append)
			return &default__map_setold_ex__with__seq_enumerate__and__seq_operator_setitem;
		if ((DeeMH_map_operator_trygetitem_t)DeeType_GetMethodHint(orig_type, Dee_TMH_map_operator_trygetitem) != &default__map_operator_trygetitem__unsupported)
			return &default__map_setold_ex__with__map_operator_trygetitem__and__map_operator_setitem;
	}
	seq_operator_setitem = (DeeMH_seq_operator_setitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_setitem);
	if (seq_operator_setitem) {
		if (seq_operator_setitem == &default__seq_operator_setitem__empty)
			return &default__map_setold_ex__empty;
		if (seq_operator_setitem == &default__seq_operator_setitem__with__seq_operator_setitem_index &&
		    (DeeMH_seq_enumerate_index_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_enumerate_index) != &default__seq_enumerate_index__unsupported)
			return &default__map_setold_ex__with__seq_enumerate_index__and__seq_operator_setitem_index;
		if ((DeeMH_seq_enumerate_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_enumerate) != &default__seq_enumerate__unsupported)
			return &default__map_setold_ex__with__seq_enumerate__and__seq_operator_setitem;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_setnew_t DCALL
mh_select_map_setnew(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_setnew_ex_t map_setnew_ex = (DeeMH_map_setnew_ex_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_setnew_ex);
	if (map_setnew_ex == &default__map_setnew_ex__empty)
		return &default__map_setnew__empty;
	if (map_setnew_ex == &default__map_setnew_ex__with__map_operator_trygetitem__and__map_setdefault)
		return &default__map_setnew__with__map_operator_trygetitem__and__map_setdefault;
	if (map_setnew_ex == &default__map_setnew_ex__with__map_operator_trygetitem__and__map_operator_setitem)
		return &default__map_setnew__with__map_operator_trygetitem__and__map_operator_setitem;
	if (map_setnew_ex)
		return &default__map_setnew__with__map_setnew_ex;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_setnew_ex_t DCALL
mh_select_map_setnew_ex(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_map_operator_trygetitem_t)DeeType_GetMethodHint(orig_type, Dee_TMH_map_operator_trygetitem) != &default__map_operator_trygetitem__unsupported) {
		DeeMH_map_operator_setitem_t map_operator_setitem;
		if ((DeeMH_map_setnew_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_setnew))
			return &default__map_setnew_ex__with__map_operator_trygetitem__and__map_setnew;
		if ((DeeMH_map_setdefault_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_setdefault))
			return &default__map_setnew_ex__with__map_operator_trygetitem__and__map_setdefault;
		map_operator_setitem = (DeeMH_map_operator_setitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_setitem);
		if (map_operator_setitem) {
			if (map_operator_setitem == &default__map_operator_setitem__with__seq_enumerate__and__seq_operator_setitem__and__seq_append ||
			    map_operator_setitem == &default__map_operator_setitem__with__seq_enumerate_index__and__seq_operator_setitem_index__and__seq_append)
				return &default__map_setnew_ex__with__map_operator_trygetitem__and__seq_append;
			return &default__map_setnew_ex__with__map_operator_trygetitem__and__map_operator_setitem;
		}
		if ((DeeMH_seq_append_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_append))
			return &default__map_setnew_ex__with__map_operator_trygetitem__and__seq_append;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_setdefault_t DCALL
mh_select_map_setdefault(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_setnew_ex_t map_setnew_ex = (DeeMH_map_setnew_ex_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_setnew_ex);
	if (map_setnew_ex == &default__map_setnew_ex__empty)
		return &default__map_setdefault__empty;
	if (map_setnew_ex == &default__map_setnew_ex__with__map_operator_trygetitem__and__map_setnew)
		return &default__map_setdefault__with__map_setnew__and__map_operator_getitem;
	if (map_setnew_ex == &default__map_setnew_ex__with__map_operator_trygetitem__and__map_operator_setitem)
		return &default__map_setdefault__with__map_operator_trygetitem__and__map_operator_setitem;
	if (map_setnew_ex)
		return &default__map_setdefault__with__map_setnew_ex;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_update_t DCALL
mh_select_map_update(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_setitem_t map_operator_setitem;
	if ((DeeMH_map_operator_inplace_add_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_inplace_add))
		return &default__map_update__with__map_operator_inplace_add;
	map_operator_setitem = (DeeMH_map_operator_setitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_setitem);
	if (map_operator_setitem == &default__map_operator_setitem__empty)
		return &default__map_update__empty;
	if (map_operator_setitem)
		return &default__map_update__with__map_operator_setitem;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_remove_t DCALL
mh_select_map_remove(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_seq_operator_delitem_t seq_operator_delitem;
	DeeMH_map_operator_delitem_t map_operator_delitem = (DeeMH_map_operator_delitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_delitem);
	if (map_operator_delitem == &default__map_operator_delitem__empty)
		return &default__map_remove__empty;
	if (map_operator_delitem != NULL &&
	    map_operator_delitem != &default__map_operator_delitem__with__map_remove) {
		if ((DeeMH_map_operator_bounditem_t)DeeType_GetMethodHint(orig_type, Dee_TMH_map_operator_bounditem) != &default__map_operator_bounditem__unsupported)
			return &default__map_remove__with__map_operator_bounditem__and__map_operator_delitem;
		if ((DeeMH_seq_operator_size_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_operator_size) != &default__seq_operator_size__unsupported)
			return &default__map_remove__with__seq_operator_size__and__map_operator_delitem;
	}

	seq_operator_delitem = (DeeMH_seq_operator_delitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_operator_delitem);
	if (seq_operator_delitem) {
		if (seq_operator_delitem == &default__seq_operator_delitem__empty)
			return &default__map_remove__empty;
		if (seq_operator_delitem == &default__seq_operator_delitem__with__seq_operator_delitem_index &&
		    (DeeMH_seq_enumerate_index_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_enumerate_index) != &default__seq_enumerate_index__unsupported)
			return &default__map_remove__with__seq_enumerate_index__and__seq_operator_delitem_index;
		if ((DeeMH_seq_enumerate_t)DeeType_GetMethodHint(orig_type, Dee_TMH_seq_enumerate) != &default__seq_enumerate__unsupported)
			return &default__map_remove__with__seq_enumerate__and__seq_operator_delitem;
	}
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_removekeys_t DCALL
mh_select_map_removekeys(DeeTypeObject *self, DeeTypeObject *orig_type) {
	if ((DeeMH_map_operator_inplace_sub_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_map_operator_inplace_sub))
		return &default__map_removekeys__with__map_operator_inplace_sub;
	if ((DeeMH_map_remove_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_remove))
		return &default__map_removekeys__with__map_remove;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_pop_t DCALL
mh_select_map_pop(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_delitem_t map_operator_delitem = (DeeMH_map_operator_delitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_delitem);
	if (map_operator_delitem == &default__map_operator_delitem__empty)
		return &default__map_pop__empty;
	if (map_operator_delitem == &default__map_operator_delitem__with__map_remove) {
		DeeMH_map_remove_t map_remove = (DeeMH_map_remove_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_remove);
		if (map_remove == &default__map_remove__empty)
			return &default__map_pop__empty;
		if (map_remove == &default__map_remove__with__seq_enumerate_index__and__seq_operator_delitem_index)
			return &default__map_pop__with__seq_enumerate_index__and__seq_operator_delitem_index;
		if (map_remove == &default__map_remove__with__seq_enumerate__and__seq_operator_delitem)
			return &default__map_pop__with__seq_enumerate__and__seq_operator_delitem;
	}
	if (map_operator_delitem &&
	    (DeeMH_map_operator_getitem_t)DeeType_GetMethodHint(orig_type, Dee_TMH_map_operator_getitem) != &default__map_operator_getitem__unsupported)
		return &default__map_pop__with__map_operator_getitem__and__map_operator_delitem;;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_pop_with_default_t DCALL
mh_select_map_pop_with_default(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_delitem_t map_operator_delitem = (DeeMH_map_operator_delitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_delitem);
	if (map_operator_delitem == &default__map_operator_delitem__empty)
		return &default__map_pop_with_default__empty;
	if (map_operator_delitem == &default__map_operator_delitem__with__map_remove) {
		DeeMH_map_remove_t map_remove = (DeeMH_map_remove_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_remove);
		if (map_remove == &default__map_remove__empty)
			return &default__map_pop_with_default__empty;
		if (map_remove == &default__map_remove__with__seq_enumerate_index__and__seq_operator_delitem_index)
			return &default__map_pop_with_default__with__seq_enumerate_index__and__seq_operator_delitem_index;
		if (map_remove == &default__map_remove__with__seq_enumerate__and__seq_operator_delitem)
			return &default__map_pop_with_default__with__seq_enumerate__and__seq_operator_delitem;
	}
	if (map_operator_delitem &&
	    (DeeMH_map_operator_trygetitem_t)DeeType_GetMethodHint(orig_type, Dee_TMH_map_operator_trygetitem) != &default__map_operator_trygetitem__unsupported)
		return &default__map_pop_with_default__with__map_operator_trygetitem__and__map_operator_delitem;;
	return NULL;
}

INTERN ATTR_PURE WUNUSED NONNULL((1, 2)) DeeMH_map_popitem_t DCALL
mh_select_map_popitem(DeeTypeObject *self, DeeTypeObject *orig_type) {
	DeeMH_map_operator_delitem_t map_operator_delitem = (DeeMH_map_operator_delitem_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_operator_delitem);
	if (map_operator_delitem) {
		DeeMH_seq_trygetfirst_t seq_trygetfirst;
		if (map_operator_delitem == &default__map_operator_delitem__empty)
			return &default__map_popitem__empty;
		if (map_operator_delitem == &default__map_operator_delitem__with__map_remove) {
			DeeMH_map_remove_t map_remove = (DeeMH_map_remove_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_map_remove);
			if (map_remove == &default__map_remove__empty)
				return &default__map_popitem__empty;
			if (map_remove == &default__map_remove__with__seq_enumerate_index__and__seq_operator_delitem_index ||
			    map_remove == &default__map_remove__with__seq_enumerate__and__seq_operator_delitem)
				return &default__map_popitem__with__seq_pop;
		}
		if ((DeeMH_seq_trygetlast_t)DeeType_GetPrivateMethodHintNoDefault(self, orig_type, Dee_TMH_seq_trygetlast))
			return &default__map_popitem__with__seq_trygetlast__and__map_operator_delitem;
		seq_trygetfirst = (DeeMH_seq_trygetfirst_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_trygetfirst);
		if (seq_trygetfirst) {
			if (seq_trygetfirst == &default__seq_trygetfirst__empty)
				return &default__map_popitem__empty;
			if (seq_trygetfirst == &default__seq_trygetfirst__with__seq_operator_size__and__operator_getitem_index_fast)
				return &default__map_popitem__with__seq_trygetlast__and__map_operator_delitem;
			return &default__map_popitem__with__seq_trygetfirst__and__map_operator_delitem;
		}
	}
	if ((DeeMH_seq_pop_t)DeeType_GetPrivateMethodHint(self, orig_type, Dee_TMH_seq_pop))
		return &default__map_popitem__with__seq_pop;
	return NULL;
}
/*[[[end]]]*/
/* clang-format on */

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINT_SELECT_C */
