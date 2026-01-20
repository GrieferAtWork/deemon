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
#ifndef GUARD_DEEMON_RUNTIME_METHOD_HINTS_H
#define GUARD_DEEMON_RUNTIME_METHOD_HINTS_H 1

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/method-hints.h>
#include <deemon/object.h>

#include <hybrid/byteorder.h> /* __BYTE_ORDER__, __ORDER_LITTLE_ENDIAN__ */

#include <stdint.h> /* uint16_t, uintptr_t */

DECL_BEGIN

#if (!defined(CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE) && \
     !defined(CONFIG_NO_STRUCT_OBJECT_FIELD_CACHE))
#if defined(__OPTIMIZE_SIZE__)
#define CONFIG_NO_STRUCT_OBJECT_FIELD_CACHE
#else /* __OPTIMIZE_SIZE__ */
#define CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE
#endif /* !__OPTIMIZE_SIZE__ */
#endif /* ... */

/* Cache for "DeeStructObject_ForeachField()" & friends */
#ifdef CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE
struct Dee_type_struct_field {
	struct Dee_type_member const *tsf_member;   /* [1..1] Field in question ("NULL" here is used as sentinel) */
	DeeTypeObject                *tsf_decltype; /* [1..1] Declaring type */
};
struct Dee_type_struct_cache {
	struct Dee_type_struct_field  *tsc_allfields; /* [0..N][lock(WRITE_ONCE)][owned] Cache for fields enumerated by `DeeStructObject_ForeachField()' */
	struct Dee_type_member const **tsc_locfields; /* [0..N][lock(WRITE_ONCE)][owned] Cache for fields enumerated by `DeeStructObject_Visit()' / `DeeStructObject_Fini()' */
};

#define Dee_type_struct_cache_alloc() \
	((struct Dee_type_struct_cache *)Dee_TryCalloc(sizeof(struct Dee_type_struct_cache)))
#define Dee_type_struct_cache_free(self) Dee_Free(self)
LOCAL NONNULL((1)) void DCALL
Dee_type_struct_cache_destroy(struct Dee_type_struct_cache *__restrict self) {
	Dee_Free(self->tsc_allfields);
	Dee_Free(self->tsc_locfields);
	Dee_type_struct_cache_free(self);
}
#endif /* CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE */




/* Address in `:tp_class->cb_members'.
 *
 * Q: Why do we store class member address here, and not the actual
 *    references to the underlying methods?
 * A: Semantically speaking, it would be possible to do that, but
 *    in practice that would form an unresolvable reference loop
 *    in code like this:
 *    >> local class MyClass1 { this = default; final member mySize; __seq_size__() -> mySize; }
 *    >> local class MyClass2: MyClass1 { this = super; __seq_size__() -> 42; }
 *    >>
 *    >> local a = MyClass1(10);
 *    >> local b = MyClass2(20);
 *    >>
 *    >> print Sequence.length(a);             // OK: 10
 *    >> print Sequence.length(b);             // OK: 42
 *    >> print Sequence.length(b as MyClass1); // OK: 20
 *    ^ When this program exits, because "MyClass1.mySize" is "final",
 *    "MyClass1.__seq_size__" will access it using "ASM_GETMEMBER_THIS_R",
 *    meaning that "MyClass1" appears in "MyClass1.__seq_size__.__refs__",
 *    and thus forms a reference loop with "MyClass1" (one that cannot be
 *    resolved because when "MyClass1" is cleared, we don't drop the cached
 *    reference to "MyClass1.__seq_size__")
 *    So to prevent this from happening, we do the next-best thing by just
 *    storing the addresses where relevant callbacks can be found in the
 *    type's class member table.
 */
typedef uint16_t Dee_mhc_slot_t;

