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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_H
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>

DECL_BEGIN

/* How default API functions from "Sequence", "Set" and "Mapping" are implemented.
 *
 * This structure gets lazily calculated when it is first needed, based on features
 * exhibited by the respective sequence type.
 *
 * Individual function pointers within this structure are all NULL by default, and
 * populated as they are needed (meaning they are `[0..1][lock(WRITE_ONCE)]').
 * Unless otherwise documented, there is always a default available for *all* of
 * these operators.
 *
 *
 * NOTE:
 * - "Default" API functions can be overwritten by sub-classes (the runtime checks
 *   for this the first type a call is made, and will select default implementations
 *   based on which functions are overwritten)
 * - "Generic" API functions can NOT be overwritten (or if they are: the runtime
 *   probably won't actually use them). "Generic" API function will often try to
 *   make use of other APIs (usually Default ones, or operators) for their impl.
 */
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *Dee_tsc_foreach_reverse_t)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *Dee_tsc_enumerate_index_t)(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *Dee_tsc_enumerate_index_reverse_t)(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_nonempty_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_getfirst_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_boundfirst_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_delfirst_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_setfirst_t)(DeeObject *self, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_getlast_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_boundlast_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_dellast_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_setlast_t)(DeeObject *self, DeeObject *value);

/* Functions that need additional variants for sequence sub-types that don't have indices (sets, maps) */
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_any_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_any_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_any_with_range_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *Dee_tsc_any_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_all_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_all_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_all_with_range_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *Dee_tsc_all_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_parity_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_parity_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_parity_with_range_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *Dee_tsc_parity_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_reduce_t)(DeeObject *self, DeeObject *combine);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *Dee_tsc_reduce_with_init_t)(DeeObject *self, DeeObject *combine, DeeObject *init);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_reduce_with_range_t)(DeeObject *self, DeeObject *combine, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) DREF DeeObject *(DCALL *Dee_tsc_reduce_with_range_and_init_t)(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);

typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_min_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_min_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_min_with_range_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) DREF DeeObject *(DCALL *Dee_tsc_min_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_max_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_max_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_max_with_range_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) DREF DeeObject *(DCALL *Dee_tsc_max_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_sum_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_sum_with_range_t)(DeeObject *self, size_t start, size_t end);

typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_count_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) size_t (DCALL *Dee_tsc_count_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_count_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *Dee_tsc_count_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_contains_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *Dee_tsc_contains_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_contains_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *Dee_tsc_contains_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_locate_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *Dee_tsc_locate_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_locate_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) DREF DeeObject *(DCALL *Dee_tsc_locate_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/*typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_rlocate_t)(DeeObject *self, DeeObject *item);*/ /* Wouldn't make sense: for reverse, you need indices */
/*typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *Dee_tsc_rlocate_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);*/ /* Wouldn't make sense: for reverse, you need indices */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_rlocate_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) DREF DeeObject *(DCALL *Dee_tsc_rlocate_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_startswith_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *Dee_tsc_startswith_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_startswith_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *Dee_tsc_startswith_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_endswith_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *Dee_tsc_endswith_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_endswith_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *Dee_tsc_endswith_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);


