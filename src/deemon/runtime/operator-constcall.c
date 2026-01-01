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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_CONSTCALL_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_CONSTCALL_C 1

#include <deemon/api.h>
#include <deemon/bytes.h>
#include <deemon/dict.h>
#include <deemon/instancemethod.h>
#include <deemon/kwds.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/rodict.h>
#include <deemon/roset.h>
#include <deemon/tuple.h>

#include <hybrid/typecore.h>
/**/

#include <stddef.h> /* uintptr_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

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
			for (i = 0; i < kwob->rd_vsize; ++i) {
				struct Dee_dict_item *item;
				item = &_DeeRoDict_GetRealVTab(kwob)[i];
				/*if (!(*check)(it->di_key))
					goto nope;*/ /* For keywords, only the value matters */
				if (!(*check)(item->di_value))
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
		for (i = 0; i < me->rd_vsize; ++i) {
			struct Dee_dict_item *item;
			item = &_DeeRoDict_GetRealVTab(me)[i];
			if (!(*check)(item->di_key))
				goto nope;
			if (!(*check)(item->di_value))
				goto nope;
		}
		return true;
	}
nope:
	return false;
}

/* Enumerate the object-like "tp_members" fields of "ob" (excluding "ob_type") */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
check_foreach_field(DeeObject *ob, bool (DCALL *check)(DeeObject *ob)) {
	DeeTypeObject *tp = Dee_TYPE(ob);
	while (tp != &DeeObject_Type) {
		struct type_member const *iter;
		iter = tp->tp_members;
		if (iter) {
			for (; iter->m_name; ++iter) {
				if (!Dee_TYPE_MEMBER_ISFIELD(iter))
					continue;
				if (iter->m_desc.md_field.mdf_type == Dee_STRUCT_OBJECT ||
				    iter->m_desc.md_field.mdf_type == Dee_STRUCT_OBJECT_OPT) {
					DeeObject *value = *(DeeObject *const *)((byte_t *)ob + iter->m_desc.md_field.mdf_offset);
					if (value && !(*check)(value))
						goto nope;
				}
			}
		}
		tp = DeeType_Base(tp);
		if unlikely(!tp)
			break;
	}
	return true;
nope:
	return false;
}

