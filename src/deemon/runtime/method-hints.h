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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINTS_H
#define GUARD_DEEMON_RUNTIME_METHOD_HINTS_H 1

#include <deemon/api.h>

#if defined(CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS) || defined(__DEEMON__)
#include <deemon/alloc.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>

#undef CONFIG_HAVE_MH_CALLMETHODCACHE
#if 1
#define CONFIG_HAVE_MH_CALLMETHODCACHE
#endif

DECL_BEGIN

union mhc_slot {
	DREF DeeObject   *c_object;
#ifdef CONFIG_HAVE_MH_CALLMETHODCACHE
	Dee_objmethod_t   c_method;
	Dee_kwobjmethod_t c_kwmethod;
#endif /* CONFIG_HAVE_MH_CALLMETHODCACHE */
};

struct Dee_type_mh_cache {
	/* Method hint function pointer caches.
	 * All of these are [0..1][lock(WRITE_ONCE)]
	 *
	 * Also note that the offset of any hint function pointer is always the "Dee_TMH_*" constant:
	 * >> void *mh_seq_operator_foreach = ((void **)tp_mhcache)[Dee_TMH_seq_operator_foreach]; */
#define Dee_type_mh_cache_gethint(self, id)    ((struct Dee_type_mh_cache_array const *)(self))->mh_funcs[id]
#define Dee_type_mh_cache_sethint(self, id, v) (void)(((struct Dee_type_mh_cache_array *)(self))->mh_funcs[id] = (v))

	/* clang-format off */
/*[[[deemon (printMhCacheNativeMembers from "..method-hints.method-hints")();]]]*/
	DeeMH_seq_operator_bool_t mh_seq_operator_bool;
	DeeMH_seq_operator_sizeob_t mh_seq_operator_sizeob;
	DeeMH_seq_operator_size_t mh_seq_operator_size;
	DeeMH_seq_operator_iter_t mh_seq_operator_iter;
	DeeMH_seq_operator_foreach_t mh_seq_operator_foreach;
	DeeMH_seq_operator_foreach_pair_t mh_seq_operator_foreach_pair;
	DeeMH_seq_operator_iterkeys_t mh_seq_operator_iterkeys;
	DeeMH_seq_operator_enumerate_t mh_seq_operator_enumerate;
	DeeMH_seq_operator_enumerate_index_t mh_seq_operator_enumerate_index;
	DeeMH_seq_operator_getitem_t mh_seq_operator_getitem;
	DeeMH_seq_operator_getitem_index_t mh_seq_operator_getitem_index;
	DeeMH_seq_operator_trygetitem_index_t mh_seq_operator_trygetitem_index;
	DeeMH_seq_operator_trygetitem_t mh_seq_operator_trygetitem;
	DeeMH_seq_operator_hasitem_t mh_seq_operator_hasitem;
	DeeMH_seq_operator_hasitem_index_t mh_seq_operator_hasitem_index;
	DeeMH_seq_operator_bounditem_t mh_seq_operator_bounditem;
	DeeMH_seq_operator_bounditem_index_t mh_seq_operator_bounditem_index;
	DeeMH_seq_operator_delitem_t mh_seq_operator_delitem;
	DeeMH_seq_operator_delitem_index_t mh_seq_operator_delitem_index;
	DeeMH_seq_operator_setitem_t mh_seq_operator_setitem;
	DeeMH_seq_operator_setitem_index_t mh_seq_operator_setitem_index;
	DeeMH_seq_operator_getrange_t mh_seq_operator_getrange;
	DeeMH_seq_operator_getrange_index_t mh_seq_operator_getrange_index;
	DeeMH_seq_operator_getrange_index_n_t mh_seq_operator_getrange_index_n;
	DeeMH_seq_operator_delrange_t mh_seq_operator_delrange;
	DeeMH_seq_operator_delrange_index_t mh_seq_operator_delrange_index;
	DeeMH_seq_operator_delrange_index_n_t mh_seq_operator_delrange_index_n;
	DeeMH_seq_operator_setrange_t mh_seq_operator_setrange;
	DeeMH_seq_operator_setrange_index_t mh_seq_operator_setrange_index;
	DeeMH_seq_operator_setrange_index_n_t mh_seq_operator_setrange_index_n;
	DeeMH_seq_operator_hash_t mh_seq_operator_hash;
	DeeMH_seq_operator_compare_t mh_seq_operator_compare;
	DeeMH_seq_operator_compare_eq_t mh_seq_operator_compare_eq;
	DeeMH_seq_operator_trycompare_eq_t mh_seq_operator_trycompare_eq;
	DeeMH_seq_operator_eq_t mh_seq_operator_eq;
	DeeMH_seq_operator_ne_t mh_seq_operator_ne;
	DeeMH_seq_operator_lo_t mh_seq_operator_lo;
	DeeMH_seq_operator_le_t mh_seq_operator_le;
	DeeMH_seq_operator_gr_t mh_seq_operator_gr;
	DeeMH_seq_operator_ge_t mh_seq_operator_ge;
	DeeMH_seq_trygetfirst_t mh_seq_trygetfirst;
	DeeMH_seq_getfirst_t mh_seq_getfirst;
	DeeMH_seq_boundfirst_t mh_seq_boundfirst;
	DeeMH_seq_delfirst_t mh_seq_delfirst;
	DeeMH_seq_setfirst_t mh_seq_setfirst;
	DeeMH_seq_trygetlast_t mh_seq_trygetlast;
	DeeMH_seq_getlast_t mh_seq_getlast;
	DeeMH_seq_boundlast_t mh_seq_boundlast;
	DeeMH_seq_dellast_t mh_seq_dellast;
	DeeMH_seq_setlast_t mh_seq_setlast;
	DeeMH_seq_any_t mh_seq_any;
	DeeMH_seq_any_with_key_t mh_seq_any_with_key;
	DeeMH_seq_any_with_range_t mh_seq_any_with_range;
	DeeMH_seq_any_with_range_and_key_t mh_seq_any_with_range_and_key;
	DeeMH_seq_all_t mh_seq_all;
	DeeMH_seq_all_with_key_t mh_seq_all_with_key;
	DeeMH_seq_all_with_range_t mh_seq_all_with_range;
	DeeMH_seq_all_with_range_and_key_t mh_seq_all_with_range_and_key;
	DeeMH_seq_erase_t mh_seq_erase;
	DeeMH_seq_insert_t mh_seq_insert;
	DeeMH_seq_insertall_t mh_seq_insertall;
	DeeMH_seq_pushfront_t mh_seq_pushfront;
	DeeMH_seq_append_t mh_seq_append;
	DeeMH_seq_extend_t mh_seq_extend;
	DeeMH_seq_xchitem_index_t mh_seq_xchitem_index;
	DeeMH_seq_clear_t mh_seq_clear;
	DeeMH_seq_pop_t mh_seq_pop;
/*[[[end]]]*/
	/* clang-format on */

