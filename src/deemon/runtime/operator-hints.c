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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_HINTS_C
#define GUARD_DEEMON_RUNTIME_OPERATOR_HINTS_C 1

#include <deemon/api.h>
#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/alloc.h>
#include <deemon/format.h>
#include <deemon/method-hints.h>
#include <deemon/operator-hints.h>
#include <deemon/seq.h>
#include <deemon/util/atomic.h>

/**/
#include <hybrid/typecore.h>

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

struct oh_init_spec_impl {
	Dee_funptr_t           ohisi_impl; /* [1..1] Default impl (e.g. `default__seq_iter__with__seq_foreach') */
	__UINTPTR_HALF_TYPE__  ohisi_dep1; /* First dependent operator (or `>= Dee_TNO_COUNT' if unused) */
	__UINTPTR_HALF_TYPE__  ohisi_dep2; /* Second dependent operator (or `>= Dee_TNO_COUNT' if unused) */
};
#define OH_INIT_SPEC_IMPL_END { NULL, 0, 0 }
#define OH_INIT_SPEC_IMPL_INIT(ohisi_impl, ohisi_dep1, ohisi_dep2) \
	{                                                              \
		/* .ohisi_impl = */ (Dee_funptr_t)(ohisi_impl),            \
		/* .ohisi_dep1 = */ ohisi_dep1,                            \
		/* .ohisi_dep2 = */ ohisi_dep2                             \
	}

struct oh_init_spec_mhint {
	enum Dee_tmh_id ohismh_id;         /* ID of the method hint usable to implement this operator. */
	DeeTypeObject  *ohismh_implements; /* [0..1] Type that must be implemented for this hint to be usable. */
	unsigned int    ohismh_seqclass;   /* [valid_if(!miso_implements)] Required sequence class for this hint. (e.g. "Dee_SEQCLASS_SEQ") */
};

#define OH_INIT_SPEC_MHINT_END { Dee_TMH_COUNT, NULL, 0 }
#define OH_INIT_SPEC_MHINT_INIT(ohismh_id, ohismh_implements, ohismh_seqclass) \
	{                                                                          \
		/* .ohismh_id         = */ ohismh_id,                                  \
		/* .ohismh_implements = */ ohismh_implements,                          \
		/* .ohismh_seqclass   = */ ohismh_seqclass                             \
	}

struct oh_init_spec {
	__UINTPTR_HALF_TYPE__            ohis_table;  /* Offset into `DeeTypeObject' for the sub-table containing this operator, or `0' when part of the primary table. */
	__UINTPTR_HALF_TYPE__            ohis_field;  /* Offset into the sub-table (or root) where the operator's function pointer is located. */
	struct oh_init_spec_impl  const *ohis_impls;  /* [0..n] Default impls of this operator (terminated by `ohisi_impl == NULL') */
	struct oh_init_spec_mhint const *ohis_mhints; /* [0..n] Method hints that can be loaded into this operator (terminated by `ohismh_id >= Dee_TMH_COUNT') */
};
#define OH_INIT_SPEC_INIT(ohis_table, ohis_field, ohis_impls, ohis_mhints) \
	{                                                                      \
		/* .ohis_table  = */ ohis_table,                                   \
		/* .ohis_field  = */ ohis_field,                                   \
		/* .ohis_impls  = */ ohis_impls,                                   \
		/* .ohis_mhints = */ ohis_mhints                                   \
	}


INTDEF struct oh_init_spec tpconst oh_init_specs[Dee_TNO_COUNT];