/* @return: * :         Index of `item' in `self'
 * @return: (size_t)-1: `item' could not be located in `self'
 * @return: (size_t)Dee_COMPARE_ERR: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_find_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *Dee_tsc_find_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_rfind_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *Dee_tsc_rfind_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* Functions for mutable sequences. */
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_erase_t)(DeeObject *self, size_t index, size_t count);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *Dee_tsc_insert_t)(DeeObject *self, size_t index, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *Dee_tsc_insertall_t)(DeeObject *self, size_t index, DeeObject *items);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_pushfront_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_append_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_extend_t)(DeeObject *self, DeeObject *items);
typedef WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *Dee_tsc_xchitem_index_t)(DeeObject *self, size_t index, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_clear_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_pop_t)(DeeObject *self, Dee_ssize_t index); /* When index is negative, count from end of sequence */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_remove_t)(DeeObject *self, DeeObject *item, size_t start, size_t end); /* @return: 1: Removed; 0: Not removed; -1: Error */
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *Dee_tsc_remove_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key); /* @return: 1: Removed; 0: Not removed; -1: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_rremove_t)(DeeObject *self, DeeObject *item, size_t start, size_t end); /* @return: 1: Removed; 0: Not removed; -1: Error */
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *Dee_tsc_rremove_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key); /* @return: 1: Removed; 0: Not removed; -1: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_removeall_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
typedef WUNUSED_T NONNULL_T((1, 2, 6)) size_t (DCALL *Dee_tsc_removeall_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_removeif_t)(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *Dee_tsc_resize_t)(DeeObject *self, size_t newsize, DeeObject *filler);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *Dee_tsc_fill_t)(DeeObject *self, size_t start, size_t end, DeeObject *filler);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_reverse_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_reversed_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_sort_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *Dee_tsc_sort_with_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_sorted_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) DREF DeeObject *(DCALL *Dee_tsc_sorted_with_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_bfind_t)(DeeObject *self, DeeObject *item, size_t start, size_t end); /* @return: (size_t)-1: Not found; @return (size_t)Dee_COMPARE_ERR: Error */
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *Dee_tsc_bfind_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key); /* @return: (size_t)-1: Not found; @return (size_t)Dee_COMPARE_ERR: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_bposition_t)(DeeObject *self, DeeObject *item, size_t start, size_t end); /* @return: (size_t)Dee_COMPARE_ERR: Error */
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *Dee_tsc_bposition_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key); /* @return: (size_t)Dee_COMPARE_ERR: Error */
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *Dee_tsc_brange_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
typedef WUNUSED_T NONNULL_T((1, 2, 5, 6)) int (DCALL *Dee_tsc_brange_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_blocate_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) DREF DeeObject *(DCALL *Dee_tsc_blocate_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* Operators for the purpose of constructing `DefaultEnumeration_With*' objects. */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_makeenumeration_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_makeenumeration_with_int_range_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *Dee_tsc_makeenumeration_with_range_t)(DeeObject *self, DeeObject *start, DeeObject *end);

union Dee_tsc_uslot {
	DREF DeeObject   *d_function;  /* [1..1][valid_if(tsc_erase == ...)] Thiscall function. */
	Dee_objmethod_t   d_method;    /* [1..1][valid_if(tsc_erase == ...)] Method callback. */
	Dee_kwobjmethod_t d_kwmethod;  /* [1..1][valid_if(tsc_erase == ...)] Method callback. */
};

struct Dee_type_seq_cache {
	/************************************************************************/
	/* For `deemon.Sequence'                                                */
	/************************************************************************/
	Dee_tsc_foreach_reverse_t         tsc_foreach_reverse;
	Dee_tsc_enumerate_index_t         tsc_enumerate_index; /* Same as normal enumerate-index, but treated like `(self as Sequence).<enumerate_index>' */
	Dee_tsc_enumerate_index_reverse_t tsc_enumerate_index_reverse;
	Dee_tsc_nonempty_t                tsc_nonempty;

	/* Operators for the purpose of constructing `DefaultEnumeration_With*' objects. */
	Dee_tsc_makeenumeration_t                tsc_makeenumeration;
	Dee_tsc_makeenumeration_with_int_range_t tsc_makeenumeration_with_int_range;
	Dee_tsc_makeenumeration_with_range_t     tsc_makeenumeration_with_range;

	/* Returns the first element of the sequence.
	 * Calls `err_empty_sequence()' when it is empty. */
	Dee_tsc_getfirst_t   tsc_getfirst;
	Dee_tsc_boundfirst_t tsc_boundfirst;
	Dee_tsc_delfirst_t   tsc_delfirst;
	Dee_tsc_setfirst_t   tsc_setfirst;

	/* Returns the last element of the sequence.
	 * Calls `err_empty_sequence()' when it is empty. */
	Dee_tsc_getlast_t   tsc_getlast;
	Dee_tsc_boundlast_t tsc_boundlast;
	Dee_tsc_dellast_t   tsc_dellast;
	Dee_tsc_setlast_t   tsc_setlast;

	/* Functions that need additional variants for sequence sub-types that don't have indices (sets, maps) */
	Dee_tsc_any_t                           tsc_any;
	Dee_tsc_any_with_key_t                  tsc_any_with_key;
	Dee_tsc_any_with_range_t                tsc_any_with_range;
	Dee_tsc_any_with_range_and_key_t        tsc_any_with_range_and_key;
	union Dee_tsc_uslot                     tsc_any_data;
	Dee_tsc_all_t                           tsc_all;
	Dee_tsc_all_with_key_t                  tsc_all_with_key;
	Dee_tsc_all_with_range_t                tsc_all_with_range;
	Dee_tsc_all_with_range_and_key_t        tsc_all_with_range_and_key;
	union Dee_tsc_uslot                     tsc_all_data;
	Dee_tsc_parity_t                        tsc_parity;
	Dee_tsc_parity_with_key_t               tsc_parity_with_key;
	Dee_tsc_parity_with_range_t             tsc_parity_with_range;
	Dee_tsc_parity_with_range_and_key_t     tsc_parity_with_range_and_key;
	union Dee_tsc_uslot                     tsc_parity_data;
	Dee_tsc_reduce_t                        tsc_reduce;
	Dee_tsc_reduce_with_init_t              tsc_reduce_with_init;
	Dee_tsc_reduce_with_range_t             tsc_reduce_with_range;
	Dee_tsc_reduce_with_range_and_init_t    tsc_reduce_with_range_and_init;
	union Dee_tsc_uslot                     tsc_reduce_data;
	Dee_tsc_min_t                           tsc_min;
	Dee_tsc_min_with_key_t                  tsc_min_with_key;
	Dee_tsc_min_with_range_t                tsc_min_with_range;
	Dee_tsc_min_with_range_and_key_t        tsc_min_with_range_and_key;
	union Dee_tsc_uslot                     tsc_min_data;
	Dee_tsc_max_t                           tsc_max;
	Dee_tsc_max_with_key_t                  tsc_max_with_key;
	Dee_tsc_max_with_range_t                tsc_max_with_range;
	Dee_tsc_max_with_range_and_key_t        tsc_max_with_range_and_key;
	union Dee_tsc_uslot                     tsc_max_data;
	Dee_tsc_sum_t                           tsc_sum;
	Dee_tsc_sum_with_range_t                tsc_sum_with_range;
	union Dee_tsc_uslot                     tsc_sum_data;
	Dee_tsc_count_t                         tsc_count;
	Dee_tsc_count_with_key_t                tsc_count_with_key;
	Dee_tsc_count_with_range_t              tsc_count_with_range;
	Dee_tsc_count_with_range_and_key_t      tsc_count_with_range_and_key;
	union Dee_tsc_uslot                     tsc_count_data;
	Dee_tsc_contains_t                      tsc_contains;
	Dee_tsc_contains_with_key_t             tsc_contains_with_key;
	Dee_tsc_contains_with_range_t           tsc_contains_with_range;
	Dee_tsc_contains_with_range_and_key_t   tsc_contains_with_range_and_key;
	union Dee_tsc_uslot                     tsc_contains_data;
	Dee_tsc_locate_t                        tsc_locate;
	Dee_tsc_locate_with_key_t               tsc_locate_with_key;
	Dee_tsc_locate_with_range_t             tsc_locate_with_range;
	Dee_tsc_locate_with_range_and_key_t     tsc_locate_with_range_and_key;
	union Dee_tsc_uslot                     tsc_locate_data;
/*	Dee_tsc_rlocate_t                       tsc_rlocate;*/
/*	Dee_tsc_rlocate_with_key_t              tsc_rlocate_with_key;*/
	Dee_tsc_rlocate_with_range_t            tsc_rlocate_with_range;
	Dee_tsc_rlocate_with_range_and_key_t    tsc_rlocate_with_range_and_key;
	union Dee_tsc_uslot                     tsc_rlocate_data;
	Dee_tsc_startswith_t                    tsc_startswith;
	Dee_tsc_startswith_with_key_t           tsc_startswith_with_key;
	Dee_tsc_startswith_with_range_t         tsc_startswith_with_range;
	Dee_tsc_startswith_with_range_and_key_t tsc_startswith_with_range_and_key;
	union Dee_tsc_uslot                     tsc_startswith_data;
	Dee_tsc_endswith_t                      tsc_endswith;
	Dee_tsc_endswith_with_key_t             tsc_endswith_with_key;
	Dee_tsc_endswith_with_range_t           tsc_endswith_with_range;
	Dee_tsc_endswith_with_range_and_key_t   tsc_endswith_with_range_and_key;
	union Dee_tsc_uslot                     tsc_endswith_data;

	/* Sequence functions. */
	Dee_tsc_find_t               tsc_find;
	Dee_tsc_find_with_key_t      tsc_find_with_key;
	union Dee_tsc_uslot          tsc_find_data;
	Dee_tsc_rfind_t              tsc_rfind;
	Dee_tsc_rfind_with_key_t     tsc_rfind_with_key;
	union Dee_tsc_uslot          tsc_rfind_data;
	Dee_tsc_erase_t              tsc_erase;
	union Dee_tsc_uslot          tsc_erase_data;
	Dee_tsc_insert_t             tsc_insert;
	union Dee_tsc_uslot          tsc_insert_data;
	Dee_tsc_insertall_t          tsc_insertall;
	union Dee_tsc_uslot          tsc_insertall_data;
	Dee_tsc_pushfront_t          tsc_pushfront;
	union Dee_tsc_uslot          tsc_pushfront_data;
	Dee_tsc_append_t             tsc_append;
	union Dee_tsc_uslot          tsc_append_data;
	Dee_tsc_extend_t             tsc_extend;
	union Dee_tsc_uslot          tsc_extend_data;
	Dee_tsc_xchitem_index_t      tsc_xchitem_index;
	union Dee_tsc_uslot          tsc_xchitem_data;
	Dee_tsc_clear_t              tsc_clear;
	union Dee_tsc_uslot          tsc_clear_data;
	Dee_tsc_pop_t                tsc_pop;
	union Dee_tsc_uslot          tsc_pop_data;
	Dee_tsc_remove_t             tsc_remove;
	Dee_tsc_remove_with_key_t    tsc_remove_with_key;
	union Dee_tsc_uslot          tsc_remove_data;
	Dee_tsc_rremove_t            tsc_rremove;
	Dee_tsc_rremove_with_key_t   tsc_rremove_with_key;
	union Dee_tsc_uslot          tsc_rremove_data;
	Dee_tsc_removeall_t          tsc_removeall;
	Dee_tsc_removeall_with_key_t tsc_removeall_with_key;
	union Dee_tsc_uslot          tsc_removeall_data;
	Dee_tsc_removeif_t           tsc_removeif;
	union Dee_tsc_uslot          tsc_removeif_data;
	Dee_tsc_resize_t             tsc_resize;
	union Dee_tsc_uslot          tsc_resize_data;
	Dee_tsc_fill_t               tsc_fill;
	union Dee_tsc_uslot          tsc_fill_data;
	Dee_tsc_reverse_t            tsc_reverse;
	union Dee_tsc_uslot          tsc_reverse_data;
	Dee_tsc_reversed_t           tsc_reversed;
	union Dee_tsc_uslot          tsc_reversed_data;
	Dee_tsc_sort_t               tsc_sort;
	Dee_tsc_sort_with_key_t      tsc_sort_with_key;
	union Dee_tsc_uslot          tsc_sort_data;
	Dee_tsc_sorted_t             tsc_sorted;
	Dee_tsc_sorted_with_key_t    tsc_sorted_with_key;
	union Dee_tsc_uslot          tsc_sorted_data;
	Dee_tsc_bfind_t              tsc_bfind;
	Dee_tsc_bfind_with_key_t     tsc_bfind_with_key;
	union Dee_tsc_uslot          tsc_bfind_data;
	Dee_tsc_bposition_t          tsc_bposition;
	Dee_tsc_bposition_with_key_t tsc_bposition_with_key;
	union Dee_tsc_uslot          tsc_bposition_data;
	Dee_tsc_brange_t             tsc_brange;
	Dee_tsc_brange_with_key_t    tsc_brange_with_key;
	union Dee_tsc_uslot          tsc_brange_data;
	Dee_tsc_blocate_t            tsc_blocate;
	Dee_tsc_blocate_with_key_t   tsc_blocate_with_key;
	union Dee_tsc_uslot          tsc_blocate_data;

	/************************************************************************/
	/* For `deemon.Set' (only allocated if derived from "Set")              */
	/************************************************************************/
	/* TODO */


	/************************************************************************/
	/* For `deemon.Mapping' (only allocated if derived from "Mapping")      */
	/************************************************************************/
	/* TODO */
};

