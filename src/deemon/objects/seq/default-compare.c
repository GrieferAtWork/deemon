/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_COMPARE_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_COMPARE_C 1

#include <deemon/api.h>

#include <deemon/object.h>

#include "default-compare.h"

DECL_BEGIN

/* @return:  1: "sc_lfr_rhs" does contain "lhs_elem"
 * @return: -2: "sc_lfr_rhs" does not contain "lhs_elem"
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
set_compare__lhs_foreach__rhs__cb(void *arg, DeeObject *lhs_elem) {
	int contains;
	DREF DeeObject *contains_ob;
	struct set_compare__lhs_foreach__rhs__data *data;
	data = (struct set_compare__lhs_foreach__rhs__data *)arg;
	contains_ob = (*data->sc_lfr_rcontains)(data->sc_lfr_rhs, lhs_elem);
	if unlikely(!contains_ob)
		goto err;
	contains = DeeObject_BoolInherited(contains_ob);
	if unlikely(contains < 0)
		goto err;
	if (!contains)
		return -2; /* Not contained */
	return 1;
err:
	return -1;
}


/* @return:  1: "mc_lfr_rhs" does contain "lhs_key" with the same value
 * @return: -2: "mc_lfr_rhs" does not contain "lhs_key", or has a different value for it
 * @return: -1: Error */
INTERN WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
map_compare__lhs_foreach__rhs__cb(void *arg, DeeObject *lhs_key, DeeObject *lhs_value) {
	int values_eq;
	DREF DeeObject *rhs_value;
	struct map_compare__lhs_foreach__rhs__data *data;
	data = (struct map_compare__lhs_foreach__rhs__data *)arg;
	rhs_value = (*data->mc_lfr_rtrygetitem)(data->mc_lfr_rhs, lhs_key);
	if unlikely(!rhs_value)
		goto err;
	if (rhs_value == ITER_DONE)
		return -2; /* Missing key */
	values_eq = DeeObject_TryCompareEq(lhs_value, rhs_value);
	Dee_Decref(rhs_value);
	if (Dee_COMPARE_ISERR(values_eq))
		goto err;
	if (Dee_COMPARE_ISNE(values_eq))
		return -2; /* Non-equal values */
	return 1;
err:
	return -1;
}


DECL_END

#ifndef __INTELLISENSE__
#define DEFINE_compare
#include "default-compare-impl.c.inl"
#define DEFINE_compareeq
#include "default-compare-impl.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_COMPARE_C */