/* clang-format off */
/*[[[deemon (printNativeOperatorHintSpecs from "..method-hints.method-hints")();]]]*/
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_assign[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_assign, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_assign, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_assign, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_move_assign[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__move_assign__with__assign, Dee_TNO_assign, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_str[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__str__with__print, Dee_TNO_print, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_print[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__print__with__str, Dee_TNO_str, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_repr[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__repr__with__printrepr, Dee_TNO_printrepr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_printrepr[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__printrepr__with__repr, Dee_TNO_repr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_bool[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_bool, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_bool, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_bool, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_call[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__call__with__call_kw, Dee_TNO_call_kw, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_call_kw[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__call_kw__with__call, Dee_TNO_call, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
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
PRIVATE struct oh_init_spec_impl tpconst oh_impls_double[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__double__with__int, Dee_TNO_int, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__double__with__int64, Dee_TNO_int64, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__double__with__int32, Dee_TNO_int32, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_hash[4] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_hash, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_hash, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_compare_eq[6] = {
	OH_INIT_SPEC_IMPL_INIT(&default__compare_eq__with__compare, Dee_TNO_compare, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__compare_eq__with__eq, Dee_TNO_eq, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__compare_eq__with__ne, Dee_TNO_ne, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__compare_eq__with__lo__and__gr, Dee_TNO_lo, Dee_TNO_gr),
	OH_INIT_SPEC_IMPL_INIT(&default__compare_eq__with__le__and__ge, Dee_TNO_le, Dee_TNO_ge),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_compare_eq[5] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_compare_eq, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_compare_eq, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_compare_eq, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_compare_eq, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
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
PRIVATE struct oh_init_spec_impl tpconst oh_impls_trycompare_eq[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__trycompare_eq__with__compare_eq, Dee_TNO_compare_eq, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_trycompare_eq[5] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_trycompare_eq, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_trycompare_eq, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_trycompare_eq, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_trycompare_eq, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_eq[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__eq__with__ne, Dee_TNO_ne, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__eq__with__compare_eq, Dee_TNO_compare_eq, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_eq[5] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_eq, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_eq, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_eq, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_eq, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_ne[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__ne__with__eq, Dee_TNO_eq, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__ne__with__compare_eq, Dee_TNO_compare_eq, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_ne[5] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_ne, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_ne, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_ne, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_ne, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
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
PRIVATE struct oh_init_spec_impl tpconst oh_impls_iter[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__iter__with__foreach, Dee_TNO_foreach, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__iter__with__foreach_pair, Dee_TNO_foreach_pair, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_iter[6] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_iter, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_iter, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_iter, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_iter, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_iter, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_foreach[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__foreach__with__iter, Dee_TNO_iter, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__foreach__with__foreach_pair, Dee_TNO_foreach_pair, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_foreach[6] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_foreach, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_foreach, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_foreach, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_foreach, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_foreach, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_foreach_pair[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__foreach_pair__with__foreach, Dee_TNO_foreach, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__foreach_pair__with__iter, Dee_TNO_iter, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_foreach_pair[7] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_foreach_pair, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_foreach_pair, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_foreach_pair, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_foreach_pair, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_foreach_pair, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_enumerate, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
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
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_sizeob, NULL, Dee_SEQCLASS_MAP),
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
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_size, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_size_fast[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__size_fast__with__, Dee_TNO_COUNT, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_contains[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_contains, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_contains, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_getitem[7] = {
	OH_INIT_SPEC_IMPL_INIT(&default__getitem__with__trygetitem, Dee_TNO_trygetitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem__with__getitem_index__and__getitem_string_len_hash, Dee_TNO_getitem_index, Dee_TNO_getitem_string_len_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem__with__getitem_index__and__getitem_string_hash, Dee_TNO_getitem_index, Dee_TNO_getitem_string_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem__with__getitem_index, Dee_TNO_getitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem__with__getitem_string_len_hash, Dee_TNO_getitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem__with__getitem_string_hash, Dee_TNO_getitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_getitem[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_getitem, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_getitem, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_trygetitem[7] = {
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem__with__getitem, Dee_TNO_getitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem__with__trygetitem_index__and__trygetitem_string_len_hash, Dee_TNO_trygetitem_index, Dee_TNO_trygetitem_string_len_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem__with__trygetitem_index__and__trygetitem_string_hash, Dee_TNO_trygetitem_index, Dee_TNO_trygetitem_string_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem__with__trygetitem_index, Dee_TNO_trygetitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem__with__trygetitem_string_len_hash, Dee_TNO_trygetitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem__with__trygetitem_string_hash, Dee_TNO_trygetitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_trygetitem[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_trygetitem, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_trygetitem, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_getitem_index[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_index__with__size__and__getitem_index_fast, Dee_TNO_size, Dee_TNO_getitem_index_fast),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_index__with__trygetitem_index, Dee_TNO_trygetitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_index__with__getitem, Dee_TNO_getitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_getitem_index[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_getitem_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_getitem_index, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_trygetitem_index[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_index__with__size__and__getitem_index_fast, Dee_TNO_size, Dee_TNO_getitem_index_fast),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_index__with__getitem_index, Dee_TNO_getitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_index__with__trygetitem, Dee_TNO_trygetitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_trygetitem_index[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_trygetitem_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_trygetitem_index, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_getitem_string_hash[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_string_hash__with__trygetitem_string_hash, Dee_TNO_trygetitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_string_hash__with__getitem, Dee_TNO_getitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_getitem_string_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_getitem_string_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_trygetitem_string_hash[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_string_hash__with__getitem_string_hash, Dee_TNO_getitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_string_hash__with__trygetitem, Dee_TNO_trygetitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_trygetitem_string_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_trygetitem_string_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_getitem_string_len_hash[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_string_len_hash__with__trygetitem_string_len_hash, Dee_TNO_trygetitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_string_len_hash__with__getitem, Dee_TNO_getitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__getitem_string_len_hash__with__getitem_string_hash, Dee_TNO_getitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_getitem_string_len_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_getitem_string_len_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_trygetitem_string_len_hash[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_string_len_hash__with__getitem_string_len_hash, Dee_TNO_getitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_string_len_hash__with__trygetitem, Dee_TNO_trygetitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__trygetitem_string_len_hash__with__trygetitem_string_hash, Dee_TNO_trygetitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_trygetitem_string_len_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_trygetitem_string_len_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_bounditem[10] = {
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem__with__size__and__getitem_index_fast, Dee_TNO_size, Dee_TNO_getitem_index_fast),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem__with__getitem, Dee_TNO_getitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem__with__bounditem_index__and__bounditem_string_len_hash, Dee_TNO_bounditem_index, Dee_TNO_bounditem_string_len_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem__with__bounditem_index__and__bounditem_string_hash, Dee_TNO_bounditem_index, Dee_TNO_bounditem_string_hash),
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
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_index__with__bounditem, Dee_TNO_bounditem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_index__with__trygetitem_index__and__hasitem_index, Dee_TNO_trygetitem_index, Dee_TNO_hasitem_index),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_index__with__trygetitem_index, Dee_TNO_trygetitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_bounditem_index[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_bounditem_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_bounditem_index, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_bounditem_string_hash[5] = {
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_hash__with__getitem_string_hash, Dee_TNO_getitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_hash__with__bounditem, Dee_TNO_bounditem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_hash__with__trygetitem_string_hash__and__hasitem_string_hash, Dee_TNO_trygetitem_string_hash, Dee_TNO_hasitem_string_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_hash__with__trygetitem_string_hash, Dee_TNO_trygetitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_bounditem_string_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_bounditem_string_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_bounditem_string_len_hash[6] = {
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_len_hash__with__getitem_string_len_hash, Dee_TNO_getitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_len_hash__with__bounditem, Dee_TNO_bounditem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_len_hash__with__trygetitem_string_len_hash__and__hasitem_string_len_hash, Dee_TNO_trygetitem_string_len_hash, Dee_TNO_hasitem_string_len_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_len_hash__with__trygetitem_string_len_hash, Dee_TNO_trygetitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__bounditem_string_len_hash__with__bounditem_string_hash, Dee_TNO_bounditem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_bounditem_string_len_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_bounditem_string_len_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_hasitem[9] = {
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__size__and__getitem_index_fast, Dee_TNO_size, Dee_TNO_getitem_index_fast),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__trygetitem, Dee_TNO_trygetitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__bounditem, Dee_TNO_bounditem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__hasitem_index__and__hasitem_string_len_hash, Dee_TNO_hasitem_index, Dee_TNO_hasitem_string_len_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__hasitem_index__and__hasitem_string_hash, Dee_TNO_hasitem_index, Dee_TNO_hasitem_string_hash),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__hasitem_index, Dee_TNO_hasitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__hasitem_string_len_hash, Dee_TNO_hasitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem__with__hasitem_string_hash, Dee_TNO_hasitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_hasitem[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_hasitem, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_hasitem, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_hasitem_index[5] = {
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_index__with__size__and__getitem_index_fast, Dee_TNO_size, Dee_TNO_getitem_index_fast),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_index__with__trygetitem_index, Dee_TNO_trygetitem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_index__with__bounditem_index, Dee_TNO_bounditem_index, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_index__with__hasitem, Dee_TNO_hasitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_hasitem_index[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_hasitem_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_hasitem_index, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_hasitem_string_hash[4] = {
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_string_hash__with__trygetitem_string_hash, Dee_TNO_trygetitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_string_hash__with__bounditem_string_hash, Dee_TNO_bounditem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_string_hash__with__hasitem, Dee_TNO_hasitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_hasitem_string_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_hasitem_string_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_hasitem_string_len_hash[5] = {
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_string_len_hash__with__trygetitem_string_len_hash, Dee_TNO_trygetitem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_string_len_hash__with__bounditem_string_len_hash, Dee_TNO_bounditem_string_len_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_string_len_hash__with__hasitem, Dee_TNO_hasitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__hasitem_string_len_hash__with__hasitem_string_hash, Dee_TNO_hasitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_hasitem_string_len_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_hasitem_string_len_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
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
	OH_INIT_SPEC_IMPL_INIT(&default__delitem_index__with__delitem, Dee_TNO_delitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_delitem_index[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_delitem_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_delitem_index, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_delitem_string_hash[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__delitem_string_hash__with__delitem, Dee_TNO_delitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_delitem_string_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_delitem_string_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_delitem_string_len_hash[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__delitem_string_len_hash__with__delitem, Dee_TNO_delitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__delitem_string_len_hash__with__delitem_string_hash, Dee_TNO_delitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_delitem_string_len_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_delitem_string_len_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
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
	OH_INIT_SPEC_IMPL_INIT(&default__setitem_index__with__setitem, Dee_TNO_setitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_setitem_index[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_setitem_index, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_setitem_index, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_setitem_string_hash[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__setitem_string_hash__with__setitem, Dee_TNO_setitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_setitem_string_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_setitem_string_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_setitem_string_len_hash[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__setitem_string_len_hash__with__setitem, Dee_TNO_setitem, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__setitem_string_len_hash__with__setitem_string_hash, Dee_TNO_setitem_string_hash, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_setitem_string_len_hash[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_setitem_string_len_hash, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
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
PRIVATE struct oh_init_spec_impl tpconst oh_impls_getrange_index_n[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__getrange_index_n__with__getrange, Dee_TNO_getrange, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_getrange_index_n[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_getrange_index_n, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_END
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
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_inv[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_inv, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_add[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_add, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_add, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
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
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_sub[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_sub, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_sub, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
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
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_mul[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_mul__with__mul, Dee_TNO_mul, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_inplace_mul[2] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_seq_operator_inplace_mul, NULL, Dee_SEQCLASS_SEQ),
	OH_INIT_SPEC_MHINT_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_div[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_div__with__div, Dee_TNO_div, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_mod[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_mod__with__mod, Dee_TNO_mod, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_shl[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_shl__with__shl, Dee_TNO_shl, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_shr[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_shr__with__shr, Dee_TNO_shr, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_and[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_and, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_and, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
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
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_or[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_or__with__or, Dee_TNO_or, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_mhint tpconst oh_mhints_xor[3] = {
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_set_operator_xor, NULL, Dee_SEQCLASS_SET),
	OH_INIT_SPEC_MHINT_INIT(Dee_TMH_map_operator_xor, NULL, Dee_SEQCLASS_MAP),
	OH_INIT_SPEC_MHINT_END
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
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_pow[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_pow__with__pow, Dee_TNO_pow, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inc[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inc__with__inplace_add, Dee_TNO_inplace_add, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__inc__with__add, Dee_TNO_add, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_dec[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__dec__with__inplace_sub, Dee_TNO_inplace_sub, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__dec__with__sub, Dee_TNO_sub, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_enter[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__enter__with__leave, Dee_TNO_leave, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_leave[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__leave__with__enter, Dee_TNO_enter, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
INTERN_TPCONST struct oh_init_spec tpconst oh_init_specs[98] = {
	/* tp_init.tp_assign                     */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_init.tp_assign), NULL, oh_mhints_assign),
	/* tp_init.tp_move_assign                */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_init.tp_move_assign), oh_impls_move_assign, NULL),
	/* tp_cast.tp_str                        */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_str), oh_impls_str, NULL),
	/* tp_cast.tp_print                      */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_print), oh_impls_print, NULL),
	/* tp_cast.tp_repr                       */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_repr), oh_impls_repr, NULL),
	/* tp_cast.tp_printrepr                  */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_printrepr), oh_impls_printrepr, NULL),
	/* tp_cast.tp_bool                       */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_bool), NULL, oh_mhints_bool),
	/* tp_call                               */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_call), oh_impls_call, NULL),
	/* tp_call_kw                            */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_call_kw), oh_impls_call_kw, NULL),
	/* tp_iter_next                          */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_iter_next), oh_impls_iter_next, NULL),
	/* tp_iterator->tp_nextpair              */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_iterator), offsetof(struct type_iterator, tp_nextpair), oh_impls_nextpair, NULL),
	/* tp_iterator->tp_nextkey               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_iterator), offsetof(struct type_iterator, tp_nextkey), oh_impls_nextkey, NULL),
	/* tp_iterator->tp_nextvalue             */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_iterator), offsetof(struct type_iterator, tp_nextvalue), oh_impls_nextvalue, NULL),
	/* tp_iterator->tp_advance               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_iterator), offsetof(struct type_iterator, tp_advance), oh_impls_advance, NULL),
	/* tp_math->tp_int                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_int), oh_impls_int, NULL),
	/* tp_math->tp_int32                     */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_int32), oh_impls_int32, NULL),
	/* tp_math->tp_int64                     */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_int64), oh_impls_int64, NULL),
	/* tp_math->tp_double                    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_double), oh_impls_double, NULL),
	/* tp_cmp->tp_hash                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_hash), NULL, oh_mhints_hash),
	/* tp_cmp->tp_compare_eq                 */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_compare_eq), oh_impls_compare_eq, oh_mhints_compare_eq),
	/* tp_cmp->tp_compare                    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_compare), oh_impls_compare, oh_mhints_compare),
	/* tp_cmp->tp_trycompare_eq              */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_trycompare_eq), oh_impls_trycompare_eq, oh_mhints_trycompare_eq),
	/* tp_cmp->tp_eq                         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_eq), oh_impls_eq, oh_mhints_eq),
	/* tp_cmp->tp_ne                         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_ne), oh_impls_ne, oh_mhints_ne),
	/* tp_cmp->tp_lo                         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_lo), oh_impls_lo, oh_mhints_lo),
	/* tp_cmp->tp_le                         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_le), oh_impls_le, oh_mhints_le),
	/* tp_cmp->tp_gr                         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_gr), oh_impls_gr, oh_mhints_gr),
	/* tp_cmp->tp_ge                         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_cmp), offsetof(struct type_cmp, tp_ge), oh_impls_ge, oh_mhints_ge),
	/* tp_seq->tp_iter                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_iter), oh_impls_iter, oh_mhints_iter),
	/* tp_seq->tp_foreach                    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_foreach), oh_impls_foreach, oh_mhints_foreach),
	/* tp_seq->tp_foreach_pair               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_foreach_pair), oh_impls_foreach_pair, oh_mhints_foreach_pair),
	/* tp_seq->tp_sizeob                     */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_sizeob), oh_impls_sizeob, oh_mhints_sizeob),
	/* tp_seq->tp_size                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_size), oh_impls_size, oh_mhints_size),
	/* tp_seq->tp_size_fast                  */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_size_fast), oh_impls_size_fast, NULL),
	/* tp_seq->tp_contains                   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_contains), NULL, oh_mhints_contains),
	/* tp_seq->tp_getitem                    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getitem), oh_impls_getitem, oh_mhints_getitem),
	/* tp_seq->tp_trygetitem                 */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_trygetitem), oh_impls_trygetitem, oh_mhints_trygetitem),
	/* tp_seq->tp_getitem_index_fast         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getitem_index_fast), NULL, NULL),
	/* tp_seq->tp_getitem_index              */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getitem_index), oh_impls_getitem_index, oh_mhints_getitem_index),
	/* tp_seq->tp_trygetitem_index           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_trygetitem_index), oh_impls_trygetitem_index, oh_mhints_trygetitem_index),
	/* tp_seq->tp_getitem_string_hash        */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getitem_string_hash), oh_impls_getitem_string_hash, oh_mhints_getitem_string_hash),
	/* tp_seq->tp_trygetitem_string_hash     */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_trygetitem_string_hash), oh_impls_trygetitem_string_hash, oh_mhints_trygetitem_string_hash),
	/* tp_seq->tp_getitem_string_len_hash    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getitem_string_len_hash), oh_impls_getitem_string_len_hash, oh_mhints_getitem_string_len_hash),
	/* tp_seq->tp_trygetitem_string_len_hash */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_trygetitem_string_len_hash), oh_impls_trygetitem_string_len_hash, oh_mhints_trygetitem_string_len_hash),
	/* tp_seq->tp_bounditem                  */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_bounditem), oh_impls_bounditem, oh_mhints_bounditem),
	/* tp_seq->tp_bounditem_index            */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_bounditem_index), oh_impls_bounditem_index, oh_mhints_bounditem_index),
	/* tp_seq->tp_bounditem_string_hash      */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_bounditem_string_hash), oh_impls_bounditem_string_hash, oh_mhints_bounditem_string_hash),
	/* tp_seq->tp_bounditem_string_len_hash  */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_bounditem_string_len_hash), oh_impls_bounditem_string_len_hash, oh_mhints_bounditem_string_len_hash),
	/* tp_seq->tp_hasitem                    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_hasitem), oh_impls_hasitem, oh_mhints_hasitem),
	/* tp_seq->tp_hasitem_index              */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_hasitem_index), oh_impls_hasitem_index, oh_mhints_hasitem_index),
	/* tp_seq->tp_hasitem_string_hash        */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_hasitem_string_hash), oh_impls_hasitem_string_hash, oh_mhints_hasitem_string_hash),
	/* tp_seq->tp_hasitem_string_len_hash    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_hasitem_string_len_hash), oh_impls_hasitem_string_len_hash, oh_mhints_hasitem_string_len_hash),
	/* tp_seq->tp_delitem                    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_delitem), oh_impls_delitem, oh_mhints_delitem),
	/* tp_seq->tp_delitem_index              */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_delitem_index), oh_impls_delitem_index, oh_mhints_delitem_index),
	/* tp_seq->tp_delitem_string_hash        */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_delitem_string_hash), oh_impls_delitem_string_hash, oh_mhints_delitem_string_hash),
	/* tp_seq->tp_delitem_string_len_hash    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_delitem_string_len_hash), oh_impls_delitem_string_len_hash, oh_mhints_delitem_string_len_hash),
	/* tp_seq->tp_setitem                    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_setitem), oh_impls_setitem, oh_mhints_setitem),
	/* tp_seq->tp_setitem_index              */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_setitem_index), oh_impls_setitem_index, oh_mhints_setitem_index),
	/* tp_seq->tp_setitem_string_hash        */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_setitem_string_hash), oh_impls_setitem_string_hash, oh_mhints_setitem_string_hash),
	/* tp_seq->tp_setitem_string_len_hash    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_setitem_string_len_hash), oh_impls_setitem_string_len_hash, oh_mhints_setitem_string_len_hash),
	/* tp_seq->tp_getrange                   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getrange), oh_impls_getrange, oh_mhints_getrange),
	/* tp_seq->tp_getrange_index             */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getrange_index), oh_impls_getrange_index, oh_mhints_getrange_index),
	/* tp_seq->tp_getrange_index_n           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_getrange_index_n), oh_impls_getrange_index_n, oh_mhints_getrange_index_n),
	/* tp_seq->tp_delrange                   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_delrange), oh_impls_delrange, oh_mhints_delrange),
	/* tp_seq->tp_delrange_index             */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_delrange_index), oh_impls_delrange_index, oh_mhints_delrange_index),
	/* tp_seq->tp_delrange_index_n           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_delrange_index_n), oh_impls_delrange_index_n, oh_mhints_delrange_index_n),
	/* tp_seq->tp_setrange                   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_setrange), oh_impls_setrange, oh_mhints_setrange),
	/* tp_seq->tp_setrange_index             */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_setrange_index), oh_impls_setrange_index, oh_mhints_setrange_index),
	/* tp_seq->tp_setrange_index_n           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_setrange_index_n), oh_impls_setrange_index_n, oh_mhints_setrange_index_n),
	/* tp_math->tp_inv                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inv), NULL, oh_mhints_inv),
	/* tp_math->tp_pos                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_pos), NULL, NULL),
	/* tp_math->tp_neg                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_neg), NULL, NULL),
	/* tp_math->tp_add                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_add), NULL, oh_mhints_add),
	/* tp_math->tp_inplace_add               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_add), oh_impls_inplace_add, oh_mhints_inplace_add),
	/* tp_math->tp_sub                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_sub), NULL, oh_mhints_sub),
	/* tp_math->tp_inplace_sub               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_sub), oh_impls_inplace_sub, oh_mhints_inplace_sub),
	/* tp_math->tp_mul                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_mul), NULL, NULL),
	/* tp_math->tp_inplace_mul               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_mul), oh_impls_inplace_mul, oh_mhints_inplace_mul),
	/* tp_math->tp_div                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_div), NULL, NULL),
	/* tp_math->tp_inplace_div               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_div), oh_impls_inplace_div, NULL),
	/* tp_math->tp_mod                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_mod), NULL, NULL),
	/* tp_math->tp_inplace_mod               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_mod), oh_impls_inplace_mod, NULL),
	/* tp_math->tp_shl                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_shl), NULL, NULL),
	/* tp_math->tp_inplace_shl               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_shl), oh_impls_inplace_shl, NULL),
	/* tp_math->tp_shr                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_shr), NULL, NULL),
	/* tp_math->tp_inplace_shr               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_shr), oh_impls_inplace_shr, NULL),
	/* tp_math->tp_and                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_and), NULL, oh_mhints_and),
	/* tp_math->tp_inplace_and               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_and), oh_impls_inplace_and, oh_mhints_inplace_and),
	/* tp_math->tp_or                        */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_or), NULL, NULL),
	/* tp_math->tp_inplace_or                */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_or), oh_impls_inplace_or, NULL),
	/* tp_math->tp_xor                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_xor), NULL, oh_mhints_xor),
	/* tp_math->tp_inplace_xor               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_xor), oh_impls_inplace_xor, oh_mhints_inplace_xor),
	/* tp_math->tp_pow                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_pow), NULL, NULL),
	/* tp_math->tp_inplace_pow               */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_pow), oh_impls_inplace_pow, NULL),
	/* tp_math->tp_inc                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inc), oh_impls_inc, NULL),
	/* tp_math->tp_dec                       */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_dec), oh_impls_dec, NULL),
	/* tp_with->tp_enter                     */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_with), offsetof(struct type_with, tp_enter), oh_impls_enter, NULL),
	/* tp_with->tp_leave                     */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_with), offsetof(struct type_with, tp_leave), oh_impls_leave, NULL),
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
		/* TODO: Print these case switches dynamically */
	case offsetof(DeeTypeObject, tp_gc):
		return sizeof(struct type_gc);
	case offsetof(DeeTypeObject, tp_math):
		return sizeof(struct type_math);
	case offsetof(DeeTypeObject, tp_cmp):
		return sizeof(struct type_cmp);
	case offsetof(DeeTypeObject, tp_seq):
		return sizeof(struct type_seq);
	case offsetof(DeeTypeObject, tp_iterator):
		return sizeof(struct type_iterator);
	case offsetof(DeeTypeObject, tp_attr):
		return sizeof(struct type_attr);
	case offsetof(DeeTypeObject, tp_with):
		return sizeof(struct type_with);
	case offsetof(DeeTypeObject, tp_buffer):
		return sizeof(struct type_buffer);
	default:
#ifdef NDEBUG
		__builtin_unreachable();
#else /* NDEBUG */
		Dee_Fatalf("Bad operator table offset: %" PRFuSIZ,
		           (size_t)offsetof_table);
		break;
#endif /* !NDEBUG */
	}
}

PRIVATE WUNUSED byte_t *DCALL
type_tno_tryalloc_table(__UINTPTR_HALF_TYPE__ offsetof_table) {
	size_t size = type_tno_sizeof_table(offsetof_table);
	return (byte_t *)Dee_UntrackAlloc(Dee_TryCalloc(size));
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
				table = new_table;
			}
			ASSERT(table);
		}
	}
	table += specs->ohis_field;
	atomic_write((Dee_funptr_t *)table, value);
	return true;
}


PRIVATE WUNUSED NONNULL((1)) Dee_funptr_t DCALL
type_tno_has_nondefault(DeeTypeObject const *__restrict self, enum Dee_tno_id id) {
	Dee_funptr_t result;
	struct oh_init_spec const *specs = &oh_init_specs[id];
	byte_t const *table = (byte_t const *)self;
	if (specs->ohis_table != 0) {
		table += specs->ohis_table;
		table = atomic_read((byte_t **)table);
		if (!table)
			return NULL;
	}
	table += specs->ohis_field;
	result = atomic_read((Dee_funptr_t *)table);
	if (result && specs->ohis_impls) {
		/* Check if "result" appears in "specs->ohis_impls" */
		struct oh_init_spec_impl const *iter;
		for (iter = specs->ohis_impls; iter->ohisi_impl; ++iter) {
			if (result == iter->ohisi_impl)
				return NULL;
		}
	}
	return result;
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
INTERN WUNUSED NONNULL((1)) size_t
(DCALL DeeType_SelectMissingNativeOperator)(DeeTypeObject const *__restrict self, enum Dee_tno_id id,
                                            struct Dee_tno_assign actions[Dee_TNO_ASSIGN_MAXLEN]) {
	size_t i, result;
	struct oh_init_spec const *specs = &oh_init_specs[id];
	struct oh_init_spec_impl const *impls = specs->ohis_impls;
	if unlikely(!impls)
		return 0;

	/* Step #1: prefer those with dependencies that are all: not IS_OPERATOR_HINT_DEAFULT_IMPL */
	for (i = 0; impls[i].ohisi_impl; ++i) {
		struct oh_init_spec_impl const *impl = &impls[i];
		if (impl->ohisi_dep1 < Dee_TNO_COUNT &&
		    !type_tno_has_nondefault(self, (enum Dee_tno_id)impl->ohisi_dep1))
			continue;
		if (impl->ohisi_dep2 < Dee_TNO_COUNT &&
		    !type_tno_has_nondefault(self, (enum Dee_tno_id)impl->ohisi_dep2))
			continue;
		/* Found an impl where all dependencies are provided natively -> use it! */
		actions[0].tnoa_id = id;
		actions[0].tnoa_cb = impl->ohisi_impl;
		return 1;
	}

	/* Step #2: prefer those that were defined first in /src/method-hints/*.h
	 * For this case, we must also account for transitive dependencies. */
#if Dee_TNO_ASSIGN_MAXLEN > 1
	for (i = 0; impls[i].ohisi_impl; ++i) {
		size_t result = 1;
		struct oh_init_spec_impl const *impl = &impls[i];
		ASSERTF(impl->ohisi_dep1 < Dee_TNO_COUNT || impl->ohisi_dep2 < Dee_TNO_COUNT,
		        "The no-dependencies case should have been handled above");
		if (impl->ohisi_dep1 < Dee_TNO_COUNT &&
		    !type_tno_has_nondefault(self, (enum Dee_tno_id)impl->ohisi_dep1)) {
			size_t dep_n_actions;
			dep_n_actions = DeeType_SelectMissingNativeOperator(self,
			                                                    (enum Dee_tno_id)impl->ohisi_dep1,
			                                                    actions + result);
			if (!dep_n_actions)
				continue;
			result += dep_n_actions;
		}
		if (impl->ohisi_dep2 < Dee_TNO_COUNT &&
		    !type_tno_has_nondefault(self, (enum Dee_tno_id)impl->ohisi_dep2)) {
			size_t dep_n_actions;
			dep_n_actions = DeeType_SelectMissingNativeOperator(self,
			                                                    (enum Dee_tno_id)impl->ohisi_dep2,
			                                                    actions + result);
			if (!dep_n_actions)
				continue;
			result += dep_n_actions;
		}

		/* All dependencies are being fulfilled -> use this impl! */
		ASSERTF(result >= 2, "At least 2 actions, else the first loop should have handled this one");
		actions[0].tnoa_id = id;
		actions[0].tnoa_cb = impl->ohisi_impl;
		return result;
	}
#endif /* Dee_TNO_ASSIGN_MAXLEN > 1 */

	/* Fallback: no implementation possible */
	return 0;
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
			type_tno_tryset(self, id, actions[0].tnoa_cb);
			result = actions[0].tnoa_cb;
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) bool
(DCALL do_DeeType_InheritOperatorWithoutHints2)(DeeTypeObject *__restrict from,
                                                DeeTypeObject *__restrict into,
                                                enum Dee_tno_id id, Dee_funptr_t impl,
                                                bool first_call);
PRIVATE WUNUSED NONNULL((1, 2)) bool
(DCALL do_DeeType_InheritOperatorWithoutHints)(DeeTypeObject *__restrict from,
                                               DeeTypeObject *__restrict into,
                                               enum Dee_tno_id id) {
	Dee_funptr_t funptr = type_tno_get(from, id);
	ASSERTF(funptr, "Transitive operator '%u' used as dependency was not initialized in '%s'",
	        (unsigned int)id, from->tp_name);
	return do_DeeType_InheritOperatorWithoutHints2(from, into, id, funptr, false);
}

PRIVATE WUNUSED NONNULL((1, 2, 4)) bool
(DCALL do_DeeType_InheritOperatorWithoutHints2)(DeeTypeObject *__restrict from,
                                                DeeTypeObject *__restrict into,
                                                enum Dee_tno_id id, Dee_funptr_t impl,
                                                bool first_call) {
	struct oh_init_spec const *specs = &oh_init_specs[id];
	struct oh_init_spec_impl const *impls = specs->ohis_impls;
	ASSERT(from != into);
	ASSERT(DeeType_GetNativeOperatorWithoutHints(from, id) == impl);

	/* Check if the given "impl" is one of the defaults that has dependencies.
	 * If that is the case, then we must ensure that those dependencies are also
	 * inherited (if not already present in `into').
	 *
	 * We're allowed to assume that "impl" is the only correct impl, so there is
	 * no need to check if some other impl should be used instead (no other impl
	 * should be) */
	if (impls) {
		for (; impls->ohisi_impl; ++impls) {
			if (impls->ohisi_impl != impl)
				continue;

			/* Found the impl in question -> must now also inherit *its* dependencies */
			if (impls->ohisi_dep1 < Dee_TNO_COUNT &&
			    !type_tno_get(into, (enum Dee_tno_id)impls->ohisi_dep1) &&
			    !do_DeeType_InheritOperatorWithoutHints(from, into, (enum Dee_tno_id)impls->ohisi_dep1))
				return false;
			if (impls->ohisi_dep2 < Dee_TNO_COUNT &&
			    !type_tno_get(into, (enum Dee_tno_id)impls->ohisi_dep2) &&
			    !do_DeeType_InheritOperatorWithoutHints(from, into, (enum Dee_tno_id)impls->ohisi_dep2))
				return false;
			break;
		}
	}

	Dee_DPRINTF("[RT] Inherit operator '%u' from '%s' into '%s' [ptr: %p]\n",
	            (unsigned int)id, from->tp_name, into->tp_name, *(void **)&impl);
	/* It's OK if the last write fails (because we still hand the correct pointer
	 * to our caller), but it's not OK if any of the intermediate writes fail (as
	 * in that case, the returned callback would try to access unallocated memory) */
	return type_tno_tryset(into, id, impl) || first_call;
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
INTERN WUNUSED NONNULL((1, 2, 4)) bool
(DCALL DeeType_InheritOperatorWithoutHints)(DeeTypeObject *__restrict from,
                                            DeeTypeObject *__restrict into,
                                            enum Dee_tno_id id, Dee_funptr_t impl) {
	return do_DeeType_InheritOperatorWithoutHints2(from, into, id, impl, true);
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
				if (iter->ohismh_implements) {
					if (!DeeType_Implements(self, iter->ohismh_implements))
						continue;
				} else if (iter->ohismh_seqclass != Dee_SEQCLASS_UNKNOWN) {
					if (DeeType_GetSeqClass(self) != iter->ohismh_seqclass)
						continue;
				}

				/* Only check for method hints privately! */
				result = DeeType_GetPrivateMethodHint(self, self, iter->ohismh_id);
				if (result)
					break;
			}
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_InheritOperator)(DeeTypeObject *__restrict self, enum Dee_tno_id id) {
	DeeTypeMRO mro;
	DeeTypeObject *iter = DeeTypeMRO_Init(&mro, self);
	while ((iter = DeeTypeMRO_NextDirectBase(&mro, iter)) != NULL) {
		Dee_funptr_t result = DeeType_GetNativeOperatorWithoutUnsupported(iter, id);
		if (result) {
			/* Inherit this operator (even if it's a method hint). */
			/* XXX: Shouldn't we re-evaluate method hints here if encountered? */
			if unlikely(!DeeType_InheritOperatorWithoutHints(iter, self, id, result))
				return NULL;
			return result;
		}
	}
	return NULL;
}

/* Same as `DeeType_GetNativeOperatorWithoutInherit', but actually also does
 * the operator inherit part (meaning that this is the actual master-function
 * that's called when you invoke one of the standard operators whose callback
 * is currently set to "NULL" within its relevant type) */
INTERN WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperatorWithoutUnsupported)(DeeTypeObject *__restrict self, enum Dee_tno_id id) {
	Dee_funptr_t result = DeeType_GetNativeOperatorWithoutInherit(self, id);
	if (result == NULL)
		result = DeeType_InheritOperator(self, id);
	return result;
}

/* Same as `DeeType_GetNativeOperatorWithoutUnsupported()', but never returns NULL
 * (for any operator linked against a deemon user-code ID (e.g. "OPERATOR_ITER")
 * and instead returns special implementations for each operator that simply call
 * `err_unimplemented_operator()' with the relevant arguments, before returning
 * whatever is indicative of an error in the context of the native operator. */
INTERN WUNUSED NONNULL((1)) Dee_funptr_t
(DCALL DeeType_GetNativeOperator)(DeeTypeObject *__restrict self, enum Dee_tno_id id) {
	Dee_funptr_t result = DeeType_GetNativeOperatorWithoutUnsupported(self, id);
	if unlikely(!result) /* Note how we don't write this impl back to "self" */
		result = DeeType_GetNativeOperatorUnsupported(id);
	return result;
}


DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_HINTS_C */