/* Enumerate the object-like "tp_members" fields of "ob" (excluding "ob_type") */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2, 3)) bool DCALL
check_foreach_field2(DeeObject *a, DeeObject *b,
                     bool (DCALL *check)(DeeObject *lhs, DeeObject *rhs)) {
	DeeTypeObject *tp = Dee_TYPE(a);
	if (tp != Dee_TYPE(b))
		goto nope; /* Types must be identical */
	while (tp != &DeeObject_Type) {
		struct type_member const *iter;
		iter = tp->tp_members;
		if (iter) {
			for (; iter->m_name; ++iter) {
				if (!Dee_TYPE_MEMBER_ISFIELD(iter))
					continue;
				if (iter->m_desc.md_field.mdf_type == Dee_STRUCT_OBJECT ||
				    iter->m_desc.md_field.mdf_type == Dee_STRUCT_OBJECT_OPT) {
					DeeObject *lhs = *(DeeObject *const *)((byte_t *)a + iter->m_desc.md_field.mdf_offset);
					DeeObject *rhs = *(DeeObject *const *)((byte_t *)b + iter->m_desc.md_field.mdf_offset);
					if ((lhs != NULL) != (rhs != NULL))
						goto nope;
					if (lhs && !(*check)(lhs, rhs))
						goto nope;
				}
			}
		}
		tp = DeeType_Base(tp);
		if unlikely(!tp)
			break;
	}
	return true;
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
ob_is_elem_const_castable(DeeObject *ob) {
	return check_foreach_elem(ob, &ob_is_const_castable);
}

PRIVATE ATTR_PURE WUNUSED bool DCALL
ob_is_elem_const_str(DeeObject *ob) {
	return check_foreach_elem(ob, &ob_is_const_str);
}

PRIVATE ATTR_PURE WUNUSED bool DCALL
ob_is_fields_const_str(DeeObject *ob) {
	return check_foreach_field(ob, &ob_is_const_str);
}

PRIVATE ATTR_PURE WUNUSED bool DCALL
ob_is_fields_const_repr(DeeObject *ob) {
	return check_foreach_field(ob, &ob_is_const_repr);
}

PRIVATE ATTR_PURE WUNUSED bool DCALL
ob_is_const_cmpeq(DeeObject *a, DeeObject *b) {
	DeeTypeObject *tp = Dee_TYPE(a);
	uintptr_t eq_flags, ne_flags;
	bool has_eq, has_hash;
	if (tp != Dee_TYPE(b))
		return false;
	has_eq   = DeeType_HasOperator(tp, OPERATOR_EQ);
	has_hash = DeeType_HasOperator(tp, OPERATOR_NE);
	if (!has_eq)
		return !has_hash;
	eq_flags = DeeType_GetOperatorFlags(tp, OPERATOR_EQ);
	if (!(eq_flags & METHOD_FCONSTCALL) ||
	    !(DeeMethodFlags_VerifyConstCallCondition(eq_flags, a, 1, &b, NULL)))
		return false;
	ne_flags = DeeType_GetOperatorFlags(tp, OPERATOR_NE);
	if (!(ne_flags & METHOD_FCONSTCALL) ||
	    !(DeeMethodFlags_VerifyConstCallCondition(ne_flags, a, 1, &b, NULL)))
		return false;
	if (has_hash) {
		uintptr_t hash_flags;
		hash_flags = DeeType_GetOperatorFlags(tp, OPERATOR_HASH);
		if (!(hash_flags & METHOD_FCONSTCALL) ||
		    !(DeeMethodFlags_VerifyConstCallCondition(hash_flags, a, 0, NULL, NULL)))
			return false;
		if (Dee_TYPE(b) != tp) {
			hash_flags = DeeType_GetOperatorFlags(Dee_TYPE(b), OPERATOR_HASH);
			if (!(hash_flags & METHOD_FCONSTCALL) ||
			    !(DeeMethodFlags_VerifyConstCallCondition(hash_flags, b, 0, NULL, NULL)))
				return false;
		}
	}
	return true;
}

PRIVATE ATTR_CONST WUNUSED uintptr_t DCALL
join_flags(uintptr_t a, uintptr_t b) {
	uintptr_t result = a & b;
	if (result & METHOD_FCONSTCALL) {
		if ((a & METHOD_FCONSTCALL_IF_MASK) != (b & METHOD_FCONSTCALL_IF_MASK)) {
			result &= ~METHOD_FCONSTCALL_IF_MASK;
			if ((a & METHOD_FCONSTCALL_IF_MASK) == METHOD_FCONSTCALL_IF_TRUE) {
				result |= b & METHOD_FCONSTCALL_IF_MASK;
			} else if ((b & METHOD_FCONSTCALL_IF_MASK) == METHOD_FCONSTCALL_IF_TRUE) {
				result |= a & METHOD_FCONSTCALL_IF_MASK;
			} else {
				result &= ~METHOD_FCONSTCALL;
			}
		}
	}
	return result;
}

PRIVATE ATTR_PURE WUNUSED bool DCALL
ob_is_const_cmp(DeeObject *a, DeeObject *b) {
	DeeTypeObject *tp = Dee_TYPE(a);
	uintptr_t eq_flags, lo_flags;
	bool has_eq, has_lo, has_le;
	if (tp != Dee_TYPE(b))
		return false;
	/* NOTE: No need to check for NE, GR, GE -- those are always present when their
	 *       logical inverse is present due to `default__ne__with__eq', ... */
	has_eq = DeeType_HasOperator(tp, OPERATOR_EQ);
	has_lo = DeeType_HasOperator(tp, OPERATOR_LO);
	has_le = DeeType_HasOperator(tp, OPERATOR_LE);
	if (!has_eq || !has_lo || !has_le)
		return !has_eq && !has_lo && !has_le;

	/* EQ and NE must be present and constexpr */
	eq_flags = DeeType_GetOperatorFlags(tp, OPERATOR_EQ);
	if (!(eq_flags & METHOD_FCONSTCALL))
		return false;
	eq_flags = join_flags(eq_flags, DeeType_GetOperatorFlags(tp, OPERATOR_NE));
	if (!(eq_flags & METHOD_FCONSTCALL) ||
	    !(DeeMethodFlags_VerifyConstCallCondition(eq_flags, a, 1, &b, NULL)))
		return false;

	/* The other compare operators must also be present, but
	 * are allowed to have a different constexpr condition. */
	lo_flags = DeeType_GetOperatorFlags(tp, OPERATOR_LO);
	if (!(lo_flags & METHOD_FCONSTCALL))
		return false;
	lo_flags = join_flags(lo_flags, DeeType_GetOperatorFlags(tp, OPERATOR_LE));
	if (!(lo_flags & METHOD_FCONSTCALL))
		return false;
	lo_flags = join_flags(lo_flags, DeeType_GetOperatorFlags(tp, OPERATOR_GR));
	if (!(lo_flags & METHOD_FCONSTCALL))
		return false;
	lo_flags = join_flags(lo_flags, DeeType_GetOperatorFlags(tp, OPERATOR_GE));
	if (!(lo_flags & METHOD_FCONSTCALL))
		return false;
	return DeeMethodFlags_VerifyConstCallCondition(lo_flags, a, 1, &b, NULL);
}


PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
ob_is_func_constcall(DeeObject *ob, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	DeeTypeObject *tp = Dee_TYPE(ob);
	if (tp == &DeeInstanceMethod_Type) {
		DeeInstanceMethodObject *me = (DeeInstanceMethodObject *)ob;
		uintptr_t flags = DeeType_GetOperatorFlags(Dee_TYPE(me->im_func), OPERATOR_CALL);
		return (flags & METHOD_FCONSTCALL) &&
		       DeeMethodFlags_VerifyConstCallCondition(flags, me->im_this, argc, argv, kw);
	} else if (tp == &DeeObjMethod_Type ||
	           tp == &DeeKwObjMethod_Type) {
		struct objmethod_origin origin;
		DeeObjMethodObject *me = (DeeObjMethodObject *)ob;
		if (!DeeObjMethod_GetOrigin((DeeObject *)me, &origin))
			goto nope;
		return (origin.omo_decl->m_flag & METHOD_FCONSTCALL) &&
		       DeeMethodFlags_VerifyConstCallCondition(origin.omo_decl->m_flag,
		                                               me->om_this, argc, argv, kw);
	} else if (tp == &DeeClsMethod_Type ||
	           tp == &DeeKwClsMethod_Type) {
		struct objmethod_origin origin;
		DeeClsMethodObject *me = (DeeClsMethodObject *)ob;
		if (argc == 0)
			goto nope;
		if (!DeeClsMethod_GetOrigin((DeeObject *)me, &origin))
			goto nope;
		return (origin.omo_decl->m_flag & METHOD_FCONSTCALL) &&
		       DeeMethodFlags_VerifyConstCallCondition(origin.omo_decl->m_flag,
		                                               argv[0], argc + 1, argv - 1, kw);
	} else if (tp == &DeeClsProperty_Type) {
		struct clsproperty_origin origin;
		DeeClsPropertyObject *me = (DeeClsPropertyObject *)ob;
		if (argc == 0)
			goto nope;
		if (!DeeClsProperty_GetOrigin((DeeObject *)me, &origin))
			goto nope;
		return (origin.cpo_decl->gs_flags & METHOD_FCONSTCALL) &&
		       DeeMethodFlags_VerifyConstCallCondition(origin.cpo_decl->gs_flags,
		                                               argv[0], argc + 1, argv - 1, kw);
	} else if (tp == &DeeClsMember_Type) {
		DeeClsMemberObject *me = (DeeClsMemberObject *)ob;
		if (TYPE_MEMBER_ISCONST(&me->cmb_memb))
			return true;
		/* "Const && Atomic" would mean that the field can change, so we require "Const && !Atomic" */
		return (me->cmb_memb.m_desc.md_field.mdf_type & (STRUCT_ATOMIC | STRUCT_CONST)) == STRUCT_CONST;
	} else if (tp == &DeeCMethod_Type || tp == &DeeCMethod0_Type ||
	           tp == &DeeCMethod1_Type || tp == &DeeKwCMethod_Type) {
		DeeCMethodObject *me = (DeeCMethodObject *)ob;
		return (me->cm_flags & METHOD_FCONSTCALL) &&
		       DeeMethodFlags_VerifyConstCallCondition(me->cm_flags, NULL, argc, argv, kw);
	}
nope:
	return false;
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

	case METHOD_FCONSTCALL_IF_THISELEM_CONSTSTR:
		return thisarg && check_foreach_elem(thisarg, &ob_is_const_str);

	case METHOD_FCONSTCALL_IF_THISELEM_CONSTREPR:
		return thisarg && check_foreach_elem(thisarg, &ob_is_const_repr);

	case METHOD_FCONSTCALL_IF_THISELEM_CONSTHASH:
		return thisarg && check_foreach_elem(thisarg, &ob_is_const_hash);

	case METHOD_FCONSTCALL_IF_THISELEM_CONSTDEEP:
		return thisarg && check_foreach_elem(thisarg, &ob_is_const_deep);

	// TODO: case METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ:
	// TODO: case METHOD_FCONSTCALL_IF_SEQ_CONSTCMP:

	// TODO: case METHOD_FCONSTCALL_IF_SEQ_CONSTCONTAINS:
	// TODO: case METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS:
	// TODO: case METHOD_FCONSTCALL_IF_MAP_CONSTCONTAINS:

	case METHOD_FCONSTCALL_IF_ARGSELEM_CONSTCAST_ROBYTES:
		if (thisarg && DeeBytes_Check(thisarg) && DeeBytes_WRITABLE(thisarg))
			goto nope;
		return check_foreach_args_kw(argc, argv, kw, &ob_is_elem_const_castable);

	case METHOD_FCONSTCALL_IF_ARGSELEM_CONSTSTR_ROBYTES:
		if (thisarg && DeeBytes_Check(thisarg) && DeeBytes_WRITABLE(thisarg))
			goto nope;
		return check_foreach_args_kw(argc, argv, kw, &ob_is_elem_const_str);

	case METHOD_FCONSTCALL_IF_ARGS_CONSTCAST_ROBYTES:
		if (thisarg && DeeBytes_Check(thisarg) && DeeBytes_WRITABLE(thisarg))
			goto nope;
		return check_foreach_args_kw(argc, argv, kw, &ob_is_const_castable_or_robytes);

	case METHOD_FCONSTCALL_IF_ARGS_CONSTSTR_ROBYTES:
		if (thisarg && DeeBytes_Check(thisarg) && DeeBytes_WRITABLE(thisarg))
			goto nope;
		return check_foreach_args_kw(argc, argv, kw, &ob_is_const_str);

	case METHOD_FCONSTCALL_IF_FUNC_IS_CONSTCALL:
		return thisarg && ob_is_func_constcall(thisarg, argc, argv, kw);

	case METHOD_FCONSTCALL_IF_FIELDS_CONSTSTR:
		return (!thisarg || ob_is_fields_const_str(thisarg)) &&
		       (check_foreach_args_kw(argc, argv, kw, &ob_is_fields_const_str));
	case METHOD_FCONSTCALL_IF_FIELDS_CONSTREPR:
		return (!thisarg || ob_is_fields_const_repr(thisarg)) &&
		       (check_foreach_args_kw(argc, argv, kw, &ob_is_fields_const_repr));
	case METHOD_FCONSTCALL_IF_FIELDS_CONSTCMPEQ:
		return thisarg && argc == 1 && check_foreach_field2(thisarg, argv[0], &ob_is_const_cmpeq);
	case METHOD_FCONSTCALL_IF_FIELDS_CONSTCMP:
		return thisarg && argc == 1 && check_foreach_field2(thisarg, argv[0], &ob_is_const_cmp);

	default: break;
	}
nope:
	return false;
}

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_CONSTCALL_C */
