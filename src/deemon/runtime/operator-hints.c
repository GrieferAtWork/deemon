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
#include <deemon/operator-hints.h>

/**/
#include <hybrid/typecore.h>

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

struct oh_init_spec {
	__UINTPTR_HALF_TYPE__           ohis_table; /* Offset into `DeeTypeObject' for the sub-table containing this operator, or `0' when part of the primary table. */
	__UINTPTR_HALF_TYPE__           ohis_field; /* Offset into the sub-table (or root) where the operator's function pointer is located. */
	struct oh_init_spec_impl const *ohis_impls; /* [0..n] Default impls of this operator (terminated by `ohisi_impl == NULL') */
	/* TODO: Specs describing which method hints can be loaded into this operator,
	 *       as well as under which circumstances each of those methods hints is
	 *       applicable (s.a. the new operator inherit concept in "class.h")
	 *       e.g. `Dee_TNO_foreach_pair' can inherit from:
	 *        - `Dee_TMH_set_operator_foreach_pair'
	 *        - `Dee_TMH_seq_operator_foreach_pair'
	 *       and in that order (iow: only inherit from seq if inherit from set isn't
	 *       possible due to the type not having a set- or map- sequence class)
	 */
};
#define OH_INIT_SPEC_INIT(ohis_table, ohis_field, ohis_impls) \
	{                                                         \
		/* .ohis_table = */ ohis_table,                       \
		/* .ohis_field = */ ohis_field,                       \
		/* .ohis_impls = */ ohis_impls                        \
	}


INTDEF struct oh_init_spec tpconst oh_init_specs[Dee_TNO_COUNT];

/* clang-format off */
/*[[[deemon (printNativeOperatorHintSpecs from "..method-hints.method-hints")();]]]*/
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
PRIVATE struct oh_init_spec_impl tpconst oh_impls_iter[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__iter__with__foreach, Dee_TNO_foreach, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__iter__with__foreach_pair, Dee_TNO_foreach_pair, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_foreach[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__foreach__with__iter, Dee_TNO_iter, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__foreach__with__foreach_pair, Dee_TNO_foreach_pair, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_foreach_pair[3] = {
	OH_INIT_SPEC_IMPL_INIT(&default__foreach_pair__with__iter, Dee_TNO_iter, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_INIT(&default__foreach_pair__with__foreach, Dee_TNO_foreach, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_add[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_add__with__add, Dee_TNO_add, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_sub[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_sub__with__sub, Dee_TNO_sub, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_mul[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_mul__with__mul, Dee_TNO_mul, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
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
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_and[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_and__with__and, Dee_TNO_and, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_or[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_or__with__or, Dee_TNO_or, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
};
PRIVATE struct oh_init_spec_impl tpconst oh_impls_inplace_xor[2] = {
	OH_INIT_SPEC_IMPL_INIT(&default__inplace_xor__with__xor, Dee_TNO_xor, Dee_TNO_COUNT),
	OH_INIT_SPEC_IMPL_END
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
INTERN_TPCONST struct oh_init_spec tpconst oh_init_specs[48] = {
	/* tp_cast.tp_str            */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_str), oh_impls_str),
	/* tp_cast.tp_print          */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_print), oh_impls_print),
	/* tp_cast.tp_repr           */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_repr), oh_impls_repr),
	/* tp_cast.tp_printrepr      */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_printrepr), oh_impls_printrepr),
	/* tp_cast.tp_bool           */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_cast.tp_bool), NULL),
	/* tp_call                   */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_call), oh_impls_call),
	/* tp_call_kw                */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_call_kw), oh_impls_call_kw),
	/* tp_iter_next              */ OH_INIT_SPEC_INIT(0, offsetof(DeeTypeObject, tp_iter_next), oh_impls_iter_next),
	/* tp_iterator->tp_nextpair  */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_iterator), offsetof(struct type_iterator, tp_nextpair), oh_impls_nextpair),
	/* tp_iterator->tp_nextkey   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_iterator), offsetof(struct type_iterator, tp_nextkey), oh_impls_nextkey),
	/* tp_iterator->tp_nextvalue */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_iterator), offsetof(struct type_iterator, tp_nextvalue), oh_impls_nextvalue),
	/* tp_iterator->tp_advance   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_iterator), offsetof(struct type_iterator, tp_advance), oh_impls_advance),
	/* tp_math->tp_int           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_int), oh_impls_int),
	/* tp_math->tp_int32         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_int32), oh_impls_int32),
	/* tp_math->tp_int64         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_int64), oh_impls_int64),
	/* tp_math->tp_double        */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_double), oh_impls_double),
	/* tp_seq->tp_iter           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_iter), oh_impls_iter),
	/* tp_seq->tp_foreach        */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_foreach), oh_impls_foreach),
	/* tp_seq->tp_foreach_pair   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_seq), offsetof(struct type_seq, tp_foreach_pair), oh_impls_foreach_pair),
	/* tp_math->tp_inv           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inv), NULL),
	/* tp_math->tp_pos           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_pos), NULL),
	/* tp_math->tp_neg           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_neg), NULL),
	/* tp_math->tp_add           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_add), NULL),
	/* tp_math->tp_inplace_add   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_add), oh_impls_inplace_add),
	/* tp_math->tp_sub           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_sub), NULL),
	/* tp_math->tp_inplace_sub   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_sub), oh_impls_inplace_sub),
	/* tp_math->tp_mul           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_mul), NULL),
	/* tp_math->tp_inplace_mul   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_mul), oh_impls_inplace_mul),
	/* tp_math->tp_div           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_div), NULL),
	/* tp_math->tp_inplace_div   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_div), oh_impls_inplace_div),
	/* tp_math->tp_mod           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_mod), NULL),
	/* tp_math->tp_inplace_mod   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_mod), oh_impls_inplace_mod),
	/* tp_math->tp_shl           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_shl), NULL),
	/* tp_math->tp_inplace_shl   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_shl), oh_impls_inplace_shl),
	/* tp_math->tp_shr           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_shr), NULL),
	/* tp_math->tp_inplace_shr   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_shr), oh_impls_inplace_shr),
	/* tp_math->tp_and           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_and), NULL),
	/* tp_math->tp_inplace_and   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_and), oh_impls_inplace_and),
	/* tp_math->tp_or            */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_or), NULL),
	/* tp_math->tp_inplace_or    */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_or), oh_impls_inplace_or),
	/* tp_math->tp_xor           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_xor), NULL),
	/* tp_math->tp_inplace_xor   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_xor), oh_impls_inplace_xor),
	/* tp_math->tp_pow           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_pow), NULL),
	/* tp_math->tp_inplace_pow   */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inplace_pow), oh_impls_inplace_pow),
	/* tp_math->tp_inc           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_inc), oh_impls_inc),
	/* tp_math->tp_dec           */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_math), offsetof(struct type_math, tp_dec), oh_impls_dec),
	/* tp_with->tp_enter         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_with), offsetof(struct type_with, tp_enter), oh_impls_enter),
	/* tp_with->tp_leave         */ OH_INIT_SPEC_INIT(offsetof(DeeTypeObject, tp_with), offsetof(struct type_with, tp_leave), oh_impls_leave),
};
/*[[[end]]]*/
/* clang-format on */


DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_HINTS_C */
