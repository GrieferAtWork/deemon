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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_OPERATOR_METHODS_C_INL
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_OPERATOR_METHODS_C_INL 1

#ifdef __INTELLISENSE__
#include "default-api-methods.c"
#endif /* __INTELLISENSE__ */

#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/string.h>

#include "../../runtime/kwlist.h"

#undef SSIZE_MIN
#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MIN __SSIZE_MIN__
#define SSIZE_MAX __SSIZE_MAX__

DECL_BEGIN

/* Default operators as type_method-s */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___bool__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, ":__bool__"))
		goto err;
	result = DeeSeq_OperatorBool(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___iter__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":__iter__"))
		goto err;
	return DeeSeq_OperatorIter(self);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___size__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":__size__"))
		goto err;
	return DeeSeq_OperatorSizeOb(self);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___contains__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *x;
	if (DeeArg_Unpack(argc, argv, "o:__contains__", &x))
		goto err;
	return DeeSeq_OperatorContains(self, x);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___getitem__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *index;
	if (DeeArg_Unpack(argc, argv, "o:__getitem__", &index))
		goto err;
	return DeeSeq_OperatorGetItem(self, index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___delitem__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *index;
	if (DeeArg_Unpack(argc, argv, "o:__delitem__", &index))
		goto err;
	if unlikely(DeeSeq_OperatorDelItem(self, index))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___setitem__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *index, *value;
	if (DeeArg_Unpack(argc, argv, "oo:__setitem__", &index, &value))
		goto err;
	if unlikely(DeeSeq_OperatorSetItem(self, index, value))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___getrange__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *start = Dee_None, *end = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end, "|oo:__getrange__", &start, &end))
		goto err;
	return DeeSeq_OperatorGetRange(self, start, end);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___delrange__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *start = Dee_None, *end = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end, "|oo:__delrange__", &start, &end))
		goto err;
	if unlikely(DeeSeq_OperatorDelRange(self, start, end))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___setrange__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DeeObject *start, *end, *values;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end_values,
	                    "ooo:__setrange__", &start, &end, &values))
		goto err;
	if unlikely(DeeSeq_OperatorSetRange(self, start, end, values))
		goto err;
	return_none;
err:
	return NULL;
}

#define DEFAULT_SEQ_FOREACH_STOP SSIZE_MIN
struct default_seq___foreach___data {
	DeeObject      *dsfd_cb;  /* [1..1] User-defined callback */
	DREF DeeObject *dsfd_res; /* [0..1] Foreach result */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq___foreach___cb(void *arg, DeeObject *elem) {
	DREF DeeObject *result;
	struct default_seq___foreach___data *data;
	data = (struct default_seq___foreach___data *)arg;
	ASSERT(!data->dsfd_res);
	result = DeeObject_Call(data->dsfd_cb, 1, &elem);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(result);
		return 0;
	}
	data->dsfd_res = result; /* Inherit reference */
	return DEFAULT_SEQ_FOREACH_STOP;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
default_seq___enumerate___cb(void *arg, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *key_and_value[2];
	struct default_seq___foreach___data *data;
	data = (struct default_seq___foreach___data *)arg;
	ASSERT(!data->dsfd_res);
	key_and_value[0] = key;
	key_and_value[1] = value;
	result = DeeObject_Call(data->dsfd_cb, value ? 2 : 1, key_and_value);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(result);
		return 0;
	}
	data->dsfd_res = result; /* Inherit reference */
	return DEFAULT_SEQ_FOREACH_STOP;
err:
	return -1;
}

#ifdef __OPTIMIZE_SIZE__
#define default_seq___foreach_pair___cb default_seq___enumerate___cb
#else /* __OPTIMIZE_SIZE__ */
PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
default_seq___foreach_pair___cb(void *arg, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *key_and_value[2];
	struct default_seq___foreach___data *data;
	data = (struct default_seq___foreach___data *)arg;
	ASSERT(!data->dsfd_res);
	key_and_value[0] = key;
	key_and_value[1] = value;
	result = DeeObject_Call(data->dsfd_cb, 2, key_and_value);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(result);
		return 0;
	}
	data->dsfd_res = result; /* Inherit reference */
	return DEFAULT_SEQ_FOREACH_STOP;
