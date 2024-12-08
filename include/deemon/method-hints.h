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
#ifndef GUARD_DEEMON_METHOD_HINTS_H
#define GUARD_DEEMON_METHOD_HINTS_H 1

#include "api.h"

#include "object.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_type_method_hint type_method_hint
#endif /* DEE_SOURCE */

/*
 * Method hints are declaration pairs that come in the form of an entry in a
 * type's `tp_methods' array using a pre-defined function pointer (declared
 * in this header), as well as an entry in a type's `tp_method_hints' array,
 * which then points to the low-level C implementation of the function.
 *
 * These method hints are only available for certain, commonly overwritten,
 * default functions of objects, and are used to:
 * - Make it easier to override default functions without needing to re-write
 *   the same argument processing stub every time the function gets defined.
 * - Allow for fasting dispatching to the actual, underlying function within
 *   optimized _hostasm code.
 *
 *
 * >> PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
 * >> myob_setdefault(MyObject *self, DeeObject *key, DeeObject *value) {
 * >>     // This gets called by:
 * >>     // - `MyObject().setdefault(...)'
 * >>     // - `Mapping.setdefault(MyObject(), ...)'
 * >>     ...
 * >> }
 * >>
 * >> PRIVATE struct type_method_hint myob_method_hints[] = {
 * >>     TYPE_METHOD_HINT(map_setdefault, (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&myob_setdefault),
 * >>     TYPE_METHOD_HINT_END
 * >> };
 * >>
 * >> PRIVATE struct type_method myob_methods[] = {
 * >>     TYPE_METHOD_HINTREF(map_setdefault),
 * >>     TYPE_METHOD_END
 * >> };
 * >>
 * >> PRIVATE DeeTypeObject MyObject_Type = {
 * >>     ...
 * >>     .tp_methods      = myob_methods,
 * >>     .tp_method_hints = myob_method_hints,
 * >> };
 *
 */

enum Dee_tmh_id {
	/* !!! CAUTION !!! Method hint IDs are prone to arbitrarily change !!!
	 *
	 * Do not make use of these IDs if you're developing a DEX module and
	 * wish to remain compatible with the deemon core across many version. */
#define Dee_DEFINE_TYPE_METHOD_HINT_FUNC(attr, Treturn, cc, func_name, params) \
	Dee_TMH_##func_name,
#include "method-hints.def"
	Dee_TMH_COUNT
};

enum {
#define Dee_DEFINE_TYPE_METHOD_HINT_ATTR(attr_name, method_name, wrapper_flags, doc, wrapper_params) \
	_Dee_TMH_WRAPPER_FLAGS_##attr_name = wrapper_flags,
#include "method-hints.def"
};

