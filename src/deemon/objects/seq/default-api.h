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

#ifndef LOCAL_FOR_VARIANTS
#include <deemon/api.h>
#include <deemon/class.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#else /* !LOCAL_FOR_VARIANTS */
#define CONFIG_BUILDING_DEEMON
#define __SIZEOF_POINTER__ 4 /* Doesn't matter */
#define Dee_OPERATOR_USERCOUNT 0x003e /* Doesn't matter */
#define _DEE_WITHOUT_INCLUDES
#include "../../../../include/deemon/class.h"
#endif /* !LOCAL_FOR_VARIANTS */

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

typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_operator_bool_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_operator_iter_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_operator_sizeob_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_operator_contains_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_operator_getitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_operator_delitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *Dee_tsc_operator_setitem_t)(DeeObject *self, DeeObject *index, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *Dee_tsc_operator_getrange_t)(DeeObject *self, DeeObject *start, DeeObject *end);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *Dee_tsc_operator_delrange_t)(DeeObject *self, DeeObject *start, DeeObject *end);
typedef WUNUSED_T NONNULL_T((1, 2, 3, 4)) int (DCALL *Dee_tsc_operator_setrange_t)(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *Dee_tsc_operator_foreach_t)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *Dee_tsc_operator_enumerate_t)(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *Dee_tsc_operator_enumerate_index_t)(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_operator_bounditem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_operator_hasitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) size_t (DCALL *Dee_tsc_operator_size_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) size_t (DCALL *Dee_tsc_operator_size_fast_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_operator_getitem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_operator_delitem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *Dee_tsc_operator_setitem_index_t)(DeeObject *self, size_t index, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_operator_bounditem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_operator_hasitem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_operator_getrange_index_t)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_operator_delrange_index_t)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *Dee_tsc_operator_setrange_index_t)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_operator_getrange_index_n_t)(DeeObject *self, Dee_ssize_t start);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_operator_delrange_index_n_t)(DeeObject *self, Dee_ssize_t start);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *Dee_tsc_operator_setrange_index_n_t)(DeeObject *self, Dee_ssize_t start, DeeObject *values);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_operator_trygetitem_t)(DeeObject *self, DeeObject *index);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_operator_trygetitem_index_t)(DeeObject *self, size_t index);
typedef WUNUSED_T NONNULL_T((1)) Dee_hash_t (DCALL *Dee_tsc_operator_hash_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_operator_compare_eq_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_operator_compare_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_operator_trycompare_eq_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_operator_eq_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_operator_ne_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_operator_lo_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_operator_le_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_operator_gr_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_operator_ge_t)(DeeObject *self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_operator_inplace_add_t)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_operator_inplace_mul_t)(DREF DeeObject **__restrict p_self, DeeObject *some_object);

typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *Dee_tsc_seq_foreach_reverse_t)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
typedef WUNUSED_T NONNULL_T((1, 2)) Dee_ssize_t (DCALL *Dee_tsc_seq_enumerate_index_reverse_t)(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_trygetfirst_t)(DeeObject *__restrict self); /* @return: ITER_DONE: Sequence is empty */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_getfirst_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_seq_boundfirst_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_seq_delfirst_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_setfirst_t)(DeeObject *self, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_trygetlast_t)(DeeObject *__restrict self); /* @return: ITER_DONE: Sequence is empty */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_getlast_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_seq_boundlast_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_seq_dellast_t)(DeeObject *__restrict self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_setlast_t)(DeeObject *self, DeeObject *value);

/* Functions that need additional variants for sequence sub-types that don't have indices (sets, maps) */
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_seq_any_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_any_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_seq_any_with_range_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *Dee_tsc_seq_any_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_seq_all_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_all_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_seq_all_with_range_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *Dee_tsc_seq_all_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_seq_parity_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_parity_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_seq_parity_with_range_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *Dee_tsc_seq_parity_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_seq_reduce_t)(DeeObject *self, DeeObject *combine);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *Dee_tsc_seq_reduce_with_init_t)(DeeObject *self, DeeObject *combine, DeeObject *init);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_seq_reduce_with_range_t)(DeeObject *self, DeeObject *combine, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) DREF DeeObject *(DCALL *Dee_tsc_seq_reduce_with_range_and_init_t)(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);

typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_min_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_seq_min_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_min_with_range_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) DREF DeeObject *(DCALL *Dee_tsc_seq_min_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_max_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_seq_max_with_key_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_max_with_range_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) DREF DeeObject *(DCALL *Dee_tsc_seq_max_with_range_and_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_sum_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_sum_with_range_t)(DeeObject *self, size_t start, size_t end);

typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_seq_count_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) size_t (DCALL *Dee_tsc_seq_count_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_seq_count_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *Dee_tsc_seq_count_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_contains_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *Dee_tsc_seq_contains_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_contains_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *Dee_tsc_seq_contains_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_seq_locate_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *Dee_tsc_seq_locate_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_seq_locate_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) DREF DeeObject *(DCALL *Dee_tsc_seq_locate_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/*typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_seq_rlocate_t)(DeeObject *self, DeeObject *item);*/ /* Wouldn't make sense: for reverse, you need indices */
/*typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *Dee_tsc_seq_rlocate_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);*/ /* Wouldn't make sense: for reverse, you need indices */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_seq_rlocate_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) DREF DeeObject *(DCALL *Dee_tsc_seq_rlocate_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_startswith_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *Dee_tsc_seq_startswith_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_startswith_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *Dee_tsc_seq_startswith_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_endswith_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *Dee_tsc_seq_endswith_with_key_t)(DeeObject *self, DeeObject *item, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_endswith_with_range_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *Dee_tsc_seq_endswith_with_range_and_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);


/* @return: * :         Index of `item' in `self'
 * @return: (size_t)-1: `item' could not be located in `self'
 * @return: (size_t)Dee_COMPARE_ERR: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_seq_find_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *Dee_tsc_seq_find_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_seq_rfind_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *Dee_tsc_seq_rfind_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* Functions for mutable sequences. */
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_seq_erase_t)(DeeObject *self, size_t index, size_t count);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *Dee_tsc_seq_insert_t)(DeeObject *self, size_t index, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *Dee_tsc_seq_insertall_t)(DeeObject *self, size_t index, DeeObject *items);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_pushfront_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_append_t)(DeeObject *self, DeeObject *item);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_extend_t)(DeeObject *self, DeeObject *items);
typedef WUNUSED_T NONNULL_T((1, 3)) DREF DeeObject *(DCALL *Dee_tsc_seq_xchitem_index_t)(DeeObject *self, size_t index, DeeObject *value);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_seq_clear_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_pop_t)(DeeObject *self, Dee_ssize_t index); /* When index is negative, count from end of sequence */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_remove_t)(DeeObject *self, DeeObject *item, size_t start, size_t end); /* @return: 1: Removed; 0: Not removed; -1: Error */
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *Dee_tsc_seq_remove_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key); /* @return: 1: Removed; 0: Not removed; -1: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_seq_rremove_t)(DeeObject *self, DeeObject *item, size_t start, size_t end); /* @return: 1: Removed; 0: Not removed; -1: Error */
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *Dee_tsc_seq_rremove_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key); /* @return: 1: Removed; 0: Not removed; -1: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_seq_removeall_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
typedef WUNUSED_T NONNULL_T((1, 2, 6)) size_t (DCALL *Dee_tsc_seq_removeall_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_seq_removeif_t)(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
typedef WUNUSED_T NONNULL_T((1, 3)) int (DCALL *Dee_tsc_seq_resize_t)(DeeObject *self, size_t newsize, DeeObject *filler);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *Dee_tsc_seq_fill_t)(DeeObject *self, size_t start, size_t end, DeeObject *filler);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_seq_reverse_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_reversed_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1)) int (DCALL *Dee_tsc_seq_sort_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) int (DCALL *Dee_tsc_seq_sort_with_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_sorted_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 4)) DREF DeeObject *(DCALL *Dee_tsc_seq_sorted_with_key_t)(DeeObject *self, size_t start, size_t end, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_seq_bfind_t)(DeeObject *self, DeeObject *item, size_t start, size_t end); /* @return: (size_t)-1: Not found; @return (size_t)Dee_COMPARE_ERR: Error */
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *Dee_tsc_seq_bfind_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key); /* @return: (size_t)-1: Not found; @return (size_t)Dee_COMPARE_ERR: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) size_t (DCALL *Dee_tsc_seq_bposition_t)(DeeObject *self, DeeObject *item, size_t start, size_t end); /* @return: (size_t)Dee_COMPARE_ERR: Error */
typedef WUNUSED_T NONNULL_T((1, 2, 5)) size_t (DCALL *Dee_tsc_seq_bposition_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key); /* @return: (size_t)Dee_COMPARE_ERR: Error */
typedef WUNUSED_T NONNULL_T((1, 2, 5)) int (DCALL *Dee_tsc_seq_brange_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t result_range[2]);
typedef WUNUSED_T NONNULL_T((1, 2, 5, 6)) int (DCALL *Dee_tsc_seq_brange_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key, size_t result_range[2]);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_seq_blocate_t)(DeeObject *self, DeeObject *item, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 5)) DREF DeeObject *(DCALL *Dee_tsc_seq_blocate_with_key_t)(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

/* Operators for the purpose of constructing `DefaultEnumeration_With*' objects. */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_makeenumeration_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_seq_makeenumeration_with_int_range_t)(DeeObject *self, size_t start, size_t end);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *Dee_tsc_seq_makeenumeration_with_range_t)(DeeObject *self, DeeObject *start, DeeObject *end);







/************************************************************************/
/* For `deemon.Set'                                                     */
/************************************************************************/
/* Insert a key into a set
 * @return: 1 : Given `key' was inserted and wasn't already present
 * @return: 0 : Given `key' was already present
 * @return: -1: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_set_insert_t)(DeeObject *self, DeeObject *key);

/* Remove a key from a set
 * @return: 1 : Given `key' was removed
 * @return: 0 : Given `key' was wasn't present
 * @return: -1: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_set_remove_t)(DeeObject *self, DeeObject *key);

/* Insert `key' if not already present and re-return `key'.
 * If already present, return the pre-existing (and equal) instance instead.
 * @return: NULL: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_set_unify_t)(DeeObject *self, DeeObject *key);

/* @return: 0 : Success
 * @return: -1: Error  */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_set_insertall_t)(DeeObject *self, DeeObject *keys);
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_set_removeall_t)(DeeObject *self, DeeObject *keys);

typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_set_pop_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_set_pop_with_default_t)(DeeObject *self, DeeObject *default_);








/************************************************************************/
/* For `deemon.Mapping'                                                 */
/************************************************************************/

/* Override the value of a pre-existing key
 * @param: value: The value to overwrite that of `key' with (so-long as `key' already exists)
 * @return: 1 :   The value of `key' was set to `value'
 * @return: 0 :   The given `key' doesn't exist (nothing was updated)
 * @return: -1:   Error */
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *Dee_tsc_map_setold_t)(DeeObject *self, DeeObject *key, DeeObject *value);
/* @return: * :        The value of `key' was set to `value' (returned object is the old value)
 * @return: ITER_DONE: The given `key' doesn't exist (nothing was updated)
 * @return: NULL:      Error */
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *Dee_tsc_map_setold_ex_t)(DeeObject *self, DeeObject *key, DeeObject *value);

/* Insert a new key whilst making sure that the key doesn't already exist
 * @param: value: The value to overwrite that of `key' with (so-long as `key' already exists)
 * @return: 1 :   The value of `key' was set to `value' (the key didn't exist or used to be unbound)
 * @return: 0 :   The given `key' already exists (nothing was inserted)
 * @return: -1:   Error */
typedef WUNUSED_T NONNULL_T((1, 2, 3)) int (DCALL *Dee_tsc_map_setnew_t)(DeeObject *self, DeeObject *key, DeeObject *value);
/* @return: ITER_DONE: The value of `key' was set to `value' (the key didn't exist or used to be unbound)
 * @return: * :        The given `key' already exists (nothing was inserted; returned object is the already-present value)
 * @return: -1:        Error */
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *Dee_tsc_map_setnew_ex_t)(DeeObject *self, DeeObject *key, DeeObject *value);

/* Same semantic functionality as `Dee_tsc_map_setnew_ex_t': insert if not already present
 * @return: * : The value associated with key after the call:
 *              - if already present and nothing was inserted, its old value
 *              - if used-to-be absent/unbound and was assigned/inserted, `value'
 * @return: NULL: Error */
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *Dee_tsc_map_setdefault_t)(DeeObject *self, DeeObject *key, DeeObject *value);

/* Copy all key-value pairs from `items' and assign them to `self'.
 * Same as `for (local key, value: items) self[key] = value;'
 * @return: 0 : Success
 * @return: -1: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_map_update_t)(DeeObject *self, DeeObject *items);

/* Remove a single key, returning true/false indicative of that key having been removed.
 * @return: 1 : Key was removed
 * @return: 0 : Key didn't exist (nothing was removed)
 * @return: -1: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_map_remove_t)(DeeObject *self, DeeObject *key);

/* Delete all keys that appear in `keys'.
 * Same as `for (local key: keys) del self[key];'
 * @return: 0 : Success
 * @return: -1: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) int (DCALL *Dee_tsc_map_removekeys_t)(DeeObject *self, DeeObject *keys);

/* Remove/unbind `key' and return whatever used to be assigned to it.
 * When the key was already absent/unbound, return `default_' or throw a `KeyError'
 * @return: * :   The old value of `key'
 * @return: NULL: Error */
typedef WUNUSED_T NONNULL_T((1, 2)) DREF DeeObject *(DCALL *Dee_tsc_map_pop_t)(DeeObject *self, DeeObject *key);
typedef WUNUSED_T NONNULL_T((1, 2, 3)) DREF DeeObject *(DCALL *Dee_tsc_map_pop_with_default_t)(DeeObject *self, DeeObject *key, DeeObject *default_);

/* Remove a random key/value pair from `self' and store it in `key_and_value' (returns "none" if nothing found) */
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_map_popitem_t)(DeeObject *self);

typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_map_keys_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_map_values_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_map_iterkeys_t)(DeeObject *self);
typedef WUNUSED_T NONNULL_T((1)) DREF DeeObject *(DCALL *Dee_tsc_map_itervalues_t)(DeeObject *self);





union Dee_tsc_uslot {
	DREF DeeObject   *d_function;  /* [1..1][valid_if(:tsc_* == ...)] Thiscall function. */
	Dee_objmethod_t   d_method;    /* [1..1][valid_if(:tsc_* == ...)] Method callback. */
	Dee_kwobjmethod_t d_kwmethod;  /* [1..1][valid_if(:tsc_* == ...)] Method callback. */
};

