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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_HINTS_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_HINTS_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>           /* Dee_Allocac, Dee_Free, Dee_TryCalloc, Dee_UntrackAlloc */
#include <deemon/class.h>           /* DeeClass_TryGetPrivateOperatorPtr, Dee_CLASS_OPERATOR_PRINT, Dee_CLASS_OPERATOR_PRINTREPR */
#include <deemon/format.h>          /* PRFuSIZ */
#include <deemon/method-hints.h>    /* DeeType_*MethodHint*, Dee_tmh_id */
#include <deemon/object.h>
#include <deemon/operator-hints.h>  /* DeeType_GetNativeOperatorOOM, DeeType_GetNativeOperatorUnsupported, DeeType_GetOperatorOfTno, Dee_compact_tno_id_t, Dee_tno_id, usrtype__* */
#include <deemon/seq.h>             /* DeeType_GetSeqClass, Dee_SEQCLASS_* */
#include <deemon/system-features.h> /* memcpyc, memmovedownc, memset */
#include <deemon/util/atomic.h>     /* atomic_* */

#include <hybrid/typecore.h> /* __BYTE_TYPE__, __CHAR_BIT__, __UINTPTR_HALF_TYPE__ */

#include "method-hint-defaults.h"
#include "method-hints.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

DECL_BEGIN
STATIC_ASSERT_MSG(Dee_BOUND_ERR == Dee_HAS_ERR &&
                  Dee_BOUND_MISSING == Dee_HAS_NO &&
                  Dee_BOUND_YES == Dee_HAS_YES,
                  "These equivalences are required in order to allow type implementations to "
                  "unconditionally implement (e.g.) `tp_hasitem' using the same function pointer "
                  "as used to implement `tp_bounditem'");

STATIC_ASSERT_MSG(Dee_BOUND_ERR < 0, "Required for hasitem=bounditem aliasing");
STATIC_ASSERT_MSG(Dee_BOUND_MISSING == 0, "Required for hasitem=bounditem aliasing");
STATIC_ASSERT_MSG(Dee_BOUND_YES > 0, "Required for hasitem=bounditem aliasing");
STATIC_ASSERT_MSG(Dee_BOUND_NO > 0, "Required for hasitem=bounditem aliasing");


struct oh_init_spec_class {
	Dee_funptr_t   ohisc_usertyp; /* [1..1] `usertyp__*' implementation. */
	Dee_operator_t ohisc_dep1;    /* Operator used by `ohis_class', or `OPERATOR_USERCOUNT' if not applicable */;
	Dee_operator_t ohisc_dep2;    /* Operator used by `ohis_class', or `OPERATOR_USERCOUNT' if not applicable */;
};
#define OH_INIT_SPEC_CLASS_END { NULL, OPERATOR_USERCOUNT, OPERATOR_USERCOUNT }
#define OH_INIT_SPEC_CLASS_INIT(ohisc_usertyp, ohisc_dep1, ohisc_dep2) \
	{                                                                  \
		/* .ohisc_usertyp = */ (Dee_funptr_t)(ohisc_usertyp),          \
		/* .ohisc_dep1    = */ ohisc_dep1,                             \
		/* .ohisc_dep2    = */ ohisc_dep2                              \
	}

struct oh_init_spec_impl {
	Dee_funptr_t          ohisi_impl;    /* [1..1] Default impl (e.g. `default__seq_iter__with__seq_foreach') */
	__UINTPTR_HALF_TYPE__ ohisi_deps[2]; /* Dependent operators (or `>= Dee_TNO_COUNT' if unused; when both are equal, the impl is disliked) */
};
#define oh_init_spec_impl_isdisliked(self) ((self)->ohisi_deps[0] == (self)->ohisi_deps[1])

#define OH_INIT_SPEC_IMPL_END { NULL, { (__UINTPTR_HALF_TYPE__)Dee_TNO_COUNT, (__UINTPTR_HALF_TYPE__)Dee_TNO_COUNT } }
#define OH_INIT_SPEC_IMPL_INIT(ohisi_impl, ohisi_dep1, ohisi_dep2) \
	{                                                              \
		/* .ohisi_impl = */ (Dee_funptr_t)(ohisi_impl),            \
		/* .ohisi_deps = */ {                                      \
			(__UINTPTR_HALF_TYPE__)(ohisi_dep1),                   \
			(__UINTPTR_HALF_TYPE__)(ohisi_dep2)                    \
		}                                                          \
	}

struct oh_init_inherit_as {
	Dee_funptr_t ohia_optr; /* [1..1] Map this function pointer... */
	Dee_funptr_t ohia_nptr; /* [1..1] ... into this one */
};
#define OH_INIT_INHERIT_AS_END { NULL, NULL }
#define OH_INIT_INHERIT_AS_INIT(ohia_optr, ohia_nptr) \
	{                                                 \
		/* .ohia_optr = */ (Dee_funptr_t)(ohia_optr), \
		/* .ohia_nptr = */ (Dee_funptr_t)(ohia_nptr)  \
	}

struct oh_init_spec_mhint {
	enum Dee_tmh_id ohismh_id;         /* ID of the method hint usable to implement this operator. */
	DeeTypeObject  *ohismh_implements; /* [0..1] Type that must be implemented for this hint to be usable. */
	unsigned int    ohismh_seqclass;   /* [valid_if(!miso_implements)] Required sequence class for this hint. (e.g. "Dee_SEQCLASS_SEQ") */
};

/* Check if "type" can use "self" */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
oh_init_spec_mhint_canuse(struct oh_init_spec_mhint const *__restrict self,
                          DeeTypeObject const *__restrict type) {
	if (self->ohismh_implements) {
		if (!DeeType_Implements(type, self->ohismh_implements))
			goto nope;
		if (type == self->ohismh_implements)
			goto nope; /* Don't inherit from the abstract origin */
	} else if (self->ohismh_seqclass != Dee_SEQCLASS_UNKNOWN) {
		if (DeeType_GetSeqClass(type) != self->ohismh_seqclass)
			goto nope;
		if (DeeType_IsSeqClassBase(type, self->ohismh_seqclass))
			goto nope; /* Don't inherit from the abstract origin */
	}
	return true;
nope:
	return false;
}

/* Same as `oh_init_spec_mhint_canuse', but returns
 * true even when "type" is the hint's abstract origin. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
oh_init_spec_mhint_caninherit(struct oh_init_spec_mhint const *__restrict self,
                              DeeTypeObject const *__restrict type) {
	if (self->ohismh_implements) {
		if (!DeeType_Implements(type, self->ohismh_implements))
			goto nope;
	} else if (self->ohismh_seqclass != Dee_SEQCLASS_UNKNOWN) {
		if (DeeType_GetSeqClass(type) != self->ohismh_seqclass)
			goto nope;
	}
	return true;
nope:
	return false;
}

#define OH_INIT_SPEC_MHINT_END { Dee_TMH_COUNT, NULL, 0 }
#define OH_INIT_SPEC_MHINT_INIT(ohismh_id, ohismh_implements, ohismh_seqclass) \
	{                                                                          \
		/* .ohismh_id         = */ ohismh_id,                                  \
		/* .ohismh_implements = */ ohismh_implements,                          \
		/* .ohismh_seqclass   = */ ohismh_seqclass                             \
	}

struct oh_init_spec {
	__UINTPTR_HALF_TYPE__            ohis_table;   /* Offset into `DeeTypeObject' for the sub-table containing this operator, or `0' when part of the primary table. */
	__UINTPTR_HALF_TYPE__            ohis_field;   /* Offset into the sub-table (or root) where the operator's function pointer is located. */
	struct oh_init_spec_class const *ohis_class;   /* [0..n] Class(usertyp) impls of this operator (terminated by `ohisc_usertyp == NULL') */
	struct oh_init_spec_impl  const *ohis_impls;   /* [0..n] Default impls of this operator (terminated by `ohisi_impl == NULL') */
	struct oh_init_spec_mhint const *ohis_mhints;  /* [0..n] Method hints that can be loaded into this operator (terminated by `ohismh_id >= Dee_TMH_COUNT') */
	struct oh_init_inherit_as const *ohis_inherit; /* [0..n] Rules for transforming special operator impls during inherit (terminated by `ohia_optr == NULL') */
};
#define OH_INIT_SPEC_INIT(ohis_table, ohis_field, ohis_class,    \
                          ohis_impls, ohis_mhints, ohis_inherit) \
	{                                                            \
		/* .ohis_table   = */ ohis_table,                        \
		/* .ohis_field   = */ ohis_field,                        \
		/* .ohis_class   = */ ohis_class,                        \
		/* .ohis_impls   = */ ohis_impls,                        \
		/* .ohis_mhints  = */ ohis_mhints,                       \
		/* .ohis_inherit = */ ohis_inherit                       \
	}


INTDEF struct oh_init_spec tpconst oh_init_specs[Dee_TNO_COUNT];