#define Dee_DEFINE_TYPE_METHOD_HINT_FUNC(attr, Treturn, cc, func_name, params) \
	typedef Treturn (cc *Dee_mh_##func_name##_t) params;
#define Dee_DEFINE_TYPE_METHOD_HINT_ATTR(attr_name, method_name, wrapper_flags, doc, wrapper_params) \
	DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeMH_##attr_name) wrapper_params;           \
	DDATDEF char const DeeMH_##attr_name##_doc[];                                                    \
	DDATDEF char const DeeMH_##attr_name##_name[];
#include "method-hints.def"



/* Used to declare type method hints in C */
struct Dee_type_method_hint {
	enum Dee_tmh_id tmh_id;    /* Method hint ID (one of `Dee_TMH_*') */
	uint32_t        tmh_flags; /* Method flags (set of `Dee_METHOD_F*') */
	Dee_funptr_t    tmh_func;  /* [1..1] Method hint implementation (custom/type-specific) (NULL marks end-of-list) */
};

#ifdef __INTELLISENSE__
/* c++ magic to assert that the function pointers passed to `Dee_TYPE_METHOD_HINT_F'
 * and `Dee_TYPE_METHOD_HINT' are binary-compatible with whatever the resp. method
 * hint expects for its prototype (but note that pointer bases can be exchanged for
 * arbitrary types, meaning this doesn't fail if you use the real object types in
 * function parameters). */
extern "C++" {namespace __intern {
template<class T1, class T2> struct __PRIVATE_binary_compatible { enum{_value=false}; };
template<class T1, class T2> struct __PRIVATE_binary_compatible<T1 *, T2 *> { enum{_value=true}; };
template<class T> struct __PRIVATE_binary_compatible<T, T> { enum{_value=true}; };
template<> struct __PRIVATE_binary_compatible<void(), void()> { enum{_value=true}; };
template<class A1, class A2> struct __PRIVATE_binary_compatible<void(A1), void(A2)>: __PRIVATE_binary_compatible<A1, A2> { };
template<class A1, class... TARGS1, class A2, class... TARGS2>
struct __PRIVATE_binary_compatible<void(A1, TARGS1...), void(A2, TARGS2...)> {
	enum{_value = __PRIVATE_binary_compatible<A1, A2>::_value &&
	              __PRIVATE_binary_compatible<void(TARGS1...), void(TARGS2...)>::_value};
};
template<class T> struct __PRIVATE_match_method_hint { static Dee_funptr_t _match(T); };
template<class RT1, class... TARGS1> struct __PRIVATE_match_method_hint<RT1(DCALL *)(TARGS1...)> {
	template<class RT2, class... TARGS2>
	static ::__intern::____INTELLISENSE_enableif<
	::__intern::__PRIVATE_binary_compatible<RT1, RT2>::_value &&
	::__intern::__PRIVATE_binary_compatible<void(TARGS1...), void(TARGS2...)>::_value,
	Dee_funptr_t>::__type _match(RT2(DCALL *)(TARGS2...));
};
}}

#define Dee_TYPE_METHOD_HINT_F(func_name, func, flags) \
	{ Dee_TMH_##func_name, flags, ::__intern::__PRIVATE_match_method_hint<Dee_mh_##func_name##_t>::_match(func) }
#else /* __INTELLISENSE__ */
#define Dee_TYPE_METHOD_HINT_F(func_name, func, flags) \
	{ Dee_TMH_##func_name, flags, (Dee_funptr_t)(func) }
#endif /* !__INTELLISENSE__ */
#define Dee_TYPE_METHOD_HINT(func_name, func) Dee_TYPE_METHOD_HINT_F(func_name, func, 0)
#define Dee_TYPE_METHOD_HINT_END { (enum Dee_tmh_id)0, 0, NULL }


/* Link a type method in as part of a type's `tp_method_hints' array.
 * Behavior is undefined/depends-on-the-method-in-question if a type
 * defines a method as a hint reference, but fails to implement all
 * method hints used by the hinted method attribute. */
#define Dee_TYPE_METHOD_HINTREF(attr_name) \
	{ DeeMH_##attr_name##_name,            \
	  (Dee_objmethod_t)&DeeMH_##attr_name, \
	  DeeMH_##attr_name##_doc,             \
	  _Dee_TMH_WRAPPER_FLAGS_##attr_name }


#ifdef DEE_SOURCE
#define TYPE_METHOD_HINT     Dee_TYPE_METHOD_HINT
#define TYPE_METHOD_HINT_F   Dee_TYPE_METHOD_HINT_F
#define TYPE_METHOD_HINT_END Dee_TYPE_METHOD_HINT_END
#define TYPE_METHOD_HINTREF  Dee_TYPE_METHOD_HINTREF
#endif /* DEE_SOURCE */

/* Returns a pointer to method hint's entry in `self->tp_method_hints' */
DFUNDEF ATTR_PURE WUNUSED Dee_funptr_t DCALL
DeeType_GetPrivateMethodHint(DeeTypeObject *__restrict self, enum Dee_tmh_id id);

/* Same as `DeeType_GetPrivateMethodHint', but also searches the type's
 * MRO for all matches regarding attributes named "id", and returns the
 * native version for that attribute (or `NULL' if it doesn't have one)
 *
 * This function can also be used to query the optimized, internal
 * implementation of built-in sequence (TSC) functions. */
DFUNDEF ATTR_PURE WUNUSED Dee_funptr_t DCALL
DeeType_GetMethodHint(DeeTypeObject *__restrict self, enum Dee_tmh_id id);





#ifdef CONFIG_BUILDING_DEEMON

/* Type sequence operator definition functions. */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_mh_seq_foreach_reverse_t DCALL DeeType_TryRequireSeqForeachReverse(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_mh_seq_enumerate_index_reverse_t DCALL DeeType_TryRequireSeqEnumerateIndexReverse(DeeTypeObject *__restrict self);

/* Operators for the purpose of constructing `DefaultEnumeration_With*' objects. */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_makeenumeration_t DCALL DeeType_RequireSeqMakeEnumeration(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_makeenumeration_with_int_range_t DCALL DeeType_RequireSeqMakeEnumerationWithIntRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_makeenumeration_with_range_t DCALL DeeType_RequireSeqMakeEnumerationWithRange(DeeTypeObject *__restrict self);

/*[[[begin:seq_operators]]]*/
/* Sequence operators... */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_bool_t DCALL DeeType_RequireSeqOperatorBool(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_iter_t DCALL DeeType_RequireSeqOperatorIter(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_sizeob_t DCALL DeeType_RequireSeqOperatorSizeOb(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_contains_t DCALL DeeType_RequireSeqOperatorContains(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getitem_t DCALL DeeType_RequireSeqOperatorGetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delitem_t DCALL DeeType_RequireSeqOperatorDelItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setitem_t DCALL DeeType_RequireSeqOperatorSetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getrange_t DCALL DeeType_RequireSeqOperatorGetRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delrange_t DCALL DeeType_RequireSeqOperatorDelRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setrange_t DCALL DeeType_RequireSeqOperatorSetRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_foreach_t DCALL DeeType_RequireSeqOperatorForeach(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_foreach_pair_t DCALL DeeType_RequireSeqOperatorForeachPair(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_enumerate_t DCALL DeeType_RequireSeqOperatorEnumerate(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_enumerate_index_t DCALL DeeType_RequireSeqOperatorEnumerateIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_iterkeys_t DCALL DeeType_RequireSeqOperatorIterKeys(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_bounditem_t DCALL DeeType_RequireSeqOperatorBoundItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_hasitem_t DCALL DeeType_RequireSeqOperatorHasItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_size_t DCALL DeeType_RequireSeqOperatorSize(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_size_fast_t DCALL DeeType_RequireSeqOperatorSizeFast(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getitem_index_t DCALL DeeType_RequireSeqOperatorGetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delitem_index_t DCALL DeeType_RequireSeqOperatorDelItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setitem_index_t DCALL DeeType_RequireSeqOperatorSetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_bounditem_index_t DCALL DeeType_RequireSeqOperatorBoundItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_hasitem_index_t DCALL DeeType_RequireSeqOperatorHasItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getrange_index_t DCALL DeeType_RequireSeqOperatorGetRangeIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delrange_index_t DCALL DeeType_RequireSeqOperatorDelRangeIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setrange_index_t DCALL DeeType_RequireSeqOperatorSetRangeIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_getrange_index_n_t DCALL DeeType_RequireSeqOperatorGetRangeIndexN(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_delrange_index_n_t DCALL DeeType_RequireSeqOperatorDelRangeIndexN(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_setrange_index_n_t DCALL DeeType_RequireSeqOperatorSetRangeIndexN(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_trygetitem_t DCALL DeeType_RequireSeqOperatorTryGetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_trygetitem_index_t DCALL DeeType_RequireSeqOperatorTryGetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_hash_t DCALL DeeType_RequireSeqOperatorHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_compare_eq_t DCALL DeeType_RequireSeqOperatorCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_compare_t DCALL DeeType_RequireSeqOperatorCompare(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_trycompare_eq_t DCALL DeeType_RequireSeqOperatorTryCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_eq_t DCALL DeeType_RequireSeqOperatorEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_ne_t DCALL DeeType_RequireSeqOperatorNe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_lo_t DCALL DeeType_RequireSeqOperatorLo(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_le_t DCALL DeeType_RequireSeqOperatorLe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_gr_t DCALL DeeType_RequireSeqOperatorGr(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_ge_t DCALL DeeType_RequireSeqOperatorGe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_inplace_add_t DCALL DeeType_RequireSeqOperatorInplaceAdd(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_operator_inplace_mul_t DCALL DeeType_RequireSeqOperatorInplaceMul(DeeTypeObject *__restrict self);

/* Set operators... */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_iter_t DCALL DeeType_RequireSetOperatorIter(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_foreach_t DCALL DeeType_RequireSetOperatorForeach(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_size_t DCALL DeeType_RequireSetOperatorSize(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_sizeob_t DCALL DeeType_RequireSetOperatorSizeOb(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_hash_t DCALL DeeType_RequireSetOperatorHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_compare_eq_t DCALL DeeType_RequireSetOperatorCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_trycompare_eq_t DCALL DeeType_RequireSetOperatorTryCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_eq_t DCALL DeeType_RequireSetOperatorEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_ne_t DCALL DeeType_RequireSetOperatorNe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_lo_t DCALL DeeType_RequireSetOperatorLo(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_le_t DCALL DeeType_RequireSetOperatorLe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_gr_t DCALL DeeType_RequireSetOperatorGr(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_ge_t DCALL DeeType_RequireSetOperatorGe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inv_t DCALL DeeType_RequireSetOperatorInv(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_add_t DCALL DeeType_RequireSetOperatorAdd(DeeTypeObject *__restrict self); /* {"a"} + {"b"}         -> {"a","b"} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_sub_t DCALL DeeType_RequireSetOperatorSub(DeeTypeObject *__restrict self); /* {"a","b"} - {"b"}     -> {"a"} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_and_t DCALL DeeType_RequireSetOperatorAnd(DeeTypeObject *__restrict self); /* {"a","b"} & {"a"}     -> {"a"} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_xor_t DCALL DeeType_RequireSetOperatorXor(DeeTypeObject *__restrict self); /* {"a","b"} ^ {"a","c"} -> {"b","c"} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inplace_add_t DCALL DeeType_RequireSetOperatorInplaceAdd(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inplace_sub_t DCALL DeeType_RequireSetOperatorInplaceSub(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inplace_and_t DCALL DeeType_RequireSetOperatorInplaceAnd(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_operator_inplace_xor_t DCALL DeeType_RequireSetOperatorInplaceXor(DeeTypeObject *__restrict self);

/* Map operators... */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_contains_t DCALL DeeType_RequireMapOperatorContains(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_getitem_t DCALL DeeType_RequireMapOperatorGetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_delitem_t DCALL DeeType_RequireMapOperatorDelItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_setitem_t DCALL DeeType_RequireMapOperatorSetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_enumerate_t DCALL DeeType_RequireMapOperatorEnumerate(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_enumerate_index_t DCALL DeeType_RequireMapOperatorEnumerateIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_bounditem_t DCALL DeeType_RequireMapOperatorBoundItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_hasitem_t DCALL DeeType_RequireMapOperatorHasItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_getitem_index_t DCALL DeeType_RequireMapOperatorGetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_delitem_index_t DCALL DeeType_RequireMapOperatorDelItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_setitem_index_t DCALL DeeType_RequireMapOperatorSetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_bounditem_index_t DCALL DeeType_RequireMapOperatorBoundItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_hasitem_index_t DCALL DeeType_RequireMapOperatorHasItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trygetitem_t DCALL DeeType_RequireMapOperatorTryGetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trygetitem_index_t DCALL DeeType_RequireMapOperatorTryGetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trygetitem_string_hash_t DCALL DeeType_RequireMapOperatorTryGetItemStringHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_getitem_string_hash_t DCALL DeeType_RequireMapOperatorGetItemStringHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_delitem_string_hash_t DCALL DeeType_RequireMapOperatorDelItemStringHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_setitem_string_hash_t DCALL DeeType_RequireMapOperatorSetItemStringHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_bounditem_string_hash_t DCALL DeeType_RequireMapOperatorBoundItemStringHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_hasitem_string_hash_t DCALL DeeType_RequireMapOperatorHasItemStringHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trygetitem_string_len_hash_t DCALL DeeType_RequireMapOperatorTryGetItemStringLenHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_getitem_string_len_hash_t DCALL DeeType_RequireMapOperatorGetItemStringLenHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_delitem_string_len_hash_t DCALL DeeType_RequireMapOperatorDelItemStringLenHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_setitem_string_len_hash_t DCALL DeeType_RequireMapOperatorSetItemStringLenHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_bounditem_string_len_hash_t DCALL DeeType_RequireMapOperatorBoundItemStringLenHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_hasitem_string_len_hash_t DCALL DeeType_RequireMapOperatorHasItemStringLenHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_compare_eq_t DCALL DeeType_RequireMapOperatorCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_trycompare_eq_t DCALL DeeType_RequireMapOperatorTryCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_eq_t DCALL DeeType_RequireMapOperatorEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_ne_t DCALL DeeType_RequireMapOperatorNe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_lo_t DCALL DeeType_RequireMapOperatorLo(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_le_t DCALL DeeType_RequireMapOperatorLe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_gr_t DCALL DeeType_RequireMapOperatorGr(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_ge_t DCALL DeeType_RequireMapOperatorGe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_add_t DCALL DeeType_RequireMapOperatorAdd(DeeTypeObject *__restrict self); /* {"a":1} + {"b":2}       -> {"a":1,"b":2} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_sub_t DCALL DeeType_RequireMapOperatorSub(DeeTypeObject *__restrict self); /* {"a":1,"b":2} - {"a"}   -> {"b":2} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_and_t DCALL DeeType_RequireMapOperatorAnd(DeeTypeObject *__restrict self); /* {"a":1,"b":2} & {"a"}   -> {"a":1} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_xor_t DCALL DeeType_RequireMapOperatorXor(DeeTypeObject *__restrict self); /* {"a":1,"b":2} ^ {"a":3} -> {"b":2} */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_inplace_add_t DCALL DeeType_RequireMapOperatorInplaceAdd(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_inplace_sub_t DCALL DeeType_RequireMapOperatorInplaceSub(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_inplace_and_t DCALL DeeType_RequireMapOperatorInplaceAnd(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_operator_inplace_xor_t DCALL DeeType_RequireMapOperatorInplaceXor(DeeTypeObject *__restrict self);
/*[[[end:seq_operators]]]*/

/* Sequence function... */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_trygetfirst_t DCALL DeeType_RequireSeqTryGetFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_trygetlast_t DCALL DeeType_RequireSeqTryGetLast(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_any_t DCALL DeeType_RequireSeqAny(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_any_with_key_t DCALL DeeType_RequireSeqAnyWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_any_with_range_t DCALL DeeType_RequireSeqAnyWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_any_with_range_and_key_t DCALL DeeType_RequireSeqAnyWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_all_t DCALL DeeType_RequireSeqAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_all_with_key_t DCALL DeeType_RequireSeqAllWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_all_with_range_t DCALL DeeType_RequireSeqAllWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_all_with_range_and_key_t DCALL DeeType_RequireSeqAllWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_parity_t DCALL DeeType_RequireSeqParity(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_parity_with_key_t DCALL DeeType_RequireSeqParityWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_parity_with_range_t DCALL DeeType_RequireSeqParityWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_parity_with_range_and_key_t DCALL DeeType_RequireSeqParityWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reduce_t DCALL DeeType_RequireSeqReduce(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reduce_with_init_t DCALL DeeType_RequireSeqReduceWithInit(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reduce_with_range_t DCALL DeeType_RequireSeqReduceWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reduce_with_range_and_init_t DCALL DeeType_RequireSeqReduceWithRangeAndInit(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_min_t DCALL DeeType_RequireSeqMin(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_min_with_key_t DCALL DeeType_RequireSeqMinWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_min_with_range_t DCALL DeeType_RequireSeqMinWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_min_with_range_and_key_t DCALL DeeType_RequireSeqMinWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_max_t DCALL DeeType_RequireSeqMax(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_max_with_key_t DCALL DeeType_RequireSeqMaxWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_max_with_range_t DCALL DeeType_RequireSeqMaxWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_max_with_range_and_key_t DCALL DeeType_RequireSeqMaxWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sum_t DCALL DeeType_RequireSeqSum(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sum_with_range_t DCALL DeeType_RequireSeqSumWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_count_t DCALL DeeType_RequireSeqCount(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_count_with_key_t DCALL DeeType_RequireSeqCountWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_count_with_range_t DCALL DeeType_RequireSeqCountWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_count_with_range_and_key_t DCALL DeeType_RequireSeqCountWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_contains_t DCALL DeeType_RequireSeqContains(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_contains_with_key_t DCALL DeeType_RequireSeqContainsWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_contains_with_range_t DCALL DeeType_RequireSeqContainsWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_contains_with_range_and_key_t DCALL DeeType_RequireSeqContainsWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_locate_t DCALL DeeType_RequireSeqLocate(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_locate_with_key_t DCALL DeeType_RequireSeqLocateWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_locate_with_range_t DCALL DeeType_RequireSeqLocateWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_locate_with_range_and_key_t DCALL DeeType_RequireSeqLocateWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rlocate_with_range_t DCALL DeeType_RequireSeqRLocateWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rlocate_with_range_and_key_t DCALL DeeType_RequireSeqRLocateWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_startswith_t DCALL DeeType_RequireSeqStartsWith(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_startswith_with_key_t DCALL DeeType_RequireSeqStartsWithWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_startswith_with_range_t DCALL DeeType_RequireSeqStartsWithWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_startswith_with_range_and_key_t DCALL DeeType_RequireSeqStartsWithWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_endswith_t DCALL DeeType_RequireSeqEndsWith(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_endswith_with_key_t DCALL DeeType_RequireSeqEndsWithWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_endswith_with_range_t DCALL DeeType_RequireSeqEndsWithWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_endswith_with_range_and_key_t DCALL DeeType_RequireSeqEndsWithWithRangeAndKey(DeeTypeObject *__restrict self);

/* Mutable sequence functions */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_find_t DCALL DeeType_RequireSeqFind(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_find_with_key_t DCALL DeeType_RequireSeqFindWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rfind_t DCALL DeeType_RequireSeqRFind(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rfind_with_key_t DCALL DeeType_RequireSeqRFindWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_erase_t DCALL DeeType_RequireSeqErase(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_insert_t DCALL DeeType_RequireSeqInsert(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_insertall_t DCALL DeeType_RequireSeqInsertAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_pushfront_t DCALL DeeType_RequireSeqPushFront(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_append_t DCALL DeeType_RequireSeqAppend(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_extend_t DCALL DeeType_RequireSeqExtend(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_xchitem_index_t DCALL DeeType_RequireSeqXchItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_clear_t DCALL DeeType_RequireSeqClear(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_pop_t DCALL DeeType_RequireSeqPop(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_remove_t DCALL DeeType_RequireSeqRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_remove_with_key_t DCALL DeeType_RequireSeqRemoveWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rremove_t DCALL DeeType_RequireSeqRRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_rremove_with_key_t DCALL DeeType_RequireSeqRRemoveWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_removeall_t DCALL DeeType_RequireSeqRemoveAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_removeall_with_key_t DCALL DeeType_RequireSeqRemoveAllWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_removeif_t DCALL DeeType_RequireSeqRemoveIf(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_resize_t DCALL DeeType_RequireSeqResize(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_fill_t DCALL DeeType_RequireSeqFill(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reverse_t DCALL DeeType_RequireSeqReverse(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_reversed_t DCALL DeeType_RequireSeqReversed(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sort_t DCALL DeeType_RequireSeqSort(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sort_with_key_t DCALL DeeType_RequireSeqSortWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sorted_t DCALL DeeType_RequireSeqSorted(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_sorted_with_key_t DCALL DeeType_RequireSeqSortedWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_bfind_t DCALL DeeType_RequireSeqBFind(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_bfind_with_key_t DCALL DeeType_RequireSeqBFindWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_bposition_t DCALL DeeType_RequireSeqBPosition(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_bposition_with_key_t DCALL DeeType_RequireSeqBPositionWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_brange_t DCALL DeeType_RequireSeqBRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_brange_with_key_t DCALL DeeType_RequireSeqBRangeWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_blocate_t DCALL DeeType_RequireSeqBLocate(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_seq_blocate_with_key_t DCALL DeeType_RequireSeqBLocateWithKey(DeeTypeObject *__restrict self);

/* Set functions */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_insert_t DCALL DeeType_RequireSetInsert(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_remove_t DCALL DeeType_RequireSetRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_unify_t DCALL DeeType_RequireSetUnify(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_insertall_t DCALL DeeType_RequireSetInsertAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_removeall_t DCALL DeeType_RequireSetRemoveAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_pop_t DCALL DeeType_RequireSetPop(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_set_pop_with_default_t DCALL DeeType_RequireSetPopWithDefault(DeeTypeObject *__restrict self);

/* Map functions */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setold_t DCALL DeeType_RequireMapSetOld(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setold_ex_t DCALL DeeType_RequireMapSetOldEx(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setnew_t DCALL DeeType_RequireMapSetNew(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setnew_ex_t DCALL DeeType_RequireMapSetNewEx(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_setdefault_t DCALL DeeType_RequireMapSetDefault(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_update_t DCALL DeeType_RequireMapUpdate(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_remove_t DCALL DeeType_RequireMapRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_removekeys_t DCALL DeeType_RequireMapRemoveKeys(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_pop_t DCALL DeeType_RequireMapPop(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_pop_with_default_t DCALL DeeType_RequireMapPopWithDefault(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_mh_map_popitem_t DCALL DeeType_RequireMapPopItem(DeeTypeObject *__restrict self);

#else /* CONFIG_BUILDING_DEEMON */
#define DeeType_TryRequireSeqForeachReverse(self)               ((Dee_mh_seq_foreach_reverse_t)DeeType_GetMethodHint(self, Dee_TMH_seq_foreach_reverse))
#define DeeType_TryRequireSeqEnumerateIndexReverse(self)        ((Dee_mh_seq_enumerate_index_reverse_t)DeeType_GetMethodHint(self, Dee_TMH_seq_enumerate_index_reverse))
#define DeeType_RequireSeqMakeEnumeration(self)                 ((Dee_mh_seq_makeenumeration_t)DeeType_GetMethodHint(self, Dee_TMH_seq_makeenumeration))
#define DeeType_RequireSeqMakeEnumerationWithIntRange(self)     ((Dee_mh_seq_makeenumeration_with_int_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_makeenumeration_with_int_range))
#define DeeType_RequireSeqMakeEnumerationWithRange(self)        ((Dee_mh_seq_makeenumeration_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_makeenumeration_with_range))
#define DeeType_RequireSeqOperatorBool(self)                    ((Dee_mh_seq_operator_bool_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_bool))
#define DeeType_RequireSeqOperatorIter(self)                    ((Dee_mh_seq_operator_iter_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_iter))
#define DeeType_RequireSeqOperatorSizeOb(self)                  ((Dee_mh_seq_operator_sizeob_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_sizeob))
#define DeeType_RequireSeqOperatorContains(self)                ((Dee_mh_seq_operator_contains_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_contains))
#define DeeType_RequireSeqOperatorGetItem(self)                 ((Dee_mh_seq_operator_getitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getitem))
#define DeeType_RequireSeqOperatorDelItem(self)                 ((Dee_mh_seq_operator_delitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delitem))
#define DeeType_RequireSeqOperatorSetItem(self)                 ((Dee_mh_seq_operator_setitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setitem))
#define DeeType_RequireSeqOperatorGetRange(self)                ((Dee_mh_seq_operator_getrange_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getrange))
#define DeeType_RequireSeqOperatorDelRange(self)                ((Dee_mh_seq_operator_delrange_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delrange))
#define DeeType_RequireSeqOperatorSetRange(self)                ((Dee_mh_seq_operator_setrange_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setrange))
#define DeeType_RequireSeqOperatorForeach(self)                 ((Dee_mh_seq_operator_foreach_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_foreach))
#define DeeType_RequireSeqOperatorForeachPair(self)             ((Dee_mh_seq_operator_foreach_pair_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_foreach_pair))
#define DeeType_RequireSeqOperatorEnumerate(self)               ((Dee_mh_seq_operator_enumerate_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_enumerate))
#define DeeType_RequireSeqOperatorEnumerateIndex(self)          ((Dee_mh_seq_operator_enumerate_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_enumerate_index))
#define DeeType_RequireSeqOperatorIterKeys(self)                ((Dee_mh_seq_operator_iterkeys_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_iterkeys))
#define DeeType_RequireSeqOperatorBoundItem(self)               ((Dee_mh_seq_operator_bounditem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_bounditem))
#define DeeType_RequireSeqOperatorHasItem(self)                 ((Dee_mh_seq_operator_hasitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_hasitem))
#define DeeType_RequireSeqOperatorSize(self)                    ((Dee_mh_seq_operator_size_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_size))
#define DeeType_RequireSeqOperatorSizeFast(self)                ((Dee_mh_seq_operator_size_fast_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_size_fast))
#define DeeType_RequireSeqOperatorGetItemIndex(self)            ((Dee_mh_seq_operator_getitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getitem_index))
#define DeeType_RequireSeqOperatorDelItemIndex(self)            ((Dee_mh_seq_operator_delitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delitem_index))
#define DeeType_RequireSeqOperatorSetItemIndex(self)            ((Dee_mh_seq_operator_setitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setitem_index))
#define DeeType_RequireSeqOperatorBoundItemIndex(self)          ((Dee_mh_seq_operator_bounditem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_bounditem_index))
#define DeeType_RequireSeqOperatorHasItemIndex(self)            ((Dee_mh_seq_operator_hasitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_hasitem_index))
#define DeeType_RequireSeqOperatorGetRangeIndex(self)           ((Dee_mh_seq_operator_getrange_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getrange_index))
#define DeeType_RequireSeqOperatorDelRangeIndex(self)           ((Dee_mh_seq_operator_delrange_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delrange_index))
#define DeeType_RequireSeqOperatorSetRangeIndex(self)           ((Dee_mh_seq_operator_setrange_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setrange_index))
#define DeeType_RequireSeqOperatorGetRangeIndexN(self)          ((Dee_mh_seq_operator_getrange_index_n_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_getrange_index_n))
#define DeeType_RequireSeqOperatorDelRangeIndexN(self)          ((Dee_mh_seq_operator_delrange_index_n_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_delrange_index_n))
#define DeeType_RequireSeqOperatorSetRangeIndexN(self)          ((Dee_mh_seq_operator_setrange_index_n_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_setrange_index_n))
#define DeeType_RequireSeqOperatorTryGetItem(self)              ((Dee_mh_seq_operator_trygetitem_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_trygetitem))
#define DeeType_RequireSeqOperatorTryGetItemIndex(self)         ((Dee_mh_seq_operator_trygetitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_trygetitem_index))
#define DeeType_RequireSeqOperatorHash(self)                    ((Dee_mh_seq_operator_hash_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_hash))
#define DeeType_RequireSeqOperatorCompareEq(self)               ((Dee_mh_seq_operator_compare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_compare_eq))
#define DeeType_RequireSeqOperatorCompare(self)                 ((Dee_mh_seq_operator_compare_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_compare))
#define DeeType_RequireSeqOperatorTryCompareEq(self)            ((Dee_mh_seq_operator_trycompare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_trycompare_eq))
#define DeeType_RequireSeqOperatorEq(self)                      ((Dee_mh_seq_operator_eq_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_eq))
#define DeeType_RequireSeqOperatorNe(self)                      ((Dee_mh_seq_operator_ne_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_ne))
#define DeeType_RequireSeqOperatorLo(self)                      ((Dee_mh_seq_operator_lo_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_lo))
#define DeeType_RequireSeqOperatorLe(self)                      ((Dee_mh_seq_operator_le_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_le))
#define DeeType_RequireSeqOperatorGr(self)                      ((Dee_mh_seq_operator_gr_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_gr))
#define DeeType_RequireSeqOperatorGe(self)                      ((Dee_mh_seq_operator_ge_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_ge))
#define DeeType_RequireSeqOperatorInplaceAdd(self)              ((Dee_mh_seq_operator_inplace_add_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_inplace_add))
#define DeeType_RequireSeqOperatorInplaceMul(self)              ((Dee_mh_seq_operator_inplace_mul_t)DeeType_GetMethodHint(self, Dee_TMH_seq_operator_inplace_mul))
#define DeeType_RequireSetOperatorIter(self)                    ((Dee_mh_set_operator_iter_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_iter))
#define DeeType_RequireSetOperatorForeach(self)                 ((Dee_mh_set_operator_foreach_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_foreach))
#define DeeType_RequireSetOperatorSize(self)                    ((Dee_mh_set_operator_size_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_size))
#define DeeType_RequireSetOperatorSizeOb(self)                  ((Dee_mh_set_operator_sizeob_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_sizeob))
#define DeeType_RequireSetOperatorHash(self)                    ((Dee_mh_set_operator_hash_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_hash))
#define DeeType_RequireSetOperatorCompareEq(self)               ((Dee_mh_set_operator_compare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_compare_eq))
#define DeeType_RequireSetOperatorTryCompareEq(self)            ((Dee_mh_set_operator_trycompare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_trycompare_eq))
#define DeeType_RequireSetOperatorEq(self)                      ((Dee_mh_set_operator_eq_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_eq))
#define DeeType_RequireSetOperatorNe(self)                      ((Dee_mh_set_operator_ne_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_ne))
#define DeeType_RequireSetOperatorLo(self)                      ((Dee_mh_set_operator_lo_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_lo))
#define DeeType_RequireSetOperatorLe(self)                      ((Dee_mh_set_operator_le_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_le))
#define DeeType_RequireSetOperatorGr(self)                      ((Dee_mh_set_operator_gr_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_gr))
#define DeeType_RequireSetOperatorGe(self)                      ((Dee_mh_set_operator_ge_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_ge))
#define DeeType_RequireSetOperatorInv(self)                     ((Dee_mh_set_operator_inv_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inv))
#define DeeType_RequireSetOperatorAdd(self)                     ((Dee_mh_set_operator_add_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_add))
#define DeeType_RequireSetOperatorSub(self)                     ((Dee_mh_set_operator_sub_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_sub))
#define DeeType_RequireSetOperatorAnd(self)                     ((Dee_mh_set_operator_and_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_and))
#define DeeType_RequireSetOperatorXor(self)                     ((Dee_mh_set_operator_xor_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_xor))
#define DeeType_RequireSetOperatorInplaceAdd(self)              ((Dee_mh_set_operator_inplace_add_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inplace_add))
#define DeeType_RequireSetOperatorInplaceSub(self)              ((Dee_mh_set_operator_inplace_sub_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inplace_sub))
#define DeeType_RequireSetOperatorInplaceAnd(self)              ((Dee_mh_set_operator_inplace_and_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inplace_and))
#define DeeType_RequireSetOperatorInplaceXor(self)              ((Dee_mh_set_operator_inplace_xor_t)DeeType_GetMethodHint(self, Dee_TMH_set_operator_inplace_xor))
#define DeeType_RequireMapOperatorContains(self)                ((Dee_mh_map_operator_contains_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_contains))
#define DeeType_RequireMapOperatorGetItem(self)                 ((Dee_mh_map_operator_getitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_getitem))
#define DeeType_RequireMapOperatorDelItem(self)                 ((Dee_mh_map_operator_delitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_delitem))
#define DeeType_RequireMapOperatorSetItem(self)                 ((Dee_mh_map_operator_setitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_setitem))
#define DeeType_RequireMapOperatorEnumerate(self)               ((Dee_mh_map_operator_enumerate_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_enumerate))
#define DeeType_RequireMapOperatorEnumerateIndex(self)          ((Dee_mh_map_operator_enumerate_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_enumerate_index))
#define DeeType_RequireMapOperatorBoundItem(self)               ((Dee_mh_map_operator_bounditem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_bounditem))
#define DeeType_RequireMapOperatorHasItem(self)                 ((Dee_mh_map_operator_hasitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_hasitem))
#define DeeType_RequireMapOperatorGetItemIndex(self)            ((Dee_mh_map_operator_getitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_getitem_index))
#define DeeType_RequireMapOperatorDelItemIndex(self)            ((Dee_mh_map_operator_delitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_delitem_index))
#define DeeType_RequireMapOperatorSetItemIndex(self)            ((Dee_mh_map_operator_setitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_setitem_index))
#define DeeType_RequireMapOperatorBoundItemIndex(self)          ((Dee_mh_map_operator_bounditem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_bounditem_index))
#define DeeType_RequireMapOperatorHasItemIndex(self)            ((Dee_mh_map_operator_hasitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_hasitem_index))
#define DeeType_RequireMapOperatorTryGetItem(self)              ((Dee_mh_map_operator_trygetitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trygetitem))
#define DeeType_RequireMapOperatorTryGetItemIndex(self)         ((Dee_mh_map_operator_trygetitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trygetitem_index))
#define DeeType_RequireMapOperatorTryGetItemStringHash(self)    ((Dee_mh_map_operator_trygetitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trygetitem_string_hash))
#define DeeType_RequireMapOperatorGetItemStringHash(self)       ((Dee_mh_map_operator_getitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_getitem_string_hash))
#define DeeType_RequireMapOperatorDelItemStringHash(self)       ((Dee_mh_map_operator_delitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_delitem_string_hash))
#define DeeType_RequireMapOperatorSetItemStringHash(self)       ((Dee_mh_map_operator_setitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_setitem_string_hash))
#define DeeType_RequireMapOperatorBoundItemStringHash(self)     ((Dee_mh_map_operator_bounditem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_bounditem_string_hash))
#define DeeType_RequireMapOperatorHasItemStringHash(self)       ((Dee_mh_map_operator_hasitem_string_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_hasitem_string_hash))
#define DeeType_RequireMapOperatorTryGetItemStringLenHash(self) ((Dee_mh_map_operator_trygetitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trygetitem_string_len_hash))
#define DeeType_RequireMapOperatorGetItemStringLenHash(self)    ((Dee_mh_map_operator_getitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_getitem_string_len_hash))
#define DeeType_RequireMapOperatorDelItemStringLenHash(self)    ((Dee_mh_map_operator_delitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_delitem_string_len_hash))
#define DeeType_RequireMapOperatorSetItemStringLenHash(self)    ((Dee_mh_map_operator_setitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_setitem_string_len_hash))
#define DeeType_RequireMapOperatorBoundItemStringLenHash(self)  ((Dee_mh_map_operator_bounditem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_bounditem_string_len_hash))
#define DeeType_RequireMapOperatorHasItemStringLenHash(self)    ((Dee_mh_map_operator_hasitem_string_len_hash_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_hasitem_string_len_hash))
#define DeeType_RequireMapOperatorCompareEq(self)               ((Dee_mh_map_operator_compare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_compare_eq))
#define DeeType_RequireMapOperatorTryCompareEq(self)            ((Dee_mh_map_operator_trycompare_eq_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_trycompare_eq))
#define DeeType_RequireMapOperatorEq(self)                      ((Dee_mh_map_operator_eq_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_eq))
#define DeeType_RequireMapOperatorNe(self)                      ((Dee_mh_map_operator_ne_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_ne))
#define DeeType_RequireMapOperatorLo(self)                      ((Dee_mh_map_operator_lo_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_lo))
#define DeeType_RequireMapOperatorLe(self)                      ((Dee_mh_map_operator_le_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_le))
#define DeeType_RequireMapOperatorGr(self)                      ((Dee_mh_map_operator_gr_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_gr))
#define DeeType_RequireMapOperatorGe(self)                      ((Dee_mh_map_operator_ge_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_ge))
#define DeeType_RequireMapOperatorAdd(self)                     ((Dee_mh_map_operator_add_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_add))
#define DeeType_RequireMapOperatorSub(self)                     ((Dee_mh_map_operator_sub_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_sub))
#define DeeType_RequireMapOperatorAnd(self)                     ((Dee_mh_map_operator_and_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_and))
#define DeeType_RequireMapOperatorXor(self)                     ((Dee_mh_map_operator_xor_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_xor))
#define DeeType_RequireMapOperatorInplaceAdd(self)              ((Dee_mh_map_operator_inplace_add_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_inplace_add))
#define DeeType_RequireMapOperatorInplaceSub(self)              ((Dee_mh_map_operator_inplace_sub_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_inplace_sub))
#define DeeType_RequireMapOperatorInplaceAnd(self)              ((Dee_mh_map_operator_inplace_and_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_inplace_and))
#define DeeType_RequireMapOperatorInplaceXor(self)              ((Dee_mh_map_operator_inplace_xor_t)DeeType_GetMethodHint(self, Dee_TMH_map_operator_inplace_xor))
#define DeeType_RequireSeqTryGetFirst(self)                     ((Dee_mh_seq_trygetfirst_t)DeeType_GetMethodHint(self, Dee_TMH_seq_trygetfirst))
#define DeeType_RequireSeqTryGetLast(self)                      ((Dee_mh_seq_trygetlast_t)DeeType_GetMethodHint(self, Dee_TMH_seq_trygetlast))
#define DeeType_RequireSeqAny(self)                             ((Dee_mh_seq_any_t)DeeType_GetMethodHint(self, Dee_TMH_seq_any))
#define DeeType_RequireSeqAnyWithKey(self)                      ((Dee_mh_seq_any_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_any_with_key))
#define DeeType_RequireSeqAnyWithRange(self)                    ((Dee_mh_seq_any_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_any_with_range))
#define DeeType_RequireSeqAnyWithRangeAndKey(self)              ((Dee_mh_seq_any_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_any_with_range_and_key))
#define DeeType_RequireSeqAll(self)                             ((Dee_mh_seq_all_t)DeeType_GetMethodHint(self, Dee_TMH_seq_all))
#define DeeType_RequireSeqAllWithKey(self)                      ((Dee_mh_seq_all_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_all_with_key))
#define DeeType_RequireSeqAllWithRange(self)                    ((Dee_mh_seq_all_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_all_with_range))
#define DeeType_RequireSeqAllWithRangeAndKey(self)              ((Dee_mh_seq_all_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_all_with_range_and_key))
#define DeeType_RequireSeqParity(self)                          ((Dee_mh_seq_parity_t)DeeType_GetMethodHint(self, Dee_TMH_seq_parity))
#define DeeType_RequireSeqParityWithKey(self)                   ((Dee_mh_seq_parity_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_parity_with_key))
#define DeeType_RequireSeqParityWithRange(self)                 ((Dee_mh_seq_parity_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_parity_with_range))
#define DeeType_RequireSeqParityWithRangeAndKey(self)           ((Dee_mh_seq_parity_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_parity_with_range_and_key))
#define DeeType_RequireSeqReduce(self)                          ((Dee_mh_seq_reduce_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reduce))
#define DeeType_RequireSeqReduceWithInit(self)                  ((Dee_mh_seq_reduce_with_init_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reduce_with_init))
#define DeeType_RequireSeqReduceWithRange(self)                 ((Dee_mh_seq_reduce_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reduce_with_range))
#define DeeType_RequireSeqReduceWithRangeAndInit(self)          ((Dee_mh_seq_reduce_with_range_and_init_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reduce_with_range_and_init))
#define DeeType_RequireSeqMin(self)                             ((Dee_mh_seq_min_t)DeeType_GetMethodHint(self, Dee_TMH_seq_min))
#define DeeType_RequireSeqMinWithKey(self)                      ((Dee_mh_seq_min_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_min_with_key))
#define DeeType_RequireSeqMinWithRange(self)                    ((Dee_mh_seq_min_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_min_with_range))
#define DeeType_RequireSeqMinWithRangeAndKey(self)              ((Dee_mh_seq_min_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_min_with_range_and_key))
#define DeeType_RequireSeqMax(self)                             ((Dee_mh_seq_max_t)DeeType_GetMethodHint(self, Dee_TMH_seq_max))
#define DeeType_RequireSeqMaxWithKey(self)                      ((Dee_mh_seq_max_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_max_with_key))
#define DeeType_RequireSeqMaxWithRange(self)                    ((Dee_mh_seq_max_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_max_with_range))
#define DeeType_RequireSeqMaxWithRangeAndKey(self)              ((Dee_mh_seq_max_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_max_with_range_and_key))
#define DeeType_RequireSeqSum(self)                             ((Dee_mh_seq_sum_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sum))
#define DeeType_RequireSeqSumWithRange(self)                    ((Dee_mh_seq_sum_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sum_with_range))
#define DeeType_RequireSeqCount(self)                           ((Dee_mh_seq_count_t)DeeType_GetMethodHint(self, Dee_TMH_seq_count))
#define DeeType_RequireSeqCountWithKey(self)                    ((Dee_mh_seq_count_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_count_with_key))
#define DeeType_RequireSeqCountWithRange(self)                  ((Dee_mh_seq_count_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_count_with_range))
#define DeeType_RequireSeqCountWithRangeAndKey(self)            ((Dee_mh_seq_count_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_count_with_range_and_key))
#define DeeType_RequireSeqContains(self)                        ((Dee_mh_seq_contains_t)DeeType_GetMethodHint(self, Dee_TMH_seq_contains))
#define DeeType_RequireSeqContainsWithKey(self)                 ((Dee_mh_seq_contains_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_contains_with_key))
#define DeeType_RequireSeqContainsWithRange(self)               ((Dee_mh_seq_contains_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_contains_with_range))
#define DeeType_RequireSeqContainsWithRangeAndKey(self)         ((Dee_mh_seq_contains_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_contains_with_range_and_key))
#define DeeType_RequireSeqLocate(self)                          ((Dee_mh_seq_locate_t)DeeType_GetMethodHint(self, Dee_TMH_seq_locate))
#define DeeType_RequireSeqLocateWithKey(self)                   ((Dee_mh_seq_locate_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_locate_with_key))
#define DeeType_RequireSeqLocateWithRange(self)                 ((Dee_mh_seq_locate_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_locate_with_range))
#define DeeType_RequireSeqLocateWithRangeAndKey(self)           ((Dee_mh_seq_locate_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_locate_with_range_and_key))
#define DeeType_RequireSeqRLocateWithRange(self)                ((Dee_mh_seq_rlocate_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rlocate_with_range))
#define DeeType_RequireSeqRLocateWithRangeAndKey(self)          ((Dee_mh_seq_rlocate_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rlocate_with_range_and_key))
#define DeeType_RequireSeqStartsWith(self)                      ((Dee_mh_seq_startswith_t)DeeType_GetMethodHint(self, Dee_TMH_seq_startswith))
#define DeeType_RequireSeqStartsWithWithKey(self)               ((Dee_mh_seq_startswith_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_startswith_with_key))
#define DeeType_RequireSeqStartsWithWithRange(self)             ((Dee_mh_seq_startswith_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_startswith_with_range))
#define DeeType_RequireSeqStartsWithWithRangeAndKey(self)       ((Dee_mh_seq_startswith_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_startswith_with_range_and_key))
#define DeeType_RequireSeqEndsWith(self)                        ((Dee_mh_seq_endswith_t)DeeType_GetMethodHint(self, Dee_TMH_seq_endswith))
#define DeeType_RequireSeqEndsWithWithKey(self)                 ((Dee_mh_seq_endswith_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_endswith_with_key))
#define DeeType_RequireSeqEndsWithWithRange(self)               ((Dee_mh_seq_endswith_with_range_t)DeeType_GetMethodHint(self, Dee_TMH_seq_endswith_with_range))
#define DeeType_RequireSeqEndsWithWithRangeAndKey(self)         ((Dee_mh_seq_endswith_with_range_and_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_endswith_with_range_and_key))
#define DeeType_RequireSeqFind(self)                            ((Dee_mh_seq_find_t)DeeType_GetMethodHint(self, Dee_TMH_seq_find))
#define DeeType_RequireSeqFindWithKey(self)                     ((Dee_mh_seq_find_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_find_with_key))
#define DeeType_RequireSeqRFind(self)                           ((Dee_mh_seq_rfind_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rfind))
#define DeeType_RequireSeqRFindWithKey(self)                    ((Dee_mh_seq_rfind_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rfind_with_key))
#define DeeType_RequireSeqErase(self)                           ((Dee_mh_seq_erase_t)DeeType_GetMethodHint(self, Dee_TMH_seq_erase))
#define DeeType_RequireSeqInsert(self)                          ((Dee_mh_seq_insert_t)DeeType_GetMethodHint(self, Dee_TMH_seq_insert))
#define DeeType_RequireSeqInsertAll(self)                       ((Dee_mh_seq_insertall_t)DeeType_GetMethodHint(self, Dee_TMH_seq_insertall))
#define DeeType_RequireSeqPushFront(self)                       ((Dee_mh_seq_pushfront_t)DeeType_GetMethodHint(self, Dee_TMH_seq_pushfront))
#define DeeType_RequireSeqAppend(self)                          ((Dee_mh_seq_append_t)DeeType_GetMethodHint(self, Dee_TMH_seq_append))
#define DeeType_RequireSeqExtend(self)                          ((Dee_mh_seq_extend_t)DeeType_GetMethodHint(self, Dee_TMH_seq_extend))
#define DeeType_RequireSeqXchItemIndex(self)                    ((Dee_mh_seq_xchitem_index_t)DeeType_GetMethodHint(self, Dee_TMH_seq_xchitem_index))
#define DeeType_RequireSeqClear(self)                           ((Dee_mh_seq_clear_t)DeeType_GetMethodHint(self, Dee_TMH_seq_clear))
#define DeeType_RequireSeqPop(self)                             ((Dee_mh_seq_pop_t)DeeType_GetMethodHint(self, Dee_TMH_seq_pop))
#define DeeType_RequireSeqRemove(self)                          ((Dee_mh_seq_remove_t)DeeType_GetMethodHint(self, Dee_TMH_seq_remove))
#define DeeType_RequireSeqRemoveWithKey(self)                   ((Dee_mh_seq_remove_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_remove_with_key))
#define DeeType_RequireSeqRRemove(self)                         ((Dee_mh_seq_rremove_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rremove))
#define DeeType_RequireSeqRRemoveWithKey(self)                  ((Dee_mh_seq_rremove_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_rremove_with_key))
#define DeeType_RequireSeqRemoveAll(self)                       ((Dee_mh_seq_removeall_t)DeeType_GetMethodHint(self, Dee_TMH_seq_removeall))
#define DeeType_RequireSeqRemoveAllWithKey(self)                ((Dee_mh_seq_removeall_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_removeall_with_key))
#define DeeType_RequireSeqRemoveIf(self)                        ((Dee_mh_seq_removeif_t)DeeType_GetMethodHint(self, Dee_TMH_seq_removeif))
#define DeeType_RequireSeqResize(self)                          ((Dee_mh_seq_resize_t)DeeType_GetMethodHint(self, Dee_TMH_seq_resize))
#define DeeType_RequireSeqFill(self)                            ((Dee_mh_seq_fill_t)DeeType_GetMethodHint(self, Dee_TMH_seq_fill))
#define DeeType_RequireSeqReverse(self)                         ((Dee_mh_seq_reverse_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reverse))
#define DeeType_RequireSeqReversed(self)                        ((Dee_mh_seq_reversed_t)DeeType_GetMethodHint(self, Dee_TMH_seq_reversed))
#define DeeType_RequireSeqSort(self)                            ((Dee_mh_seq_sort_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sort))
#define DeeType_RequireSeqSortWithKey(self)                     ((Dee_mh_seq_sort_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sort_with_key))
#define DeeType_RequireSeqSorted(self)                          ((Dee_mh_seq_sorted_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sorted))
#define DeeType_RequireSeqSortedWithKey(self)                   ((Dee_mh_seq_sorted_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_sorted_with_key))
#define DeeType_RequireSeqBFind(self)                           ((Dee_mh_seq_bfind_t)DeeType_GetMethodHint(self, Dee_TMH_seq_bfind))
#define DeeType_RequireSeqBFindWithKey(self)                    ((Dee_mh_seq_bfind_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_bfind_with_key))
#define DeeType_RequireSeqBPosition(self)                       ((Dee_mh_seq_bposition_t)DeeType_GetMethodHint(self, Dee_TMH_seq_bposition))
#define DeeType_RequireSeqBPositionWithKey(self)                ((Dee_mh_seq_bposition_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_bposition_with_key))
#define DeeType_RequireSeqBRange(self)                          ((Dee_mh_seq_brange_t)DeeType_GetMethodHint(self, Dee_TMH_seq_brange))
#define DeeType_RequireSeqBRangeWithKey(self)                   ((Dee_mh_seq_brange_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_brange_with_key))
#define DeeType_RequireSeqBLocate(self)                         ((Dee_mh_seq_blocate_t)DeeType_GetMethodHint(self, Dee_TMH_seq_blocate))
#define DeeType_RequireSeqBLocateWithKey(self)                  ((Dee_mh_seq_blocate_with_key_t)DeeType_GetMethodHint(self, Dee_TMH_seq_blocate_with_key))
#define DeeType_RequireSetInsert(self)                          ((Dee_mh_set_insert_t)DeeType_GetMethodHint(self, Dee_TMH_set_insert))
#define DeeType_RequireSetRemove(self)                          ((Dee_mh_set_remove_t)DeeType_GetMethodHint(self, Dee_TMH_set_remove))
#define DeeType_RequireSetUnify(self)                           ((Dee_mh_set_unify_t)DeeType_GetMethodHint(self, Dee_TMH_set_unify))
#define DeeType_RequireSetInsertAll(self)                       ((Dee_mh_set_insertall_t)DeeType_GetMethodHint(self, Dee_TMH_set_insertall))
#define DeeType_RequireSetRemoveAll(self)                       ((Dee_mh_set_removeall_t)DeeType_GetMethodHint(self, Dee_TMH_set_removeall))
#define DeeType_RequireSetPop(self)                             ((Dee_mh_set_pop_t)DeeType_GetMethodHint(self, Dee_TMH_set_pop))
#define DeeType_RequireSetPopWithDefault(self)                  ((Dee_mh_set_pop_with_default_t)DeeType_GetMethodHint(self, Dee_TMH_set_pop_with_default))
#define DeeType_RequireMapSetOld(self)                          ((Dee_mh_map_setold_t)DeeType_GetMethodHint(self, Dee_TMH_map_setold))
#define DeeType_RequireMapSetOldEx(self)                        ((Dee_mh_map_setold_ex_t)DeeType_GetMethodHint(self, Dee_TMH_map_setold_ex))
#define DeeType_RequireMapSetNew(self)                          ((Dee_mh_map_setnew_t)DeeType_GetMethodHint(self, Dee_TMH_map_setnew))
#define DeeType_RequireMapSetNewEx(self)                        ((Dee_mh_map_setnew_ex_t)DeeType_GetMethodHint(self, Dee_TMH_map_setnew_ex))
#define DeeType_RequireMapSetDefault(self)                      ((Dee_mh_map_setdefault_t)DeeType_GetMethodHint(self, Dee_TMH_map_setdefault))
#define DeeType_RequireMapUpdate(self)                          ((Dee_mh_map_update_t)DeeType_GetMethodHint(self, Dee_TMH_map_update))
#define DeeType_RequireMapRemove(self)                          ((Dee_mh_map_remove_t)DeeType_GetMethodHint(self, Dee_TMH_map_remove))
#define DeeType_RequireMapRemoveKeys(self)                      ((Dee_mh_map_removekeys_t)DeeType_GetMethodHint(self, Dee_TMH_map_removekeys))
#define DeeType_RequireMapPop(self)                             ((Dee_mh_map_pop_t)DeeType_GetMethodHint(self, Dee_TMH_map_pop))
#define DeeType_RequireMapPopWithDefault(self)                  ((Dee_mh_map_pop_with_default_t)DeeType_GetMethodHint(self, Dee_TMH_map_pop_with_default))
#define DeeType_RequireMapPopItem(self)                         ((Dee_mh_map_popitem_t)DeeType_GetMethodHint(self, Dee_TMH_map_popitem))
#endif /* !CONFIG_BUILDING_DEEMON */

#define DeeType_RequireMapOperatorHash DeeType_RequireSetOperatorHash

DECL_END

#endif /* !GUARD_DEEMON_METHOD_HINTS_H */