struct Dee_type_seq_cache {
	/************************************************************************/
	/* For `deemon.Sequence'                                                */
	/************************************************************************/

	/* Operators... */
	Dee_tsc_operator_bool_t             tsc_seq_operator_bool;
	Dee_tsc_operator_iter_t             tsc_seq_operator_iter;
	Dee_tsc_operator_sizeob_t           tsc_seq_operator_sizeob;
	Dee_tsc_operator_contains_t         tsc_seq_operator_contains;
	Dee_tsc_operator_getitem_t          tsc_seq_operator_getitem;
	Dee_tsc_operator_delitem_t          tsc_seq_operator_delitem;
	Dee_tsc_operator_setitem_t          tsc_seq_operator_setitem;
	Dee_tsc_operator_getrange_t         tsc_seq_operator_getrange;
	Dee_tsc_operator_delrange_t         tsc_seq_operator_delrange;
	Dee_tsc_operator_setrange_t         tsc_seq_operator_setrange;
	Dee_tsc_operator_foreach_t          tsc_seq_operator_foreach;
	Dee_tsc_operator_enumerate_t        tsc_seq_operator_enumerate;
	Dee_tsc_operator_enumerate_index_t  tsc_seq_operator_enumerate_index;
	Dee_tsc_operator_bounditem_t        tsc_seq_operator_bounditem;
	Dee_tsc_operator_hasitem_t          tsc_seq_operator_hasitem;
	Dee_tsc_operator_size_t             tsc_seq_operator_size;
	Dee_tsc_operator_size_fast_t        tsc_seq_operator_size_fast;
	Dee_tsc_operator_getitem_index_t    tsc_seq_operator_getitem_index;
	Dee_tsc_operator_delitem_index_t    tsc_seq_operator_delitem_index;
	Dee_tsc_operator_setitem_index_t    tsc_seq_operator_setitem_index;
	Dee_tsc_operator_bounditem_index_t  tsc_seq_operator_bounditem_index;
	Dee_tsc_operator_hasitem_index_t    tsc_seq_operator_hasitem_index;
	Dee_tsc_operator_getrange_index_t   tsc_seq_operator_getrange_index;
	Dee_tsc_operator_delrange_index_t   tsc_seq_operator_delrange_index;
	Dee_tsc_operator_setrange_index_t   tsc_seq_operator_setrange_index;
	Dee_tsc_operator_getrange_index_n_t tsc_seq_operator_getrange_index_n;
	Dee_tsc_operator_delrange_index_n_t tsc_seq_operator_delrange_index_n;
	Dee_tsc_operator_setrange_index_n_t tsc_seq_operator_setrange_index_n;
	Dee_tsc_operator_trygetitem_t       tsc_seq_operator_trygetitem;
	Dee_tsc_operator_trygetitem_index_t tsc_seq_operator_trygetitem_index;
	Dee_tsc_operator_hash_t             tsc_seq_operator_hash;
	Dee_tsc_operator_compare_eq_t       tsc_seq_operator_compare_eq;
	Dee_tsc_operator_compare_t          tsc_seq_operator_compare;
	Dee_tsc_operator_trycompare_eq_t    tsc_seq_operator_trycompare_eq;
	Dee_tsc_operator_eq_t               tsc_seq_operator_eq;
	Dee_tsc_operator_ne_t               tsc_seq_operator_ne;
	Dee_tsc_operator_lo_t               tsc_seq_operator_lo;
	Dee_tsc_operator_le_t               tsc_seq_operator_le;
	Dee_tsc_operator_gr_t               tsc_seq_operator_gr;
	Dee_tsc_operator_ge_t               tsc_seq_operator_ge;
	Dee_tsc_operator_inplace_add_t      tsc_seq_operator_inplace_add;
	Dee_tsc_operator_inplace_mul_t      tsc_seq_operator_inplace_mul;

	/* Common utility functions... */
	Dee_tsc_seq_foreach_reverse_t         tsc_seq_foreach_reverse;
	Dee_tsc_seq_enumerate_index_reverse_t tsc_seq_enumerate_index_reverse;

	/* Operators for the purpose of constructing `DefaultEnumeration_With*' objects. */
	Dee_tsc_seq_makeenumeration_t                tsc_seq_makeenumeration;
	Dee_tsc_seq_makeenumeration_with_int_range_t tsc_seq_makeenumeration_with_int_range;
	Dee_tsc_seq_makeenumeration_with_range_t     tsc_seq_makeenumeration_with_range;

	/* Returns the first element of the sequence.
	 * Calls `err_empty_sequence()' when it is empty. */
	Dee_tsc_seq_trygetfirst_t tsc_seq_trygetfirst;
	Dee_tsc_seq_getfirst_t    tsc_seq_getfirst;
	union Dee_tsc_uslot       tsc_seq_getfirst_data;
	Dee_tsc_seq_boundfirst_t  tsc_seq_boundfirst;
	Dee_tsc_seq_delfirst_t    tsc_seq_delfirst;
	union Dee_tsc_uslot       tsc_seq_delfirst_data;
	Dee_tsc_seq_setfirst_t    tsc_seq_setfirst;
	union Dee_tsc_uslot       tsc_seq_setfirst_data;

	/* Returns the last element of the sequence.
	 * Calls `err_empty_sequence()' when it is empty. */
	Dee_tsc_seq_trygetlast_t tsc_seq_trygetlast;
	Dee_tsc_seq_getlast_t    tsc_seq_getlast;
	union Dee_tsc_uslot      tsc_seq_getlast_data;
	Dee_tsc_seq_boundlast_t  tsc_seq_boundlast;
	Dee_tsc_seq_dellast_t    tsc_seq_dellast;
	union Dee_tsc_uslot      tsc_seq_dellast_data;
	Dee_tsc_seq_setlast_t    tsc_seq_setlast;
	union Dee_tsc_uslot      tsc_seq_setlast_data;

	/* Functions that need additional variants for sequence sub-types that don't have indices (sets, maps) */
	Dee_tsc_seq_any_t                           tsc_seq_any;
	Dee_tsc_seq_any_with_key_t                  tsc_seq_any_with_key;
	Dee_tsc_seq_any_with_range_t                tsc_seq_any_with_range;
	Dee_tsc_seq_any_with_range_and_key_t        tsc_seq_any_with_range_and_key;
	union Dee_tsc_uslot                     tsc_seq_any_data;
	Dee_tsc_seq_all_t                           tsc_seq_all;
	Dee_tsc_seq_all_with_key_t                  tsc_seq_all_with_key;
	Dee_tsc_seq_all_with_range_t                tsc_seq_all_with_range;
	Dee_tsc_seq_all_with_range_and_key_t        tsc_seq_all_with_range_and_key;
	union Dee_tsc_uslot                     tsc_seq_all_data;
	Dee_tsc_seq_parity_t                        tsc_seq_parity;
	Dee_tsc_seq_parity_with_key_t               tsc_seq_parity_with_key;
	Dee_tsc_seq_parity_with_range_t             tsc_seq_parity_with_range;
	Dee_tsc_seq_parity_with_range_and_key_t     tsc_seq_parity_with_range_and_key;
	union Dee_tsc_uslot                         tsc_seq_parity_data;
	Dee_tsc_seq_reduce_t                        tsc_seq_reduce;
	Dee_tsc_seq_reduce_with_init_t              tsc_seq_reduce_with_init;
	Dee_tsc_seq_reduce_with_range_t             tsc_seq_reduce_with_range;
	Dee_tsc_seq_reduce_with_range_and_init_t    tsc_seq_reduce_with_range_and_init;
	union Dee_tsc_uslot                         tsc_seq_reduce_data;
	Dee_tsc_seq_min_t                           tsc_seq_min;
	Dee_tsc_seq_min_with_key_t                  tsc_seq_min_with_key;
	Dee_tsc_seq_min_with_range_t                tsc_seq_min_with_range;
	Dee_tsc_seq_min_with_range_and_key_t        tsc_seq_min_with_range_and_key;
	union Dee_tsc_uslot                         tsc_seq_min_data;
	Dee_tsc_seq_max_t                           tsc_seq_max;
	Dee_tsc_seq_max_with_key_t                  tsc_seq_max_with_key;
	Dee_tsc_seq_max_with_range_t                tsc_seq_max_with_range;
	Dee_tsc_seq_max_with_range_and_key_t        tsc_seq_max_with_range_and_key;
	union Dee_tsc_uslot                         tsc_seq_max_data;
	Dee_tsc_seq_sum_t                           tsc_seq_sum;
	Dee_tsc_seq_sum_with_range_t                tsc_seq_sum_with_range;
	union Dee_tsc_uslot                         tsc_seq_sum_data;
	Dee_tsc_seq_count_t                         tsc_seq_count;
	Dee_tsc_seq_count_with_key_t                tsc_seq_count_with_key;
	Dee_tsc_seq_count_with_range_t              tsc_seq_count_with_range;
	Dee_tsc_seq_count_with_range_and_key_t      tsc_seq_count_with_range_and_key;
	union Dee_tsc_uslot                         tsc_seq_count_data;
	Dee_tsc_seq_contains_t                      tsc_seq_contains;
	Dee_tsc_seq_contains_with_key_t             tsc_seq_contains_with_key;
	Dee_tsc_seq_contains_with_range_t           tsc_seq_contains_with_range;
	Dee_tsc_seq_contains_with_range_and_key_t   tsc_seq_contains_with_range_and_key;
	union Dee_tsc_uslot                         tsc_seq_contains_data;
	Dee_tsc_seq_locate_t                        tsc_seq_locate;
	Dee_tsc_seq_locate_with_key_t               tsc_seq_locate_with_key;
	Dee_tsc_seq_locate_with_range_t             tsc_seq_locate_with_range;
	Dee_tsc_seq_locate_with_range_and_key_t     tsc_seq_locate_with_range_and_key;
	union Dee_tsc_uslot                         tsc_seq_locate_data;
/*	Dee_tsc_seq_rlocate_t                       tsc_seq_rlocate;*/
/*	Dee_tsc_seq_rlocate_with_key_t              tsc_seq_rlocate_with_key;*/
	Dee_tsc_seq_rlocate_with_range_t            tsc_seq_rlocate_with_range;
	Dee_tsc_seq_rlocate_with_range_and_key_t    tsc_seq_rlocate_with_range_and_key;
	union Dee_tsc_uslot                         tsc_seq_rlocate_data;
	Dee_tsc_seq_startswith_t                    tsc_seq_startswith;
	Dee_tsc_seq_startswith_with_key_t           tsc_seq_startswith_with_key;
	Dee_tsc_seq_startswith_with_range_t         tsc_seq_startswith_with_range;
	Dee_tsc_seq_startswith_with_range_and_key_t tsc_seq_startswith_with_range_and_key;
	union Dee_tsc_uslot                         tsc_seq_startswith_data;
	Dee_tsc_seq_endswith_t                      tsc_seq_endswith;
	Dee_tsc_seq_endswith_with_key_t             tsc_seq_endswith_with_key;
	Dee_tsc_seq_endswith_with_range_t           tsc_seq_endswith_with_range;
	Dee_tsc_seq_endswith_with_range_and_key_t   tsc_seq_endswith_with_range_and_key;
	union Dee_tsc_uslot                         tsc_seq_endswith_data;

	/* Sequence functions. */
	Dee_tsc_seq_find_t               tsc_seq_find;
	Dee_tsc_seq_find_with_key_t      tsc_seq_find_with_key;
	union Dee_tsc_uslot              tsc_seq_find_data;
	Dee_tsc_seq_rfind_t              tsc_seq_rfind;
	Dee_tsc_seq_rfind_with_key_t     tsc_seq_rfind_with_key;
	union Dee_tsc_uslot              tsc_seq_rfind_data;
	Dee_tsc_seq_erase_t              tsc_seq_erase;
	union Dee_tsc_uslot              tsc_seq_erase_data;
	Dee_tsc_seq_insert_t             tsc_seq_insert;
	union Dee_tsc_uslot              tsc_seq_insert_data;
	Dee_tsc_seq_insertall_t          tsc_seq_insertall;
	union Dee_tsc_uslot              tsc_seq_insertall_data;
	Dee_tsc_seq_pushfront_t          tsc_seq_pushfront;
	union Dee_tsc_uslot              tsc_seq_pushfront_data;
	Dee_tsc_seq_append_t             tsc_seq_append;
	union Dee_tsc_uslot              tsc_seq_append_data;
	Dee_tsc_seq_extend_t             tsc_seq_extend;
	union Dee_tsc_uslot              tsc_seq_extend_data;
	Dee_tsc_seq_xchitem_index_t      tsc_seq_xchitem_index;
	union Dee_tsc_uslot              tsc_seq_xchitem_data;
	Dee_tsc_seq_clear_t              tsc_seq_clear;
	union Dee_tsc_uslot              tsc_seq_clear_data;
	Dee_tsc_seq_pop_t                tsc_seq_pop;
	union Dee_tsc_uslot              tsc_seq_pop_data;
	Dee_tsc_seq_remove_t             tsc_seq_remove;
	Dee_tsc_seq_remove_with_key_t    tsc_seq_remove_with_key;
	union Dee_tsc_uslot              tsc_seq_remove_data;
	Dee_tsc_seq_rremove_t            tsc_seq_rremove;
	Dee_tsc_seq_rremove_with_key_t   tsc_seq_rremove_with_key;
	union Dee_tsc_uslot              tsc_seq_rremove_data;
	Dee_tsc_seq_removeall_t          tsc_seq_removeall;
	Dee_tsc_seq_removeall_with_key_t tsc_seq_removeall_with_key;
	union Dee_tsc_uslot              tsc_seq_removeall_data;
	Dee_tsc_seq_removeif_t           tsc_seq_removeif;
	union Dee_tsc_uslot              tsc_seq_removeif_data;
	Dee_tsc_seq_resize_t             tsc_seq_resize;
	union Dee_tsc_uslot              tsc_seq_resize_data;
	Dee_tsc_seq_fill_t               tsc_seq_fill;
	union Dee_tsc_uslot              tsc_seq_fill_data;
	Dee_tsc_seq_reverse_t            tsc_seq_reverse;
	union Dee_tsc_uslot              tsc_seq_reverse_data;
	Dee_tsc_seq_reversed_t           tsc_seq_reversed;
	union Dee_tsc_uslot              tsc_seq_reversed_data;
	Dee_tsc_seq_sort_t               tsc_seq_sort;
	Dee_tsc_seq_sort_with_key_t      tsc_seq_sort_with_key;
	union Dee_tsc_uslot              tsc_seq_sort_data;
	Dee_tsc_seq_sorted_t             tsc_seq_sorted;
	Dee_tsc_seq_sorted_with_key_t    tsc_seq_sorted_with_key;
	union Dee_tsc_uslot              tsc_seq_sorted_data;
	Dee_tsc_seq_bfind_t              tsc_seq_bfind;
	Dee_tsc_seq_bfind_with_key_t     tsc_seq_bfind_with_key;
	union Dee_tsc_uslot              tsc_seq_bfind_data;
	Dee_tsc_seq_bposition_t          tsc_seq_bposition;
	Dee_tsc_seq_bposition_with_key_t tsc_seq_bposition_with_key;
	union Dee_tsc_uslot              tsc_seq_bposition_data;
	Dee_tsc_seq_brange_t             tsc_seq_brange;
	Dee_tsc_seq_brange_with_key_t    tsc_seq_brange_with_key;
	union Dee_tsc_uslot              tsc_seq_brange_data;
	Dee_tsc_seq_blocate_t            tsc_seq_blocate;
	Dee_tsc_seq_blocate_with_key_t   tsc_seq_blocate_with_key;
	union Dee_tsc_uslot              tsc_seq_blocate_data;