/* Destroy a lazily allocated sequence operator cache table. */
INTDEF NONNULL((1)) void DCALL
Dee_type_seq_cache_destroy(struct Dee_type_seq_cache *__restrict self);

INTDEF WUNUSED NONNULL((1)) struct Dee_type_seq_cache *DCALL
DeeType_TryRequireSeqCache(DeeTypeObject *__restrict self);

/* Type sequence operator definition functions. */
INTDEF WUNUSED NONNULL((1)) Dee_tsc_foreach_reverse_t DCALL DeeType_SeqCache_TryRequireForeachReverse(DeeTypeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_tsc_enumerate_index_reverse_t DCALL DeeType_SeqCache_TryRequireEnumerateIndexReverse(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL DeeType_SeqCache_HasPrivateEnumerateIndexReverse(DeeTypeObject *orig_type, DeeTypeObject *self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_enumerate_index_t DCALL DeeType_SeqCache_RequireEnumerateIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_nonempty_t DCALL DeeType_SeqCache_RequireNonEmpty(DeeTypeObject *__restrict self);

/* Operators for the purpose of constructing `DefaultEnumeration_With*' objects. */
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_makeenumeration_t DCALL DeeType_SeqCache_RequireMakeEnumeration(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_makeenumeration_with_int_range_t DCALL DeeType_SeqCache_RequireMakeEnumerationWithIntRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_makeenumeration_with_range_t DCALL DeeType_SeqCache_RequireMakeEnumerationWithRange(DeeTypeObject *__restrict self);

INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_getfirst_t DCALL DeeType_SeqCache_RequireGetFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_boundfirst_t DCALL DeeType_SeqCache_RequireBoundFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_delfirst_t DCALL DeeType_SeqCache_RequireDelFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_setfirst_t DCALL DeeType_SeqCache_RequireSetFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_getlast_t DCALL DeeType_SeqCache_RequireGetLast(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_boundlast_t DCALL DeeType_SeqCache_RequireBoundLast(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_dellast_t DCALL DeeType_SeqCache_RequireDelLast(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_setlast_t DCALL DeeType_SeqCache_RequireSetLast(DeeTypeObject *__restrict self);

/* Functions that need additional variants for sequence sub-types that don't have indices (sets, maps) */
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_any_t DCALL DeeType_SeqCache_RequireAny(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_any_with_key_t DCALL DeeType_SeqCache_RequireAnyWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_any_with_range_t DCALL DeeType_SeqCache_RequireAnyWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_any_with_range_and_key_t DCALL DeeType_SeqCache_RequireAnyWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_all_t DCALL DeeType_SeqCache_RequireAll(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_all_with_key_t DCALL DeeType_SeqCache_RequireAllWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_all_with_range_t DCALL DeeType_SeqCache_RequireAllWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_all_with_range_and_key_t DCALL DeeType_SeqCache_RequireAllWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_parity_t DCALL DeeType_SeqCache_RequireParity(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_parity_with_key_t DCALL DeeType_SeqCache_RequireParityWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_parity_with_range_t DCALL DeeType_SeqCache_RequireParityWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_parity_with_range_and_key_t DCALL DeeType_SeqCache_RequireParityWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_reduce_t DCALL DeeType_SeqCache_RequireReduce(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_reduce_with_init_t DCALL DeeType_SeqCache_RequireReduceWithInit(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_reduce_with_range_t DCALL DeeType_SeqCache_RequireReduceWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_reduce_with_range_and_init_t DCALL DeeType_SeqCache_RequireReduceWithRangeAndInit(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_min_t DCALL DeeType_SeqCache_RequireMin(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_min_with_key_t DCALL DeeType_SeqCache_RequireMinWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_min_with_range_t DCALL DeeType_SeqCache_RequireMinWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_min_with_range_and_key_t DCALL DeeType_SeqCache_RequireMinWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_max_t DCALL DeeType_SeqCache_RequireMax(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_max_with_key_t DCALL DeeType_SeqCache_RequireMaxWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_max_with_range_t DCALL DeeType_SeqCache_RequireMaxWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_max_with_range_and_key_t DCALL DeeType_SeqCache_RequireMaxWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_sum_t DCALL DeeType_SeqCache_RequireSum(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_sum_with_range_t DCALL DeeType_SeqCache_RequireSumWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_count_t DCALL DeeType_SeqCache_RequireCount(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_count_with_key_t DCALL DeeType_SeqCache_RequireCountWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_count_with_range_t DCALL DeeType_SeqCache_RequireCountWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_count_with_range_and_key_t DCALL DeeType_SeqCache_RequireCountWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_contains_t DCALL DeeType_SeqCache_RequireContains(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_contains_with_key_t DCALL DeeType_SeqCache_RequireContainsWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_contains_with_range_t DCALL DeeType_SeqCache_RequireContainsWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_contains_with_range_and_key_t DCALL DeeType_SeqCache_RequireContainsWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_locate_t DCALL DeeType_SeqCache_RequireLocate(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_locate_with_key_t DCALL DeeType_SeqCache_RequireLocateWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_locate_with_range_t DCALL DeeType_SeqCache_RequireLocateWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_locate_with_range_and_key_t DCALL DeeType_SeqCache_RequireLocateWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_rlocate_with_range_t DCALL DeeType_SeqCache_RequireRLocateWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_rlocate_with_range_and_key_t DCALL DeeType_SeqCache_RequireRLocateWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_startswith_t DCALL DeeType_SeqCache_RequireStartsWith(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_startswith_with_key_t DCALL DeeType_SeqCache_RequireStartsWithWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_startswith_with_range_t DCALL DeeType_SeqCache_RequireStartsWithWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_startswith_with_range_and_key_t DCALL DeeType_SeqCache_RequireStartsWithWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_endswith_t DCALL DeeType_SeqCache_RequireEndsWith(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_endswith_with_key_t DCALL DeeType_SeqCache_RequireEndsWithWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_endswith_with_range_t DCALL DeeType_SeqCache_RequireEndsWithWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_endswith_with_range_and_key_t DCALL DeeType_SeqCache_RequireEndsWithWithRangeAndKey(DeeTypeObject *__restrict self);

/* Mutable sequence functions */
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_find_t DCALL DeeType_SeqCache_RequireFind(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_find_with_key_t DCALL DeeType_SeqCache_RequireFindWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_rfind_t DCALL DeeType_SeqCache_RequireRFind(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_rfind_with_key_t DCALL DeeType_SeqCache_RequireRFindWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_erase_t DCALL DeeType_SeqCache_RequireErase(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_insert_t DCALL DeeType_SeqCache_RequireInsert(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_insertall_t DCALL DeeType_SeqCache_RequireInsertAll(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_pushfront_t DCALL DeeType_SeqCache_RequirePushFront(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_append_t DCALL DeeType_SeqCache_RequireAppend(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_extend_t DCALL DeeType_SeqCache_RequireExtend(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_xchitem_index_t DCALL DeeType_SeqCache_RequireXchItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_clear_t DCALL DeeType_SeqCache_RequireClear(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_pop_t DCALL DeeType_SeqCache_RequirePop(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_remove_t DCALL DeeType_SeqCache_RequireRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_remove_with_key_t DCALL DeeType_SeqCache_RequireRemoveWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_rremove_t DCALL DeeType_SeqCache_RequireRRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_rremove_with_key_t DCALL DeeType_SeqCache_RequireRRemoveWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_removeall_t DCALL DeeType_SeqCache_RequireRemoveAll(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_removeall_with_key_t DCALL DeeType_SeqCache_RequireRemoveAllWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_removeif_t DCALL DeeType_SeqCache_RequireRemoveIf(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_resize_t DCALL DeeType_SeqCache_RequireResize(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_fill_t DCALL DeeType_SeqCache_RequireFill(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_reverse_t DCALL DeeType_SeqCache_RequireReverse(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_reversed_t DCALL DeeType_SeqCache_RequireReversed(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_sort_t DCALL DeeType_SeqCache_RequireSort(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_sort_with_key_t DCALL DeeType_SeqCache_RequireSortWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_sorted_t DCALL DeeType_SeqCache_RequireSorted(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_sorted_with_key_t DCALL DeeType_SeqCache_RequireSortedWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_bfind_t DCALL DeeType_SeqCache_RequireBFind(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_bfind_with_key_t DCALL DeeType_SeqCache_RequireBFindWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_bposition_t DCALL DeeType_SeqCache_RequireBPosition(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_bposition_with_key_t DCALL DeeType_SeqCache_RequireBPositionWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_brange_t DCALL DeeType_SeqCache_RequireBRange(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_brange_with_key_t DCALL DeeType_SeqCache_RequireBRangeWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_blocate_t DCALL DeeType_SeqCache_RequireBLocate(DeeTypeObject *__restrict self);
INTDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_blocate_with_key_t DCALL DeeType_SeqCache_RequireBLocateWithKey(DeeTypeObject *__restrict self);

/* Same as `DeeObject_EnumerateIndex()', but also works for treats `self' as `self as Sequence' */
#define DeeSeq_EnumerateIndex(self, proc, arg, start, end) \
	(*DeeType_SeqCache_RequireEnumerateIndex(Dee_TYPE(self)))(self, proc, arg, start, end)


/* Helpers for constructing enumeration proxy objects. */
#define DeeSeq_MakeEnumeration(self) \
	(*DeeType_SeqCache_RequireMakeEnumeration(Dee_TYPE(self)))(self)
#define DeeSeq_MakeEnumerationWithIntRange(self, start, end) \
	(*DeeType_SeqCache_RequireMakeEnumerationWithIntRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_MakeEnumerationWithRange(self, start, end) \
	(*DeeType_SeqCache_RequireMakeEnumerationWithRange(Dee_TYPE(self)))(self, start, end)

/* Helpers for enumerating a sequence by invoking a given callback. */
INTDEF NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Enumerate(DeeObject *self, DeeObject *cb);
INTDEF NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_EnumerateWithIntRange(DeeObject *self, DeeObject *cb, size_t start, size_t end);
INTDEF NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL DeeSeq_EnumerateWithRange(DeeObject *self, DeeObject *cb, DeeObject *start, DeeObject *end);



/* Helpers to quickly invoke default sequence functions. */
#define DeeSeq_GetFirst(self)    (*DeeType_SeqCache_RequireGetFirst(Dee_TYPE(self)))(self)
#define DeeSeq_BoundFirst(self)  (*DeeType_SeqCache_RequireBoundFirst(Dee_TYPE(self)))(self)
#define DeeSeq_DelFirst(self)    (*DeeType_SeqCache_RequireDelFirst(Dee_TYPE(self)))(self)
#define DeeSeq_SetFirst(self, v) (*DeeType_SeqCache_RequireSetFirst(Dee_TYPE(self)))(self, v)
#define DeeSeq_GetLast(self)     (*DeeType_SeqCache_RequireGetLast(Dee_TYPE(self)))(self)
#define DeeSeq_BoundLast(self)   (*DeeType_SeqCache_RequireBoundLast(Dee_TYPE(self)))(self)
#define DeeSeq_DelLast(self)     (*DeeType_SeqCache_RequireDelLast(Dee_TYPE(self)))(self)
#define DeeSeq_SetLast(self, v)  (*DeeType_SeqCache_RequireSetLast(Dee_TYPE(self)))(self, v)

#define DeeSeq_NonEmpty(self) (*DeeType_SeqCache_RequireNonEmpty(Dee_TYPE(self)))(self)

/* Invoke sequence functions */
/* TODO: Remove the "new_" prefix and remove functions from "seq_mutable.c" */
#define new_DeeSeq_Any(self)                                                (*DeeType_SeqCache_RequireAny(Dee_TYPE(self)))(self)
#define new_DeeSeq_AnyWithKey(self, key)                                    (*DeeType_SeqCache_RequireAnyWithKey(Dee_TYPE(self)))(self, key)
#define new_DeeSeq_AnyWithRange(self, start, end)                           (*DeeType_SeqCache_RequireAnyWithRange(Dee_TYPE(self)))(self, start, end)
#define new_DeeSeq_AnyWithRangeAndKey(self, start, end, key)                (*DeeType_SeqCache_RequireAnyWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define new_DeeSeq_All(self)                                                (*DeeType_SeqCache_RequireAll(Dee_TYPE(self)))(self)
#define new_DeeSeq_AllWithKey(self, key)                                    (*DeeType_SeqCache_RequireAllWithKey(Dee_TYPE(self)))(self, key)
#define new_DeeSeq_AllWithRange(self, start, end)                           (*DeeType_SeqCache_RequireAllWithRange(Dee_TYPE(self)))(self, start, end)
#define new_DeeSeq_AllWithRangeAndKey(self, start, end, key)                (*DeeType_SeqCache_RequireAllWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define new_DeeSeq_Parity(self)                                             (*DeeType_SeqCache_RequireParity(Dee_TYPE(self)))(self)
#define new_DeeSeq_ParityWithKey(self, key)                                 (*DeeType_SeqCache_RequireParityWithKey(Dee_TYPE(self)))(self, key)
#define new_DeeSeq_ParityWithRange(self, start, end)                        (*DeeType_SeqCache_RequireParityWithRange(Dee_TYPE(self)))(self, start, end)
#define new_DeeSeq_ParityWithRangeAndKey(self, start, end, key)             (*DeeType_SeqCache_RequireParityWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define new_DeeSeq_Reduce(self, combine)                                    (*DeeType_SeqCache_RequireReduce(Dee_TYPE(self)))(self, combine)
#define new_DeeSeq_ReduceWithInit(self, combine, init)                      (*DeeType_SeqCache_RequireReduceWithInit(Dee_TYPE(self)))(self, combine, init)
#define new_DeeSeq_ReduceWithRange(self, combine, start, end)               (*DeeType_SeqCache_RequireReduceWithRange(Dee_TYPE(self)))(self, combine, start, end)
#define new_DeeSeq_ReduceWithRangeAndInit(self, combine, start, end, init)  (*DeeType_SeqCache_RequireReduceWithRangeAndInit(Dee_TYPE(self)))(self, combine, start, end, init)
#define new_DeeSeq_Min(self)                                                (*DeeType_SeqCache_RequireMin(Dee_TYPE(self)))(self)
#define new_DeeSeq_MinWithKey(self, key)                                    (*DeeType_SeqCache_RequireMinWithKey(Dee_TYPE(self)))(self, key)
#define new_DeeSeq_MinWithRange(self, start, end)                           (*DeeType_SeqCache_RequireMinWithRange(Dee_TYPE(self)))(self, start, end)
#define new_DeeSeq_MinWithRangeAndKey(self, start, end, key)                (*DeeType_SeqCache_RequireMinWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define new_DeeSeq_Max(self)                                                (*DeeType_SeqCache_RequireMax(Dee_TYPE(self)))(self)
#define new_DeeSeq_MaxWithKey(self, key)                                    (*DeeType_SeqCache_RequireMaxWithKey(Dee_TYPE(self)))(self, key)
#define new_DeeSeq_MaxWithRange(self, start, end)                           (*DeeType_SeqCache_RequireMaxWithRange(Dee_TYPE(self)))(self, start, end)
#define new_DeeSeq_MaxWithRangeAndKey(self, start, end, key)                (*DeeType_SeqCache_RequireMaxWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define new_DeeSeq_Sum(self)                                                (*DeeType_SeqCache_RequireSum(Dee_TYPE(self)))(self)
#define new_DeeSeq_SumWithRange(self, start, end)                           (*DeeType_SeqCache_RequireSumWithRange(Dee_TYPE(self)))(self, start, end)
#define new_DeeSeq_Count(self, item)                                        (*DeeType_SeqCache_RequireCount(Dee_TYPE(self)))(self, item)
#define new_DeeSeq_CountWithKey(self, item, key)                            (*DeeType_SeqCache_RequireCountWithKey(Dee_TYPE(self)))(self, item, key)
#define new_DeeSeq_CountWithRange(self, item, start, end)                   (*DeeType_SeqCache_RequireCountWithRange(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_CountWithRangeAndKey(self, item, start, end, key)        (*DeeType_SeqCache_RequireCountWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define new_DeeSeq_Contains(self, item)                                     (*DeeType_SeqCache_RequireContains(Dee_TYPE(self)))(self, item)
#define new_DeeSeq_ContainsWithKey(self, item, key)                         (*DeeType_SeqCache_RequireContainsWithKey(Dee_TYPE(self)))(self, item, key)
#define new_DeeSeq_ContainsWithRange(self, item, start, end)                (*DeeType_SeqCache_RequireContainsWithRange(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_ContainsWithRangeAndKey(self, item, start, end, key)     (*DeeType_SeqCache_RequireContainsWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define new_DeeSeq_Locate(self, item)                                       (*DeeType_SeqCache_RequireLocate(Dee_TYPE(self)))(self, item)
#define new_DeeSeq_LocateWithKey(self, item, key)                           (*DeeType_SeqCache_RequireLocateWithKey(Dee_TYPE(self)))(self, item, key)
#define new_DeeSeq_LocateWithRange(self, item, start, end)                  (*DeeType_SeqCache_RequireLocateWithRange(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_LocateWithRangeAndKey(self, item, start, end, key)       (*DeeType_SeqCache_RequireLocateWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define new_DeeSeq_RLocateWithRange(self, item, start, end)                 (*DeeType_SeqCache_RequireRLocateWithRange(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_RLocateWithRangeAndKey(self, item, start, end, key)      (*DeeType_SeqCache_RequireRLocateWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define new_DeeSeq_StartsWith(self, item)                                   (*DeeType_SeqCache_RequireStartsWith(Dee_TYPE(self)))(self, item)
#define new_DeeSeq_StartsWithWithKey(self, item, key)                       (*DeeType_SeqCache_RequireStartsWithWithKey(Dee_TYPE(self)))(self, item, key)
#define new_DeeSeq_StartsWithWithRange(self, item, start, end)              (*DeeType_SeqCache_RequireStartsWithWithRange(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_StartsWithWithRangeAndKey(self, item, start, end, key)   (*DeeType_SeqCache_RequireStartsWithWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define new_DeeSeq_EndsWith(self, item)                                     (*DeeType_SeqCache_RequireEndsWith(Dee_TYPE(self)))(self, item)
#define new_DeeSeq_EndsWithWithKey(self, item, key)                         (*DeeType_SeqCache_RequireEndsWithWithKey(Dee_TYPE(self)))(self, item, key)
#define new_DeeSeq_EndsWithWithRange(self, item, start, end)                (*DeeType_SeqCache_RequireEndsWithWithRange(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_EndsWithWithRangeAndKey(self, item, start, end, key)     (*DeeType_SeqCache_RequireEndsWithWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define new_DeeSeq_Find(self, item, start, end)                             (*DeeType_SeqCache_RequireFind(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_FindWithKey(self, item, start, end, key)                 (*DeeType_SeqCache_RequireFindWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define new_DeeSeq_RFind(self, item, start, end)                            (*DeeType_SeqCache_RequireRFind(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_RFindWithKey(self, item, start, end, key)                (*DeeType_SeqCache_RequireRFindWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define new_DeeSeq_Erase(self, index, count)                                (*DeeType_SeqCache_RequireErase(Dee_TYPE(self)))(self, index, count)
#define new_DeeSeq_Insert(self, index, item)                                (*DeeType_SeqCache_RequireInsert(Dee_TYPE(self)))(self, index, item)
#define new_DeeSeq_InsertAll(self, index, items)                            (*DeeType_SeqCache_RequireInsertAll(Dee_TYPE(self)))(self, index, items)
#define new_DeeSeq_PushFront(self, item)                                    (*DeeType_SeqCache_RequirePushFront(Dee_TYPE(self)))(self, item)
#define new_DeeSeq_Append(self, item)                                       (*DeeType_SeqCache_RequireAppend(Dee_TYPE(self)))(self, item)
#define new_DeeSeq_Extend(self, items)                                      (*DeeType_SeqCache_RequireExtend(Dee_TYPE(self)))(self, items)
#define new_DeeSeq_XchItemIndex(self, index, value)                         (*DeeType_SeqCache_RequireXchItemIndex(Dee_TYPE(self)))(self, index, value)
#define new_DeeSeq_Clear(self)                                              (*DeeType_SeqCache_RequireClear(Dee_TYPE(self)))(self)
#define new_DeeSeq_Pop(self, index)                                         (*DeeType_SeqCache_RequirePop(Dee_TYPE(self)))(self, index)
#define new_DeeSeq_Remove(self, item, start, end)                           (*DeeType_SeqCache_RequireRemove(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_RemoveWithKey(self, item, start, end, key)               (*DeeType_SeqCache_RequireRemoveWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define new_DeeSeq_RRemove(self, item, start, end)                          (*DeeType_SeqCache_RequireRRemove(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_RRemoveWithKey(self, item, start, end, key)              (*DeeType_SeqCache_RequireRRemoveWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define new_DeeSeq_RemoveAll(self, item, start, end, max)                   (*DeeType_SeqCache_RequireRemoveAll(Dee_TYPE(self)))(self, item, start, end, max)
#define new_DeeSeq_RemoveAllWithKey(self, item, start, end, max, key)       (*DeeType_SeqCache_RequireRemoveAllWithKey(Dee_TYPE(self)))(self, item, start, end, max, key)
#define new_DeeSeq_RemoveIf(self, should, start, end, max)                  (*DeeType_SeqCache_RequireRemoveIf(Dee_TYPE(self)))(self, should, start, end, max)
#define new_DeeSeq_Resize(self, newsize, filler)                            (*DeeType_SeqCache_RequireResize(Dee_TYPE(self)))(self, newsize, filler)
#define new_DeeSeq_Fill(self, start, end, filler)                           (*DeeType_SeqCache_RequireFill(Dee_TYPE(self)))(self, start, end, filler)
#define new_DeeSeq_Reverse(self, start, end)                                (*DeeType_SeqCache_RequireReverse(Dee_TYPE(self)))(self, start, end)
#define new_DeeSeq_Reversed(self, start, end)                               (*DeeType_SeqCache_RequireReversed(Dee_TYPE(self)))(self, start, end)
#define new_DeeSeq_Sort(self, start, end)                                   (*DeeType_SeqCache_RequireSort(Dee_TYPE(self)))(self, start, end)
#define new_DeeSeq_SortWithKey(self, start, end, key)                       (*DeeType_SeqCache_RequireSortWithKey(Dee_TYPE(self)))(self, start, end, key)
#define new_DeeSeq_Sorted(self, start, end)                                 (*DeeType_SeqCache_RequireSorted(Dee_TYPE(self)))(self, start, end)
#define new_DeeSeq_SortedWithKey(self, start, end, key)                     (*DeeType_SeqCache_RequireSortedWithKey(Dee_TYPE(self)))(self, start, end, key)
#define new_DeeSeq_BFind(self, item, start, end)                            (*DeeType_SeqCache_RequireBFind(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_BFindWithKey(self, item, start, end, key)                (*DeeType_SeqCache_RequireBFindWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define new_DeeSeq_BPosition(self, item, start, end)                        (*DeeType_SeqCache_RequireBPosition(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_BPositionWithKey(self, item, start, end, key)            (*DeeType_SeqCache_RequireBPositionWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define new_DeeSeq_BRange(self, item, start, end, result_range)             (*DeeType_SeqCache_RequireBRange(Dee_TYPE(self)))(self, item, start, end, result_range)
#define new_DeeSeq_BRangeWithKey(self, item, start, end, key, result_range) (*DeeType_SeqCache_RequireBRangeWithKey(Dee_TYPE(self)))(self, item, start, end, key, result_range)
#define new_DeeSeq_BLocate(self, item, start, end)                          (*DeeType_SeqCache_RequireBLocate(Dee_TYPE(self)))(self, item, start, end)
#define new_DeeSeq_BLocateWithKey(self, item, start, end, key)              (*DeeType_SeqCache_RequireBLocateWithKey(Dee_TYPE(self)))(self, item, start, end, key)


/* Possible implementations for sequence cache functions. */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndex(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachReverseWithSizeAndTryGetItemIndex(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachReverseWithSizeObAndGetItem(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndex(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexReverseWithSizeAndTryGetItemIndex(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexReverseWithSizeObAndGetItem(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithGetAttr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithGetItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithForeachDefault(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithBoundAttr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithBoundItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithBoundItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithForeachDefault(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithDelAttr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithDelItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithDelItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithError(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithSetAttr(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithSetItem(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithSetItemIndex(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithError(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithGetAttr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithSizeObAndGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithSizeAndGetItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithSizeAndGetItemIndexFast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithForeachDefault(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithBoundAttr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithSizeObAndBoundItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithSizeAndBoundItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithSizeAndGetItemIndexFast(DeeObject *__restrict self);
#define DeeSeq_DefaultBoundLastWithForeachDefault DeeSeq_DefaultBoundFirstWithForeachDefault
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithDelAttr(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithDelItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithDelItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithError(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithSetAttr(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithSetItem(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithSetItemIndex(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithError(DeeObject *self, DeeObject *value);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultNonEmptyWithError(DeeObject *__restrict self);


/* Functions that need additional variants for sequence sub-types that don't have indices (sets, maps) */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithCallAttrAny(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithCallAnyDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithCallAnyDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithCallAnyDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAttrAnyForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAttrAnyForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithCallAttrAny(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithCallAnyDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithCallAnyDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithCallAnyDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithCallAttrAny(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithCallAnyDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithCallAnyDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithCallAnyDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithCallAttrAll(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithCallAllDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithCallAllDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithCallAllDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAttrAllForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAttrAllForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithCallAttrAll(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithCallAllDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithCallAllDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithCallAllDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithCallAttrAll(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithCallAllDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithCallAllDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithCallAllDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithCallAttrParity(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithCallParityDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithCallParityDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithCallParityDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallAttrParityForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallAttrParityForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithCallAttrParity(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithCallParityDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithCallParityDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithCallParityDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithCallAttrParity(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithCallParityDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithCallParityDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithCallParityDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithCallAttrReduce(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithCallReduceDataFunction(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithCallReduceDataMethod(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithCallReduceDataKwMethod(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithForeach(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallAttrReduceForSeq(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataFunctionForSeq(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataMethodForSeq(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataKwMethodForSeq(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallAttrReduceForSetOrMap(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataFunctionForSetOrMap(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataMethodForSetOrMap(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataKwMethodForSetOrMap(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithForeach(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithCallAttrReduce(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithCallReduceDataFunction(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithCallReduceDataMethod(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithCallReduceDataKwMethod(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithEnumerateIndex(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithCallAttrReduce(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithCallReduceDataFunction(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithCallReduceDataMethod(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithCallReduceDataKwMethod(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithEnumerateIndex(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithCallAttrMin(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithCallMinDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithCallMinDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithCallMinDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallAttrMinForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallAttrMinForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithCallAttrMin(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithCallMinDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithCallMinDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithCallMinDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithCallAttrMin(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithCallMinDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithCallMinDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithCallMinDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithCallAttrMax(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithCallMaxDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithCallMaxDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithCallMaxDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallAttrMaxForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallAttrMaxForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithCallAttrMax(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithCallMaxDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithCallMaxDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithCallMaxDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithCallAttrMax(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithCallMaxDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithCallMaxDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithCallMaxDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithCallAttrSum(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithCallSumDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithCallSumDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithCallSumDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithCallAttrSum(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithCallSumDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithCallSumDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithCallSumDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithEnumerateIndex(DeeObject *self, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithCallAttrCount(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithCallCountDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithCallCountDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithCallCountDataKwMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithForeach(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallAttrCountForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataFunctionForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataKwMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallAttrCountForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataFunctionForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataKwMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithForeach(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithCallAttrCount(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithCallCountDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithCallCountDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithCallCountDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithCallAttrCount(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithCallCountDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithCallCountDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithCallCountDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithCallAttrContains(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithCallContainsDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithCallContainsDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithCallContainsDataKwMethod(DeeObject *self, DeeObject *item);
/*INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithContains(DeeObject *self, DeeObject *item);*/
#define DeeSeq_DefaultContainsWithContains DeeObject_ContainsAsBool
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithForeach(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallAttrContainsForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallContainsDataFunctionForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallContainsDataMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallContainsDataKwMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallAttrContainsForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallContainsDataFunctionForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallContainsDataMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithCallContainsDataKwMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithForeach(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithCallAttrContains(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithCallContainsDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithCallContainsDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithCallContainsDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithCallAttrContains(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithCallContainsDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithCallContainsDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithCallContainsDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallAttrLocate(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallLocateDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallLocateDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallLocateDataKwMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithForeach(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallAttrLocateForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallLocateDataFunctionForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallLocateDataMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallLocateDataKwMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallAttrLocateForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallLocateDataFunctionForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallLocateDataMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallLocateDataKwMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithForeach(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithCallAttrLocate(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithCallLocateDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithCallLocateDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithCallLocateDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeAndKeyWithCallAttrLocate(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeAndKeyWithCallLocateDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeAndKeyWithCallLocateDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeAndKeyWithCallLocateDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeAndKeyWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithCallAttrRLocate(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithCallRLocateDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithCallRLocateDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithCallRLocateDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithTSCEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeAndKeyWithCallAttrRLocate(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeAndKeyWithCallRLocateDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeAndKeyWithCallRLocateDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeAndKeyWithCallRLocateDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeAndKeyWithTSCEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeAndKeyWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithCallAttrStartsWith(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithCallStartsWithDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithCallStartsWithDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithCallStartsWithDataKwMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithTSCFirst(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallAttrStartsWithForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataFunctionForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataKwMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallAttrStartsWithForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataFunctionForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataKwMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithTSCFirst(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithCallAttrStartsWith(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithCallStartsWithDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithCallStartsWithDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithCallStartsWithDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallAttrStartsWith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallStartsWithDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallStartsWithDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallStartsWithDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithCallAttrEndsWith(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithCallEndsWithDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithCallEndsWithDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithCallEndsWithDataKwMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithTSCLast(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallAttrEndsWithForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataFunctionForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataKwMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallAttrEndsWithForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataFunctionForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataKwMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithTSCLast(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithCallAttrEndsWith(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithCallEndsWithDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithCallEndsWithDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithCallEndsWithDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallAttrEndsWith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallEndsWithDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallEndsWithDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallEndsWithDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);



/* Mutable sequence functions */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithCallAttrFind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithCallFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithCallFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithCallFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithCallAttrFind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithCallFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithCallFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithCallFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithCallAttrRFind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithCallRFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithCallRFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithCallRFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithTSCEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithCallAttrRFind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithCallRFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithCallRFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithCallRFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithTSCEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithCallAttrErase(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithCallEraseDataFunction(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithCallEraseDataMethod(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithCallEraseDataKwMethod(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithDelRangeIndex(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithPop(DeeObject *self, size_t index, size_t count);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultEraseWithError(DeeObject *self, size_t index, size_t count);

INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithCallAttrInsert(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithCallInsertDataFunction(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithCallInsertDataMethod(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithCallInsertDataKwMethod(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithTSCInsertAll(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithError(DeeObject *self, size_t index, DeeObject *item);

INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithCallAttrInsertAll(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithCallInsertAllDataFunction(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithCallInsertAllDataMethod(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithCallInsertAllDataKwMethod(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithSetRangeIndex(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithTSCInsertForeach(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithError(DeeObject *self, size_t index, DeeObject *items);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithCallAttrPushFront(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithCallPushFrontDataFunction(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithCallPushFrontDataMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithCallPushFrontDataKwMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithTSCInsert(DeeObject *self, DeeObject *item);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAttrAppend(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAttrPushBack(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAppendDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAppendDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAppendDataKwMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithTSCExtend(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithSizeAndTSCInsert(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithError(DeeObject *self, DeeObject *item);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithCallAttrExtend(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithCallExtendDataFunction(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithCallExtendDataMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithCallExtendDataKwMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithTSCAppendForeach(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithSizeAndTSCInsertAll(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithError(DeeObject *self, DeeObject *items);

INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithCallAttrXchItem(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithCallXchItemDataFunction(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithCallXchItemDataMethod(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithCallXchItemDataKwMethod(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithGetItemIndexAndSetItemIndex(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL DeeSeq_DefaultXchItemIndexWithError(DeeObject *self, size_t index, DeeObject *value);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithCallAttrClear(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithCallClearDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithCallClearDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithCallClearDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithDelRangeIndexN(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithSetRangeIndexN(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithTSCErase(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithCallAttrPop(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithCallPopDataFunction(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithCallPopDataMethod(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithCallPopDataKwMethod(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithSizeAndGetItemIndexAndTSCErase(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithError(DeeObject *self, Dee_ssize_t index);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallAttrRemove(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithTSCRemoveAll(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithTSCRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithTSCFindAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallAttrRemove(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithTSCRemoveAllWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithTSCRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithTSCFindWithKeyAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallAttrRRemove(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallRRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallRRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallRRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithTSCRFindAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithTSCEnumerateIndexReverseAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallAttrRRemove(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithTSCRFindWithKeyAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithTSCEnumerateIndexReverseAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallAttrRemoveAll(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallRemoveAllDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallRemoveAllDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallRemoveAllDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithTSCRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithTSCRemove(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);

INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallAttrRemoveAll(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithTSCRemoveWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallAttrRemoveIf(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallRemoveIfDataFunction(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallRemoveIfDataMethod(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallRemoveIfDataKwMethod(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithTSCRemoveAllWithKey(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithError(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);

INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithCallAttrResize(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithCallResizeDataFunction(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithCallResizeDataMethod(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithCallResizeDataKwMethod(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithSizeAndSetRangeIndexAndDelRangeIndex(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithSizeAndSetRangeIndex(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithSizeAndTSCEraseAndTSCExtend(DeeObject *self, size_t newsize, DeeObject *filler);

INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithCallAttrFill(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithCallFillDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithCallFillDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithCallFillDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithSizeAndSetRangeIndex(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithEnumerateIndexAndSetItemIndex(DeeObject *self, size_t start, size_t end, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultFillWithError(DeeObject *self, size_t start, size_t end, DeeObject *filler);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithCallAttrReverse(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithCallReverseDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithCallReverseDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithCallReverseDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithTSCReversedAndSetRangeIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithSizeAndGetItemIndexAndSetItemIndexAndDelItemIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithSizeAndGetItemIndexAndSetItemIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithError(DeeObject *self, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithCallAttrReversed(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithCallReversedDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithCallReversedDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithCallReversedDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithProxySizeAndGetItemIndexFast(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithProxySizeAndGetItemIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithProxySizeAndTryGetItemIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultReversedWithCopyForeachDefault(DeeObject *self, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithCallAttrSort(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithCallSortDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithCallSortDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithCallSortDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithTSCSortedAndSetRangeIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithSizeAndGetItemIndexAndSetItemIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithError(DeeObject *self, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithCallAttrSort(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithCallSortDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithCallSortDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithCallSortDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithTSCSortedAndSetRangeIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithSizeAndGetItemIndexAndSetItemIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithError(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCallAttrSorted(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCallSortedDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCallSortedDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCallSortedDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCopySizeAndGetItemIndexFast(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCopySizeAndTryGetItemIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithCopyForeachDefault(DeeObject *self, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCallAttrSorted(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCallSortedDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCallSortedDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCallSortedDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCopySizeAndGetItemIndexFast(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCopySizeAndTryGetItemIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultSortedWithKeyWithCopyForeachDefault(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithCallAttrBFind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithCallBFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithCallBFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithCallBFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithTSCBRange(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithCallAttrBFind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithCallBFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithCallBFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithCallBFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithTSCBRangeWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithCallAttrBPosition(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithCallBPositionDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithCallBPositionDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithCallBPositionDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithTSCBRange(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithCallAttrBPosition(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithCallBPositionDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithCallBPositionDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithCallBPositionDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithTSCBRangeWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultBRangeWithCallAttrBRange(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultBRangeWithCallBRangeDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultBRangeWithCallBRangeDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultBRangeWithCallBRangeDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultBRangeWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultBRangeWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);

INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL DeeSeq_DefaultBRangeWithKeyWithCallAttrBRange(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL DeeSeq_DefaultBRangeWithKeyWithCallBRangeDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL DeeSeq_DefaultBRangeWithKeyWithCallBRangeDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL DeeSeq_DefaultBRangeWithKeyWithCallBRangeDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL DeeSeq_DefaultBRangeWithKeyWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
INTDEF WUNUSED NONNULL((1, 2, 5, 6)) int DCALL DeeSeq_DefaultBRangeWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithCallAttrBLocate(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithCallBLocateDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithCallBLocateDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithCallBLocateDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithTSCBFindAndGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithKeyWithCallAttrBLocate(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithKeyWithCallBLocateDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithKeyWithCallBLocateDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithKeyWithCallBLocateDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithKeyWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithKeyWithTSCBFindWithKeyAndGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);


/* Default sequence function hooks (used as function pointers of `type_method' / `type_getset' of Sequence/Set/Mapping) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_getfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default_seq_boundfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default_seq_delfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default_seq_setfirst(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_getlast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default_seq_boundlast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default_seq_dellast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default_seq_setlast(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) int DCALL default_seq_bool(DeeObject *__restrict self);


/* Default sequence function pointers (including ones for mutable sequences). */
/* Functions that need additional variants for sequence sub-types that don't have indices (sets, maps) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_enumerate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_any(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_all(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_parity(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_reduce(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_min(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_max(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_sum(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_count(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_contains(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_locate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_rlocate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_startswith(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_endswith(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_find(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_rfind(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_erase(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_insert(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_insertall(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_pushfront(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_append(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_extend(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_xchitem(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_clear(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_pop(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_popfront(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_popback(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_remove(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_rremove(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_removeall(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_removeif(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_resize(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_fill(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_reverse(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_reversed(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_sort(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_sorted(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_bfind(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_bposition(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_brange(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_blocate(DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_H */