union Dee_mhc_traits {
	uintptr_t            mht_word; /* Status word */
	struct {
		/* Make it so "tt_load" is always the lower half of "mht_word" */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		Dee_type_trait_t tt_load;  /* Set of loaded traits */
		Dee_type_trait_t tt_have;  /* Set of supported traits */
#else /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
		Dee_type_trait_t tt_have;  /* Set of supported traits */
		Dee_type_trait_t tt_load;  /* Set of loaded traits */
#endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
	} mht_traits;
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
	DeeMH_seq_operator_getitem_t mh_seq_operator_getitem;
	DeeMH_seq_operator_getitem_index_t mh_seq_operator_getitem_index;
	DeeMH_seq_operator_trygetitem_t mh_seq_operator_trygetitem;
	DeeMH_seq_operator_trygetitem_index_t mh_seq_operator_trygetitem_index;
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
	DeeMH_seq_operator_assign_t mh_seq_operator_assign;
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
	DeeMH_seq_operator_add_t mh_seq_operator_add;
	DeeMH_seq_operator_mul_t mh_seq_operator_mul;
	DeeMH_seq_operator_inplace_add_t mh_seq_operator_inplace_add;
	DeeMH_seq_operator_inplace_mul_t mh_seq_operator_inplace_mul;
	DeeMH_seq_enumerate_t mh_seq_enumerate;
	DeeMH_seq_enumerate_index_t mh_seq_enumerate_index;
	DeeMH_seq_makeenumeration_t mh_seq_makeenumeration;
	DeeMH_seq_makeenumeration_with_range_t mh_seq_makeenumeration_with_range;
	DeeMH_seq_makeenumeration_with_intrange_t mh_seq_makeenumeration_with_intrange;
	DeeMH_seq_foreach_reverse_t mh_seq_foreach_reverse;
	DeeMH_seq_enumerate_index_reverse_t mh_seq_enumerate_index_reverse;
	DeeMH_seq_unpack_t mh_seq_unpack;
	DeeMH_seq_unpack_ex_t mh_seq_unpack_ex;
	DeeMH_seq_unpack_ub_t mh_seq_unpack_ub;
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
	DeeMH_seq_cached_t mh_seq_cached;
	DeeMH_seq_frozen_t mh_seq_frozen;
	DeeMH_seq_any_t mh_seq_any;
	DeeMH_seq_any_with_key_t mh_seq_any_with_key;
	DeeMH_seq_any_with_range_t mh_seq_any_with_range;
	DeeMH_seq_any_with_range_and_key_t mh_seq_any_with_range_and_key;
	DeeMH_seq_all_t mh_seq_all;
	DeeMH_seq_all_with_key_t mh_seq_all_with_key;
	DeeMH_seq_all_with_range_t mh_seq_all_with_range;
	DeeMH_seq_all_with_range_and_key_t mh_seq_all_with_range_and_key;
	DeeMH_seq_parity_t mh_seq_parity;
	DeeMH_seq_parity_with_key_t mh_seq_parity_with_key;
	DeeMH_seq_parity_with_range_t mh_seq_parity_with_range;
	DeeMH_seq_parity_with_range_and_key_t mh_seq_parity_with_range_and_key;
	DeeMH_seq_reduce_t mh_seq_reduce;
	DeeMH_seq_reduce_with_init_t mh_seq_reduce_with_init;
	DeeMH_seq_reduce_with_range_t mh_seq_reduce_with_range;
	DeeMH_seq_reduce_with_range_and_init_t mh_seq_reduce_with_range_and_init;
	DeeMH_seq_min_t mh_seq_min;
	DeeMH_seq_min_with_key_t mh_seq_min_with_key;
	DeeMH_seq_min_with_range_t mh_seq_min_with_range;
	DeeMH_seq_min_with_range_and_key_t mh_seq_min_with_range_and_key;
	DeeMH_seq_max_t mh_seq_max;
	DeeMH_seq_max_with_key_t mh_seq_max_with_key;
	DeeMH_seq_max_with_range_t mh_seq_max_with_range;
	DeeMH_seq_max_with_range_and_key_t mh_seq_max_with_range_and_key;
	DeeMH_seq_sum_t mh_seq_sum;
	DeeMH_seq_sum_with_range_t mh_seq_sum_with_range;
	DeeMH_seq_count_t mh_seq_count;
	DeeMH_seq_count_with_key_t mh_seq_count_with_key;
	DeeMH_seq_count_with_range_t mh_seq_count_with_range;
	DeeMH_seq_count_with_range_and_key_t mh_seq_count_with_range_and_key;
	DeeMH_seq_contains_t mh_seq_contains;
	DeeMH_seq_contains_with_key_t mh_seq_contains_with_key;
	DeeMH_seq_contains_with_range_t mh_seq_contains_with_range;
	DeeMH_seq_contains_with_range_and_key_t mh_seq_contains_with_range_and_key;
	DeeMH_seq_operator_contains_t mh_seq_operator_contains;
	DeeMH_seq_locate_t mh_seq_locate;
	DeeMH_seq_locate_with_range_t mh_seq_locate_with_range;
	DeeMH_seq_rlocate_t mh_seq_rlocate;
	DeeMH_seq_rlocate_with_range_t mh_seq_rlocate_with_range;
	DeeMH_seq_startswith_t mh_seq_startswith;
	DeeMH_seq_startswith_with_key_t mh_seq_startswith_with_key;
	DeeMH_seq_startswith_with_range_t mh_seq_startswith_with_range;
	DeeMH_seq_startswith_with_range_and_key_t mh_seq_startswith_with_range_and_key;
	DeeMH_seq_endswith_t mh_seq_endswith;
	DeeMH_seq_endswith_with_key_t mh_seq_endswith_with_key;
	DeeMH_seq_endswith_with_range_t mh_seq_endswith_with_range;
	DeeMH_seq_endswith_with_range_and_key_t mh_seq_endswith_with_range_and_key;
	DeeMH_seq_find_t mh_seq_find;
	DeeMH_seq_find_with_key_t mh_seq_find_with_key;
	DeeMH_seq_rfind_t mh_seq_rfind;
	DeeMH_seq_rfind_with_key_t mh_seq_rfind_with_key;
	DeeMH_seq_erase_t mh_seq_erase;
	DeeMH_seq_insert_t mh_seq_insert;
	DeeMH_seq_insertall_t mh_seq_insertall;
	DeeMH_seq_pushfront_t mh_seq_pushfront;
	DeeMH_seq_append_t mh_seq_append;
	DeeMH_seq_extend_t mh_seq_extend;
	DeeMH_seq_xchitem_index_t mh_seq_xchitem_index;
	DeeMH_seq_clear_t mh_seq_clear;
	DeeMH_seq_pop_t mh_seq_pop;
	DeeMH_seq_remove_t mh_seq_remove;
	DeeMH_seq_remove_with_key_t mh_seq_remove_with_key;
	DeeMH_seq_rremove_t mh_seq_rremove;
	DeeMH_seq_rremove_with_key_t mh_seq_rremove_with_key;
	DeeMH_seq_removeall_t mh_seq_removeall;
	DeeMH_seq_removeall_with_key_t mh_seq_removeall_with_key;
	DeeMH_seq_removeif_t mh_seq_removeif;
	DeeMH_seq_resize_t mh_seq_resize;
	DeeMH_seq_fill_t mh_seq_fill;
	DeeMH_seq_reverse_t mh_seq_reverse;
	DeeMH_seq_reversed_t mh_seq_reversed;
	DeeMH_seq_sort_t mh_seq_sort;
	DeeMH_seq_sort_with_key_t mh_seq_sort_with_key;
	DeeMH_seq_sorted_t mh_seq_sorted;
	DeeMH_seq_sorted_with_key_t mh_seq_sorted_with_key;
	DeeMH_seq_bfind_t mh_seq_bfind;
	DeeMH_seq_bfind_with_key_t mh_seq_bfind_with_key;
	DeeMH_seq_bposition_t mh_seq_bposition;
	DeeMH_seq_bposition_with_key_t mh_seq_bposition_with_key;
	DeeMH_seq_brange_t mh_seq_brange;
	DeeMH_seq_brange_with_key_t mh_seq_brange_with_key;
	DeeMH_set_operator_iter_t mh_set_operator_iter;
	DeeMH_set_operator_foreach_t mh_set_operator_foreach;
	DeeMH_set_operator_sizeob_t mh_set_operator_sizeob;
	DeeMH_set_operator_size_t mh_set_operator_size;
	DeeMH_set_operator_hash_t mh_set_operator_hash;
	DeeMH_set_operator_compare_eq_t mh_set_operator_compare_eq;
	DeeMH_set_operator_trycompare_eq_t mh_set_operator_trycompare_eq;
	DeeMH_set_operator_eq_t mh_set_operator_eq;
	DeeMH_set_operator_ne_t mh_set_operator_ne;
	DeeMH_set_operator_lo_t mh_set_operator_lo;
	DeeMH_set_operator_le_t mh_set_operator_le;
	DeeMH_set_operator_gr_t mh_set_operator_gr;
	DeeMH_set_operator_ge_t mh_set_operator_ge;
	DeeMH_set_operator_bool_t mh_set_operator_bool;
	DeeMH_set_operator_inv_t mh_set_operator_inv;
	DeeMH_set_operator_add_t mh_set_operator_add;
	DeeMH_set_operator_sub_t mh_set_operator_sub;
	DeeMH_set_operator_and_t mh_set_operator_and;
	DeeMH_set_operator_xor_t mh_set_operator_xor;
	DeeMH_set_operator_inplace_add_t mh_set_operator_inplace_add;
	DeeMH_set_operator_inplace_sub_t mh_set_operator_inplace_sub;
	DeeMH_set_operator_inplace_and_t mh_set_operator_inplace_and;
	DeeMH_set_operator_inplace_xor_t mh_set_operator_inplace_xor;
	DeeMH_set_frozen_t mh_set_frozen;
	DeeMH_set_unify_t mh_set_unify;
	DeeMH_set_insert_t mh_set_insert;
	DeeMH_set_insertall_t mh_set_insertall;
	DeeMH_set_remove_t mh_set_remove;
	DeeMH_set_removeall_t mh_set_removeall;
	DeeMH_set_pop_t mh_set_pop;
	DeeMH_set_pop_with_default_t mh_set_pop_with_default;
	DeeMH_set_trygetfirst_t mh_set_trygetfirst;
	DeeMH_set_getfirst_t mh_set_getfirst;
	DeeMH_set_boundfirst_t mh_set_boundfirst;
	DeeMH_set_delfirst_t mh_set_delfirst;
	DeeMH_set_setfirst_t mh_set_setfirst;
	DeeMH_set_trygetlast_t mh_set_trygetlast;
	DeeMH_set_getlast_t mh_set_getlast;
	DeeMH_set_boundlast_t mh_set_boundlast;
	DeeMH_set_dellast_t mh_set_dellast;
	DeeMH_set_setlast_t mh_set_setlast;
	DeeMH_map_operator_iter_t mh_map_operator_iter;
	DeeMH_map_operator_foreach_pair_t mh_map_operator_foreach_pair;
	DeeMH_map_operator_sizeob_t mh_map_operator_sizeob;
	DeeMH_map_operator_size_t mh_map_operator_size;
	DeeMH_map_operator_hash_t mh_map_operator_hash;
	DeeMH_map_operator_getitem_t mh_map_operator_getitem;
	DeeMH_map_operator_trygetitem_t mh_map_operator_trygetitem;
	DeeMH_map_operator_getitem_index_t mh_map_operator_getitem_index;
	DeeMH_map_operator_trygetitem_index_t mh_map_operator_trygetitem_index;
	DeeMH_map_operator_getitem_string_hash_t mh_map_operator_getitem_string_hash;
	DeeMH_map_operator_trygetitem_string_hash_t mh_map_operator_trygetitem_string_hash;
	DeeMH_map_operator_getitem_string_len_hash_t mh_map_operator_getitem_string_len_hash;
	DeeMH_map_operator_trygetitem_string_len_hash_t mh_map_operator_trygetitem_string_len_hash;
	DeeMH_map_operator_bounditem_t mh_map_operator_bounditem;
	DeeMH_map_operator_bounditem_index_t mh_map_operator_bounditem_index;
	DeeMH_map_operator_bounditem_string_hash_t mh_map_operator_bounditem_string_hash;
	DeeMH_map_operator_bounditem_string_len_hash_t mh_map_operator_bounditem_string_len_hash;
	DeeMH_map_operator_hasitem_t mh_map_operator_hasitem;
	DeeMH_map_operator_hasitem_index_t mh_map_operator_hasitem_index;
	DeeMH_map_operator_hasitem_string_hash_t mh_map_operator_hasitem_string_hash;
	DeeMH_map_operator_hasitem_string_len_hash_t mh_map_operator_hasitem_string_len_hash;
	DeeMH_map_operator_delitem_t mh_map_operator_delitem;
	DeeMH_map_operator_delitem_index_t mh_map_operator_delitem_index;
	DeeMH_map_operator_delitem_string_hash_t mh_map_operator_delitem_string_hash;
	DeeMH_map_operator_delitem_string_len_hash_t mh_map_operator_delitem_string_len_hash;
	DeeMH_map_operator_setitem_t mh_map_operator_setitem;
	DeeMH_map_operator_setitem_index_t mh_map_operator_setitem_index;
	DeeMH_map_operator_setitem_string_hash_t mh_map_operator_setitem_string_hash;
	DeeMH_map_operator_setitem_string_len_hash_t mh_map_operator_setitem_string_len_hash;
	DeeMH_map_operator_contains_t mh_map_operator_contains;
	DeeMH_map_keys_t mh_map_keys;
	DeeMH_map_iterkeys_t mh_map_iterkeys;
	DeeMH_map_values_t mh_map_values;
	DeeMH_map_itervalues_t mh_map_itervalues;
	DeeMH_map_enumerate_t mh_map_enumerate;
	DeeMH_map_enumerate_range_t mh_map_enumerate_range;
	DeeMH_map_makeenumeration_t mh_map_makeenumeration;
	DeeMH_map_makeenumeration_with_range_t mh_map_makeenumeration_with_range;
	DeeMH_map_operator_compare_eq_t mh_map_operator_compare_eq;
	DeeMH_map_operator_trycompare_eq_t mh_map_operator_trycompare_eq;
	DeeMH_map_operator_eq_t mh_map_operator_eq;
	DeeMH_map_operator_ne_t mh_map_operator_ne;
	DeeMH_map_operator_lo_t mh_map_operator_lo;
	DeeMH_map_operator_le_t mh_map_operator_le;
	DeeMH_map_operator_gr_t mh_map_operator_gr;
	DeeMH_map_operator_ge_t mh_map_operator_ge;
	DeeMH_map_operator_add_t mh_map_operator_add;
	DeeMH_map_operator_sub_t mh_map_operator_sub;
	DeeMH_map_operator_and_t mh_map_operator_and;
	DeeMH_map_operator_xor_t mh_map_operator_xor;
	DeeMH_map_operator_inplace_add_t mh_map_operator_inplace_add;
	DeeMH_map_operator_inplace_sub_t mh_map_operator_inplace_sub;
	DeeMH_map_operator_inplace_and_t mh_map_operator_inplace_and;
	DeeMH_map_operator_inplace_xor_t mh_map_operator_inplace_xor;
	DeeMH_map_frozen_t mh_map_frozen;
	DeeMH_map_setold_t mh_map_setold;
	DeeMH_map_setold_ex_t mh_map_setold_ex;
	DeeMH_map_setnew_t mh_map_setnew;
	DeeMH_map_setnew_ex_t mh_map_setnew_ex;
	DeeMH_map_setdefault_t mh_map_setdefault;
	DeeMH_map_update_t mh_map_update;
	DeeMH_map_remove_t mh_map_remove;
	DeeMH_map_removekeys_t mh_map_removekeys;
	DeeMH_map_pop_t mh_map_pop;
	DeeMH_map_pop_with_default_t mh_map_pop_with_default;
	DeeMH_map_popitem_t mh_map_popitem;
/*[[[end]]]*/
	/* clang-format on */