err:
	return -1;
}
#endif /* !__OPTIMIZE_SIZE__ */

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___foreach__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	Dee_ssize_t status;
	struct default_seq___foreach___data data;
	if (DeeArg_Unpack(argc, argv, "o:__foreach__", &data.dsfd_cb))
		goto err;
	data.dsfd_res = NULL;
	status = DeeSeq_OperatorForeach(self, &default_seq___foreach___cb, &data);
	if (status == DEFAULT_SEQ_FOREACH_STOP && data.dsfd_res)
		return data.dsfd_res; /* Inherit reference */
	if unlikely(status < 0)
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___foreach_pair__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	Dee_ssize_t status;
	struct default_seq___foreach___data data;
	if (DeeArg_Unpack(argc, argv, "o:__foreach_pair__", &data.dsfd_cb))
		goto err;
	data.dsfd_res = NULL;
	status = DeeSeq_OperatorForeachPair(self, &default_seq___foreach_pair___cb, &data);
	if (status == DEFAULT_SEQ_FOREACH_STOP && data.dsfd_res)
		return data.dsfd_res; /* Inherit reference */
	if unlikely(status < 0)
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___enumerate__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	Dee_ssize_t status;
	struct default_seq___foreach___data data;
	if (DeeArg_Unpack(argc, argv, "o:__enumerate__", &data.dsfd_cb))
		goto err;
	data.dsfd_res = NULL;
	status = DeeSeq_OperatorEnumerate(self, &default_seq___enumerate___cb, &data);
	if (status == DEFAULT_SEQ_FOREACH_STOP && data.dsfd_res)
		return data.dsfd_res; /* Inherit reference */
	if unlikely(status < 0)
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED Dee_ssize_t DCALL
default_seq___enumerate_index___cb(void *arg, size_t key, DeeObject *value) {
	DREF DeeObject *result;
	DeeObject *key_and_value[2];
	struct default_seq___foreach___data *data;
	data = (struct default_seq___foreach___data *)arg;
	ASSERT(!data->dsfd_res);
	key_and_value[0] = DeeInt_NewSize(key);
	if unlikely(!key_and_value[0])
		goto err;
	key_and_value[1] = value;
	result = DeeObject_Call(data->dsfd_cb, value ? 2 : 1, key_and_value);
	Dee_Decref(key_and_value[0]);
	if unlikely(!result)
		goto err;
	if (DeeNone_Check(result)) {
		Dee_DecrefNokill(result);
		return 0;
	}
	data->dsfd_res = result; /* Inherit reference */
	return DEFAULT_SEQ_FOREACH_STOP;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___enumerate_index__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t status;
	size_t start = 0, end = (size_t)-1;
	struct default_seq___foreach___data data;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__cb_start_end,
	                    "o|" UNPuSIZ UNPuSIZ ":__enumerate_index__",
	                    &data.dsfd_cb, &start, &end))
		goto err;
	data.dsfd_res = NULL;
	status = DeeSeq_OperatorEnumerateIndex(self, &default_seq___enumerate_index___cb, &data, start, end);
	if (status == DEFAULT_SEQ_FOREACH_STOP && data.dsfd_res)
		return data.dsfd_res; /* Inherit reference */
	if unlikely(status < 0)
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___iterkeys__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":__iterkeys__"))
		goto err;
	return DeeSeq_OperatorIterKeys(self);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___bounditem__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *index;
	bool allow_missing = true;
	if (DeeArg_Unpack(argc, argv, "o|b:__bounditem__", &index, &allow_missing))
		goto err;
	switch (DeeSeq_OperatorBoundItem(self, index)) {
	case -2:
		if unlikely(!allow_missing) {
			err_unknown_key(self, index);
			goto err;
		}
		ATTR_FALLTHROUGH
	default:
		return_false;
	case 1:
		return_true;
	case -1:
		break; /* Error */
	}
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___hasitem__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *index;
	if (DeeArg_Unpack(argc, argv, "o:__hasitem__", &index))
		goto err;
	result = DeeSeq_OperatorHasItem(self, index);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___size_fast__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t result;
	if (DeeArg_Unpack(argc, argv, ":__size_fast__"))
		goto err;
	result = DeeSeq_OperatorSizeFast(self);
	if (result == (size_t)-1)
		return_none;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___getitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t index;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":__getitem_index__", &index))
		goto err;
	return DeeSeq_OperatorGetItemIndex(self, index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___delitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t index;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":__delitem_index__", &index))
		goto err;
	if unlikely(DeeSeq_OperatorDelItemIndex(self, index))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___setitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t index;
	DeeObject *value;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "o:__setitem_index__", &index, &value))
		goto err;
	if unlikely(DeeSeq_OperatorSetItemIndex(self, index, value))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___bounditem_index__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t index;
	bool allow_missing = true;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|b:__bounditem_index__", &index, &allow_missing))
		goto err;
	switch (DeeSeq_OperatorBoundItemIndex(self, index)) {
	case -2:
		if unlikely(!allow_missing) {
			err_unknown_key_int(self, index);
			goto err;
		}
		ATTR_FALLTHROUGH
	default:
		return_false;
	case 1:
		return_true;
	case -1:
		break; /* Error */
	}
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___hasitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	size_t index;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":__hasitem_index__", &index))
		goto err;
	result = DeeSeq_OperatorHasItemIndex(self, index);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___getrange_index__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t start = 0, end_index;
	DeeObject *end = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end, "|" UNPdSIZ "o:__getrange_index__", &start, &end))
		goto err;
	if (DeeNone_Check(end))
		return DeeSeq_OperatorGetRangeIndexN(self, start);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return DeeSeq_OperatorGetRangeIndex(self, start, end_index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___delrange_index__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	Dee_ssize_t start = 0, end_index;
	DeeObject *end = Dee_None;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end, "|" UNPdSIZ "o:__delrange_index__", &start, &end))
		goto err;
	if (DeeNone_Check(end)) {
		result = DeeSeq_OperatorDelRangeIndexN(self, start);
	} else {
		if (DeeObject_AsSSize(end, &end_index))
			goto err;
		result = DeeSeq_OperatorDelRangeIndex(self, start, end_index);
	}
	if unlikely(result)
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___setrange_index__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	int result;
	Dee_ssize_t start = 0, end_index;
	DeeObject *end;
	DeeObject *values;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__start_end,
	                    UNPdSIZ "oo:__setrange_index__",
	                    &start, &end, &values))
		goto err;
	if (DeeNone_Check(end)) {
		result = DeeSeq_OperatorSetRangeIndexN(self, start, values);
	} else {
		if (DeeObject_AsSSize(end, &end_index))
			goto err;
		result = DeeSeq_OperatorSetRangeIndex(self, start, end_index, values);
	}
	if unlikely(result)
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___trygetitem__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *index, *def = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:__trygetitem__", &index, &def))
		goto err;
	result = DeeSeq_OperatorTryGetItem(self, index);
	if (result == ITER_DONE) {
		result = def;
		Dee_Incref(def);
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___trygetitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *def = Dee_None;
	size_t index;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:__trygetitem_index__", &index, &def))
		goto err;
	result = DeeSeq_OperatorTryGetItemIndex(self, index);
	if (result == ITER_DONE) {
		result = def;
		Dee_Incref(def);
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___hash__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	Dee_hash_t result;
	if (DeeArg_Unpack(argc, argv, ":__hash__"))
		goto err;
	result = DeeSeq_OperatorHash(self);
	return DeeInt_NewHash(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___compare_eq__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__compare_eq__", &rhs))
		goto err;
	result = DeeSeq_OperatorCompareEq(self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___compare__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int diff;
	DeeObject *rhs;
	DeeObject *result;
	if (DeeArg_Unpack(argc, argv, "o:__compare__", &rhs))
		goto err;
	diff = DeeSeq_OperatorCompare(self, rhs);
	if unlikely(diff == Dee_COMPARE_ERR)
		goto err;
	ASSERT(diff == -1 || diff == 0 || diff == 1);
	result = DeeInt_FromSign(diff);
	return_reference_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___trycompare_eq__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__trycompare_eq__", &rhs))
		goto err;
	result = DeeSeq_OperatorTryCompareEq(self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___eq__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__eq__", &rhs))
		goto err;
	return DeeSeq_OperatorEq(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___ne__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__ne__", &rhs))
		goto err;
	return DeeSeq_OperatorNe(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___lo__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__lo__", &rhs))
		goto err;
	return DeeSeq_OperatorLo(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___le__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__le__", &rhs))
		goto err;
	return DeeSeq_OperatorLe(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___gr__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__gr__", &rhs))
		goto err;
	return DeeSeq_OperatorGr(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___ge__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__ge__", &rhs))
		goto err;
	return DeeSeq_OperatorGe(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___inplace_add__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *items;
	if (DeeArg_Unpack(argc, argv, "o:__inplace_add__", &items))
		goto err;
	Dee_Incref(self);
	if unlikely(DeeSeq_OperatorInplaceAdd(&self, items))
		goto err_self;
	return self;
err_self:
	Dee_Decref(self);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_seq___inplace_mul__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *factor;
	if (DeeArg_Unpack(argc, argv, "o:__inplace_mul__", &factor))
		goto err;
	Dee_Incref(self);
	if unlikely(DeeSeq_OperatorInplaceMul(&self, factor))
		goto err_self;
	return self;
err_self:
	Dee_Decref(self);
err:
	return NULL;
}






INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_set___hash__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	Dee_hash_t result;
	if (DeeArg_Unpack(argc, argv, ":__hash__"))
		goto err;
	result = DeeSet_OperatorHash(self);
	return DeeInt_NewHash(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_set___compare_eq__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__compare_eq__", &rhs))
		goto err;
	result = DeeSet_OperatorCompareEq(self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_set___trycompare_eq__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__trycompare_eq__", &rhs))
		goto err;
	result = DeeSet_OperatorTryCompareEq(self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_set___eq__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__eq__", &rhs))
		goto err;
	return DeeSet_OperatorEq(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_set___ne__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__ne__", &rhs))
		goto err;
	return DeeSet_OperatorNe(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_set___lo__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__lo__", &rhs))
		goto err;
	return DeeSet_OperatorLo(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_set___le__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__le__", &rhs))
		goto err;
	return DeeSet_OperatorLe(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_set___gr__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__gr__", &rhs))
		goto err;
	return DeeSet_OperatorGr(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_set___ge__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__ge__", &rhs))
		goto err;
	return DeeSet_OperatorGe(self, rhs);
err:
	return NULL;
}








/************************************************************************/
/* MAP                                                                  */
/************************************************************************/

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___contains__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *x;
	if (DeeArg_Unpack(argc, argv, "o:__contains__", &x))
		goto err;
	return DeeMap_OperatorContains(self, x);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___getitem__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *index;
	if (DeeArg_Unpack(argc, argv, "o:__getitem__", &index))
		goto err;
	return DeeMap_OperatorGetItem(self, index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___delitem__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *index;
	if (DeeArg_Unpack(argc, argv, "o:__delitem__", &index))
		goto err;
	if unlikely(DeeMap_OperatorDelItem(self, index))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___setitem__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *index, *value;
	if (DeeArg_Unpack(argc, argv, "oo:__setitem__", &index, &value))
		goto err;
	if unlikely(DeeMap_OperatorSetItem(self, index, value))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___enumerate__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	Dee_ssize_t status;
	struct default_seq___foreach___data data;
	if (DeeArg_Unpack(argc, argv, "o:__enumerate__", &data.dsfd_cb))
		goto err;
	data.dsfd_res = NULL;
	status = DeeMap_OperatorEnumerate(self, &default_seq___enumerate___cb, &data);
	if (status == DEFAULT_SEQ_FOREACH_STOP && data.dsfd_res)
		return data.dsfd_res; /* Inherit reference */
	if unlikely(status < 0)
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___enumerate_index__(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t status;
	size_t start = 0, end = (size_t)-1;
	struct default_seq___foreach___data data;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__cb_start_end,
	                    "o|" UNPuSIZ UNPuSIZ ":__enumerate_index__",
	                    &data.dsfd_cb, &start, &end))
		goto err;
	data.dsfd_res = NULL;
	status = DeeMap_OperatorEnumerateIndex(self, &default_seq___enumerate_index___cb, &data, start, end);
	if (status == DEFAULT_SEQ_FOREACH_STOP && data.dsfd_res)
		return data.dsfd_res; /* Inherit reference */
	if unlikely(status < 0)
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___iterkeys__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":__iterkeys__"))
		goto err;
	return DeeMap_OperatorIterKeys(self);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___bounditem__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *index;
	bool allow_missing = true;
	if (DeeArg_Unpack(argc, argv, "o|b:__bounditem__", &index, &allow_missing))
		goto err;
	switch (DeeMap_OperatorBoundItem(self, index)) {
	case -2:
		if unlikely(!allow_missing) {
			err_unknown_key(self, index);
			goto err;
		}
		ATTR_FALLTHROUGH
	default:
		return_false;
	case 1:
		return_true;
	case -1:
		break; /* Error */
	}
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___hasitem__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *index;
	if (DeeArg_Unpack(argc, argv, "o:__hasitem__", &index))
		goto err;
	result = DeeMap_OperatorHasItem(self, index);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___getitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t index;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":__getitem_index__", &index))
		goto err;
	return DeeMap_OperatorGetItemIndex(self, index);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___delitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t index;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":__delitem_index__", &index))
		goto err;
	if unlikely(DeeMap_OperatorDelItemIndex(self, index))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___setitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t index;
	DeeObject *value;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "o:__setitem_index__", &index, &value))
		goto err;
	if unlikely(DeeMap_OperatorSetItemIndex(self, index, value))
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___bounditem_index__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	size_t index;
	bool allow_missing = true;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|b:__bounditem_index__", &index, &allow_missing))
		goto err;
	switch (DeeMap_OperatorBoundItemIndex(self, index)) {
	case -2:
		if unlikely(!allow_missing) {
			err_unknown_key_int(self, index);
			goto err;
		}
		ATTR_FALLTHROUGH
	default:
		return_false;
	case 1:
		return_true;
	case -1:
		break; /* Error */
	}
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___hasitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	size_t index;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":__hasitem_index__", &index))
		goto err;
	result = DeeMap_OperatorHasItemIndex(self, index);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___trygetitem__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *index, *def = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:__trygetitem__", &index, &def))
		goto err;
	result = DeeMap_OperatorTryGetItem(self, index);
	if (result == ITER_DONE) {
		result = def;
		Dee_Incref(def);
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___trygetitem_index__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *def = Dee_None;
	size_t index;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:__trygetitem_index__", &index, &def))
		goto err;
	result = DeeMap_OperatorTryGetItemIndex(self, index);
	if (result == ITER_DONE) {
		result = def;
		Dee_Incref(def);
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___trygetitem_string__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *key, *def = Dee_None;
	if (DeeArg_Unpack(argc, argv, "o|o:__trygetitem_string__", &key, &def))
		goto err;
	if (DeeBytes_Check(key)) {
		char const *data = (char const *)DeeBytes_DATA(key);
		size_t size      = DeeBytes_SIZE(key);
		Dee_hash_t hash  = Dee_HashPtr(data, size);
		result = DeeMap_OperatorTryGetItemStringLenHash(self, data, size, hash);
	} else if (DeeString_Check(key)) {
		result = DeeMap_OperatorTryGetItemStringHash(self, DeeString_STR(key), DeeString_Hash(key));
	} else {
		DeeObject_TypeAssertFailed2(key, &DeeBytes_Type, &DeeString_Type);
		goto err;
	}
	if (result == ITER_DONE) {
		result = def;
		Dee_Incref(def);
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___getitem_string__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__getitem_string__", &key))
		goto err;
	if (DeeBytes_Check(key)) {
		char const *data = (char const *)DeeBytes_DATA(key);
		size_t size      = DeeBytes_SIZE(key);
		Dee_hash_t hash  = Dee_HashPtr(data, size);
		result = DeeMap_OperatorGetItemStringLenHash(self, data, size, hash);
	} else if (DeeString_Check(key)) {
		result = DeeMap_OperatorGetItemStringHash(self, DeeString_STR(key), DeeString_Hash(key));
	} else {
		DeeObject_TypeAssertFailed2(key, &DeeBytes_Type, &DeeString_Type);
		goto err;
	}
	return result;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___delitem_string__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *key;
	if (DeeArg_Unpack(argc, argv, "o:__delitem_string__", &key))
		goto err;
	if (DeeBytes_Check(key)) {
		char const *data = (char const *)DeeBytes_DATA(key);
		size_t size      = DeeBytes_SIZE(key);
		Dee_hash_t hash  = Dee_HashPtr(data, size);
		result = DeeMap_OperatorDelItemStringLenHash(self, data, size, hash);
	} else if (DeeString_Check(key)) {
		result = DeeMap_OperatorDelItemStringHash(self, DeeString_STR(key), DeeString_Hash(key));
	} else {
		DeeObject_TypeAssertFailed2(key, &DeeBytes_Type, &DeeString_Type);
		goto err;
	}
	if unlikely(result)
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___setitem_string__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *key, *value;
	if (DeeArg_Unpack(argc, argv, "oo:__setitem_string__", &key, &value))
		goto err;
	if (DeeBytes_Check(key)) {
		char const *data = (char const *)DeeBytes_DATA(key);
		size_t size      = DeeBytes_SIZE(key);
		Dee_hash_t hash  = Dee_HashPtr(data, size);
		result = DeeMap_OperatorSetItemStringLenHash(self, data, size, hash, value);
	} else if (DeeString_Check(key)) {
		result = DeeMap_OperatorSetItemStringHash(self, DeeString_STR(key), DeeString_Hash(key), value);
	} else {
		DeeObject_TypeAssertFailed2(key, &DeeBytes_Type, &DeeString_Type);
		goto err;
	}
	if unlikely(result)
		goto err;
	return_none;
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___bounditem_string__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *key;
	bool allow_missing = true;
	if (DeeArg_Unpack(argc, argv, "o|b:__bounditem_string__", &key, &allow_missing))
		goto err;
	if (DeeBytes_Check(key)) {
		char const *data = (char const *)DeeBytes_DATA(key);
		size_t size      = DeeBytes_SIZE(key);
		Dee_hash_t hash  = Dee_HashPtr(data, size);
		result = DeeMap_OperatorBoundItemStringLenHash(self, data, size, hash);
	} else if (DeeString_Check(key)) {
		result = DeeMap_OperatorBoundItemStringHash(self, DeeString_STR(key), DeeString_Hash(key));
	} else {
		DeeObject_TypeAssertFailed2(key, &DeeBytes_Type, &DeeString_Type);
		goto err;
	}
	switch (result) {
	case -2:
		if unlikely(!allow_missing) {
			err_unknown_key(self, key);
			goto err;
		}
		ATTR_FALLTHROUGH
	default:
		return_false;
	case 1:
		return_true;
	case -1:
		break; /* Error */
	}
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___hasitem_string__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *key;
	bool allow_missing = true;
	if (DeeArg_Unpack(argc, argv, "o|b:__bounditem_string__", &key, &allow_missing))
		goto err;
	if (DeeBytes_Check(key)) {
		char const *data = (char const *)DeeBytes_DATA(key);
		size_t size      = DeeBytes_SIZE(key);
		Dee_hash_t hash  = Dee_HashPtr(data, size);
		result = DeeMap_OperatorHasItemStringLenHash(self, data, size, hash);
	} else if (DeeString_Check(key)) {
		result = DeeMap_OperatorHasItemStringHash(self, DeeString_STR(key), DeeString_Hash(key));
	} else {
		DeeObject_TypeAssertFailed2(key, &DeeBytes_Type, &DeeString_Type);
		goto err;
	}
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___compare_eq__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__compare_eq__", &rhs))
		goto err;
	result = DeeMap_OperatorCompareEq(self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___trycompare_eq__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__trycompare_eq__", &rhs))
		goto err;
	result = DeeMap_OperatorTryCompareEq(self, rhs);
	if unlikely(result == Dee_COMPARE_ERR)
		goto err;
	return_bool(result == 0);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___eq__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__eq__", &rhs))
		goto err;
	return DeeMap_OperatorEq(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___ne__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__ne__", &rhs))
		goto err;
	return DeeMap_OperatorNe(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___lo__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__lo__", &rhs))
		goto err;
	return DeeMap_OperatorLo(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___le__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__le__", &rhs))
		goto err;
	return DeeMap_OperatorLe(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___gr__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__gr__", &rhs))
		goto err;
	return DeeMap_OperatorGr(self, rhs);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
default_map___ge__(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *rhs;
	if (DeeArg_Unpack(argc, argv, "o:__ge__", &rhs))
		goto err;
	return DeeMap_OperatorGe(self, rhs);
err:
	return NULL;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_OPERATOR_METHODS_C_INL */