	/* Method hint attribute data caches.
	 * All of these are [0..1][lock(WRITE_ONCE)] */

	/* clang-format off */
/*[[[deemon (printMhCacheAttributeMembers from "..method-hints.method-hints")();]]]*/
#define MHC_COUNT 38
#define MHC_FIRST mhc___seq_bool__
	union mhc_slot mhc___seq_bool__;
	union mhc_slot mhc___seq_size__;
	union mhc_slot mhc___seq_iter__;
	union mhc_slot mhc___seq_iterkeys__;
	union mhc_slot mhc___seq_enumerate__;
	union mhc_slot mhc___seq_getitem__;
	union mhc_slot mhc___seq_delitem__;
	union mhc_slot mhc___seq_setitem__;
	union mhc_slot mhc___seq_getrange__;
	union mhc_slot mhc___seq_delrange__;
	union mhc_slot mhc___seq_setrange__;
	union mhc_slot mhc___seq_hash__;
	union mhc_slot mhc___seq_compare__;
	union mhc_slot mhc___seq_compare_eq__;
	union mhc_slot mhc___seq_trycompare_eq__;
	union mhc_slot mhc___seq_eq__;
	union mhc_slot mhc___seq_ne__;
	union mhc_slot mhc___seq_lo__;
	union mhc_slot mhc___seq_le__;
	union mhc_slot mhc___seq_gr__;
	union mhc_slot mhc___seq_ge__;
	union mhc_slot mhc_get___seq_first__;
	union mhc_slot mhc_del___seq_first__;
	union mhc_slot mhc_set___seq_first__;
	union mhc_slot mhc_get___seq_last__;
	union mhc_slot mhc_del___seq_last__;
	union mhc_slot mhc_set___seq_last__;
	union mhc_slot mhc___seq_any__;
	union mhc_slot mhc___seq_all__;
	union mhc_slot mhc___seq_erase__;
	union mhc_slot mhc___seq_insert__;
	union mhc_slot mhc___seq_insertall__;
	union mhc_slot mhc___seq_pushfront__;
	union mhc_slot mhc___seq_append__;
	union mhc_slot mhc___seq_extend__;
	union mhc_slot mhc___seq_xchitem__;
	union mhc_slot mhc___seq_clear__;
	union mhc_slot mhc___seq_pop__;
#define MHC_LAST mhc___seq_pop__
/*[[[end]]]*/
	/* clang-format on */
};

#define Dee_type_mh_cache_alloc() \
	((struct Dee_type_mh_cache *)Dee_TryCalloc(sizeof(struct Dee_type_mh_cache)))
#define Dee_type_mh_cache_free(self) Dee_Free(self)

INTDEF NONNULL((1)) void DCALL
Dee_type_mh_cache_destroy(struct Dee_type_mh_cache *__restrict self);

INTDEF WUNUSED NONNULL((1)) struct Dee_type_mh_cache *DCALL
Dee_type_mh_cache_of(DeeTypeObject *__restrict self);

INTDEF Dee_funptr_t tpconst mh_unsupported_impls[Dee_TMH_COUNT];
#define DeeType_GetUnsupportedMethodHint(id) mh_unsupported_impls[id]

DECL_END
#endif /* CONFIG_EXPERIMENTAL_UNIFIED_METHOD_HINTS */

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINTS_H */