	union Dee_mhc_traits mh_explicit_traits; /* Explicitly defined type traits */
	union Dee_mhc_traits mh_traits;          /* All type traits (including implicit ones) */

	/* clang-format off */
/*[[[deemon (printMhCacheAttributeMembers from "..method-hints.method-hints")();]]]*/
#define MHC_COUNT 144
#define MHC_FIRST mhc___seq_bool__
	Dee_mhc_slot_t mhc___seq_bool__;
	Dee_mhc_slot_t mhc___seq_size__;
	Dee_mhc_slot_t mhc___seq_iter__;
	Dee_mhc_slot_t mhc___seq_getitem__;
	Dee_mhc_slot_t mhc___seq_delitem__;
	Dee_mhc_slot_t mhc___seq_setitem__;
	Dee_mhc_slot_t mhc___seq_getrange__;
	Dee_mhc_slot_t mhc___seq_delrange__;
	Dee_mhc_slot_t mhc___seq_setrange__;
	Dee_mhc_slot_t mhc___seq_assign__;
	Dee_mhc_slot_t mhc___seq_hash__;
	Dee_mhc_slot_t mhc___seq_compare__;
	Dee_mhc_slot_t mhc___seq_compare_eq__;
	Dee_mhc_slot_t mhc___seq_eq__;
	Dee_mhc_slot_t mhc___seq_ne__;
	Dee_mhc_slot_t mhc___seq_lo__;
	Dee_mhc_slot_t mhc___seq_le__;
	Dee_mhc_slot_t mhc___seq_gr__;
	Dee_mhc_slot_t mhc___seq_ge__;
	Dee_mhc_slot_t mhc___seq_add__;
	Dee_mhc_slot_t mhc___seq_mul__;
	Dee_mhc_slot_t mhc___seq_inplace_add__;
	Dee_mhc_slot_t mhc___seq_inplace_mul__;
	Dee_mhc_slot_t mhc___seq_enumerate__;
	Dee_mhc_slot_t mhc___seq_enumerate_items__;
	Dee_mhc_slot_t mhc___seq_unpack__;
	Dee_mhc_slot_t mhc___seq_unpackub__;
	Dee_mhc_slot_t mhc_get___seq_first__;
	Dee_mhc_slot_t mhc_del___seq_first__;
	Dee_mhc_slot_t mhc_set___seq_first__;
	Dee_mhc_slot_t mhc_get___seq_last__;
	Dee_mhc_slot_t mhc_del___seq_last__;
	Dee_mhc_slot_t mhc_set___seq_last__;
	Dee_mhc_slot_t mhc_get___seq_cached__;
	Dee_mhc_slot_t mhc_get___seq_frozen__;
	Dee_mhc_slot_t mhc___seq_any__;
	Dee_mhc_slot_t mhc___seq_all__;
	Dee_mhc_slot_t mhc___seq_parity__;
	Dee_mhc_slot_t mhc___seq_reduce__;
	Dee_mhc_slot_t mhc___seq_min__;
	Dee_mhc_slot_t mhc___seq_max__;
	Dee_mhc_slot_t mhc___seq_sum__;
	Dee_mhc_slot_t mhc___seq_count__;
	Dee_mhc_slot_t mhc___seq_contains__;
	Dee_mhc_slot_t mhc___seq_locate__;
	Dee_mhc_slot_t mhc___seq_rlocate__;
	Dee_mhc_slot_t mhc___seq_startswith__;
	Dee_mhc_slot_t mhc___seq_endswith__;
	Dee_mhc_slot_t mhc___seq_find__;
	Dee_mhc_slot_t mhc___seq_rfind__;
	Dee_mhc_slot_t mhc___seq_erase__;
	Dee_mhc_slot_t mhc___seq_insert__;
	Dee_mhc_slot_t mhc___seq_insertall__;
	Dee_mhc_slot_t mhc___seq_pushfront__;
	Dee_mhc_slot_t mhc___seq_append__;
	Dee_mhc_slot_t mhc___seq_extend__;
	Dee_mhc_slot_t mhc___seq_xchitem__;
	Dee_mhc_slot_t mhc___seq_clear__;
	Dee_mhc_slot_t mhc___seq_pop__;
	Dee_mhc_slot_t mhc___seq_remove__;
	Dee_mhc_slot_t mhc___seq_rremove__;
	Dee_mhc_slot_t mhc___seq_removeall__;
	Dee_mhc_slot_t mhc___seq_removeif__;
	Dee_mhc_slot_t mhc___seq_resize__;
	Dee_mhc_slot_t mhc___seq_fill__;
	Dee_mhc_slot_t mhc___seq_reverse__;
	Dee_mhc_slot_t mhc___seq_reversed__;
	Dee_mhc_slot_t mhc___seq_sort__;
	Dee_mhc_slot_t mhc___seq_sorted__;
	Dee_mhc_slot_t mhc___seq_bfind__;
	Dee_mhc_slot_t mhc___seq_bposition__;
	Dee_mhc_slot_t mhc___seq_brange__;
	Dee_mhc_slot_t mhc___set_iter__;
	Dee_mhc_slot_t mhc___set_size__;
	Dee_mhc_slot_t mhc___set_hash__;
	Dee_mhc_slot_t mhc___set_compare_eq__;
	Dee_mhc_slot_t mhc___set_eq__;
	Dee_mhc_slot_t mhc___set_ne__;
	Dee_mhc_slot_t mhc___set_lo__;
	Dee_mhc_slot_t mhc___set_le__;
	Dee_mhc_slot_t mhc___set_gr__;
	Dee_mhc_slot_t mhc___set_ge__;
	Dee_mhc_slot_t mhc___set_bool__;
	Dee_mhc_slot_t mhc___set_inv__;
	Dee_mhc_slot_t mhc___set_add__;
	Dee_mhc_slot_t mhc___set_sub__;
	Dee_mhc_slot_t mhc___set_and__;
	Dee_mhc_slot_t mhc___set_xor__;
	Dee_mhc_slot_t mhc___set_inplace_add__;
	Dee_mhc_slot_t mhc___set_inplace_sub__;
	Dee_mhc_slot_t mhc___set_inplace_and__;
	Dee_mhc_slot_t mhc___set_inplace_xor__;
	Dee_mhc_slot_t mhc_get___set_frozen__;
	Dee_mhc_slot_t mhc___set_unify__;
	Dee_mhc_slot_t mhc___set_insert__;
	Dee_mhc_slot_t mhc___set_insertall__;
	Dee_mhc_slot_t mhc___set_remove__;
	Dee_mhc_slot_t mhc___set_removeall__;
	Dee_mhc_slot_t mhc___set_pop__;
	Dee_mhc_slot_t mhc_get___set_first__;
	Dee_mhc_slot_t mhc_del___set_first__;
	Dee_mhc_slot_t mhc_set___set_first__;
	Dee_mhc_slot_t mhc_get___set_last__;
	Dee_mhc_slot_t mhc_del___set_last__;
	Dee_mhc_slot_t mhc_set___set_last__;
	Dee_mhc_slot_t mhc___map_iter__;
	Dee_mhc_slot_t mhc___map_size__;
	Dee_mhc_slot_t mhc___map_hash__;
	Dee_mhc_slot_t mhc___map_getitem__;
	Dee_mhc_slot_t mhc___map_delitem__;
	Dee_mhc_slot_t mhc___map_setitem__;
	Dee_mhc_slot_t mhc___map_contains__;
	Dee_mhc_slot_t mhc_get___map_keys__;
	Dee_mhc_slot_t mhc_get___map_iterkeys__;
	Dee_mhc_slot_t mhc_get___map_values__;
	Dee_mhc_slot_t mhc_get___map_itervalues__;
	Dee_mhc_slot_t mhc___map_enumerate__;
	Dee_mhc_slot_t mhc___map_enumerate_items__;
	Dee_mhc_slot_t mhc___map_compare_eq__;
	Dee_mhc_slot_t mhc___map_eq__;
	Dee_mhc_slot_t mhc___map_ne__;
	Dee_mhc_slot_t mhc___map_lo__;
	Dee_mhc_slot_t mhc___map_le__;
	Dee_mhc_slot_t mhc___map_gr__;
	Dee_mhc_slot_t mhc___map_ge__;
	Dee_mhc_slot_t mhc___map_add__;
	Dee_mhc_slot_t mhc___map_sub__;
	Dee_mhc_slot_t mhc___map_and__;
	Dee_mhc_slot_t mhc___map_xor__;
	Dee_mhc_slot_t mhc___map_inplace_add__;
	Dee_mhc_slot_t mhc___map_inplace_sub__;
	Dee_mhc_slot_t mhc___map_inplace_and__;
	Dee_mhc_slot_t mhc___map_inplace_xor__;
	Dee_mhc_slot_t mhc_get___map_frozen__;
	Dee_mhc_slot_t mhc___map_setold__;
	Dee_mhc_slot_t mhc___map_setold_ex__;
	Dee_mhc_slot_t mhc___map_setnew__;
	Dee_mhc_slot_t mhc___map_setnew_ex__;
	Dee_mhc_slot_t mhc___map_setdefault__;
	Dee_mhc_slot_t mhc___map_update__;
	Dee_mhc_slot_t mhc___map_remove__;
	Dee_mhc_slot_t mhc___map_removekeys__;
	Dee_mhc_slot_t mhc___map_pop__;
	Dee_mhc_slot_t mhc___map_popitem__;
#define MHC_LAST mhc___map_popitem__
/*[[[end]]]*/
	/* clang-format on */