	/************************************************************************/
	/* For `deemon.Set'                                                     */
	/************************************************************************/
	/* TODO: Set operators */
	Dee_tsc_set_insert_t           tsc_set_insert;
	union Dee_tsc_uslot            tsc_set_insert_data;
	Dee_tsc_set_remove_t           tsc_set_remove;
	union Dee_tsc_uslot            tsc_set_remove_data;
	Dee_tsc_set_unify_t            tsc_set_unify;
	union Dee_tsc_uslot            tsc_set_unify_data;
	Dee_tsc_set_insertall_t        tsc_set_insertall;
	union Dee_tsc_uslot            tsc_set_insertall_data;
	Dee_tsc_set_removeall_t        tsc_set_removeall;
	union Dee_tsc_uslot            tsc_set_removeall_data;
	Dee_tsc_set_pop_t              tsc_set_pop;
	Dee_tsc_set_pop_with_default_t tsc_set_pop_with_default;
	union Dee_tsc_uslot            tsc_set_pop_data;

	/************************************************************************/
	/* For `deemon.Mapping'                                                 */
	/************************************************************************/
	/* TODO: Map operators */
	Dee_tsc_map_setold_t           tsc_map_setold;
	union Dee_tsc_uslot            tsc_map_setold_data;
	Dee_tsc_map_setold_ex_t        tsc_map_setold_ex;
	union Dee_tsc_uslot            tsc_map_setold_ex_data;
	Dee_tsc_map_setnew_t           tsc_map_setnew;
	union Dee_tsc_uslot            tsc_map_setnew_data;
	Dee_tsc_map_setnew_ex_t        tsc_map_setnew_ex;
	union Dee_tsc_uslot            tsc_map_setnew_ex_data;
	Dee_tsc_map_setdefault_t       tsc_map_setdefault;
	union Dee_tsc_uslot            tsc_map_setdefault_data;
	Dee_tsc_map_update_t           tsc_map_update;
	union Dee_tsc_uslot            tsc_map_update_data;
	Dee_tsc_map_remove_t           tsc_map_remove;
	union Dee_tsc_uslot            tsc_map_remove_data;
	Dee_tsc_map_removekeys_t       tsc_map_removekeys;
	union Dee_tsc_uslot            tsc_map_removekeys_data;
	Dee_tsc_map_pop_t              tsc_map_pop;
	Dee_tsc_map_pop_with_default_t tsc_map_pop_with_default;
	union Dee_tsc_uslot            tsc_map_pop_data;
	Dee_tsc_map_popitem_t          tsc_map_popitem;
	union Dee_tsc_uslot            tsc_map_popitem_data;
	Dee_tsc_map_keys_t             tsc_map_keys;
	union Dee_tsc_uslot            tsc_map_keys_data;
	Dee_tsc_map_values_t           tsc_map_values;
	union Dee_tsc_uslot            tsc_map_values_data;
	Dee_tsc_map_iterkeys_t         tsc_map_iterkeys;
	union Dee_tsc_uslot            tsc_map_iterkeys_data;
	Dee_tsc_map_itervalues_t       tsc_map_itervalues;
	union Dee_tsc_uslot            tsc_map_itervalues_data;
};

/* Destroy a lazily allocated sequence operator cache table. */
INTDEF NONNULL((1)) void DCALL
Dee_type_seq_cache_destroy(struct Dee_type_seq_cache *__restrict self);

INTDEF WUNUSED NONNULL((1)) struct Dee_type_seq_cache *DCALL
DeeType_TryRequireSeqCache(DeeTypeObject *__restrict self);