/* clang-format off */
/*[[[deemon (printNativeOperatorHintSpecs from "..method-hints.method-hints")();]]]*/
PRIVATE struct oh_init_spec_class tpconst oh_class_assign[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__assign__with__ASSIGN, OPERATOR_ASSIGN, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_assign[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_assign, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_assign, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_assign, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_move_assign[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__move_assign__with__MOVEASSIGN, OPERATOR_MOVEASSIGN, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_move_assign[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__move_assign__with__assign, Dee_TNO_assign, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_str[3] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__str__with__STR, OPERATOR_STR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_INIT(&usrtype__str__with__PRINT, Dee_CLASS_OPERATOR_PRINT, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_str[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__str__with__print, Dee_TNO_print, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_print[3] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__print__with__STR, OPERATOR_STR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_INIT(&usrtype__print__with__PRINT, Dee_CLASS_OPERATOR_PRINT, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_print[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__print__with__str, Dee_TNO_str, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_repr[3] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__repr__with__REPR, OPERATOR_REPR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_INIT(&usrtype__repr__with__PRINTREPR, Dee_CLASS_OPERATOR_PRINTREPR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_repr[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__repr__with__printrepr, Dee_TNO_printrepr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_printrepr[3] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__printrepr__with__REPR, OPERATOR_REPR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_INIT(&usrtype__printrepr__with__PRINTREPR, Dee_CLASS_OPERATOR_PRINTREPR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_printrepr[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__printrepr__with__repr, Dee_TNO_repr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_bool[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__bool__with__BOOL, OPERATOR_BOOL, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_bool[5] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_bool, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_bool, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_bool, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_bool, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_call[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__call__with__CALL, OPERATOR_CALL, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_call[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__call__with__call_kw, Dee_TNO_call_kw, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_call_kw[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__call_kw__with__CALL, OPERATOR_CALL, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_call_kw[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__call_kw__with__call, Dee_TNO_call, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_thiscall[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__thiscall__with__call, Dee_TNO_call, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_thiscall_kw[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__thiscall_kw__with__call_kw, Dee_TNO_call_kw, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__thiscall_kw__with__thiscall, Dee_TNO_thiscall, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
PRIVATE struct oh_init_spec_impl tpconst oh_impls_call_tuple[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__call_tuple__with__call, Dee_TNO_call, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_call_tuple_kw[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__call_tuple_kw__with__call_kw, Dee_TNO_call_kw, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_thiscall_tuple[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__thiscall_tuple__with__thiscall, Dee_TNO_thiscall, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_thiscall_tuple_kw[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__thiscall_tuple_kw__with__thiscall_kw, Dee_TNO_thiscall_kw, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
PRIVATE struct oh_init_spec_class tpconst oh_class_iter_next[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__iter_next__with__ITERNEXT, OPERATOR_ITERNEXT, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_iter_next[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__iter_next__with__nextpair, Dee_TNO_nextpair, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_nextpair[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__nextpair__with__iter_next, Dee_TNO_iter_next, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_nextkey[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__nextkey__with__iter_next, Dee_TNO_iter_next, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__nextkey__with__nextpair, Dee_TNO_nextpair, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_nextvalue[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__nextvalue__with__iter_next, Dee_TNO_iter_next, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__nextvalue__with__nextpair, Dee_TNO_nextpair, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_advance[5] = {
	OH_INIT_SPEC_IMPL_INIT(&default__advance__with__nextkey, Dee_TNO_nextkey, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__advance__with__nextvalue, Dee_TNO_nextvalue, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__advance__with__nextpair, Dee_TNO_nextpair, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__advance__with__iter_next, Dee_TNO_iter_next, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_int[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__int__with__INT, OPERATOR_INT, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_int[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__int__with__int64, Dee_TNO_int64, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__int__with__int32, Dee_TNO_int32, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__int__with__double, Dee_TNO_double, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_int32[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__int32__with__int64, Dee_TNO_int64, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__int32__with__int, Dee_TNO_int, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__int32__with__double, Dee_TNO_double, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_int64[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__int64__with__int32, Dee_TNO_int32, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__int64__with__int, Dee_TNO_int, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__int64__with__double, Dee_TNO_double, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_double[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__double__with__FLOAT, OPERATOR_FLOAT, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_double[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__double__with__int, Dee_TNO_int, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__double__with__int64, Dee_TNO_int64, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__double__with__int32, Dee_TNO_int32, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_hash[3] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__hash__with__HASH, OPERATOR_HASH, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_INIT(&usrtype__hash__with__, OPERATOR_USERCOUNT, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_hash[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_hash, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_hash, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_inherit_as tpconst oh_inherit_hash[2] = {
	OH_INIT_INHERIT_AS_INIT(&default__seq_operator_hash__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_hash__with__seq_operator_size__and__seq_operator_trygetitem_index),
	OH_INIT_INHERIT_AS_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_compare_eq[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__compare_eq__with__, OPERATOR_USERCOUNT, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_compare_eq[6] = {
	OH_INIT_SPEC_IMPL_INIT(&default__compare_eq__with__compare, Dee_TNO_compare, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__compare_eq__with__eq, Dee_TNO_eq, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__compare_eq__with__ne, Dee_TNO_ne, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__compare_eq__with__lo__and__gr, Dee_TNO_lo, Dee_TNO_gr),
	OH_INIT_SPEC_IMPL_INIT(&default__compare_eq__with__le__and__ge, Dee_TNO_le, Dee_TNO_ge),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_compare_eq[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_compare_eq, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_compare_eq, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_compare_eq, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_inherit_as tpconst oh_inherit_compare_eq[2] = {
	OH_INIT_INHERIT_AS_INIT(&default__seq_operator_compare_eq__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_compare_eq__with__seq_operator_size__and__seq_operator_trygetitem_index),
	OH_INIT_INHERIT_AS_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_compare[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__compare__with__, OPERATOR_USERCOUNT, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_compare[11] = {
	OH_INIT_SPEC_IMPL_INIT(&default__compare__with__eq__and__lo, Dee_TNO_eq, Dee_TNO_lo),
	OH_INIT_SPEC_IMPL_INIT(&default__compare__with__eq__and__le, Dee_TNO_eq, Dee_TNO_le),
	OH_INIT_SPEC_IMPL_INIT(&default__compare__with__eq__and__gr, Dee_TNO_eq, Dee_TNO_gr),
	OH_INIT_SPEC_IMPL_INIT(&default__compare__with__eq__and__ge, Dee_TNO_eq, Dee_TNO_ge),
	OH_INIT_SPEC_IMPL_INIT(&default__compare__with__ne__and__lo, Dee_TNO_ne, Dee_TNO_lo),
	OH_INIT_SPEC_IMPL_INIT(&default__compare__with__ne__and__le, Dee_TNO_ne, Dee_TNO_le),
	OH_INIT_SPEC_IMPL_INIT(&default__compare__with__ne__and__gr, Dee_TNO_ne, Dee_TNO_gr),
	OH_INIT_SPEC_IMPL_INIT(&default__compare__with__ne__and__ge, Dee_TNO_ne, Dee_TNO_ge),
	OH_INIT_SPEC_IMPL_INIT(&default__compare__with__lo__and__gr, Dee_TNO_lo, Dee_TNO_gr),
	OH_INIT_SPEC_IMPL_INIT(&default__compare__with__le__and__ge, Dee_TNO_le, Dee_TNO_ge),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_compare[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_compare, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_inherit_as tpconst oh_inherit_compare[2] = {
	OH_INIT_INHERIT_AS_INIT(&default__seq_operator_compare__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_compare__with__seq_operator_size__and__seq_operator_trygetitem_index),
	OH_INIT_INHERIT_AS_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_trycompare_eq[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__trycompare_eq__with__, OPERATOR_USERCOUNT, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_trycompare_eq[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__trycompare_eq__with__compare_eq, Dee_TNO_compare_eq, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_trycompare_eq[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_trycompare_eq, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_trycompare_eq, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_trycompare_eq, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_eq[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__eq__with__EQ, OPERATOR_EQ, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_eq[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__eq__with__ne, Dee_TNO_ne, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__eq__with__compare_eq, Dee_TNO_compare_eq, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_eq[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_eq, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_eq, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_eq, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_ne[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__ne__with__NE, OPERATOR_NE, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_ne[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__ne__with__eq, Dee_TNO_eq, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__ne__with__compare_eq, Dee_TNO_compare_eq, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_ne[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_ne, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_ne, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_ne, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_lo[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__lo__with__LO, OPERATOR_LO, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_lo[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__lo__with__ge, Dee_TNO_ge, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__lo__with__compare, Dee_TNO_compare, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_lo[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_lo, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_lo, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_lo, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_le[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__le__with__LE, OPERATOR_LE, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_le[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__le__with__gr, Dee_TNO_gr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__le__with__compare, Dee_TNO_compare, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_le[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_le, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_le, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_le, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_gr[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__gr__with__GR, OPERATOR_GR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_gr[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__gr__with__le, Dee_TNO_le, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__gr__with__compare, Dee_TNO_compare, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_gr[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_gr, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_gr, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_gr, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_ge[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__ge__with__GE, OPERATOR_GE, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_ge[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__ge__with__lo, Dee_TNO_lo, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__ge__with__compare, Dee_TNO_compare, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_ge[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_ge, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_ge, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_ge, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_iter[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__iter__with__ITER, OPERATOR_ITER, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_iter[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__iter__with__foreach, Dee_TNO_foreach, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__iter__with__foreach_pair, Dee_TNO_foreach_pair, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_iter[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_iter, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_iter, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_iter, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_inherit_as tpconst oh_inherit_iter[2] = {
	OH_INIT_INHERIT_AS_INIT(&default__seq_operator_iter__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_iter__with__seq_operator_size__and__seq_operator_trygetitem_index),
	OH_INIT_INHERIT_AS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_foreach[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__foreach__with__iter, Dee_TNO_iter, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__foreach__with__foreach_pair, Dee_TNO_foreach_pair, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_foreach[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_foreach, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_foreach, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_inherit_as tpconst oh_inherit_foreach[2] = {
	OH_INIT_INHERIT_AS_INIT(&default__seq_operator_foreach__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_foreach__with__seq_operator_size__and__seq_operator_trygetitem_index),
	OH_INIT_INHERIT_AS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_foreach_pair[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__foreach_pair__with__foreach, Dee_TNO_foreach, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__foreach_pair__with__iter, Dee_TNO_iter, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_foreach_pair[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_foreach_pair, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_foreach_pair, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_enumerate, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_sizeob[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__sizeob__with__SIZE, OPERATOR_SIZE, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_sizeob[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__sizeob__with__size, Dee_TNO_size, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_sizeob[6] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_sizeob, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_sizeob, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_sizeob, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_sizeob, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_sizeob, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_size[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__size__with__sizeob, Dee_TNO_sizeob, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_size[6] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_size, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_size, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_size, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_size, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_size, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_size_fast[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__size_fast__with__, Dee_TNO_COUNT, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_contains[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__contains__with__CONTAINS, OPERATOR_CONTAINS, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_contains[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_contains, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_contains, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_getitem[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__getitem__with__GETITEM, OPERATOR_GETITEM, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_getitem[8] = {
	OH_INIT_SPEC_IMPL_INIT(&default__getitem__with__getitem_index__and__getitem_string_len_hash, Dee_TNO_getitem_index, Dee_TNO_getitem_string_len_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem__with__getitem_index__and__getitem_string_hash, Dee_TNO_getitem_index, Dee_TNO_getitem_string_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem__with__getitem_index, Dee_TNO_getitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem__with__getitem_string_len_hash, Dee_TNO_getitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem__with__getitem_string_hash, Dee_TNO_getitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem__with__trygetitem__and__hasitem, Dee_TNO_trygetitem, Dee_TNO_hasitem),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem__with__trygetitem, Dee_TNO_trygetitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_getitem[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_getitem, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_getitem, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_getitem_index[5] = {
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_index__with__size__and__getitem_index_fast, Dee_TNO_size, Dee_TNO_getitem_index_fast),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_index__with__trygetitem_index__and__hasitem_index, Dee_TNO_trygetitem_index, Dee_TNO_hasitem_index),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_index__with__getitem, Dee_TNO_getitem, Dee_TNO_getitem),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_index__with__trygetitem_index, Dee_TNO_trygetitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_getitem_index[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_getitem_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_getitem_index, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_getitem_string_hash[5] = {
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_string_hash__with__getitem_string_len_hash, Dee_TNO_getitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash, Dee_TNO_trygetitem_string_hash, Dee_TNO_hasitem_string_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_string_hash__with__getitem, Dee_TNO_getitem, Dee_TNO_getitem),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_string_hash__with__trygetitem_string_hash, Dee_TNO_trygetitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_getitem_string_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_getitem_string_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_getitem_string_len_hash[5] = {
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_string_len_hash__with__getitem_string_hash, Dee_TNO_getitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash, Dee_TNO_trygetitem_string_len_hash, Dee_TNO_hasitem_string_len_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_string_len_hash__with__getitem, Dee_TNO_getitem, Dee_TNO_getitem),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_string_len_hash__with__trygetitem_string_len_hash, Dee_TNO_trygetitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_getitem_string_len_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_getitem_string_len_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_trygetitem[7] = {
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash, Dee_TNO_trygetitem_index, Dee_TNO_trygetitem_string_len_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash, Dee_TNO_trygetitem_index, Dee_TNO_trygetitem_string_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem__with__trygetitem_index, Dee_TNO_trygetitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem__with__trygetitem_string_len_hash, Dee_TNO_trygetitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem__with__trygetitem_string_hash, Dee_TNO_trygetitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem__with__getitem, Dee_TNO_getitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_trygetitem[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_trygetitem, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_trygetitem, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_trygetitem_index[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_index__with__size__and__getitem_index_fast, Dee_TNO_size, Dee_TNO_getitem_index_fast),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_index__with__getitem_index, Dee_TNO_getitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_index__with__trygetitem, Dee_TNO_trygetitem, Dee_TNO_trygetitem),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_trygetitem_index[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_trygetitem_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_trygetitem_index, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_trygetitem_string_hash[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_string_hash__with__trygetitem_string_len_hash, Dee_TNO_trygetitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_string_hash__with__getitem_string_hash, Dee_TNO_getitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_string_hash__with__trygetitem, Dee_TNO_trygetitem, Dee_TNO_trygetitem),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_trygetitem_string_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_trygetitem_string_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_trygetitem_string_len_hash[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_string_len_hash__with__trygetitem_string_hash, Dee_TNO_trygetitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_string_len_hash__with__getitem_string_len_hash, Dee_TNO_getitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_string_len_hash__with__trygetitem, Dee_TNO_trygetitem, Dee_TNO_trygetitem),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_trygetitem_string_len_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_trygetitem_string_len_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_bounditem[10] = {
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem__with__size__and__getitem_index_fast, Dee_TNO_size, Dee_TNO_getitem_index_fast),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem__with__bounditem_index__and__bounditem_string_len_hash, Dee_TNO_bounditem_index, Dee_TNO_bounditem_string_len_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem__with__bounditem_index__and__bounditem_string_hash, Dee_TNO_bounditem_index, Dee_TNO_bounditem_string_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem__with__getitem, Dee_TNO_getitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem__with__trygetitem__and__hasitem, Dee_TNO_trygetitem, Dee_TNO_hasitem),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem__with__bounditem_index, Dee_TNO_bounditem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem__with__bounditem_string_len_hash, Dee_TNO_bounditem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem__with__bounditem_string_hash, Dee_TNO_bounditem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem__with__trygetitem, Dee_TNO_trygetitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_bounditem[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_bounditem, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_bounditem, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_bounditem_index[6] = {
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_index__with__size__and__getitem_index_fast, Dee_TNO_size, Dee_TNO_getitem_index_fast),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_index__with__getitem_index, Dee_TNO_getitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_index__with__trygetitem_index__and__hasitem_index, Dee_TNO_trygetitem_index, Dee_TNO_hasitem_index),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_index__with__bounditem, Dee_TNO_bounditem, Dee_TNO_bounditem),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_index__with__trygetitem_index, Dee_TNO_trygetitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_bounditem_index[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_bounditem_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_bounditem_index, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_bounditem_string_hash[6] = {
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_hash__with__getitem_string_hash, Dee_TNO_getitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_hash__with__bounditem_string_len_hash, Dee_TNO_bounditem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash, Dee_TNO_trygetitem_string_hash, Dee_TNO_hasitem_string_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_hash__with__bounditem, Dee_TNO_bounditem, Dee_TNO_bounditem),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_hash__with__trygetitem_string_hash, Dee_TNO_trygetitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_bounditem_string_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_bounditem_string_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_bounditem_string_len_hash[6] = {
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_len_hash__with__getitem_string_len_hash, Dee_TNO_getitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash, Dee_TNO_trygetitem_string_len_hash, Dee_TNO_hasitem_string_len_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_len_hash__with__bounditem_string_hash, Dee_TNO_bounditem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_len_hash__with__bounditem, Dee_TNO_bounditem, Dee_TNO_bounditem),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_len_hash__with__trygetitem_string_len_hash, Dee_TNO_trygetitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_bounditem_string_len_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_bounditem_string_len_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_hasitem[8] = {
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__bounditem, Dee_TNO_bounditem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__hasitem_index__and__hasitem_string_len_hash, Dee_TNO_hasitem_index, Dee_TNO_hasitem_string_len_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__hasitem_index__and__hasitem_string_hash, Dee_TNO_hasitem_index, Dee_TNO_hasitem_string_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__hasitem_index, Dee_TNO_hasitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__hasitem_string_len_hash, Dee_TNO_hasitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__hasitem_string_hash, Dee_TNO_hasitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__size__and__getitem_index_fast, Dee_TNO_size, Dee_TNO_getitem_index_fast),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_hasitem[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_hasitem, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_hasitem, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_hasitem_index[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_index__with__bounditem_index, Dee_TNO_bounditem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_index__with__size__and__getitem_index_fast, Dee_TNO_size, Dee_TNO_getitem_index_fast),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_index__with__hasitem, Dee_TNO_hasitem, Dee_TNO_hasitem),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_hasitem_index[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_hasitem_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_hasitem_index, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_hasitem_string_hash[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_string_hash__with__bounditem_string_hash, Dee_TNO_bounditem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_string_hash__with__hasitem_string_len_hash, Dee_TNO_hasitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_string_hash__with__hasitem, Dee_TNO_hasitem, Dee_TNO_hasitem),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_hasitem_string_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_hasitem_string_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_hasitem_string_len_hash[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_string_len_hash__with__bounditem_string_len_hash, Dee_TNO_bounditem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_string_len_hash__with__hasitem_string_hash, Dee_TNO_hasitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_string_len_hash__with__hasitem, Dee_TNO_hasitem, Dee_TNO_hasitem),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_hasitem_string_len_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_hasitem_string_len_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_delitem[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__delitem__with__DELITEM, OPERATOR_DELITEM, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_delitem[6] = {
	OH_INIT_SPEC_IMPL_INIT(&default__delitem__with__delitem_index__and__delitem_string_len_hash, Dee_TNO_delitem_index, Dee_TNO_delitem_string_len_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__delitem__with__delitem_index__and__delitem_string_hash, Dee_TNO_delitem_index, Dee_TNO_delitem_string_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__delitem__with__delitem_index, Dee_TNO_delitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__delitem__with__delitem_string_len_hash, Dee_TNO_delitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__delitem__with__delitem_string_hash, Dee_TNO_delitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_delitem[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_delitem, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_delitem, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_delitem_index[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__delitem_index__with__delitem, Dee_TNO_delitem, Dee_TNO_delitem),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_delitem_index[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_delitem_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_delitem_index, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_delitem_string_hash[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__delitem_string_hash__with__delitem_string_len_hash, Dee_TNO_delitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__delitem_string_hash__with__delitem, Dee_TNO_delitem, Dee_TNO_delitem),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_delitem_string_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_delitem_string_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_delitem_string_len_hash[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__delitem_string_len_hash__with__delitem, Dee_TNO_delitem, Dee_TNO_delitem),
	OH_INIT_SPEC_IMPL_INIT(&default__delitem_string_len_hash__with__delitem_string_hash, Dee_TNO_delitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_delitem_string_len_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_delitem_string_len_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_setitem[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__setitem__with__SETITEM, OPERATOR_SETITEM, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_setitem[6] = {
	OH_INIT_SPEC_IMPL_INIT(&default__setitem__with__setitem_index__and__setitem_string_len_hash, Dee_TNO_setitem_index, Dee_TNO_setitem_string_len_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__setitem__with__setitem_index__and__setitem_string_hash, Dee_TNO_setitem_index, Dee_TNO_setitem_string_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__setitem__with__setitem_index, Dee_TNO_setitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__setitem__with__setitem_string_len_hash, Dee_TNO_setitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__setitem__with__setitem_string_hash, Dee_TNO_setitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_setitem[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_setitem, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_setitem, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_setitem_index[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__setitem_index__with__setitem, Dee_TNO_setitem, Dee_TNO_setitem),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_setitem_index[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_setitem_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_setitem_index, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_setitem_string_hash[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__setitem_string_hash__with__setitem_string_len_hash, Dee_TNO_setitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__setitem_string_hash__with__setitem, Dee_TNO_setitem, Dee_TNO_setitem),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_setitem_string_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_setitem_string_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_setitem_string_len_hash[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__setitem_string_len_hash__with__setitem, Dee_TNO_setitem, Dee_TNO_setitem),
	OH_INIT_SPEC_IMPL_INIT(&default__setitem_string_len_hash__with__setitem_string_hash, Dee_TNO_setitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_setitem_string_len_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_setitem_string_len_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_getrange[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__getrange__with__GETRANGE, OPERATOR_GETRANGE, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_getrange[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__getrange__with__getrange_index__and__getrange_index_n, Dee_TNO_getrange_index, Dee_TNO_getrange_index_n),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_getrange[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_getrange, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_getrange_index[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__getrange_index__with__getrange, Dee_TNO_getrange, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_getrange_index[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_getrange_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_inherit_as tpconst oh_inherit_getrange_index[2] = {
	OH_INIT_INHERIT_AS_INIT(&default__seq_operator_getrange_index__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_trygetitem_index),
	OH_INIT_INHERIT_AS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_getrange_index_n[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__getrange_index_n__with__getrange, Dee_TNO_getrange, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_getrange_index_n[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_getrange_index_n, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_inherit_as tpconst oh_inherit_getrange_index_n[2] = {
	OH_INIT_INHERIT_AS_INIT(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__operator_getitem_index_fast, &default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_trygetitem_index),
	OH_INIT_INHERIT_AS_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_delrange[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__delrange__with__DELRANGE, OPERATOR_DELRANGE, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_delrange[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__delrange__with__delrange_index__and__delrange_index_n, Dee_TNO_delrange_index, Dee_TNO_delrange_index_n),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_delrange[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_delrange, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_delrange_index[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__delrange_index__with__delrange, Dee_TNO_delrange, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_delrange_index[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_delrange_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_delrange_index_n[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__delrange_index_n__with__delrange, Dee_TNO_delrange, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_delrange_index_n[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_delrange_index_n, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_setrange[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__setrange__with__SETRANGE, OPERATOR_SETRANGE, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_setrange[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__setrange__with__setrange_index__and__setrange_index_n, Dee_TNO_setrange_index, Dee_TNO_setrange_index_n),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_setrange[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_setrange, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_setrange_index[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__setrange_index__with__setrange, Dee_TNO_setrange, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_setrange_index[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_setrange_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_setrange_index_n[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__setrange_index_n__with__setrange, Dee_TNO_setrange, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_setrange_index_n[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_setrange_index_n, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_inv[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__inv__with__INV, OPERATOR_INV, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_inv[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_inv, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_pos[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__pos__with__POS, OPERATOR_POS, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_neg[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__neg__with__NEG, OPERATOR_NEG, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_add[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__add__with__ADD, OPERATOR_ADD, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_add[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_add, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_add, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_add, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_inplace_add[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__inplace_add__with__INPLACE_ADD, OPERATOR_INPLACE_ADD, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_add[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_add__with__add, Dee_TNO_add, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_inplace_add[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_inplace_add, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_inplace_add, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_inplace_add, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_sub[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__sub__with__SUB, OPERATOR_SUB, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_sub[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_sub, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_sub, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_inplace_sub[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__inplace_sub__with__INPLACE_SUB, OPERATOR_INPLACE_SUB, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_sub[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_sub__with__sub, Dee_TNO_sub, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_inplace_sub[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_inplace_sub, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_inplace_sub, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_mul[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__mul__with__MUL, OPERATOR_MUL, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_mul[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_mul, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_inplace_mul[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__inplace_mul__with__INPLACE_MUL, OPERATOR_INPLACE_MUL, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_mul[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_mul__with__mul, Dee_TNO_mul, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_inplace_mul[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_inplace_mul, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_div[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__div__with__DIV, OPERATOR_DIV, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_inplace_div[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__inplace_div__with__INPLACE_DIV, OPERATOR_INPLACE_DIV, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_div[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_div__with__div, Dee_TNO_div, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_mod[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__mod__with__MOD, OPERATOR_MOD, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_inplace_mod[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__inplace_mod__with__INPLACE_MOD, OPERATOR_INPLACE_MOD, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_mod[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_mod__with__mod, Dee_TNO_mod, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_shl[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__shl__with__SHL, OPERATOR_SHL, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_inplace_shl[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__inplace_shl__with__INPLACE_SHL, OPERATOR_INPLACE_SHL, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_shl[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_shl__with__shl, Dee_TNO_shl, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_shr[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__shr__with__SHR, OPERATOR_SHR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_inplace_shr[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__inplace_shr__with__INPLACE_SHR, OPERATOR_INPLACE_SHR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_shr[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_shr__with__shr, Dee_TNO_shr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_and[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__and__with__AND, OPERATOR_AND, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_and[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_and, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_and, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_inplace_and[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__inplace_and__with__INPLACE_AND, OPERATOR_INPLACE_AND, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_and[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_and__with__and, Dee_TNO_and, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_inplace_and[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_inplace_and, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_inplace_and, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_or[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__or__with__OR, OPERATOR_OR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_or[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_add, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_add, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_inplace_or[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__inplace_or__with__INPLACE_OR, OPERATOR_INPLACE_OR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_or[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_or__with__or, Dee_TNO_or, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_inplace_or[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_inplace_add, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_inplace_add, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_xor[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__xor__with__XOR, OPERATOR_XOR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_xor[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_xor, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_xor, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_inplace_xor[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__inplace_xor__with__INPLACE_XOR, OPERATOR_INPLACE_XOR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_xor[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_xor__with__xor, Dee_TNO_xor, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_inplace_xor[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_inplace_xor, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_inplace_xor, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_pow[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__pow__with__POW, OPERATOR_POW, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_inplace_pow[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__inplace_pow__with__INPLACE_POW, OPERATOR_INPLACE_POW, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_pow[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_pow__with__pow, Dee_TNO_pow, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_inc[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__inc__with__INC, OPERATOR_INC, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inc[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inc__with__inplace_add, Dee_TNO_inplace_add, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__inc__with__add, Dee_TNO_add, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_dec[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__dec__with__DEC, OPERATOR_DEC, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_dec[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__dec__with__inplace_sub, Dee_TNO_inplace_sub, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__dec__with__sub, Dee_TNO_sub, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_enter[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__enter__with__ENTER, OPERATOR_ENTER, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_enter[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__enter__with__leave, Dee_TNO_leave, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_leave[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__leave__with__LEAVE, OPERATOR_LEAVE, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_leave[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__leave__with__enter, Dee_TNO_enter, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_getattr[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__getattr__with__GETATTR, OPERATOR_GETATTR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_getattr_string_hash[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__getattr_string_hash__with__getattr, Dee_TNO_getattr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_getattr_string_len_hash[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__getattr_string_len_hash__with__getattr, Dee_TNO_getattr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_boundattr[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__boundattr__with__getattr, Dee_TNO_getattr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_boundattr_string_hash[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__boundattr_string_hash__with__getattr_string_hash, Dee_TNO_getattr_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__boundattr_string_hash__with__boundattr, Dee_TNO_boundattr, Dee_TNO_boundattr),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_boundattr_string_len_hash[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__boundattr_string_len_hash__with__getattr_string_len_hash, Dee_TNO_getattr_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__boundattr_string_len_hash__with__boundattr, Dee_TNO_boundattr, Dee_TNO_boundattr),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_hasattr[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__hasattr__with__boundattr, Dee_TNO_boundattr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_hasattr_string_hash[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__hasattr_string_hash__with__boundattr_string_hash, Dee_TNO_boundattr_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_hasattr_string_len_hash[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__hasattr_string_len_hash__with__boundattr_string_len_hash, Dee_TNO_boundattr_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_delattr[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__delattr__with__DELATTR, OPERATOR_DELATTR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_delattr_string_hash[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__delattr_string_hash__with__delattr, Dee_TNO_delattr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_delattr_string_len_hash[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__delattr_string_len_hash__with__delattr, Dee_TNO_delattr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_class tpconst oh_class_setattr[2] = {
	OH_INIT_SPEC_CLASS_INIT(&usrtype__setattr__with__SETATTR, OPERATOR_SETATTR, OPERATOR_USERCOUNT),
	OH_INIT_SPEC_CLASS_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_setattr_string_hash[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__setattr_string_hash__with__setattr, Dee_TNO_setattr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_setattr_string_len_hash[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__setattr_string_len_hash__with__setattr, Dee_TNO_setattr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
INTERN_TPCONST struct oh_init_spec tpconst oh_init_specs[] = {
	/* tp_init.tp_assign                     */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_init.tp_assign), oh_class_assign, NULL, oh_mhints_assign, NULL),
	/* tp_init.tp_move_assign                */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_init.tp_move_assign), oh_class_move_assign, oh_impls_move_assign, NULL, NULL),
	/* tp_cast.tp_str                        */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_str), oh_class_str, oh_impls_str, NULL, NULL),
	/* tp_cast.tp_print                      */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_print), oh_class_print, oh_impls_print, NULL, NULL),
	/* tp_cast.tp_repr                       */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_repr), oh_class_repr, oh_impls_repr, NULL, NULL),
	/* tp_cast.tp_printrepr                  */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_printrepr), oh_class_printrepr, oh_impls_printrepr, NULL, NULL),
	/* tp_cast.tp_bool                       */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_bool), oh_class_bool, NULL, oh_mhints_bool, NULL),
	/* tp_call                               */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_call), oh_class_call, oh_impls_call, NULL, NULL),
	/* tp_callable->tp_call_kw               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_callable), offsetof(struct type_callable, tp_call_kw), oh_class_call_kw, oh_impls_call_kw, NULL, NULL),
	/* tp_callable->tp_thiscall              */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_callable), offsetof(struct type_callable, tp_thiscall), NULL, oh_impls_thiscall, NULL, NULL),
	/* tp_callable->tp_thiscall_kw           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_callable), offsetof(struct type_callable, tp_thiscall_kw), NULL, oh_impls_thiscall_kw, NULL, NULL),
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* tp_callable->tp_call_tuple            */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_callable), offsetof(struct type_callable, tp_call_tuple), NULL, oh_impls_call_tuple, NULL, NULL),
	/* tp_callable->tp_call_tuple_kw         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_callable), offsetof(struct type_callable, tp_call_tuple_kw), NULL, oh_impls_call_tuple_kw, NULL, NULL),
	/* tp_callable->tp_thiscall_tuple        */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_callable), offsetof(struct type_callable, tp_thiscall_tuple), NULL, oh_impls_thiscall_tuple, NULL, NULL),
	/* tp_callable->tp_thiscall_tuple_kw     */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_callable), offsetof(struct type_callable, tp_thiscall_tuple_kw), NULL, oh_impls_thiscall_tuple_kw, NULL, NULL),
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
	/* tp_iter_next                          */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_iter_next), oh_class_iter_next, oh_impls_iter_next, NULL, NULL),
	/* tp_iterator->tp_nextpair              */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_iterator), offsetof(struct type_iterator, tp_nextpair), NULL, oh_impls_nextpair, NULL, NULL),
	/* tp_iterator->tp_nextkey               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_iterator), offsetof(struct type_iterator, tp_nextkey), NULL, oh_impls_nextkey, NULL, NULL),
	/* tp_iterator->tp_nextvalue             */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_iterator), offsetof(struct type_iterator, tp_nextvalue), NULL, oh_impls_nextvalue, NULL, NULL),
	/* tp_iterator->tp_advance               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_iterator), offsetof(struct type_iterator, tp_advance), NULL, oh_impls_advance, NULL, NULL),
	/* tp_math->tp_int                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_int), oh_class_int, oh_impls_int, NULL, NULL),
	/* tp_math->tp_int32                     */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_int32), NULL, oh_impls_int32, NULL, NULL),
	/* tp_math->tp_int64                     */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_int64), NULL, oh_impls_int64, NULL, NULL),
	/* tp_math->tp_double                    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_double), oh_class_double, oh_impls_double, NULL, NULL),
	/* tp_cmp->tp_hash                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_hash), oh_class_hash, NULL, oh_mhints_hash, oh_inherit_hash),
	/* tp_cmp->tp_compare_eq                 */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_compare_eq), oh_class_compare_eq, oh_impls_compare_eq, oh_mhints_compare_eq, oh_inherit_compare_eq),
	/* tp_cmp->tp_compare                    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_compare), oh_class_compare, oh_impls_compare, oh_mhints_compare, oh_inherit_compare),
	/* tp_cmp->tp_trycompare_eq              */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_trycompare_eq), oh_class_trycompare_eq, oh_impls_trycompare_eq, oh_mhints_trycompare_eq, NULL),
	/* tp_cmp->tp_eq                         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_eq), oh_class_eq, oh_impls_eq, oh_mhints_eq, NULL),
	/* tp_cmp->tp_ne                         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_ne), oh_class_ne, oh_impls_ne, oh_mhints_ne, NULL),
	/* tp_cmp->tp_lo                         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_lo), oh_class_lo, oh_impls_lo, oh_mhints_lo, NULL),
	/* tp_cmp->tp_le                         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_le), oh_class_le, oh_impls_le, oh_mhints_le, NULL),
	/* tp_cmp->tp_gr                         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_gr), oh_class_gr, oh_impls_gr, oh_mhints_gr, NULL),
	/* tp_cmp->tp_ge                         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_ge), oh_class_ge, oh_impls_ge, oh_mhints_ge, NULL),
	/* tp_seq->tp_iter                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_iter), oh_class_iter, oh_impls_iter, oh_mhints_iter, oh_inherit_iter),
	/* tp_seq->tp_foreach                    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_foreach), NULL, oh_impls_foreach, oh_mhints_foreach, oh_inherit_foreach),
	/* tp_seq->tp_foreach_pair               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_foreach_pair), NULL, oh_impls_foreach_pair, oh_mhints_foreach_pair, NULL),
	/* tp_seq->tp_sizeob                     */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_sizeob), oh_class_sizeob, oh_impls_sizeob, oh_mhints_sizeob, NULL),
	/* tp_seq->tp_size                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_size), NULL, oh_impls_size, oh_mhints_size, NULL),
	/* tp_seq->tp_size_fast                  */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_size_fast), NULL, oh_impls_size_fast, NULL, NULL),
	/* tp_seq->tp_contains                   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_contains), oh_class_contains, NULL, oh_mhints_contains, NULL),
	/* tp_seq->tp_getitem_index_fast         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getitem_index_fast), NULL, NULL, NULL, NULL),
	/* tp_seq->tp_getitem                    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getitem), oh_class_getitem, oh_impls_getitem, oh_mhints_getitem, NULL),
	/* tp_seq->tp_getitem_index              */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getitem_index), NULL, oh_impls_getitem_index, oh_mhints_getitem_index, NULL),
	/* tp_seq->tp_getitem_string_hash        */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getitem_string_hash), NULL, oh_impls_getitem_string_hash, oh_mhints_getitem_string_hash, NULL),
	/* tp_seq->tp_getitem_string_len_hash    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getitem_string_len_hash), NULL, oh_impls_getitem_string_len_hash, oh_mhints_getitem_string_len_hash, NULL),
	/* tp_seq->tp_trygetitem                 */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_trygetitem), NULL, oh_impls_trygetitem, oh_mhints_trygetitem, NULL),
	/* tp_seq->tp_trygetitem_index           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_trygetitem_index), NULL, oh_impls_trygetitem_index, oh_mhints_trygetitem_index, NULL),
	/* tp_seq->tp_trygetitem_string_hash     */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_trygetitem_string_hash), NULL, oh_impls_trygetitem_string_hash, oh_mhints_trygetitem_string_hash, NULL),
	/* tp_seq->tp_trygetitem_string_len_hash */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_trygetitem_string_len_hash), NULL, oh_impls_trygetitem_string_len_hash, oh_mhints_trygetitem_string_len_hash, NULL),
	/* tp_seq->tp_bounditem                  */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_bounditem), NULL, oh_impls_bounditem, oh_mhints_bounditem, NULL),
	/* tp_seq->tp_bounditem_index            */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_bounditem_index), NULL, oh_impls_bounditem_index, oh_mhints_bounditem_index, NULL),
	/* tp_seq->tp_bounditem_string_hash      */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_bounditem_string_hash), NULL, oh_impls_bounditem_string_hash, oh_mhints_bounditem_string_hash, NULL),
	/* tp_seq->tp_bounditem_string_len_hash  */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_bounditem_string_len_hash), NULL, oh_impls_bounditem_string_len_hash, oh_mhints_bounditem_string_len_hash, NULL),
	/* tp_seq->tp_hasitem                    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_hasitem), NULL, oh_impls_hasitem, oh_mhints_hasitem, NULL),
	/* tp_seq->tp_hasitem_index              */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_hasitem_index), NULL, oh_impls_hasitem_index, oh_mhints_hasitem_index, NULL),
	/* tp_seq->tp_hasitem_string_hash        */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_hasitem_string_hash), NULL, oh_impls_hasitem_string_hash, oh_mhints_hasitem_string_hash, NULL),
	/* tp_seq->tp_hasitem_string_len_hash    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_hasitem_string_len_hash), NULL, oh_impls_hasitem_string_len_hash, oh_mhints_hasitem_string_len_hash, NULL),
	/* tp_seq->tp_delitem                    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_delitem), oh_class_delitem, oh_impls_delitem, oh_mhints_delitem, NULL),
	/* tp_seq->tp_delitem_index              */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_delitem_index), NULL, oh_impls_delitem_index, oh_mhints_delitem_index, NULL),
	/* tp_seq->tp_delitem_string_hash        */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_delitem_string_hash), NULL, oh_impls_delitem_string_hash, oh_mhints_delitem_string_hash, NULL),
	/* tp_seq->tp_delitem_string_len_hash    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_delitem_string_len_hash), NULL, oh_impls_delitem_string_len_hash, oh_mhints_delitem_string_len_hash, NULL),
	/* tp_seq->tp_setitem                    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_setitem), oh_class_setitem, oh_impls_setitem, oh_mhints_setitem, NULL),
	/* tp_seq->tp_setitem_index              */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_setitem_index), NULL, oh_impls_setitem_index, oh_mhints_setitem_index, NULL),
	/* tp_seq->tp_setitem_string_hash        */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_setitem_string_hash), NULL, oh_impls_setitem_string_hash, oh_mhints_setitem_string_hash, NULL),
	/* tp_seq->tp_setitem_string_len_hash    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_setitem_string_len_hash), NULL, oh_impls_setitem_string_len_hash, oh_mhints_setitem_string_len_hash, NULL),
	/* tp_seq->tp_getrange                   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getrange), oh_class_getrange, oh_impls_getrange, oh_mhints_getrange, NULL),
	/* tp_seq->tp_getrange_index             */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getrange_index), NULL, oh_impls_getrange_index, oh_mhints_getrange_index, oh_inherit_getrange_index),
	/* tp_seq->tp_getrange_index_n           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getrange_index_n), NULL, oh_impls_getrange_index_n, oh_mhints_getrange_index_n, oh_inherit_getrange_index_n),
	/* tp_seq->tp_delrange                   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_delrange), oh_class_delrange, oh_impls_delrange, oh_mhints_delrange, NULL),
	/* tp_seq->tp_delrange_index             */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_delrange_index), NULL, oh_impls_delrange_index, oh_mhints_delrange_index, NULL),
	/* tp_seq->tp_delrange_index_n           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_delrange_index_n), NULL, oh_impls_delrange_index_n, oh_mhints_delrange_index_n, NULL),
	/* tp_seq->tp_setrange                   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_setrange), oh_class_setrange, oh_impls_setrange, oh_mhints_setrange, NULL),
	/* tp_seq->tp_setrange_index             */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_setrange_index), NULL, oh_impls_setrange_index, oh_mhints_setrange_index, NULL),
	/* tp_seq->tp_setrange_index_n           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_setrange_index_n), NULL, oh_impls_setrange_index_n, oh_mhints_setrange_index_n, NULL),
	/* tp_math->tp_inv                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inv), oh_class_inv, NULL, oh_mhints_inv, NULL),
	/* tp_math->tp_pos                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_pos), oh_class_pos, NULL, NULL, NULL),
	/* tp_math->tp_neg                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_neg), oh_class_neg, NULL, NULL, NULL),
	/* tp_math->tp_add                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_add), oh_class_add, NULL, oh_mhints_add, NULL),
	/* tp_math->tp_inplace_add               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_add), oh_class_inplace_add, oh_impls_inplace_add, oh_mhints_inplace_add, NULL),
	/* tp_math->tp_sub                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_sub), oh_class_sub, NULL, oh_mhints_sub, NULL),
	/* tp_math->tp_inplace_sub               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_sub), oh_class_inplace_sub, oh_impls_inplace_sub, oh_mhints_inplace_sub, NULL),
	/* tp_math->tp_mul                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_mul), oh_class_mul, NULL, oh_mhints_mul, NULL),
	/* tp_math->tp_inplace_mul               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_mul), oh_class_inplace_mul, oh_impls_inplace_mul, oh_mhints_inplace_mul, NULL),
	/* tp_math->tp_div                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_div), oh_class_div, NULL, NULL, NULL),
	/* tp_math->tp_inplace_div               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_div), oh_class_inplace_div, oh_impls_inplace_div, NULL, NULL),
	/* tp_math->tp_mod                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_mod), oh_class_mod, NULL, NULL, NULL),
	/* tp_math->tp_inplace_mod               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_mod), oh_class_inplace_mod, oh_impls_inplace_mod, NULL, NULL),
	/* tp_math->tp_shl                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_shl), oh_class_shl, NULL, NULL, NULL),
	/* tp_math->tp_inplace_shl               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_shl), oh_class_inplace_shl, oh_impls_inplace_shl, NULL, NULL),
	/* tp_math->tp_shr                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_shr), oh_class_shr, NULL, NULL, NULL),
	/* tp_math->tp_inplace_shr               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_shr), oh_class_inplace_shr, oh_impls_inplace_shr, NULL, NULL),
	/* tp_math->tp_and                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_and), oh_class_and, NULL, oh_mhints_and, NULL),
	/* tp_math->tp_inplace_and               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_and), oh_class_inplace_and, oh_impls_inplace_and, oh_mhints_inplace_and, NULL),
	/* tp_math->tp_or                        */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_or), oh_class_or, NULL, oh_mhints_or, NULL),
	/* tp_math->tp_inplace_or                */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_or), oh_class_inplace_or, oh_impls_inplace_or, oh_mhints_inplace_or, NULL),
	/* tp_math->tp_xor                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_xor), oh_class_xor, NULL, oh_mhints_xor, NULL),
	/* tp_math->tp_inplace_xor               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_xor), oh_class_inplace_xor, oh_impls_inplace_xor, oh_mhints_inplace_xor, NULL),
	/* tp_math->tp_pow                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_pow), oh_class_pow, NULL, NULL, NULL),
	/* tp_math->tp_inplace_pow               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_pow), oh_class_inplace_pow, oh_impls_inplace_pow, NULL, NULL),
	/* tp_math->tp_inc                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inc), oh_class_inc, oh_impls_inc, NULL, NULL),
	/* tp_math->tp_dec                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_dec), oh_class_dec, oh_impls_dec, NULL, NULL),
	/* tp_with->tp_enter                     */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_with), offsetof(struct type_with, tp_enter), oh_class_enter, oh_impls_enter, NULL, NULL),
	/* tp_with->tp_leave                     */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_with), offsetof(struct type_with, tp_leave), oh_class_leave, oh_impls_leave, NULL, NULL),
	/* tp_attr->tp_getattr                   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_getattr), oh_class_getattr, NULL, NULL, NULL),
	/* tp_attr->tp_getattr_string_hash       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_getattr_string_hash), NULL, oh_impls_getattr_string_hash, NULL, NULL),
	/* tp_attr->tp_getattr_string_len_hash   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_getattr_string_len_hash), NULL, oh_impls_getattr_string_len_hash, NULL, NULL),
	/* tp_attr->tp_boundattr                 */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_boundattr), NULL, oh_impls_boundattr, NULL, NULL),
	/* tp_attr->tp_boundattr_string_hash     */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_boundattr_string_hash), NULL, oh_impls_boundattr_string_hash, NULL, NULL),
	/* tp_attr->tp_boundattr_string_len_hash */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_boundattr_string_len_hash), NULL, oh_impls_boundattr_string_len_hash, NULL, NULL),
	/* tp_attr->tp_hasattr                   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_hasattr), NULL, oh_impls_hasattr, NULL, NULL),
	/* tp_attr->tp_hasattr_string_hash       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_hasattr_string_hash), NULL, oh_impls_hasattr_string_hash, NULL, NULL),
	/* tp_attr->tp_hasattr_string_len_hash   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_hasattr_string_len_hash), NULL, oh_impls_hasattr_string_len_hash, NULL, NULL),
	/* tp_attr->tp_delattr                   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_delattr), oh_class_delattr, NULL, NULL, NULL),
	/* tp_attr->tp_delattr_string_hash       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_delattr_string_hash), NULL, oh_impls_delattr_string_hash, NULL, NULL),
	/* tp_attr->tp_delattr_string_len_hash   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_delattr_string_len_hash), NULL, oh_impls_delattr_string_len_hash, NULL, NULL),
	/* tp_attr->tp_setattr                   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_setattr), oh_class_setattr, NULL, NULL, NULL),
	/* tp_attr->tp_setattr_string_hash       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_setattr_string_hash), NULL, oh_impls_setattr_string_hash, NULL, NULL),
	/* tp_attr->tp_setattr_string_len_hash   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_attr), offsetof(struct type_attr, tp_setattr_string_len_hash), NULL, oh_impls_setattr_string_len_hash, NULL, NULL),
};
/*[[[end]]]*/
/* clang-format on */

PRIVATE WUNUSED NONNULL((1)) Dee_funptr_t DCALL
type_tno_get(DeeTypeObject const *__restrict self, enum Dee_tno_id id) {
	struct oh_init_spec const *specs = &oh_init_specs[id];
	byte_t const *table = (byte_t const *)self;
	if (specs->ohis_table != 0) {
		table += specs->ohis_table;
		table = atomic_read((byte_t **)table);
		if (!table)
			return NULL;
	}
	table += specs->ohis_field;
	return atomic_read((Dee_funptr_t *)table);
}

PRIVATE ATTR_CONST WUNUSED size_t DCALL
type_tno_sizeof_table(__UINTPTR_HALF_TYPE__ offsetof_table) {
	switch (offsetof_table) {
		/* clang-format off */
/*[[[deemon (printTypeTnoSizeofTableCases from "..method-hints.method-hints")();]]]*/
	case offsetof(DeeTypeObject, tp_callable):
		return sizeof(struct type_callable);
	case offsetof(DeeTypeObject, tp_iterator):
		return sizeof(struct type_iterator);
	case offsetof(DeeTypeObject, tp_math):
		return sizeof(struct type_math);
	case offsetof(DeeTypeObject, tp_cmp):
		return sizeof(struct type_cmp);
	case offsetof(DeeTypeObject, tp_seq):
		return sizeof(struct type_seq);
	case offsetof(DeeTypeObject, tp_with):
		return sizeof(struct type_with);
	case offsetof(DeeTypeObject, tp_attr):
		return sizeof(struct type_attr);
/*[[[end]]]*/
		/* clang-format on */
	default: break;
	}
#ifdef NDEBUG
	__builtin_unreachable();
#else /* NDEBUG */
	Dee_XFatalf("Bad operator table offset: %" PRFuSIZ,
	            (size_t)offsetof_table);
#endif /* !NDEBUG */
}

PRIVATE WUNUSED byte_t *DCALL
type_tno_tryalloc_table(__UINTPTR_HALF_TYPE__ offsetof_table) {
	size_t size = type_tno_sizeof_table(offsetof_table);
	return (byte_t *)Dee_TryCalloc(size);
}

/* @return false: OOM */
PRIVATE NONNULL((1)) bool DCALL
type_tno_tryset(DeeTypeObject const *__restrict self,
                enum Dee_tno_id id, Dee_funptr_t value) {
	struct oh_init_spec const *specs = &oh_init_specs[id];
	byte_t *table = (byte_t *)self;
	if (specs->ohis_table != 0) {
		table += specs->ohis_table;
		table = atomic_read((byte_t **)table);
		if (!table) {
			byte_t *new_table;
			new_table = type_tno_tryalloc_table(specs->ohis_table);
			if unlikely(!new_table)
				return false;
			table = (byte_t *)self;
			table += specs->ohis_table;
			if unlikely(!atomic_cmpxch((byte_t **)table, NULL, new_table)) {
				Dee_Free(new_table);
				table = atomic_read((byte_t **)table);
			} else {
				/* If the table was allocated for a static (non-heap) type,
				 * then untrack it from the memory leak detector, since we
				 * don't care that it (most definitely) won't be freed on
				 * program exit. */
				if (!DeeType_IsHeapType(self)) {
					new_table = (byte_t *)Dee_UntrackAlloc(new_table);
					/* TODO: This leaks memory when "self" is statically allocated within a DEX module.
					 *       When the module gets unloaded, "new_table" won't get free'd.
					 *
					 * Solution: Have an API "Dee_MallocStatic()" and "Dee_FreeStatic()" that take
					 *           an extra "void const **p_target_addr" argument and bind the
					 *           allocation to that module's lifetime. This only works for DEX
					 *           modules (since DEE modules cannot contain *truely* static objects),
					 *           and when the dex module is unloaded, all memory allocated using
					 *           this API will be automatically free'd, and `*p_target_addr' of
					 *           every call will be set to `NULL' (in order to restore what was
					 *           presumably the original allocation state).
					 *
					 * The same also goes for pretty much all other instances where "Dee_UntrackAlloc"
					 * is currently being used to suppress leak notifications with the argument of
					 * the associated heap memory being meant for a dex module.
					 */
				}
				table = new_table;
			}
			ASSERT(table);
		}
	}
	table += specs->ohis_field;
	atomic_write((Dee_funptr_t *)table, value);
	return true;
}


struct Dee_tno_assign {
	enum Dee_tno_id       tnoa_id;          /* Operator slot ID to write to */
	Dee_funptr_t          tnoa_cb;          /* [1..1] Function pointer to write to `tnoa_id' */
	__UINTPTR_HALF_TYPE__ tnoa_pres_dep[2]; /* Already-present dependencies of `tnoa_cb' (or `Dee_TNO_COUNT') */
};

/* The max # of operator slots that might ever need to be assigned at once.
 * iow: this is the length of the longest non-looping dependency chain that
 *      can be formed using `default__*__with__*' callbacks below. */
/*[[[deemon (print_TNO_ASSIGN_MAXLEN from "..method-hints.method-hints")();]]]*/
/* { Dee_TNO_getitem,                    &default__getitem__with__getitem_index__and__getitem_string_len_hash }
 * { Dee_TNO_hasitem_string_len_hash,    &default__hasitem_string_len_hash__with__hasitem }
 * { Dee_TNO_bounditem,                  &default__bounditem__with__trygetitem__and__hasitem }
 * { Dee_TNO_getitem_index,              &default__getitem_index__with__trygetitem_index__and__hasitem_index }
 * { Dee_TNO_trygetitem_index,           &default__trygetitem_index__with__trygetitem }
 * { Dee_TNO_trygetitem,                 &default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash }
 * { Dee_TNO_trygetitem_string_len_hash, &default__trygetitem_string_len_hash__with__trygetitem_string_hash }
 * { Dee_TNO_trygetitem_string_hash,     &default__trygetitem_string_hash__with__getitem_string_hash }
 * { Dee_TNO_getitem_string_hash,        &default__getitem_string_hash__with__getitem_string_len_hash }
 * { Dee_TNO_getitem_string_len_hash,    &default__getitem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash }
 * { Dee_TNO_hasitem_string_len_hash,    &default__hasitem_string_len_hash__with__bounditem_string_len_hash }
 * { Dee_TNO_bounditem_string_len_hash,  &default__bounditem_string_len_hash__with__bounditem_string_hash }
 * { Dee_TNO_bounditem_string_hash,      &default__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash }
 * { Dee_TNO_hasitem_string_hash,        &default__hasitem_string_hash__with__hasitem }
 * { Dee_TNO_hasitem,                    &default__hasitem__with__bounditem }
 * { Dee_TNO_bounditem,                  &default__bounditem__with__bounditem_index__and__bounditem_string_len_hash }
 * { Dee_TNO_bounditem_index,            &default__bounditem_index__with__trygetitem_index__and__hasitem_index }
 * { Dee_TNO_hasitem_index,              &default__hasitem_index__with__size__and__getitem_index_fast }
 * { Dee_TNO_size,                       &default__size__with__sizeob }
 * { Dee_TNO_sizeob,                     &default__sizeob__with__size } */
#define Dee_TNO_ASSIGN_MAXLEN 20 /* 23 with duplicates */
/*[[[end]]]*/


PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_tno_assign_contains(struct Dee_tno_assign const *actions,
                        size_t actions_count, enum Dee_tno_id id) {
	size_t i;
	for (i = 0; i < actions_count; ++i) {
		if (actions[i].tnoa_id == id)
			return true;
	}
	return false;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_tno_assign_haspresdep(struct Dee_tno_assign const *actions,
                          size_t actions_count, enum Dee_tno_id id) {
	size_t i;
	for (i = 0; i < actions_count; ++i) {
		size_t j;
		for (j = 0; j < COMPILER_LENOF(actions[i].tnoa_pres_dep); ++j) {
			if ((enum Dee_tno_id)actions[i].tnoa_pres_dep[j] == id)
				return true;
		}
	}
	return false;
}

PRIVATE ATTR_CONST WUNUSED bool DCALL
is_disliked_impl(enum Dee_tno_id id, Dee_funptr_t impl) {
	struct oh_init_spec const *specs = &oh_init_specs[id];
	struct oh_init_spec_impl const *iter = specs->ohis_impls;
	ASSERT(iter);
	for (;; ++iter) {
		ASSERT(iter->ohisi_impl);
		if (iter->ohisi_impl == impl)
			return oh_init_spec_impl_isdisliked(iter);
	}
}

/* Returns the # score from "actions". Here, a lower score is preferred over a greater one. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) size_t DCALL
Dee_tno_assign_score(struct Dee_tno_assign const *actions,
                     size_t actions_count) {
	size_t i;
	size_t n_presdep = 0;
	size_t n_disliked = 0;
	size_t result;
	for (i = 0; i < actions_count; ++i) {
		size_t j;
		for (j = 0; j < COMPILER_LENOF(actions[i].tnoa_pres_dep); ++j) {
			enum Dee_tno_id dep = (enum Dee_tno_id)actions[i].tnoa_pres_dep[j];
			if (dep >= Dee_TNO_COUNT)
				break;
			if (Dee_tno_assign_haspresdep(actions, i, dep))
				continue;
			if (j == 1 && (enum Dee_tno_id)actions[i].tnoa_pres_dep[0] == dep)
				continue;
			++n_presdep; /* New, distinct, present dependency */
		}
		if (is_disliked_impl(actions[i].tnoa_id, actions[i].tnoa_cb))
			++n_disliked;
	}
	result = (actions_count * COMPILER_LENOF(actions->tnoa_pres_dep)) - n_presdep;
	result += n_disliked << ((sizeof(size_t) * __CHAR_BIT__) / 2);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t
(DCALL do_DeeType_SelectMissingNativeOperator)(DeeTypeObject const *__restrict self, enum Dee_tno_id id,
                                               struct Dee_tno_assign actions[Dee_TNO_ASSIGN_MAXLEN],
                                               size_t actions_count) {
#ifdef Dee_Allocac
	struct Dee_tno_assign *winner_actions = (struct Dee_tno_assign *)Dee_Allocac(Dee_TNO_ASSIGN_MAXLEN - actions_count,
	                                                                             sizeof(struct Dee_tno_assign));
#else /* Dee_Allocac */
	struct Dee_tno_assign winner_actions[Dee_TNO_ASSIGN_MAXLEN];
#endif /* !Dee_Allocac */
	size_t impl_i, winner_action_nacts, winner_action_score;
	struct oh_init_spec const *specs = &oh_init_specs[id];
	struct oh_init_spec_impl const *impls = specs->ohis_impls;
	if unlikely(!impls)
		return 0;
	ASSERT(!Dee_tno_assign_contains(actions, actions_count, id));
	actions[actions_count].tnoa_id = id;

	winner_action_nacts = (size_t)-1;
	winner_action_score = (size_t)-1;
	for (impl_i = 0; impls[impl_i].ohisi_impl; ++impl_i) {
		size_t dep_i, dependency_nacts, dependency_score;
		struct oh_init_spec_impl const *impl = &impls[impl_i];
		enum Dee_tno_id missing_dependencies[2];
		actions[actions_count].tnoa_cb = impl->ohisi_impl;

		/* Load dependencies of this impl. */
		missing_dependencies[0] = (enum Dee_tno_id)impl->ohisi_deps[0];
		missing_dependencies[1] = (enum Dee_tno_id)impl->ohisi_deps[1];
		if (missing_dependencies[1] == missing_dependencies[0])
			missing_dependencies[1] = Dee_TNO_COUNT; /* Ignore duplicate dependencies (marker for [[disliked]]) */
		actions[actions_count].tnoa_pres_dep[0] = Dee_TNO_COUNT;
		actions[actions_count].tnoa_pres_dep[1] = Dee_TNO_COUNT;
		for (dep_i = 0; dep_i < COMPILER_LENOF(missing_dependencies); ++dep_i) {
			if (missing_dependencies[dep_i] >= Dee_TNO_COUNT)
				break; /* No more dependencies... */
			if (Dee_tno_assign_contains(actions, actions_count, missing_dependencies[dep_i])) {
				/* Implementation has a dependencies that is already being assigned,
				 * but will be assigned *AFTER* the action we are currently filling.
				 *
				 * iow: if we were to allow use of this dependency, there would not
				 *      only be a dependency loop (that would most definitely result
				 *      in a hard stack-overflow if we allowed it), but there would
				 *      be a moment where an operator was assigned with a dependency
				 *      that hasn't already been assigned. */
				goto next_implementation;
			}
			if (DeeType_GetNativeOperatorWithoutDefaults(self, missing_dependencies[dep_i])) {
				actions[actions_count].tnoa_pres_dep[dep_i] = (__UINTPTR_HALF_TYPE__)missing_dependencies[dep_i];
				missing_dependencies[dep_i] = Dee_TNO_COUNT; /* Dependency is present */
			}
		}

		/* Handle the case where the first dependency was resolved. */
		if (missing_dependencies[0] == Dee_TNO_COUNT) {
			missing_dependencies[0] = missing_dependencies[1];
			missing_dependencies[1] = Dee_TNO_COUNT;

			/* Check for optimal case: all dependencies are natively present */
			if (missing_dependencies[0] == Dee_TNO_COUNT)
				return actions_count + 1;
		}

		/* Slow path: recursively generate assignment instructions for dependencies. */
		dependency_nacts = actions_count + 1;
		for (dep_i = 0; dep_i < COMPILER_LENOF(missing_dependencies); ++dep_i) {
			if (missing_dependencies[dep_i] >= Dee_TNO_COUNT)
				break; /* No more dependencies... */
			if (dep_i == 0 && missing_dependencies[1] < Dee_TNO_COUNT) {
				/* Special handling required: must prevent the recursive call from
				 * prematurely filling in the second dependency (since *we* want to
				 * fill that one in) */
				size_t old_dependency_score = dependency_nacts;
				actions[old_dependency_score].tnoa_id = missing_dependencies[1];
				DBG_memset(&actions[old_dependency_score].tnoa_cb, 0xcc,
				           sizeof(actions[old_dependency_score].tnoa_cb));
				dependency_nacts = do_DeeType_SelectMissingNativeOperator(self, missing_dependencies[dep_i],
				                                                          actions, old_dependency_score + 1);
				if (dependency_nacts == 0) /* Implementation not possible due to missing dependencies */
					goto next_implementation;
				ASSERT(dependency_nacts >= old_dependency_score);
				memmovedownc(&actions[old_dependency_score], &actions[old_dependency_score + 1],
				             dependency_nacts - old_dependency_score, sizeof(struct Dee_tno_assign));
				--dependency_nacts;
			} else {
				dependency_nacts = do_DeeType_SelectMissingNativeOperator(self, missing_dependencies[dep_i],
				                                                          actions, dependency_nacts);
				if (dependency_nacts == 0) /* Implementation not possible due to missing dependencies */
					goto next_implementation;
			}
		}

		/* See if we got a better winner impl! For this purpose (in order):
		 *
		 * #1 Minimize the number of [[disliked]] impls being used (including
		 *    "impl->ohisi_impl" which we ourselves assumed ourselves above). iow:
		 *    prefer "default__hasitem_string_len_hash__with__bounditem_string_len_hash" +
		 *           "default__bounditem_string_len_hash__with__trygetitem_string_len_hash"
		 *    over   "default__hasitem_string_len_hash__with__hasitem" +
		 *           "default__hasitem__with__bounditem" +
		 *           "default__bounditem__with__trygetitem"
		 *           "default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash"
		 *    (even though the later has more recursively present (non-default) dependencies)
		 *
		 * #2 Maximize the number of recursively present (non-default) dependencies. iow:
		 *    prefer "default__getitem__with__getitem_index__and__getitem_string_len_hash"
		 *    over   "default__getitem__with__getitem_index"
		 *
		 * #3 Minimize the number of transitive dependencies. iow:
		 *    prefer "default__bounditem__with__trygetitem"
		 *    over   "default__bounditem__with__bounditem_index" +
		 *           "default__bounditem_index__with__trygetitem_index" +
		 *           "default__trygetitem_index__with__trygetitem"
		 */
		dependency_score = Dee_tno_assign_score(actions + actions_count,
		                                        dependency_nacts - actions_count);
		if ((winner_action_score > dependency_score) ||
		    (winner_action_score == dependency_score &&
		     winner_action_nacts > dependency_nacts)) {
			winner_action_nacts = dependency_nacts;
			winner_action_score = dependency_score;
			memcpyc(winner_actions, actions + actions_count,
			        winner_action_nacts - actions_count,
			        sizeof(struct Dee_tno_assign));
		}
next_implementation:;
	}

	/* Use the winner (if we have one) */
	if (winner_action_nacts != (size_t)-1) {
		memcpyc(actions + actions_count, winner_actions,
		        winner_action_nacts - actions_count,
		        sizeof(struct Dee_tno_assign));
		return winner_action_nacts;
	}

	/* Fallback: no implementation possible */
	return 0;
}

/* Looking at related operators that actually *are* present,
 * and assuming that `id' isn't implemented, return the most
 * applicable default implementation for the operator.
 *
 * e.g. Given a type that defines `tp_iter' and `id=Dee_TNO_foreach',
 *      this function would return `&default__foreach__with__iter'
 *
 * When no related operators are present (or `id' doesn't
 * have *any* related operators), return `NULL' instead.
 *
 * NOTE: This function does *!!NOT!!* return method hint pointers
 * @param actions: Actions that need to be performed (multiple assignments
 *                 might be necessary in case of a transitive dependency)
 *                 Stored actions must be performed in *REVERSE* order
 *                 The first (last-written) action is always for `id'
 * @return: The number of actions written to `actions' */
PRIVATE WUNUSED NONNULL((1)) size_t
(DCALL DeeType_SelectMissingNativeOperator)(DeeTypeObject const *__restrict self, enum Dee_tno_id id,
                                            struct Dee_tno_assign actions[Dee_TNO_ASSIGN_MAXLEN]) {
	return do_DeeType_SelectMissingNativeOperator(self, id, actions, 0);
}


INTDEF ATTR_CONST WUNUSED bool DCALL /* Implemented in "./method-hint-super-invoke.c" */
Dee_tmh_isdefault_or_usrtype(enum Dee_tmh_id id, Dee_funptr_t funptr);

/* Return an actual, user-defined operator "id"
 * (*NOT* allowing stuff like `default__size__with__sizeob'
 * or `default__seq_operator_size__with__seq_operator_sizeob')
 * Also never returns `DeeType_GetNativeOperatorOOM()' or
 * `DeeType_GetNativeOperatorUnsupported()' */
INTERN WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperatorWithoutDefaults)(DeeTypeObject const *__restrict self,
                                                 enum Dee_tno_id id) {
	Dee_funptr_t result = type_tno_get(self, id);
	if (result) {
		/* Check if `result' might be a default operator
		 * implementation (including method hint defaults). */
		struct oh_init_spec const *specs = &oh_init_specs[id];
		if (specs->ohis_class) {
			/* Important: `Dee_tmh_isdefault_or_usrtype' would indicate
			 * that usrtype impls would be default impls, but for our
			 * purpose, that mustn't actually be the case. As such, filter
			 * out these impls here and treat them as non-default. */
			struct oh_init_spec_class const *iter = specs->ohis_class;
			for (; iter->ohisc_usertyp; ++iter) {
				if (result == iter->ohisc_usertyp)
					goto yes;
			}
		}
		if (specs->ohis_impls) {
			struct oh_init_spec_impl const *iter = specs->ohis_impls;
			for (; iter->ohisi_impl; ++iter) {
				if (result == iter->ohisi_impl)
					goto nope;
			}
		}
		if (specs->ohis_mhints) {
			struct oh_init_spec_mhint const *iter = specs->ohis_mhints;
			for (; iter->ohismh_id < Dee_TMH_COUNT; ++iter) {
				if (Dee_tmh_isdefault_or_usrtype(iter->ohismh_id, result))
					goto nope;
			}
		}
#ifdef CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS
		if (result == DeeType_GetNativeOperatorUnsupported(id))
			goto nope;
#endif /* CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS */
	}
yes:
	return result;
nope:
	return NULL;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
present_depv_contains(enum Dee_tno_id const *present_depv,
                      size_t n, enum Dee_tno_id dep) {
	size_t i;
	for (i = 0; i < n; ++i) {
		if (present_depv[i] == dep)
			return true;
	}
	return false;
}

/* Wrapper around `DeeType_SelectMissingNativeOperator' that checks if the
 * operator is already defined, and if not: see if can be substituted via
 * some other set of native operators (in which case: do that substitution
 * and then return the operator's function pointer) */
INTERN WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperatorWithoutHints)(DeeTypeObject *__restrict self, enum Dee_tno_id id) {
	Dee_funptr_t result = type_tno_get(self, id);
	if (!result) {
		struct Dee_tno_assign actions[Dee_TNO_ASSIGN_MAXLEN];
		size_t n_actions = DeeType_SelectMissingNativeOperator(self, id, actions);
		if (n_actions > 0) {
			/* If all currently present dependencies from "actions" have been inherited
			 * from a singular type, then "id" also has to be inherited from that type
			 * and cannot be loaded here (meaning: we have to return "NULL")
			 *
			 * Without this, we would get sub-optimal inheritance like:
			 * >> @[nobuiltin] class MyCell: Cell { this = super; }
			 * - MyCell.<native>.tp_compare       = cell_compare
			 * - MyCell.<native>.tp_compare_eq    = cell_compare | default__compare_eq__with__compare
			 * - MyCell.<native>.tp_trycompare_eq = cell_trycompare_eq | default__trycompare_eq__with__compare_eq
			 * Which one you get depends on which operator is accessed first (iow: inconsistent behavior)
			 *
			 * With this, we **always** get optimal (and consistent) inheritance like:
			 * - MyCell.<native>.tp_compare       = cell_compare
			 * - MyCell.<native>.tp_compare_eq    = cell_compare
			 * - MyCell.<native>.tp_trycompare_eq = cell_trycompare_eq
			 */
			size_t i_actions, present_depc;
			enum Dee_tno_id present_depv[Dee_TNO_ASSIGN_MAXLEN + 1];
			for (i_actions = 0, present_depc = 0; i_actions < n_actions; ++i_actions) {
				size_t pres_i;
				for (pres_i = 0; pres_i < COMPILER_LENOF(actions[i_actions].tnoa_pres_dep); ++pres_i) {
					enum Dee_tno_id present_dep = (enum Dee_tno_id)actions[i_actions].tnoa_pres_dep[pres_i];
					if ((present_dep < Dee_TNO_COUNT) &&
					    !present_depv_contains(present_depv, present_depc, present_dep)) {
						ASSERT(present_depc < COMPILER_LENOF(present_depv));
						present_depv[present_depc] = present_dep;
						++present_depc;
					}
				}
			}

			if (present_depc > 0) {
				DeeTypeObject *dep_origin;
				dep_origin = DeeType_GetNativeOperatorOrigin(self, present_depv[0]);
				ASSERTF(dep_origin, "Why is this NULL? This operator is known to be present!");
				if (dep_origin != self) {
					/* Check if all other, present dependencies also originate from "dep_origin"
					 * If that ends up being the case, then we mustn't assign the default impl
					 * described by "actions", since the operator in question *NEEDS* to be
					 * inherited (from "dep_origin") */
					size_t present_depi;
					for (present_depi = 1; present_depi < present_depc; ++present_depi) {
						DeeTypeObject *dep_origin2;
						dep_origin2 = DeeType_GetNativeOperatorOrigin(self, present_depv[present_depi]);
						ASSERTF(dep_origin, "Why is this NULL? This operator is known to be present!");
						if (dep_origin2 != dep_origin)
							goto perform_actions;
					}

					/* Nope: this operator has to be inherited (from "dep_origin") */
					return NULL;
				}
			}

perform_actions:
			/* TODO: Think about getting rid of "default__iter__with__foreach"
			 * - It's not even fully implemented, and it would be super-slow.
			 * - By not implementing it, our caller would be forced to use
			 *   method hints (which are able to re-implement it for sequence
			 *   types, but are also smart enough to use size+getitem instead
			 *   if at all possible)
			 * - Some stuff actually uses that impls:
			 *   >> CRTL+SHIFT+F: DEFIMPL(&default__iter__with__foreach) */

			while (n_actions > 1) {
				struct Dee_tno_assign *action;
				--n_actions;
				action = &actions[n_actions];
				if (!type_tno_tryset(self, action->tnoa_id, action->tnoa_cb)) {
					Dee_DPRINTF("[error] OOM while trying to allocate operator table for '%d'='%p'\n",
					            action->tnoa_id, *(void **)&action->tnoa_cb);
					return DeeType_GetNativeOperatorOOM(id);
				}
			}
			ASSERT(actions[0].tnoa_id == id);
			(void)type_tno_tryset(self, id, actions[0].tnoa_cb);
			result = actions[0].tnoa_cb;
		}
	}
	return result;
}

#if 0
PRIVATE ATTR_CONST WUNUSED bool DCALL
is_native_operator_default_impl(enum Dee_tno_id id, Dee_funptr_t impl) {
	struct oh_init_spec const *specs = &oh_init_specs[id];
	if (specs->ohis_impls) {
		struct oh_init_spec_impl const *iter;
		for (iter = specs->ohis_impls; iter->ohisi_impl; ++iter) {
			if (iter->ohisi_impl == impl)
				return true;
		}
	}
	return false;
}
#endif


/* Check if "impl" is a method-hint implementation that needs to be transformed when inherited.
 * If so, return the transformed function pointer, but if not: re-return "impl" as-is.
 *
 * Returns "NULL" if the operator cannot be inherited due to dependency conflicts. */
PRIVATE ATTR_PURE NONNULL((1, 2, 4)) Dee_funptr_t DCALL
DeeType_MapTMHInTNOForInherit(DeeTypeObject *__restrict from,
                              DeeTypeObject *__restrict into,
                              enum Dee_tno_id id, Dee_funptr_t impl) {
	struct oh_init_spec const *specs = &oh_init_specs[id];
	if (specs->ohis_inherit) {
		struct oh_init_inherit_as const *iter = specs->ohis_inherit;
		for (; iter->ohia_optr; ++iter) {
			if (impl == iter->ohia_optr) {
				impl = iter->ohia_nptr;
				break;
			}
		}
	}

	/* Manually exclude native operator default impls here! */
	if (specs->ohis_mhints /*&& !is_native_operator_default_impl(id, impl)*/) {
		bool has_unsupported_default = false;
		struct oh_init_spec_mhint const *iter = specs->ohis_mhints;

		/* When inheriting method hint wrappers, map them to their full method hints.
		 * e.g.: Replace "default__set_operator_add" (or any other default impl such
		 * as "tdefault__set_operator_add__with_callobjectcache___set_add__") with
		 * its relevant default impl for "into". If "into" doesn't support such a
		 * default impl, must inherit as "default__set_operator_add".
		 *
		 * This is also the trick that allows types to inherit set math operators
		 * from "Sequence" (although they don't get mapped here)
		 *
		 * If we were to map operators here, even when "from" isn't supposed to
		 * support them, then we might confused "default__seq_operator_iter__empty"
		 * with "default__set_operator_iter__empty" (because the later is an alias
		 * for the former, so "default__set_operator_iter" has to stay as-is if
		 * not natively supported by the "from" type) */
		for (; iter->ohismh_id < Dee_TMH_COUNT; ++iter) {
			Dee_funptr_t mapped;
			if (!oh_init_spec_mhint_caninherit(iter, from))
				continue; /* The hint must obviously be usable by "from" */
			mapped = DeeType_MapDefaultMethodHintOperatorImplForInherit(into, iter->ohismh_id, impl);
			if (mapped) {
				/* In order to allow being inherited by "into",
				 * that type must support the operator, too! */
				if (!oh_init_spec_mhint_canuse(iter, into)) {
					has_unsupported_default = true;
					continue;
				}
				return mapped;
			}
		}
		if (has_unsupported_default)
			return NULL;
	}

	/* Super-special case: "tp_getitem_index_fast" can only be inherited
	 * if certain method hints are shared between "from" and "into". */
	if (id == Dee_TNO_getitem_index_fast) {
		static enum Dee_tmh_id const giif_deps[] = {
			Dee_TMH_seq_operator_getitem_index,
			Dee_TMH_seq_operator_getitem,
		};
		size_t i;
		for (i = 0; i < COMPILER_LENOF(giif_deps); ++i) {
			enum Dee_tmh_id dep = giif_deps[i];
			Dee_funptr_t from_ptr, into_ptr;
			if ((from_ptr = DeeType_GetMethodHint(from, dep)) == NULL)
				goto nope;
			if ((into_ptr = DeeType_GetMethodHint(into, dep)) == NULL)
				goto nope;
			if (from_ptr != into_ptr)
				goto nope;
		}
	}
	return impl;
nope:
	return NULL;
}


struct Dee_tno_inherit {
	enum Dee_tno_id tnoi_id; /* Operator slot ID to write to */
	Dee_funptr_t    tnoi_cb; /* [1..1] Function pointer to write to `tnoi_id' */
};

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) bool DCALL
Dee_tno_inherit_contains(struct Dee_tno_inherit const actions[Dee_TNO_ASSIGN_MAXLEN],
                         size_t actions_count, enum Dee_tno_id id) {
	size_t i;
	for (i = 0; i < actions_count; ++i) {
		if (actions[i].tnoi_id == id)
			return true;
	}
	return false;
}



/* @return: * :   Actual, inherited implementation
 * @return: NULL: Operator cannot be inherited (not implemented for type "into") */
PRIVATE WUNUSED NONNULL((1, 2, 4, 5, 6)) Dee_funptr_t
(DCALL do_DeeType_InheritNativeOperatorWithoutHints)(DeeTypeObject *__restrict from,
                                                     DeeTypeObject *__restrict into, enum Dee_tno_id id,
                                                     struct Dee_tno_inherit actions[Dee_TNO_ASSIGN_MAXLEN],
                                                     size_t *__restrict p_nactions, Dee_funptr_t impl) {
	struct oh_init_spec const *specs = &oh_init_specs[id];
	ASSERT(from != into);
	ASSERT(DeeType_GetNativeOperatorWithoutHints(from, id) == impl);

	/* Inherit this operator (even if it's a method hint).
	 *
	 * The reason we can do this is because our caller previously tried to
	 * make use of `DeeType_GetNativeOperatorWithoutInherit', which already
	 * attempted to load a method hint into the operator. As such, we know
	 * that no method-hint implementation of the operator is available for
	 * this type in particular, meaning that if we happen to inherit a hint
	 * impl from a base-type, then that impl would *always* be the correct
	 * impl for "into", too.
	 *
	 * However, if the impl being inherited is a method hint, and has a
	 * "[[inherit_as(...)]]" annotation, then we must inherit the referenced
	 * implementation, rather than the original one. (This is needed to
	 * map method hint default impls that assume the presence of specific
	 * operators within `Dee_TYPE(into)'). */
	impl = DeeType_MapTMHInTNOForInherit(from, into, id, impl);
	if unlikely(!impl) {
		/* Unfulfilled conditions */
		return NULL;
	}

	/* Check if the given "impl" is one of the defaults that has dependencies.
	 * If that is the case, then we must ensure that those dependencies are also
	 * inherited (if not already present in `into').
	 *
	 * We're allowed to assume that "impl" is the only correct impl, so there is
	 * no need to check if some other impl should be used instead (no other impl
	 * should be) */
	if (specs->ohis_impls) {
		struct oh_init_spec_impl const *iter = specs->ohis_impls;
		for (; iter->ohisi_impl; ++iter) {
			size_t dep_i;
			if (iter->ohisi_impl != impl)
				continue;

			/* Found the impl in question -> must now also inherit *its* dependencies */
			for (dep_i = 0; dep_i < COMPILER_LENOF(iter->ohisi_deps); ++dep_i) {
				Dee_funptr_t dep_impl, into_dep_impl;
				enum Dee_tno_id dep = (enum Dee_tno_id)iter->ohisi_deps[dep_i];
				if ((unsigned int)dep >= Dee_TNO_COUNT)
					break;

				/* Check if the dependency is already present in "into". */
				into_dep_impl = type_tno_get(into, dep);
				if (into_dep_impl) {
#ifdef CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS
					if (into_dep_impl == DeeType_GetNativeOperatorUnsupported(dep))
						return NULL; /* Unsatisfied dependency */
#endif /* CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS */
					continue;
				}

				/* Also skip if "dep" is already appears in "actions"
				 * (because it will be present by the time "impl" gets written) */
				if (Dee_tno_inherit_contains(actions, *p_nactions, dep))
					continue;

				dep_impl = type_tno_get(from, dep);
#ifdef CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS
#define LOCAL_VERIFY_DEP_IMPL() (dep_impl != NULL && dep_impl != DeeType_GetNativeOperatorUnsupported(dep))
#else /* CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS */
#define LOCAL_VERIFY_DEP_IMPL() (dep_impl != NULL)
#endif /* !CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS */
				ASSERTF(LOCAL_VERIFY_DEP_IMPL(),
				        "Transitive operator '%u' used as dependency was not initialized in '%k'",
				        (unsigned int)dep, from);
#undef LOCAL_VERIFY_DEP_IMPL
				dep_impl = do_DeeType_InheritNativeOperatorWithoutHints(from, into, dep, actions, p_nactions, dep_impl);
				if unlikely(!dep_impl)
					return NULL; /* Unsatisfied dependency */
			}
			break;
		}
	}

	/* Append the assignment action. */
	{
		size_t n = (*p_nactions)++;
		ASSERT(n < Dee_TNO_ASSIGN_MAXLEN);
		actions[n].tnoi_id = id;
		actions[n].tnoi_cb = impl;
	}
	return impl;
}

/* Ignoring method hints that might have been able to implement "id" along
 * the way, and assuming that `DeeType_GetNativeOperatorWithoutHints(from, id)'
 * returned `impl', make sure that `impl' can be (and is) inherited by `into'.
 *
 * This function is allowed to assume that "impl" really is what should be
 * inherited for the specified "id" (if it is not correct, everything breaks)
 *
 * @return: Indicative of a successful inherit (inherit may fail when "impl"
 *          is `default__*__with__*', and dependencies could not be written
 *          due to OOM, though in this case, no error is thrown) */
PRIVATE WUNUSED NONNULL((1, 2, 4)) Dee_funptr_t
(DCALL DeeType_InheritNativeOperatorWithoutHints)(DeeTypeObject *__restrict from,
                                                  DeeTypeObject *__restrict into,
                                                  enum Dee_tno_id id, Dee_funptr_t impl) {
	size_t n_actions = 0;
	Dee_funptr_t result;
	struct Dee_tno_inherit actions[Dee_TNO_ASSIGN_MAXLEN];
	result = do_DeeType_InheritNativeOperatorWithoutHints(from, into, id, actions, &n_actions, impl);
	if likely(result) {
		size_t i;
		/* Perform inherit actions. */
		ASSERT(n_actions >= 1);
		ASSERT(actions[n_actions - 1].tnoi_id == id);
		ASSERT(actions[n_actions - 1].tnoi_cb == result);
		for (i = 0; i < n_actions; ++i) {
			bool ok;
#ifndef Dee_DPRINT_IS_NOOP
			Dee_operator_t op = DeeType_GetOperatorOfTno(actions[i].tnoi_id);
			struct Dee_opinfo const *info = op < OPERATOR_USERCOUNT
			    ? DeeTypeType_GetOperatorById(Dee_TYPE(from), op)
			    : NULL;
			Dee_DPRINTF("[RT] Inherit '%s.operator %s' into '%s' [tno: %u, ptr: %p]\n",
			            DeeType_GetName(from), info ? info->oi_sname : "?",
			            DeeType_GetName(into),
			            (unsigned int)actions[i].tnoi_id,
			            *(void **)&actions[i].tnoi_cb);
#endif /* !Dee_DPRINT_IS_NOOP */
			/* special handling when inheriting certain operators */
			switch (actions[i].tnoi_id) {
			case Dee_TNO_int:
			case Dee_TNO_int32:
			case Dee_TNO_int64:
				if (from->tp_flags & TP_FTRUNCATE)
					atomic_or(&into->tp_flags, TP_FTRUNCATE);
				break;
			default: break;
			}

			ok = type_tno_tryset(into, actions[i].tnoi_id, actions[i].tnoi_cb);
			if likely(ok)
				continue;
			/* It's OK if the last write fails (because we still hand the correct pointer
			 * to our caller), but it's not OK if any of the intermediate writes fail (as
			 * in that case, the returned callback would try to access unallocated memory) */
			if (i == n_actions - 1)
				break;
			return DeeType_GetNativeOperatorOOM(id);
		}
	}
	return result;
}



/* Same as `DeeType_GetNativeOperatorWithoutHints', but also load operators
 * from method hints (though don't inherit them from base-types, yet). */
INTERN WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperatorWithoutInherit)(DeeTypeObject *__restrict self, enum Dee_tno_id id) {
	Dee_funptr_t result = DeeType_GetNativeOperatorWithoutHints(self, id);
	if (!result) {
		/* Check if the operator can be implemented using method hints. */
		struct oh_init_spec const *specs = &oh_init_specs[id];
		struct oh_init_spec_mhint const *iter = specs->ohis_mhints;
		if (iter) {
			for (; iter->ohismh_id < Dee_TMH_COUNT; ++iter) {
				if (!oh_init_spec_mhint_canuse(iter, self))
					continue;

				/* Only check for method hints privately! */
				result = DeeType_GetPrivateMethodHint(self, self, iter->ohismh_id);
				if (result) {
					/* Remember that this operator has been inherited from method hints. */
					type_tno_tryset(self, id, result);
					break;
				}
			}
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_InheritNativeOperator)(DeeTypeObject *__restrict self, enum Dee_tno_id id) {
	DeeTypeMRO mro;
	DeeTypeObject *iter = DeeTypeMRO_Init(&mro, self);
	while ((iter = DeeTypeMRO_NextDirectBase(&mro, iter)) != NULL) {
		Dee_funptr_t result = DeeType_GetNativeOperatorWithoutUnsupported(iter, id);
		if (result)
			return DeeType_InheritNativeOperatorWithoutHints(iter, self, id, result);
	}
	return NULL;
}

/* Same as `DeeType_GetNativeOperatorWithoutUnsupported()', but never returns NULL
 * (for any operator linked against a deemon user-code ID (e.g. "OPERATOR_ITER"))
 * and instead returns special implementations for each operator that simply call
 * `err_unimplemented_operator()' with the relevant arguments, before returning
 * whatever is indicative of an error in the context of the native operator. */
PUBLIC WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperator)(DeeTypeObject *__restrict self, enum Dee_tno_id id) {
#ifdef CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS
	Dee_funptr_t result = DeeType_GetNativeOperatorWithoutInherit(self, id);
	if unlikely(!result) {
		result = DeeType_InheritNativeOperator(self, id);
		if unlikely(result == NULL) {
			/* Operator isn't supported :( -- if possible, try to remember that fact */
			result = DeeType_GetNativeOperatorUnsupported(id);
			if likely(result)
				(void)type_tno_tryset(self, id, result);
		}
	}
	return result;
#else /* CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS */
	Dee_funptr_t result = DeeType_GetNativeOperatorWithoutUnsupported(self, id);
	if unlikely(!result) /* Note how we don't write this impl back to "self" */
		result = DeeType_GetNativeOperatorUnsupported(id);
	return result;
#endif /* !CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS */
}

/* Same as `DeeType_GetNativeOperatorWithoutInherit', but actually also does the
 * operator inherit part (meaning that this is the low-level* master-function
 * that's called when you invoke one of the standard operators whose callback
 * is currently set to "NULL" within its relevant type)
 * [*] The actual master function is `DeeType_GetNativeOperator', but that
 *     one only adds coalesce to `DeeType_GetNativeOperatorUnsupported()' */
PUBLIC WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperatorWithoutUnsupported)(DeeTypeObject *__restrict self, enum Dee_tno_id id) {
#ifdef CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS
#ifdef __OPTIMIZE_SIZE__
	Dee_funptr_t result = DeeType_GetNativeOperator(self, id);
#else /* __OPTIMIZE_SIZE__ */
	Dee_funptr_t result = type_tno_get(self, id);
	if unlikely(result == NULL)
		result = DeeType_GetNativeOperator(self, id);
#endif /* !__OPTIMIZE_SIZE__ */
	if (result == DeeType_GetNativeOperatorUnsupported(id))
		result = NULL;
	return result;
#else /* CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS */
	Dee_funptr_t result = DeeType_GetNativeOperatorWithoutInherit(self, id);
	if unlikely(!result)
		result = DeeType_InheritNativeOperator(self, id);
	return result;
#endif /* !CONFIG_CACHE_UNSUPPORTED_NATIVE_OPERATORS */
}


PRIVATE ATTR_PURE WUNUSED NONNULL((1)) DeeTypeObject *
(DCALL DeeClass_GetOperatorOrigin)(DeeTypeObject *__restrict self, Dee_operator_t op) {
	DeeTypeObject *iter;
	DeeTypeMRO mro;
	ASSERT(DeeType_IsClass(self));
	iter = DeeTypeMRO_Init(&mro, self);
	do {
		if (DeeClass_TryGetPrivateOperatorPtr(iter, op))
			return iter;
	} while ((iter = DeeTypeMRO_Next(&mro, iter)) != NULL && DeeType_IsClass(iter));
	return self;
}

/* Find the type where native operator "id" has been inherited from.
 * This function correctly handles:
 * - when "id" is implemented as an alias for another operator (in which case it
 *   returns the origin of the operator's first dependency, or "self" if there are
 *   no dependencies)
 * - when "id" is implemented using a method hint, return the origin of that hint
 * - when "id" is not implemented at all, return "NULL" */
INTERN WUNUSED NONNULL((1)) DeeTypeObject *
(DCALL DeeType_GetNativeOperatorOrigin)(DeeTypeObject *__restrict self, enum Dee_tno_id id) {
	DeeTypeMRO mro;
	DeeTypeObject *iter;
	struct oh_init_spec const *specs = &oh_init_specs[id];
	struct oh_init_spec_impl const *impls;
	struct oh_init_spec_mhint const *mhints;
	Dee_funptr_t funptr = DeeType_GetNativeOperatorWithoutUnsupported(self, id);
	if (funptr == NULL)
		return NULL;

	/* Check if the native operator points as a class implementation */
	if (specs->ohis_class && DeeType_IsClass(self)) {
		struct oh_init_spec_class const *usrtype = specs->ohis_class;
		for (; usrtype->ohisc_usertyp; ++usrtype) {
			if (usrtype->ohisc_usertyp == funptr) {
				/* TODO: What should we return when dependencies come from different types? */
				if (usrtype->ohisc_dep1 != OPERATOR_USERCOUNT)
					return DeeClass_GetOperatorOrigin(self, usrtype->ohisc_dep1);
				if (usrtype->ohisc_dep2 != OPERATOR_USERCOUNT)
					return DeeClass_GetOperatorOrigin(self, usrtype->ohisc_dep2);
				return self;
			}
		}
	}

	/* Check if the native operator is implemented by shadowing another operator. */
	impls = specs->ohis_impls;
	if (impls) {
		for (; impls->ohisi_impl; ++impls) {
			if (impls->ohisi_impl == funptr) {
				/* TODO: What should we return when dependencies come from different types? */
				if (impls->ohisi_deps[0] < Dee_TNO_COUNT) {
					/* TODO: Stack overflow here between Dee_TNO_enter / Dee_TNO_leave when running:
					 *       >> deemon -F -Wno-usage -Wno-user include/deemon/cxx/none.h */
					return DeeType_GetNativeOperatorOrigin(self, (enum Dee_tno_id)impls->ohisi_deps[0]);
				}
				if (impls->ohisi_deps[1] < Dee_TNO_COUNT)
					return DeeType_GetNativeOperatorOrigin(self, (enum Dee_tno_id)impls->ohisi_deps[1]);
				return self;
			}
		}
	}

	/* Check if the operator is implemented via method hints. */
	mhints = specs->ohis_mhints;
	if (mhints) {
		for (; mhints->ohismh_id < Dee_TMH_COUNT; ++mhints) {
			if (!oh_init_spec_mhint_canuse(mhints, self))
				continue;
			if (funptr == DeeType_GetPrivateMethodHint(self, self, mhints->ohismh_id))
				return self; /* Implemented via method hint within "self" */
		}
	}

	/* There doesn't seem to be anything special about "funptr".
	 * Iow: it appears to be a custom C implementation. As such,
	 *      check if it has been inherited from one of the type's
	 *      direct bases */
	iter = DeeTypeMRO_Init(&mro, self);
	while ((iter = DeeTypeMRO_NextDirectBase(&mro, iter)) != NULL) {
		Dee_funptr_t base_impl = DeeType_GetNativeOperatorWithoutInherit(iter, id);
		if (base_impl == funptr) {
			/* Operator was inherited from "iter"
			 * -> recursively see where *it* got the operator from. */
			return DeeType_GetNativeOperatorOrigin(iter, id);
		}

		/* The method hint may have gotten mapped during inherit -> check for that case. */
		base_impl = DeeType_MapTMHInTNOForInherit(iter, self, id, base_impl);
		if (base_impl == funptr)
			return DeeType_GetNativeOperatorOrigin(iter, id);
	}

	/* Not inherited -> this is where the operator originates from! */
	return self;
}




STATIC_ASSERT_MSG(Dee_TNO_COUNT <= (((unsigned int)1 << (sizeof(Dee_compact_tno_id_t) * __CHAR_BIT__)) - 1),
                  "Dee_compact_tno_id_t must be made larger");

INTERN_CONST Dee_compact_tno_id_t const _DeeType_GetTnoOfOperator[Dee_OPERATOR_USERCOUNT] = {
	/* [OPERATOR_0000_CONSTRUCTOR] = */ (Dee_compact_tno_id_t)Dee_TNO_COUNT,
	/* [OPERATOR_0001_COPY]        = */ (Dee_compact_tno_id_t)Dee_TNO_COUNT,
	/* [OPERATOR_0002_DEEPCOPY]    = */ (Dee_compact_tno_id_t)Dee_TNO_COUNT,
	/* [OPERATOR_0003_DESTRUCTOR]  = */ (Dee_compact_tno_id_t)Dee_TNO_COUNT,
	/* [OPERATOR_0004_ASSIGN]      = */ (Dee_compact_tno_id_t)Dee_TNO_assign,
	/* [OPERATOR_0005_MOVEASSIGN]  = */ (Dee_compact_tno_id_t)Dee_TNO_move_assign,
	/* [OPERATOR_0006_STR]         = */ (Dee_compact_tno_id_t)Dee_TNO_str,
	/* [OPERATOR_0007_REPR]        = */ (Dee_compact_tno_id_t)Dee_TNO_repr,
	/* [OPERATOR_0008_BOOL]        = */ (Dee_compact_tno_id_t)Dee_TNO_bool,
	/* [OPERATOR_0009_ITERNEXT]    = */ (Dee_compact_tno_id_t)Dee_TNO_iter_next,
	/* [OPERATOR_000A_CALL]        = */ (Dee_compact_tno_id_t)Dee_TNO_call,
	/* [OPERATOR_000B_INT]         = */ (Dee_compact_tno_id_t)Dee_TNO_int,
	/* [OPERATOR_000C_FLOAT]       = */ (Dee_compact_tno_id_t)Dee_TNO_double,
	/* [OPERATOR_000D_INV]         = */ (Dee_compact_tno_id_t)Dee_TNO_inv,
	/* [OPERATOR_000E_POS]         = */ (Dee_compact_tno_id_t)Dee_TNO_pos,
	/* [OPERATOR_000F_NEG]         = */ (Dee_compact_tno_id_t)Dee_TNO_neg,
	/* [OPERATOR_0010_ADD]         = */ (Dee_compact_tno_id_t)Dee_TNO_add,
	/* [OPERATOR_0011_SUB]         = */ (Dee_compact_tno_id_t)Dee_TNO_sub,
	/* [OPERATOR_0012_MUL]         = */ (Dee_compact_tno_id_t)Dee_TNO_mul,
	/* [OPERATOR_0013_DIV]         = */ (Dee_compact_tno_id_t)Dee_TNO_div,
	/* [OPERATOR_0014_MOD]         = */ (Dee_compact_tno_id_t)Dee_TNO_mod,
	/* [OPERATOR_0015_SHL]         = */ (Dee_compact_tno_id_t)Dee_TNO_shl,
	/* [OPERATOR_0016_SHR]         = */ (Dee_compact_tno_id_t)Dee_TNO_shr,
	/* [OPERATOR_0017_AND]         = */ (Dee_compact_tno_id_t)Dee_TNO_and,
	/* [OPERATOR_0018_OR]          = */ (Dee_compact_tno_id_t)Dee_TNO_or,
	/* [OPERATOR_0019_XOR]         = */ (Dee_compact_tno_id_t)Dee_TNO_xor,
	/* [OPERATOR_001A_POW]         = */ (Dee_compact_tno_id_t)Dee_TNO_pow,
	/* [OPERATOR_001B_INC]         = */ (Dee_compact_tno_id_t)Dee_TNO_inc,
	/* [OPERATOR_001C_DEC]         = */ (Dee_compact_tno_id_t)Dee_TNO_dec,
	/* [OPERATOR_001D_INPLACE_ADD] = */ (Dee_compact_tno_id_t)Dee_TNO_inplace_add,
	/* [OPERATOR_001E_INPLACE_SUB] = */ (Dee_compact_tno_id_t)Dee_TNO_inplace_sub,
	/* [OPERATOR_001F_INPLACE_MUL] = */ (Dee_compact_tno_id_t)Dee_TNO_inplace_mul,
	/* [OPERATOR_0020_INPLACE_DIV] = */ (Dee_compact_tno_id_t)Dee_TNO_inplace_div,
	/* [OPERATOR_0021_INPLACE_MOD] = */ (Dee_compact_tno_id_t)Dee_TNO_inplace_mod,
	/* [OPERATOR_0022_INPLACE_SHL] = */ (Dee_compact_tno_id_t)Dee_TNO_inplace_shl,
	/* [OPERATOR_0023_INPLACE_SHR] = */ (Dee_compact_tno_id_t)Dee_TNO_inplace_shr,
	/* [OPERATOR_0024_INPLACE_AND] = */ (Dee_compact_tno_id_t)Dee_TNO_inplace_and,
	/* [OPERATOR_0025_INPLACE_OR]  = */ (Dee_compact_tno_id_t)Dee_TNO_inplace_or,
	/* [OPERATOR_0026_INPLACE_XOR] = */ (Dee_compact_tno_id_t)Dee_TNO_inplace_xor,
	/* [OPERATOR_0027_INPLACE_POW] = */ (Dee_compact_tno_id_t)Dee_TNO_inplace_pow,
	/* [OPERATOR_0028_HASH]        = */ (Dee_compact_tno_id_t)Dee_TNO_hash,
	/* [OPERATOR_0029_EQ]          = */ (Dee_compact_tno_id_t)Dee_TNO_eq,
	/* [OPERATOR_002A_NE]          = */ (Dee_compact_tno_id_t)Dee_TNO_ne,
	/* [OPERATOR_002B_LO]          = */ (Dee_compact_tno_id_t)Dee_TNO_lo,
	/* [OPERATOR_002C_LE]          = */ (Dee_compact_tno_id_t)Dee_TNO_le,
	/* [OPERATOR_002D_GR]          = */ (Dee_compact_tno_id_t)Dee_TNO_gr,
	/* [OPERATOR_002E_GE]          = */ (Dee_compact_tno_id_t)Dee_TNO_ge,
	/* [OPERATOR_002F_ITER]        = */ (Dee_compact_tno_id_t)Dee_TNO_iter,
	/* [OPERATOR_0030_SIZE]        = */ (Dee_compact_tno_id_t)Dee_TNO_size,
	/* [OPERATOR_0031_CONTAINS]    = */ (Dee_compact_tno_id_t)Dee_TNO_contains,
	/* [OPERATOR_0032_GETITEM]     = */ (Dee_compact_tno_id_t)Dee_TNO_getitem,
	/* [OPERATOR_0033_DELITEM]     = */ (Dee_compact_tno_id_t)Dee_TNO_delitem,
	/* [OPERATOR_0034_SETITEM]     = */ (Dee_compact_tno_id_t)Dee_TNO_setitem,
	/* [OPERATOR_0035_GETRANGE]    = */ (Dee_compact_tno_id_t)Dee_TNO_getrange,
	/* [OPERATOR_0036_DELRANGE]    = */ (Dee_compact_tno_id_t)Dee_TNO_delrange,
	/* [OPERATOR_0037_SETRANGE]    = */ (Dee_compact_tno_id_t)Dee_TNO_setrange,
	/* [OPERATOR_0038_GETATTR]     = */ (Dee_compact_tno_id_t)Dee_TNO_getattr,
	/* [OPERATOR_0039_DELATTR]     = */ (Dee_compact_tno_id_t)Dee_TNO_delattr,
	/* [OPERATOR_003A_SETATTR]     = */ (Dee_compact_tno_id_t)Dee_TNO_setattr,
	/* [OPERATOR_003B_ENUMATTR]    = */ (Dee_compact_tno_id_t)Dee_TNO_COUNT,
	/* [OPERATOR_003C_ENTER]       = */ (Dee_compact_tno_id_t)Dee_TNO_enter,
	/* [OPERATOR_003D_LEAVE]       = */ (Dee_compact_tno_id_t)Dee_TNO_leave,
};

INTERN Dee_operator_t const _DeeType_GetOperatorOfTno[Dee_TNO_COUNT] = {
/* clang-format off */
/*[[[deemon (printGetOperatorOfTnoInit from "..method-hints.method-hints")();]]]*/
	/* [Dee_TNO_assign]                     = */ OPERATOR_ASSIGN,
	/* [Dee_TNO_move_assign]                = */ OPERATOR_MOVEASSIGN,
	/* [Dee_TNO_str]                        = */ OPERATOR_STR,
	/* [Dee_TNO_print]                      = */ OPERATOR_STR,
	/* [Dee_TNO_repr]                       = */ OPERATOR_REPR,
	/* [Dee_TNO_printrepr]                  = */ OPERATOR_REPR,
	/* [Dee_TNO_bool]                       = */ OPERATOR_BOOL,
	/* [Dee_TNO_call]                       = */ OPERATOR_CALL,
	/* [Dee_TNO_call_kw]                    = */ OPERATOR_CALL,
	/* [Dee_TNO_thiscall]                   = */ OPERATOR_CALL,
	/* [Dee_TNO_thiscall_kw]                = */ OPERATOR_CALL,
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* [Dee_TNO_call_tuple]                 = */ OPERATOR_CALL,
	/* [Dee_TNO_call_tuple_kw]              = */ OPERATOR_CALL,
	/* [Dee_TNO_thiscall_tuple]             = */ OPERATOR_CALL,
	/* [Dee_TNO_thiscall_tuple_kw]          = */ OPERATOR_CALL,
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
	/* [Dee_TNO_iter_next]                  = */ OPERATOR_ITERNEXT,
	/* [Dee_TNO_nextpair]                   = */ OPERATOR_ITERNEXT,
	/* [Dee_TNO_nextkey]                    = */ OPERATOR_ITERNEXT,
	/* [Dee_TNO_nextvalue]                  = */ OPERATOR_ITERNEXT,
	/* [Dee_TNO_advance]                    = */ OPERATOR_ITERNEXT,
	/* [Dee_TNO_int]                        = */ OPERATOR_INT,
	/* [Dee_TNO_int32]                      = */ OPERATOR_INT,
	/* [Dee_TNO_int64]                      = */ OPERATOR_INT,
	/* [Dee_TNO_double]                     = */ OPERATOR_FLOAT,
	/* [Dee_TNO_hash]                       = */ OPERATOR_HASH,
	/* [Dee_TNO_compare_eq]                 = */ OPERATOR_EQ,
	/* [Dee_TNO_compare]                    = */ OPERATOR_LO,
	/* [Dee_TNO_trycompare_eq]              = */ OPERATOR_EQ,
	/* [Dee_TNO_eq]                         = */ OPERATOR_EQ,
	/* [Dee_TNO_ne]                         = */ OPERATOR_NE,
	/* [Dee_TNO_lo]                         = */ OPERATOR_LO,
	/* [Dee_TNO_le]                         = */ OPERATOR_LE,
	/* [Dee_TNO_gr]                         = */ OPERATOR_GR,
	/* [Dee_TNO_ge]                         = */ OPERATOR_GE,
	/* [Dee_TNO_iter]                       = */ OPERATOR_ITER,
	/* [Dee_TNO_foreach]                    = */ OPERATOR_ITER,
	/* [Dee_TNO_foreach_pair]               = */ OPERATOR_ITER,
	/* [Dee_TNO_sizeob]                     = */ OPERATOR_SIZE,
	/* [Dee_TNO_size]                       = */ OPERATOR_SIZE,
	/* [Dee_TNO_size_fast]                  = */ OPERATOR_SIZE,
	/* [Dee_TNO_contains]                   = */ OPERATOR_CONTAINS,
	/* [Dee_TNO_getitem_index_fast]         = */ OPERATOR_USERCOUNT,
	/* [Dee_TNO_getitem]                    = */ OPERATOR_GETITEM,
	/* [Dee_TNO_getitem_index]              = */ OPERATOR_GETITEM,
	/* [Dee_TNO_getitem_string_hash]        = */ OPERATOR_GETITEM,
	/* [Dee_TNO_getitem_string_len_hash]    = */ OPERATOR_GETITEM,
	/* [Dee_TNO_trygetitem]                 = */ OPERATOR_GETITEM,
	/* [Dee_TNO_trygetitem_index]           = */ OPERATOR_GETITEM,
	/* [Dee_TNO_trygetitem_string_hash]     = */ OPERATOR_GETITEM,
	/* [Dee_TNO_trygetitem_string_len_hash] = */ OPERATOR_GETITEM,
	/* [Dee_TNO_bounditem]                  = */ OPERATOR_GETITEM,
	/* [Dee_TNO_bounditem_index]            = */ OPERATOR_GETITEM,
	/* [Dee_TNO_bounditem_string_hash]      = */ OPERATOR_GETITEM,
	/* [Dee_TNO_bounditem_string_len_hash]  = */ OPERATOR_GETITEM,
	/* [Dee_TNO_hasitem]                    = */ OPERATOR_GETITEM,
	/* [Dee_TNO_hasitem_index]              = */ OPERATOR_GETITEM,
	/* [Dee_TNO_hasitem_string_hash]        = */ OPERATOR_GETITEM,
	/* [Dee_TNO_hasitem_string_len_hash]    = */ OPERATOR_GETITEM,
	/* [Dee_TNO_delitem]                    = */ OPERATOR_DELITEM,
	/* [Dee_TNO_delitem_index]              = */ OPERATOR_DELITEM,
	/* [Dee_TNO_delitem_string_hash]        = */ OPERATOR_DELITEM,
	/* [Dee_TNO_delitem_string_len_hash]    = */ OPERATOR_DELITEM,
	/* [Dee_TNO_setitem]                    = */ OPERATOR_SETITEM,
	/* [Dee_TNO_setitem_index]              = */ OPERATOR_SETITEM,
	/* [Dee_TNO_setitem_string_hash]        = */ OPERATOR_SETITEM,
	/* [Dee_TNO_setitem_string_len_hash]    = */ OPERATOR_SETITEM,
	/* [Dee_TNO_getrange]                   = */ OPERATOR_GETRANGE,
	/* [Dee_TNO_getrange_index]             = */ OPERATOR_GETRANGE,
	/* [Dee_TNO_getrange_index_n]           = */ OPERATOR_GETRANGE,
	/* [Dee_TNO_delrange]                   = */ OPERATOR_DELRANGE,
	/* [Dee_TNO_delrange_index]             = */ OPERATOR_DELRANGE,
	/* [Dee_TNO_delrange_index_n]           = */ OPERATOR_DELRANGE,
	/* [Dee_TNO_setrange]                   = */ OPERATOR_SETRANGE,
	/* [Dee_TNO_setrange_index]             = */ OPERATOR_SETRANGE,
	/* [Dee_TNO_setrange_index_n]           = */ OPERATOR_SETRANGE,
	/* [Dee_TNO_inv]                        = */ OPERATOR_INV,
	/* [Dee_TNO_pos]                        = */ OPERATOR_POS,
	/* [Dee_TNO_neg]                        = */ OPERATOR_NEG,
	/* [Dee_TNO_add]                        = */ OPERATOR_ADD,
	/* [Dee_TNO_inplace_add]                = */ OPERATOR_INPLACE_ADD,
	/* [Dee_TNO_sub]                        = */ OPERATOR_SUB,
	/* [Dee_TNO_inplace_sub]                = */ OPERATOR_INPLACE_SUB,
	/* [Dee_TNO_mul]                        = */ OPERATOR_MUL,
	/* [Dee_TNO_inplace_mul]                = */ OPERATOR_INPLACE_MUL,
	/* [Dee_TNO_div]                        = */ OPERATOR_DIV,
	/* [Dee_TNO_inplace_div]                = */ OPERATOR_INPLACE_DIV,
	/* [Dee_TNO_mod]                        = */ OPERATOR_MOD,
	/* [Dee_TNO_inplace_mod]                = */ OPERATOR_INPLACE_MOD,
	/* [Dee_TNO_shl]                        = */ OPERATOR_SHL,
	/* [Dee_TNO_inplace_shl]                = */ OPERATOR_INPLACE_SHL,
	/* [Dee_TNO_shr]                        = */ OPERATOR_SHR,
	/* [Dee_TNO_inplace_shr]                = */ OPERATOR_INPLACE_SHR,
	/* [Dee_TNO_and]                        = */ OPERATOR_AND,
	/* [Dee_TNO_inplace_and]                = */ OPERATOR_INPLACE_AND,
	/* [Dee_TNO_or]                         = */ OPERATOR_OR,
	/* [Dee_TNO_inplace_or]                 = */ OPERATOR_INPLACE_OR,
	/* [Dee_TNO_xor]                        = */ OPERATOR_XOR,
	/* [Dee_TNO_inplace_xor]                = */ OPERATOR_INPLACE_XOR,
	/* [Dee_TNO_pow]                        = */ OPERATOR_POW,
	/* [Dee_TNO_inplace_pow]                = */ OPERATOR_INPLACE_POW,
	/* [Dee_TNO_inc]                        = */ OPERATOR_INC,
	/* [Dee_TNO_dec]                        = */ OPERATOR_DEC,
	/* [Dee_TNO_enter]                      = */ OPERATOR_ENTER,
	/* [Dee_TNO_leave]                      = */ OPERATOR_LEAVE,
	/* [Dee_TNO_getattr]                    = */ OPERATOR_GETATTR,
	/* [Dee_TNO_getattr_string_hash]        = */ OPERATOR_GETATTR,
	/* [Dee_TNO_getattr_string_len_hash]    = */ OPERATOR_GETATTR,
	/* [Dee_TNO_boundattr]                  = */ OPERATOR_GETATTR,
	/* [Dee_TNO_boundattr_string_hash]      = */ OPERATOR_GETATTR,
	/* [Dee_TNO_boundattr_string_len_hash]  = */ OPERATOR_GETATTR,
	/* [Dee_TNO_hasattr]                    = */ OPERATOR_GETATTR,
	/* [Dee_TNO_hasattr_string_hash]        = */ OPERATOR_GETATTR,
	/* [Dee_TNO_hasattr_string_len_hash]    = */ OPERATOR_GETATTR,
	/* [Dee_TNO_delattr]                    = */ OPERATOR_DELATTR,
	/* [Dee_TNO_delattr_string_hash]        = */ OPERATOR_DELATTR,
	/* [Dee_TNO_delattr_string_len_hash]    = */ OPERATOR_DELATTR,
	/* [Dee_TNO_setattr]                    = */ OPERATOR_SETATTR,
	/* [Dee_TNO_setattr_string_hash]        = */ OPERATOR_SETATTR,
	/* [Dee_TNO_setattr_string_len_hash]    = */ OPERATOR_SETATTR,
/*[[[end]]]*/
/* clang-format on */
};

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_HINTS_C */