	/* Other non-generated type cache data goes here... */
#ifdef CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE
	struct Dee_type_struct_cache *mhc_structcache; /* [0..1][lock(WRITE_ONCE)][owned] Struct enumeration cache */
#define _Dee_type_mh_cache_fini_structcache_(self)           \
	unlikely((self)->mhc_structcache)                        \
	? Dee_type_struct_cache_destroy((self)->mhc_structcache) \
	: (void)0,
#else /* CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE */
#define _Dee_type_mh_cache_fini_structcache_(self) /* nothing */
#endif /* !CONFIG_HAVE_STRUCT_OBJECT_FIELD_CACHE */
};


#define Dee_type_mh_cache_alloc() \
	((struct Dee_type_mh_cache *)Dee_TryCalloc(sizeof(struct Dee_type_mh_cache)))
#define Dee_type_mh_cache_free(self) Dee_Free(self)
#define Dee_type_mh_cache_destroy(self)         \
	(_Dee_type_mh_cache_fini_structcache_(self) \
	 Dee_type_mh_cache_free(self))

INTDEF WUNUSED NONNULL((1)) struct Dee_type_mh_cache *DCALL
Dee_type_mh_cache_of(DeeTypeObject *__restrict self);

INTDEF Dee_funptr_t tpconst mh_unsupported_impls[Dee_TMH_COUNT];
#define DeeType_GetUnsupportedMethodHint(id) mh_unsupported_impls[id]

/* Statically initialized method cache where all hints use their "$empty" impl (if defined) */
INTDEF struct Dee_type_mh_cache mh_cache_empty;


/* Allowed to assume that "seqclass != Dee_SEQCLASS_UNKNOWN" */
#define DeeType_IsSeqClassBase(self, seqclass) \
	((self) == _Dee_SEQCLASS_BASES[(seqclass) - 1])
INTDEF DeeTypeObject const *tpconst _Dee_SEQCLASS_BASES[];

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_METHOD_HINTS_H */