/* Type sequence operator definition functions. */
INTDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_tsc_seq_foreach_reverse_t DCALL DeeType_TryRequireSeqForeachReverse(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE WUNUSED NONNULL((1)) Dee_tsc_seq_enumerate_index_reverse_t DCALL DeeType_TryRequireSeqEnumerateIndexReverse(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL DeeType_HasPrivateSeqEnumerateIndexReverse(DeeTypeObject *orig_type, DeeTypeObject *self);

/* Operators for the purpose of constructing `DefaultEnumeration_With*' objects. */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_makeenumeration_t DCALL DeeType_RequireSeqMakeEnumeration(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_makeenumeration_with_int_range_t DCALL DeeType_RequireSeqMakeEnumerationWithIntRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_makeenumeration_with_range_t DCALL DeeType_RequireSeqMakeEnumerationWithRange(DeeTypeObject *__restrict self);

/* Sequence operators... */
/*[[[begin:seq_operators]]]*/
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_bool_t DCALL DeeType_RequireSeqOperatorBool(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_iter_t DCALL DeeType_RequireSeqOperatorIter(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_sizeob_t DCALL DeeType_RequireSeqOperatorSizeOb(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_contains_t DCALL DeeType_RequireSeqOperatorContains(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_getitem_t DCALL DeeType_RequireSeqOperatorGetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_delitem_t DCALL DeeType_RequireSeqOperatorDelItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_setitem_t DCALL DeeType_RequireSeqOperatorSetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_getrange_t DCALL DeeType_RequireSeqOperatorGetRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_delrange_t DCALL DeeType_RequireSeqOperatorDelRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_setrange_t DCALL DeeType_RequireSeqOperatorSetRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_foreach_t DCALL DeeType_RequireSeqOperatorForeach(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_enumerate_t DCALL DeeType_RequireSeqOperatorEnumerate(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_enumerate_index_t DCALL DeeType_RequireSeqOperatorEnumerateIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_bounditem_t DCALL DeeType_RequireSeqOperatorBoundItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_hasitem_t DCALL DeeType_RequireSeqOperatorHasItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_size_t DCALL DeeType_RequireSeqOperatorSize(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_size_fast_t DCALL DeeType_RequireSeqOperatorSizeFast(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_getitem_index_t DCALL DeeType_RequireSeqOperatorGetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_delitem_index_t DCALL DeeType_RequireSeqOperatorDelItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_setitem_index_t DCALL DeeType_RequireSeqOperatorSetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_bounditem_index_t DCALL DeeType_RequireSeqOperatorBoundItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_hasitem_index_t DCALL DeeType_RequireSeqOperatorHasItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_getrange_index_t DCALL DeeType_RequireSeqOperatorGetRangeIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_delrange_index_t DCALL DeeType_RequireSeqOperatorDelRangeIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_setrange_index_t DCALL DeeType_RequireSeqOperatorSetRangeIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_getrange_index_n_t DCALL DeeType_RequireSeqOperatorGetRangeIndexN(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_delrange_index_n_t DCALL DeeType_RequireSeqOperatorDelRangeIndexN(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_setrange_index_n_t DCALL DeeType_RequireSeqOperatorSetRangeIndexN(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_trygetitem_t DCALL DeeType_RequireSeqOperatorTryGetItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_trygetitem_index_t DCALL DeeType_RequireSeqOperatorTryGetItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_hash_t DCALL DeeType_RequireSeqOperatorHash(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_compare_eq_t DCALL DeeType_RequireSeqOperatorCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_compare_t DCALL DeeType_RequireSeqOperatorCompare(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_trycompare_eq_t DCALL DeeType_RequireSeqOperatorTryCompareEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_eq_t DCALL DeeType_RequireSeqOperatorEq(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_ne_t DCALL DeeType_RequireSeqOperatorNe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_lo_t DCALL DeeType_RequireSeqOperatorLo(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_le_t DCALL DeeType_RequireSeqOperatorLe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_gr_t DCALL DeeType_RequireSeqOperatorGr(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_ge_t DCALL DeeType_RequireSeqOperatorGe(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_inplace_add_t DCALL DeeType_RequireSeqOperatorInplaceAdd(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_operator_inplace_mul_t DCALL DeeType_RequireSeqOperatorInplaceMul(DeeTypeObject *__restrict self);
/*[[[end:seq_operators]]]*/

/*
 * List default implementations returned by `DeeType_RequireSeqOperator*'
 */

/*[[[deemon
import * from deemon;
import ipc;
local operatorNames = [];
local inOperators = false;
for (local l: File.open(__FILE__, "rb")) {
	l = l.strip();
	if (!inOperators) {
		if (l == "/" "*[[[begin:seq_operators]]]*" "/")
			inOperators = true;
		continue;
	}
	if (l == "/" "*[[[end:seq_operators]]]*" "/")
		break;
	local START_MARKER = "DeeType_RequireSeqOperator";
	local start = l.find(START_MARKER);
	if (start < 0)
		continue;
	start += #START_MARKER;
	local end = l.find("(", start);
	if (end < 0)
		continue;
	local name = l[start:end].strip();
	if (name !is none)
		operatorNames.append(name);
}

for (local name: operatorNames) {
	local proc = ipc.Process(ipc.Process.current.exe, [
		"deemon", "-E",
		f"-DLOCAL_FOR_VARIANTS",
		f"-DDEFINE_DeeType_RequireSeqOperator{name}",
		"default-api-require-operator-impl.c.inl"
	]);
	local r, w = ipc.Pipe.new()...;
	proc.stdout = w;
	proc.start();
	w.close();
	local data = r.readall();
	local e = proc.join();
	if (e != 0)
		throw Error(f"Process failed: {e}");
	data = "\n".join(for (local l: data.splitlines(false).each.strip())
		if (l && !l.startswith("#")) l
	);
	local impls = [f"DeeSeq_Operator{name}"];
	local i = 0, end = #data;
	while (i < end) {
		none, i = data.refind(r"\breturn\b", i)...;
		if (i is none)
			break;
		local exprEnd = data.find(";", i);
		if (exprEnd < 0)
			break;
		local expr = data[i:exprEnd].strip().strip("&").strip();
		if (expr !in impls)
			impls.append(expr);
		i = exprEnd + 1;
	}

	local pad = " " * ((operatorNames.each.length > ...) - #name);
	print(f"#define DeeType_SeqCache_Variants_Operator{name}(cb){pad}"),;
	for (local impl: impls)
		print(f" cb({impl})"),;
	print;
}
]]]*/
#define DeeType_SeqCache_Variants_OperatorBool(cb)            cb(DeeSeq_OperatorBool) cb((*(Dee_tsc_operator_bool_t)&none_i1)) cb(DeeSeq_DefaultBoolWithSize) cb(DeeSeq_DefaultBoolWithSizeOb) cb(DeeSeq_DefaultBoolWithForeach) cb(DeeSeq_DefaultBoolWithCompareEq) cb(DeeSeq_DefaultBoolWithEq) cb(DeeSeq_DefaultBoolWithNe) cb(DeeSeq_DefaultBoolWithForeachDefault) cb(DeeSeq_DefaultOperatorBoolWithError)
#define DeeType_SeqCache_Variants_OperatorIter(cb)            cb(DeeSeq_OperatorIter) cb(DeeSeq_DefaultOperatorIterWithError)
#define DeeType_SeqCache_Variants_OperatorSizeOb(cb)          cb(DeeSeq_OperatorSizeOb) cb(DeeSeq_DefaultOperatorSizeObWithEmpty) cb(DeeSeq_DefaultOperatorSizeObWithSeqOperatorSize) cb(DeeSeq_DefaultOperatorSizeObWithError)
#define DeeType_SeqCache_Variants_OperatorContains(cb)        cb(DeeSeq_OperatorContains) cb(DeeSeq_DefaultOperatorContainsWithMapTryGetItem) cb(DeeSeq_DefaultOperatorContainsWithEmpty) cb(DeeSeq_DefaultContainsWithForeachDefault) cb(DeeSeq_DefaultOperatorContainsWithError)
#define DeeType_SeqCache_Variants_OperatorGetItem(cb)         cb(DeeSeq_OperatorGetItem) cb(DeeSeq_DefaultOperatorGetItemWithEmpty) cb(DeeSeq_DefaultOperatorGetItemWithSeqGetItemIndex) cb(DeeSeq_DefaultOperatorGetItemWithError)
#define DeeType_SeqCache_Variants_OperatorDelItem(cb)         cb(DeeSeq_OperatorDelItem) cb(DeeSeq_DefaultOperatorDelItemWithError)
#define DeeType_SeqCache_Variants_OperatorSetItem(cb)         cb(DeeSeq_OperatorSetItem) cb(DeeSeq_DefaultOperatorSetItemWithError)
#define DeeType_SeqCache_Variants_OperatorGetRange(cb)        cb(DeeSeq_OperatorGetRange) cb(DeeSeq_DefaultOperatorGetRangeWithEmpty) cb(DeeSeq_DefaultOperatorGetRangeWithSeqGetRangeIndexAndGetRangeIndexN) cb(DeeSeq_DefaultOperatorGetRangeWithError)
#define DeeType_SeqCache_Variants_OperatorDelRange(cb)        cb(DeeSeq_OperatorDelRange) cb(DeeSeq_DefaultOperatorDelRangeWithError)
#define DeeType_SeqCache_Variants_OperatorSetRange(cb)        cb(DeeSeq_OperatorSetRange) cb(DeeSeq_DefaultOperatorSetRangeWithError)
#define DeeType_SeqCache_Variants_OperatorForeach(cb)         cb(DeeSeq_OperatorForeach) cb(DeeSeq_DefaultOperatorForeachWithError)
#define DeeType_SeqCache_Variants_OperatorEnumerate(cb)       cb(DeeSeq_OperatorEnumerate) cb((*(Dee_tsc_operator_enumerate_t)&DeeSeq_DefaultOperatorForeachWithEmpty)) cb(DeeSeq_DefaultEnumerateWithCounterAndForeach) cb((*(Dee_tsc_operator_enumerate_t)&DeeSeq_DefaultOperatorForeachWithError))
#define DeeType_SeqCache_Variants_OperatorEnumerateIndex(cb)  cb(DeeSeq_OperatorEnumerateIndex) cb(DeeSeq_DefaultOperatorEnumerateIndexWithEmpty) cb(DeeSeq_DefaultEnumerateIndexWithCounterAndForeach) cb(DeeSeq_DefaultOperatorEnumerateIndexWithError)
#define DeeType_SeqCache_Variants_OperatorBoundItem(cb)       cb(DeeSeq_OperatorBoundItem) cb(DeeSeq_DefaultOperatorBoundItemWithEmpty) cb(DeeSeq_DefaultOperatorBoundItemWithSeqBoundItemIndex) cb(DeeSeq_DefaultOperatorBoundItemWithError)
#define DeeType_SeqCache_Variants_OperatorHasItem(cb)         cb(DeeSeq_OperatorHasItem) cb((*(Dee_tsc_operator_hasitem_t)&none_i2)) cb(DeeSeq_DefaultOperatorHasItemWithSeqHasItemIndex) cb(DeeSeq_DefaultOperatorHasItemWithError)
#define DeeType_SeqCache_Variants_OperatorSize(cb)            cb(DeeSeq_OperatorSize) cb(DeeSeq_DefaultOperatorSizeWithEmpty) cb(DeeSeq_DefaultSizeWithForeachPair) cb(DeeSeq_DefaultSizeWithForeach) cb(DeeSeq_DefaultOperatorSizeWithError)
#define DeeType_SeqCache_Variants_OperatorSizeFast(cb)        cb(DeeSeq_OperatorSizeFast) cb(DeeObject_DefaultSizeFastWithErrorNotFast)
#define DeeType_SeqCache_Variants_OperatorGetItemIndex(cb)    cb(DeeSeq_OperatorGetItemIndex) cb(DeeSeq_DefaultOperatorGetItemIndexWithEmpty) cb(DeeSeq_DefaultGetItemIndexWithForeachDefault) cb(DeeSeq_DefaultOperatorGetItemIndexWithError)
#define DeeType_SeqCache_Variants_OperatorDelItemIndex(cb)    cb(DeeSeq_OperatorDelItemIndex) cb(DeeSeq_DefaultOperatorDelItemIndexWithError)
#define DeeType_SeqCache_Variants_OperatorSetItemIndex(cb)    cb(DeeSeq_OperatorSetItemIndex) cb(DeeSeq_DefaultOperatorSetItemIndexWithError)
#define DeeType_SeqCache_Variants_OperatorBoundItemIndex(cb)  cb(DeeSeq_OperatorBoundItemIndex) cb((*(Dee_tsc_operator_bounditem_index_t)&DeeSeq_DefaultOperatorBoundItemWithEmpty)) cb(DeeSeq_DefaultOperatorBoundItemIndexWithSeqSize) cb(DeeSeq_DefaultOperatorBoundItemIndexWithError)
#define DeeType_SeqCache_Variants_OperatorHasItemIndex(cb)    cb(DeeSeq_OperatorHasItemIndex) cb((*(Dee_tsc_operator_hasitem_index_t)&(*(Dee_tsc_operator_hasitem_t)&none_i2))) cb(DeeSeq_DefaultOperatorHasItemIndexWithSeqSize) cb(DeeSeq_DefaultOperatorHasItemIndexWithError)
#define DeeType_SeqCache_Variants_OperatorGetRangeIndex(cb)   cb(DeeSeq_OperatorGetRangeIndex) cb((*(Dee_tsc_operator_getrange_index_t)&DeeSeq_DefaultOperatorGetRangeWithEmpty)) cb(DeeSeq_DefaultOperatorGetRangeIndexWithIterAndSeqSize) cb(DeeSeq_DefaultOperatorGetRangeIndexWithError)
#define DeeType_SeqCache_Variants_OperatorDelRangeIndex(cb)   cb(DeeSeq_OperatorDelRangeIndex) cb(DeeSeq_DefaultOperatorDelRangeIndexWithError)
#define DeeType_SeqCache_Variants_OperatorSetRangeIndex(cb)   cb(DeeSeq_OperatorSetRangeIndex) cb(DeeSeq_DefaultOperatorSetRangeIndexWithError)
#define DeeType_SeqCache_Variants_OperatorGetRangeIndexN(cb)  cb(DeeSeq_OperatorGetRangeIndexN) cb(DeeSeq_DefaultOperatorGetRangeIndexNWithEmpty) cb(DeeSeq_DefaultOperatorGetRangeIndexNWithIterAndSeqSize) cb(DeeSeq_DefaultOperatorGetRangeIndexNWithError)
#define DeeType_SeqCache_Variants_OperatorDelRangeIndexN(cb)  cb(DeeSeq_OperatorDelRangeIndexN) cb(DeeSeq_DefaultOperatorDelRangeIndexNWithError)
#define DeeType_SeqCache_Variants_OperatorSetRangeIndexN(cb)  cb(DeeSeq_OperatorSetRangeIndexN) cb(DeeSeq_DefaultOperatorSetRangeIndexNWithError)
#define DeeType_SeqCache_Variants_OperatorTryGetItem(cb)      cb(DeeSeq_OperatorTryGetItem) cb(DeeSeq_DefaultOperatorTryGetItemWithEmpty) cb(DeeSeq_DefaultOperatorTryGetItemWithSeqTryGetItemIndex) cb(DeeSeq_DefaultOperatorGetItemWithError)
#define DeeType_SeqCache_Variants_OperatorTryGetItemIndex(cb) cb(DeeSeq_OperatorTryGetItemIndex) cb((*(Dee_tsc_operator_trygetitem_index_t)&DeeSeq_DefaultOperatorTryGetItemWithEmpty)) cb(DeeSeq_DefaultTryGetItemIndexWithForeachDefault) cb(DeeSeq_DefaultOperatorGetItemIndexWithError)
#define DeeType_SeqCache_Variants_OperatorHash(cb)            cb(DeeSeq_OperatorHash) cb(DeeSeq_DefaultOperatorHashWithEmpty) cb(DeeSeq_DefaultHashWithForeachDefault) cb(DeeSeq_DefaultOperatorHashWithError)
#define DeeType_SeqCache_Variants_OperatorCompareEq(cb)       cb(DeeSeq_OperatorCompareEq) cb(DeeSeq_DefaultOperatorCompareWithEmpty) cb(DeeSeq_DefaultCompareEqWithForeachDefault) cb(DeeSeq_DefaultOperatorCompareEqWithError)
#define DeeType_SeqCache_Variants_OperatorCompare(cb)         cb(DeeSeq_OperatorCompare) cb(DeeSeq_DefaultOperatorCompareWithEmpty) cb(DeeSeq_DefaultCompareWithForeachDefault) cb(DeeSeq_DefaultOperatorCompareWithError)
#define DeeType_SeqCache_Variants_OperatorTryCompareEq(cb)    cb(DeeSeq_OperatorTryCompareEq) cb(DeeSeq_DefaultOperatorTryCompareEqWithEmpty) cb(DeeSeq_DefaultCompareEqWithForeachDefault) cb(DeeSeq_DefaultOperatorTryCompareEqWithError)
#define DeeType_SeqCache_Variants_OperatorEq(cb)              cb(DeeSeq_OperatorEq) cb(DeeSeq_DefaultOperatorEqWithEmpty) cb(DeeSeq_DefaultOperatorEqWithSeqCompareEq) cb(DeeSeq_DefaultOperatorEqWithError)
#define DeeType_SeqCache_Variants_OperatorNe(cb)              cb(DeeSeq_OperatorNe) cb(DeeSeq_DefaultOperatorNeWithEmpty) cb(DeeSeq_DefaultOperatorNeWithSeqCompareEq) cb(DeeSeq_DefaultOperatorNeWithError)
#define DeeType_SeqCache_Variants_OperatorLo(cb)              cb(DeeSeq_OperatorLo) cb(DeeSeq_DefaultOperatorLoWithEmpty) cb(DeeSeq_DefaultOperatorLoWithSeqCompare) cb(DeeSeq_DefaultOperatorLoWithError)
#define DeeType_SeqCache_Variants_OperatorLe(cb)              cb(DeeSeq_OperatorLe) cb(DeeSeq_DefaultOperatorLeWithEmpty) cb(DeeSeq_DefaultOperatorLeWithSeqCompare) cb(DeeSeq_DefaultOperatorLeWithError)
#define DeeType_SeqCache_Variants_OperatorGr(cb)              cb(DeeSeq_OperatorGr) cb(DeeSeq_DefaultOperatorGrWithEmpty) cb(DeeSeq_DefaultOperatorGrWithSeqCompare) cb(DeeSeq_DefaultOperatorGrWithError)
#define DeeType_SeqCache_Variants_OperatorGe(cb)              cb(DeeSeq_OperatorGe) cb(DeeSeq_DefaultOperatorGeWithEmpty) cb(DeeSeq_DefaultOperatorGeWithSeqCompare) cb(DeeSeq_DefaultOperatorGeWithError)
#define DeeType_SeqCache_Variants_OperatorInplaceAdd(cb)      cb(DeeSeq_OperatorInplaceAdd) cb(DeeSeq_DefaultOperatorInplaceAddWithTSCExtend) cb(DeeObject_DefaultInplaceAddWithAdd)
#define DeeType_SeqCache_Variants_OperatorInplaceMul(cb)      cb(DeeSeq_OperatorInplaceMul) cb(DeeSeq_DefaultOperatorInplaceMulWithTSCClearAndTSCExtend) cb(DeeObject_DefaultInplaceMulWithMul)
/*[[[end]]]*/


/* Sequence function... */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_trygetfirst_t DCALL DeeType_RequireSeqTryGetFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_getfirst_t DCALL DeeType_RequireSeqGetFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_boundfirst_t DCALL DeeType_RequireSeqBoundFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_delfirst_t DCALL DeeType_RequireSeqDelFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_setfirst_t DCALL DeeType_RequireSeqSetFirst(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_trygetlast_t DCALL DeeType_RequireSeqTryGetLast(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_getlast_t DCALL DeeType_RequireSeqGetLast(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_boundlast_t DCALL DeeType_RequireSeqBoundLast(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_dellast_t DCALL DeeType_RequireSeqDelLast(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_setlast_t DCALL DeeType_RequireSeqSetLast(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_any_t DCALL DeeType_RequireSeqAny(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_any_with_key_t DCALL DeeType_RequireSeqAnyWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_any_with_range_t DCALL DeeType_RequireSeqAnyWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_any_with_range_and_key_t DCALL DeeType_RequireSeqAnyWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_all_t DCALL DeeType_RequireSeqAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_all_with_key_t DCALL DeeType_RequireSeqAllWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_all_with_range_t DCALL DeeType_RequireSeqAllWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_all_with_range_and_key_t DCALL DeeType_RequireSeqAllWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_parity_t DCALL DeeType_RequireSeqParity(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_parity_with_key_t DCALL DeeType_RequireSeqParityWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_parity_with_range_t DCALL DeeType_RequireSeqParityWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_parity_with_range_and_key_t DCALL DeeType_RequireSeqParityWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_reduce_t DCALL DeeType_RequireSeqReduce(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_reduce_with_init_t DCALL DeeType_RequireSeqReduceWithInit(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_reduce_with_range_t DCALL DeeType_RequireSeqReduceWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_reduce_with_range_and_init_t DCALL DeeType_RequireSeqReduceWithRangeAndInit(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_min_t DCALL DeeType_RequireSeqMin(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_min_with_key_t DCALL DeeType_RequireSeqMinWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_min_with_range_t DCALL DeeType_RequireSeqMinWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_min_with_range_and_key_t DCALL DeeType_RequireSeqMinWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_max_t DCALL DeeType_RequireSeqMax(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_max_with_key_t DCALL DeeType_RequireSeqMaxWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_max_with_range_t DCALL DeeType_RequireSeqMaxWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_max_with_range_and_key_t DCALL DeeType_RequireSeqMaxWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_sum_t DCALL DeeType_RequireSeqSum(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_sum_with_range_t DCALL DeeType_RequireSeqSumWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_count_t DCALL DeeType_RequireSeqCount(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_count_with_key_t DCALL DeeType_RequireSeqCountWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_count_with_range_t DCALL DeeType_RequireSeqCountWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_count_with_range_and_key_t DCALL DeeType_RequireSeqCountWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_contains_t DCALL DeeType_RequireSeqContains(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_contains_with_key_t DCALL DeeType_RequireSeqContainsWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_contains_with_range_t DCALL DeeType_RequireSeqContainsWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_contains_with_range_and_key_t DCALL DeeType_RequireSeqContainsWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_locate_t DCALL DeeType_RequireSeqLocate(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_locate_with_key_t DCALL DeeType_RequireSeqLocateWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_locate_with_range_t DCALL DeeType_RequireSeqLocateWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_locate_with_range_and_key_t DCALL DeeType_RequireSeqLocateWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_rlocate_with_range_t DCALL DeeType_RequireSeqRLocateWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_rlocate_with_range_and_key_t DCALL DeeType_RequireSeqRLocateWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_startswith_t DCALL DeeType_RequireSeqStartsWith(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_startswith_with_key_t DCALL DeeType_RequireSeqStartsWithWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_startswith_with_range_t DCALL DeeType_RequireSeqStartsWithWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_startswith_with_range_and_key_t DCALL DeeType_RequireSeqStartsWithWithRangeAndKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_endswith_t DCALL DeeType_RequireSeqEndsWith(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_endswith_with_key_t DCALL DeeType_RequireSeqEndsWithWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_endswith_with_range_t DCALL DeeType_RequireSeqEndsWithWithRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_endswith_with_range_and_key_t DCALL DeeType_RequireSeqEndsWithWithRangeAndKey(DeeTypeObject *__restrict self);

/* Mutable sequence functions */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_find_t DCALL DeeType_RequireSeqFind(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_find_with_key_t DCALL DeeType_RequireSeqFindWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_rfind_t DCALL DeeType_RequireSeqRFind(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_rfind_with_key_t DCALL DeeType_RequireSeqRFindWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_erase_t DCALL DeeType_RequireSeqErase(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_insert_t DCALL DeeType_RequireSeqInsert(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_insertall_t DCALL DeeType_RequireSeqInsertAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_pushfront_t DCALL DeeType_RequireSeqPushFront(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_append_t DCALL DeeType_RequireSeqAppend(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_extend_t DCALL DeeType_RequireSeqExtend(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_xchitem_index_t DCALL DeeType_RequireSeqXchItemIndex(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_clear_t DCALL DeeType_RequireSeqClear(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_pop_t DCALL DeeType_RequireSeqPop(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_remove_t DCALL DeeType_RequireSeqRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_remove_with_key_t DCALL DeeType_RequireSeqRemoveWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_rremove_t DCALL DeeType_RequireSeqRRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_rremove_with_key_t DCALL DeeType_RequireSeqRRemoveWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_removeall_t DCALL DeeType_RequireSeqRemoveAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_removeall_with_key_t DCALL DeeType_RequireSeqRemoveAllWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_removeif_t DCALL DeeType_RequireSeqRemoveIf(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_resize_t DCALL DeeType_RequireSeqResize(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_fill_t DCALL DeeType_RequireSeqFill(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_reverse_t DCALL DeeType_RequireSeqReverse(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_reversed_t DCALL DeeType_RequireSeqReversed(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_sort_t DCALL DeeType_RequireSeqSort(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_sort_with_key_t DCALL DeeType_RequireSeqSortWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_sorted_t DCALL DeeType_RequireSeqSorted(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_sorted_with_key_t DCALL DeeType_RequireSeqSortedWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_bfind_t DCALL DeeType_RequireSeqBFind(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_bfind_with_key_t DCALL DeeType_RequireSeqBFindWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_bposition_t DCALL DeeType_RequireSeqBPosition(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_bposition_with_key_t DCALL DeeType_RequireSeqBPositionWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_brange_t DCALL DeeType_RequireSeqBRange(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_brange_with_key_t DCALL DeeType_RequireSeqBRangeWithKey(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_blocate_t DCALL DeeType_RequireSeqBLocate(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_seq_blocate_with_key_t DCALL DeeType_RequireSeqBLocateWithKey(DeeTypeObject *__restrict self);

/* Set functions */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_insert_t DCALL DeeType_RequireSetInsert(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_remove_t DCALL DeeType_RequireSetRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_unify_t DCALL DeeType_RequireSetUnify(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_insertall_t DCALL DeeType_RequireSetInsertAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_removeall_t DCALL DeeType_RequireSetRemoveAll(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_pop_t DCALL DeeType_RequireSetPop(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_set_pop_with_default_t DCALL DeeType_RequireSetPopWithDefault(DeeTypeObject *__restrict self);

/* Map functions */
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setold_t DCALL DeeType_RequireMapSetOld(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setold_ex_t DCALL DeeType_RequireMapSetOldEx(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setnew_t DCALL DeeType_RequireMapSetNew(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setnew_ex_t DCALL DeeType_RequireMapSetNewEx(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_setdefault_t DCALL DeeType_RequireMapSetDefault(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_update_t DCALL DeeType_RequireMapUpdate(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_remove_t DCALL DeeType_RequireMapRemove(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_removekeys_t DCALL DeeType_RequireMapRemoveKeys(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_pop_t DCALL DeeType_RequireMapPop(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_pop_with_default_t DCALL DeeType_RequireMapPopWithDefault(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_popitem_t DCALL DeeType_RequireMapPopItem(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_keys_t DCALL DeeType_RequireMapKeys(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_values_t DCALL DeeType_RequireMapValues(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_iterkeys_t DCALL DeeType_RequireMapIterKeys(DeeTypeObject *__restrict self);
INTDEF ATTR_PURE ATTR_RETNONNULL WUNUSED NONNULL((1)) Dee_tsc_map_itervalues_t DCALL DeeType_RequireMapIterValues(DeeTypeObject *__restrict self);


/* Helpers for constructing enumeration proxy objects. */
#define DeeSeq_InvokeMakeEnumeration(self)                         (*DeeType_RequireSeqMakeEnumeration(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeMakeEnumerationWithIntRange(self, start, end) (*DeeType_RequireSeqMakeEnumerationWithIntRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeMakeEnumerationWithRange(self, start, end)    (*DeeType_RequireSeqMakeEnumerationWithRange(Dee_TYPE(self)))(self, start, end)

/* Helpers for enumerating a sequence by invoking a given callback. */
INTDEF NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_Enumerate(DeeObject *self, DeeObject *cb);
INTDEF NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_EnumerateWithIntRange(DeeObject *self, DeeObject *cb, size_t start, size_t end);
INTDEF NONNULL((1, 2, 3, 4)) DREF DeeObject *DCALL DeeSeq_EnumerateWithRange(DeeObject *self, DeeObject *cb, DeeObject *start, DeeObject *end);



/* Generic sequence operators: treat "self" as a (possibly writable) indexable sequence.
 *
 * When "self" doesn't override any sequence operators, throw errors (unless
 * "self" explicitly uses operators from "Sequence", in which case it is treated
 * like an empty sequence).
 *
 * For this purpose, trust the return value of `DeeType_GetSeqClass()',
 * and wrap/modify operator invocation such that the object behaves as
 * though it was an indexable sequence. */
#define DeeSeq_OperatorBool(self)                                  (*DeeType_RequireSeqOperatorBool(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorIter(self)                                  (*DeeType_RequireSeqOperatorIter(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorSizeOb(self)                                (*DeeType_RequireSeqOperatorSizeOb(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorContains(self, some_object)                 (*DeeType_RequireSeqOperatorContains(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorGetItem(self, index)                        (*DeeType_RequireSeqOperatorGetItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorDelItem(self, index)                        (*DeeType_RequireSeqOperatorDelItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorSetItem(self, index, value)                 (*DeeType_RequireSeqOperatorSetItem(Dee_TYPE(self)))(self, index, value)
#define DeeSeq_OperatorGetRange(self, start, end)                  (*DeeType_RequireSeqOperatorGetRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_OperatorDelRange(self, start, end)                  (*DeeType_RequireSeqOperatorDelRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_OperatorSetRange(self, start, end, values)          (*DeeType_RequireSeqOperatorSetRange(Dee_TYPE(self)))(self, start, end, values)
#define DeeSeq_OperatorForeach(self, proc, arg)                    (*DeeType_RequireSeqOperatorForeach(Dee_TYPE(self)))(self, proc, arg)
#define DeeSeq_OperatorEnumerate(self, proc, arg)                  (*DeeType_RequireSeqOperatorEnumerate(Dee_TYPE(self)))(self, proc, arg)
#define DeeSeq_OperatorEnumerateIndex(self, proc, arg, start, end) (*DeeType_RequireSeqOperatorEnumerateIndex(Dee_TYPE(self)))(self, proc, arg, start, end)
#define DeeSeq_OperatorBoundItem(self, index)                      (*DeeType_RequireSeqOperatorBoundItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorHasItem(self, index)                        (*DeeType_RequireSeqOperatorHasItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorSize(self)                                  (*DeeType_RequireSeqOperatorSize(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorSizeFast(self)                              (*DeeType_RequireSeqOperatorSizeFast(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorGetItemIndex(self, index)                   (*DeeType_RequireSeqOperatorGetItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorDelItemIndex(self, index)                   (*DeeType_RequireSeqOperatorDelItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorSetItemIndex(self, index, value)            (*DeeType_RequireSeqOperatorSetItemIndex(Dee_TYPE(self)))(self, index, value)
#define DeeSeq_OperatorBoundItemIndex(self, index)                 (*DeeType_RequireSeqOperatorBoundItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorHasItemIndex(self, index)                   (*DeeType_RequireSeqOperatorHasItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorGetRangeIndex(self, start, end)             (*DeeType_RequireSeqOperatorGetRangeIndex(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_OperatorDelRangeIndex(self, start, end)             (*DeeType_RequireSeqOperatorDelRangeIndex(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_OperatorSetRangeIndex(self, start, end, values)     (*DeeType_RequireSeqOperatorSetRangeIndex(Dee_TYPE(self)))(self, start, end, values)
#define DeeSeq_OperatorGetRangeIndexN(self, start)                 (*DeeType_RequireSeqOperatorGetRangeIndexN(Dee_TYPE(self)))(self, start)
#define DeeSeq_OperatorDelRangeIndexN(self, start)                 (*DeeType_RequireSeqOperatorDelRangeIndexN(Dee_TYPE(self)))(self, start)
#define DeeSeq_OperatorSetRangeIndexN(self, start, values)         (*DeeType_RequireSeqOperatorSetRangeIndexN(Dee_TYPE(self)))(self, start, values)
#define DeeSeq_OperatorTryGetItem(self, index)                     (*DeeType_RequireSeqOperatorTryGetItem(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorTryGetItemIndex(self, index)                (*DeeType_RequireSeqOperatorTryGetItemIndex(Dee_TYPE(self)))(self, index)
#define DeeSeq_OperatorHash(self)                                  (*DeeType_RequireSeqOperatorHash(Dee_TYPE(self)))(self)
#define DeeSeq_OperatorCompareEq(self, some_object)                (*DeeType_RequireSeqOperatorCompareEq(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorCompare(self, some_object)                  (*DeeType_RequireSeqOperatorCompare(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorTryCompareEq(self, some_object)             (*DeeType_RequireSeqOperatorTryCompareEq(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorEq(self, some_object)                       (*DeeType_RequireSeqOperatorEq(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorNe(self, some_object)                       (*DeeType_RequireSeqOperatorNe(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorLo(self, some_object)                       (*DeeType_RequireSeqOperatorLo(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorLe(self, some_object)                       (*DeeType_RequireSeqOperatorLe(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorGr(self, some_object)                       (*DeeType_RequireSeqOperatorGr(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorGe(self, some_object)                       (*DeeType_RequireSeqOperatorGe(Dee_TYPE(self)))(self, some_object)
#define DeeSeq_OperatorInplaceAdd(p_self, some_object)             (*DeeType_RequireSeqOperatorInplaceAdd(Dee_TYPE(*(p_self))))(p_self, some_object)
#define DeeSeq_OperatorInplaceMul(p_self, some_object)             (*DeeType_RequireSeqOperatorInplaceMul(Dee_TYPE(*(p_self))))(p_self, some_object)
INTDEF WUNUSED NONNULL((1)) int (DCALL DeeSeq_OperatorBool)(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeSeq_OperatorIter)(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeSeq_OperatorSizeOb)(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeSeq_OperatorContains)(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeSeq_OperatorGetItem)(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeSeq_OperatorDelItem)(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeSeq_OperatorSetItem)(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeSeq_OperatorGetRange)(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeSeq_OperatorDelRange)(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeSeq_OperatorSetRange)(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t (DCALL DeeSeq_OperatorForeach)(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t (DCALL DeeSeq_OperatorEnumerate)(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t (DCALL DeeSeq_OperatorEnumerateIndex)(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeSeq_OperatorBoundItem)(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeSeq_OperatorHasItem)(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1)) size_t (DCALL DeeSeq_OperatorSize)(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t (DCALL DeeSeq_OperatorSizeFast)(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeSeq_OperatorGetItemIndex)(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int (DCALL DeeSeq_OperatorDelItemIndex)(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1, 3)) int (DCALL DeeSeq_OperatorSetItemIndex)(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) int (DCALL DeeSeq_OperatorBoundItemIndex)(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int (DCALL DeeSeq_OperatorHasItemIndex)(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeSeq_OperatorGetRangeIndex)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1)) int (DCALL DeeSeq_OperatorDelRangeIndex)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
INTDEF WUNUSED NONNULL((1, 4)) int (DCALL DeeSeq_OperatorSetRangeIndex)(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeSeq_OperatorGetRangeIndexN)(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) int (DCALL DeeSeq_OperatorDelRangeIndexN)(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1, 3)) int (DCALL DeeSeq_OperatorSetRangeIndexN)(DeeObject *self, Dee_ssize_t start, DeeObject *values);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeSeq_OperatorTryGetItem)(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL DeeSeq_OperatorTryGetItemIndex)(DeeObject *self, size_t index);
INTDEF struct type_seq DeeSeq_OperatorSeq;
INTDEF WUNUSED NONNULL((1)) Dee_hash_t (DCALL DeeSeq_OperatorHash)(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeSeq_OperatorCompareEq)(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeSeq_OperatorCompare)(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeSeq_OperatorTryCompareEq)(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeSeq_OperatorEq)(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeSeq_OperatorNe)(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeSeq_OperatorLo)(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeSeq_OperatorLe)(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeSeq_OperatorGr)(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeSeq_OperatorGe)(DeeObject *self, DeeObject *some_object);
INTDEF struct type_cmp DeeSeq_OperatorCmp;
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeSeq_OperatorInplaceAdd)(DREF DeeObject **__restrict p_self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeSeq_OperatorInplaceMul)(DREF DeeObject **__restrict p_self, DeeObject *some_object);



/* Helpers to quickly invoke default sequence functions. */
#define DeeSeq_InvokeTryGetFirst(self)                                        (*DeeType_RequireSeqTryGetFirst(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeGetFirst(self)                                           (*DeeType_RequireSeqGetFirst(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeBoundFirst(self)                                         (*DeeType_RequireSeqBoundFirst(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeDelFirst(self)                                           (*DeeType_RequireSeqDelFirst(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeSetFirst(self, v)                                        (*DeeType_RequireSeqSetFirst(Dee_TYPE(self)))(self, v)
#define DeeSeq_InvokeTryGetLast(self)                                         (*DeeType_RequireSeqTryGetLast(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeGetLast(self)                                            (*DeeType_RequireSeqGetLast(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeBoundLast(self)                                          (*DeeType_RequireSeqBoundLast(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeDelLast(self)                                            (*DeeType_RequireSeqDelLast(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeSetLast(self, v)                                         (*DeeType_RequireSeqSetLast(Dee_TYPE(self)))(self, v)
#define DeeSeq_InvokeAny(self)                                                (*DeeType_RequireSeqAny(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeAnyWithKey(self, key)                                    (*DeeType_RequireSeqAnyWithKey(Dee_TYPE(self)))(self, key)
#define DeeSeq_InvokeAnyWithRange(self, start, end)                           (*DeeType_RequireSeqAnyWithRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeAnyWithRangeAndKey(self, start, end, key)                (*DeeType_RequireSeqAnyWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define DeeSeq_InvokeAll(self)                                                (*DeeType_RequireSeqAll(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeAllWithKey(self, key)                                    (*DeeType_RequireSeqAllWithKey(Dee_TYPE(self)))(self, key)
#define DeeSeq_InvokeAllWithRange(self, start, end)                           (*DeeType_RequireSeqAllWithRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeAllWithRangeAndKey(self, start, end, key)                (*DeeType_RequireSeqAllWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define DeeSeq_InvokeParity(self)                                             (*DeeType_RequireSeqParity(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeParityWithKey(self, key)                                 (*DeeType_RequireSeqParityWithKey(Dee_TYPE(self)))(self, key)
#define DeeSeq_InvokeParityWithRange(self, start, end)                        (*DeeType_RequireSeqParityWithRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeParityWithRangeAndKey(self, start, end, key)             (*DeeType_RequireSeqParityWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define DeeSeq_InvokeReduce(self, combine)                                    (*DeeType_RequireSeqReduce(Dee_TYPE(self)))(self, combine)
#define DeeSeq_InvokeReduceWithInit(self, combine, init)                      (*DeeType_RequireSeqReduceWithInit(Dee_TYPE(self)))(self, combine, init)
#define DeeSeq_InvokeReduceWithRange(self, combine, start, end)               (*DeeType_RequireSeqReduceWithRange(Dee_TYPE(self)))(self, combine, start, end)
#define DeeSeq_InvokeReduceWithRangeAndInit(self, combine, start, end, init)  (*DeeType_RequireSeqReduceWithRangeAndInit(Dee_TYPE(self)))(self, combine, start, end, init)
#define DeeSeq_InvokeMin(self)                                                (*DeeType_RequireSeqMin(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeMinWithKey(self, key)                                    (*DeeType_RequireSeqMinWithKey(Dee_TYPE(self)))(self, key)
#define DeeSeq_InvokeMinWithRange(self, start, end)                           (*DeeType_RequireSeqMinWithRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeMinWithRangeAndKey(self, start, end, key)                (*DeeType_RequireSeqMinWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define DeeSeq_InvokeMax(self)                                                (*DeeType_RequireSeqMax(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeMaxWithKey(self, key)                                    (*DeeType_RequireSeqMaxWithKey(Dee_TYPE(self)))(self, key)
#define DeeSeq_InvokeMaxWithRange(self, start, end)                           (*DeeType_RequireSeqMaxWithRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeMaxWithRangeAndKey(self, start, end, key)                (*DeeType_RequireSeqMaxWithRangeAndKey(Dee_TYPE(self)))(self, start, end, key)
#define DeeSeq_InvokeSum(self)                                                (*DeeType_RequireSeqSum(Dee_TYPE(self)))(self)
#define DeeSeq_InvokeSumWithRange(self, start, end)                           (*DeeType_RequireSeqSumWithRange(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeCount(self, item)                                        (*DeeType_RequireSeqCount(Dee_TYPE(self)))(self, item)
#define DeeSeq_InvokeCountWithKey(self, item, key)                            (*DeeType_RequireSeqCountWithKey(Dee_TYPE(self)))(self, item, key)
#define DeeSeq_InvokeCountWithRange(self, item, start, end)                   (*DeeType_RequireSeqCountWithRange(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeCountWithRangeAndKey(self, item, start, end, key)        (*DeeType_RequireSeqCountWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeContains(self, item)                                     (*DeeType_RequireSeqContains(Dee_TYPE(self)))(self, item)
#define DeeSeq_InvokeContainsWithKey(self, item, key)                         (*DeeType_RequireSeqContainsWithKey(Dee_TYPE(self)))(self, item, key)
#define DeeSeq_InvokeContainsWithRange(self, item, start, end)                (*DeeType_RequireSeqContainsWithRange(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeContainsWithRangeAndKey(self, item, start, end, key)     (*DeeType_RequireSeqContainsWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeLocate(self, item)                                       (*DeeType_RequireSeqLocate(Dee_TYPE(self)))(self, item)
#define DeeSeq_InvokeLocateWithKey(self, item, key)                           (*DeeType_RequireSeqLocateWithKey(Dee_TYPE(self)))(self, item, key)
#define DeeSeq_InvokeLocateWithRange(self, item, start, end)                  (*DeeType_RequireSeqLocateWithRange(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeLocateWithRangeAndKey(self, item, start, end, key)       (*DeeType_RequireSeqLocateWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeRLocateWithRange(self, item, start, end)                 (*DeeType_RequireSeqRLocateWithRange(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeRLocateWithRangeAndKey(self, item, start, end, key)      (*DeeType_RequireSeqRLocateWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeStartsWith(self, item)                                   (*DeeType_RequireSeqStartsWith(Dee_TYPE(self)))(self, item)
#define DeeSeq_InvokeStartsWithWithKey(self, item, key)                       (*DeeType_RequireSeqStartsWithWithKey(Dee_TYPE(self)))(self, item, key)
#define DeeSeq_InvokeStartsWithWithRange(self, item, start, end)              (*DeeType_RequireSeqStartsWithWithRange(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeStartsWithWithRangeAndKey(self, item, start, end, key)   (*DeeType_RequireSeqStartsWithWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeEndsWith(self, item)                                     (*DeeType_RequireSeqEndsWith(Dee_TYPE(self)))(self, item)
#define DeeSeq_InvokeEndsWithWithKey(self, item, key)                         (*DeeType_RequireSeqEndsWithWithKey(Dee_TYPE(self)))(self, item, key)
#define DeeSeq_InvokeEndsWithWithRange(self, item, start, end)                (*DeeType_RequireSeqEndsWithWithRange(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeEndsWithWithRangeAndKey(self, item, start, end, key)     (*DeeType_RequireSeqEndsWithWithRangeAndKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeFind(self, item, start, end)                             (*DeeType_RequireSeqFind(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeFindWithKey(self, item, start, end, key)                 (*DeeType_RequireSeqFindWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeRFind(self, item, start, end)                            (*DeeType_RequireSeqRFind(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeRFindWithKey(self, item, start, end, key)                (*DeeType_RequireSeqRFindWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeErase(self, index, count)                                (*DeeType_RequireSeqErase(Dee_TYPE(self)))(self, index, count)
#define DeeSeq_InvokeInsert(self, index, item)                                (*DeeType_RequireSeqInsert(Dee_TYPE(self)))(self, index, item)
#define DeeSeq_InvokeInsertAll(self, index, items)                            (*DeeType_RequireSeqInsertAll(Dee_TYPE(self)))(self, index, items)
#define DeeSeq_InvokePushFront(self, item)                                    (*DeeType_RequireSeqPushFront(Dee_TYPE(self)))(self, item)
#define DeeSeq_InvokeAppend(self, item)                                       (*DeeType_RequireSeqAppend(Dee_TYPE(self)))(self, item)
#define DeeSeq_InvokeExtend(self, items)                                      (*DeeType_RequireSeqExtend(Dee_TYPE(self)))(self, items)
#define DeeSeq_InvokeXchItemIndex(self, index, value)                         (*DeeType_RequireSeqXchItemIndex(Dee_TYPE(self)))(self, index, value)
#define DeeSeq_InvokeClear(self)                                              (*DeeType_RequireSeqClear(Dee_TYPE(self)))(self)
#define DeeSeq_InvokePop(self, index)                                         (*DeeType_RequireSeqPop(Dee_TYPE(self)))(self, index)
#define DeeSeq_InvokeRemove(self, item, start, end)                           (*DeeType_RequireSeqRemove(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeRemoveWithKey(self, item, start, end, key)               (*DeeType_RequireSeqRemoveWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeRRemove(self, item, start, end)                          (*DeeType_RequireSeqRRemove(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeRRemoveWithKey(self, item, start, end, key)              (*DeeType_RequireSeqRRemoveWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeRemoveAll(self, item, start, end, max)                   (*DeeType_RequireSeqRemoveAll(Dee_TYPE(self)))(self, item, start, end, max)
#define DeeSeq_InvokeRemoveAllWithKey(self, item, start, end, max, key)       (*DeeType_RequireSeqRemoveAllWithKey(Dee_TYPE(self)))(self, item, start, end, max, key)
#define DeeSeq_InvokeRemoveIf(self, should, start, end, max)                  (*DeeType_RequireSeqRemoveIf(Dee_TYPE(self)))(self, should, start, end, max)
#define DeeSeq_InvokeResize(self, newsize, filler)                            (*DeeType_RequireSeqResize(Dee_TYPE(self)))(self, newsize, filler)
#define DeeSeq_InvokeFill(self, start, end, filler)                           (*DeeType_RequireSeqFill(Dee_TYPE(self)))(self, start, end, filler)
#define DeeSeq_InvokeReverse(self, start, end)                                (*DeeType_RequireSeqReverse(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeReversed(self, start, end)                               (*DeeType_RequireSeqReversed(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeSort(self, start, end)                                   (*DeeType_RequireSeqSort(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeSortWithKey(self, start, end, key)                       (*DeeType_RequireSeqSortWithKey(Dee_TYPE(self)))(self, start, end, key)
#define DeeSeq_InvokeSorted(self, start, end)                                 (*DeeType_RequireSeqSorted(Dee_TYPE(self)))(self, start, end)
#define DeeSeq_InvokeSortedWithKey(self, start, end, key)                     (*DeeType_RequireSeqSortedWithKey(Dee_TYPE(self)))(self, start, end, key)
#define DeeSeq_InvokeBFind(self, item, start, end)                            (*DeeType_RequireSeqBFind(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeBFindWithKey(self, item, start, end, key)                (*DeeType_RequireSeqBFindWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeBPosition(self, item, start, end)                        (*DeeType_RequireSeqBPosition(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeBPositionWithKey(self, item, start, end, key)            (*DeeType_RequireSeqBPositionWithKey(Dee_TYPE(self)))(self, item, start, end, key)
#define DeeSeq_InvokeBRange(self, item, start, end, result_range)             (*DeeType_RequireSeqBRange(Dee_TYPE(self)))(self, item, start, end, result_range)
#define DeeSeq_InvokeBRangeWithKey(self, item, start, end, key, result_range) (*DeeType_RequireSeqBRangeWithKey(Dee_TYPE(self)))(self, item, start, end, key, result_range)
#define DeeSeq_InvokeBLocate(self, item, start, end)                          (*DeeType_RequireSeqBLocate(Dee_TYPE(self)))(self, item, start, end)
#define DeeSeq_InvokeBLocateWithKey(self, item, start, end, key)              (*DeeType_RequireSeqBLocateWithKey(Dee_TYPE(self)))(self, item, start, end, key)

/* Set functions */
#define DeeSet_InvokeInsert(self, key)              (*DeeType_RequireSetInsert(Dee_TYPE(self)))(self, key)
#define DeeSet_InvokeRemove(self, key)              (*DeeType_RequireSetRemove(Dee_TYPE(self)))(self, key)
#define DeeSet_InvokeUnify(self, key)               (*DeeType_RequireSetUnify(Dee_TYPE(self)))(self, key)
#define DeeSet_InvokeInsertAll(self, keys)          (*DeeType_RequireSetInsertAll(Dee_TYPE(self)))(self, keys)
#define DeeSet_InvokeRemoveAll(self, keys)          (*DeeType_RequireSetRemoveAll(Dee_TYPE(self)))(self, keys)
#define DeeSet_InvokePop(self)                      (*DeeType_RequireSetPop(Dee_TYPE(self)))(self)
#define DeeSet_InvokePopWithDefault(self, default_) (*DeeType_RequireSetPopWithDefault(Dee_TYPE(self)))(self, default_)

/* Map functions */
#define DeeMap_InvokeSetOld(self, key, value)            (*DeeType_RequireMapSetOld(Dee_TYPE(self)))(self, key, value)
#define DeeMap_InvokeSetOldEx(self, key, value)          (*DeeType_RequireMapSetOldEx(Dee_TYPE(self)))(self, key, value)
#define DeeMap_InvokeSetNew(self, key, value)            (*DeeType_RequireMapSetNew(Dee_TYPE(self)))(self, key, value)
#define DeeMap_InvokeSetNewEx(self, key, value)          (*DeeType_RequireMapSetNewEx(Dee_TYPE(self)))(self, key, value)
#define DeeMap_InvokeSetDefault(self, key, value)        (*DeeType_RequireMapSetDefault(Dee_TYPE(self)))(self, key, value)
#define DeeMap_InvokeUpdate(self, items)                 (*DeeType_RequireMapUpdate(Dee_TYPE(self)))(self, items)
#define DeeMap_InvokeRemove(self, key)                   (*DeeType_RequireMapRemove(Dee_TYPE(self)))(self, key)
#define DeeMap_InvokeRemoveKeys(self, keys)              (*DeeType_RequireMapRemoveKeys(Dee_TYPE(self)))(self, keys)
#define DeeMap_InvokePop(self, key)                      (*DeeType_RequireMapPop(Dee_TYPE(self)))(self, key)
#define DeeMap_InvokePopWithDefault(self, key, default_) (*DeeType_RequireMapPopWithDefault(Dee_TYPE(self)))(self, key, default_)
#define DeeMap_InvokePopItem(self)                       (*DeeType_RequireMapPopItem(Dee_TYPE(self)))(self)
#define DeeMap_InvokeKeys(self)                          (*DeeType_RequireMapKeys(Dee_TYPE(self)))(self)
#define DeeMap_InvokeValues(self)                        (*DeeType_RequireMapValues(Dee_TYPE(self)))(self)
#define DeeMap_InvokeIterKeys(self)                      (*DeeType_RequireMapIterKeys(Dee_TYPE(self)))(self)
#define DeeMap_InvokeIterValues(self)                    (*DeeType_RequireMapIterValues(Dee_TYPE(self)))(self)

/* Possible implementations for sequence cache operators. */
#define DeeSeq_DefaultOperatorBoolWithEmpty (*(Dee_tsc_operator_bool_t)&none_i1)
INTDEF int DCALL none_i1(void *UNUSED(a));
/*INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorBoolWithEmpty(DeeObject *__restrict self);*/
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorBoolWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorIterWithEmpty(DeeObject *__restrict self);
INTDEF /*WUNUSED*/ NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorIterWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorSizeObWithSeqOperatorSize(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorSizeObWithEmpty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorSizeObWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorContainsWithMapTryGetItem(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorContainsWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorContainsWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetItemWithSeqGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetItemWithEmpty(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetItemWithError(DeeObject *self, DeeObject *index);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorDelItemWithEmpty(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorDelItemWithError(DeeObject *self, DeeObject *index);

INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultOperatorSetItemWithEmpty(DeeObject *self, DeeObject *index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultOperatorSetItemWithError(DeeObject *self, DeeObject *index, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeWithSeqGetRangeIndexAndGetRangeIndexN(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeWithEmpty(DeeObject *self, DeeObject *start, DeeObject *end);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeWithError(DeeObject *self, DeeObject *start, DeeObject *end);

INTDEF int DCALL none_i3(void *a, void *b, void *c);
#define DeeSeq_DefaultOperatorDelRangeWithEmpty (*(Dee_tsc_operator_delrange_t)&none_i3)
/*INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultOperatorDelRangeWithEmpty(DeeObject *self, DeeObject *start, DeeObject *end);*/
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultOperatorDelRangeWithError(DeeObject *self, DeeObject *start, DeeObject *end);

INTDEF int DCALL none_i4(void *a, void *b, void *c, void *d);
#define DeeSeq_DefaultOperatorSetRangeWithEmpty (*(Dee_tsc_operator_setrange_t)&none_i4)
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL DeeSeq_DefaultOperatorSetRangeWithError(DeeObject *self, DeeObject *start, DeeObject *end, DeeObject *values);

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultOperatorForeachWithEmpty(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultOperatorForeachWithError(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);

#define DeeSeq_DefaultOperatorEnumerateWithEmpty (*(Dee_tsc_operator_enumerate_t)&DeeSeq_DefaultOperatorForeachWithEmpty)
/*INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultOperatorEnumerateWithEmpty(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg);*/
#define DeeSeq_DefaultOperatorEnumerateWithError (*(Dee_tsc_operator_enumerate_t)&DeeSeq_DefaultOperatorForeachWithError)
/*INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultOperatorEnumerateWithError(DeeObject *__restrict self, Dee_enumerate_t proc, void *arg);*/

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultOperatorEnumerateIndexWithEmpty(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultOperatorEnumerateIndexWithError(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorBoundItemWithSeqBoundItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorBoundItemWithEmpty(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorBoundItemWithError(DeeObject *self, DeeObject *index);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorHasItemWithSeqHasItemIndex(DeeObject *self, DeeObject *index);
/*INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorHasItemWithEmpty(DeeObject *self, DeeObject *index);*/
INTDEF int DCALL none_i2(void *a, void *b);
#define DeeSeq_DefaultOperatorHasItemWithEmpty (*(Dee_tsc_operator_hasitem_t)&none_i2)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorHasItemWithError(DeeObject *self, DeeObject *index);

INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultOperatorSizeWithEmpty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultOperatorSizeWithError(DeeObject *__restrict self);

#define DeeSeq_DefaultOperatorSizeFastWithEmpty DeeSeq_DefaultOperatorSizeWithEmpty
#define DeeSeq_DefaultOperatorSizeFastWithError DeeObject_DefaultSizeFastWithErrorNotFast
/*INTDEF WUNUSED NONNULL((1)) size_t DCALL DeeSeq_DefaultOperatorSizeFastWithError(DeeObject *__restrict self);*/

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetItemIndexWithEmpty(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetItemIndexWithError(DeeObject *self, size_t index);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorDelItemIndexWithEmpty(DeeObject *self, size_t index);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorDelItemIndexWithError(DeeObject *self, size_t index);

INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultOperatorSetItemIndexWithEmpty(DeeObject *self, size_t index, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultOperatorSetItemIndexWithError(DeeObject *self, size_t index, DeeObject *value);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorBoundItemIndexWithSeqSize(DeeObject *self, size_t index);
/*INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorBoundItemIndexWithEmpty(DeeObject *self, size_t index);*/
#define DeeSeq_DefaultOperatorBoundItemIndexWithEmpty (*(Dee_tsc_operator_bounditem_index_t)&DeeSeq_DefaultOperatorBoundItemWithEmpty)
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorBoundItemIndexWithError(DeeObject *self, size_t index);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorHasItemIndexWithSeqSize(DeeObject *self, size_t index);
/*INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorHasItemIndexWithEmpty(DeeObject *self, size_t index);*/
#define DeeSeq_DefaultOperatorHasItemIndexWithEmpty (*(Dee_tsc_operator_hasitem_index_t)&DeeSeq_DefaultOperatorHasItemWithEmpty)
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorHasItemIndexWithError(DeeObject *self, size_t index);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeIndexWithIterAndSeqSize(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);
/*INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeIndexWithEmpty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);*/
#define DeeSeq_DefaultOperatorGetRangeIndexWithEmpty (*(Dee_tsc_operator_getrange_index_t)&DeeSeq_DefaultOperatorGetRangeWithEmpty)
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeIndexWithError(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);

#define DeeSeq_DefaultOperatorDelRangeIndexWithEmpty (*(Dee_tsc_operator_delrange_index_t)&none_i3)
/*INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorDelRangeIndexWithEmpty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);*/
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorDelRangeIndexWithError(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end);

#define DeeSeq_DefaultOperatorSetRangeIndexWithEmpty (*(Dee_tsc_operator_setrange_index_t)&none_i4)
/*INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultOperatorSetRangeIndexWithEmpty(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);*/
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultOperatorSetRangeIndexWithError(DeeObject *self, Dee_ssize_t start, Dee_ssize_t end, DeeObject *values);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeIndexNWithIterAndSeqSize(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeIndexNWithEmpty(DeeObject *self, Dee_ssize_t start);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGetRangeIndexNWithError(DeeObject *self, Dee_ssize_t start);

#define DeeSeq_DefaultOperatorDelRangeIndexNWithEmpty (*(Dee_tsc_operator_delrange_index_n_t)&none_i2)
/*INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorDelRangeIndexNWithEmpty(DeeObject *self, Dee_ssize_t start);*/
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultOperatorDelRangeIndexNWithError(DeeObject *self, Dee_ssize_t start);

#define DeeSeq_DefaultOperatorSetRangeIndexNWithEmpty (*(Dee_tsc_operator_setrange_index_n_t)&none_i3)
/*INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultOperatorSetRangeIndexNWithEmpty(DeeObject *self, Dee_ssize_t start, DeeObject *values);*/
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultOperatorSetRangeIndexNWithError(DeeObject *self, Dee_ssize_t start, DeeObject *values);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorTryGetItemWithSeqTryGetItemIndex(DeeObject *self, DeeObject *index);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorTryGetItemWithEmpty(DeeObject *self, DeeObject *index);
#define DeeSeq_DefaultOperatorTryGetItemWithError DeeSeq_DefaultOperatorGetItemWithError

/*INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorTryGetItemIndexWithEmpty(DeeObject *self, size_t index);*/
#define DeeSeq_DefaultOperatorTryGetItemIndexWithEmpty (*(Dee_tsc_operator_trygetitem_index_t)&DeeSeq_DefaultOperatorTryGetItemWithEmpty)
#define DeeSeq_DefaultOperatorTryGetItemIndexWithError DeeSeq_DefaultOperatorGetItemIndexWithError

INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL DeeSeq_DefaultOperatorHashWithEmpty(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) Dee_hash_t DCALL DeeSeq_DefaultOperatorHashWithError(DeeObject *__restrict self);

/*INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorCompareEqWithEmpty(DeeObject *self, DeeObject *some_object);*/
#define DeeSeq_DefaultOperatorCompareEqWithEmpty DeeSeq_DefaultOperatorCompareWithEmpty
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorCompareEqWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorCompareWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorCompareWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorTryCompareEqWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorTryCompareEqWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorEqWithSeqCompareEq(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorEqWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorEqWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorNeWithSeqCompareEq(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorNeWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorNeWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorLoWithSeqCompare(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorLoWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorLoWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorLeWithSeqCompare(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorLeWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorLeWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGrWithSeqCompare(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGrWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGrWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGeWithSeqCompare(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGeWithEmpty(DeeObject *self, DeeObject *some_object);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultOperatorGeWithError(DeeObject *self, DeeObject *some_object);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorInplaceAddWithTSCExtend(DREF DeeObject **__restrict p_self, DeeObject *some_object);
#define DeeSeq_DefaultOperatorInplaceAddWithEmpty DeeSeq_DefaultOperatorInplaceAddWithError
#define DeeSeq_DefaultOperatorInplaceAddWithError DeeObject_DefaultInplaceAddWithAdd

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultOperatorInplaceMulWithTSCClearAndTSCExtend(DREF DeeObject **__restrict p_self, DeeObject *some_object);
#define DeeSeq_DefaultOperatorInplaceMulWithEmpty DeeSeq_DefaultOperatorInplaceMulWithError
#define DeeSeq_DefaultOperatorInplaceMulWithError DeeObject_DefaultInplaceMulWithMul



/* Possible implementations for sequence cache functions. */
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachReverseWithSizeAndGetItemIndex(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachReverseWithSizeAndTryGetItemIndex(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultForeachReverseWithSizeObAndGetItem(DeeObject *__restrict self, Dee_foreach_t proc, void *arg);

INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndexFast(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexReverseWithSizeAndGetItemIndex(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexReverseWithSizeAndTryGetItemIndex(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL DeeSeq_DefaultEnumerateIndexReverseWithSizeObAndGetItem(DeeObject *__restrict self, Dee_enumerate_index_t proc, void *arg, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithSeqGetFirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithTryGetItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithTryGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithSizeAndGetItemIndexFast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithSizeAndGetItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithSizeAndGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetFirstWithForeach(DeeObject *__restrict self);
#define DeeSeq_DefaultTryGetFirstWithError DeeSeq_DefaultGetFirstWithError

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithCallAttrGetFirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithCallGetFirstDataFunction(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithGetItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithForeach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetFirstWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithCallAttrGetFirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithCallGetFirstDataFunction(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithBoundItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithBoundItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithForeach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundFirstWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithCallAttrDelFirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithCallDelFirstDataFunction(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithDelItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithDelItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithSeqGetFirstAndSetRemove(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithSeqGetFirstAndMaplikeDelItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelFirstWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithCallAttrSetFirst(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithCallSetFirstDataFunction(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithSetItemIndex(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithSetItem(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetFirstWithError(DeeObject *self, DeeObject *value);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetLastWithSeqGetLast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetLastWithSizeAndGetItemIndexFast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetLastWithSizeAndGetItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetLastWithSizeAndGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultTryGetLastWithForeach(DeeObject *__restrict self);
#define DeeSeq_DefaultTryGetLastWithError DeeSeq_DefaultGetLastWithError

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithCallAttrGetLast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithCallGetLastDataFunction(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithSizeAndGetItemIndexFast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithSizeAndGetItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithSizeAndGetItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithForeach(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultGetLastWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithCallAttrGetLast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithCallGetLastDataFunction(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithSizeAndBoundItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithSizeAndBoundItem(DeeObject *__restrict self);
#define DeeSeq_DefaultBoundLastWithForeach DeeSeq_DefaultBoundFirstWithForeach
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultBoundLastWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithCallAttrDelLast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithCallDelLastDataFunction(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithSizeAndDelItemIndex(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithSizeAndDelItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithSeqGetLastAndSetRemove(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithSeqGetLastAndMaplikeDelItem(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultDelLastWithError(DeeObject *__restrict self);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithCallAttrSetLast(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithCallSetLastDataFunction(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithSizeAndSetItemIndex(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithSizeAndSetItem(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultSetLastWithError(DeeObject *self, DeeObject *value);


/* Functions that need additional variants for sequence sub-types that don't have indices (sets, maps) */
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithCallAttrAny(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithCallAnyDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithCallAnyDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithCallAnyDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithSeqForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAttrAnyForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAttrAnyForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithCallAnyDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAnyWithKeyWithSeqForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithCallAttrAny(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithCallAnyDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithCallAnyDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithCallAnyDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAnyWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithCallAttrAny(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithCallAnyDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithCallAnyDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithCallAnyDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAnyWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithCallAttrAll(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithCallAllDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithCallAllDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithCallAllDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithSeqForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAttrAllForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAttrAllForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithCallAllDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAllWithKeyWithSeqForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithCallAttrAll(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithCallAllDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithCallAllDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithCallAllDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultAllWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithCallAttrAll(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithCallAllDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithCallAllDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithCallAllDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultAllWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithCallAttrParity(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithCallParityDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithCallParityDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithCallParityDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithSeqForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallAttrParityForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallAttrParityForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithCallParityDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultParityWithKeyWithSeqForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithCallAttrParity(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithCallParityDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithCallParityDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithCallParityDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultParityWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithCallAttrParity(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithCallParityDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithCallParityDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithCallParityDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultParityWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithCallAttrReduce(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithCallReduceDataFunction(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithCallReduceDataMethod(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithCallReduceDataKwMethod(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithSeqForeach(DeeObject *self, DeeObject *combine);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallAttrReduceForSeq(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataFunctionForSeq(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataMethodForSeq(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataKwMethodForSeq(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallAttrReduceForSetOrMap(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataFunctionForSetOrMap(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataMethodForSetOrMap(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithCallReduceDataKwMethodForSetOrMap(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithInitWithSeqForeach(DeeObject *self, DeeObject *combine, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithCallAttrReduce(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithCallReduceDataFunction(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithCallReduceDataMethod(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithCallReduceDataKwMethod(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *combine, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithCallAttrReduce(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithCallReduceDataFunction(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithCallReduceDataMethod(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithCallReduceDataKwMethod(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultReduceWithRangeAndInitWithSeqEnumerateIndex(DeeObject *self, DeeObject *combine, size_t start, size_t end, DeeObject *init);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithCallAttrMin(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithCallMinDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithCallMinDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithCallMinDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithSeqForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallAttrMinForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallAttrMinForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithCallMinDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithKeyWithSeqForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithCallAttrMin(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithCallMinDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithCallMinDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithCallMinDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithCallAttrMin(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithCallMinDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithCallMinDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithCallMinDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMinWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithCallAttrMax(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithCallMaxDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithCallMaxDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithCallMaxDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithSeqForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallAttrMaxForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataFunctionForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataKwMethodForSeq(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallAttrMaxForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataFunctionForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithCallMaxDataKwMethodForSetOrMap(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithKeyWithSeqForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithCallAttrMax(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithCallMaxDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithCallMaxDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithCallMaxDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithCallAttrMax(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithCallMaxDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithCallMaxDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithCallMaxDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) DREF DeeObject *DCALL DeeSeq_DefaultMaxWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithCallAttrSum(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithCallSumDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithCallSumDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithCallSumDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithSeqForeach(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithCallAttrSum(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithCallSumDataFunction(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithCallSumDataMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithCallSumDataKwMethod(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultSumWithRangeWithSeqEnumerateIndex(DeeObject *self, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithCallAttrCount(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithCallCountDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithCallCountDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithCallCountDataKwMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithSeqForeach(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallAttrCountForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataFunctionForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataKwMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallAttrCountForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataFunctionForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithCallCountDataKwMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) size_t DCALL DeeSeq_DefaultCountWithKeyWithSeqForeach(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithCallAttrCount(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithCallCountDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithCallCountDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithCallCountDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultCountWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithCallAttrCount(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithCallCountDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithCallCountDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithCallCountDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultCountWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

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
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultContainsWithKeyWithSeqForeach(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithCallAttrContains(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithCallContainsDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithCallContainsDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithCallContainsDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultContainsWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithCallAttrContains(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithCallContainsDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithCallContainsDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithCallContainsDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultContainsWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallAttrLocate(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallLocateDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallLocateDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithCallLocateDataKwMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithSeqForeach(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallAttrLocateForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallLocateDataFunctionForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallLocateDataMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallLocateDataKwMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallAttrLocateForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallLocateDataFunctionForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallLocateDataMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithCallLocateDataKwMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithKeyWithSeqForeach(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithCallAttrLocate(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithCallLocateDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithCallLocateDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithCallLocateDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeAndKeyWithCallAttrLocate(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeAndKeyWithCallLocateDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeAndKeyWithCallLocateDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeAndKeyWithCallLocateDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultLocateWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithCallAttrRLocate(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithCallRLocateDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithCallRLocateDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithCallRLocateDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithSeqEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeAndKeyWithCallAttrRLocate(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeAndKeyWithCallRLocateDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeAndKeyWithCallRLocateDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeAndKeyWithCallRLocateDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeAndKeyWithSeqEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultRLocateWithRangeAndKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithCallAttrStartsWith(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithCallStartsWithDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithCallStartsWithDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithCallStartsWithDataKwMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithSeqTryGetFirst(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallAttrStartsWithForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataFunctionForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataKwMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallAttrStartsWithForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataFunctionForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithCallStartsWithDataKwMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultStartsWithWithKeyWithSeqTryGetFirst(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithCallAttrStartsWith(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithCallStartsWithDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithCallStartsWithDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithCallStartsWithDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultStartsWithWithRangeWithSeqTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallAttrStartsWith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallStartsWithDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallStartsWithDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithCallStartsWithDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultStartsWithWithRangeAndKeyWithSeqTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithCallAttrEndsWith(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithCallEndsWithDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithCallEndsWithDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithCallEndsWithDataKwMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithSeqTryGetLast(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallAttrEndsWithForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataFunctionForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataKwMethodForSeq(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallAttrEndsWithForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataFunctionForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithCallEndsWithDataKwMethodForSetOrMap(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeSeq_DefaultEndsWithWithKeyWithSeqTryGetLast(DeeObject *self, DeeObject *item, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithCallAttrEndsWith(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithCallEndsWithDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithCallEndsWithDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithCallEndsWithDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultEndsWithWithRangeWithSeqSizeAndSeqTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallAttrEndsWith(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallEndsWithDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallEndsWithDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithCallEndsWithDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultEndsWithWithRangeAndKeyWithSeqSizeAndSeqTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);



/* Mutable sequence functions */
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithCallAttrFind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithCallFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithCallFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithCallFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultFindWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithCallAttrFind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithCallFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithCallFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithCallFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultFindWithKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithCallAttrRFind(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithCallRFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithCallRFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithCallRFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithSeqEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRFindWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithCallAttrRFind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithCallRFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithCallRFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithCallRFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndexReverse(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultRFindWithKeyWithSeqEnumerateIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

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
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithSeqInsertAll(DeeObject *self, size_t index, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertWithError(DeeObject *self, size_t index, DeeObject *item);

INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithCallAttrInsertAll(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithCallInsertAllDataFunction(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithCallInsertAllDataMethod(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithCallInsertAllDataKwMethod(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithSetRangeIndex(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithSeqInsert(DeeObject *self, size_t index, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultInsertAllWithError(DeeObject *self, size_t index, DeeObject *items);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithCallAttrPushFront(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithCallPushFrontDataFunction(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithCallPushFrontDataMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithCallPushFrontDataKwMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultPushFrontWithSeqInsert(DeeObject *self, DeeObject *item);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAttrAppend(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAttrPushBack(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAppendDataFunction(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAppendDataMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithCallAppendDataKwMethod(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithSeqExtend(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithSizeAndSeqInsert(DeeObject *self, DeeObject *item);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultAppendWithError(DeeObject *self, DeeObject *item);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithCallAttrExtend(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithCallExtendDataFunction(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithCallExtendDataMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithCallExtendDataKwMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithSeqAppend(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultExtendWithSizeAndSeqInsertAll(DeeObject *self, DeeObject *items);
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
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithSeqErase(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithSeqRemoveAll(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithMapRemoveKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultClearWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithCallAttrPop(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithCallPopDataFunction(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithCallPopDataMethod(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithCallPopDataKwMethod(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithSizeAndGetItemIndexAndSeqErase(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, Dee_ssize_t index);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSeq_DefaultPopWithError(DeeObject *self, Dee_ssize_t index);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallAttrRemove(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithCallRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithSeqRemoveAll(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithSeqRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithSeqFindAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithSeqEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRemoveWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallAttrRemove(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithCallRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithSeqRemoveAllWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithSeqRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithSeqFindWithKeyAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithSeqEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRemoveWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallAttrRRemove(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallRRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallRRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithCallRRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithTSeqFindAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithSeqEnumerateIndexReverseAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithSeqEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSeq_DefaultRRemoveWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallAttrRRemove(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithCallRRemoveDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithSeqRFindWithKeyAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithSeqEnumerateIndexReverseAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithSeqEnumerateIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeSeq_DefaultRRemoveWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallAttrRemoveAll(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallRemoveAllDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallRemoveAllDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithCallRemoveAllDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithSeqRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithSeqRemove(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveAllWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max);

INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallAttrRemoveAll(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithCallRemoveAllDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithSeqRemoveIf(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithSeqRemoveWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 6)) size_t DCALL DeeSeq_DefaultRemoveAllWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallAttrRemoveIf(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallRemoveIfDataFunction(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallRemoveIfDataMethod(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithCallRemoveIfDataKwMethod(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithSeqRemoveAllWithKey(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithSizeAndGetItemIndexAndDelItemIndex(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultRemoveIfWithError(DeeObject *self, DeeObject *should, size_t start, size_t end, size_t max);

INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithCallAttrResize(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithCallResizeDataFunction(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithCallResizeDataMethod(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithCallResizeDataKwMethod(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithSizeAndSetRangeIndexAndDelRangeIndex(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithSizeAndSetRangeIndex(DeeObject *self, size_t newsize, DeeObject *filler);
INTDEF WUNUSED NONNULL((1, 3)) int DCALL DeeSeq_DefaultResizeWithSizeAndSeqEraseAndSeqExtend(DeeObject *self, size_t newsize, DeeObject *filler);

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
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultReverseWithSeqReversedAndSetRangeIndex(DeeObject *self, size_t start, size_t end);
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
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithSeqSortedAndSetRangeIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithSizeAndGetItemIndexAndSetItemIndex(DeeObject *self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) int DCALL DeeSeq_DefaultSortWithError(DeeObject *self, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithCallAttrSort(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithCallSortDataFunction(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithCallSortDataMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithCallSortDataKwMethod(DeeObject *self, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 4)) int DCALL DeeSeq_DefaultSortWithKeyWithSeqSortedAndSetRangeIndex(DeeObject *self, size_t start, size_t end, DeeObject *key);
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
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithSeqBRange(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBFindWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithCallAttrBFind(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithCallBFindDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithCallBFindDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithCallBFindDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithSeqBRangeWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBFindWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithCallAttrBPosition(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithCallBPositionDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithCallBPositionDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithCallBPositionDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithSeqBRange(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) size_t DCALL DeeSeq_DefaultBPositionWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithCallAttrBPosition(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithCallBPositionDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithCallBPositionDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithCallBPositionDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) size_t DCALL DeeSeq_DefaultBPositionWithKeyWithSeqBRangeWithKey(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
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
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithSeqBFindAndGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithError(DeeObject *self, DeeObject *item, size_t start, size_t end);

INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithKeyWithCallAttrBLocate(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithKeyWithCallBLocateDataFunction(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithKeyWithCallBLocateDataMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithKeyWithCallBLocateDataKwMethod(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithKeyWithSizeAndTryGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithKeyWithSeqBFindWithKeyAndGetItemIndex(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *DCALL DeeSeq_DefaultBLocateWithKeyWithError(DeeObject *self, DeeObject *item, size_t start, size_t end, DeeObject *key);



/************************************************************************/
/* For `deemon.Set'                                                     */
/************************************************************************/
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithCallAttrInsert(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithCallInsertDataFunction(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithCallInsertDataMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithCallInsertDataKwMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithSeqSizeAndSetInsertAll(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithMapSetNew(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithSeqSeqContainsAndSeqAppend(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithCallAttrRemove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithCallRemoveDataFunction(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithCallRemoveDataMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithCallRemoveDataKwMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithSeqSizeAndSeqRemoveAll(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithMapTryGetItemAndMapDelItem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithSeqRemove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithCallAttrUnify(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithCallUnifyDataFunction(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithCallUnifyDataMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithCallUnifyDataKwMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithSetInsertAndSeqForeach(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithSeqLocateAndSeqAppend(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultUnifyWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithCallAttrInsertAll(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithCallInsertAllDataFunction(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithCallInsertAllDataMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithCallInsertAllDataKwMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithInplaceAdd(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithInplaceOr(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithSetInsert(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultInsertAllWithError(DeeObject *self, DeeObject *keys);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithCallAttrRemoveAll(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithCallRemoveAllDataFunction(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithCallRemoveAllDataMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithCallRemoveAllDataKwMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithInplaceSub(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithSetRemove(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeSet_DefaultRemoveAllWithError(DeeObject *self, DeeObject *keys);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithCallAttrPop(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithCallPopDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithCallPopDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithCallPopDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithSetFirstAndSetRemove(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithMapPopItem(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithSeqPop(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeSet_DefaultPopWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithCallAttrPop(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithCallPopDataFunction(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithCallPopDataMethod(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithCallPopDataKwMethod(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithSeqTryGetFirstAndSetRemove(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithMapPopItem(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithSeqPop(DeeObject *self, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeSet_DefaultPopWithDefaultWithError(DeeObject *self, DeeObject *default_);



/************************************************************************/
/* For `deemon.Mapping'                                                 */
/************************************************************************/
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithCallAttrSetOld(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithCallSetOldDataFunction(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithCallSetOldDataMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithCallSetOldDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithMapSetOldEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithBoundItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetOldWithError(DeeObject *self, DeeObject *key, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithCallAttrSetOldEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithCallSetOldExDataFunction(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithCallSetOldExDataMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithCallSetOldExDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetOldExWithError(DeeObject *self, DeeObject *key, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithCallAttrSetNew(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithCallSetNewDataFunction(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithCallSetNewDataMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithCallSetNewDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithMapSetNewEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithBoundItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithBoundItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithTryGetItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithGetItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeMap_DefaultSetNewWithError(DeeObject *self, DeeObject *key, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithCallAttrSetNewEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithCallSetNewExDataFunction(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithCallSetNewExDataMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithCallSetNewExDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithTryGetItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithGetItemAndMapSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetNewExWithError(DeeObject *self, DeeObject *key, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithCallAttrSetDefault(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithCallSetDefaultDataFunction(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithCallSetDefaultDataMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithCallSetDefaultDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithMapSetNewEx(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithMapSetNewAndGetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithTryGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithGetItemAndSetItem(DeeObject *self, DeeObject *key, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultSetDefaultWithError(DeeObject *self, DeeObject *key, DeeObject *value);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithCallAttrUpdate(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithCallUpdateDataFunction(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithCallUpdateDataMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithCallUpdateDataKwMethod(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithInplaceAdd(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithInplaceOr(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithSetItem(DeeObject *self, DeeObject *items);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultUpdateWithError(DeeObject *self, DeeObject *items);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithCallAttrRemove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithCallRemoveDataFunction(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithCallRemoveDataMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithCallRemoveDataKwMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithBoundItemAndDelItem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithSizeAndDelItem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithSizeAndMapRemoveKeys(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithCallAttrRemoveKeys(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithCallRemoveKeysDataFunction(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithCallRemoveKeysDataMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithCallRemoveKeysDataKwMethod(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithDelItem(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithMapRemove(DeeObject *self, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeMap_DefaultRemoveKeysWithError(DeeObject *self, DeeObject *keys);

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithCallAttrPop(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithCallPopDataFunction(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithCallPopDataMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithCallPopDataKwMethod(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithGetItemAndMapRemove(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithGetItemAndDelItem(DeeObject *self, DeeObject *key);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeMap_DefaultPopWithError(DeeObject *self, DeeObject *key);

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithCallAttrPop(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithCallPopDataFunction(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithCallPopDataMethod(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithCallPopDataKwMethod(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithTryGetItemAndMapRemove(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithTryGetItemAndDelItem(DeeObject *self, DeeObject *key, DeeObject *default_);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL DeeMap_DefaultPopWithDefaultWithError(DeeObject *self, DeeObject *key, DeeObject *default_);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithCallAttrPopItem(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithCallPopItemDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithCallPopItemDataMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithCallPopItemDataKwMethod(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithSeqTryGetFirstAndMapRemove(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithSeqTryGetFirstAndDelItem(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultPopItemWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultKeysWithCallAttrKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultKeysWithCallKeysDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultKeysWithMapIterKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultKeysWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultValuesWithCallAttrValues(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultValuesWithCallValuesDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultValuesWithMapIterValues(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultValuesWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithCallAttrIterKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithCallIterKeysDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithMapKeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterKeysWithError(DeeObject *self);

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterValuesWithCallAttrIterValues(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterValuesWithCallIterValuesDataFunction(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterValuesWithMapValues(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterValuesWithIter(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeMap_DefaultIterValuesWithError(DeeObject *self);





/* Default sequence function hooks (used as function pointers of `type_method' / `type_getset' of Sequence/Set/Mapping) */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_getfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default_seq_boundfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default_seq_delfirst(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default_seq_setfirst(DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_seq_getlast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default_seq_boundlast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL default_seq_dellast(DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL default_seq_setlast(DeeObject *self, DeeObject *value);


/* Default sequence function pointers (including ones for mutable sequences). */
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


/* Default set function pointers (including ones for mutable sets). */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set_insert(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set_remove(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set_insertall(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set_removeall(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set_unify(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_set_pop(DeeObject *self, size_t argc, DeeObject *const *argv);


/* Default map function pointers (including ones for mutable maps). */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_get(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_setold(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_setold_ex(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_setnew(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_setnew_ex(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_setdefault(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_update(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_remove(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_removekeys(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_pop(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_popitem(DeeObject *self, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_keys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_values(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_iterkeys(DeeObject *self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL default_map_itervalues(DeeObject *self);


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_API_H */
