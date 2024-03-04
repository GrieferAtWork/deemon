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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_CONSTCALL_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_CONSTCALL_C 1

#include <deemon/api.h>
#include <deemon/bytes.h>
#include <deemon/kwds.h>
#include <deemon/object.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/tuple.h>

DECL_BEGIN

PRIVATE ATTR_PURE WUNUSED ATTR_INS(2, 1) NONNULL((4)) bool DCALL
check_foreach_args_kw(size_t argc, DeeObject *const *argv, DeeObject *kw,
                      bool (DCALL *check)(DeeObject *ob)) {
	size_t i;
	for (i = 0; i < argc; ++i) {
		if (!(*check)(argv[i]))
			goto nope;
	}
	if (kw) {
		DeeTypeObject *tp_kw = Dee_TYPE(kw);
		if (tp_kw == &DeeKwds_Type) {
			/* No extra values in here :) */
		} else if (tp_kw == &DeeRoDict_Type) {
			DeeRoDictObject *kwob = (DeeRoDictObject *)kw;
			for (i = 0; i <= kwob->rd_mask; ++i) {
				struct rodict_item *it = &kwob->rd_elem[i];
				if (!it->rdi_key)
					continue;
				/*if (!(*check)(it->rdi_key))
					goto nope;*/ /* For keywords, only the value matters */
				if (!(*check)(it->rdi_value))
					goto nope;
			}
		} else {
			/* TODO: Support for `DeeKwdsMapping_Type' */
			/* TODO: Support for `DeeBlackListKwds_Type' */
			/* TODO: Support for `DeeBlackListKw_Type' */

			/* Unknown keywords type (better be safe than sorry) */
			goto nope;
		}
	}
	return true;
nope:
	return false;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
check_foreach_elem(DeeObject *seq, bool (DCALL *check)(DeeObject *ob)) {
	DeeTypeObject *tp_seq = Dee_TYPE(seq);
	if (tp_seq == &DeeTuple_Type) {
		size_t i;
		DeeTupleObject *me = (DeeTupleObject *)seq;
		for (i = 0; i < me->t_size; ++i) {
			if (!(*check)(me->t_elem[i]))
				goto nope;
		}
		return true;
	} else if (tp_seq == &DeeRoSet_Type) {
		size_t i;
		DeeRoSetObject *me = (DeeRoSetObject *)seq;
		for (i = 0; i <= me->rs_mask; ++i) {
			DeeObject *key = me->rs_elem[i].rsi_key;
			if (!key)
				continue;
			if (!(*check)(key))
				goto nope;
		}
		return true;
	} else if (tp_seq == &DeeRoDict_Type) {
		size_t i;
		DeeRoDictObject *me = (DeeRoDictObject *)seq;
		for (i = 0; i <= me->rd_mask; ++i) {
			struct rodict_item *it = &me->rd_elem[i];
			if (!it->rdi_key)
				continue;
			if (!(*check)(it->rdi_key))
				goto nope;
			if (!(*check)(it->rdi_value))
				goto nope;
		}
		return true;
	}
nope:
	return false;
}

PRIVATE ATTR_PURE WUNUSED bool DCALL
ob_is_const_castable(DeeObject *ob) {
	return DeeType_IsConstCastable(Dee_TYPE(ob));
}

PRIVATE ATTR_PURE WUNUSED bool DCALL
ob_is_const_castable_or_robytes(DeeObject *ob) {
	DeeTypeObject *tp = Dee_TYPE(ob);
	if (tp == &DeeBytes_Type)
		return !DeeBytes_WRITABLE(ob);
	return DeeType_IsConstCastable(tp);
}

PRIVATE ATTR_PURE WUNUSED bool DCALL
ob_is_const_str(DeeObject *ob) {
	DeeTypeObject *tp = Dee_TYPE(ob);
	uintptr_t str_flags;
	if (!DeeType_HasOperator(tp, OPERATOR_STR))
		return true;
	str_flags = DeeType_GetOperatorFlags(tp, OPERATOR_STR);
	return (str_flags & METHOD_FCONSTCALL) &&
	       DeeMethodFlags_VerifyConstCallCondition(str_flags, ob, 0, NULL, NULL);
}

PRIVATE ATTR_PURE WUNUSED bool DCALL
ob_is_const_repr(DeeObject *ob) {
	DeeTypeObject *tp = Dee_TYPE(ob);
	uintptr_t repr_flags;
	if (!DeeType_HasOperator(tp, OPERATOR_REPR))
		return true;
	repr_flags = DeeType_GetOperatorFlags(tp, OPERATOR_REPR);
	return (repr_flags & METHOD_FCONSTCALL) &&
	       DeeMethodFlags_VerifyConstCallCondition(repr_flags, ob, 0, NULL, NULL);
}

PRIVATE ATTR_PURE WUNUSED bool DCALL
ob_is_const_hash(DeeObject *ob) {
	DeeTypeObject *tp = Dee_TYPE(ob);
	uintptr_t hash_flags;
	if (!DeeType_HasOperator(tp, OPERATOR_HASH))
		return true;
	hash_flags = DeeType_GetOperatorFlags(tp, OPERATOR_HASH);
	return (hash_flags & METHOD_FCONSTCALL) &&
	       DeeMethodFlags_VerifyConstCallCondition(hash_flags, ob, 0, NULL, NULL);
}

PRIVATE ATTR_PURE WUNUSED bool DCALL
ob_is_const_deep(DeeObject *ob) {
	DeeTypeObject *tp = Dee_TYPE(ob);
	uintptr_t deepcopy_flags;
	if (!DeeType_HasOperator(tp, OPERATOR_DEEPCOPY))
		return true;
	deepcopy_flags = DeeType_GetOperatorFlags(tp, OPERATOR_DEEPCOPY);
	return (deepcopy_flags & METHOD_FCONSTCALL) &&
	       DeeMethodFlags_VerifyConstCallCondition(deepcopy_flags, ob, 0, NULL, NULL);
}

PRIVATE ATTR_PURE WUNUSED bool DCALL
ob_is_elem_const_str(DeeObject *ob) {
	return check_foreach_elem(ob, &ob_is_const_str);
}


/* Check if the condition from `flags & METHOD_FCONSTCALL_IF_MASK' is
 * fulfilled when applied to the given argument list (which does not
 * include the "this" argument, if there would have been one). */
PUBLIC ATTR_PURE WUNUSED ATTR_INS(4, 3) bool
(DCALL DeeMethodFlags_VerifyConstCallCondition)(uintptr_t flags, DeeObject *thisarg,
                                                size_t argc, DeeObject *const *argv,
                                                DeeObject *kw) {
	(void)thisarg;
	switch (flags & METHOD_FCONSTCALL_IF_MASK) {

	case METHOD_FCONSTCALL_IF_TRUE:
		return true;

	case METHOD_FCONSTCALL_IF_ARGS_CONSTCAST:
		return check_foreach_args_kw(argc, argv, kw, &ob_is_const_castable);

	case METHOD_FCONSTCALL_IF_ARGS_CONSTSTR:
		return check_foreach_args_kw(argc, argv, kw, &ob_is_const_str);

	case METHOD_FCONSTCALL_IF_THISELEM_CONSTSTR:
		return thisarg && check_foreach_elem(thisarg, &ob_is_const_str);

	case METHOD_FCONSTCALL_IF_THISELEM_CONSTREPR:
		return thisarg && check_foreach_elem(thisarg, &ob_is_const_repr);

	case METHOD_FCONSTCALL_IF_THISELEM_CONSTHASH:
		return thisarg && check_foreach_elem(thisarg, &ob_is_const_hash);

	case METHOD_FCONSTCALL_IF_THISELEM_CONSTDEEP:
		return thisarg && check_foreach_elem(thisarg, &ob_is_const_deep);

	//TODO: case METHOD_FCONSTCALL_IF_SEQ_CONSTCOMPARE:
	//TODO: case METHOD_FCONSTCALL_IF_SEQ_CONSTCONTAINS:
	//TODO: case METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS:
	//TODO: case METHOD_FCONSTCALL_IF_MAP_CONSTCONTAINS:

	case METHOD_FCONSTCALL_IF_ARGSELEM_CONSTSTR_ROBYTES:
		if (thisarg && DeeBytes_Check(thisarg) && DeeBytes_WRITABLE(thisarg))
			goto nope;
		return check_foreach_args_kw(argc, argv, kw, &ob_is_elem_const_str);

	case METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES:
		if (thisarg && DeeBytes_Check(thisarg) && DeeBytes_WRITABLE(thisarg))
			goto nope;
		return check_foreach_args_kw(argc, argv, kw, &ob_is_const_castable_or_robytes);


	default: break;
	}
nope:
	return false;
}

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_CONSTCALL_C */
